/* SPDX-License-Identifier: ISC */

#include "euler/util/logger.h"

using euler::util::Logger;

static Logger::Severity
severity_from_symbol(mrb_state *mrb, const mrb_sym sym)
{
	if (sym == EULER_SYM(debug)) return Logger::Severity::Debug;
	if (sym == EULER_SYM(info)) return Logger::Severity::Info;
	if (sym == EULER_SYM(warn)) return Logger::Severity::Warn;
	if (sym == EULER_SYM(error)) return Logger::Severity::Error;
	if (sym == EULER_SYM(fatal)) return Logger::Severity::Fatal;
	return Logger::Severity::Unknown;
}

static mrb_value
log(mrb_state *mrb, const mrb_value self)
{
	using namespace euler::util;
	const auto logger = Reference<Logger>::unwrap(mrb, self);
	const auto state = State::get(mrb);
	mrb_sym severity_sym;
	const char *msg;
	mrb_int len;
	state->rb().get_args("ns", &severity_sym, &msg, &len);
	const auto severity = severity_from_symbol(mrb, severity_sym);
	const std::string_view message(msg, static_cast<std::size_t>(len));
	logger->log(severity, "{}", message);
	return mrb_nil_value();
}

template <Logger::Severity Severity>
static mrb_value
log_severity(mrb_state *mrb, const mrb_value self)
{
	using namespace euler::util;
	const auto logger = Reference<Logger>::unwrap(mrb, self);
	const auto state = State::get(mrb);
	const char *msg;
	state->rb().get_args("z", &msg);
	logger->log(Severity, "{}", msg);
	return mrb_nil_value();
}

RClass *
Logger::init(const Reference<State> &state, RClass *mod, RClass *)
{
	const auto cls = state->rb().define_class_under(mod, "Logger",
	    state->object_class());
	MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA);
	state->rb().define_method(cls, "log", ::log, MRB_ARGS_REQ(2));
	state->rb().define_method(cls, "debug", log_severity<Severity::Debug>,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(cls, "info", log_severity<Severity::Info>,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(cls, "warn", log_severity<Severity::Warn>,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(cls, "error", log_severity<Severity::Error>,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(cls, "fatal", log_severity<Severity::Fatal>,
	    MRB_ARGS_REQ(1));
	state->rb().define_method(cls, "unknown",
	    log_severity<Severity::Unknown>, MRB_ARGS_REQ(1));
	return cls;
}

Logger::Severity
Logger::parse_severity(const std::string_view str)
{
	if (str == "debug") return Severity::Debug;
	if (str == "info") return Severity::Info;
	if (str == "warn") return Severity::Warn;
	if (str == "error") return Severity::Error;
	if (str == "fatal") return Severity::Fatal;
	return Severity::Unknown;
}

const char *
Logger::severity_to_string(const Severity level)
{
	switch (level) {
	case Severity::Debug: return "debug";
	case Severity::Info: return "info";
	case Severity::Warn: return "warn";
	case Severity::Error: return "error";
	case Severity::Fatal: return "fatal";
	default: return "unknown";
	}
}