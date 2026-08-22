/* SPDX-License-Identifier: ISC */

#ifndef EULER_GRAPHICS_RENDER_TARGET_H
#define EULER_GRAPHICS_RENDER_TARGET_H

#include <array>
#include <vector>

#include <glm/glm.hpp>

#include "euler/graphics/color.h"
#include "euler/graphics/font.h"
#include "euler/graphics/image.h"
#include "euler/util/ext.h"
#include "euler/util/object.h"
#include "euler/util/state.h"

#ifndef DEFAULT_CURVE_SEGMENTS
#define DEFAULT_CURVE_SEGMENTS 22
#endif

namespace euler::graphics {
class RenderTarget : public util::Object {
	BIND_MRUBY("Euler::Graphics::RenderTarget", RenderTarget,
	    graphics.render_target);

public:
	static constexpr int16_t DEFAULT_SEGMENTS = DEFAULT_CURVE_SEGMENTS;
	using Vec2i16 = glm::i16vec2;
	using Vec2f = glm::f32vec2;
	using Line = std::array<Vec2i16, 2>;
	using PointSet = std::vector<Vec2i16>;

	struct ScissorCommand {
		Vec2i16 position;
		Vec2i16 size;
	};

	struct LineCommand {
		Line points;
		Color color;
		uint16_t line_thickness = 1;
	};

	struct CurveCommand {
		std::array<Vec2i16, 4> points;
		Color color;
		int16_t segments = DEFAULT_SEGMENTS;
		uint16_t line_thickness = 1;
	};

	struct RectCommand {
		Vec2i16 position;
		Vec2i16 size;
		Color color;
		uint16_t rounding = 0;
		uint16_t line_thickness = 1;
		bool fill = false;
	};

	struct CircleCommand {
		Vec2i16 center;
		Vec2i16 size;
		Color color;
		uint16_t line_thickness = 1;
		bool fill = false;
	};

	struct ArcCommand {
		Vec2i16 center;
		uint16_t radius;
		Vec2f angles;
		Color color;
		uint16_t line_thickness = 1;
		bool fill = false;
	};

	struct TriangleCommand {
		std::array<Vec2i16, 3> points;
		Color color;
		uint16_t line_thickness = 1;
		bool fill = false;
	};

	struct PolygonCommand {
		PointSet points;
		Color color;
		uint16_t line_thickness = 1;
		bool fill = false;
	};

	struct TextCommand {
		util::Reference<Font> font;
		Color background;
		Color foreground;
		Vec2i16 position;
		Vec2i16 size;
		float height;
		std::string text;
	};

	struct ImageCommand {
		Vec2i16 position;
		Vec2i16 size;
		util::Reference<Image> image;
		std::optional<Color> color;
	};

	virtual void scissor(const ScissorCommand &cmd) = 0;
	virtual void line(const LineCommand &cmd) = 0;
	virtual void curve(const CurveCommand &cmd) = 0;
	virtual void rect(const RectCommand &cmd) = 0;
	virtual void circle(const CircleCommand &cmd) = 0;
	virtual void arc(const ArcCommand &cmd) = 0;
	virtual void triangle(const TriangleCommand &cmd) = 0;
	virtual void polygon(const PolygonCommand &cmd) = 0;
	virtual void text(const TextCommand &cmd) = 0;
	virtual void image(const ImageCommand &cmd) = 0;

	virtual util::Reference<Image> to_image() const = 0;
	[[nodiscard]] virtual util::Reference<util::State> state() const = 0;
};
} /* namespace euler::graphics */

#endif /* EULER_GRAPHICS_RENDER_TARGET_H */
