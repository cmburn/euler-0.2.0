/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_CONFIG_H
#define EULER_UTIL_CONFIG_H

#include <string>

#include "euler/util/logger.h"
#include "euler/util/version.h"

namespace euler::util {
struct Config {
	std::string progname = "euler";
	std::string title = "Euler Game";
	Logger::Severity severity = Logger::Severity::Debug;
	Version app_version;
#ifdef EULER_NATIVE
	enum class PresentMode {
		Immediate,
		Fifo,
		FifoRelaxed,
		Mailbox,
	};
	std::optional<uint32_t> preferred_gpu;
	PresentMode present_mode = PresentMode::Fifo;
#endif
};
} /* namespace euler::util */

#endif /* EULER_UTIL_CONFIG_H */
