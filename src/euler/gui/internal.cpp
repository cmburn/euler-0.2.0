/* SPDX-License-Identifier: ISC */

#include "euler/gui/internal.h"

nk_color
euler::gui::to_nuklear(const util::Reference<util::State> &,
    const util::Color color)
{
	return nk_color {
		.r = color.red(),
		.g = color.green(),
		.b = color.blue(),
		.a = color.alpha(),
	};
}

nk_symbol_type
euler::gui::to_nuklear(const util::Reference<util::State> &state,
    const Symbol symbol)
{
	switch (symbol) {
	case Symbol::None: return NK_SYMBOL_NONE;
	case Symbol::X: return NK_SYMBOL_X;
	case Symbol::Underscore: return NK_SYMBOL_UNDERSCORE;
	case Symbol::SolidCircle: return NK_SYMBOL_CIRCLE_SOLID;
	case Symbol::OutlinedCircle: return NK_SYMBOL_CIRCLE_OUTLINE;
	case Symbol::SolidRectangle: return NK_SYMBOL_RECT_SOLID;
	case Symbol::OutlinedRectangle: return NK_SYMBOL_RECT_OUTLINE;
	case Symbol::UpTriangle: return NK_SYMBOL_TRIANGLE_UP;
	case Symbol::DownTriangle: return NK_SYMBOL_TRIANGLE_DOWN;
	case Symbol::LeftTriangle: return NK_SYMBOL_TRIANGLE_LEFT;
	case Symbol::RightTriangle: return NK_SYMBOL_TRIANGLE_RIGHT;
	case Symbol::Plus: return NK_SYMBOL_PLUS;
	case Symbol::Minus: return NK_SYMBOL_MINUS;
	case Symbol::OutlinedUpTriangle: return NK_SYMBOL_TRIANGLE_UP_OUTLINE;
	case Symbol::OutlinedDownTriangle:
		return NK_SYMBOL_TRIANGLE_DOWN_OUTLINE;
	case Symbol::OutlinedLeftTriangle:
		return NK_SYMBOL_TRIANGLE_LEFT_OUTLINE;
	case Symbol::OutlinedRightTriangle:
		return NK_SYMBOL_TRIANGLE_RIGHT_OUTLINE;
	default:
		state->rb().raise(state->rb().argument_error(),
		    "Invalid symbol passed");
		std::unreachable();
	}
}

nk_flags
euler::gui::to_nuklear_text(const util::Reference<util::State> &,
    const Alignment align)
{
	nk_flags out = 0;
	switch (align.horizontal) {
	case Alignment::Horizontal::Left: out |= NK_TEXT_ALIGN_LEFT; break;
	case Alignment::Horizontal::Center:
		out |= NK_TEXT_ALIGN_CENTERED;
		break;
	case Alignment::Horizontal::Right: out |= NK_TEXT_ALIGN_RIGHT; break;
	}
	switch (align.vertical) {
	case Alignment::Vertical::Top: out |= NK_TEXT_ALIGN_TOP; break;
	case Alignment::Vertical::Middle: out |= NK_TEXT_ALIGN_MIDDLE; break;
	case Alignment::Vertical::Bottom: out |= NK_TEXT_ALIGN_BOTTOM; break;
	}
	return out;
}

struct nk_image
euler::gui::to_nuklear(const util::Reference<util::State> &,
    const util::Reference<util::Image> &)
{
	return {};
}

euler::util::Color
euler::gui::from_nuklear(const util::Reference<util::State> &,
    const nk_color color)
{
	return util::Color {
		color.r,
		color.g,
		color.b,
		color.a,
	};
}

euler::gui::Symbol
euler::gui::to_symbol(mrb_state *mrb, const mrb_sym sym)
{
	// switch (sym) {
	// case EULER_SYM(none): return euler::gui::Symbol::None;
	// case EULER_SYM(x): return euler::gui::Symbol::X;
	// case EULER_SYM(underscore): return euler::gui::Symbol::Underscore;
	// case EULER_SYM(solid_circle): return euler::gui::Symbol::SolidCircle;
	// case EULER_SYM(outlined_circle):
	// 	return euler::gui::Symbol::OutlinedCircle;
	// case EULER_SYM(solid_rectangle):
	// 	return euler::gui::Symbol::SolidRectangle;
	// case EULER_SYM(outlined_rectangle):
	// 	return euler::gui::Symbol::OutlinedRectangle;
	// case EULER_SYM(up_triangle): return euler::gui::Symbol::UpTriangle;
	// case EULER_SYM(down_triangle): return
	// euler::gui::Symbol::DownTriangle; case EULER_SYM(left_triangle):
	// return euler::gui::Symbol::LeftTriangle; case
	// EULER_SYM(right_triangle): return euler::gui::Symbol::RightTriangle;
	// case EULER_SYM(plus): return euler::gui::Symbol::Plus;
	// case EULER_SYM(minus): return euler::gui::Symbol::Minus;
	// case EULER_SYM(outlined_up_triangle):
	// 	return euler::gui::Symbol::OutlinedUpTriangle;
	// case EULER_SYM(outlined_down_triangle):
	// 	return euler::gui::Symbol::OutlinedDownTriangle;
	// case EULER_SYM(outlined_left_triangle):
	// 	return euler::gui::Symbol::OutlinedLeftTriangle;
	// case EULER_SYM(outlined_right_triangle):
	// 	return euler::gui::Symbol::OutlinedRightTriangle;
	// default: throw std::invalid_argument("Invalid symbol");
	// }
	if (sym == EULER_SYM(none)) return Symbol::None;
	if (sym == EULER_SYM(x)) return Symbol::X;
	if (sym == EULER_SYM(underscore)) return Symbol::Underscore;
	if (sym == EULER_SYM(solid_circle)) return Symbol::SolidCircle;
	if (sym == EULER_SYM(outlined_circle)) return Symbol::OutlinedCircle;
	if (sym == EULER_SYM(solid_rectangle)) return Symbol::SolidRectangle;
	if (sym == EULER_SYM(outlined_rectangle))
		return Symbol::OutlinedRectangle;
	if (sym == EULER_SYM(up_triangle)) return Symbol::UpTriangle;
	if (sym == EULER_SYM(down_triangle)) return Symbol::DownTriangle;
	if (sym == EULER_SYM(left_triangle)) return Symbol::LeftTriangle;
	if (sym == EULER_SYM(right_triangle)) return Symbol::RightTriangle;
	if (sym == EULER_SYM(plus)) return Symbol::Plus;
	if (sym == EULER_SYM(minus)) return Symbol::Minus;
	if (sym == EULER_SYM(outlined_up_triangle))
		return Symbol::OutlinedUpTriangle;
	if (sym == EULER_SYM(outlined_down_triangle))
		return Symbol::OutlinedDownTriangle;
	if (sym == EULER_SYM(outlined_left_triangle))
		return Symbol::OutlinedLeftTriangle;
	if (sym == EULER_SYM(outlined_right_triangle))
		return Symbol::OutlinedRightTriangle;
	throw std::invalid_argument("Invalid symbol");
}

mrb_value
euler::gui::from_symbol(mrb_state *mrb, const euler::gui::Symbol symbol)
{
	switch (symbol) {
	case euler::gui::Symbol::None: return mrb_symbol_value(EULER_SYM(none));
	case euler::gui::Symbol::X: return mrb_symbol_value(EULER_SYM(x));
	case euler::gui::Symbol::Underscore:
		return mrb_symbol_value(EULER_SYM(underscore));
	case euler::gui::Symbol::SolidCircle:
		return mrb_symbol_value(EULER_SYM(solid_circle));
	case euler::gui::Symbol::OutlinedCircle:
		return mrb_symbol_value(EULER_SYM(outlined_circle));
	case euler::gui::Symbol::SolidRectangle:
		return mrb_symbol_value(EULER_SYM(solid_rectangle));
	case euler::gui::Symbol::OutlinedRectangle:
		return mrb_symbol_value(EULER_SYM(outlined_rectangle));
	case euler::gui::Symbol::UpTriangle:
		return mrb_symbol_value(EULER_SYM(up_triangle));
	case euler::gui::Symbol::DownTriangle:
		return mrb_symbol_value(EULER_SYM(down_triangle));
	case euler::gui::Symbol::LeftTriangle:
		return mrb_symbol_value(EULER_SYM(left_triangle));
	case euler::gui::Symbol::RightTriangle:
		return mrb_symbol_value(EULER_SYM(right_triangle));
	case euler::gui::Symbol::Plus: return mrb_symbol_value(EULER_SYM(plus));
	case euler::gui::Symbol::Minus:
		return mrb_symbol_value(EULER_SYM(minus));
	case euler::gui::Symbol::OutlinedUpTriangle:
		return mrb_symbol_value(EULER_SYM(outlined_up_triangle));
	case euler::gui::Symbol::OutlinedDownTriangle:
		return mrb_symbol_value(EULER_SYM(outlined_down_triangle));
	case euler::gui::Symbol::OutlinedLeftTriangle:
		return mrb_symbol_value(EULER_SYM(outlined_left_triangle));
	case euler::gui::Symbol::OutlinedRightTriangle:
		return mrb_symbol_value(EULER_SYM(outlined_right_triangle));
	default: throw std::invalid_argument("Invalid symbol");
	}
}