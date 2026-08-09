/* SPDX-License-Identifier: ISC */

#include "euler/gui/user_interface.h"

using euler::gui::UserInterface;

static struct nk_image
to_nk(const euler::util::Reference<euler::util::Image> &img)
{
	/* TODO */
	return {};
}

static inline nk_keys
to_nk(const UserInterface::Key key)
{
	switch (key) {
	case UserInterface::Key::Shift: return NK_KEY_SHIFT;
	case UserInterface::Key::Ctrl: return NK_KEY_CTRL;
	case UserInterface::Key::Delete: return NK_KEY_DEL;
	case UserInterface::Key::Enter: return NK_KEY_ENTER;
	case UserInterface::Key::Tab: return NK_KEY_TAB;
	case UserInterface::Key::Backspace: return NK_KEY_BACKSPACE;
	case UserInterface::Key::Copy: return NK_KEY_COPY;
	case UserInterface::Key::Cut: return NK_KEY_CUT;
	case UserInterface::Key::Paste: return NK_KEY_PASTE;
	case UserInterface::Key::Up: return NK_KEY_UP;
	case UserInterface::Key::Down: return NK_KEY_DOWN;
	case UserInterface::Key::Left: return NK_KEY_LEFT;
	case UserInterface::Key::Right: return NK_KEY_RIGHT;
	case UserInterface::Key::TextInsertMode: return NK_KEY_TEXT_INSERT_MODE;
	case UserInterface::Key::TextReplaceMode:
		return NK_KEY_TEXT_REPLACE_MODE;
	case UserInterface::Key::TextResetMode: return NK_KEY_TEXT_RESET_MODE;
	case UserInterface::Key::TextLineStart: return NK_KEY_TEXT_LINE_START;
	case UserInterface::Key::TextLineEnd: return NK_KEY_TEXT_LINE_END;
	case UserInterface::Key::TextStart: return NK_KEY_TEXT_START;
	case UserInterface::Key::TextEnd: return NK_KEY_TEXT_END;
	case UserInterface::Key::TextUndo: return NK_KEY_TEXT_UNDO;
	case UserInterface::Key::TextRedo: return NK_KEY_TEXT_REDO;
	case UserInterface::Key::TextSelectAll: return NK_KEY_TEXT_SELECT_ALL;
	case UserInterface::Key::TextWordLeft: return NK_KEY_TEXT_WORD_LEFT;
	case UserInterface::Key::TextWordRight: return NK_KEY_TEXT_WORD_RIGHT;
	case UserInterface::Key::ScrollStart: return NK_KEY_SCROLL_START;
	case UserInterface::Key::ScrollEnd: return NK_KEY_SCROLL_END;
	case UserInterface::Key::ScrollDown: return NK_KEY_SCROLL_DOWN;
	case UserInterface::Key::ScrollUp: return NK_KEY_SCROLL_UP;
	default: std::unreachable();
	}
}

static inline nk_buttons
to_nk(UserInterface::MouseButton button)
{
	switch (button) {
	case UserInterface::MouseButton::Left: return NK_BUTTON_LEFT;
	case UserInterface::MouseButton::Middle: return NK_BUTTON_MIDDLE;
	case UserInterface::MouseButton::Right: return NK_BUTTON_RIGHT;
	case UserInterface::MouseButton::Double: return NK_BUTTON_DOUBLE;
	default: std::unreachable();
	}
}

static inline nk_symbol_type
to_nk(UserInterface::Symbol sym)
{
	switch (sym) {
	case UserInterface::Symbol::None: return NK_SYMBOL_NONE;
	case UserInterface::Symbol::X: return NK_SYMBOL_X;
	case UserInterface::Symbol::Underscore: return NK_SYMBOL_UNDERSCORE;
	case UserInterface::Symbol::SolidCircle: return NK_SYMBOL_CIRCLE_SOLID;
	case UserInterface::Symbol::OutlinedCircle:
		return NK_SYMBOL_CIRCLE_OUTLINE;
	case UserInterface::Symbol::SolidRectangle: return NK_SYMBOL_RECT_SOLID;
	case UserInterface::Symbol::OutlinedRectangle:
		return NK_SYMBOL_RECT_OUTLINE;
	case UserInterface::Symbol::UpTriangle: return NK_SYMBOL_TRIANGLE_UP;
	case UserInterface::Symbol::DownTriangle:
		return NK_SYMBOL_TRIANGLE_DOWN;
	case UserInterface::Symbol::LeftTriangle:
		return NK_SYMBOL_TRIANGLE_LEFT;
	case UserInterface::Symbol::RightTriangle:
		return NK_SYMBOL_TRIANGLE_RIGHT;
	case UserInterface::Symbol::Plus: return NK_SYMBOL_PLUS;
	case UserInterface::Symbol::Minus: return NK_SYMBOL_MINUS;
	case UserInterface::Symbol::OutlinedUpTriangle:
		return NK_SYMBOL_TRIANGLE_UP_OUTLINE;
	case UserInterface::Symbol::OutlinedDownTriangle:
		return NK_SYMBOL_TRIANGLE_DOWN_OUTLINE;
	case UserInterface::Symbol::OutlinedLeftTriangle:
		return NK_SYMBOL_TRIANGLE_LEFT_OUTLINE;
	case UserInterface::Symbol::OutlinedRightTriangle:
		return NK_SYMBOL_TRIANGLE_RIGHT_OUTLINE;
	default: std::unreachable();
	}
}

static inline nk_heading
to_nk(const UserInterface::Heading heading)
{
	switch (heading) {
	case UserInterface::Heading::Up: return NK_UP;
	case UserInterface::Heading::Right: return NK_RIGHT;
	case UserInterface::Heading::Down: return NK_DOWN;
	case UserInterface::Heading::Left: return NK_LEFT;
	default: std::unreachable();
	}
}

static inline struct nk_rect
to_nk(const UserInterface::Rectangle rect)
{
	return (struct nk_rect) {
		.x = rect.position.x,
		.y = rect.position.y,
		.w = rect.size.x,
		.h = rect.size.y,
	};
}

static inline nk_flags
to_nk(UserInterface::Flags flags)
{
}

static inline UserInterface::Rectangle
from_nk(const struct nk_rect rect)
{
	return UserInterface::Rectangle {
		.position = { rect.x, rect.y },
		.size = { rect.w, rect.h },
	};
}

void
UserInterface::input_begin()
{
	nk_input_begin(&_context);
}

void
UserInterface::input_motion(const int x, const int y)
{
	nk_input_motion(&_context, x, y);
}

void
UserInterface::input_key(const Key key, const bool down)
{
	const auto nk = to_nk(key);
	nk_input_key(&_context, nk, down);
}

void
UserInterface::input_button(MouseButton button, int x, int y, bool down)
{
	const auto nk = to_nk(button);
	nk_input_button(&_context, nk, x, y, down);
}

void
UserInterface::input_scroll(Vec2f val)
{
	nk_input_scroll(&_context, { val.x, val.y });
}

void
UserInterface::input_char(const char c)
{
	nk_input_char(&_context, c);
}

void
UserInterface::input_glyph(const Glyph glyph)
{
	char buf[glyph.size()] = { 0 };
	memcpy(buf, glyph.data(), glyph.size());
	nk_input_glyph(&_context, buf);
}

bool
UserInterface::begin(const Window &window)
{
	if (window.title != nullptr) {
		return nk_begin_titled(&_context, window.name, window.title,
		    to_nk(window.bounds), to_nk(window.flags));
	}
	return nk_begin(&_context, window.name, to_nk(window.bounds),
	    window.flags.to_nk());
}

void
UserInterface::end()
{
	nk_end(&_context);
}

UserInterface::Rectangle
UserInterface::window_bounds()
{
	const auto rect = nk_window_get_bounds(&_context);
	return from_nk(rect);
}

UserInterface::Vec2f
UserInterface::window_position()
{
	const auto vec = nk_window_get_position(&_context);
	return Vec2f { vec.x, vec.y };
}

UserInterface::Vec2f
UserInterface::window_size()
{
	const auto vec = nk_window_get_size(&_context);
	return Vec2f { vec.x, vec.y };
}

float
UserInterface::window_width()
{
	return nk_window_get_width(&_context);
}

float
UserInterface::window_height()
{
	return nk_window_get_height(&_context);
}

UserInterface::Rectangle
UserInterface::window_content_region()
{
	const auto vec = nk_window_get_content_region(&_context);
	return from_nk(vec);
}

UserInterface::Vec2f
UserInterface::window_content_region_min()
{
	const auto vec = nk_window_get_content_region_min(&_context);
	return Vec2f { vec.x, vec.y };
}

UserInterface::Vec2f
UserInterface::window_content_region_max()
{
	const auto vec = nk_window_get_content_region_max(&_context);
	return Vec2f { vec.x, vec.y };
}

UserInterface::Vec2f
UserInterface::window_content_region_size()
{
	const auto vec = nk_window_get_content_region_size(&_context);
	return Vec2f { vec.x, vec.y };
}

UserInterface::Vec2i
UserInterface::window_scroll()
{
	nk_uint x, y;
	nk_window_get_scroll(&_context, &x, &y);
	return Vec2i { x, y };
}

bool
UserInterface::window_has_focus()
{
	return nk_window_has_focus(&_context);
}

bool
UserInterface::window_is_hovered()
{
	return nk_window_is_hovered(&_context);
}

bool
UserInterface::window_is_collapsed(const char *name)
{
	return nk_window_is_collapsed(&_context, name);
}

bool
UserInterface::window_is_closed(const char *name)
{
	return nk_window_is_closed(&_context, name);
}

bool
UserInterface::window_is_hidden(const char *name)
{
	return nk_window_is_hidden(&_context, name);
}

bool
UserInterface::window_is_active(const char *name)
{
	return nk_window_is_active(&_context, name);
}

bool
UserInterface::window_is_any_hovered()
{
	return nk_window_is_any_hovered(&_context);
}

bool
UserInterface::item_is_any_active()
{
	return nk_item_is_any_active(&_context);
}

void
UserInterface::window_set_bounds(const char *name, Rectangle bounds)
{
	nk_window_set_bounds(&_context, name, bounds.to_nk());
}

void
UserInterface::window_set_position(const char *name, Vec2f position)
{
	nk_window_set_position(&_context, name, { position.x, position.y });
}

void
UserInterface::window_set_size(const char *name, Vec2f size)
{
	nk_window_set_size(&_context, name, { size.x, size.y });
}

void
UserInterface::window_set_focus(const char *name)
{
	nk_window_set_focus(&_context, name);
}
void
UserInterface::window_set_scroll(Vec2i16 offset)
{
	nk_window_set_scroll(&_context, offset.x, offset.y);
}

void
UserInterface::window_close(const char *name)
{
	nk_window_close(&_context, name);
}

void
UserInterface::window_collapse(const char *name, const CollapseState state)
{
	switch (state) {
	case CollapseState::Minimized:
		nk_window_collapse(&_context, name, NK_MINIMIZED);
		break;
	case CollapseState::Maximized:
		nk_window_collapse(&_context, name, NK_MAXIMIZED);
		break;
	}
}

void
UserInterface::window_show(const char *name, const ShowState state)
{
	switch (state) {
	case ShowState::Hidden:
		nk_window_show(&_context, name, NK_HIDDEN);
		break;
	case ShowState::Shown: nk_window_show(&_context, name, NK_SHOWN); break;
	}
}

void
UserInterface::rule_horizontal(util::Color color, bool rounding)
{
	nk_rule_horizontal(&_context, color.to_nk(), rounding);
}

void
UserInterface::layout_set_min_row_height(float height)
{
	nk_layout_set_min_row_height(&_context, height);
}

void
UserInterface::layout_reset_min_row_height()
{
	nk_layout_reset_min_row_height(&_context);
}

UserInterface::Rectangle
UserInterface::layout_widget_bounds()
{
	const auto rect = nk_layout_widget_bounds(&_context);
	return Rectangle::from_nk(rect);
}

float
UserInterface::layout_ratio_from_pixel(float pixel_width)
{
	return nk_layout_ratio_from_pixel(&_context, pixel_width);
}

void
UserInterface::layout_row_dynamic(float height, int cols)
{
	nk_layout_row_dynamic(&_context, height, cols);
}

void
UserInterface::layout_row_static(float height, int item_width, int cols)
{
	nk_layout_row_static(&_context, height, item_width, cols);
}

void
UserInterface::layout_row_begin(LayoutFormat fmt, float row_height, int cols)
{
	nk_layout_format nk_fmt;
	switch (fmt) {
	case LayoutFormat::Static: nk_fmt = NK_STATIC; break;
	case LayoutFormat::Dynamic: nk_fmt = NK_DYNAMIC; break;
	default: std::unreachable();
	}
	nk_layout_row_begin(&_context, nk_fmt, row_height, cols);
}

void
UserInterface::layout_row_end()
{
	nk_layout_row_end(&_context);
}

void
UserInterface::layout_row_push(float value)
{
	nk_layout_row_push(&_context, value);
}

void
UserInterface::layout_row_template_begin(float row_height)
{
	nk_layout_row_template_begin(&_context, row_height);
}

void
UserInterface::layout_row_template_push_dynamic()
{
	nk_layout_row_template_push_dynamic(&_context);
}

void
UserInterface::layout_row_template_push_variable(float min_width)
{
	nk_layout_row_template_push_variable(&_context, min_width);
}

void
UserInterface::layout_row_template_push_static(float width)
{
	nk_layout_row_template_push_static(&_context, width);
}

void
UserInterface::layout_space_begin(LayoutFormat fmt, float height,
    int widget_count)
{
	nk_layout_format layout;
	switch (fmt) {
	case LayoutFormat::Dynamic: layout = NK_DYNAMIC; break;
	case LayoutFormat::Static: layout = NK_STATIC; break;
	default:
		throw util::make_error<util::ArgumentError>(ruby(),
		    "Unexpected layout {}", static_cast<int>(fmt));
	}
	nk_layout_space_begin(&_context, layout, height, widget_count);
}

void
UserInterface::layout_space_push(Rectangle bounds)
{
	nk_layout_space_push(&_context, bounds.to_nk());
}

void
UserInterface::layout_space_end()
{
	nk_layout_space_end(&_context);
}

UserInterface::Rectangle
UserInterface::layout_space_bounds()
{
	const auto rect = nk_layout_space_bounds(&_context);
	return Rectangle::from_nk(rect);
}

UserInterface::Vec2f
UserInterface::layout_space_to_screen(Vec2f vec)
{
	const struct nk_vec2 arg = {
		.x = vec.x,
		.y = vec.y,
	};
	const auto [x, y] = nk_layout_space_to_screen(&_context, arg);
	return Vec2f {
		x,
		y,
	};
}

UserInterface::Vec2f
UserInterface::layout_space_to_local(Vec2f vec)
{
	const struct nk_vec2 arg = {
		.x = vec.x,
		.y = vec.y,
	};
	const auto [x, y] = nk_layout_space_to_local(&_context, arg);
	return Vec2f {
		x,
		y,
	};
}

UserInterface::Rectangle
UserInterface::layout_space_rect_to_screen(Rectangle bounds)
{
	const auto rect
	    = nk_layout_space_rect_to_screen(&_context, bounds.to_nk());
	return from_nk(rect);
}

UserInterface::Rectangle
UserInterface::layout_space_rect_to_local(Rectangle bounds)
{
	const auto rect
	    = nk_layout_space_rect_to_local(&_context, bounds.to_nk());
	return from_nk(rect);
}

void
UserInterface::spacer()
{
	nk_spacer(&_context);
}
bool
UserInterface::group_begin(const GroupSettings &group)
{
	if (group.title == nullptr) {
		return nk_group_begin(&_context, group.name,
		    group.flags.to_nk());
	}
	return nk_group_begin_titled(&_context, group.name, group.title,
	    group.flags.to_nk());
}

void
UserInterface::group_end()
{
	nk_group_end(&_context);
}

bool
UserInterface::group_scrolled_begin(const GroupSettings &group, Vec2i16 &offset)
{
	nk_scroll vec = {
		.x = static_cast<nk_uint>(offset.x),
		.y = static_cast<nk_uint>(offset.y),
	};
	const auto ans = nk_group_scrolled_begin(&_context, &vec, group.title,
	    group.flags.to_nk());
	offset.x = static_cast<int16_t>(vec.x);
	offset.y = static_cast<int16_t>(vec.y);
	return ans;
}

void
UserInterface::group_scrolled_end()
{
	nk_group_scrolled_end(&_context);
}

UserInterface::Vec2i16
UserInterface::group_scroll(const char *id)
{
	nk_scroll scroll = {};
	nk_group_get_scroll(&_context, id, &scroll.x, &scroll.y);
	return Vec2i16 {
		scroll.x,
		scroll.y,
	};
}

void
UserInterface::group_set_scroll(const char *id, Vec2i16 offset)
{
	const nk_scroll scroll = {
		.x = static_cast<nk_uint>(offset.x),
		.y = static_cast<nk_uint>(offset.y),
	};
	nk_group_set_scroll(&_context, id, scroll.x, scroll.y);
}

bool
UserInterface::tree_push(TreeType type, const char *title, CollapseState state,
    mrb_sym id, const util::Reference<util::Image> &image)
{
	nk_tree_type tree_type = {};
	switch (type) {
	case TreeType::Node: tree_type = NK_TREE_NODE; break;
	case TreeType::Tab: tree_type = NK_TREE_TAB; break;
	}
	nk_collapse_states collapse_state = {};
	switch (state) {
	case CollapseState::Minimized: collapse_state = NK_MINIMIZED; break;
	case CollapseState::Maximized: collapse_state = NK_MAXIMIZED; break;
	}
	return nk_tree_push_hashed(&_context, tree_type, title, collapse_state,
	    reinterpret_cast<const char *>(&id), sizeof(id), 0);
}

void
UserInterface::tree_pop()
{
	nk_tree_pop(&_context);
}

UserInterface::Rectangle
UserInterface::widget_bounds()
{
	const auto rect = nk_widget_bounds(&_context);
	return Rectangle::from_nk(rect);
}

UserInterface::Vec2f
UserInterface::widget_position()
{
	const auto [x, y] = nk_widget_position(&_context);
	return Vec2f {
		x,
		y,
	};
}

UserInterface::Vec2f
UserInterface::widget_size()
{
	const auto [x, y] = nk_widget_size(&_context);
	return Vec2f {
		x,
		y,
	};
}

float
UserInterface::widget_width()
{
	return nk_widget_width(&_context);
}

float
UserInterface::widget_height()
{
	return nk_widget_height(&_context);
}

bool
UserInterface::widget_is_hovered()
{
	return nk_widget_is_hovered(&_context);
}

bool
UserInterface::widget_is_mouse_clicked(MouseButton button)
{
	const auto mb = to_nk(button);
	return nk_widget_is_mouse_clicked(&_context, mb);
}

bool
UserInterface::widget_has_mouse_click_down(MouseButton button, bool down)
{
	const auto mb = to_nk(button);
	return nk_widget_has_mouse_click_down(&_context, mb, down);
}

void
UserInterface::spacing(const int cols)
{
	nk_spacing(&_context, cols);
}

void
UserInterface::widget_disable_begin()
{
	nk_widget_disable_begin(&_context);
}

void
UserInterface::widget_disable_end()
{
	nk_widget_disable_end(&_context);
}

void
UserInterface::text(const char *text, Alignment flags)
{
	nk_text(&_context, text, strlen(text), flags.to_nk());
}

void
UserInterface::text_colored(const char *text, Alignment flags,
    util::Color color)
{
	nk_text_colored(&_context, text, strlen(text), flags.to_nk(),
	    color.to_nk());
}

void
UserInterface::text_wrap(const char *text)
{
	nk_text_wrap(&_context, text, strlen(text));
}

void
UserInterface::text_wrap_colored(const char *text, util::Color color)
{
	nk_text_wrap_colored(&_context, text, strlen(text), color.to_nk());
}

void
UserInterface::image(const util::Reference<util::Image> &)
{
	/* TODO */
}

void
UserInterface::image_color(const util::Reference<util::Image> &, util::Color)
{
	/* TODO */
}

bool
UserInterface::button(const Button &button)
{

	if (button.label.has_value()) {

	} else {
	}
	/* TODO */
	return false;
}

bool
UserInterface::checkbox(const Option &checkbox)
{
	nk_bool nk_active = checkbox.active;
	nk_checkbox_text_align(&_context, checkbox.text.data(),
	    checkbox.text.length(), &nk_active,
	    checkbox.widget_alignment.to_nk(), checkbox.text_alignment.to_nk());
	return nk_active;
}

bool
UserInterface::radio(const Option &radio)
{
	nk_bool nk_active = radio.active;
	nk_radio_text_align(&_context, radio.text.data(), radio.text.size(),
	    &nk_active, radio.widget_alignment.to_nk(),
	    radio.text_alignment.to_nk());
	return nk_active;
}

bool
UserInterface::selectable(const Selectable &selectable)
{
	nk_bool nk_active = selectable.active;
	if (!selectable.icon.has_value()) {
		nk_selectable_text(&_context, selectable.text.data(),
		    selectable.text.length(), selectable.alignment.to_nk(),
		    &nk_active);
	} else if (std::holds_alternative<util::Reference<util::Image>>(
	               *selectable.icon)) {
		const auto img
		    = std::get<util::Reference<util::Image>>(*selectable.icon);
		nk_selectable_image_text(&_context, to_nk(img),
		    selectable.text.data(), selectable.text.length(),
		    selectable.alignment.to_nk(), &nk_active);
	} else if (std::holds_alternative<Symbol>(*selectable.icon)) {
		const auto sym = std::get<Symbol>(*selectable.icon);
		nk_selectable_symbol_text(&_context, to_nk(sym),
		    selectable.text.data(), selectable.text.length(),
		    selectable.alignment.to_nk(), &nk_active);
	}
	return nk_active;
}

float
UserInterface::slider(const Slider &slider)
{
	float value = slider.value;
	nk_slider_float(&_context, slider.min, &value, slider.max, slider.step);
	return value;
}

float
UserInterface::knob(const Knob &knob)
{
	float value = knob.value;
	const auto dir = to_nk(knob.zero_direction);
	nk_knob_float(&_context, knob.min, &value, knob.max, knob.step, dir,
	    knob.dead_zone_degrees);
	return value;
}

size_t
UserInterface::progress(const Progress &progress)
{
	size_t out;
	nk_progress(&_context, &out, progress.max, progress.modifiable);
	return out;
}

euler::util::Color
UserInterface::color_picker(util::Color color)
{
	nk_colorf c = {
		.r = static_cast<float>(color.red()),
		.g = static_cast<float>(color.green()),
		.b = static_cast<float>(color.blue()),
		.a = static_cast<float>(color.alpha()),
	};
	nk_color_pick(&_context, &c, NK_RGBA);
	return util::Color(c.r, c.g, c.b, c.a);
}

double
UserInterface::property(const Property &property)
{
	double value = property.value;
	nk_property_double(&_context, property.name.c_str(), property.min,
	    &value, property.max, property.step, property.inc_per_pixel);
	return value;
}

void
UserInterface::tooltip(const char *text)
{
	nk_tooltip(&_context, text);
}

static nk_chart_type
chart_type(UserInterface::Chart::Type in)
{
	switch (in) {
	case UserInterface::Chart::Type::Lines: return NK_CHART_LINES;
	case UserInterface::Chart::Type::Column: return NK_CHART_COLUMN;
	default: std::unreachable();
	}
}

bool
UserInterface::chart_begin(const Chart &chart)
{
	const auto color = chart.color.value_or(style().chart.color).to_nk();
	const auto highlight
	    = chart.highlight.value_or(style().chart.selected_color).to_nk();
	const auto type = chart_type(chart.type);
	return nk_chart_begin_colored(&_context, type, color, highlight,
	    chart.count, chart.min, chart.max);
}

void
UserInterface::chart_add_slot(const Chart &chart)
{
	const auto type = chart_type(chart.type);
	nk_chart_add_slot(&_context, type, chart.count, chart.min, chart.max);
}

void
UserInterface::chart_push(float value, int pos)
{
	if (pos == -1) nk_chart_push(&_context, value);
	else nk_chart_push_slot(&_context, value, pos);
}

void
UserInterface::chart_end()
{
	nk_chart_end(&_context);
}

void
UserInterface::plot(Chart::Type type, const std::function<float(int index)> &fn,
    int count, int offset)
{
	using Fn = std::function<float(int index)>;
	static constexpr auto wrapper = [](void *user, int index) -> float {
		auto ptr = static_cast<Fn *>(user);
		return (*ptr)(index);
	};
	auto guard = [&](int index) -> float {
		try {
			return fn(index);
		} catch (const std::exception &) {
			nk_chart_end(&_context);
			throw;
		}
	};
	const auto nk_type = chart_type(type);
	nk_plot_function(&_context, nk_type, &guard, wrapper, count, offset);
}

bool
UserInterface::popup_begin(const Popup &popup)
{
	nk_popup_type type = {};
	switch (popup.type) {
	case Popup::Type::Static: type = NK_POPUP_STATIC; break;
	case Popup::Type::Dynamic: type = NK_POPUP_DYNAMIC; break;
	default: std::unreachable();
	}
	return nk_popup_begin(&_context, type, popup.title.c_str(),
	    popup.flags.to_nk(), popup.bounds.to_nk());
}

void
UserInterface::popup_close()
{
	nk_popup_close(&_context);
}

void
UserInterface::popup_end()
{
	nk_popup_end(&_context);
}

UserInterface::Vec2i
UserInterface::popup_get_scroll()
{
	nk_uint x = 0, y = 0;
	nk_popup_get_scroll(&_context, &x, &y);
	return Vec2i(x, y);
}

void
UserInterface::popup_set_scroll(Vec2i scroll)
{
	nk_popup_set_scroll(&_context, scroll.x, scroll.y);
}

int
UserInterface::combobox(const Combobox &combo)
{
	size_t str_size = 0;
	for (const auto &str : combo.values) str_size += str.length() + 1;
	std::vector buf(str_size, '\0');
	char *ptr = &buf[0];
	for (const auto &str : combo.values) {
		memcpy(ptr, str.c_str(), str.length() + 1);
		ptr += str.length() + 1;
	}
	return nk_combo_string(&_context, buf.data(), combo.selected,
	    combo.values.size(), combo.item_height,
	    (struct nk_vec2) {
	        .x = combo.size.x,
	        .y = combo.size.y,
	    });
}

bool
UserInterface::combobox_begin(const AbstractCombobox &combo)
{
	struct nk_vec2 size = {
		.x = combo.size.x,
		.y = combo.size.y,
	};
	if (!combo.visual.has_value()) {
		if (!combo.selected.has_value()) {
			throw util::make_error<util::ArgumentError>(ruby(),
			    "Combobox must be supplied with either text or a "
			    "visual element");
		}
		return nk_combo_begin_text(&_context, combo.selected->c_str(),
		    combo.selected->length(), size);
	}
	const auto &visual = combo.visual.value();
	if (std::holds_alternative<util::Reference<util::Image>>(visual)) {
		const auto img = std::get<util::Reference<util::Image>>(visual);
		const auto nk_img = target()->to_nk(img);
		if (combo.selected.has_value()) {
			const auto &text = combo.selected.value();
			return nk_combo_begin_image_text(&_context,
			    text.c_str(), text.length(), nk_img, size);
		}
		return nk_combo_begin_image(&_context, nk_img, size);
	}
	if (std::holds_alternative<Symbol>(visual)) {
		const auto sym = std::get<Symbol>(visual);
		const auto nk_sym = to_nk(sym);
		if (combo.selected.has_value()) {
			const auto &text = combo.selected.value();
			return nk_combo_begin_symbol_text(&_context,
			    text.c_str(), text.length(), nk_sym, size);
		}
		return nk_combo_begin_symbol(&_context, nk_sym, size);
	}
	assert(std::holds_alternative<util::Color>(visual));
	if (combo.selected.has_value()) {
		throw util::make_error<util::ArgumentError>(ruby(),
		    "Color entries in a combo box may not have a "
		    "label");
	}
	const auto col = std::get<util::Color>(visual);
	return nk_combo_begin_color(&_context, col.to_nk(), size);
}

bool
UserInterface::combobox_item(const AbstractComboboxItem &item)
{
}

void
UserInterface::combobox_end()
{
}

bool
UserInterface::contextual(const Contextual &contextual)
{
}

bool
UserInterface::contextual_item(const ContextualItem &item)
{
}

void
UserInterface::contextual_end()
{
}

bool
UserInterface::menu_begin(const Menu &menu)
{
}

bool
UserInterface::menu_item(const Menu &item)
{
}

void
UserInterface::menu_end()
{
}
