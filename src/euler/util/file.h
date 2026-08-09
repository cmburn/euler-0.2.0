/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_FILE_H
#define EULER_UTIL_FILE_H

#include <cstddef>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "euler/util/object.h"
#include "euler/util/time.h"

namespace euler::util {
class File : public Object {
public:
	enum class Mode {
		Read,
		Write,
		ReadWrite,
	};
	enum class PathType {
		None,
		File,
		Directory,
		Other,
	};
	struct Info {
		PathType type = PathType::None;
		uint64_t size = 0;
		Time create_time;
		Time modify_time;
		Time access_time;

		[[nodiscard]] bool
		exists() const
		{
			return type != PathType::None;
		}
	};
	[[nodiscard]] virtual Info info() const = 0;
	[[nodiscard]] virtual std::vector<std::byte> read() const = 0;
	[[nodiscard]] virtual bool exists() const = 0;
	virtual void read(std::vector<std::byte> &out) const = 0;
	virtual void write(std::span<const std::byte> contents) const = 0;
	virtual void remove() const = 0;
	virtual void rename(std::string_view to) const = 0;
	virtual void copy(std::string_view to) const = 0;
};
} /* namespace euler::util */

#endif /* EULER_UTIL_FILE_H */
