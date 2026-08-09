/* SPDX-License-Identifier: ISC */

#ifndef EULER_APP_NATIVE_LOGGER_H
#define EULER_APP_NATIVE_LOGGER_H

#include <condition_variable>
#include <deque>
#include <filesystem>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "euler/util/logger.h"

namespace euler::app::native {
class State;

class Logger final : public util::Logger {
	friend class State;

public:
	Logger(std::string_view progname, std::string_view subsystem,
	    Severity severity);
	[[nodiscard]] std::string subsystem() const override;
	void set_subsystem(std::string_view name) override;
	[[nodiscard]] Severity severity() const override;
	void set_severity(Severity level) override;
	[[nodiscard]] util::Reference<util::Logger> copy(
	    std::optional<std::string_view> subsystem) const override;
	[[nodiscard]] std::string progname() const override;
	void set_progname(std::string_view name) override;

	~Logger() override;

protected:
	void write_log(Severity level,
	    const std::string &message) const override;

private:
	Logger(const Logger &other,
	    const std::optional<std::string_view> &subsystem);
	std::string format_message(Severity level,
	    const std::string &message) const;
	static std::string_view severity_name(Severity level);

	struct Sink {
		FILE *output;
		std::atomic<Severity> severity = Severity::Info;
		std::condition_variable cv;
		std::mutex mutex;
		std::deque<std::pair<Severity, std::string>> queue;

		void push(Severity sev, std::string_view msg);
		void flush_all();
		void flush();
		void launch_thread();

		explicit Sink(FILE *output, Severity severity = Severity::Info);
	};

	static std::shared_ptr<Sink> stdout_sink();
	static std::shared_ptr<Sink> stderr_sink();

	std::string _progname;
	mutable std::mutex _progname_mutex;
	std::string _subsystem;
	mutable std::mutex _subsystem_mutex;
	std::atomic<Severity> _severity = Severity::Info;
	std::vector<std::shared_ptr<Sink>> _sinks;
	mutable std::mutex _sinks_mutex;
};
} /* namespace euler::app::native */

#endif /* EULER_APP_NATIVE_LOGGER_H */
