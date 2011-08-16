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

// mapnik
#include <mapnik/grid/grid_renderer.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text_processing.hpp>

namespace mapnik {

template <typename T>
void grid_renderer<T>::process(text_symbolizer const& sym,
                              Feature const& feature,
                              proj_transform const& prj_trans)
{
    metawriter_with_properties writer = sym.get_metawriter();

    box2d<double> dims(0, 0, width_, height_);
    placement_finder<label_collision_detector4> finder(detector_, dims);

    stroker_ptr strk = font_manager_.get_stroker();

    bool placement_found = false;
    text_placement_info_ptr placement = sym.get_placement_options()->get_placement_info();

    while (!placement_found && placement->next()) {
        text_processor &processor = *(placement->properties.processor);
        text_symbolizer_properties const& p = placement->properties;

        processed_text text(font_manager_, scale_factor_ * (1.0/pixmap_.get_resolution()));
        processor.process(text, feature);
        string_info &info = text.get_string_info();

        unsigned num_geom = feature.num_geometries();
        for (unsigned i=0; i<num_geom; ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            if (geom.num_points() == 0) continue; // don't bother with empty geometries

            if (writer.first)
                placement->collect_extents = true; // needed for inmem metawriter

            double angle = 0.0;
            if (p.orientation)
            {
                // apply rotation
                value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*(p.orientation));
                angle = result.to_double();
            }
            finder.find_placement(*placement, info, angle, geom, t_, prj_trans);

            if (!placement->placements.size()) {
                continue;
            } else {
                placement_found = true;
            }
            text_renderer<T> ren(pixmap_, font_manager_, *strk);
            for (unsigned int ii = 0; ii < placement->placements.size(); ++ii)
            {
                double x = placement->placements[ii].starting_x;
                double y = placement->placements[ii].starting_y;
                ren.prepare_glyphs(&(placement->placements[ii]));
                ren.render_id(feature.id(),x,y,2);
            }

//            if (writer.first) writer.first->add_text(*placement, faces, feature, t_, writer.second);
        }
    }
    if (placement_found)
        pixmap_.add_feature(feature);
}

template void grid_renderer<grid>::process(text_symbolizer const&,
                                              Feature const&,
                                              proj_transform const&);

}
 
