/* SPDX-License-Identifier: ISC */

#ifndef EULER_GUI_USER_INTERFACE_H
#define EULER_GUI_USER_INTERFACE_H

#include <functional>
#include <optional>
#include <unordered_map>
#include <variant>

#include <glm/glm.hpp>

#include "euler/util/color.h"
#include "euler/util/image.h"
#include "euler/util/object.h"

namespace euler::gui {
class UserInterface final : public util::Object {
public:
	using Vec2i16 = glm::i16vec2;
	using Vec2f = glm::f32vec2;
	struct Rectangle {
		Vec2f position;
		Vec2f size;
	};
	using Glyph = std::array<char, 4>;
	using Rune = uint32_t;

	enum class Symbol {
		None,
		X,
		Underscore,
		SolidCircle,
		OutlinedCircle,
		SolidRectangle,
		OutlinedRectangle,
		UpTriangle,
		DownTriangle,
		LeftTriangle,
		RightTriangle,
		Plus,
		Minus,
		OutlinedUpTriangle,
		OutlinedDownTriangle,
		OutlinedLeftTriangle,
		OutlinedRightTriangle,
	};

	struct Alignment {
		enum class Horizontal : uint16_t { Left, Center, Right };
		enum class Vertical : uint16_t { Top, Middle, Bottom };
		Horizontal horizontal = Horizontal::Center;
		Vertical vertical = Vertical::Middle;
		// nk_flags to_nk() const;
	};

	enum class MouseButton {
		Left,
		Middle,
		Right,
		Double,
	};

	enum class Heading {
		Up,
		Right,
		Down,
		Left,
	};

	enum class Key {
		None,
		Shift,
		Ctrl,
		Delete,
		Enter,
		Tab,
		Backspace,
		Copy,
		Cut,
		Paste,
		Up,
		Down,
		Left,
		Right,
		TextInsertMode,
		TextReplaceMode,
		TextResetMode,
		TextLineStart,
		TextLineEnd,
		TextStart,
		TextEnd,
		TextUndo,
		TextRedo,
		TextSelectAll,
		TextWordLeft,
		TextWordRight,
		ScrollStart,
		ScrollEnd,
		ScrollDown,
		ScrollUp,
	};

	using Icon = std::variant<util::Reference<util::Image>, Symbol>;

	using Visual
	    = std::variant<util::Reference<util::Image>, Symbol, util::Color>;

	struct ColorTable {
		util::Color text;
		util::Color window;
		util::Color header;
		util::Color border;
		util::Color button;
		util::Color button_hover;
		util::Color button_active;
		util::Color toggle;
		util::Color toggle_hover;
		util::Color toggle_cursor;
		util::Color select;
		util::Color select_active;
		util::Color slider;
		util::Color slider_cursor;
		util::Color slider_cursor_hover;
		util::Color slider_cursor_active;
		util::Color property;
		util::Color edit;
		util::Color edit_cursor;
		util::Color dropdown;
		util::Color chart;
		util::Color chart_color;
		util::Color chart_color_highlight;
		util::Color scrollbar;
		util::Color scrollbar_cursor;
		util::Color scrollbar_cursor_hover;
		util::Color scrollbar_cursor_active;
		util::Color tab_header;
		util::Color knob;
		util::Color knob_cursor;
		util::Color knob_cursor_hover;
		util::Color knob_cursor_active;

		static constexpr size_t ATTRIBUTE_COUNT = 32;

		void to_nuklear(const util::Reference<util::State> &state,
		    nk_color table[ATTRIBUTE_COUNT]) const;
	};

	static_assert((sizeof(ColorTable) / sizeof(util::Color))
	        == ColorTable::ATTRIBUTE_COUNT,
	    "ColorTable does not match Nuklear color table size");

	/* Each GUI instance is associated with a Style. */
	struct Style {
		struct Item {
			struct NineSlice {
				util::Reference<util::Image> image;
				uint16_t left = 0;
				uint16_t top = 0;
				uint16_t right = 0;
				uint16_t bottom = 0;
			};

			std::variant<util::Color, util::Reference<util::Image>,
			    NineSlice>
			    data;

			[[nodiscard]] nk_style_item to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Text {
			util::Color color;
			Vec2f padding = { 0.0f, 0.0f };

			[[nodiscard]] nk_style_text to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Button {
			Item normal;
			Item hover;
			Item active;
			util::Color border_color;
			float color_factor_background = 1.0f;
			util::Color text_background;
			util::Color text_normal;
			util::Color text_hover;
			util::Color text_active;
			Alignment text_alignment;
			float color_factor_text = 1.0f;
			float border = 0.0f;
			float rounding = 0.0f;
			Vec2f padding = { 0.0f, 0.0f };
			Vec2f image_padding = { 0.0f, 0.0f };
			Vec2f touch_padding = { 0.0f, 0.0f };

			[[nodiscard]] nk_style_button to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Toggle {
			Item normal;
			Item hover;
			Item active;
			util::Color border_color;
			Item cursor_normal;
			Item cursor_hover;
			util::Color text_normal;
			util::Color text_hover;
			util::Color text_active;
			util::Color text_background;
			Alignment text_alignment;
			Vec2f padding = { 0.0f, 0.0f };
			Vec2f touch_padding = { 0.0f, 0.0f };
			float spacing = 0.0f;
			float border = 0.0f;

			[[nodiscard]] nk_style_toggle to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Selectable {
			/* background (inactive) */
			Item normal;
			Item hover;
			Item pressed;

			/* background (active) */
			Item normal_active;
			Item hover_active;
			Item pressed_active;

			/* text color (inactive) */
			util::Color text_normal;
			util::Color text_hover;
			util::Color text_pressed;

			/* text color (active) */
			util::Color text_normal_active;
			util::Color text_hover_active;
			util::Color text_pressed_active;
			util::Color text_background;
			Alignment text_alignment;

			float rounding = 0.0f;
			Vec2f padding = { 0.0f, 0.0f };
			Vec2f touch_padding = { 0.0f, 0.0f };
			Vec2f image_padding = { 0.0f, 0.0f };

			[[nodiscard]] nk_style_selectable to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Slider {
			/* background */
			Item normal;
			Item hover;
			Item active;
			util::Color border_color;

			/* background bar */
			util::Color bar_normal;
			util::Color bar_active;
			util::Color bar_filled;

			/* cursor */
			Item cursor_normal;
			Item cursor_hover;
			Item cursor_active;

			/* properties */
			float border = 0.0f;
			float rounding = 0.0f;
			float bar_height = 0.0f;
			Vec2f padding = { 0.0f, 0.0f };
			Vec2f spacing = { 0.0f, 0.0f };
			Vec2f cursor_size = { 0.0f, 0.0f };

			[[nodiscard]] nk_style_slider to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Knob {
			Item normal;
			Item hover;
			Item active;
			util::Color border_color;

			util::Color knob_normal;
			util::Color knob_hover;
			util::Color knob_active;
			util::Color knob_border_color;

			util::Color cursor_normal;
			util::Color cursor_hover;
			util::Color cursor_active;

			float border = 0.0f;
			float knob_border = 0.0f;
			Vec2f padding = { 0.0f, 0.0f };
			Vec2f spacing = { 0.0f, 0.0f };

			[[nodiscard]] nk_style_knob to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct ProgressBar {
			Item normal;
			Item active;
			Item hover;
			util::Color border_color;

			Item cursor_normal;
			Item cursor_hover;
			Item cursor_active;
			util::Color cursor_border_color;

			float rounding;
			float border;
			float cursor_border;
			float cursor_rounding;
			Vec2f padding = { 0.0f, 0.0f };

			[[nodiscard]] nk_style_progress to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Scrollbar {
			Item normal;
			Item hover;
			Item active;
			util::Color border_color;
			Item cursor_normal;
			Item cursor_hover;
			Item cursor_active;
			util::Color cursor_border_color;
			float border = 0.0f;
			float rounding = 0.0f;
			float border_cursor = 0.0f;
			float rounding_cursor = 0.0f;
			Vec2f padding = { 0.0f, 0.0f };

			nk_style_scrollbar to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct TextEdit {
			Item normal;
			Item hover;
			Item active;
			util::Color border_color;

			Scrollbar scrollbar;

			util::Color cursor_normal;
			util::Color cursor_hover;
			util::Color cursor_text_normal;
			util::Color cursor_text_hover;
			util::Color text_normal;
			util::Color text_hover;
			util::Color text_active;
			util::Color selected_normal;
			util::Color selected_hover;
			util::Color selected_text_normal;
			util::Color selected_text_hover;
			float border = 0.0f;
			float rounding = 0.0f;
			float cursor_size = 1.0f;
			Vec2f scrollbar_size = { 0.0f, 0.0f };
			Vec2f padding = { 0.0f, 0.0f };
			float row_padding = 0.0f;

			nk_style_edit to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Property {
			Item normal;
			Item hover;
			Item active;
			util::Color border_color;

			util::Color label_normal;
			util::Color label_hover;
			util::Color label_active;

			Symbol sym_left;
			Symbol sym_right;

			float border = 0.0f;
			float rounding = 0.0f;
			Vec2f padding = { 0.0f, 0.0f };
			TextEdit exit;
			Button increase_button;
			Button decrease_button;

			nk_style_property to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Chart {
			Item background;
			util::Color border_color;
			util::Color selected_color;
			util::Color color;
			float border = 0.0f;
			float rounding = 0.0f;
			Vec2f padding = { 0.0f, 0.0f };

			nk_style_chart to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Tab {
			Item background;
			util::Color border_color;
			util::Color text;
			Button tab_maximize_button;
			Button tab_minimize_button;
			Button node_maximize_button;
			Button node_minimize_button;
			float border = 0.0f;
			float rounding = 0.0f;
			float indent = 0.0f;
			Vec2f padding = { 0.0f, 0.0f };
			Vec2f spacing = { 0.0f, 0.0f };

			nk_style_tab to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Dropdown {
			Item normal;
			Item hover;
			Item active;
			util::Color border_color;
			util::Color label_normal;
			util::Color label_hover;
			util::Color label_active;
			Symbol symbol_normal;
			Symbol symbol_hover;
			Symbol symbol_active;
			Button button;
			float border = 0.0f;
			float rounding = 0.0f;
			Vec2f content_padding;
			Vec2f button_padding;
			Vec2f spacing;

			nk_style_combo to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		struct Window {
			struct Header {
				Item normal;
				Item hover;
				Item active;
				Button close_button;
				Button minimize_button;
				util::Color label_normal;
				util::Color label_hover;
				util::Color label_active;
				Vec2f padding = { 0.0f, 0.0f };
				Vec2f label_padding = { 0.0f, 0.0f };
				Vec2f spacing = { 0.0f, 0.0f };
				nk_style_window_header to_nuklear(
				    const util::Reference<util::State> &state)
				    const;
				static Header from_nuklear(
				    const util::Reference<util::State> &state,
				    const nk_style_window_header &header);
			};
			Header header;
			Item fixed_background;
			util::Color background;
			util::Color border_color;
			util::Color popup_border_color;
			util::Color dropdown_border_color;
			util::Color contextual_border_color;
			util::Color menu_border_color;
			util::Color group_border_color;
			util::Color tooltip_border_color;
			Item scaler;
			float border = 0.0f;
			float dropdown_border = 0.0f;
			float contextual_border = 0.0f;
			float menu_border = 0.0f;
			float group_border = 0.0f;
			float tooltip_border = 0.0f;
			float popup_border = 0.0f;
			float rounding = 0.0f;
			Vec2f spacing = { 0.0f, 0.0f };
			Vec2f group_padding = { 0.0f, 0.0f };
			Vec2f popup_padding = { 0.0f, 0.0f };
			Vec2f dropdown_padding;
			Vec2f contextual_padding;
			Vec2f menu_padding;
			Vec2f tooltip_padding;

			nk_style_window to_nuklear(
			    const util::Reference<util::State> &state) const;
		};

		util::Reference<util::Font> font;
		Text text;
		Button button;
		Button contextual_button;
		Button menu_button;
		Toggle option;
		Toggle checkbox;
		Selectable selectable;
		Slider slider;
		Knob knob;
		ProgressBar progress_bar;
		Property property;
		TextEdit text_edit;
		Chart chart;
		Scrollbar scroll_horizontal;
		Scrollbar scroll_vertical;
		Tab tab;
		Dropdown dropdown;
		Window window;

		nk_style to_nuklear(
		    const util::Reference<util::State> &state) const;
	};

	struct Flags {
		bool border : 1 = false;
		bool moveable : 1 = false;
		bool scalable : 1 = false;
		bool closeable : 1 = false;
		bool minimizable : 1 = false;
		bool no_scrollbar : 1 = false;
		bool title : 1 = false;
		bool scroll_auto_hide : 1 = false;
		bool background : 1 = false;
		bool scale_left : 1 = false;
		bool no_input : 1 = false;
		nk_flags to_nk() const;
	};

	struct Settings {
		Rectangle rect;
		Flags flags;
	};

	struct Window {
		const char *name;
		const char *title;
		Rectangle bounds;
		Flags flags;
	};

	struct GroupSettings {
		const char *name;
		const char *title;
		Flags flags;
	};

	enum class CollapseState : bool {
		Minimized,
		Maximized,
	};

	enum class ShowState : bool {
		Hidden,
		Shown,
	};

	enum class LayoutFormat : bool {
		Static,
		Dynamic,
	};

	enum class TreeType : bool {
		Node,
		Tab,
	};

	enum class WidgetLayoutStates {
		Invalid,
		Valid,
		Rom,
		Disabled,
	};

	struct WidgetStates {
		bool modified : 1 = false;
		bool inactive : 1 = false;
		bool entered : 1 = false;
		bool hover : 1 = false;
		bool activated : 1 = false;
		bool left : 1 = false;

		bool
		is_hovered() const
		{
			return hover && modified;
		}

		bool
		is_active() const
		{
			return activated && modified;
		}
	};

	struct Button {
		/* If 'None', go with whatever is the current global option */
		enum class Behavior : uint8_t {
			None,
			Default,
			Repeater,
		};

		enum class Type : uint8_t {
			Empty,
			Symbol,
			Image,
			Invalid,
		};

		std::optional<std::string> label;
		std::optional<Icon> icon;
		util::Color color;
		Behavior behavior = Behavior::None;
		Alignment alignment = {};
	};

	struct Option {
		std::string text;
		Alignment widget_alignment;
		Alignment text_alignment;
		bool active;
	};

	struct Selectable {
		std::string text;
		Alignment alignment;
		std::optional<Icon> icon;
		bool active;
	};

	struct Slider {
		float min;
		float value;
		float max;
		float step;
	};

	struct Knob {
		float min;
		float value;
		float max;
		float step;
		Heading zero_direction;
		float dead_zone_degrees;
	};

	struct Progress {
		size_t current;
		size_t max;
		bool modifiable;
	};

	struct Property {
		std::string name;
		double min;
		double value;
		double max;
		double step;
		float inc_per_pixel;
	};

	struct Chart {
		enum class Type : bool {
			Lines,
			Column,
		};
		std::optional<util::Color> color;
		std::optional<util::Color> highlight;
		int count;
		float min;
		float max;
		Type type;
	};

	struct Popup {
		enum class Type : bool {
			Static,
			Dynamic,
		};
		std::string title;
		Flags flags;
		Rectangle bounds;
		Type type;
	};

	struct Combobox {
		std::vector<std::string> values;
		int selected;
		int item_height;
		Vec2f size;
	};

	struct AbstractCombobox {
		std::optional<std::string> selected;
		std::optional<Visual> visual;
		Vec2f size;
	};

	struct AbstractComboboxItem {
		std::string item;
		Icon icon;
		Flags alignment;
	};

	struct Contextual {
		Vec2f position;
		Flags flags;
		Rectangle trigger_bounds;
	};

	struct ContextualItem {
		std::string text;
		std::optional<Icon> icon;
		Flags flags;
	};

	struct Menu {
		std::string title;
		std::optional<Icon> icon;
		Flags flags;
	};

	/*
	 * Not yet implemented:
	 *  - nk_window_find
	 *  - nk_window_get_panel
	 *  - nk_window_get_canvas
	 *  - nk_list_view_begin
	 *  - nk_list_view_end
	 *  - nk_widget
	 *  - nk_widget_fitting
	 *
	 * There's a few functions not implemented due to being wrappers of
	 * existing functions:
	 *  - nk_window_collapse_if
	 *  - nk_window_show_if
	 *  - nk_scrolled_offset
	 */

	/* input */
	void input_begin();
	void input_motion(int x, int y);
	void input_key(Key key, bool down);
	void input_button(MouseButton button, int x, int y, bool down);
	void input_scroll(Vec2f val);
	void input_char(char c);
	void input_glyph(Glyph glyph);

	/* window */
	bool begin(const Window &window);
	void end();
	Rectangle window_bounds();
	Vec2f window_position();
	Vec2f window_size();
	float window_width();
	float window_height();
	Rectangle window_content_region();
	Vec2f window_content_region_min();
	Vec2f window_content_region_max();
	Vec2f window_content_region_size();
	Vec2i16 window_scroll();
	bool window_has_focus();
	bool window_is_hovered();
	bool window_is_collapsed(const char *name);
	bool window_is_closed(const char *name);
	bool window_is_hidden(const char *name);
	bool window_is_active(const char *name);
	bool window_is_any_hovered();
	bool item_is_any_active();
	void window_set_bounds(const char *name, Rectangle bounds);
	void window_set_position(const char *name, Vec2f position);
	void window_set_size(const char *name, Vec2f size);
	void window_set_focus(const char *name);
	void window_set_scroll(Vec2i16 offset);
	void window_close(const char *name);
	void window_collapse(const char *name, CollapseState state);
	void window_show(const char *name, ShowState state);
	void rule_horizontal(util::Color color, bool rounding);

	/* layout */
	void layout_set_min_row_height(float height);
	void layout_reset_min_row_height();
	Rectangle layout_widget_bounds();
	float layout_ratio_from_pixel(float pixel_width);
	void layout_row_dynamic(float height, int cols);
	void layout_row_static(float height, int item_width, int cols);
	// void layout_row(LayoutFormat fmt, float row_height, int cols,
	//     const Callback &fn);
	void layout_row_begin(LayoutFormat fmt, float row_height, int cols);
	void layout_row_end();
	void layout_row_push(float value);
	void layout_row_template_begin(float row_height);
	void layout_row_template_push_dynamic();
	void layout_row_template_push_variable(float min_width);
	void layout_row_template_push_static(float width);
	void layout_space_begin(LayoutFormat fmt, float height,
	    int widget_count);
	void layout_space_push(Rectangle bounds);
	void layout_space_end();
	Rectangle layout_space_bounds();
	Vec2f layout_space_to_screen(Vec2f vec);
	Vec2f layout_space_to_local(Vec2f vec);
	Rectangle layout_space_rect_to_screen(Rectangle bounds);
	Rectangle layout_space_rect_to_local(Rectangle bounds);
	void spacer();

	/* group */
	bool group_begin(const GroupSettings &group);
	void group_end();
	bool group_scrolled_begin(const GroupSettings &group, Vec2i16 &offset);
	void group_scrolled_end();
	Vec2i16 group_scroll(const char *id);
	void group_set_scroll(const char *id, Vec2i16 offset);

	/* tree */
	bool tree_push(TreeType type, const char *title, CollapseState state,
	    mrb_sym id, const util::Reference<util::Image> &image = nullptr);
	void tree_pop();

	/* widget */
	Rectangle widget_bounds();
	Vec2f widget_position();
	Vec2f widget_size();
	float widget_width();
	float widget_height();
	bool widget_is_hovered();
	bool widget_is_mouse_clicked(MouseButton button);
	bool widget_has_mouse_click_down(MouseButton button, bool down);
	void spacing(int cols);
	void widget_disable_begin();
	void widget_disable_end();

	/* text */

	void text(const char *text, Alignment flags);
	void text_colored(const char *text, Alignment flags, util::Color color);
	void text_wrap(const char *text);
	void text_wrap_colored(const char *text, util::Color color);

	/* image */
	void image(const util::Reference<util::Image> &image);
	void image_color(const util::Reference<util::Image> &image,
	    util::Color color);

	/* basic elements */
	bool button(const Button &button);
	bool checkbox(const Option &checkbox);
	bool radio(const Option &radio);
	bool selectable(const Selectable &selectable);
	float slider(const Slider &slider);
	float knob(const Knob &knob);
	size_t progress(const Progress &progress);
	util::Color color_picker(util::Color color);
	double property(const Property &property);
	void tooltip(const char *text);

	/* chart */
	bool chart_begin(const Chart &chart);
	void chart_add_slot(const Chart &chart);
	void chart_push(float value, int pos = -1);
	void chart_end();
	void plot(Chart::Type type, const std::function<float(int index)> &fn,
	    int count, int offset);

	/* popup */
	bool popup_begin(const Popup &popup);
	void popup_close();
	void popup_end();
	Vec2i16 popup_get_scroll();
	void popup_set_scroll(Vec2i16 scroll);

	/* TODO: text edit */

	/* combobox */
	int combobox(const Combobox &combo);
	bool combobox_begin(const AbstractCombobox &combo);
	bool combobox_item(const AbstractComboboxItem &item);
	void combobox_end();

	/* contextual */
	bool contextual(const Contextual &contextual);
	bool contextual_item(const ContextualItem &item);
	void contextual_end();

	/* menu */
	bool menu_begin(const Menu &menu);
	bool menu_item(const Menu &item);
	void menu_end();

	util::Reference<util::State> state() const;
	util::Reference<util::RubyState> ruby() const;
	util::Reference<Target> target() const;

	const Style &
	style() const
	{
		return _style;
	}

private:
	nk_context _context = {};
	Style _style = {};
	std::unordered_map<mrb_sym, nk_key> _key_map;
	util::WeakReference<Target> _target;
};

} /* namespace euler::gui */

#endif /* EULER_GUI_USER_INTERFACE_H */
