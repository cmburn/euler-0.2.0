/* SPDX-License-Identifier: ISC */

#include <sys/syslog.h>

#include <SDL3/SDL.h>
#include <quill/LogFunctions.h>
#include <quill/Backend.h>
#include <quill/Frontend.h>
#include <quill/Logger.h>
#include <quill/sinks/ConsoleSink.h>


#include "euler/app/native/logger.h"
#include "euler/util/logger.h"

using euler::app::native::Logger;
using Severity = Logger::Severity;

static quill::LogLevel
to_quill(Logger::Severity severity)
{
	switch (severity) {
	case Severity::Debug: return quill::LogLevel::Debug;
	case Severity::Info: return quill::LogLevel::Info;
	case Severity::Warn: return quill::LogLevel::Warning;
	case Severity::Error: return quill::LogLevel::Error;
	case Severity::Fatal: return quill::LogLevel::Critical;
	case Severity::Unknown: [[fallthrough]];
	default: return quill::LogLevel::None;
	}
}

static bool
logger_exists(const std::string &name)
{
	return quill::Frontend::get_logger(name) != nullptr;
}
static constexpr auto LOG_FMT_TIME = "%a %b %e %H:%M:%S %Y";
static constexpr auto LOG_FMT_STR = "[%(time)] [%(log_level:<5)] "
                                    "[%(logger)] -- %(message)";

static quill::Logger *
make_logger(const std::string &name, const Logger::Severity severity)
{
	auto ptr = quill::Frontend::get_logger(name);
	if (ptr != nullptr) {
		if (ptr->get_log_level() != to_quill(severity)) {
			quill::warning(ptr,
			    "Logger {} already exists with a different log "
			    "level; defaulting to the most verbose.");
		}
		return ptr;
	}
	auto console_sink
	    = quill::Frontend::create_or_get_sink<quill::ConsoleSink>("stdout");
	auto log_fmt = quill::PatternFormatterOptions(LOG_FMT_STR);
	log_fmt.timestamp_pattern = LOG_FMT_TIME;
	ptr = quill::Frontend::create_logger(name, { console_sink },
		log_fmt);
	ptr->set_log_level(to_quill(severity));

	return ptr;
}

Logger::Logger(std::string_view progname, std::string_view subsystem,
    Severity severity)
    : _name(std::format("{}::{}", progname, subsystem))
    , _logger_exists(logger_exists(_name))
    , _logger(make_logger(_name, severity))
    , _progname(progname)
    , _subsystem(subsystem)
    , _severity(severity)
{
}

std::string
Logger::subsystem() const
{
	return _subsystem;
}

Severity
Logger::severity() const
{
	return _severity;
}

euler::util::Reference<euler::util::Logger>
Logger::copy(const std::optional<std::string_view> subsystem) const
{
	return util::make_reference<Logger>(_progname,
	    subsystem.value_or(_subsystem), _severity);
}

void
Logger::global_init()
{
	static std::once_flag once;
	std::call_once(once, []() {
		const quill::BackendOptions opts = {};
		quill::Backend::start(opts);
	});
}

void
Logger::write_log(const Severity level, const std::string &message) const
{
	const auto ql = to_quill(level);
	quill::log(_logger, "", ql, "{}", quill::SourceLocation::current(),
	    message);
}

std::string
Logger::progname() const
{
	return _progname;
}

Logger::~Logger() { _logger->flush_log(); }
