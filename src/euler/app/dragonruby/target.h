/* SPDX-License-Identifier: ISC */

#ifndef EULER_APP_DRAGONRUBY_RENDERER_H
#define EULER_APP_DRAGONRUBY_RENDERER_H

#include <unordered_set>

#include "euler/graphics/target.h"
#include "euler/util/image.h"
#include "euler/util/object.h"

namespace euler::app::dragonruby {
class RubyState;
class State;

class Target final : public graphics::Target {
public:
	~Target() override;
	[[nodiscard]] util::Reference<util::State> state() const override;
	void scissor(const ScissorCommand &cmd) override;
	void line(const LineCommand &cmd) override;
	void curve(const CurveCommand &cmd) override;
	void rect(const RectCommand &cmd) override;
	void circle(const CircleCommand &cmd) override;
	void arc(const ArcCommand &cmd) override;
	void triangle(const TriangleCommand &cmd) override;
	void polygon(const PolygonCommand &cmd) override;
	void text(const TextCommand &cmd) override;
	void image(const ImageCommand &cmd) override;

private:
	struct PointCommand {
		Vec2i16 position;
		int16_t radius;
		util::Color color;
	};

	void draw_line(const std::function<bool(PointCommand &)> &fn);

	void draw_uploaded(const std::string &label, Vec2i16 position);

	mrb_value args() const;
	mrb_sym color_sym(util::Color);

	std::vector<TriangleCommand> triangulate(const PolygonCommand &cmd);

	void thin_line(const LineCommand &cmd);

	// void point(int16_t x, int16_t y, int16_t radius, util::Color color);

	void filled_circle(const CircleCommand &cmd,
	    const util::Reference<util::Image> &canvas);

	void internal_point(const PointCommand &cmd);
	mrb_value grid();
	mrb_value render_target();

	static constexpr int16_t ZERO_I16 = 0;
	void update_size();
	// void set_pixel(int16_t x, int16_t y, util::Color color);
	util::Reference<util::Image> draw_circle(const CircleCommand &cmd,
	    const std::string &label);
	void upload_image(const std::string &label,
	    const util::Reference<util::Image> &image);

	void push_output(mrb_sym sym, mrb_value value);

	mrb_sym register_color(util::Color color);

	util::Reference<RubyState> ruby() const;

public:
	void render(mrb_value target,
	    std::function<void(
	        const util::Reference<graphics::UserInterface> &)>
	        ui) override;
	struct nk_image to_nk(
	    const util::Reference<util::Image> &image) override;

private:
	util::WeakReference<RubyState> _ruby;
	util::WeakReference<State> _state;
	struct {
		mrb_sym h_px = MRB_Qnil;
		mrb_sym w_px = MRB_Qnil;
		mrb_sym x = MRB_Qnil;
		mrb_sym y = MRB_Qnil;
		mrb_sym x1 = MRB_Qnil;
		mrb_sym y1 = MRB_Qnil;
		mrb_sym x2 = MRB_Qnil;
		mrb_sym y2 = MRB_Qnil;
		mrb_sym x3 = MRB_Qnil;
		mrb_sym y3 = MRB_Qnil;
		mrb_sym source_x = MRB_Qnil;
		mrb_sym source_y = MRB_Qnil;
		mrb_sym source_x2 = MRB_Qnil;
		mrb_sym source_y2 = MRB_Qnil;
		mrb_sym source_x3 = MRB_Qnil;
		mrb_sym source_y3 = MRB_Qnil;
		mrb_sym r = MRB_Qnil;
		mrb_sym g = MRB_Qnil;
		mrb_sym b = MRB_Qnil;
		mrb_sym a = MRB_Qnil;
		mrb_sym w = MRB_Qnil;
		mrb_sym h = MRB_Qnil;
		mrb_sym path = MRB_Qnil;
		mrb_sym anchor_x = MRB_Qnil;
		mrb_sym anchor_y = MRB_Qnil;
		mrb_sym outputs = MRB_Qnil;
		mrb_sym sprites = MRB_Qnil;
		mrb_sym borders = MRB_Qnil;
		mrb_sym lines = MRB_Qnil;
		mrb_sym lshift = MRB_Qnil;
		mrb_sym angle = MRB_Qnil;
		mrb_sym angle_anchor_x = MRB_Qnil;
		mrb_sym angle_anchor_y = MRB_Qnil;
		mrb_sym solid = MRB_Qnil;
		mrb_sym size_enum = MRB_Qnil;
		mrb_sym font = MRB_Qnil;
		mrb_sym text = MRB_Qnil;
	} _symbols;
	int16_t _width = 0;
	int16_t _height = 0;
	mrb_value _render_target = mrb_nil_value();
	mrb_value _self = mrb_nil_value();
	std::unordered_map<util::Color, mrb_sym> _colors;
	std::unordered_map<std::string, util::Reference<util::Image>> _uploaded;
	util::Reference<util::Image> _canvas;
};
} /* namespace euler::app::dragonruby */

#endif /* EULER_APP_DRAGONRUBY_RENDERER_H */
