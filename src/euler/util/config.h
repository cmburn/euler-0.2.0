/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_CONFIG_H
#define EULER_UTIL_CONFIG_H

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "euler/util/logger.h"
#include "euler/util/version.h"

namespace euler::util {
struct Config {
	std::optional<std::string> title = "Euler Game";
	std::optional<Version> app_version;
	std::optional<Logger::Severity> severity;
#ifdef EULER_NATIVE
	enum class PresentMode {
		Immediate,
		Fifo,
		FifoRelaxed,
		Mailbox,
	};
	std::optional<uint32_t> preferred_gpu;
	std::optional<PresentMode> present_mode;
	std::vector<std::string> shader_paths;
#endif
	static Config from_argv(int argc, const char **argv);
	static Config from_json(const nlohmann::json &json);
	nlohmann::json to_json() const;
	/* Values in `other` will be set in a copy of `this`, overriding any
	 * existing value. If it is a list/vector, the values will be prepended
	 * to the existing list/vector. */
	Config merge(const Config &other) const;
};

} /* namespace euler::util */

template <> struct nlohmann::adl_serializer<euler::util::Config> {
	static void
	to_json(json &j, const euler::util::Config &c)
	{
		j = c.to_json();
	}

	static void
	from_json(const json &j, euler::util::Config &c)
	{
		c = euler::util::Config::from_json(j);
	}
};

#endif /* EULER_UTIL_CONFIG_H */
