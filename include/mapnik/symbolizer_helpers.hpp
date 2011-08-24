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
#ifndef SYMBOLIZER_HELPERS_HPP
#define SYMBOLIZER_HELPERS_HPP

#include <mapnik/text_symbolizer.hpp>
#include <mapnik/text_processing.hpp>
#include <mapnik/placement_finder.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/feature.hpp>

#include <boost/shared_ptr.hpp>

namespace mapnik {
class text_symbolizer_helper
{
private:
    boost::shared_ptr<processed_text> text;
public:
    text_symbolizer_helper() : text() {}

template <typename FaceManagerT, typename DetectorT>
text_placement_info_ptr get_placement(const text_symbolizer &sym, const Feature &feature, FaceManagerT &font_manager, DetectorT &detector, proj_transform const& prj_trans, CoordTransform const &t_, unsigned width, unsigned height, double scale_factor)
{
    text = boost::shared_ptr<processed_text>(new processed_text(font_manager, scale_factor));
    metawriter_with_properties writer = sym.get_metawriter();

    box2d<double> dims(0, 0, width, height);
    placement_finder<label_collision_detector4> finder(detector, dims);

    text_placement_info_ptr placement = sym.get_placement_options()->get_placement_info();
    placement->init(scale_factor);
    if (writer.first)
        placement->collect_extents = true; // needed for inmem metawriter

    while (placement->next()) {
        text_processor &processor = placement->properties.processor;
        text_symbolizer_properties const& p = placement->properties;

        text->clear();
        processor.process(*text, feature);
        string_info &info = text->get_string_info();

        unsigned num_geom = feature.num_geometries();
        for (unsigned i=0; i<num_geom; ++i)
        {
            geometry_type const& geom = feature.get_geometry(i);
            if (geom.num_points() == 0) continue; // don't bother with empty geometries

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
            }
            if (writer.first) writer.first->add_text(*placement, font_manager, feature, t_, writer.second);

            return placement;
        }
    }
    return placement; //No placement found. This placement has empty placements member, so no rendering is done
}
};

}
#endif
