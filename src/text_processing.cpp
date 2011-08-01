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
    virtual void apply(text_properties p, Feature const& feature) = 0;
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
    virtual void apply(text_properties p, Feature const& feature);
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
    return "Text: "+to_expression_string(*text_);
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

void fixed_formating_token::apply(text_properties p, const Feature &feature)
{
    if (fill_) p.fill = *fill_;
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

token_list::token_list():
    list_()
{
}

void token_list::from_xml(const boost::property_tree::ptree &pt)
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
            token->set_fill(get_opt_attr<color>(itr->second, "color"));
            list_.push_back(token);
            from_xml(itr->second); /* Parse children, making a list out of a tree. */
            list_.push_back(new end_format_token());
        } else if (itr->first != "<xmlcomment>" && itr->first != "<xmlattr>" && itr->first != "Placement") {
            std::cerr << "Unknown item" << itr->first;
        }
    }

#if 0

    std::list<abstract_token *>::const_iterator itr2 = list_.begin();
    std::list<abstract_token *>::const_iterator end2 = list_.end();

    std::cerr << "Debug output\n";
    for (; itr2 != end2; ++itr2) {
        std::cout << (*itr2)->to_xml_string() << "\n";
    }
#endif

}






} /* namespace */
