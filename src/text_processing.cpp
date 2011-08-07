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
#include <mapnik/text_processing.hpp>
#include <mapnik/text_placements.hpp>
#include <mapnik/color.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/filter_factory.hpp>
#include <mapnik/ptree_helpers.hpp>
#include <mapnik/expression_string.hpp>

#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>

#include <stack>

namespace mapnik {
using boost::property_tree::ptree;
using boost::optional;

class abstract_token
{
public:
    virtual ~abstract_token() {}
    virtual std::string to_xml_string() = 0;
};

class abstract_formating_token : public abstract_token
{
public:
    virtual void apply(text_properties &p, Feature const& feature) = 0;
};

class abstract_text_token : public abstract_token
{
public:
    virtual UnicodeString to_string(Feature const& feature) = 0;
};

class end_format_token : public abstract_token
{
public:
    end_format_token() {}
    std::string to_xml_string();
};

class expression_token: public abstract_text_token
{
public:
    expression_token(expression_ptr text);
    UnicodeString to_string(Feature const& feature);
    std::string to_xml_string();
    void set_expression(expression_ptr text);
    expression_ptr get_expression();
private:
    expression_ptr text_;
};

class fixed_formating_token : public abstract_formating_token
{
public:
    fixed_formating_token();
    virtual void apply(text_properties &p, Feature const& feature);
    std::string to_xml_string();
    void set_fill(optional<color> c);
private:
    boost::optional<color> fill_;
};

/************************************************************/

expression_token::expression_token(expression_ptr text):
    text_(text)
{
}

void expression_token::set_expression(expression_ptr text)
{
    text_ = text;
}

expression_ptr expression_token::get_expression()
{
    return text_;
}

UnicodeString expression_token::to_string(const Feature &feature)
{
    value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature), *text_);
    return result.to_unicode();
}

std::string expression_token::to_xml_string()
{
    return to_expression_string(*text_);
}

/************************************************************/

fixed_formating_token::fixed_formating_token():
    fill_()
{
}

void fixed_formating_token::set_fill(optional<color> c)
{
    fill_ = c;
}

void fixed_formating_token::apply(text_properties &p, const Feature &feature)
{
    if (fill_) p.fill = *fill_;
    /*expression_ptr angle_expr = p.orientation;
    if (angle_expr)
    {
        // apply rotation
        value_type result = boost::apply_visitor(evaluate<Feature,value_type>(feature),*angle_expr);
        angle = result.to_double();
    }*/
}

std::string fixed_formating_token::to_xml_string()
{
    return "<Format>";
}

/************************************************************/

std::string end_format_token::to_xml_string()
{
    return "</Format>";
}

/************************************************************/

text_processor::text_processor():
    list_()
{
}

void text_processor::from_xml(const boost::property_tree::ptree &pt)
{
    ptree::const_iterator itr = pt.begin();
    ptree::const_iterator end = pt.end();
    for (; itr != end; ++itr) {
        if (itr->first == "<xmltext>") {
            std::string data = itr->second.data();
            boost::trim(data);
            if (data.empty()) continue;
            expression_token *token = new expression_token(parse_expression(data, "utf8"));
            list_.push_back(token);
        } else if (itr->first == "Format") {
            fixed_formating_token *token = new fixed_formating_token();
            token->set_fill(get_opt_attr<color>(itr->second, "fill"));
            list_.push_back(token);
            from_xml(itr->second); /* Parse children, making a list out of a tree. */
            list_.push_back(new end_format_token());
        } else if (itr->first != "<xmlcomment>" && itr->first != "<xmlattr>" && itr->first != "Placement") {
            std::cerr << "Unknown item" << itr->first;
        }
    }
}

void text_processor::process(processed_text &output, Feature const& feature)
{
    std::list<abstract_token *>::const_iterator itr = list_.begin();
    std::list<abstract_token *>::const_iterator end = list_.end();
    std::stack<text_properties> formats;
    formats.push(defaults_);

    for (; itr != end; ++itr) {
        abstract_text_token *text = dynamic_cast<abstract_text_token *>(*itr);
        abstract_formating_token *format = dynamic_cast<abstract_formating_token *>(*itr);;
        end_format_token *end = dynamic_cast<end_format_token *>(*itr);;
        if (text) {
            UnicodeString text_str = text->to_string(feature);
            text_properties const& p = formats.top();
            /* TODO: Make a class out of text_transform which does the work! */
            if (p.text_transform == UPPERCASE)
            {
                text_str = text_str.toUpper();
            }
            else if (p.text_transform == LOWERCASE)
            {
                text_str = text_str.toLower();
            }
            else if (p.text_transform == CAPITALIZE)
            {
                text_str = text_str.toTitle(NULL);
            }
            if (text_str.length() > 0) {
                output.push_back(processed_expression(p, text_str));
            }
        } else if (format) {
            text_properties next_properties = formats.top();
            format->apply(next_properties, feature);
            formats.push(next_properties);
        } else if (end) {
            /* Always keep at least the defaults_ on stack. */
            if (formats.size() > 1) {
                formats.pop();
            } else {
                std::cerr << "Warning: Internal mapnik error. More elements popped than pushed in text_processor::process()\n";
                output.clear();
                return;
            }
        }
    }
}

void text_processor::set_defaults(const text_properties &defaults)
{
    defaults_ = defaults;
}

/************************************************************/

void processed_text::push_back(processed_expression const& exp)
{
    expr_list_.push_back(exp);
}

processed_text::expression_list::const_iterator processed_text::begin()
{
    return expr_list_.begin();
}

processed_text::expression_list::const_iterator processed_text::end()
{
    return expr_list_.end();
}

bool processed_text::find_point_placement(double x, double y)
{
    buffer_char_sizes();
    return false;
}

processed_text::processed_text(DetectorType & detector, face_manager<freetype_engine> & font_manager, box2d<double> dimensions, double scale_factor)
    : detector_(detector), dimensions_(dimensions), font_manager_(font_manager), scale_factor_(scale_factor)
{

}

void processed_text::clear()
{
    expr_list_.clear();
}


void processed_text::buffer_char_sizes()
{
    //TODO: Make sure this is only called once
    expression_list::iterator itr = expr_list_.begin();
    expression_list::iterator end = expr_list_.end();
    for (; itr != end; ++itr)
    {
        text_properties const &p = itr->p;
        face_set_ptr faces;
        if (p.fontset.size() > 0)
        {
            faces = font_manager_.get_face_set(p.fontset);
        }
        else
        {
            faces = font_manager_.get_face_set(p.face_name);
        }
        if (faces->size() <= 0)
        {
            throw config_error("Unable to find specified font face '" + p.face_name + "'");
        }
        faces->set_pixel_sizes(p.text_size * scale_factor_);
        itr->info = new string_info(itr->str);
        faces->get_string_info(*(itr->info));
    }
}



} /* namespace */
