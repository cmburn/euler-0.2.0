/* SPDX-License-Identifier: ISC */

#include <cassert>
#include <utility>

#include "euler/app/dragonruby/logger.h"
#include "euler/util/logger.h"

using euler::app::dragonruby::Logger;

namespace dragonruby {

/*
 * // Per Amir, these are the log levels that DragonRuby uses.
 * typedef enum LogLevel
 * {
 * 	// periodic (SPAMMY!) logging, like framerate updates, etc.
 * 	LOG_LEVEL_SPAM=0,
 * 	// Debug stuff that might be uninteresting/spammy
 * 	LOG_LEVEL_DEBUG,
 * 	// basic information. Good default log level.
 * 	LOG_LEVEL_INFO,
 * 	// warnings that should be noticed.
 * 	LOG_LEVEL_WARN,
 * 	// errors that should always be noticed.
 * 	LOG_LEVEL_ERROR,
 * 	// items that should be logged
 * 	LOG_LEVEL_UNFILTERED=0x7FFFFFFE,
 * 	// (almost) unconditionally.
 * 	LOG_LEVEL_NOTHING=0x7FFFFFFF   // don't ever log this thing.
 * } LogLevel;
 */

enum class LogLevel {
	/* periodic (SPAMMY!) logging, like framerate updates, etc. */
	Spam = 0,
	/* Debug stuff that might be uninteresting/spammy */
	Debug = 1,
	/* basic information. Good default log level. */
	Info = 2,
	/* warnings that should be noticed. */
	Warn = 3,
	/* errors that should always be noticed. */
	Error = 4,
	/* items that should be logged (almost) unconditionally. */
	Unfiltered = 0x7FFFFFFE,
	/* don't ever log this thing. */
	Nothing = 0x7FFFFFFF
};

} /* namespace dragonruby */

static int
severity_to_dr(euler::util::Logger::Severity severity)
{
	switch (severity) {
	case Logger::Severity::Debug:
		return static_cast<int>(dragonruby::LogLevel::Debug);
	case Logger::Severity::Info:
		return static_cast<int>(dragonruby::LogLevel::Info);
	case Logger::Severity::Warn:
		return static_cast<int>(dragonruby::LogLevel::Warn);
	case Logger::Severity::Error: [[fallthrough]];
	case Logger::Severity::Fatal:
		return static_cast<int>(dragonruby::LogLevel::Error);
	case Logger::Severity::Unknown:
		return static_cast<int>(dragonruby::LogLevel::Unfiltered);
	default: std::unreachable();
	}
}

Logger::Logger(std::string_view subsystem, log_write_fn log_write)
    : _subsystem(subsystem)
    , _severity(Severity::Debug)
    , _log_write(log_write)
{
}

std::string
Logger::subsystem() const
{
	std::scoped_lock lock(_subsystem_mutex);
	return _subsystem;
}

void
Logger::set_subsystem(const std::string_view name)
{
	std::scoped_lock lock(_subsystem_mutex);
	_subsystem = name;
}

Logger::Severity
Logger::severity() const
{
	std::scoped_lock lock(_severity_mutex);
	return _severity;
}

void
Logger::set_severity(const Severity level)
{
	std::scoped_lock lock(_severity_mutex);
	_severity = level;
}

euler::util::Reference<euler::util::Logger>
Logger::copy(const std::optional<std::string_view> subsystem) const
{
	return util::make_reference<Logger>(
	    subsystem.value_or(this->subsystem()), _log_write);
}

void
Logger::write_log(const Severity level, const std::string &message) const
{
	assert(_log_write != nullptr);
	_log_write(subsystem().c_str(), severity_to_dr(level), message.c_str());
}