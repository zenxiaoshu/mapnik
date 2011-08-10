/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Artem Pavlenko
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

// mapnik
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/text_processing.hpp>

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(text_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    text_processor &processor = *(sym.get_placement_options()->properties.processor); //TODO
    text_symbolizer_properties const& p = sym.get_placement_options()->properties; //TODO
    /* placement_options will return a text_processor which then produces the text_symbolizer_properties. For now a lot of this is a hack to not completely break other code in mapnik. */
    /*
    bool placement_found = false;
    while (!placement_found && placement->next())
    {
        while (!placement_found && placement->next_position_only())
        {
    text_placement_info_ptr placement = sym.get_placement_options()->get_placement_info();
    text_symbolizer_properties &p = placement->properties;
        }
    }*/
    processed_text text(detector_, font_manager_, box2d<double>(0, 0, width_, height_), scale_factor_);
    processor.process(text, feature);
    processed_text::expression_list::const_iterator itr = text.begin();
    processed_text::expression_list::const_iterator end = text.end();
    for (; itr!=end; ++itr)
    {
        std::cout << "Text '" << itr->str << "' with color '" << itr->p.fill.to_string() << "'\n";
    }
    unsigned num_geom = feature.num_geometries();
    for (unsigned i=0; i<num_geom; ++i)
    {
        geometry_type const& geom = feature.get_geometry(i);
        if (geom.num_points() == 0) continue; // don't bother with empty geometries
        if (p.label_placement == POINT_PLACEMENT ||
            p.label_placement == INTERIOR_PLACEMENT)
        {
            double label_x=0.0;
            double label_y=0.0;
            double z=0.0;
            if (p.label_placement == POINT_PLACEMENT)
                geom.label_position(&label_x, &label_y);
            else
                geom.label_interior_position(&label_x, &label_y);
            prj_trans.backward(label_x,label_y, z);
            t_.forward(&label_x,&label_y);

            text.find_point_placement(label_x, label_y);
        //    finder.update_detector(*placement);
        }
    }
#if 0
    for (unsigned int ii = 0; ii < text_placement.placements.size(); ++ii)
    {
        double x = text_placement.placements[ii].starting_x;
        double y = text_placement.placements[ii].starting_y;
        ren.prepare_glyphs(&text_placement.placements[ii]);
        ren.render(x,y);
    }
#endif



#if 0
    typedef  coord_transform2<CoordTransform,geometry_type> path_type;


        face_set_ptr faces;
        if (p.fontset.size() > 0)
        {
            faces = font_manager_.get_face_set(p.fontset);
        }
        else
        {
            faces = font_manager_.get_face_set(p.face_name);
        }

        stroker_ptr strk = font_manager_.get_stroker();
        if (!(faces->size() > 0 && strk))
        {
            throw config_error("Unable to find specified font face '" + p.face_name + "'");
        }
        text_renderer<T> ren(pixmap_, faces, *strk);
        ren.set_pixel_size(p.text_size * scale_factor_);
        ren.set_fill(p.fill);
        ren.set_halo_fill(p.halo_fill);
        ren.set_halo_radius(p.halo_radius * scale_factor_);
        ren.set_opacity(p.text_opacity);

        box2d<double> dims(0, 0, width_, height_);
        placement_finder<label_collision_detector4> finder(detector_, dims);

        string_info info(text);

        faces->get_string_info(info);
        metawriter_with_properties writer = sym.get_metawriter();


            while (!placement_found && placement->next_position_only())
            {
                placement->init(&info, scale_factor_);
                if (writer.first)
                    placement->collect_extents = true; // needed for inmem metawriter

                else if ( geom.num_points() > 1 && p.label_placement == LINE_PLACEMENT)
                {
                    path_type path(t_,geom,prj_trans);
                    finder.find_line_placements<path_type>(*placement, path);
                }

                if (!placement->placements.size()) continue;
                placement_found = true;


                if (writer.first) writer.first->add_text(*placement, faces, feature, t_, writer.second);
            }
        }
    }
#endif
}

template void agg_renderer<image_32>::process(text_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
