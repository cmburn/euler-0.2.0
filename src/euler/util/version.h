/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_VERSION_H
#define EULER_UTIL_VERSION_H

#include <cassert>
#include <cstdint>
#include <format>

#include "euler/util/ext.h"
#include "euler/util/state.h"

namespace euler::util {

class Version {
	BIND_MRUBY_DATA("Euler::Util::Version", Version, util.version);

public:
	/* major and minor must be < 1024, patch must be < 4096 */
	static constexpr uint16_t MAJOR_MAX = (1 << 10) - 1;
	static constexpr uint16_t MINOR_MAX = (1 << 10) - 1;
	static constexpr uint16_t PATCH_MAX = (1 << 12) - 1;

	explicit constexpr Version(const uint16_t major = 0,
	    const uint16_t minor = 0, const uint16_t patch = 0)
	    : _major(major)
	    , _minor(minor)
	    , _patch(patch)
	{
		assert(major <= MAJOR_MAX);
		assert(minor <= MINOR_MAX);
		assert(patch <= PATCH_MAX);
	}

	[[nodiscard]] uint32_t to_int() const;

	[[nodiscard]] explicit constexpr
	operator uint32_t() const
	{
		return to_int();
	}

	constexpr uint32_t
	major() const
	{
		return _major;
	}

	constexpr uint32_t
	minor() const
	{
		return _minor;
	}
	constexpr uint32_t
	patch() const
	{
		return _patch;
	}

	void
	set_major(const uint32_t major)
	{
		assert(major < (1 << 10));
		_major = major;
	}

	void
	set_minor(const uint32_t minor)
	{
		assert(minor < (1 << 10));
		_minor = minor;
	}

	void
	set_patch(const uint32_t patch)
	{
		assert(patch < (1 << 12));
		_patch = patch;
	}

	[[nodiscard]] std::string
	to_string() const
	{
		return std::format("v{}.{}.{}", _major, _minor, _patch);
	}

#ifdef EULER_NATIVE
	uint32_t to_vulkan() const;
#endif

	static Version parse(std::string_view str);

private:
	uint16_t _major : 10;
	uint16_t _minor : 10;
	uint16_t _patch : 12;
};

Version engine_version();

}

#endif /* EULER_UTIL_VERSION_H */
