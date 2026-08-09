/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_LOGGER_H
#define EULER_UTIL_LOGGER_H

#include <format>
#include <optional>
#include <string_view>

#include <mruby.h>
#include <mruby/data.h>

#include "euler/util/ext.h"
#include "euler/util/object.h"

namespace euler::util {
class Logger : public Object {
	BIND_MRUBY("Euler::Util::Logger", Logger, util.logger);

public:
	enum class Severity {
		Debug,
		Info,
		Warn,
		Error,
		Fatal,
		Unknown,
	};

	static Severity parse_severity(std::string_view str);

	[[nodiscard]] virtual std::string subsystem() const = 0;
	virtual void set_subsystem(std::string_view name) = 0;
	[[nodiscard]] virtual Severity severity() const = 0;
	virtual void set_severity(Severity level) = 0;
	[[nodiscard]] virtual std::string progname() const = 0;
	virtual void set_progname(std::string_view name) = 0;

	[[nodiscard]] virtual Reference<Logger> copy(
	    std::optional<std::string_view> subsystem = std::nullopt) const
	    = 0;

	template <typename... Args>
	void
	log(Severity level, const std::format_string<Args...> &message,
	    Args &&...args) const
	{
		if (level < severity()) return;
		auto str = std::format(message, std::forward<Args>(args)...);
		write_log(level, str);
	}

	template <typename... Args>
	void
	debug(const std::format_string<Args...> &message, Args &&...args) const
	{
		log(Severity::Debug, message, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void
	info(const std::format_string<Args...> &message, Args &&...args) const
	{
		log(Severity::Info, message, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void
	warn(const std::format_string<Args...> &message, Args &&...args) const
	{
		log(Severity::Warn, message, std::forward<Args>(args)...);
	}

	template <typename... Args>
	void
	error(const std::format_string<Args...> &message, Args &&...args) const
	{
		log(Severity::Error, message, std::forward<Args>(args)...);
	}

	template <typename... Args>
	[[noreturn]] void
	fatal(const std::format_string<Args...> &message, Args &&...args) const
	{
		log(Severity::Fatal, message, std::forward<Args>(args)...);
		std::abort();
	}

protected:
	virtual void write_log(Severity level, const std::string &message) const
	    = 0;
};
} /* namespace euler::util */

#endif /* EULER_UTIL_LOGGER_H */
