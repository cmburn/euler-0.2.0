/* SPDX-License-Identifier: ISC */

#include "euler/app/native/ruby_state.h"

#include <cassert>
#include <functional>
#include <mrbconf.h>
#include <mruby.h>
#include <sstream>
#include <stdexcept>
#include <SDL3/SDL_stdinc.h>
#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/dump.h>
#include <mruby/error.h>
#include <mruby/gc.h>
#include <mruby/internal.h>
#include <mruby/irep.h>
#include <mruby/numeric.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/throw.h>

#include "euler/app/native/state.h"
#include "euler/util/error.h"

using euler::app::native::RubyState;

/* ReSharper disable once CppNotAllPathsReturnValue */
static auto
wrap_call(mrb_state *mrb, const auto &func) -> decltype(func())
{
	assert(mrb != nullptr);
	using T = decltype(func());
	auto prev_jmpbuf = mrb->jmp; // save previous jmpbuf
	mrb_jmpbuf new_jmpbuf = {};
	auto state = euler::app::native::State::get(mrb);
	MRB_TRY(&new_jmpbuf)
	{
		if constexpr (std::is_void_v<T>) {
			func();
			mrb->jmp = prev_jmpbuf;
			state->mrb()->raise_on_error();
		} else {
			auto result = func();
			mrb->jmp = prev_jmpbuf;
			state->mrb()->raise_on_error();
			return result;
		}
	}
	MRB_CATCH(&new_jmpbuf)
	{
		mrb->jmp = prev_jmpbuf;
		state->mrb()->raise_on_error();
		return decltype(func())();
	}
	MRB_END_EXC(&new_jmpbuf)
}

#define WRAP_CALL(EXPR) wrap_call(_mrb, [&]() { return (EXPR); })

RubyState::RubyState()
    : _mrb(mrb_open())
{
}

RubyState::~RubyState() { close(); }

mrb_state *
RubyState::mrb() const
{
	return _mrb;
}

void
RubyState::raise_on_error()
{
	/* TODO */
}

void
RubyState::raise(RClass *c, const char *msg)
{
	mrb_raise(_mrb, c, msg);
}

void
RubyState::raisef(RClass *c, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto value = vformat(fmt, ap);
	va_end(ap);
	const char *msg = string_cstr(value);
	WRAP_CALL(raise(c, msg));
	std::unreachable();
}

RClass *
RubyState::module_get(const char *name)
{
	return WRAP_CALL(mrb_module_get(_mrb, name));
}

RClass *
RubyState::module_get_under(RClass *outer, const char *name)
{
	return WRAP_CALL(mrb_module_get_under(_mrb, outer, name));
}

RClass *
RubyState::define_module_under(RClass *outer, const char *name)
{
	return WRAP_CALL(mrb_define_module_under(_mrb, outer, name));
}

RClass *
RubyState::class_get_under(RClass *outer, const char *name)
{
	return WRAP_CALL(mrb_class_get_under(_mrb, outer, name));
}

mrb_bool
RubyState::class_ptr_p(mrb_value obj)
{
	switch (mrb_type(obj)) {
	case MRB_TT_CLASS:
	case MRB_TT_SCLASS:
	case MRB_TT_MODULE: return true;
	default: return false;
	}
}

RClass *
RubyState::define_class_under(RClass *outer, const char *name, RClass *super)
{
	return WRAP_CALL(mrb_define_class_under(_mrb, outer, name, super));
}

void
RubyState::define_module_function(RClass *cla, const char *name, mrb_func_t fun,
    mrb_aspec aspec)
{
	WRAP_CALL(mrb_define_module_function(_mrb, cla, name, fun, aspec));
}

void
RubyState::define_method(RClass *cla, const char *name, mrb_func_t func,
    mrb_aspec aspec)
{
	WRAP_CALL(mrb_define_method(_mrb, cla, name, func, aspec));
}

void
RubyState::define_class_method(RClass *cla, const char *name, mrb_func_t fun,
    mrb_aspec aspec)
{
	return WRAP_CALL(mrb_define_class_method(_mrb, cla, name, fun, aspec));
}

mrb_int
RubyState::get_args(mrb_args_format format, ...)
{
	std::vector<void *> args;
	va_list ap;
	va_start(ap, format);
	auto argc = get_argc();
	if (argc <= 0) {
		va_end(ap);
		return argc;
	}
	if (format[strlen(format) - 1] == ':') ++argc;
	for (mrb_int i = 0; i < argc; ++i) args.push_back(va_arg(ap, void *));
	va_end(ap);
	auto out = get_args_a(format, args.data());
	return out;
}

mrb_value
RubyState::str_new_cstr(const char *str)
{
	return WRAP_CALL(mrb_str_new_cstr(_mrb, str));
}

RData *
RubyState::data_object_alloc(RClass *klass, void *datap,
    const mrb_data_type *type)
{
	return WRAP_CALL(mrb_data_object_alloc(_mrb, klass, datap, type));
}

mrb_value
RubyState::ensure_float_type(mrb_value val)
{
	return WRAP_CALL(mrb_ensure_float_type(_mrb, val));
}

mrb_value
RubyState::ensure_integer_type(mrb_value val)
{
	return WRAP_CALL(mrb_ensure_integer_type(_mrb, val));
}

mrb_irep *
RubyState::add_irep()
{
	return WRAP_CALL(mrb_add_irep(_mrb));
}

void
RubyState::alias_method(RClass *c, mrb_sym a, mrb_sym b)
{
	WRAP_CALL(mrb_alias_method(_mrb, c, a, b));
}

mrb_value
RubyState::any_to_s(mrb_value obj)
{
	return WRAP_CALL(mrb_any_to_s(_mrb, obj));
}

void
RubyState::argnum_error(mrb_int argc, int min, int max)
{
	WRAP_CALL(mrb_argnum_error(_mrb, argc, min, max));
}

mrb_value
RubyState::ary_clear(mrb_value self)
{
	return WRAP_CALL(mrb_ary_clear(_mrb, self));
}

void
RubyState::ary_concat(mrb_value self, mrb_value other)
{
	WRAP_CALL(mrb_ary_concat(_mrb, self, other));
}

mrb_value
RubyState::ary_entry(mrb_value ary, mrb_int offset)
{
	return WRAP_CALL(mrb_ary_entry(ary, offset));
}

mrb_value
RubyState::ary_join(mrb_value ary, mrb_value sep)
{
	return WRAP_CALL(mrb_ary_join(_mrb, ary, sep));
}

void
RubyState::ary_modify(RArray *ary)
{
	WRAP_CALL(mrb_ary_modify(_mrb, ary));
}

mrb_value
RubyState::ary_new()
{
	return WRAP_CALL(mrb_ary_new(_mrb));
}

mrb_value
RubyState::ary_new_capa(mrb_int cap)
{
	return WRAP_CALL(mrb_ary_new_capa(_mrb, cap));
}

mrb_value
RubyState::ary_new_from_values(mrb_int size, const mrb_value *vals)
{
	return WRAP_CALL(mrb_ary_new_from_values(_mrb, size, vals));
}

mrb_value
RubyState::ary_pop(mrb_value ary)
{
	return WRAP_CALL(mrb_ary_pop(_mrb, ary));
}

void
RubyState::ary_push(mrb_value array, mrb_value value)
{
	WRAP_CALL(mrb_ary_push(_mrb, array, value));
}

mrb_value
RubyState::ary_ref(mrb_value ary, mrb_int n)
{
	return WRAP_CALL(mrb_ary_ref(_mrb, ary, n));
}

void
RubyState::ary_replace(mrb_value self, mrb_value other)
{
	WRAP_CALL(mrb_ary_replace(_mrb, self, other));
}

mrb_value
RubyState::ary_resize(mrb_value ary, mrb_int new_len)
{
	return WRAP_CALL(mrb_ary_resize(_mrb, ary, new_len));
}

void
RubyState::ary_set(mrb_value ary, mrb_int n, mrb_value val)
{
	WRAP_CALL(mrb_ary_set(_mrb, ary, n, val));
}

mrb_value
RubyState::ary_shift(mrb_value self)
{
	return WRAP_CALL(mrb_ary_shift(_mrb, self));
}

mrb_value
RubyState::ary_splat(mrb_value value)
{
	return WRAP_CALL(mrb_ary_splat(_mrb, value));
}

mrb_value
RubyState::ary_splice(mrb_value self, mrb_int head, mrb_int len, mrb_value rpl)
{
	return WRAP_CALL(mrb_ary_splice(_mrb, self, head, len, rpl));
}

mrb_value
RubyState::ary_unshift(mrb_value self, mrb_value item)
{
	return WRAP_CALL(mrb_ary_unshift(_mrb, self, item));
}

mrb_value
RubyState::assoc_new(mrb_value car, mrb_value cdr)
{
	return WRAP_CALL(mrb_assoc_new(_mrb, car, cdr));
}

mrb_value
RubyState::attr_get(mrb_value obj, mrb_sym id)
{
	return WRAP_CALL(mrb_attr_get(_mrb, obj, id));
}

void
RubyState::bug(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto value = vformat(fmt, ap);
	va_end(ap);
	const char *msg = string_cstr(value);
	WRAP_CALL(bug(msg));
}

void *
RubyState::calloc(size_t count, size_t size)
{
	return WRAP_CALL(mrb_calloc(_mrb, count, size));
}

mrb_value
RubyState::check_array_type(mrb_value self)
{
	return WRAP_CALL(mrb_check_array_type(_mrb, self));
}

mrb_value
RubyState::check_hash_type(mrb_value hash)
{
	return WRAP_CALL(mrb_check_hash_type(_mrb, hash));
}

mrb_value
RubyState::check_intern(const char *str, size_t len)
{
	return WRAP_CALL(mrb_check_intern(_mrb, str, len));
}

mrb_value
RubyState::check_intern_cstr(const char *str)
{
	return WRAP_CALL(mrb_check_intern_cstr(_mrb, str));
}

mrb_value
RubyState::check_intern_str(mrb_value val)
{
	return WRAP_CALL(mrb_check_intern_str(_mrb, val));
}

mrb_value
RubyState::check_string_type(mrb_value str)
{
	return WRAP_CALL(mrb_check_string_type(_mrb, str));
}

void
RubyState::check_type(mrb_value x, mrb_vtype t)
{
	WRAP_CALL(mrb_check_type(_mrb, x, t));
}

mrb_bool
RubyState::class_defined(const char *name)
{
	return WRAP_CALL(mrb_class_defined(_mrb, name));
}

mrb_bool
RubyState::class_defined_id(mrb_sym name)
{
	return WRAP_CALL(mrb_class_defined_id(_mrb, name));
}

mrb_bool
RubyState::class_defined_under(RClass *outer, const char *name)
{
	return WRAP_CALL(mrb_class_defined_under(_mrb, outer, name));
}

mrb_bool
RubyState::class_defined_under_id(RClass *outer, mrb_sym name)
{
	return WRAP_CALL(mrb_class_defined_under_id(_mrb, outer, name));
}

RClass *
RubyState::class_get(const char *name)
{
	return WRAP_CALL(mrb_class_get(_mrb, name));
}

RClass *
RubyState::class_get_id(mrb_sym name)
{
	return WRAP_CALL(mrb_class_get_id(_mrb, name));
}

RClass *
RubyState::class_get_under_id(RClass *outer, mrb_sym name)
{
	return WRAP_CALL(mrb_class_get_under_id(_mrb, outer, name));
}

const char *
RubyState::class_name(RClass *cls)
{
	return WRAP_CALL(mrb_class_name(_mrb, cls));
}

RClass *
RubyState::class_new(RClass *super)
{
	return WRAP_CALL(mrb_class_new(_mrb, super));
}

mrb_value
RubyState::class_path(RClass *c)
{
	return WRAP_CALL(mrb_class_path(_mrb, c));
}

RClass *
RubyState::class_real(RClass *cl)
{
	return WRAP_CALL(mrb_class_real(cl));
}

void
RubyState::close()
{
	WRAP_CALL(mrb_close(_mrb));
}

RProc *
RubyState::closure_new_cfunc(mrb_func_t func, int nlocals)
{
	return WRAP_CALL(mrb_closure_new_cfunc(_mrb, func, nlocals));
}

mrb_int
RubyState::cmp(mrb_value obj1, mrb_value obj2)
{
	return WRAP_CALL(mrb_cmp(_mrb, obj1, obj2));
}

mrb_bool
RubyState::const_defined(mrb_value value, mrb_sym sym)
{
	return WRAP_CALL(mrb_const_defined(_mrb, value, sym));
}

mrb_bool
RubyState::const_defined_at(mrb_value mod, mrb_sym id)
{
	return WRAP_CALL(mrb_const_defined_at(_mrb, mod, id));
}

mrb_value
RubyState::const_get(mrb_value value, mrb_sym sym)
{
	return WRAP_CALL(mrb_const_get(_mrb, value, sym));
}

void
RubyState::const_remove(mrb_value value, mrb_sym sym)
{
	WRAP_CALL(mrb_const_remove(_mrb, value, sym));
}

void
RubyState::const_set(mrb_value value, mrb_sym sym, mrb_value val)
{
	WRAP_CALL(mrb_const_set(_mrb, value, sym, val));
}

template <typename T>
static T
parse_value(RubyState &state, const char *s, bool badcheck, const char *name,
    std::function<T(const char *, char **)> parse_fn)
{
	const auto len = strlen(s);
	char *endptr = nullptr;
	const char *str_end = s + len;
	const auto value = parse_fn(s, &endptr);
	if (!badcheck || endptr == str_end) return value;
	const auto std_class = state.mrb()->eStandardError_class;
	state.raisef(std_class, "invalid value for %s(): \"%s\"", name, s);
}

double
RubyState::cstr_to_dbl(const char *s, mrb_bool badcheck)
{
	return parse_value<double>(*this, s, badcheck, "Float", SDL_strtod);
}

mrb_value
RubyState::cstr_to_inum(const char *s, mrb_int base, mrb_bool badcheck)
{
	static_assert(std::is_same_v<mrb_int, long>
	        || std::is_same_v<mrb_int, long long>,
	    "unexpected mrb_int type");
	const auto value = parse_value<mrb_int>(*this, s, badcheck, "Integer",
	    [base](const char *str, char **endptr) {
		    return SDL_strtol(str, endptr, base);
	    });
	return WRAP_CALL(mrb_int_value(_mrb, value));
}

mrb_bool
RubyState::cv_defined(mrb_value mod, mrb_sym sym)
{
	return WRAP_CALL(mrb_cv_defined(_mrb, mod, sym));
}

mrb_value
RubyState::cv_get(mrb_value mod, mrb_sym sym)
{
	return WRAP_CALL(mrb_cv_get(_mrb, mod, sym));
}

void
RubyState::cv_set(mrb_value mod, mrb_sym sym, mrb_value v)
{
	WRAP_CALL(mrb_cv_set(_mrb, mod, sym, v));
}

void *
RubyState::data_check_get_ptr(mrb_value value, const mrb_data_type *type)
{
	return WRAP_CALL(mrb_data_check_get_ptr(_mrb, value, type));
}

void
RubyState::data_check_type(mrb_value value, const mrb_data_type *type)
{
	WRAP_CALL(mrb_data_check_type(_mrb, value, type));
}

void *
RubyState::data_get_ptr(mrb_value value, const mrb_data_type *type)
{
	return WRAP_CALL(mrb_data_get_ptr(_mrb, value, type));
}

const char *
RubyState::debug_get_filename(const mrb_irep *irep, uint32_t pc)
{
	return WRAP_CALL(mrb_debug_get_filename(_mrb, irep, pc));
}

int32_t
RubyState::debug_get_line(const mrb_irep *irep, uint32_t pc)
{
	return WRAP_CALL(mrb_debug_get_line(_mrb, irep, pc));
}

mrb_irep_debug_info *
RubyState::debug_info_alloc(mrb_irep *irep)
{
	return WRAP_CALL(mrb_debug_info_alloc(_mrb, irep));
}

mrb_irep_debug_info_file *
RubyState::debug_info_append_file(mrb_irep_debug_info *info,
    const char *filename, uint16_t *lines, uint32_t start_pos, uint32_t end_pos)
{
	return WRAP_CALL(mrb_debug_info_append_file(_mrb, info, filename, lines,
	    start_pos, end_pos));
}

void
RubyState::debug_info_free(mrb_irep_debug_info *d)
{
	WRAP_CALL(mrb_debug_info_free(_mrb, d));
}

void
RubyState::define_alias(RClass *c, const char *a, const char *b)
{
	WRAP_CALL(mrb_define_alias(_mrb, c, a, b));
}

void
RubyState::define_alias_id(RClass *c, mrb_sym a, mrb_sym b)
{
	WRAP_CALL(mrb_define_alias_id(_mrb, c, a, b));
}

RClass *
RubyState::define_class(const char *name, RClass *super)
{
	return WRAP_CALL(mrb_define_class(_mrb, name, super));
}

RClass *
RubyState::define_class_id(mrb_sym name, RClass *super)
{
	return WRAP_CALL(mrb_define_class_id(_mrb, name, super));
}

void
RubyState::define_class_method_id(RClass *cla, mrb_sym name, mrb_func_t fun,
    mrb_aspec aspec)
{
	WRAP_CALL(mrb_define_class_method_id(_mrb, cla, name, fun, aspec));
}

RClass *
RubyState::define_class_under_id(RClass *outer, mrb_sym name, RClass *super)
{
	return WRAP_CALL(mrb_define_class_under_id(_mrb, outer, name, super));
}

void
RubyState::define_const(RClass *cla, const char *name, mrb_value val)
{
	WRAP_CALL(mrb_define_const(_mrb, cla, name, val));
}

void
RubyState::define_const_id(RClass *cla, mrb_sym name, mrb_value val)
{
	WRAP_CALL(mrb_define_const_id(_mrb, cla, name, val));
}

void
RubyState::define_global_const(const char *name, mrb_value val)
{
	WRAP_CALL(mrb_define_global_const(_mrb, name, val));
}

void
RubyState::define_method_id(RClass *c, mrb_sym mid, mrb_func_t func,
    mrb_aspec aspec)
{
	WRAP_CALL(mrb_define_method_id(_mrb, c, mid, func, aspec));
}

void
RubyState::define_method_raw(RClass *c, mrb_sym mid, mrb_method_t meth)
{
	WRAP_CALL(mrb_define_method_raw(_mrb, c, mid, meth));
}

RClass *
RubyState::define_module(const char *name)
{
	return WRAP_CALL(mrb_define_module(_mrb, name));
}

void
RubyState::define_module_function_id(RClass *cla, mrb_sym name, mrb_func_t fun,
    mrb_aspec aspec)
{
	WRAP_CALL(mrb_define_module_function_id(_mrb, cla, name, fun, aspec));
}

RClass *
RubyState::define_module_id(mrb_sym name)
{
	return WRAP_CALL(mrb_define_module_id(_mrb, name));
}

RClass *
RubyState::define_module_under_id(RClass *outer, mrb_sym name)
{
	return WRAP_CALL(mrb_define_module_under_id(_mrb, outer, name));
}

void
RubyState::define_singleton_method(RObject *cla, const char *name,
    mrb_func_t fun, mrb_aspec aspec)
{
	WRAP_CALL(mrb_define_singleton_method(_mrb, cla, name, fun, aspec));
}

void
RubyState::define_singleton_method_id(RObject *cla, mrb_sym name,
    mrb_func_t fun, mrb_aspec aspec)
{
	WRAP_CALL(mrb_define_singleton_method_id(_mrb, cla, name, fun, aspec));
}

mrb_value
RubyState::ensure(mrb_func_t body, mrb_value b_data, mrb_func_t ensure,
    mrb_value e_data)
{
	return WRAP_CALL(mrb_ensure(_mrb, body, b_data, ensure, e_data));
}

mrb_value
RubyState::ensure_array_type(mrb_value self)
{
	return WRAP_CALL(mrb_ensure_array_type(_mrb, self));
}

mrb_value
RubyState::ensure_hash_type(mrb_value hash)
{
	return WRAP_CALL(mrb_ensure_hash_type(_mrb, hash));
}

mrb_value
RubyState::ensure_string_type(mrb_value str)
{
	return WRAP_CALL(mrb_ensure_string_type(_mrb, str));
}

mrb_bool
RubyState::eql(mrb_value obj1, mrb_value obj2)
{
	return WRAP_CALL(mrb_eql(_mrb, obj1, obj2));
}

mrb_bool
RubyState::equal(mrb_value obj1, mrb_value obj2)
{
	return WRAP_CALL(mrb_equal(_mrb, obj1, obj2));
}

mrb_value
RubyState::exc_backtrace(mrb_value exc)
{
	return WRAP_CALL(mrb_exc_backtrace(_mrb, exc));
}

RClass *
RubyState::exc_get_id(mrb_sym name)
{
	return WRAP_CALL(mrb_exc_get_id(_mrb, name));
}

mrb_value
RubyState::exc_new(RClass *c, const char *ptr, size_t len)
{
	return WRAP_CALL(mrb_exc_new(_mrb, c, ptr, len));
}

mrb_value
RubyState::exc_new_str(RClass *c, mrb_value str)
{
	return WRAP_CALL(mrb_exc_new_str(_mrb, c, str));
}

void
RubyState::exc_raise(mrb_value exc)
{
	WRAP_CALL(mrb_exc_raise(_mrb, exc));
}

mrb_value
RubyState::f_raise(mrb_value value)
{
	return WRAP_CALL(mrb_f_raise(_mrb, value));
}

mrb_value
RubyState::fiber_alive_p(mrb_value fib)
{
	return WRAP_CALL(mrb_fiber_alive_p(_mrb, fib));
}

mrb_value
RubyState::fiber_resume(mrb_value fib, mrb_int argc, const mrb_value *argv)
{
	return WRAP_CALL(mrb_fiber_resume(_mrb, fib, argc, argv));
}

mrb_value
RubyState::fiber_yield(mrb_int argc, const mrb_value *argv)
{
	return WRAP_CALL(mrb_fiber_yield(_mrb, argc, argv));
}

void
RubyState::field_write_barrier(RBasic *a, RBasic *b)
{
	WRAP_CALL(mrb_field_write_barrier(_mrb, a, b));
}

mrb_value
RubyState::integer_to_str(mrb_value x, mrb_int base)
{
	return WRAP_CALL(mrb_integer_to_str(_mrb, x, base));
}

mrb_value
RubyState::float_to_integer(mrb_value val)
{
	return WRAP_CALL(mrb_float_to_integer(_mrb, val));
}

double
RubyState::float_read(const char *str, char **endptr)
{
	return WRAP_CALL(mrb_float_read(str, endptr));
}

int
RubyState::float_to_cstr(char *buf, size_t len, const char *fmt, mrb_float f)
{
	const auto rb_val = WRAP_CALL(mrb_float_value(_mrb, f));
	const auto val = WRAP_CALL(mrb_float_to_str(_mrb, rb_val, fmt));
	const auto str = mrb_string_value_ptr(_mrb, val);
	const auto str_len = mrb_string_value_len(_mrb, val);
	return strlcpy(buf, str, std::min(len, static_cast<size_t>(str_len)));
}

mrb_value
RubyState::float_to_str(mrb_value x, const char *fmt)
{
	return WRAP_CALL(mrb_float_to_str(_mrb, x, fmt));
}

mrb_value
RubyState::format(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	const auto value = vformat(format, ap);
	va_end(ap);
	return value;
}

void
RubyState::free(void *ptr)
{
	WRAP_CALL(mrb_free(_mrb, ptr));
}

void
RubyState::free_context(mrb_context *c)
{
	WRAP_CALL(mrb_free_context(_mrb, c));
}

void
RubyState::frozen_error(void *frozen_obj)
{
	WRAP_CALL(mrb_frozen_error(_mrb, frozen_obj));
}

void
RubyState::full_gc()
{
	WRAP_CALL(mrb_full_gc(_mrb));
}

mrb_bool
RubyState::func_basic_p(mrb_value obj, mrb_sym mid, mrb_func_t func)
{
	return WRAP_CALL(mrb_func_basic_p(_mrb, obj, mid, func));
}

mrb_value
RubyState::funcall(mrb_value val, const char *name, mrb_int argc, ...)
{
	va_list ap;
	va_start(ap, argc);
	std::vector<mrb_value> argv;
	argv.reserve(argc);
	for (mrb_int i = 0; i < argc; ++i)
		argv.push_back(va_arg(ap, mrb_value));
	va_end(ap);
	const mrb_sym sym = intern_cstr(name);
	return funcall_argv(val, sym, argc, argv.data());
}

mrb_value
RubyState::funcall_argv(mrb_value val, mrb_sym name, mrb_int argc,
    const mrb_value *argv)
{
	return WRAP_CALL(mrb_funcall_argv(_mrb, val, name, argc, argv));
}

mrb_value
RubyState::funcall_with_block(mrb_value val, mrb_sym name, mrb_int argc,
    const mrb_value *argv, mrb_value block)
{
	return WRAP_CALL(
	    mrb_funcall_with_block(_mrb, val, name, argc, argv, block));
}

void
RubyState::garbage_collect()
{
	WRAP_CALL(mrb_garbage_collect(_mrb));
}

void
RubyState::gc_mark(RBasic *obj)
{
	WRAP_CALL(mrb_gc_mark(_mrb, obj));
}

void
RubyState::gc_protect(mrb_value obj)
{
	WRAP_CALL(mrb_gc_protect(_mrb, obj));
}

void
RubyState::gc_register(mrb_value obj)
{
	WRAP_CALL(mrb_gc_register(_mrb, obj));
}

void
RubyState::gc_unregister(mrb_value obj)
{
	WRAP_CALL(mrb_gc_unregister(_mrb, obj));
}

RProc *
RubyState::generate_code(mrb_parser_state *ps)
{
	return WRAP_CALL(mrb_generate_code(_mrb, ps));
}

mrb_value
RubyState::get_arg1()
{
	return WRAP_CALL(mrb_get_arg1(_mrb));
}

mrb_int
RubyState::get_argc()
{
	return WRAP_CALL(mrb_get_argc(_mrb));
}

const mrb_value *
RubyState::get_argv()
{
	return WRAP_CALL(mrb_get_argv(_mrb));
}

mrb_int
RubyState::get_args_a(mrb_args_format format, void **ptr)
{
	return WRAP_CALL(mrb_get_args_a(_mrb, format, ptr));
}

mrb_value
RubyState::get_backtrace()
{
	return WRAP_CALL(mrb_get_backtrace(_mrb));
}

mrb_value
RubyState::gv_get(mrb_sym sym)
{
	return WRAP_CALL(mrb_gv_get(_mrb, sym));
}

void
RubyState::gv_remove(mrb_sym sym)
{
	WRAP_CALL(mrb_gv_remove(_mrb, sym));
}

void
RubyState::gv_set(mrb_sym sym, mrb_value val)
{
	WRAP_CALL(mrb_gv_set(_mrb, sym, val));
}

void
RubyState::hash_check_kdict(mrb_value self)
{
	const auto keys = hash_keys(self);
	const mrb_int len = RARRAY_LEN(keys);
	for (mrb_int i = 0; i < len; ++i) {
		const auto key = ary_entry(keys, i);
		if (mrb_symbol_p(key)) continue;
		const auto state = euler::app::native::State::get(_mrb);
		throw euler::util::ArgumentError(state->mrb(),
		    "keyword argument with non symbol keys");
	}
}

mrb_value
RubyState::hash_clear(mrb_value hash)
{
	return WRAP_CALL(mrb_hash_clear(_mrb, hash));
}

mrb_value
RubyState::hash_delete_key(mrb_value hash, mrb_value key)
{
	return WRAP_CALL(mrb_hash_delete_key(_mrb, hash, key));
}

mrb_value
RubyState::hash_dup(mrb_value hash)
{
	return WRAP_CALL(mrb_hash_dup(_mrb, hash));
}

mrb_bool
RubyState::hash_empty_p(mrb_value self)
{
	return WRAP_CALL(mrb_hash_empty_p(_mrb, self));
}

mrb_value
RubyState::hash_fetch(mrb_value hash, mrb_value key, mrb_value def)
{
	return WRAP_CALL(mrb_hash_fetch(_mrb, hash, key, def));
}

void
RubyState::hash_foreach(RHash *hash, mrb_hash_foreach_func *func, void *p)
{
	WRAP_CALL(mrb_hash_foreach(_mrb, hash, func, p));
}

mrb_value
RubyState::hash_get(mrb_value hash, mrb_value key)
{
	return WRAP_CALL(mrb_hash_get(_mrb, hash, key));
}

mrb_bool
RubyState::hash_key_p(mrb_value hash, mrb_value key)
{
	return WRAP_CALL(mrb_hash_key_p(_mrb, hash, key));
}

mrb_value
RubyState::hash_keys(mrb_value hash)
{
	return WRAP_CALL(mrb_hash_keys(_mrb, hash));
}

void
RubyState::hash_merge(mrb_value hash1, mrb_value hash2)
{
	WRAP_CALL(mrb_hash_merge(_mrb, hash1, hash2));
}

mrb_value
RubyState::hash_new()
{
	return WRAP_CALL(mrb_hash_new(_mrb));
}

mrb_value
RubyState::hash_new_capa(mrb_int capa)
{
	return WRAP_CALL(mrb_hash_new_capa(_mrb, capa));
}

void
RubyState::hash_set(mrb_value hash, mrb_value key, mrb_value val)
{
	WRAP_CALL(mrb_hash_set(_mrb, hash, key, val));
}

mrb_int
RubyState::hash_size(mrb_value hash)
{
	return WRAP_CALL(mrb_hash_size(_mrb, hash));
}

mrb_value
RubyState::hash_values(mrb_value hash)
{
	return WRAP_CALL(mrb_hash_values(_mrb, hash));
}

void
RubyState::include_module(RClass *cla, RClass *included)
{
	WRAP_CALL(mrb_include_module(_mrb, cla, included));
}

void
RubyState::incremental_gc()
{
	WRAP_CALL(mrb_incremental_gc(_mrb));
}

mrb_value
RubyState::inspect(mrb_value obj)
{
	return WRAP_CALL(mrb_inspect(_mrb, obj));
}

mrb_value
RubyState::instance_new(mrb_value cv)
{
	return WRAP_CALL(mrb_instance_new(_mrb, cv));
}

mrb_sym
RubyState::intern(const char *str, size_t len)
{
	return WRAP_CALL(mrb_intern(_mrb, str, len));
}

mrb_sym
RubyState::intern_check(const char *str, size_t len)
{
	return WRAP_CALL(mrb_intern_check(_mrb, str, len));
}

mrb_sym
RubyState::intern_check_cstr(const char *str)
{
	return WRAP_CALL(mrb_intern_check_cstr(_mrb, str));
}

mrb_sym
RubyState::intern_check_str(mrb_value str)
{
	return WRAP_CALL(mrb_intern_check_str(_mrb, str));
}

mrb_sym
RubyState::intern_cstr(const char *str)
{
	return WRAP_CALL(mrb_intern_cstr(_mrb, str));
}

mrb_sym
RubyState::intern_static(const char *str, size_t len)
{
	return WRAP_CALL(mrb_intern_static(_mrb, str, len));
}

mrb_sym
RubyState::intern_str(mrb_value str)
{
	return WRAP_CALL(mrb_intern_str(_mrb, str));
}

void
RubyState::iv_copy(mrb_value dst, mrb_value src)
{
	WRAP_CALL(mrb_iv_copy(_mrb, dst, src));
}

mrb_bool
RubyState::iv_defined(mrb_value obj, mrb_sym sym)
{
	return WRAP_CALL(mrb_iv_defined(_mrb, obj, sym));
}

void
RubyState::iv_foreach(mrb_value obj, mrb_iv_foreach_func *func, void *p)
{
	WRAP_CALL(mrb_iv_foreach(_mrb, obj, func, p));
}

mrb_value
RubyState::iv_get(mrb_value obj, mrb_sym sym)
{
	return WRAP_CALL(mrb_iv_get(_mrb, obj, sym));
}

void
RubyState::iv_name_sym_check(mrb_sym sym)
{
	WRAP_CALL(mrb_iv_name_sym_check(_mrb, sym));
}

mrb_bool
RubyState::iv_name_sym_p(mrb_sym sym)
{
	return WRAP_CALL(mrb_iv_name_sym_p(_mrb, sym));
}

mrb_value
RubyState::iv_remove(mrb_value obj, mrb_sym sym)
{
	return WRAP_CALL(mrb_iv_remove(_mrb, obj, sym));
}

void
RubyState::iv_set(mrb_value obj, mrb_sym sym, mrb_value v)
{
	WRAP_CALL(mrb_iv_set(_mrb, obj, sym, v));
}

mrb_value
RubyState::load_detect_file_cxt(FILE *fp, mrb_ccontext *c)
{
	return WRAP_CALL(mrb_load_detect_file_cxt(_mrb, fp, c));
}

mrb_value
RubyState::load_exec(mrb_parser_state *p, mrb_ccontext *c)
{
	return WRAP_CALL(mrb_load_exec(_mrb, p, c));
}

mrb_value
RubyState::load_file(FILE *file)
{
	return WRAP_CALL(mrb_load_file(_mrb, file));
}

mrb_value
RubyState::load_file_cxt(FILE *file, mrb_ccontext *cxt)
{
	return WRAP_CALL(mrb_load_file_cxt(_mrb, file, cxt));
}

mrb_value
RubyState::load_irep(const uint8_t *data)
{
	return WRAP_CALL(mrb_load_irep(_mrb, data));
}

mrb_value
RubyState::load_irep_buf(const void *data, size_t len)
{
	return WRAP_CALL(mrb_load_irep_buf(_mrb, data, len));
}

mrb_value
RubyState::load_irep_buf_cxt(const void *data, size_t len, mrb_ccontext *cxt)
{
	return WRAP_CALL(mrb_load_irep_buf_cxt(_mrb, data, len, cxt));
}

mrb_value
RubyState::load_irep_cxt(const uint8_t *data, mrb_ccontext *cxt)
{
	return WRAP_CALL(mrb_load_irep_cxt(_mrb, data, cxt));
}

mrb_value
RubyState::load_irep_file(FILE *file)
{
	return WRAP_CALL(mrb_load_irep_file(_mrb, file));
}

mrb_value
RubyState::load_irep_file_cxt(FILE *file, mrb_ccontext *cxt)
{
	return WRAP_CALL(mrb_load_irep_file_cxt(_mrb, file, cxt));
}

mrb_value
RubyState::load_nstring(const char *s, size_t len)
{
	return WRAP_CALL(mrb_load_nstring(_mrb, s, len));
}

mrb_value
RubyState::load_nstring_cxt(const char *s, size_t len, mrb_ccontext *cxt)
{
	return WRAP_CALL(mrb_load_nstring_cxt(_mrb, s, len, cxt));
}

mrb_value
RubyState::load_proc(const RProc *proc)
{
	return WRAP_CALL(mrb_load_proc(_mrb, proc));
}

mrb_value
RubyState::load_string(const char *s)
{
	return WRAP_CALL(mrb_load_string(_mrb, s));
}

mrb_value
RubyState::load_string_cxt(const char *s, mrb_ccontext *cxt)
{
	return WRAP_CALL(mrb_load_string_cxt(_mrb, s, cxt));
}

void *
RubyState::malloc(size_t size)
{
	return WRAP_CALL(mrb_malloc(_mrb, size));
}

void *
RubyState::malloc_simple(size_t size)
{
	return WRAP_CALL(mrb_malloc_simple(_mrb, size));
}

mrb_method_t
RubyState::method_search(RClass *cl, mrb_sym sym)
{
	return WRAP_CALL(mrb_method_search(_mrb, cl, sym));
}

mrb_method_t
RubyState::method_search_vm(RClass **ptr, mrb_sym sym)
{
	return WRAP_CALL(mrb_method_search_vm(_mrb, ptr, sym));
}

void
RubyState::mod_cv_set(RClass *c, mrb_sym sym, mrb_value v)
{
	WRAP_CALL(mrb_mod_cv_set(_mrb, c, sym, v));
}

RClass *
RubyState::module_get_id(mrb_sym name)
{
	return WRAP_CALL(mrb_module_get_id(_mrb, name));
}

RClass *
RubyState::module_get_under_id(RClass *outer, mrb_sym name)
{
	return WRAP_CALL(mrb_module_get_under_id(_mrb, outer, name));
}

RClass *
RubyState::module_new()
{
	return WRAP_CALL(mrb_module_new(_mrb));
}

void
RubyState::mt_foreach(RClass *cls, mrb_mt_foreach_func *fn, void *ptr)
{
	WRAP_CALL(mrb_mt_foreach(_mrb, cls, fn, ptr));
}

void
RubyState::name_error(mrb_sym id, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto value = vformat(fmt, ap);
	va_end(ap);
	const char *msg = string_cstr(value);
	WRAP_CALL(mrb_name_error(_mrb, id, "%s", msg));
}

void
RubyState::no_method_error(mrb_sym id, mrb_value args, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto value = vformat(fmt, ap);
	va_end(ap);
	const char *msg = string_cstr(value);
	WRAP_CALL(mrb_no_method_error(_mrb, id, args, "%s", msg));
}

void
RubyState::notimplement()
{
	WRAP_CALL(mrb_notimplement(_mrb));
}

mrb_value
RubyState::notimplement_m(mrb_value m)
{
	return WRAP_CALL(mrb_notimplement_m(_mrb, m));
}

mrb_value
RubyState::num_minus(mrb_value x, mrb_value y)
{
	return WRAP_CALL(mrb_num_minus(_mrb, x, y));
}

mrb_value
RubyState::num_mul(mrb_value x, mrb_value y)
{
	return WRAP_CALL(mrb_num_mul(_mrb, x, y));
}

mrb_value
RubyState::num_plus(mrb_value x, mrb_value y)
{
	return WRAP_CALL(mrb_num_plus(_mrb, x, y));
}

RBasic *
RubyState::obj_alloc(mrb_vtype type, RClass *cls)
{
	return WRAP_CALL(mrb_obj_alloc(_mrb, type, cls));
}

mrb_value
RubyState::obj_as_string(mrb_value obj)
{
	return WRAP_CALL(mrb_obj_as_string(_mrb, obj));
}

RClass *
RubyState::obj_class(mrb_value obj)
{
	return WRAP_CALL(mrb_obj_class(_mrb, obj));
}

const char *
RubyState::obj_classname(mrb_value obj)
{
	return WRAP_CALL(mrb_obj_classname(_mrb, obj));
}

mrb_value
RubyState::obj_clone(mrb_value self)
{
	return WRAP_CALL(mrb_obj_clone(_mrb, self));
}

mrb_value
RubyState::obj_dup(mrb_value obj)
{
	return WRAP_CALL(mrb_obj_dup(_mrb, obj));
}

mrb_bool
RubyState::obj_eq(mrb_value a, mrb_value b)
{
	return WRAP_CALL(mrb_obj_eq(_mrb, a, b));
}

mrb_bool
RubyState::obj_equal(mrb_value a, mrb_value b)
{
	return WRAP_CALL(mrb_obj_equal(_mrb, a, b));
}

mrb_value
RubyState::obj_freeze(mrb_value obj)
{
	return WRAP_CALL(mrb_obj_freeze(_mrb, obj));
}

mrb_int
RubyState::obj_id(mrb_value obj)
{
	return WRAP_CALL(mrb_obj_id(obj));
}

mrb_value
RubyState::obj_inspect(mrb_value self)
{
	return WRAP_CALL(mrb_obj_inspect(_mrb, self));
}

mrb_bool
RubyState::obj_is_instance_of(mrb_value obj, RClass *c)
{
	return WRAP_CALL(mrb_obj_is_instance_of(_mrb, obj, c));
}

mrb_bool
RubyState::obj_is_kind_of(mrb_value obj, RClass *c)
{
	return WRAP_CALL(mrb_obj_is_kind_of(_mrb, obj, c));
}

mrb_bool
RubyState::obj_iv_defined(RObject *obj, mrb_sym sym)
{
	return WRAP_CALL(mrb_obj_iv_defined(_mrb, obj, sym));
}

mrb_value
RubyState::obj_iv_get(RObject *obj, mrb_sym sym)
{
	return WRAP_CALL(mrb_obj_iv_get(_mrb, obj, sym));
}

void
RubyState::obj_iv_set(RObject *obj, mrb_sym sym, mrb_value v)
{
	WRAP_CALL(mrb_obj_iv_set(_mrb, obj, sym, v));
}

mrb_value
RubyState::obj_new(RClass *c, mrb_int argc, const mrb_value *argv)
{
	return WRAP_CALL(mrb_obj_new(_mrb, c, argc, argv));
}

mrb_bool
RubyState::obj_respond_to(RClass *c, mrb_sym mid)
{
	return WRAP_CALL(mrb_obj_respond_to(_mrb, c, mid));
}

mrb_sym
RubyState::obj_to_sym(mrb_value name)
{
	return WRAP_CALL(mrb_obj_to_sym(_mrb, name));
}

mrb_bool
RubyState::object_dead_p(RBasic *object)
{
	return WRAP_CALL(mrb_object_dead_p(_mrb, object));
}

void
RubyState::p(mrb_value val)
{
	WRAP_CALL(mrb_p(_mrb, val));
}

mrb_parser_state *
RubyState::parse_file(FILE *file, mrb_ccontext *ctx)
{
	return WRAP_CALL(mrb_parse_file(_mrb, file, ctx));
}

mrb_parser_state *
RubyState::parse_nstring(const char *str, size_t len, mrb_ccontext *ctx)
{
	return WRAP_CALL(mrb_parse_nstring(_mrb, str, len, ctx));
}

mrb_parser_state *
RubyState::parse_string(const char *str, mrb_ccontext *ctx)
{
	return WRAP_CALL(mrb_parse_string(_mrb, str, ctx));
}

void
RubyState::parser_free(mrb_parser_state *p)
{
	WRAP_CALL(mrb_parser_free(p));
}

mrb_sym
RubyState::parser_get_filename(mrb_parser_state *p, uint16_t idx)
{
	return WRAP_CALL(mrb_parser_get_filename(p, idx));
}

mrb_parser_state *
RubyState::parser_new()
{
	return WRAP_CALL(mrb_parser_new(_mrb));
}

void
RubyState::parser_parse(mrb_parser_state *p, mrb_ccontext *ctx)
{
	WRAP_CALL(mrb_parser_parse(p, ctx));
}

void
RubyState::parser_set_filename(mrb_parser_state *p, const char *str)
{
	WRAP_CALL(mrb_parser_set_filename(p, str));
}

void
RubyState::prepend_module(RClass *cla, RClass *prepended)
{
	WRAP_CALL(mrb_prepend_module(_mrb, cla, prepended));
}

void
RubyState::print_backtrace()
{
	WRAP_CALL(mrb_print_backtrace(_mrb));
}

void
RubyState::print_error()
{
	WRAP_CALL(mrb_print_error(_mrb));
}

mrb_value
RubyState::proc_cfunc_env_get(mrb_int idx)
{
	return WRAP_CALL(mrb_proc_cfunc_env_get(_mrb, idx));
}

RProc *
RubyState::proc_new_cfunc(mrb_func_t func)
{
	return WRAP_CALL(mrb_proc_new_cfunc(_mrb, func));
}

RProc *
RubyState::proc_new_cfunc_with_env(mrb_func_t func, mrb_int argc,
    const mrb_value *argv)
{
	return WRAP_CALL(mrb_proc_new_cfunc_with_env(_mrb, func, argc, argv));
}

mrb_value
RubyState::protect(mrb_func_t body, mrb_value data, mrb_bool *state)
{
	return WRAP_CALL(mrb_protect(_mrb, body, data, state));
}

mrb_value
RubyState::ptr_to_str(void *p)
{
	return WRAP_CALL(mrb_ptr_to_str(_mrb, p));
}

enum mrb_range_beg_len
RubyState::range_beg_len(mrb_value range, mrb_int *begp, mrb_int *lenp,
    mrb_int len, mrb_bool trunc)
{
	return WRAP_CALL(
	    mrb_range_beg_len(_mrb, range, begp, lenp, len, trunc));
}

mrb_value
RubyState::range_new(mrb_value start, mrb_value end, mrb_bool exclude)
{
	return WRAP_CALL(mrb_range_new(_mrb, start, end, exclude));
}

RRange *
RubyState::range_ptr(mrb_value range)
{
	return WRAP_CALL(mrb_range_ptr(_mrb, range));
}

mrb_irep *
RubyState::read_irep(const uint8_t *data)
{
	return WRAP_CALL(mrb_read_irep(_mrb, data));
}

mrb_irep *
RubyState::read_irep_buf(const void *data, size_t len)
{
	return WRAP_CALL(mrb_read_irep_buf(_mrb, data, len));
}

void *
RubyState::realloc(void *ptr, size_t len)
{
	return WRAP_CALL(mrb_realloc(_mrb, ptr, len));
}

void *
RubyState::realloc_simple(void *ptr, size_t len)
{
	return WRAP_CALL(mrb_realloc_simple(_mrb, ptr, len));
}

void
RubyState::remove_method(RClass *c, mrb_sym sym)
{
	WRAP_CALL(mrb_remove_method(_mrb, c, sym));
}

mrb_value
RubyState::rescue(mrb_func_t body, mrb_value b_data, mrb_func_t rescue,
    mrb_value r_data)
{
	return WRAP_CALL(mrb_rescue(_mrb, body, b_data, rescue, r_data));
}

mrb_value
RubyState::rescue_exceptions(mrb_func_t body, mrb_value b_data,
    mrb_func_t rescue, mrb_value r_data, mrb_int len, RClass **classes)
{
	return WRAP_CALL(mrb_rescue_exceptions(_mrb, body, b_data, rescue,
	    r_data, len, classes));
}

mrb_bool
RubyState::respond_to(mrb_value obj, mrb_sym mid)
{
	return WRAP_CALL(mrb_respond_to(_mrb, obj, mid));
}

void
RubyState::show_copyright()
{
	WRAP_CALL(mrb_show_copyright(_mrb));
}

void
RubyState::show_version()
{
	WRAP_CALL(mrb_show_version(_mrb));
}

mrb_value
RubyState::singleton_class(mrb_value val)
{
	return WRAP_CALL(mrb_singleton_class(_mrb, val));
}

RClass *
RubyState::singleton_class_ptr(mrb_value val)
{
	return WRAP_CALL(mrb_singleton_class_ptr(_mrb, val));
}

void
RubyState::stack_extend(mrb_int n)
{
	WRAP_CALL(mrb_stack_extend(_mrb, n));
}

void
RubyState::state_atexit(mrb_atexit_func func)
{
	WRAP_CALL(mrb_state_atexit(_mrb, func));
}

mrb_value
RubyState::str_append(mrb_value str, mrb_value str2)
{
	return WRAP_CALL(mrb_str_append(_mrb, str, str2));
}

mrb_value
RubyState::str_cat(mrb_value str, const char *ptr, size_t len)
{
	return WRAP_CALL(mrb_str_cat(_mrb, str, ptr, len));
}

mrb_value
RubyState::str_cat_cstr(mrb_value str, const char *ptr)
{
	return WRAP_CALL(mrb_str_cat_cstr(_mrb, str, ptr));
}

mrb_value
RubyState::str_cat_str(mrb_value str, mrb_value str2)
{
	return WRAP_CALL(mrb_str_cat_str(_mrb, str, str2));
}

int
RubyState::str_cmp(mrb_value str1, mrb_value str2)
{
	return WRAP_CALL(mrb_str_cmp(_mrb, str1, str2));
}

void
RubyState::str_concat(mrb_value self, mrb_value other)
{
	WRAP_CALL(mrb_str_concat(_mrb, self, other));
}

mrb_value
RubyState::str_dup(mrb_value str)
{
	return WRAP_CALL(mrb_str_dup(_mrb, str));
}

mrb_bool
RubyState::str_equal(mrb_value str1, mrb_value str2)
{
	return WRAP_CALL(mrb_str_equal(_mrb, str1, str2));
}

mrb_int
RubyState::str_index(mrb_value str, const char *p, mrb_int len, mrb_int offset)
{
	return WRAP_CALL(mrb_str_index(_mrb, str, p, len, offset));
}

mrb_value
RubyState::str_intern(mrb_value self)
{
	return WRAP_CALL(mrb_str_intern(_mrb, self));
}

void
RubyState::str_modify(RString *s)
{
	WRAP_CALL(mrb_str_modify(_mrb, s));
}

void
RubyState::str_modify_keep_ascii(RString *s)
{
	WRAP_CALL(mrb_str_modify_keep_ascii(_mrb, s));
}

mrb_value
RubyState::str_new(const char *p, size_t len)
{
	return WRAP_CALL(mrb_str_new(_mrb, p, static_cast<mrb_int>(len)));
}

mrb_value
RubyState::str_new_capa(size_t capa)
{
	return WRAP_CALL(mrb_str_new_capa(_mrb, static_cast<mrb_int>(capa)));
}

mrb_value
RubyState::str_new_static(const char *p, size_t len)
{
	return WRAP_CALL(
	    mrb_str_new_static(_mrb, p, static_cast<mrb_int>(len)));
}

mrb_value
RubyState::str_plus(mrb_value a, mrb_value b)
{
	return WRAP_CALL(mrb_str_plus(_mrb, a, b));
}

mrb_value
RubyState::str_resize(mrb_value str, mrb_int len)
{
	return WRAP_CALL(mrb_str_resize(_mrb, str, len));
}

mrb_int
RubyState::str_strlen(RString *s)
{
	return WRAP_CALL(mrb_str_strlen(_mrb, s));
}

mrb_value
RubyState::str_substr(mrb_value str, mrb_int beg, mrb_int len)
{
	return WRAP_CALL(mrb_str_substr(_mrb, str, beg, len));
}

char *
RubyState::str_to_cstr(mrb_value str)
{
	return WRAP_CALL(mrb_str_to_cstr(_mrb, str));
}

double
RubyState::str_to_dbl(mrb_value str, mrb_bool badcheck)
{
	return WRAP_CALL(mrb_str_to_dbl(_mrb, str, badcheck));
}

mrb_value
RubyState::str_to_integer(mrb_value str, mrb_int base, mrb_bool badcheck)
{
	return WRAP_CALL(mrb_str_to_integer(_mrb, str, base, badcheck));
}

const char *
RubyState::string_cstr(mrb_value str)
{
	return WRAP_CALL(mrb_string_cstr(_mrb, str));
}

mrb_value
RubyState::string_type(mrb_value str)
{
	return WRAP_CALL(mrb_ensure_string_type(_mrb, str));
}

const char *
RubyState::string_value_cstr(mrb_value *str)
{
	return WRAP_CALL(mrb_string_value_cstr(_mrb, str));
}

mrb_int
RubyState::string_value_len(mrb_value str)
{
	return WRAP_CALL(mrb_string_value_len(_mrb, str));
}

const char *
RubyState::string_value_ptr(mrb_value str)
{
	return WRAP_CALL(mrb_string_value_ptr(_mrb, str));
}

const char *
RubyState::sym_dump(mrb_sym sym)
{
	return WRAP_CALL(mrb_sym_dump(_mrb, sym));
}

const char *
RubyState::sym_name(mrb_sym sym)
{
	return WRAP_CALL(mrb_sym_name(_mrb, sym));
}

const char *
RubyState::sym_name_len(mrb_sym sym, mrb_int *len)
{
	return WRAP_CALL(mrb_sym_name_len(_mrb, sym, len));
}

mrb_value
RubyState::sym_str(mrb_sym sym)
{
	return WRAP_CALL(mrb_sym_str(_mrb, sym));
}

void
RubyState::sys_fail(const char *mesg)
{
	WRAP_CALL(mrb_sys_fail(_mrb, mesg));
}

mrb_float
RubyState::to_flo(mrb_value x)
{
	return WRAP_CALL(mrb_as_float(_mrb, x));
}

mrb_value
RubyState::to_int(mrb_value val)
{
	return WRAP_CALL(mrb_to_int(_mrb, val));
}

mrb_value
RubyState::to_str(mrb_value val)
{
	return WRAP_CALL(mrb_to_str(_mrb, val));
}

mrb_value
RubyState::top_run(const RProc *proc, mrb_value self, mrb_int stack_keep)
{
	return WRAP_CALL(mrb_top_run(_mrb, proc, self, stack_keep));
}

mrb_value
RubyState::top_self()
{
	return WRAP_CALL(mrb_top_self(_mrb));
}

mrb_value
RubyState::type_convert(mrb_value val, mrb_vtype type, mrb_sym method)
{
	return WRAP_CALL(mrb_type_convert(_mrb, val, type, method));
}

mrb_value
RubyState::type_convert_check(mrb_value val, mrb_vtype type, mrb_sym method)
{
	return WRAP_CALL(mrb_type_convert_check(_mrb, val, type, method));
}

void
RubyState::undef_class_method(RClass *cls, const char *name)
{
	WRAP_CALL(mrb_undef_class_method(_mrb, cls, name));
}

void
RubyState::undef_class_method_id(RClass *cls, mrb_sym name)
{
	WRAP_CALL(mrb_undef_class_method_id(_mrb, cls, name));
}

void
RubyState::undef_method(RClass *cla, const char *name)
{
	WRAP_CALL(mrb_undef_method(_mrb, cla, name));
}

void
RubyState::undef_method_id(RClass *cla, mrb_sym sym)
{
	WRAP_CALL(mrb_undef_method_id(_mrb, cla, sym));
}

/* ReSharper disable once CppParameterMayBeConst */
mrb_value
RubyState::vformat(const char *format, va_list ap)
{
	return WRAP_CALL(mrb_vformat(_mrb, format, ap));
}

mrb_value
RubyState::vm_const_get(mrb_sym sym)
{
	return WRAP_CALL(mrb_vm_const_get(_mrb, sym));
}

mrb_value
RubyState::vm_cv_get(mrb_sym sym)
{
	return WRAP_CALL(mrb_vm_cv_get(_mrb, sym));
}

void
RubyState::vm_cv_set(mrb_sym sym, mrb_value val)
{
	WRAP_CALL(mrb_vm_cv_set(_mrb, sym, val));
}

RClass *
RubyState::vm_define_class(mrb_value v1, mrb_value v2, mrb_sym sym)
{
	return WRAP_CALL(mrb_vm_define_class(_mrb, v1, v2, sym));
}

RClass *
RubyState::vm_define_module(mrb_value val, mrb_sym sym)
{
	return WRAP_CALL(mrb_vm_define_module(_mrb, val, sym));
}

mrb_value
RubyState::vm_exec(const RProc *proc, const mrb_code *iseq)
{
	return WRAP_CALL(mrb_vm_exec(_mrb, proc, iseq));
}

mrb_value
RubyState::vm_run(const RProc *proc, mrb_value self, mrb_int stack_keep)
{
	return WRAP_CALL(mrb_vm_run(_mrb, proc, self, stack_keep));
}

mrb_value
RubyState::vm_special_get(mrb_sym sym)
{
	return WRAP_CALL(mrb_vm_special_get(_mrb, sym));
}

void
RubyState::vm_special_set(mrb_sym sym, mrb_value val)
{
	WRAP_CALL(mrb_vm_special_set(_mrb, sym, val));
}

void
RubyState::warn(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto value = vformat(fmt, ap);
	va_end(ap);
	const char *msg = string_cstr(value);
	WRAP_CALL(mrb_warn(_mrb, "%s", msg));
}

mrb_value
RubyState::word_boxing_cptr_value(void *ptr)
{
	return WRAP_CALL(mrb_word_boxing_cptr_value(_mrb, ptr));
}

mrb_value
RubyState::word_boxing_float_value(mrb_float f)
{
	return WRAP_CALL(mrb_word_boxing_float_value(_mrb, f));
}

mrb_value
RubyState::word_boxing_int_value(mrb_int i)
{
	return WRAP_CALL(mrb_boxing_int_value(_mrb, i));
}

void
RubyState::write_barrier(RBasic *b)
{
	WRAP_CALL(mrb_write_barrier(_mrb, b));
}

mrb_value
RubyState::yield(mrb_value b, mrb_value arg)
{
	return WRAP_CALL(mrb_yield(_mrb, b, arg));
}

mrb_value
RubyState::yield_argv(mrb_value b, mrb_int argc, const mrb_value *argv)
{
	return WRAP_CALL(mrb_yield_argv(_mrb, b, argc, argv));
}

mrb_value
RubyState::yield_with_class(mrb_value b, mrb_int argc, const mrb_value *argv,
    mrb_value self, RClass *c)
{
	return WRAP_CALL(mrb_yield_with_class(_mrb, b, argc, argv, self, c));
}

mrb_value
RubyState::obj_value(void *p)
{
	return WRAP_CALL(mrb_obj_value(p));
}

mrb_value
RubyState::int_value(mrb_int i)
{
	return WRAP_CALL(mrb_int_value(_mrb, i));
}

mrb_value
RubyState::float_value(mrb_float f)
{
	return WRAP_CALL(mrb_float_value(_mrb, f));
}

mrb_value
RubyState::symbol_value(mrb_sym i)
{
	return WRAP_CALL(mrb_symbol_value(i));
}

RClass *
RubyState::exception()
{
	return exc_get_id(intern_cstr("Error"));
}

RClass *
RubyState::standard_error()
{
	return exc_get_id(intern_cstr("StandardError"));
}

RClass *
RubyState::runtime_error()
{
	return exc_get_id(intern_cstr("RuntimeError"));
}

RClass *
RubyState::type_error()
{
	return exc_get_id(intern_cstr("TypeError"));
}

RClass *
RubyState::zero_division_error()
{
	return exc_get_id(intern_cstr("ZeroDivisionError"));
}

RClass *
RubyState::argument_error()
{
	return exc_get_id(intern_cstr("ArgumentError"));
}

RClass *
RubyState::index_error()
{
	return exc_get_id(intern_cstr("IndexError"));
}

RClass *
RubyState::range_error()
{
	return exc_get_id(intern_cstr("RangeError"));
}

RClass *
RubyState::name_error()
{
	return exc_get_id(intern_cstr("NameError"));
}

RClass *
RubyState::no_method_error()
{
	return exc_get_id(intern_cstr("NoMethodError"));
}

RClass *
RubyState::script_error()
{
	return exc_get_id(intern_cstr("ScriptError"));
}

RClass *
RubyState::syntax_error()
{
	return exc_get_id(intern_cstr("SyntaxError"));
}

RClass *
RubyState::local_jump_error()
{
	return exc_get_id(intern_cstr("LocalJumpError"));
}

RClass *
RubyState::regexp_error()
{
	return exc_get_id(intern_cstr("RegexpError"));
}

RClass *
RubyState::frozen_error()
{
	return exc_get_id(intern_cstr("FrozenError"));
}

RClass *
RubyState::not_implemented_error()
{
	return exc_get_id(intern_cstr("NotImplementedError"));
}

RClass *
RubyState::key_error()
{
	return exc_get_id(intern_cstr("KeyError"));
}

RClass *
RubyState::float_domain_error()
{
	return exc_get_id(intern_cstr("FloatDomainError"));
}

bool
RubyState::block_given_p()
{
	return WRAP_CALL(mrb_block_given_p(_mrb));
}

euler::util::Error::TypeInfo
RubyState::error_type_info(RObject *exc)
{
	using util::Error;
	const auto value = mrb_obj_value(exc);
	if (!mrb_obj_is_kind_of(_mrb, value, exception()))
		throw std::invalid_argument("Not an exception");
	if (mrb_obj_is_kind_of(_mrb, value, script_error())) {
		if (mrb_obj_is_kind_of(_mrb, value, not_implemented_error())) {
			return {
				.kind = Error::Kind::NotImplemented,
				.is_custom = !mrb_obj_is_instance_of(_mrb,
				    value, not_implemented_error()),
			};
		}
		if (mrb_obj_is_kind_of(_mrb, value, syntax_error())) {
			return {
				.kind = Error::Kind::Syntax,
				.is_custom = !mrb_obj_is_instance_of(_mrb,
				    value, syntax_error()),
			};
		}
		return {
			.kind = Error::Kind::Script,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, script_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, index_error())) {
		return {
			.kind = Error::Kind::Index,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, index_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, key_error())) {
		return {
			.kind = Error::Kind::Key,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, key_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, no_method_error())) {
		return {
			.kind = Error::Kind::NoMethod,
			.is_custom = !mrb_obj_is_instance_of(_mrb, value,
			    no_method_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, float_domain_error())) {
		return {
			.kind = Error::Kind::FloatDomain,
			.is_custom = !mrb_obj_is_instance_of(_mrb, value,
			    float_domain_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, frozen_error())) {
		return {
			.kind = Error::Kind::Frozen,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, frozen_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, argument_error())) {
		return {
			.kind = Error::Kind::Argument,
			.is_custom = !mrb_obj_is_instance_of(_mrb, value,
			    argument_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, local_jump_error())) {
		return {
			.kind = Error::Kind::LocalJump,
			.is_custom = !mrb_obj_is_instance_of(_mrb, value,
			    local_jump_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, name_error())) {
		return {
			.kind = Error::Kind::Name,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, name_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, range_error())) {
		return {
			.kind = Error::Kind::Range,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, range_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, regexp_error())) {
		return {
			.kind = Error::Kind::Regexp,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, regexp_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, runtime_error())) {
		return {
			.kind = Error::Kind::Runtime,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, runtime_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, type_error())) {
		return {
			.kind = Error::Kind::Type,
			.is_custom
			= !mrb_obj_is_instance_of(_mrb, value, type_error()),
		};
	}
	if (mrb_obj_is_kind_of(_mrb, value, zero_division_error())) {
		return {
			.kind = Error::Kind::ZeroDivision,
			.is_custom = !mrb_obj_is_instance_of(_mrb, value,
			    zero_division_error()),
		};
	}
	return {
		.kind = Error::Kind::Standard,
		.is_custom
		= !mrb_obj_is_instance_of(_mrb, value, standard_error()),
	};
}

std::string
RubyState::error_cause(RObject *exc)
{
	const auto value = mrb_obj_value(exc);
	const auto sym = intern_cstr("message");
	const auto result = funcall_argv(value, sym, 0, nullptr);
	return string_cstr(result);
}

std::string
RubyState::error_backtrace(RObject *exc)
{
	const auto value = mrb_obj_value(exc);
	const auto sym = intern_cstr("backtrace");
	const auto bt = funcall_argv(value, sym, 0, nullptr);
	if (mrb_nil_p(bt)) return {};
	std::ostringstream ss;
	for (mrb_int i = 0, len = RARRAY_LEN(bt); i < len; ++i) {
		const auto item = ary_entry(bt, i);
		ss << "  " << string_cstr(item) << "\n";
	}
	return ss.str();
}
