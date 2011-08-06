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

#include <list>
#include <utility>

namespace mapnik
{
class abstract_token;
typedef std::pair<text_properties, UnicodeString> formated_expression;
typedef std::list<formated_expression> formated_text;


class text_processor
{
public:
    text_processor();
    void from_xml(boost::property_tree::ptree const& pt);
    /*void to_xml(boost::property_tree::ptree &node); */
    void process(formated_text &output, Feature const& feature);
    void set_defaults(text_properties const& defaults);
private:
    std::list<abstract_token *> list_;
    text_properties defaults_;
};

} /* namespace */

#endif
