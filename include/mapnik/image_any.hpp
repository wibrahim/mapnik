/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2014 Artem Pavlenko
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

#ifndef MAPNIK_IMAGE_DATA_ANY_HPP
#define MAPNIK_IMAGE_DATA_ANY_HPP

#include <mapnik/image.hpp>
#include <mapnik/util/variant.hpp>

namespace mapnik {

struct image_null
{   
    using pixel_type = uint8_t;
    unsigned char const* getBytes() const { return nullptr; }
    unsigned char* getBytes() { return nullptr;}
    unsigned getSize() const { return 0; }
    unsigned getRowSize() const { return 0; }
    std::size_t width() const { return 0; }
    std::size_t height() const { return 0; }
    bool painted() const { return false; }
    bool get_premultiplied() const { return false; }
    void set_premultiplied(bool) const {}
    void set(pixel_type const&) { throw std::runtime_error("Can not set values for null image"); }
    pixel_type& operator() (std::size_t, std::size_t) 
    { 
        throw std::runtime_error("Can not set or get values for null image"); 
    }
    pixel_type const& operator() (std::size_t, std::size_t) const
    { 
        throw std::runtime_error("Can not set or get values for null image"); 
    }
};

using image_base = util::variant<image_null, 
                                      image_rgba8, 
                                      image_gray8, 
                                      image_gray16, 
                                      image_gray32f>;

// Forward declaring
struct image_any;
image_any create_image_any(int width, 
                int height, 
                image_dtype type = image_dtype_rgba8,
                bool initialize = true, 
                bool premultiplied = false, 
                bool painted = false);

namespace detail {

struct get_bytes_visitor
{
    template <typename T>
    unsigned char* operator()(T & data)
    {
        return data.getBytes();
    }
};

struct get_bytes_visitor_const
{
    template <typename T>
    unsigned char const* operator()(T const& data) const
    {
        return data.getBytes();
    }
};

struct get_width_visitor
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.width();
    }
};

struct get_height_visitor
{
    template <typename T>
    std::size_t operator()(T const& data) const
    {
        return data.height();
    }
};

struct get_premultiplied_visitor
{
    template <typename T>
    bool operator()(T const& data) const
    {
        return data.get_premultiplied();
    }
};

struct get_painted_visitor
{
    template <typename T>
    bool operator()(T const& data) const
    {
        return data.painted();
    }
};

struct get_any_size_visitor
{
    template <typename T>
    unsigned operator()(T const& data) const
    {
        return data.getSize();
    }
};

struct get_any_row_size_visitor
{
    template <typename T>
    unsigned operator()(T const& data) const
    {
        return data.getRowSize();
    }
};
} // namespace detail

struct image_any : image_base
{
    image_any() = default;

    image_any(int width,
                   int height,
                   image_dtype type = image_dtype_rgba8,
                   bool initialize = true, 
                   bool premultiplied = false, 
                   bool painted = false)
        : image_base(std::move(create_image_any(width, height, type, initialize, premultiplied, painted))) {}

    template <typename T>
    image_any(T && data) noexcept
        : image_base(std::move(data)) {}

    unsigned char const* getBytes() const
    {
        return util::apply_visitor(detail::get_bytes_visitor_const(),*this);
    }

    unsigned char* getBytes()
    {
        return util::apply_visitor(detail::get_bytes_visitor(),*this);
    }

    std::size_t width() const
    {
        return util::apply_visitor(detail::get_width_visitor(),*this);
    }

    std::size_t height() const
    {
        return util::apply_visitor(detail::get_height_visitor(),*this);
    }

    bool get_premultiplied() const
    {
        return util::apply_visitor(detail::get_premultiplied_visitor(),*this);
    }

    bool painted() const
    {
        return util::apply_visitor(detail::get_painted_visitor(),*this);
    }

    unsigned getSize() const
    {
        return util::apply_visitor(detail::get_any_size_visitor(),*this);
    }

    unsigned getRowSize() const
    {
        return util::apply_visitor(detail::get_any_row_size_visitor(),*this);
    }
};

inline image_any create_image_any(int width, 
                                int height,
                                image_dtype type, 
                                bool initialize, 
                                bool premultiplied, 
                                bool painted)
{
    switch (type)
    {
      case image_dtype_gray8:
        return image_any(std::move(image_gray8(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray16:
        return image_any(std::move(image_gray16(width, height, initialize, premultiplied, painted)));
      case image_dtype_gray32f:
        return image_any(std::move(image_gray32f(width, height, initialize, premultiplied, painted)));
      case image_dtype_null:
        return image_any(std::move(image_null()));
      case image_dtype_rgba8:
      default:
        return image_any(std::move(image_rgba8(width, height, initialize, premultiplied, painted)));
    }
}

} // end mapnik ns

#endif // MAPNIK_IMAGE_DATA_ANY_HPP