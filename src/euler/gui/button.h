/* SPDX-License-Identifier: ISC */

#ifndef EULER_GUI_BUTTON_H
#define EULER_GUI_BUTTON_H

#include <optional>

#include "euler/gui/common.h"
#include "euler/gui/element.h"
#include "euler/gui/style.h"
#include "euler/util/color.h"
#include "euler/util/image.h"

namespace euler::gui {

class Button final : public Element {
	BIND_MRUBY("Euler::GUI::Button", Button, gui.button);

public:
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

	struct Settings {
		std::optional<std::string> label = std::nullopt;
		util::Reference<util::Image> image = nullptr;
		util::Color color = util::COLOR_CLEAR;
		Symbol symbol = Symbol::None;
		std::optional<Style::Button> style = std::nullopt;
		Behavior behavior = Behavior::None;
		Alignment alignment = {};
	};

	~Button() override;

	static const Settings DEFAULT_SETTINGS;

	explicit Button(const util::Reference<Row> &,
	    const Settings & = DEFAULT_SETTINGS);

	void call(const std::function<void(util::Reference<Button> &)> &fn);

	[[nodiscard]] bool
	is_valid() const
	{
		return type() != Type::Invalid;
	}

	[[nodiscard]] util::Color
	color() const
	{
		return _color;
	}

	[[nodiscard]] const std::optional<std::string> &
	label() const
	{
		return _label;
	}

	[[nodiscard]] Symbol
	symbol() const
	{
		return _symbol;
	}

	[[nodiscard]] bool
	has_text() const
	{
		return _label.has_value();
	}

	[[nodiscard]] Type type() const;
	[[nodiscard]] bool is_pressed() const;

private:
	[[nodiscard]] nk_style_button style() const;

	std::optional<std::string> _label;
	util::Reference<util::Image> _image = nullptr;
	std::optional<Style::Button> _style;
	util::Color _color = util::COLOR_CLEAR;
	Symbol _symbol = Symbol::None;
	Behavior _behavior = Behavior::None;
	Alignment _alignment = {
		.horizontal = Alignment::Horizontal::Center,
		.vertical = Alignment::Vertical::Middle,
	};
};
} /* namespace euler::gui */

#endif /* EULER_GUI_BUTTON_H */
