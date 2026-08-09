/* SPDX-License-Identifier: ISC */

#ifndef EULER_GRAPHICS_IMAGE_LOADER_H
#define EULER_GRAPHICS_IMAGE_LOADER_H

#include "euler/graphics/color.h"
#include "euler/graphics/image.h"
#include "euler/util/object.h"

namespace euler::graphics {
class ImageLoader : public util::Object {
public:
	[[nodiscard]] virtual util::Reference<Image> load_image(
	    const char *path)
	    = 0;
	[[nodiscard]] virtual util::Reference<Image>
	create_image(const char *label, int16_t w, int16_t h, Color) = 0;
	virtual void upload_image(const char *label,
	    const util::Reference<Image> &img)
	    = 0;
};
} /* namespace euler::graphics */

#endif /* EULER_GRAPHICS_IMAGE_LOADER_H */
