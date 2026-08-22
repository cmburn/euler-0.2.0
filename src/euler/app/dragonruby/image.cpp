/* SPDX-License-Identifier: ISC */

#include "euler/app/dragonruby/image.h"

#include <cstdlib>

euler::app::dragonruby::Image::~Image()
{
	if (_own_data) free(_data);
}

float
euler::app::dragonruby::Image::width() const
{
	return _width;
}

float
euler::app::dragonruby::Image::height() const
{
	return _height;
}

bool
euler::app::dragonruby::Image::is_target() const
{
	return false;
}

void
euler::app::dragonruby::Image::set_pixel(const int16_t x, const int16_t y,
    const util::Color color)
{
	const auto row = &_data[y * _width];
	row[x] = color.to_dragonruby();
}

euler::util::Color
euler::app::dragonruby::Image::pixel(const int16_t x, const int16_t y) const
{
	const auto row = &_data[y * _width];
	return util::Color::from_dragonruby(row[x]);
}

void
euler::app::dragonruby::Image::blend_pixel(const int16_t x, const int16_t y,
    const util::Color color)
{
	if (color.alpha() == 0) return;
	const uint8_t inv_a = 0xff - color.alpha();
	const auto other = pixel(x, y);
	const uint8_t r
	    = ((color.red() * color.alpha()) + (other.red() * inv_a)) >> 8;
	const uint8_t g
	    = ((color.green() * color.alpha()) + (other.green() * inv_a)) >> 8;
	const uint8_t b
	    = ((color.blue() * color.alpha()) + (other.blue() * inv_a)) >> 8;
	const uint8_t a = std::min(0xff, color.alpha() + other.alpha());
	const auto blended = util::Color(r, g, b, a);
	set_pixel(x, y, blended);
}

bool
euler::app::dragonruby::Image::is_loaded() const
{
	return _data != nullptr;
}

euler::util::Reference<euler::util::Image>
euler::app::dragonruby::Image::display(const Frame &spec) const
{
	(void)spec;
	// TODO
	return nullptr;
}

uint32_t *
euler::app::dragonruby::Image::raw_data()
{
	return _data;
}

const uint32_t *
euler::app::dragonruby::Image::raw_data() const
{
	return _data;
}

euler::util::Reference<euler::util::Image>
euler::app::dragonruby::Image::transform(const Transform &) const
{
	// TODO
	return nullptr;
}

euler::util::Reference<euler::util::Image>
euler::app::dragonruby::Image::rotate(float) const
{
	// TODO
	return nullptr;
}

euler::util::Reference<euler::util::Image>
euler::app::dragonruby::Image::stretch(float, float)
{
	// TODO
	return nullptr;
}
std::string
euler::app::dragonruby::Image::label() const
{
	return _label;
}
