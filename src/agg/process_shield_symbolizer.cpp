/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/
//$Id$

#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/shield_symbolizer.hpp>
#include <mapnik/text_processing.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/expression_evaluator.hpp>

// boost
#include <boost/make_shared.hpp>

namespace mapnik {

template <typename T>
void  agg_renderer<T>::process(shield_symbolizer const& sym,
                               Feature const& feature,
                               proj_transform const& prj_trans)
{
    typedef  coord_transform2<CoordTransform,geometry_type> path_type;
    metawriter_with_properties writer = sym.get_metawriter();

    box2d<double> dims(0, 0, width_, height_);

    stroker_ptr strk = font_manager_.get_stroker();
    text_renderer<T> ren(pixmap_, font_manager_, *strk);

    bool placement_found = false;
    text_placement_info_ptr placement = sym.get_placement_options()->get_placement_info();
    if (writer.first)
        placement->collect_extents = true; // needed for inmem metawriter
    while (!placement_found && placement->next()) {
        text_processor &processor = placement->properties.processor;
        text_symbolizer_properties const& p = placement->properties;

        processed_text text(font_manager_, scale_factor_);
        processor.process(text, feature);
        if (!sym.get_no_text() && text.empty()) {
//            std::cout << "No text";
            continue;
        }

        string_info &info = text.get_string_info();
        char_info dummy(0, 1, 1, 0, 1);
        dummy.format = &(processor.defaults);
        if (!info.num_characters()) info.add_info(dummy); //Initialize dummy string info

        placement_finder<label_collision_detector4> finder(*placement, info, *detector_, dims);

        agg::trans_affine tr;
        boost::array<double,6> const& m = sym.get_transform(); /* TODO: placements */
        tr.load_from(&m[0]);

        std::string filename = path_processor_type::evaluate(*sym.get_filename(), feature); /* TODO: placements */
        mapnik::marker_ptr marker;
        if (!filename.empty())
        {
            boost::optional<marker_ptr> marker_tmp = marker_cache::instance()->find(filename, true);
            if (marker_tmp) marker = *marker_tmp; else continue;
        }
        else continue;
        // At this point marker is initalized and text is not empty (except if no_text is set)

        double w = marker->width();
        double h = marker->height();
        placement->init(scale_factor_, w, h, p.label_placement == LINE_PLACEMENT);

        for (unsigned i = 0; i < feature.num_geometries(); ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            if (geom.num_points() == 0 ) continue;

            path_type path(t_, geom, prj_trans);

            if (p.label_placement == POINT_PLACEMENT || p.label_placement == VERTEX_PLACEMENT || p.label_placement == INTERIOR_PLACEMENT)
            {
                // for every vertex, try and place a shield/text
                geom.rewind(0);
                position const& shield_pos = sym.get_shield_displacement();
                // I think this loop should terminate after just one iteration for INTERIOR and POINT placement.
                for( unsigned jj = 0; jj < geom.num_points(); jj++ )
                {
                    double label_x;
                    double label_y;
                    double z=0.0;

                    if (p.label_placement == VERTEX_PLACEMENT)
                        geom.vertex(&label_x,&label_y);  // by vertex
                    else if (p.label_placement == INTERIOR_PLACEMENT)
                        geom.label_interior_position(&label_x,&label_y);
                    else
                        geom.label_position(&label_x, &label_y);  // by middle of line or by point
                    prj_trans.backward(label_x,label_y, z);
                    t_.forward(&label_x,&label_y);

                    label_x += boost::get<0>(shield_pos);
                    label_y += boost::get<1>(shield_pos);

                    finder.find_point_placement(label_x, label_y, 0.0 /*angle*/);

                    // check to see if image overlaps anything too, there is only ever 1 placement found for points and verticies
                    if(placement->placements.size() == 0) continue;
                    double x = floor(placement->placements[0].starting_x);
                    double y = floor(placement->placements[0].starting_y);
                    int px;
                    int py;
                    box2d<double> label_ext;

                    if(!sym.get_unlock_image()) /*TODO: Placements */
                    {
                        // center image at text center position
                        // remove displacement from image label
                        double lx = x - boost::get<0>(p.displacement);
                        double ly = y - boost::get<1>(p.displacement);
                        px=int(floor(lx - (0.5 * w))) + 1;
                        py=int(floor(ly - (0.5 * h))) + 1;
                        label_ext.init( floor(lx - 0.5 * w), floor(ly - 0.5 * h), ceil (lx + 0.5 * w), ceil (ly + 0.5 * h) );
                    }
                    else
                    {  // center image at reference location
                        px=int(floor(label_x - 0.5 * w));
                        py=int(floor(label_y - 0.5 * h));
                        label_ext.init( floor(label_x - 0.5 * w), floor(label_y - 0.5 * h), ceil (label_x + 0.5 * w), ceil (label_y + 0.5 * h));
                    }

                    if (p.allow_overlap || detector_->has_placement(label_ext) )
                    {
                        detector_->insert(label_ext);
                        finder.update_detector();
                        if (writer.first) {
                            writer.first->add_box(box2d<double>(px,py,px+w,py+h), feature, t_, writer.second);
                            writer.first->add_text(*placement, font_manager_, feature, t_, writer.second);
                        }
                        render_marker(px, py, *marker, tr, sym.get_opacity()); /*TODO: Placement*/
                        ren.prepare_glyphs(&(placement->placements[0]));
                        ren.render(x,y);
                        placement_found = true;
                    }
                }
            }
            else if (geom.num_points() > 1 && p.label_placement == LINE_PLACEMENT)
            {

                finder.find_point_placements<path_type>(path);

                finder.update_detector();
                if (writer.first) writer.first->add_text(*placement, font_manager_, feature, t_, writer.second);

                for (unsigned int ii = 0; ii < placement->placements.size(); ++ ii)
                {
                    double x = floor(placement->placements[ii].starting_x);
                    double y = floor(placement->placements[ii].starting_y);

                    double lx = x - boost::get<0>(p.displacement);
                    double ly = y - boost::get<1>(p.displacement);
                    int px=int(floor(lx - (0.5*w))) + 1;
                    int py=int(floor(ly - (0.5*h))) + 1;

                    if (writer.first)
                        writer.first->add_box(box2d<double>(px, py, px+w, py+h), feature, t_, writer.second);
                    placement_found = true;

                    render_marker(px, py, *marker, tr, sym.get_opacity());
                    ren.prepare_glyphs(&(placement->placements[ii]));
                    ren.render(x,y);

                }
            }
        }
    }
}


template void agg_renderer<image_32>::process(shield_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
