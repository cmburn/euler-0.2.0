/* SPDX-License-Identifier: ISC */

#include <utility>

#include "euler/gui/button.h"

#include "euler/gui/context.h"
#include "euler/gui/internal.h"
#include "euler/gui/row.h"
#include "euler/gui/widget.h"
#include "euler/util/error.h"
#include "euler/util/ext.h"

using euler::gui::Button;

const Button::Settings Button::DEFAULT_SETTINGS = Settings();

static mrb_value
button_symbol(mrb_state *mrb, const mrb_value self_value)
{
	const auto state = euler::util::State::get(mrb);
	const auto self = state->unwrap<Button>(self_value);
	return euler::gui::from_symbol(mrb, self->symbol());
}

RClass *
Button::init(const euler::util::Reference<euler::util::State> &state,
    RClass *mod, RClass *)
{
	auto button = state->rb().define_class_under(mod, "Button",
	    state->object_class());
	MRB_SET_INSTANCE_TT(button, MRB_TT_DATA);
	state->rb().define_method(button, "symbol", button_symbol,
	    MRB_ARGS_NONE());
	return button;
}

Button::~Button() = default;

Button::Button(const util::Reference<Row> &row, const Settings &settings)
    : Element(row)
    , _label(settings.label)
    , _image(settings.image)
    , _color(settings.color)
    , _symbol(settings.symbol)
{
}

void
Button::call(
    [[maybe_unused]] const std::function<void(util::Reference<Button> &)> &fn)
{
	auto self = util::Reference(this);
	if (is_pressed()) fn(self);
}

Button::Type
Button::type() const
{
	if (_image != nullptr) {
		if (_symbol != Symbol::None) return Type::Invalid;
		return Type::Symbol;
	}
	if (_symbol != Symbol::None) return Type::Symbol;
	return Type::Empty;
}

nk_style_button
Button::style() const
{
	if (_style.has_value()) return _style.value().to_nuklear(state());
	return context()->style.button;
}

struct IsPressedContext {
	const euler::util::Reference<euler::util::State> &state;
	nk_context *context;
	const std::optional<euler::gui::Style::Button> &style;
	const char *title;
	const int len;
	[[nodiscard]] bool is_pressed_empty() const;
	[[nodiscard]] bool is_pressed_symbol(euler::gui::Symbol euler_sym,
	    euler::gui::Alignment euler_align) const;
	[[nodiscard]] bool is_pressed_image(
	    const euler::util::Reference<euler::util::Image> &euler_image,
	    euler::gui::Alignment euler_align) const;
};

inline bool
IsPressedContext::is_pressed_empty() const
{
	if (title != nullptr) {
		assert(len >= 0);
		if (style.has_value()) {
			const auto nk = style->to_nuklear(state);
			return nk_button_text_styled(context, &nk, title, len)
			    != nk_false;
		}
		return nk_button_text(context, title, len) != nk_false;
	}
	if (style.has_value()) {
		const auto nk = style->to_nuklear(state);
		return nk_button_symbol_styled(context, &nk, NK_SYMBOL_NONE)
		    != nk_false;
	}
	return nk_button_symbol(context, NK_SYMBOL_NONE) != nk_false;
}

inline bool
IsPressedContext::is_pressed_symbol(const euler::gui::Symbol euler_sym,
    const euler::gui::Alignment euler_align) const
{
	const auto symbol = euler::gui::to_nuklear(state, euler_sym);
	if (title == nullptr) {
		if (style.has_value()) {
			const auto nk = style->to_nuklear(state);
			return nk_button_symbol_styled(context, &nk, symbol)
			    != nk_false;
		}
		return nk_button_symbol(context, symbol) != nk_false;
	}
	assert(len >= 0);
	const auto alignment = euler::gui::to_nuklear_text(state, euler_align);
	if (style.has_value()) {
		const auto nk = style->to_nuklear(state);
		return nk_button_symbol_text_styled(context, &nk, symbol, title,
		           len, alignment)
		    != nk_false;
	}
	return nk_button_symbol_text(context, symbol, title, len, alignment)
	    != nk_false;
}

inline bool
IsPressedContext::is_pressed_image(
    const euler::util::Reference<euler::util::Image> &euler_image,
    euler::gui::Alignment euler_align) const
{
	const auto image = euler::gui::to_nuklear(state, euler_image);

	if (title != nullptr) {
		assert(len >= 0);
		const auto alignment
		    = euler::gui::to_nuklear_text(state, euler_align);
		if (style.has_value()) {
			const auto nk = style->to_nuklear(state);
			return nk_button_image_text_styled(context, &nk, image,
			    title, len, alignment);
		}
		return nk_button_image_text(context, image, title, len,
		    alignment);
	}
	if (style.has_value()) {
		const auto nk = style->to_nuklear(state);
		return nk_button_image_styled(context, &nk, image) != nk_false;
	}
	return nk_button_image(context, image) != nk_false;
}

bool
Button::is_pressed() const
{
	const char *title = has_text() ? _label->c_str() : "";
	const int len = has_text() ? static_cast<int>(_label->length()) : 0;
	const auto &state = this->state();
	const auto context = IsPressedContext {
		.state = state,
		.context = this->context(),
		.style = _style,
		.title = title,
		.len = len,
	};
	switch (type()) {
	case Type::Empty: return context.is_pressed_empty();
	case Type::Symbol:
		return context.is_pressed_symbol(_symbol, _alignment);
	case Type::Image: return context.is_pressed_image(_image, _alignment);
	case Type::Invalid:
		state->rb().raise(state->rb().argument_error(),
		    "Invalid button type");
		std::unreachable();
	default: std::unreachable();
	}
}
