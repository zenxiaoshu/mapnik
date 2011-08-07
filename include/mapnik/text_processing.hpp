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
#ifndef MAPNIK_TEXT_PROCESSING_HPP
#define MAPNIK_TEXT_PROCESSING_HPP

#include <boost/property_tree/ptree.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/text_placements.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/ctrans.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text_path.hpp>

#include <list>
namespace mapnik
{


typedef coord_transform2<CoordTransform,geometry_type> path_type; //TODO: Put this into a global header and remove it from each file it is used.
typedef label_collision_detector4 DetectorType;

class abstract_token;
class processed_text;

class processed_expression
{
public:
    processed_expression(text_properties const& properties, UnicodeString const& text) :
        p(properties), str(text), info(0) {}
    ~processed_expression() { if (info) delete info; }
    text_properties p;
    UnicodeString str;
    string_info *info;
private:
    friend class processed_text;
};

class processed_text
{
public:
    processed_text(DetectorType & detector, face_manager<freetype_engine> & font_manager, box2d<double> dimensions, double scale_factor);
    void push_back(processed_expression const& exp);
    void clear();
    typedef std::list<processed_expression> expression_list;
    expression_list::const_iterator begin();
    expression_list::const_iterator end();
    bool find_point_placement(double x, double y);
protected:
    void buffer_char_sizes();
private:
    expression_list expr_list_;
    DetectorType &detector_;
    box2d<double> dimensions_;
    face_manager<freetype_engine> & font_manager_;
    double scale_factor_;
};


class text_processor
{
public:
    text_processor();
    void from_xml(boost::property_tree::ptree const& pt);
    /*void to_xml(boost::property_tree::ptree &node); */
    void process(processed_text &output, Feature const& feature);
    void set_defaults(text_properties const& defaults);
private:
    std::list<abstract_token *> list_;
    text_properties defaults_;
};

} /* namespace */

#endif
