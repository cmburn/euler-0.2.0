/* SPDX-License-Identifier: ISC */

#include "euler/gui/row.h"

#include "euler/gui/context.h"
#include "euler/gui/internal.h"
#include "euler/gui/widget.h"
#include "euler/util/ext.h"

using euler::gui::Row;

const Row::Settings Row::DEFAULT_SETTINGS = Settings();

Row::~Row() = default;

void
Row::button(const Button::Settings &settings,
    const std::function<void(util::Reference<Button> &)> &on_click)
{
	auto button
	    = util::make_reference<Button>(util::Reference(this), settings);
	button->call(on_click);
}

euler::util::Reference<euler::gui::Button>
Row::button(const Button::Settings &settings)
{
	return util::make_reference<Button>(util::Reference(this), settings);
}

static euler::gui::Button::Settings
read_button_args(mrb_state *mrb, mrb_value *block)
{
	static const std::array KW_NAMES = {
		EULER_SYM(label),
		EULER_SYM(color),
		EULER_SYM(symbol),
	};
	mrb_value kw_values[KW_NAMES.size()];
	mrb_kwargs kwargs = {
		.num = 3,
		.required = 0,
		.table = KW_NAMES.data(),
		.values = kw_values,
		.rest = nullptr,
	};
	const auto state = euler::util::State::get(mrb);
	state->rb().get_args(":&", &kwargs, block);
	euler::gui::Button::Settings settings = {};
	if (!mrb_undef_p(kw_values[0]))
		settings.label = state->rb().string_cstr(kw_values[0]);
	if (!mrb_undef_p(kw_values[1]))
		settings.color = euler::util::Color::read(mrb, kw_values[1]);
	if (!mrb_undef_p(kw_values[2])) {
		settings.symbol
		    = euler::gui::to_symbol(mrb, mrb_symbol(kw_values[2]));
	}
	return settings;
}

static mrb_value
row_button(mrb_state *mrb, const mrb_value self_value)
{
	using namespace euler;
	auto state = util::State::get(mrb);
	auto self = state->unwrap<Row>(self_value);
	if (!state->rb().block_given_p()) {
		mrb_raise(mrb, mrb->eStandardError_class,
		    "Block required for Row#button");
	}
	mrb_value block = mrb_nil_value();
	auto settings = read_button_args(mrb, &block);
	mrb_value out = mrb_nil_value();
	self->button(settings, [&](util::Reference<gui::Button> &b) {
		const auto value = state->wrap(b);
		out = state->rb().yield(block, value);
	});
	return out;
}

RClass *
Row::init(const euler::util::Reference<euler::util::State> &state, RClass *mod,
    RClass *)
{
	const auto klass = state->rb().define_class_under(mod, "Row",
	    state->object_class());
	MRB_SET_INSTANCE_TT(klass, MRB_TT_DATA);
	state->rb().define_method(klass, "button", row_button,
	    MRB_ARGS_KEY(0, 3) | MRB_ARGS_BLOCK());
	return klass;
}

Row::Row(const util::Reference<Widget> &window, const Settings &settings)
    : _widget(window)
    , _height(settings.height)
    , _cols(settings.columns)
    , _layout(settings.layout)
{
}

void
Row::call(const std::function<void(util::Reference<Row> &)> &fn)
{
	auto self = util::Reference(this);
	const auto ctx = _widget->gui()->context();
	switch (_layout) {
	case Layout::Dynamic: nk_layout_row_dynamic(ctx, _height, _cols); break;
	case Layout::Custom:
	case Layout::Static:
		throw std::runtime_error("Row layout not yet implemented");
	}
	fn(self);
}

euler::util::Reference<euler::gui::Widget>
Row::widget() const
{
	return _widget;
}
void
Row::end_row()
{
	nk_layout_row_end(_widget->gui()->context());
}
