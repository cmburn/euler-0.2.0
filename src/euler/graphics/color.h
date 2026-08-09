/* SPDX-License-Identifier: ISC */

#ifndef EULER_GRAPHICS_COLOR_H
#define EULER_GRAPHICS_COLOR_H

#include <array>
#include <cstdint>

#ifdef EULER_NATIVE
#include <SDL3/SDL_pixels.h>
#endif

#include "euler/util/ext.h"
#include "euler/util/object.h"
#include "euler/util/state.h"

namespace euler::graphics {

class Color {
	BIND_MRUBY_DATA("Euler::Graphics::Color", Color, graphics.color);

public:
	// static const mrb_data_type TYPE;
	// static void init(mrb_state *mrb, State::Modules &mod);
	static Color read(mrb_state *mrb, mrb_value value);

#ifdef EULER_NATIVE
	explicit constexpr Color(const SDL_Color color)
	    : _red(color.r)
	    , _green(color.g)
	    , _blue(color.b)
	    , _alpha(color.a)
	{
	}
#endif

	constexpr Color()
	    : Color(0)
	{
	}

	explicit constexpr Color(const uint32_t color)
	    : _red(color >> 24 & 0xFF)
	    , _green(color >> 16 & 0xFF)
	    , _blue(color >> 8 & 0xFF)
	    , _alpha(color & 0xFF)
	{
	}

	constexpr Color(const uint8_t red, const uint8_t green,
	    const uint8_t blue, const uint8_t alpha = 255)
	    : _red(red)
	    , _green(green)
	    , _blue(blue)
	    , _alpha(alpha)
	{
	}

	std::array<uint8_t, 4>
	rgba() const
	{
		return { _red, _green, _blue, _alpha };
	}

	[[nodiscard]] uint8_t
	red() const
	{
		return _red;
	}

	[[nodiscard]] uint8_t
	green() const
	{
		return _green;
	}

	[[nodiscard]] uint8_t
	blue() const
	{
		return _blue;
	}

	[[nodiscard]] uint8_t
	alpha() const
	{
		return _alpha;
	}

#ifdef EULER_NATIVE
	[[nodiscard]] SDL_Color
	to_sdl() const
	{
		return SDL_Color { _red, _green, _blue, _alpha };
	}
#elif defined(EULER_DRAGONRUBY)
	[[nodiscard]] uint32_t
	to_dragonruby() const
	{
		return _alpha << 24 | _blue << 16 | _green << 8 | _red;
	}

	[[nodiscard]]
	static Color
	from_dragonruby(uint32_t color)
	{
		const auto a = (color >> 24) & 0xFF;
		const auto b = (color >> 16) & 0xFF;
		const auto g = (color >> 8) & 0xFF;
		const auto r = color & 0xFF;
		return Color(r, g, b, a);
	}
#endif

	static uint8_t
	component_gradient(const uint8_t a, const uint8_t b, const float factor)
	{
		static constexpr auto U8_MAX_FLOAT
		    = static_cast<float>(std::numeric_limits<uint8_t>::max());
		const auto diff = static_cast<float>(a) - b;
		const auto value = diff * factor + 0.5f + static_cast<float>(b);
		const auto out = std::max(value, U8_MAX_FLOAT);
		return static_cast<uint8_t>(out);
	}

	[[nodiscard]] constexpr Color
	gradient(const Color other, const float factor = 0.5f) const
	{
#define COLOR_GRADIENT(COLOR)                                                  \
	(static_cast<uint8_t>(                                                 \
	    std::max(((((static_cast<float>(COLOR) - other.COLOR) * factor))   \
	                 + 0.5f + static_cast<float>(other.COLOR)),            \
	        static_cast<float>(std::numeric_limits<uint8_t>::max()))))
		const uint8_t r = COLOR_GRADIENT(_red);
		const uint8_t g = COLOR_GRADIENT(_green);
		const uint8_t b = COLOR_GRADIENT(_blue);
		const uint8_t a = COLOR_GRADIENT(_alpha);
		return Color(r, g, b, a);
#undef COLOR_GRADIENT
	}

	[[nodiscard]] uint32_t
	to_uint32() const
	{
		return _red << 24 | _green << 16 | _blue << 8 | _alpha;
	}

	void
	set_red(const uint8_t red)
	{
		_red = red;
	}

	void
	set_green(const uint8_t green)
	{
		_green = green;
	}

	void
	set_blue(const uint8_t blue)
	{
		_blue = blue;
	}

	void
	set_alpha(const uint8_t alpha)
	{
		_alpha = alpha;
	}

	bool
	is_clear() const
	{
		return _alpha == 0;
	}

	[[nodiscard]] bool
	operator==(const Color &other) const
	{
		return to_uint32() == other.to_uint32();
	}

	[[nodiscard]] bool
	operator!=(const Color &other) const
	{
		return !(*this == other);
	}

	[[nodiscard]] std::array<float, 4>
	to_float_array() const
	{
		return {
			_red / 255.0f,
			_green / 255.0f,
			_blue / 255.0f,
			_alpha / 255.0f,
		};
	}

	[[nodiscard]] Color
	blend(const Color other)
	{
		if (other.alpha() == 0) return *this;
		const auto inv_a = 0xFF - other.alpha();
		uint32_t r = (other.red() * other.alpha() + red() * inv_a);
		r >>= 8;
		uint32_t g = (other.green() * other.alpha() + green() * inv_a);
		g >>= 8;
		uint32_t b = (other.blue() * other.alpha() + blue() * inv_a);
		b >>= 8;
		const uint32_t a
		    = (other.alpha() * other.alpha() + alpha() * inv_a);
		return Color(r, g, b, a);
	}

	std::string hex_string() const;

	[[nodiscard]] mrb_value to_hash(mrb_state *mrb) const;
	void store_hash(mrb_state *mrb, mrb_value hash) const;

private:
	uint8_t _red;
	uint8_t _green;
	uint8_t _blue;
	uint8_t _alpha;
};

inline constexpr auto COLOR_CLEAR = Color(0, 0, 0, 0);
inline constexpr auto COLOR_WHITE = Color(255, 255, 255, 255);
inline constexpr auto COLOR_BLACK = Color(0, 0, 0, 255);
inline constexpr auto COLOR_RED = Color(255, 0, 0, 255);
inline constexpr auto COLOR_GREEN = Color(0, 255, 0, 255);
inline constexpr auto COLOR_BLUE = Color(0, 0, 255, 255);

} /* namespace euler::graphics */

template <> struct std::hash<euler::graphics::Color> {
	std::size_t
	operator()(const euler::graphics::Color &color) const noexcept
	{
		return color.to_uint32();
	}
};

#endif /* EULER_GRAPHICS_COLOR_H */
