/* SPDX-License-Identifier: ISC */

#include <chrono>

#include "euler/util/time.h"

using euler::util::Time;

Time
Time::from_sdl(const time_type sdl_time)
{
	return Time(sdl_time);
}

Time
Time::from_unix(const time_type unix_time)
{
	const auto s = std::chrono::seconds(unix_time);
	const auto t = Duration(s).count();
	return Time(t);
}

Time
Time::now()
{
	const auto now = std::chrono::high_resolution_clock::now();
	const auto t = Duration(now.time_since_epoch()).count();
	return Time(t);
}
