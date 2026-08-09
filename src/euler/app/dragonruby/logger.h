/* SPDX-License-Identifier: ISC */

#ifndef EULER_APP_DRAGONRUBY_LOGGER_H
#define EULER_APP_DRAGONRUBY_LOGGER_H

#include <mutex>

#include "euler/util/logger.h"
#include "euler/util/object.h"

namespace euler::app::dragonruby {
class Logger final : public util::Logger {
public:
	using log_write_fn
	    = void (*)(const char *subsystem, int level, const char *str);

	Logger(std::string_view subsystem, log_write_fn log_write);
	[[nodiscard]] std::string subsystem() const override;
	void set_subsystem(std::string_view name) override;
	[[nodiscard]] Severity severity() const override;
	void set_severity(Severity level) override;
	[[nodiscard]] util::Reference<util::Logger> copy(
	    std::optional<std::string_view> subsystem) const override;

protected:
	void write_log(Severity, const std::string &) const override;

private:
	std::string _subsystem;
	mutable std::mutex _subsystem_mutex;
	Severity _severity;
	mutable std::mutex _severity_mutex;
	log_write_fn _log_write = nullptr;
};
} /* namespace euler::app::dragonruby */

#endif /* EULER_APP_DRAGONRUBY_LOGGER_H */
