/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_TIME_H
#define EULER_UTIL_TIME_H

#include <chrono>
#include <cstdint>

#ifdef EULER_NATIVE
#include <SDL3/SDL_time.h>
#endif

#include "euler/util/object.h"

namespace euler::util {
/* Time is stored as nanoseconds from the Unix epoch to match SDL */
class Time final {
public:
	typedef int64_t time_type;

private:
	explicit Time(const int64_t time)
	    : _time(time)
	{
	}

public:
	typedef std::chrono::duration<time_type, std::nano> Duration;
#ifdef EULER_NATIVE
	static Time from_sdl(SDL_Time sdl_time);
#endif
	static Time from_unix(time_type unix_time);
	static Time now();

private:
	/* Nanoseconds since unix epoch */
	time_type _time = 0;
};
} /* namespace euler::util */

#endif /* EULER_UTIL_TIME_H */
