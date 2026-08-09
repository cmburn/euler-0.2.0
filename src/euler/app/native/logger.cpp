/* SPDX-License-Identifier: ISC */

#include <cassert>

#include <SDL3/SDL.h>

#include "euler/app/native/logger.h"
#include "euler/util/logger.h"

using euler::app::native::Logger;
using Severity = Logger::Severity;

enum class Color {
	None [[maybe_unused]],
	Black [[maybe_unused]],
	Red,
	Green,

	Yellow,
	Blue [[maybe_unused]],
	Magenta,
	Cyan,
	White,
};

using enum_t = std::underlying_type_t<Severity>;
static constexpr enum_t SEVERITY_COUNT
    = static_cast<enum_t>(Severity::Unknown) + 1;
static constexpr enum_t COLOR_COUNT = static_cast<enum_t>(Color::White) + 1;

using ColorList = std::array<Color, SEVERITY_COUNT>;

static constexpr ColorList SEVERITY_COLORS = {
	/* [Severity::Debug]   = */ Color::Green,
	/* [Severity::Info]    = */ Color::Green,
	/* [Severity::Warn]    = */ Color::Yellow,
	/* [Severity::Error]   = */ Color::Red,
	/* [Severity::Fatal]   = */ Color::Magenta,
	/* [Severity::Unknown] = */ Color::White,
};

static constexpr ColorList MESSAGE_COLORS = {
	/* [Severity::Debug]   = */ Color::Cyan,
	/* [Severity::Info]    = */ Color::White,
	/* [Severity::Warn]    = */ Color::Yellow,
	/* [Severity::Error]   = */ Color::Red,
	/* [Severity::Fatal]   = */ Color::Red,
	/* [Severity::Unknown] = */ Color::White,
};

static constexpr std::array<std::string_view, COLOR_COUNT> COLORS = {
	/* [Color::Black]   = */ "\033[30m",
	/* [Color::Red]     = */ "\033[31m",
	/* [Color::Green]   = */ "\033[32m",
	/* [Color::Yellow]  = */ "\033[33m",
	/* [Color::Blue]    = */ "\033[34m",
	/* [Color::Magenta] = */ "\033[35m",
	/* [Color::Cyan]    = */ "\033[36m",
	/* [Color::White]   = */ "\033[37m",
};

static constexpr std::array<std::string_view, SEVERITY_COUNT> SEVERITY_NAMES = {
	/* [Severity::Debug]   = */ "debug",
	/* [Severity::Info]    = */ "info",
	/* [Severity::Warn]    = */ "warn",
	/* [Severity::Error]   = */ "error",
	/* [Severity::Fatal]   = */ "fatal",
	/* [Severity::Unknown] = */ "any",
};

static std::string_view
color_for(Color color)
{
	return COLORS.at(static_cast<size_t>(color));
}

std::string_view
Logger::severity_name(Severity level)
{
	const auto idx = static_cast<size_t>(level);
	return SEVERITY_NAMES.at(idx);
}

void
Logger::Sink::push(const Severity sev, const std::string_view msg)
{
	std::lock_guard lock(mutex);
	queue.emplace_back(std::make_pair(sev, std::string(msg)));
	cv.notify_one();
}

void
Logger::Sink::flush_all()
{
	std::unique_lock lock(mutex);
	cv.wait(lock, [this]() { return !queue.empty(); });
	while (!queue.empty()) flush();
}
void
Logger::Sink::flush()
{
	const auto [sev, msg] = queue.front();
	queue.pop_front();
	if (sev < severity) return;
	fprintf(this->output, "%s", msg.c_str());
	fflush(this->output);
}

void
Logger::Sink::launch_thread()
{
	std::thread([this]() {
		/* ReSharper disable once CppDFAEndlessLoop */
		while (true) flush_all();
	}).detach();
}

Logger::Sink::Sink(FILE *output, const Severity severity)
{
	this->output = output;
	this->severity = severity;
}

std::shared_ptr<Logger::Sink>
Logger::stdout_sink()
{
	static auto sink = std::make_shared<Sink>(stdout, Severity::Info);
	return sink;
}

std::shared_ptr<Logger::Sink>
Logger::stderr_sink()
{
	static auto sink = std::make_shared<Sink>(stderr, Severity::Warn);
	return sink;
}

Logger::Logger(const std::string_view progname,
    const std::string_view subsystem, const Severity severity)
    : _progname(progname)
    , _subsystem(subsystem)
    , _severity(severity)
    , _sinks({ stdout_sink(), stderr_sink() })
{
	for (const auto &sink : _sinks) sink->launch_thread();
}

std::string
Logger::subsystem() const
{
	std::lock_guard lock(_subsystem_mutex);
	return _subsystem;
}

void
Logger::set_subsystem(const std::string_view name)
{
	std::lock_guard lock(_subsystem_mutex);
	_subsystem = name;
}

Severity
Logger::severity() const
{
	return _severity;
}

void
Logger::set_severity(const Severity level)
{
	_severity = level;
}

euler::util::Reference<euler::util::Logger>
Logger::copy(const std::optional<std::string_view> subsystem) const
{
	return util::Reference(new Logger(*this, subsystem));
}

Logger::~Logger() { info("Closing logger for {}", subsystem()); }

void
Logger::write_log(const Severity level, const std::string &message) const
{
	if (level < _severity) return;
	std::lock_guard lock(_sinks_mutex);
	for (const auto &sink : _sinks) {
		sink->push(level, format_message(level, message));
	}
}

Logger::Logger(const Logger &other,
    const std::optional<std::string_view> &subsystem)
{
	{
		std::lock_guard lock(other._progname_mutex);
		_progname = other._progname;
	}
	{
		std::lock_guard lock(other._subsystem_mutex);
		_subsystem = subsystem.value_or(other._subsystem);
	}
	{
		const Severity level = other._severity;
		_severity = level;
	}
}

std::string
Logger::progname() const
{
	std::lock_guard lock(_progname_mutex);
	return _progname;
}
void
Logger::set_progname(const std::string_view name)
{
	std::lock_guard lock(_progname_mutex);
	_progname = name;
}

/* ReSharper disable once CppDFAUnreachableFunctionCall */
static SDL_DateTime
current_time()
{
	SDL_Time t;
	SDL_GetCurrentTime(&t);
	SDL_DateTime dt;
	SDL_TimeToDateTime(t, &dt, true);
	return dt;
}

static constexpr size_t MAX_TIME_STRING_SIZE = 26;

/* ReSharper disable once CppDFAUnreachableFunctionCall */
static void
write_time(std::stringstream &ss)
{
	static constexpr const char *WEEK_DAYS[] = {
		"Sun",
		"Mon",
		"Tue",
		"Wed",
		"Thu",
		"Fri",
		"Sat",
	};
	static constexpr const char *MONTHS[] = {
		"Jan",
		"Feb",
		"Mar",
		"Apr",
		"May",
		"Jun",
		"Jul",
		"Aug",
		"Sep",
		"Oct",
		"Nov",
		"Dec",
	};

	const auto dt = current_time();
	char time_string[MAX_TIME_STRING_SIZE] = { 0 };
	snprintf(time_string, MAX_TIME_STRING_SIZE,
	    "%s %s %i %02d:%02d:%02d %i", WEEK_DAYS[dt.day_of_week],
	    MONTHS[dt.month - 1], dt.day, dt.hour, dt.minute, dt.second,
	    dt.year);
	ss << time_string;
}

std::string
Logger::format_message(Severity level, const std::string &message) const
{
	static constexpr size_t MAX_SEVERITY_LENGTH = 5;
	static constexpr auto CLEAR = "\033[0m";
	const auto index = static_cast<size_t>(level);
	const auto message_color = color_for(MESSAGE_COLORS.at(index));
	const auto severity_color = color_for(SEVERITY_COLORS.at(index));
	std::stringstream ss;
	ss << message_color << "[" << CLEAR << severity_color;
	write_time(ss);
	ss << CLEAR << message_color << "] ";
	const auto sev_str = severity_name(level);
	const auto padding = MAX_SEVERITY_LENGTH - sev_str.size();
	for (size_t i = 0; i < padding; i++) ss << ' ';
	ss << message_color << "[" << severity_color << sev_str << CLEAR
	   << message_color << "]";
	{
		std::lock_guard lock1(_progname_mutex), lock2(_subsystem_mutex);
		ss << " [" << CLEAR << severity_color << _progname
		   << "::" << _subsystem << CLEAR << message_color << "] -- ";
	}
	ss << message << std::endl;
	return ss.str();
}
