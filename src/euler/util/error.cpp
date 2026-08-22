/* SPDX-License-Identifier: ISC */

#include "euler/util/error.h"

#include <sstream>
#include <utility>

#include "euler/util/ruby_state.h"
#include "state.h"

using euler::util::Error;

static std::string
format_message(const euler::util::RubyState &,
    const std::string &cause, const char *class_name,
    const std::string &backtrace)
{
	auto message = std::ostringstream();
	if (class_name) message << class_name << ": ";
	message << cause;
	// if (backtrace) message << "\nBacktrace:\n" << *backtrace;
	if (!backtrace.empty()) message << "\nBacktrace:\n" << backtrace;
	return message.str();
}

Error::~Error() = default;

Error::Error(RubyState &state, const std::string &cause,
    const char *class_name, const std::optional<std::string_view> &backtrace)
    : _state(state)
    , _cause(cause)
    , _backtrace(backtrace ? std::string(*backtrace) : "")
    , _message(format_message(state, cause, class_name, _backtrace))
{
}

Error::Error(RubyState &state, RObject *exc)
    : _state(state)
{
	_cause = state.error_cause(exc);
	_backtrace = state.error_backtrace(exc);
	_message = format_message(state, _cause,
	    state.class_name(state.obj_class(mrb_obj_value(exc))),
	    _backtrace);
}

RClass *
Error::exception_class() const
{
	return state().mrb()->eException_class;
}

mrb_value
Error::to_mrb() const
{
	const auto exc_class = exception_class();
	const auto message = state().str_new_cstr(_message.c_str());
	return state().exc_new_str(exc_class, message);
}
euler::util::RubyState &
Error::state() const
{
	return _state;
}

RClass *
euler::util::StandardError::exception_class() const
{
	return state().mrb()->eStandardError_class;
}

RClass *
euler::util::RuntimeError::exception_class() const
{
	return state().runtime_error();
}

RClass *
euler::util::TypeError::exception_class() const
{
	return state().type_error();
}

RClass *
euler::util::ZeroDivisionError::exception_class() const
{
	return state().zero_division_error();
}

RClass *
euler::util::IndexError::exception_class() const
{
	return state().index_error();
}

RClass *
euler::util::RangeError::exception_class() const
{
	return state().range_error();
}

RClass *
euler::util::NameError::exception_class() const
{
	return state().name_error();
}

RClass *
euler::util::NoMethodError::exception_class() const
{
	return state().no_method_error();
}

RClass *
euler::util::ScriptError::exception_class() const
{
	return state().script_error();
}

RClass *
euler::util::SyntaxError::exception_class() const
{
	return state().syntax_error();
}

RClass *
euler::util::LocalJumpError::exception_class() const
{
	return state().local_jump_error();
}

RClass *
euler::util::RegexpError::exception_class() const
{
	return state().regexp_error();
}

RClass *
euler::util::FrozenError::exception_class() const
{
	return state().frozen_error();
}

RClass *
euler::util::NotImplementedError::exception_class() const
{
	return state().not_implemented_error();
}

RClass *
euler::util::KeyError::exception_class() const
{
	return state().key_error();
}

RClass *
euler::util::FloatDomainError::exception_class() const
{
	return state().float_domain_error();
}

std::string
euler::util::error_cause(RubyState &state, RObject *exc)
{
	return state.error_cause(exc);
}

std::string
euler::util::error_backtrace(RubyState &state, RObject *exc)
{
	return state.error_backtrace(exc);
}

void
euler::util::throw_error(RubyState &state, RObject *exc)
{
	const auto [kind, is_custom] = state.error_type_info(exc);
	if (is_custom) {
		switch (kind) {
		case Error::Kind::Exception: throw CustomError(state, exc);
		case Error::Kind::Standard:
			throw CustomError<StandardError>(state, exc);
		case Error::Kind::Runtime:
			throw CustomError<RuntimeError>(state, exc);
		case Error::Kind::Type:
			throw CustomError<TypeError>(state, exc);
		case Error::Kind::ZeroDivision:
			throw CustomError<ZeroDivisionError>(state, exc);
		case Error::Kind::Argument:
			throw CustomError<ArgumentError>(state, exc);
		case Error::Kind::Index:
			throw CustomError<IndexError>(state, exc);
		case Error::Kind::Range:
			throw CustomError<RangeError>(state, exc);
		case Error::Kind::Name:
			throw CustomError<NameError>(state, exc);
		case Error::Kind::NoMethod:
			throw CustomError<NoMethodError>(state, exc);
		case Error::Kind::Script:
			throw CustomError<ScriptError>(state, exc);
		case Error::Kind::Syntax:
			throw CustomError<SyntaxError>(state, exc);
		case Error::Kind::LocalJump:
			throw CustomError<LocalJumpError>(state, exc);
		case Error::Kind::Regexp:
			throw CustomError<RegexpError>(state, exc);
		case Error::Kind::Frozen:
			throw CustomError<FrozenError>(state, exc);
		case Error::Kind::NotImplemented:
			throw CustomError<NotImplementedError>(state, exc);
		case Error::Kind::Key: throw CustomError<KeyError>(state, exc);
		case Error::Kind::FloatDomain:
			throw CustomError<FloatDomainError>(state, exc);
		default: std::unreachable();
		}
	}
	switch (kind) {
	case Error::Kind::Exception: throw Error(state, exc);
	case Error::Kind::Standard: throw StandardError(state, exc);
	case Error::Kind::Runtime: throw RuntimeError(state, exc);
	case Error::Kind::Type: throw TypeError(state, exc);
	case Error::Kind::ZeroDivision: throw ZeroDivisionError(state, exc);
	case Error::Kind::Argument: throw ArgumentError(state, exc);
	case Error::Kind::Index: throw IndexError(state, exc);
	case Error::Kind::Range: throw RangeError(state, exc);
	case Error::Kind::Name: throw NameError(state, exc);
	case Error::Kind::NoMethod: throw NoMethodError(state, exc);
	case Error::Kind::Script: throw ScriptError(state, exc);
	case Error::Kind::Syntax: throw SyntaxError(state, exc);
	case Error::Kind::LocalJump: throw LocalJumpError(state, exc);
	case Error::Kind::Regexp: throw RegexpError(state, exc);
	case Error::Kind::Frozen: throw FrozenError(state, exc);
	case Error::Kind::NotImplemented: throw NotImplementedError(state, exc);
	case Error::Kind::Key: throw KeyError(state, exc);
	case Error::Kind::FloatDomain: throw FloatDomainError(state, exc);
	default: std::unreachable();
	}
}
