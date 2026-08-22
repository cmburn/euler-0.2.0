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

#include <quill/Logger.h>

#include "euler/util/logger.h"


namespace euler::app::native {
class State;

class Logger final : public util::Logger {
	friend class State;

public:
	Logger(std::string_view progname, std::string_view subsystem,
	    Severity severity);
	[[nodiscard]] std::string subsystem() const override;
	[[nodiscard]] Severity severity() const override;
	[[nodiscard]] util::Reference<util::Logger> copy(
	    std::optional<std::string_view> subsystem) const override;
	[[nodiscard]] std::string progname() const override;

	~Logger() override;

	static void global_init();

protected:
	void write_log(Severity level,
	    const std::string &message) const override;

private:
	std::string format_message(Severity level,
	    const std::string &message) const;

	std::string _name;
	bool _logger_exists;
	quill::Logger *_logger;
	std::string _progname;
	std::string _subsystem;

	Severity _severity = Severity::Info;
};
} /* namespace euler::app::native */

#endif /* EULER_APP_NATIVE_LOGGER_H */
