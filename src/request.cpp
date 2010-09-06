/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2006 Artem Pavlenko
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

//$Id: map.cpp 17 2005-03-08 23:58:43Z pavlenko $,
#include <mapnik/request.hpp>

#include <mapnik/style.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/filter_featureset.hpp>
#include <mapnik/hit_test_filter.hpp>
#include <mapnik/scale_denominator.hpp>

namespace mapnik
{

/** Call cache_metawriters for each symbolizer.*/
/*struct metawriter_cache_dispatch : public boost::static_visitor<>
{
    metawriter_cache_dispatch (request const &r) : m_(m) {}

    template <typename T> void operator () (T &sym) const
    {
        sym.cache_metawriters(m_);
    }

    request const &r_;
};
*/

static const char * aspect_fix_mode_strings[] = {
    "GROW_BBOX",
    "GROW_CANVAS",
    "SHRINK_BBOX",
    "SHRINK_CANVAS",
    "ADJUST_BBOX_WIDTH",
    "ADJUST_BBOX_HEIGHT",
    "ADJUST_CANVAS_WIDTH",
    "ADJUST_CANVAS_HEIGHT",
    ""
};
   
IMPLEMENT_ENUM( aspect_fix_mode_e, aspect_fix_mode_strings );

request::request()
    : width_(400),
      height_(400),
      srs_("+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs"),
      buffer_size_(0)  {}
      //aspectFixMode_(GROW_BBOX) {}
    
request::request(int width,int height, std::string const& srs)
    : width_(width),
      height_(height),
      srs_(srs),
      buffer_size_(0)  {}
      //aspectFixMode_(GROW_BBOX) {}
   
request::request(const request& rhs)
    : width_(rhs.width_),
      height_(rhs.height_),
      srs_(rhs.srs_),
      buffer_size_(rhs.buffer_size_),
      background_(rhs.background_),
      background_image_(rhs.background_image_),
      //styles_(rhs.styles_),
      //metawriters_(rhs.metawriters_),
      //layers_(rhs.layers_),
      //aspectFixMode_(rhs.aspectFixMode_),
      currentExtent_(rhs.currentExtent_) {}
    
request& request::operator=(const request& rhs)
{
    if (this==&rhs) return *this;
    width_=rhs.width_;
    height_=rhs.height_;
    srs_=rhs.srs_;
    buffer_size_ = rhs.buffer_size_;
    background_=rhs.background_;
    background_image_=rhs.background_image_;
    //styles_=rhs.styles_;
    //metawriters_ = rhs.metawriters_;
    //layers_=rhs.layers_;
    aspectFixMode_=rhs.aspectFixMode_;
    return *this;
}
   
/*
std::map<std::string,feature_type_style> const& request::styles() const
{
    return styles_;
}
   
std::map<std::string,feature_type_style> & request::styles()
{
    return styles_;
}
   
request::style_iterator request::begin_styles()
{
    return styles_.begin();
}
    
request::style_iterator request::end_styles()
{
    return styles_.end();
}
    
request::const_style_iterator  request::begin_styles() const
{
    return styles_.begin();
}
    
request::const_style_iterator  request::end_styles() const
{
    return styles_.end();
}
    
bool request::insert_style(std::string const& name,feature_type_style const& style) 
{
    return styles_.insert(make_pair(name,style)).second;
}
    
void request::remove_style(std::string const& name) 
{
    styles_.erase(name);
}

boost::optional<feature_type_style const&> request::find_style(std::string const& name) const
{
    std::map<std::string,feature_type_style>::const_iterator itr = styles_.find(name);
    if (itr != styles_.end())
        return boost::optional<feature_type_style const&>(itr->second);
    else
        return boost::optional<feature_type_style const&>() ;
}

bool request::insert_metawriter(std::string const& name, metawriter_ptr const& writer)
{
    return metawriters_.insert(make_pair(name, writer)).second;
}

void request::remove_metawriter(std::string const& name)
{
    metawriters_.erase(name);
}

metawriter_ptr request::find_metawriter(std::string const& name) const
{
    std::map<std::string, metawriter_ptr>::const_iterator itr = metawriters_.find(name);
    if (itr != metawriters_.end())
        return itr->second;
    else
        return metawriter_ptr();
}

std::map<std::string,metawriter_ptr> const& request::metawriters() const
{
    return metawriters_;
}

request::const_metawriter_iterator request::begin_metawriters() const
{
    return metawriters_.begin();
}

request::const_metawriter_iterator request::end_metawriters() const
{
    return metawriters_.end();
}

bool request::insert_fontset(std::string const& name, font_set const& fontset) 
{
    return fontsets_.insert(make_pair(name, fontset)).second;
}
         
font_set const& request::find_fontset(std::string const& name) const
{
    std::map<std::string,font_set>::const_iterator itr = fontsets_.find(name);
    if (itr!=fontsets_.end())
        return itr->second;
    static font_set default_fontset;
    return default_fontset;
}

std::map<std::string,font_set> const& request::fontsets() const
{
    return fontsets_;
}

std::map<std::string,font_set> & request::fontsets()
{
    return fontsets_;
}

size_t request::layer_count() const
{
    return layers_.size();
}
    
void request::addLayer(const layer& l)
{
    layers_.push_back(l);
}

void request::removeLayer(size_t index)
{
    layers_.erase(layers_.begin()+index);
}
    
void request::remove_all() 
{
    layers_.clear();
    styles_.clear();
    metawriters_.clear();
}
    
const layer& request::getLayer(size_t index) const
{
    return layers_[index];
}

layer& request::getLayer(size_t index)
{
    return layers_[index];
}

std::vector<layer> const& request::layers() const
{
    return layers_;
}

std::vector<layer> & request::layers()
{
    return layers_;
}

*/

unsigned request::width() const
{
    return width_;
}

unsigned request::height() const
{
    return height_;
}
    
void request::set_width(unsigned width)
{
    if (width >= MIN_MAPSIZE && width <= MAX_MAPSIZE)
    {
        width_=width;
        fixAspectRatio();
    }   
}

void request::set_height(unsigned height)
{
    if (height >= MIN_MAPSIZE && height <= MAX_MAPSIZE)
    {
        height_=height;
        fixAspectRatio();
    }
}
    
void request::resize(unsigned width,unsigned height)
{
    if (width >= MIN_MAPSIZE && width <= MAX_MAPSIZE &&
        height >= MIN_MAPSIZE && height <= MAX_MAPSIZE)
    {
        width_=width;
        height_=height;
        fixAspectRatio();
    }
}

std::string const&  request::srs() const
{
    return srs_;
}
    
void request::set_srs(std::string const& srs)
{
    srs_ = srs;
}
   
void request::set_buffer_size( int buffer_size)
{
    buffer_size_ = buffer_size;
}

int request::buffer_size() const
{
    return buffer_size_;
}
   
boost::optional<color> const& request::background() const
{
    return background_;
}
   
void request::set_background(const color& c)
{
    background_ = c;
}
   
boost::optional<std::string> const& request::background_image() const
{
    return background_image_;
}
   
void request::set_background_image(std::string const& image_filename)
{
    background_image_ = image_filename;
}

void request::zoom(double factor)
{
    coord2d center = currentExtent_.center();
    double w = factor * currentExtent_.width();
    double h = factor * currentExtent_.height();
    currentExtent_ = box2d<double>(center.x - 0.5 * w, 
                                   center.y - 0.5 * h,
                                   center.x + 0.5 * w, 
                                   center.y + 0.5 * h);
    fixAspectRatio();
}

/* 
void request::zoom_all() 
{
    try 
    {
        projection proj0(srs_);
        box2d<double> ext;
        bool first = true;
        std::vector<layer>::const_iterator itr = layers_.begin();
        std::vector<layer>::const_iterator end = layers_.end();
        while (itr != end)
        {
            std::string const& layer_srs = itr->srs();
            projection proj1(layer_srs);
            proj_transform prj_trans(proj0,proj1);
                
            box2d<double> layerExt = itr->envelope();
            double x0 = layerExt.minx();
            double y0 = layerExt.miny();
            double z0 = 0.0;
            double x1 = layerExt.maxx();
            double y1 = layerExt.maxy();
            double z1 = 0.0;
            prj_trans.backward(x0,y0,z0);
            prj_trans.backward(x1,y1,z1);
                
            box2d<double> layerExt2(x0,y0,x1,y1);
#ifdef MAPNIK_DEBUG
            std::clog << " layer1 - > " << layerExt << "\n";
            std::clog << " layer2 - > " << layerExt2 << "\n";
#endif                
            if (first)
            {
                ext = layerExt2;
                first = false;
            }
            else 
            {
                ext.expand_to_include(layerExt2);
            }
            ++itr;
        }
        zoom_to_box(ext);
    }
    catch (proj_init_error & ex)
    {
        std::clog << "proj_init_error:" << ex.what() << '\n';
    }
}
*/
void request::zoom_to_box(const box2d<double> &box)
{
    currentExtent_=box;
    fixAspectRatio();
}


void request::fixAspectRatio()
{
    double ratio1 = (double) width_ / (double) height_;
    double ratio2 = currentExtent_.width() / currentExtent_.height();
    if (ratio1 == ratio2) return;

    switch(aspectFixMode_) 
    {
    case ADJUST_BBOX_HEIGHT:
        currentExtent_.height(currentExtent_.width() / ratio1);
        break;
    case ADJUST_BBOX_WIDTH:
        currentExtent_.width(currentExtent_.height() * ratio1);
        break;
    case ADJUST_CANVAS_HEIGHT:
        height_ = int (width_ / ratio2 + 0.5); 
        break;
    case ADJUST_CANVAS_WIDTH:
        width_ = int (height_ * ratio2 + 0.5); 
        break;
    case GROW_BBOX:
        if (ratio2 > ratio1)
            currentExtent_.height(currentExtent_.width() / ratio1);
        else 
            currentExtent_.width(currentExtent_.height() * ratio1);
        break;  
    case SHRINK_BBOX:
        if (ratio2 < ratio1)
            currentExtent_.height(currentExtent_.width() / ratio1);
        else 
            currentExtent_.width(currentExtent_.height() * ratio1);
        break;  
    case GROW_CANVAS:
        if (ratio2 > ratio1)
            width_ = (int) (height_ * ratio2 + 0.5);
        else
            height_ = int (width_ / ratio2 + 0.5); 
        break;
    case SHRINK_CANVAS:
        if (ratio2 > ratio1)
            height_ = int (width_ / ratio2 + 0.5); 
        else
            width_ = (int) (height_ * ratio2 + 0.5);
        break;
    default:
        if (ratio2 > ratio1)
            currentExtent_.height(currentExtent_.width() / ratio1);
        else 
            currentExtent_.width(currentExtent_.height() * ratio1);
        break;  
    }
}


const box2d<double>& request::get_current_extent() const
{
    return currentExtent_;
}

box2d<double> request::get_buffered_extent() const
{
    double extra = 2.0 * scale() * buffer_size_;
    box2d<double> ext(currentExtent_);
    ext.width(currentExtent_.width() + extra);
    ext.height(currentExtent_.height() + extra);
    return ext;
}
   
void request::pan(int x,int y)
{
    int dx = x - int(0.5 * width_);
    int dy = int(0.5 * height_) - y;
    double s = width_/currentExtent_.width();
    double minx  = currentExtent_.minx() + dx/s;
    double maxx  = currentExtent_.maxx() + dx/s;
    double miny  = currentExtent_.miny() + dy/s;
    double maxy  = currentExtent_.maxy() + dy/s;
    currentExtent_.init(minx,miny,maxx,maxy);
}

void request::pan_and_zoom(int x,int y,double factor)
{
    pan(x,y);
    zoom(factor);
}

double request::scale() const
{
    if (width_>0)
        return currentExtent_.width()/width_;
    return currentExtent_.width();
}

double request::scale_denominator() const 
{
    projection map_proj(srs_);
    return mapnik::scale_denominator( *this, map_proj.is_geographic());    
}


CoordTransform request::view_transform() const
{
    return CoordTransform(width_,height_,currentExtent_);
}

/*    
featureset_ptr request::query_point(unsigned index, double x, double y) const
{
    if ( index< layers_.size())
    {
        mapnik::layer const& layer = layers_[index];    
        try
        {
            double z = 0;
            mapnik::projection dest(srs_);
            mapnik::projection source(layer.srs());
            proj_transform prj_trans(source,dest);
            prj_trans.backward(x,y,z);
                
            double minx = currentExtent_.minx();
            double miny = currentExtent_.miny();
            double maxx = currentExtent_.maxx();
            double maxy = currentExtent_.maxy();
                
            prj_trans.backward(minx,miny,z);
            prj_trans.backward(maxx,maxy,z);
            double tol = (maxx - minx) / width_ * 3;
            mapnik::datasource_ptr ds = layer.datasource();
            if (ds)
            {
#ifdef MAPNIK_DEBUG
                std::clog << " query at point tol = " << tol << " (" << x << "," << y << ")\n";
#endif    
                featureset_ptr fs = ds->features_at_point(mapnik::coord2d(x,y));
                if (fs) 
                    return featureset_ptr(new filter_featureset<hit_test_filter>(fs,hit_test_filter(x,y,tol)));
            }
        }
        catch (...)
        {
#ifdef MAPNIK_DEBUG
            std::clog << "exception caught in \"query_map_point\"\n";
#endif
        }
    }
    return featureset_ptr();
}
    
featureset_ptr request::query_map_point(unsigned index, double x, double y) const
{
    if ( index< layers_.size())
    {
        mapnik::layer const& layer = layers_[index];
        CoordTransform tr = view_transform();
        tr.backward(&x,&y);
            
        try
        {
            mapnik::projection dest(srs_);
            mapnik::projection source(layer.srs());
            proj_transform prj_trans(source,dest);
            double z = 0;
            prj_trans.backward(x,y,z);
                
            double minx = currentExtent_.minx();
            double miny = currentExtent_.miny();
            double maxx = currentExtent_.maxx();
            double maxy = currentExtent_.maxy();
                
            prj_trans.backward(minx,miny,z);
            prj_trans.backward(maxx,maxy,z);
            double tol = (maxx - minx) / width_ * 3;
            mapnik::datasource_ptr ds = layer.datasource();
            if (ds)
            {
#ifdef MAPNIK_DEBUG
                std::clog << " query at point tol = " << tol << " (" << x << "," << y << ")\n";
#endif
                featureset_ptr fs = ds->features_at_point(mapnik::coord2d(x,y));
                if (fs) 
                    return featureset_ptr(new filter_featureset<hit_test_filter>(fs,hit_test_filter(x,y,tol)));
            }
        }
        catch (...)
        {
#ifdef MAPNIK_DEBUG
            std::clog << "exception caught in \"query_map_point\"\n";
#endif
        }
    }
    return featureset_ptr();
}
*/
request::~request() {}

/*
void request::init_metawriters()
{
    metawriter_cache_dispatch d(*this);
    request::style_iterator styIter = begin_styles();
    request::style_iterator styEnd = end_styles();
    for (; styIter!=styEnd; ++styIter) {
        std::vector<rule_type>& rules = styIter->second.get_rules_nonconst();
        std::vector<rule_type>::iterator ruleIter = rules.begin();
        std::vector<rule_type>::iterator ruleEnd = rules.end();
        for (; ruleIter!=ruleEnd; ++ruleIter) {
            rule_type::symbolizers::iterator symIter = ruleIter->begin();
            rule_type::symbolizers::iterator symEnd = ruleIter->end();
            for (; symIter!=symEnd; ++symIter) {
                boost::apply_visitor(d, *symIter);
            }
        }
    }
}

void request::set_metawriter_property(std::string name, std::string value)
{
    metawriter_output_properties[name] = UnicodeString::fromUTF8(value);
}

std::string request::get_metawriter_property(std::string name) const
{
    std::string result;
    to_utf8(metawriter_output_properties[name], result);
    return result;
}
*/
}
