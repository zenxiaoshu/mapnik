/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Hermann Kraus
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

#include <mapnik/text_placements.hpp>
#include <mapnik/text_placements_simple.hpp>
#include <mapnik/text_placements_list.hpp>

#include <boost/tuple/tuple.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/optional.hpp>
#include <mapnik/ptree_helpers.hpp>

namespace mapnik {

namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;
using boost::spirit::ascii::space;
using phoenix::push_back;
using phoenix::ref;
using qi::_1;
using boost::optional;

text_properties::text_properties() :
    text_size(10),
    anchor(0.0,0.5),
    label_placement(POINT_PLACEMENT),
    halign(H_AUTO),
    jalign(J_MIDDLE),
    valign(V_AUTO),
    line_spacing(0),
    character_spacing(0),
    label_spacing(0),
    label_position_tolerance(0),
    avoid_edges(false),
    minimum_distance(0.0),
    minimum_padding(0.0),
    max_char_angle_delta(22.5 * M_PI/180.0),
    force_odd_labels(false),
    allow_overlap(false),
    text_opacity(1.0),
    text_ratio(0),
    wrap_before(false),
    wrap_width(0),
    wrap_char(' '),
    text_transform(NONE),
    fill(color(0,0,0)),
    halo_fill(color(255,255,255)),
    halo_radius(0)
{
}

void text_properties::set_values_from_xml(boost::property_tree::ptree const &sym, std::map<std::string,font_set> const & fontsets)
{
    optional<unsigned> text_size_ = get_opt_attr<unsigned>(sym, "size");
    if (text_size_) text_size = *text_size_;
    optional<color> fill_ = get_opt_attr<color>(sym, "fill");
    if (fill_) fill = *fill_;
    optional<label_placement_e> placement_ = get_opt_attr<label_placement_e>(sym, "placement");
    if (placement_) label_placement = *placement_;
    optional<vertical_alignment_e> valign_ = get_opt_attr<vertical_alignment_e>(sym, "vertical-alignment");
    if (valign_) valign = *valign_;
    optional<color> halo_fill_ = get_opt_attr<color>(sym, "halo-fill");
    if (halo_fill_) halo_fill = *halo_fill_;
    optional<double> halo_radius_ = get_opt_attr<double>(sym, "halo-radius");
    if (halo_radius_) halo_radius = *halo_radius_;
    optional<unsigned> text_ratio_ = get_opt_attr<unsigned>(sym, "text-ratio");
    if (text_ratio_) text_ratio = *text_ratio_;
    optional<unsigned> wrap_width_ = get_opt_attr<unsigned>(sym, "wrap-width");
    if (wrap_width_) wrap_width = *wrap_width_;
    optional<boolean> wrap_before_ = get_opt_attr<boolean>(sym, "wrap-before");
    if (wrap_before_) wrap_before = *wrap_before_;
    optional<text_transform_e> tconvert_ = get_opt_attr<text_transform_e>(sym, "text-transform");
    if (tconvert_) text_transform = *tconvert_;
    optional<unsigned> line_spacing_ = get_opt_attr<unsigned>(sym, "line-spacing");
    if (line_spacing_) line_spacing = *line_spacing_;
    optional<unsigned> label_position_tolerance_ = get_opt_attr<unsigned>(sym, "label-position-tolerance");
    if (label_position_tolerance_) label_position_tolerance = *label_position_tolerance_;
    optional<unsigned> character_spacing_ = get_opt_attr<unsigned>(sym, "character-spacing");
    if (character_spacing_) character_spacing = *character_spacing_;
    optional<unsigned> spacing_ = get_opt_attr<unsigned>(sym, "spacing");
    if (spacing_) label_spacing = *spacing_;
    optional<unsigned> minimum_distance_ = get_opt_attr<unsigned>(sym, "minimum-distance");
    if (minimum_distance_) minimum_distance = *minimum_distance_;
    optional<unsigned> min_padding_ = get_opt_attr<unsigned>(sym, "minimum-padding");
    if (min_padding_) minimum_padding = *min_padding_;
    optional<boolean> avoid_edges_ = get_opt_attr<boolean>(sym, "avoid-edges");
    if (avoid_edges_) avoid_edges = *avoid_edges_;
    optional<boolean> allow_overlap_ = get_opt_attr<boolean>(sym, "allow-overlap");
    if (allow_overlap_) allow_overlap = *allow_overlap_;
    optional<double> opacity_ = get_opt_attr<double>(sym, "opacity");
    if (opacity_) text_opacity = *opacity_;
    optional<horizontal_alignment_e> halign_ = get_opt_attr<horizontal_alignment_e>(sym, "horizontal-alignment");
    if (halign_) halign = *halign_;
    optional<justify_alignment_e> jalign_ = get_opt_attr<justify_alignment_e>(sym, "justify-alignment");
    if (jalign_) jalign = *jalign_;
    /* Attributes needing special care */
    optional<std::string> orientation_ = get_opt_attr<std::string>(sym, "orientation");
    if (orientation_) orientation = parse_expression(*orientation_, "utf8");
    optional<std::string> name_ = get_opt_attr<std::string>(sym, "name");
    if (name_) name = parse_expression(*name_, "utf8");
    optional<double> dx = get_opt_attr<double>(sym, "dx");
    if (dx) displacement.get<0>() = *dx;
    optional<double> dy = get_opt_attr<double>(sym, "dy");
    if (dy) displacement.get<1>() = *dy;
    optional<double> max_char_angle_delta_ = get_opt_attr<double>(sym, "max-char-angle-delta");
    if (max_char_angle_delta_) max_char_angle_delta=(*max_char_angle_delta_)*(M_PI/180);
    optional<std::string> wrap_char_ = get_opt_attr<std::string>(sym, "wrap-character");
    if (wrap_char_ && (*wrap_char_).size() > 0) wrap_char = ((*wrap_char_)[0]);
    optional<std::string> face_name_ = get_opt_attr<std::string>(sym, "face-name");
    if (face_name_)
    {
        face_name = *face_name_;
    }
    optional<std::string> fontset_name_ = get_opt_attr<std::string>(sym, "fontset-name");
    std::map<std::string,font_set>::const_iterator itr = fontsets.find(*fontset_name_);
    if (itr != fontsets.end())
    {
        fontset = itr->second;
    } else
    {
        throw config_error("Unable to find any fontset named '" + *fontset_name_ + "'");
    }
    if (!face_name.empty() && !fontset.get_name().empty())
    {
        throw config_error(std::string("Can't have both face-name and fontset-name"));
    }
    if (face_name.empty() && fontset.get_name().empty())
    {
        throw config_error(std::string("Must have face-name or fontset-name"));
    }
}

text_placements::text_placements() : properties()
{
}


/************************************************************************/

text_placement_info::text_placement_info(text_placements const* parent):
    properties(parent->properties),
    scale_factor(1),
    has_dimensions(false),
    collect_extents(false)

{
}

bool text_placement_info_dummy::next()
{
    if (state) return false;
    state++;
    return true;
}

bool text_placement_info_dummy::next_position_only()
{
    if (position_state) return false;
    position_state++;
    return true;
}

text_placement_info_ptr text_placements_dummy::get_placement_info() const
{
    return text_placement_info_ptr(new text_placement_info_dummy(this));
}

void text_placement_info::init(string_info *info_, double scale_factor_,
                               unsigned w, unsigned h, bool has_dimensions_)
{
    info = info_;
    scale_factor = scale_factor_;
    dimensions = std::make_pair(w, h);
    has_dimensions = has_dimensions_;
}

/************************************************************************/

bool text_placement_info_simple::next()
{
    position_state = 0;
    if (state == 0) {
        properties.text_size = parent_->properties.text_size;
    } else {
        if (state > parent_->text_sizes_.size()) return false;
        properties.text_size = parent_->text_sizes_[state-1];
    }
    state++;
    return true;
}

bool text_placement_info_simple::next_position_only()
{
    const position &pdisp = parent_->properties.displacement;
    position &displacement = properties.displacement;
    if (position_state >= parent_->direction_.size()) return false;
    directions_t dir = parent_->direction_[position_state];
    switch (dir) {
    case EXACT_POSITION:
        displacement = pdisp;
        break;
    case NORTH:
        displacement = boost::make_tuple(0, -abs(pdisp.get<1>()));
        break;
    case EAST:
        displacement = boost::make_tuple(abs(pdisp.get<0>()), 0);
        break;
    case SOUTH:
        displacement = boost::make_tuple(0, abs(pdisp.get<1>()));
        break;
    case WEST:
        displacement = boost::make_tuple(-abs(pdisp.get<0>()), 0);
        break;
    case NORTHEAST:
        displacement = boost::make_tuple(
                     abs(pdisp.get<0>()),
                    -abs(pdisp.get<1>()));
    case SOUTHEAST:
        displacement = boost::make_tuple(
                     abs(pdisp.get<0>()),
                     abs(pdisp.get<1>()));
    case NORTHWEST:
        displacement = boost::make_tuple(
                    -abs(pdisp.get<0>()),
                    -abs(pdisp.get<1>()));
    case SOUTHWEST:
        displacement = boost::make_tuple(
                    -abs(pdisp.get<0>()),
                     abs(pdisp.get<1>()));
        break;
    default:
        std::cerr << "WARNING: Unknown placement\n";
    }
    position_state++;
    return true;
}


text_placement_info_ptr text_placements_simple::get_placement_info() const
{
    return text_placement_info_ptr(new text_placement_info_simple(this));
}

/** Positiion string: [POS][SIZE]
  * [POS] is any combination of
  * N, E, S, W, NE, SE, NW, SW, X (exact position) (separated by commas)
  * [SIZE] is a list of font sizes, separated by commas. The first font size
  * is always the one given in the TextSymbolizer's parameters.
  * First all directions are tried, then font size is reduced
  * and all directions are tried again. The process ends when a placement is
  * found or the last fontsize is tried without success.
  * Example: N,S,15,10,8 (tries placement above, then below and if
  *    that fails it tries the additional font sizes 15, 10 and 8.
  */
void text_placements_simple::set_positions(std::string positions)
{
    positions_ = positions;
    struct direction_name_ : qi::symbols<char, directions_t>
    {
        direction_name_()
        {
            add
                ("N" , NORTH)
                ("E" , EAST)
                ("S" , SOUTH)
                ("W" , WEST)
                ("NE", NORTHEAST)
                ("SE", SOUTHEAST)
                ("NW", NORTHWEST)
                ("SW", SOUTHWEST)
                ("X" , EXACT_POSITION)
            ;
        }

    } direction_name;

    std::string::iterator first = positions.begin(),  last = positions.end();
    qi::phrase_parse(first, last,
        (direction_name[push_back(ref(direction_), _1)] % ',') >> *(',' >> qi::int_[push_back(ref(text_sizes_), _1)]),
        space
    );
    if (first != last) {
        std::cerr << "WARNING: Could not parse text_placement_simple placement string ('" << positions << "').\n";
    }
    if (direction_.size() == 0) {
        std::cerr << "WARNING: text_placements_simple with no valid placments! ('"<< positions<<"')\n";
    }
}

text_placements_simple::text_placements_simple()
{
    set_positions("X");
}

text_placements_simple::text_placements_simple(std::string positions)
{
    set_positions(positions);
}

/***************************************************************************/

bool text_placement_info_list::next()
{
    position_state = 0;
    if (state == 0) {
        properties = parent_->properties;
    } else {
        if (state > parent_->list_.size()) return false;
        properties = parent_->list_[state-1];
    }
    state++;
    return true;
}

bool text_placement_info_list::next_position_only()
{
    return (position_state++ == 0);
}

text_properties & text_placements_list::add()
{
    text_properties &last = list_[list_.size()-1];
    text_properties &p = list_[list_.size()]; //Add new item
    p = last; //Preinitialize with old values
    return p;
}

text_properties & text_placements_list::get(unsigned i)
{
    return list_[i];
}

text_placement_info_ptr text_placements_list::get_placement_info() const
{
    return text_placement_info_ptr(new text_placement_info_list(this));
}

text_placements_list::text_placements_list() : text_placements(), list_()
{

}


} //namespace
