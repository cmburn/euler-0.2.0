/* SPDX-License-Identifier: ISC */

#ifndef EULER_GRAPHICS_WINDOW_H
#define EULER_GRAPHICS_WINDOW_H

#include <string_view>

#include "euler/graphics/render_target.h"

namespace euler::graphics {
class Window : public RenderTarget {
public:
	virtual int16_t width() const = 0;
	virtual int16_t height() const = 0;
	virtual std::string_view title() const = 0;
};
} /* namespace euler::graphics */

#endif /* EULER_GRAPHICS_WINDOW_H */
