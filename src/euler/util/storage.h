/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_STORAGE_H
#define EULER_UTIL_STORAGE_H

#include <functional>
#include <string_view>

#include "euler/util/file.h"
#include "euler/util/object.h"
#include "euler/util/types.h"

namespace euler::util {
class Storage : public util::Object {
public:
	struct PathInfo { };
	typedef std::function<Loop(std::string_view dirname,
	    std::string_view filename)>
	    EachCallback;
	[[nodiscard]] virtual bool ready() const = 0;
	[[nodiscard]] virtual Reference<File> file(std::string_view) const = 0;
	[[nodiscard]] virtual size_t space_remaining() const = 0;
};
} /* namespace euler::util */

#endif /* EULER_UTIL_STORAGE_H */
