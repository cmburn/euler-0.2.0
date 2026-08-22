/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_ERROR_H
#define EULER_UTIL_ERROR_H

#include <format>
#include <string>

#include "euler/util/object.h"

namespace euler::util {

class RubyState;

/* ReSharper disable CppClassCanBeFinal */
class Error : public std::exception {
public:
	~Error() override;
	// Error(RubyState &state, const std::string &message);
	// template <typename... Args>
	// Error(RubyState &state, std::format_args fmt,
	//     Args... args)
	//     : Error(state, std::format(fmt, args...))
	// {
	// }
	// Error(RubyState &state, RObject *exc);

	Error(RubyState &state, const std::string &cause,
	    const char *class_name = nullptr,
	    const std::optional<std::string_view> &backtrace = std::nullopt);
	Error(RubyState &state, RObject *exc);

	enum class Kind {
		Exception,
		Standard,
		Runtime,
		Type,
		ZeroDivision,
		Argument,
		Index,
		Range,
		Name,
		NoMethod,
		Script,
		Syntax,
		LocalJump,
		Regexp,
		Frozen,
		NotImplemented,
		Key,
		FloatDomain,
	};

	// attempts to determine the most specific exception type for the given
	// exception object, and returns it. If the type is not a known type,
	// throws std::invalid_argument.
	// static Kind underlying_type(RubyState &state,
	//     RObject *exc);

	struct TypeInfo {
		Kind kind;
		bool is_custom;
	};

	[[nodiscard]] virtual Kind
	kind() const
	{
		return Kind::Exception;
	}
	[[nodiscard]] virtual RClass *exception_class() const;
	[[nodiscard]] virtual mrb_value to_mrb() const;
	[[nodiscard]] const char *
	what() const noexcept override
	{
		return _message.c_str();
	}
	[[nodiscard]] virtual bool
	is_custom() const
	{
		return false;
	}

	// [[nodiscard]] const RubyState &state() const;
	RubyState &state() const;

private:
	RubyState &_state;

protected:
	std::string _cause;
	std::string _backtrace;

private:
	std::string _message;
};

class StandardError : public Error {
public:
	StandardError(RubyState &state,
	    const std::string &message)
	    : Error(state, message)
	{
	}
	StandardError(RubyState &state, RObject *exc)
	    : Error(state, exc)
	{
	}
	~StandardError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Standard;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class RuntimeError : public StandardError {
public:
	RuntimeError(RubyState &state,
	    const std::string &message)
	    : StandardError(state, message)
	{
	}
	RuntimeError(RubyState &state, RObject *exc)
	    : StandardError(state, exc)
	{
	}
	~RuntimeError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Runtime;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class TypeError : public Error {
public:
	TypeError(RubyState &state, const std::string &message)
	    : Error(state, message)
	{
	}
	TypeError(RubyState &state, RObject *exc)
	    : Error(state, exc)
	{
	}
	~TypeError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Type;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class ZeroDivisionError : public StandardError {
public:
	ZeroDivisionError(RubyState &state,
	    const std::string &message)
	    : StandardError(state, message)
	{
	}
	ZeroDivisionError(RubyState &state, RObject *exc)
	    : StandardError(state, exc)
	{
	}
	~ZeroDivisionError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::ZeroDivision;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class ArgumentError : public StandardError {
public:
	ArgumentError(RubyState &state,
	    const std::string &message)
	    : StandardError(state, message)
	{
	}
	ArgumentError(RubyState &state, RObject *exc)
	    : StandardError(state, exc)
	{
	}
	~ArgumentError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Argument;
	}
};

class IndexError : public StandardError {
public:
	IndexError(RubyState &state,
	    const std::string &message)
	    : StandardError(state, message)
	{
	}
	IndexError(RubyState &state, RObject *exc)
	    : StandardError(state, exc)
	{
	}
	~IndexError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Index;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class RangeError : public StandardError {
public:
	RangeError(RubyState &state,
	    const std::string &message)
	    : StandardError(state, message)
	{
	}
	RangeError(RubyState &state, RObject *exc)
	    : StandardError(state, exc)
	{
	}
	~RangeError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Range;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class NameError : public StandardError {
public:
	NameError(RubyState &state, const std::string &message)
	    : StandardError(state, message)
	{
	}
	NameError(RubyState &state, RObject *exc)
	    : StandardError(state, exc)
	{
	}
	~NameError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Name;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class NoMethodError : public NameError {
public:
	NoMethodError(RubyState &state,
	    const std::string &message)
	    : NameError(state, message)
	{
	}
	NoMethodError(RubyState &state, RObject *exc)
	    : NameError(state, exc)
	{
	}
	~NoMethodError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::NoMethod;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class ScriptError : public Error {
public:
	ScriptError(RubyState &state,
	    const std::string &message)
	    : Error(state, message)
	{
	}
	ScriptError(RubyState &state, RObject *exc)
	    : Error(state, exc)
	{
	}
	~ScriptError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Script;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class SyntaxError : public ScriptError {
public:
	SyntaxError(RubyState &state,
	    const std::string &message)
	    : ScriptError(state, message)
	{
	}
	SyntaxError(RubyState &state, RObject *exc)
	    : ScriptError(state, exc)
	{
	}
	~SyntaxError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Syntax;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class LocalJumpError : public StandardError {
public:
	LocalJumpError(RubyState &state,
	    const std::string &message)
	    : StandardError(state, message)
	{
	}
	LocalJumpError(RubyState &state, RObject *exc)
	    : StandardError(state, exc)
	{
	}
	~LocalJumpError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::LocalJump;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class RegexpError : public StandardError {
public:
	RegexpError(RubyState &state,
	    const std::string &message)
	    : StandardError(state, message)
	{
	}
	RegexpError(RubyState &state, RObject *exc)
	    : StandardError(state, exc)
	{
	}
	~RegexpError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Regexp;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class FrozenError : public RuntimeError {
public:
	FrozenError(RubyState &state,
	    const std::string &message)
	    : RuntimeError(state, message)
	{
	}
	FrozenError(RubyState &state, RObject *exc)
	    : RuntimeError(state, exc)
	{
	}
	~FrozenError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Frozen;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class NotImplementedError : public ScriptError {
public:
	NotImplementedError(RubyState &state,
	    const std::string &message)
	    : ScriptError(state, message)
	{
	}
	NotImplementedError(RubyState &state, RObject *exc)
	    : ScriptError(state, exc)
	{
	}
	~NotImplementedError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::NotImplemented;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class KeyError : public IndexError {
public:
	KeyError(RubyState &state, const std::string &message)
	    : IndexError(state, message)
	{
	}
	KeyError(RubyState &state, RObject *exc)
	    : IndexError(state, exc)
	{
	}
	~KeyError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::Key;
	}
	[[nodiscard]] RClass *exception_class() const override;
};

class FloatDomainError : public RangeError {
public:
	FloatDomainError(RubyState &state,
	    const std::string &message)
	    : RangeError(state, message)
	{
	}
	FloatDomainError(RubyState &state, RObject *exc)
	    : RangeError(state, exc)
	{
	}
	~FloatDomainError() override = default;
	[[nodiscard]] Kind
	kind() const override
	{
		return Kind::FloatDomain;
	}
	[[nodiscard]] RClass *exception_class() const override;
};
/* ReSharper restore CppClassCanBeFinal */

std::string error_cause(RubyState &state, RObject *exc);
std::string error_backtrace(RubyState &state, RObject *exc);

template <typename T>
concept ErrorType = std::derived_from<T, Error>;

template <ErrorType T = Error> class CustomError : public T {
public:
	CustomError(RubyState &state, RObject *exc)
	    : T(state, exc)
	    , _exc(exc)
	{
		this->_cause = error_cause(state, exc);
		this->_backtrace = error_backtrace(state, exc);
	}

	~CustomError() override = default;

	[[nodiscard]] bool
	is_custom() const override
	{
		return true;
	}

	[[nodiscard]] RClass *
	exception_class() const override
	{
		return this->state().obj_class(mrb_obj_value(_exc));
	}

	[[nodiscard]] const char *
	what() const noexcept override
	{
		const auto str = this->state().any_to_s(mrb_obj_value(_exc));
		return this->state().string_cstr(str);
	}

private:
	RObject *_exc;
};

template <typename T, typename... Args>
inline auto
make_error(RubyState &state,
    const std::format_string<Args...> &fmt, Args &&...args)
{
	auto message = std::format(fmt, std::forward<Args>(args)...);
	return T(state, message);
}

[[noreturn]] void throw_error(RubyState &state, RObject *exc);

} /* namespace euler::util */

#endif /* EULER_UTIL_ERROR_H */
