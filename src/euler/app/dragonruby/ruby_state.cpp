/* SPDX-License-Identifier: ISC */

#include <cassert>

#include "euler/app/dragonruby/ruby_state.h"

#include <sstream>
#include <vector>

#include <mruby/hash.h>
#include <mruby/istruct.h>

#include "euler/app/dragonruby/state.h"
#include "euler/util/error.h"

using euler::app::dragonruby::RubyState;

static auto
wrap_call(mrb_state *mrb, const auto &func) -> decltype(func())
{
	assert(mrb != nullptr);
	using T = decltype(func());
	auto prev_jmpbuf = mrb->jmp; // save previous jmpbuf
	mrb_jmpbuf new_jmpbuf = {};
	auto state = euler::util::State::get(mrb);
	MRB_TRY(&new_jmpbuf)
	{
		if constexpr (std::is_void_v<T>) {
			func();
			mrb->jmp = prev_jmpbuf; // restore previous jmpbuf
			state->mrb()->raise_on_error();
		} else {
			auto result = func();
			mrb->jmp = prev_jmpbuf; // restore previous jmpbuf
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

RubyState::RubyState(mrb_state *mrb, drb_api_t *api)
    : _mrb(mrb)
    , _api(*api)
{
}

void
RubyState::raise(RClass *c, const char *msg)
{
	WRAP_CALL(_api.mrb_raise(_mrb, c, msg));
}

void
RubyState::raisef(RClass *c, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto value = vformat(fmt, ap);
	va_end(ap);
	const char *message = string_cstr(value);
	WRAP_CALL(raise(c, message));
}

RClass *
RubyState::module_get(const char *name)
{
	return WRAP_CALL(_api.mrb_module_get(_mrb, name));
}

RClass *
RubyState::module_get_under(RClass *outer, const char *name)
{
	return WRAP_CALL(_api.mrb_module_get_under(_mrb, outer, name));
}

RClass *
RubyState::define_module_under(RClass *outer, const char *name)
{
	return WRAP_CALL(_api.mrb_define_module_under(_mrb, outer, name));
}

RClass *
RubyState::class_get_under(RClass *outer, const char *name)
{
	return WRAP_CALL(_api.mrb_class_get_under(_mrb, outer, name));
}

mrb_bool
RubyState::class_ptr_p(const mrb_value obj)
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
	return WRAP_CALL(_api.mrb_define_class_under(_mrb, outer, name, super));
}

void
RubyState::define_module_function(RClass *cla, const char *name,
    const mrb_func_t fun, const mrb_aspec aspec)
{
	return WRAP_CALL(
	    _api.mrb_define_module_function(_mrb, cla, name, fun, aspec));
}

void
RubyState::define_method(RClass *cla, const char *name, const mrb_func_t func,
    const mrb_aspec aspec)
{
	return WRAP_CALL(_api.mrb_define_method(_mrb, cla, name, func, aspec));
}

void
RubyState::define_class_method(RClass *cla, const char *name,
    const mrb_func_t fun, const mrb_aspec aspec)
{
	return WRAP_CALL(
	    _api.mrb_define_class_method(_mrb, cla, name, fun, aspec));
}

mrb_int
RubyState::get_args(const mrb_args_format format, ...)
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

/* Pulled directly from get_args_v in mruby v3.0.0 */
mrb_int
RubyState::get_args_a(mrb_args_format format, void **ptr)
{
	mrb_state *mrb = _mrb;
	const char *fmt = format;
	char c;
	mrb_int i = 0;
	mrb_int argc = _mrb->c->ci->argc;
	mrb_value *array_argv = _mrb->c->ci->stack + 1;
	mrb_bool argv_on_stack = argc >= 0;
	mrb_bool opt = FALSE;
	mrb_bool opt_skip = TRUE;
	mrb_bool given = TRUE;
	mrb_value kdict;
	mrb_bool reqkarg = FALSE;
	int argc_min = 0, argc_max = 0;
	auto state = euler::util::State::get(mrb);

	if (!argv_on_stack) {
		auto a = mrb_ary_ptr(*array_argv);
		array_argv = ARY_PTR(a);
		argc = ARY_LEN(a);
	}

	while ((c = *fmt++)) {
		switch (c) {
		case '|': opt = TRUE; break;
		case '*':
			opt_skip = FALSE;
			argc_max = -1;
			if (!reqkarg) reqkarg = strchr(fmt, ':') ? TRUE : FALSE;
			goto check_exit;
		case '!': break;
		case ':': reqkarg = TRUE;
		/* fall through */
		case '&':
		case '?':
			if (opt) opt_skip = FALSE;
			break;
		default:
			if (!opt) argc_min++;
			argc_max++;
			break;
		}
	}

check_exit:
	if (reqkarg && argc > argc_min
	    && mrb_hash_p(kdict = array_argv[argc - 1])) {
		hash_check_kdict(kdict);
		argc--;
	} else {
		kdict = mrb_nil_value();
	}

	opt = FALSE;
	i = 0;
	while ((c = *format++)) {
		mrb_value *argv = array_argv;
		mrb_bool altmode;

		switch (c) {
		case '|':
		case '*':
		case '&':
		case '?':
		case ':': break;
		default:
			if (argc <= i) {
				if (opt) {
					given = FALSE;
				} else {
					argnum_error(argc, argc_min, argc_max);
				}
			}
			break;
		}

		if (*format == '!') {
			format++;
			altmode = TRUE;
		} else {
			altmode = FALSE;
		}

		switch (c) {
		case 'o': {
			mrb_value *p;

			p = static_cast<mrb_value *>(*ptr++);
			if (i < argc) { *p = argv[i++]; }
		} break;
		case 'C': {
			mrb_value *p;

			p = static_cast<mrb_value *>(*ptr++);
			if (i < argc) {
				mrb_value ss;

				ss = argv[i++];
				if (!class_ptr_p(ss)) {
					raisef(state->mrb()->type_error(),
					    "%v is not class/module", ss);
				}
				*p = ss;
			}
		} break;
		case 'c': {
			RClass **p;

			p = static_cast<struct RClass **>(*ptr++);
			if (i < argc) {
				mrb_value ss;

				ss = argv[i++];
				if (!class_ptr_p(ss)) {
					raisef(state->mrb()->type_error(),
					    "%v is not class/module", ss);
				}
				*p = mrb_class_ptr(ss);
			}
		} break;
		case 'S': {
			mrb_value *p;

			p = static_cast<mrb_value *>(*ptr++);
			if (i < argc) {
				*p = argv[i++];
				if (!(altmode && mrb_nil_p(*p))) {
					_api.mrb_to_str(_mrb, *p);
				}
			}
		} break;
		case 'A': {
			mrb_value *p;

			p = static_cast<mrb_value *>(*ptr++);
			if (i < argc) {
				*p = argv[i++];
				if (!(altmode && mrb_nil_p(*p))) {
					check_type(*p, MRB_TT_ARRAY);
				}
			}
		} break;
		case 'H': {
			mrb_value *p;

			p = static_cast<mrb_value *>(*ptr++);
			if (i < argc) {
				*p = argv[i++];
				if (!(altmode && mrb_nil_p(*p))) {
					check_type(*p, MRB_TT_HASH);
				}
			}
		} break;
		case 's': {
			const char **ps = nullptr;
			mrb_int *pl = nullptr;

			ps = static_cast<const char **>(*ptr++);
			pl = static_cast<mrb_int *>(*ptr++);
			if (i < argc) {
				mrb_value ss;
				ss = argv[i++];
				if (altmode && mrb_nil_p(ss)) {
					*ps = nullptr;
					*pl = 0;
				} else {
					_api.mrb_to_str(_mrb, ss);
					*ps = RSTRING_PTR(ss);
					*pl = RSTRING_LEN(ss);
				}
			}
		} break;
		case 'z': {
			const char **ps;

			ps = static_cast<const char **>(*ptr++);
			if (i < argc) {
				mrb_value ss;
				ss = argv[i++];
				if (altmode && mrb_nil_p(ss)) {
					*ps = nullptr;
				} else {
					to_str(ss);
					*ps = state->mrb()->string_cstr(ss);
				}
			}
		} break;
		case 'a': {
			const mrb_value **pb;
			mrb_int *pl;

			pb = static_cast<const mrb_value **>(*ptr++);
			pl = static_cast<mrb_int *>(*ptr++);
			if (i < argc) {
				mrb_value aa;
				aa = argv[i++];
				if (altmode && mrb_nil_p(aa)) {
					*pb = nullptr;
					*pl = 0;
				} else {
					RArray *a;
					check_type(aa, MRB_TT_ARRAY);
					a = mrb_ary_ptr(aa);
					*pb = ARY_PTR(a);
					*pl = ARY_LEN(a);
				}
			}
		} break;
		case 'I': {
			void **p;

			p = static_cast<void **>(*ptr++);
			if (i < argc) {
				mrb_value ss;
				ss = argv[i++];
				if (!mrb_istruct_p(ss)) {
					raisef(state->mrb()->type_error(),
					    "%v is not inline struct", ss);
				}
				*p = mrb_istruct_ptr(ss);
			}
		} break;
#ifndef MRB_NO_FLOAT
		case 'f': {
			mrb_float *p;

			p = static_cast<mrb_float *>(*ptr++);
			if (i < argc) { *p = to_flo(argv[i++]); }
		} break;
#endif
		case 'i': {
			mrb_int *p;

			p = static_cast<mrb_int *>(*ptr++);
			if (i < argc) { *p = mrb_integer(to_int(argv[i++])); }
		} break;
		case 'b': {
			mrb_bool *boolp = static_cast<mrb_bool *>(*ptr++);

			if (i < argc) {
				mrb_value b = argv[i++];
				*boolp = mrb_test(b);
			}
		} break;
		case 'n': {
			mrb_sym *symp;

			symp = static_cast<mrb_sym *>(*ptr++);
			if (i < argc) {
				mrb_value ss;

				ss = argv[i++];
				*symp = obj_to_sym(ss);
			}
		} break;
		case 'd': {
			void **datap;
			mrb_data_type const *type;

			datap = static_cast<void **>(*ptr++);
			type
			    = static_cast<struct mrb_data_type const *>(*ptr++);
			if (i < argc) {
				mrb_value dd = argv[i++];
				if (altmode && mrb_nil_p(dd)) {
					*datap = nullptr;
				} else {
					*datap = data_get_ptr(dd, type);
				}
			}
		} break;

		case '&': {
			mrb_value *p, *bp;

			p = static_cast<mrb_value *>(*ptr++);
			if (_mrb->c->ci->argc < 0) {
				bp = _mrb->c->ci->stack + 2;
			} else {
				bp = _mrb->c->ci->stack + _mrb->c->ci->argc + 1;
			}
			if (altmode && mrb_nil_p(*bp)) {
				raise(state->mrb()->argument_error(),
				    "no block given");
			}
			*p = *bp;
		} break;
		case '|':
			if (opt_skip && i == argc) goto finish;
			opt = TRUE;
			break;
		case '?': {
			mrb_bool *p;

			p = static_cast<mrb_bool *>(*ptr++);
			*p = given;
		} break;

		case '*': {
			const mrb_value **var;
			mrb_int *pl;
			mrb_bool nocopy
			    = altmode || !argv_on_stack ? TRUE : FALSE;

			var = static_cast<const mrb_value **>(*ptr++);
			pl = static_cast<mrb_int *>(*ptr++);
			if (argc > i) {
				*pl = argc - i;
				if (*pl > 0) {
					if (nocopy) {
						*var = argv + i;
					} else {
						mrb_value args
						    = ary_new_from_values(*pl,
						        argv + i);
						RARRAY(args)->c = nullptr;
						*var = RARRAY_PTR(args);
					}
				}
				i = argc;
			} else {
				*pl = 0;
				*var = nullptr;
			}
		} break;

		case ':': {
			mrb_value ksrc
			    = mrb_hash_p(kdict) ? hash_dup(kdict) : hash_new();
			const mrb_kwargs *kwargs
			    = static_cast<const mrb_kwargs *>(*ptr++);
			mrb_value *rest;
			if (kwargs == nullptr) {
				rest = nullptr;
			} else {
				uint32_t kwnum = kwargs->num;
				uint32_t required = kwargs->required;
				const mrb_sym *kname = kwargs->table;
				mrb_value *values = kwargs->values;
				uint32_t j;
				static constexpr uint32_t keyword_max = 40;

				if (kwnum > keyword_max || required > kwnum) {
					printf("kwnum = %d\nkeyword_max = "
					       "%d\nrequired = %d\n",
					    kwnum, keyword_max, required);
					raise(state->mrb()->argument_error(),
					    "keyword number is too large");
				}

				for (j = required; j > 0;
				    j--, kname++, values++) {
					mrb_value k = symbol_value(*kname);
					if (!hash_key_p(ksrc, k)) {
						raisef(state->mrb()
						           ->argument_error(),
						    "missing keyword: %n",
						    *kname);
					}
					*values = hash_delete_key(ksrc, k);
					gc_protect(*values);
				}

				for (j = kwnum - required; j > 0;
				    j--, kname++, values++) {
					mrb_value k = symbol_value(*kname);
					if (hash_key_p(ksrc, k)) {
						*values
						    = hash_delete_key(ksrc, k);
						gc_protect(*values);
					} else {
						*values = mrb_undef_value();
					}
				}

				rest = kwargs->rest;
			}

			if (rest) {
				*rest = ksrc;
			} else if (!hash_empty_p(ksrc)) {
				ksrc = hash_keys(ksrc);
				ksrc = RARRAY_PTR(ksrc)[0];
				raisef(state->mrb()->argument_error(),
				    "unknown keyword: %v", ksrc);
			}
		} break;

		default:
			raisef(state->mrb()->argument_error(),
			    "invalid argument specifier %c", c);
		}
	}

	// #undef ARGV
	// #undef GET_ARG
	if (!c && argc > i) { argnum_error(argc, argc_min, argc_max); }
finish:
	return i;
}

mrb_value
RubyState::str_new_cstr(const char *str)
{
	return WRAP_CALL(_api.mrb_str_new_cstr(_mrb, str));
}

RData *
RubyState::data_object_alloc(RClass *klass, void *datap,
    const mrb_data_type *type)
{
	return WRAP_CALL(_api.mrb_data_object_alloc(_mrb, klass, datap, type));
}

mrb_value
RubyState::ensure_float_type(const mrb_value val)
{
	return WRAP_CALL(_api.mrb_Float(_mrb, val));
}

mrb_value
RubyState::ensure_integer_type(const mrb_value val)
{
	return WRAP_CALL(_api.mrb_Integer(_mrb, val));
}

mrb_irep *
RubyState::add_irep()
{
	return WRAP_CALL(_api.mrb_add_irep(_mrb));
}

void
RubyState::alias_method(RClass *c, const mrb_sym a, const mrb_sym b)
{
	WRAP_CALL(_api.mrb_alias_method(_mrb, c, a, b));
}

mrb_value
RubyState::any_to_s(const mrb_value obj)
{
	return WRAP_CALL(_api.mrb_any_to_s(_mrb, obj));
}

void
RubyState::argnum_error(const mrb_int argc, const int min, const int max)
{
	WRAP_CALL(_api.mrb_argnum_error(_mrb, argc, min, max));
}

mrb_value
RubyState::ary_clear(const mrb_value self)
{
	return WRAP_CALL(_api.mrb_ary_clear(_mrb, self));
}

void
RubyState::ary_concat(const mrb_value self, const mrb_value other)
{
	WRAP_CALL(_api.mrb_ary_concat(_mrb, self, other));
}

mrb_value
RubyState::ary_entry(const mrb_value ary, const mrb_int offset)
{
	return WRAP_CALL(_api.mrb_ary_entry(ary, offset));
}

mrb_value
RubyState::ary_join(const mrb_value ary, const mrb_value sep)
{
	return WRAP_CALL(_api.mrb_ary_join(_mrb, ary, sep));
}

void
RubyState::ary_modify(RArray *ary)
{
	WRAP_CALL(_api.mrb_ary_modify(_mrb, ary));
}

mrb_value
RubyState::ary_new()
{
	return WRAP_CALL(_api.mrb_ary_new(_mrb));
}

mrb_value
RubyState::ary_new_capa(const mrb_int capa)
{
	return WRAP_CALL(_api.mrb_ary_new_capa(_mrb, capa));
}

mrb_value
RubyState::ary_new_from_values(const mrb_int size, const mrb_value *vals)
{
	return WRAP_CALL(_api.mrb_ary_new_from_values(_mrb, size, vals));
}

mrb_value
RubyState::ary_pop(const mrb_value ary)
{
	return WRAP_CALL(_api.mrb_ary_pop(_mrb, ary));
}

void
RubyState::ary_push(const mrb_value array, const mrb_value value)
{
	WRAP_CALL(_api.mrb_ary_push(_mrb, array, value));
}

mrb_value
RubyState::ary_ref(const mrb_value ary, const mrb_int n)
{
	return WRAP_CALL(_api.mrb_ary_ref(_mrb, ary, n));
}

void
RubyState::ary_replace(const mrb_value self, const mrb_value other)
{
	return WRAP_CALL(_api.mrb_ary_replace(_mrb, self, other));
}

mrb_value
RubyState::ary_resize(const mrb_value ary, const mrb_int new_len)
{
	return WRAP_CALL(_api.mrb_ary_resize(_mrb, ary, new_len));
}

void
RubyState::ary_set(const mrb_value ary, const mrb_int n, const mrb_value val)
{
	return WRAP_CALL(_api.mrb_ary_set(_mrb, ary, n, val));
}

mrb_value
RubyState::ary_shift(const mrb_value self)
{
	return WRAP_CALL(_api.mrb_ary_shift(_mrb, self));
}

mrb_value
RubyState::ary_splat(const mrb_value value)
{
	return WRAP_CALL(_api.mrb_ary_splat(_mrb, value));
}

mrb_value
RubyState::ary_splice(const mrb_value self, const mrb_int head,
    const mrb_int len, const mrb_value rpl)
{
	return WRAP_CALL(_api.mrb_ary_splice(_mrb, self, head, len, rpl));
}

mrb_value
RubyState::ary_unshift(const mrb_value self, const mrb_value item)
{
	return WRAP_CALL(_api.mrb_ary_unshift(_mrb, self, item));
}

mrb_value
RubyState::assoc_new(const mrb_value car, const mrb_value cdr)
{
	return WRAP_CALL(_api.mrb_assoc_new(_mrb, car, cdr));
}

mrb_value
RubyState::attr_get(const mrb_value obj, const mrb_sym id)
{
	return WRAP_CALL(_api.mrb_attr_get(_mrb, obj, id));
}

void
RubyState::bug(const char *fmt, ...)
{
	WRAP_CALL(_api.mrb_bug(_mrb, fmt));
}

void *
RubyState::calloc(const size_t count, const size_t size)
{
	return WRAP_CALL(_api.mrb_calloc(_mrb, count, size));
}

mrb_value
RubyState::check_array_type(const mrb_value self)
{
	return WRAP_CALL(_api.mrb_check_array_type(_mrb, self));
}

mrb_value
RubyState::check_hash_type(const mrb_value hash)
{
	return WRAP_CALL(_api.mrb_check_hash_type(_mrb, hash));
}

mrb_value
RubyState::check_intern(const char *str, const size_t len)
{
	return WRAP_CALL(_api.mrb_check_intern(_mrb, str, len));
}

mrb_value
RubyState::check_intern_cstr(const char *str)
{
	return WRAP_CALL(_api.mrb_check_intern_cstr(_mrb, str));
}

mrb_value
RubyState::check_intern_str(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_check_intern_str(_mrb, str));
}

mrb_value
RubyState::check_string_type(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_check_string_type(_mrb, str));
}

void
RubyState::check_type(const mrb_value x, const mrb_vtype t)
{
	WRAP_CALL(_api.mrb_check_type(_mrb, x, t));
}

mrb_bool
RubyState::class_defined(const char *name)
{
	return WRAP_CALL(_api.mrb_class_defined(_mrb, name));
}

mrb_bool
RubyState::class_defined_id(const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_class_defined_id(_mrb, name));
}

mrb_bool
RubyState::class_defined_under(RClass *outer, const char *name)
{
	return WRAP_CALL(_api.mrb_class_defined_under(_mrb, outer, name));
}

mrb_bool
RubyState::class_defined_under_id(RClass *outer, const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_class_defined_under_id(_mrb, outer, name));
}

RClass *
RubyState::class_get(const char *name)
{
	return WRAP_CALL(_api.mrb_class_get(_mrb, name));
}

RClass *
RubyState::class_get_id(const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_class_get_id(_mrb, name));
}

RClass *
RubyState::class_get_under_id(RClass *outer, const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_class_get_under_id(_mrb, outer, name));
}

const char *
RubyState::class_name(RClass *klass)
{
	return WRAP_CALL(_api.mrb_class_name(_mrb, klass));
}

RClass *
RubyState::class_new(RClass *super)
{
	return WRAP_CALL(_api.mrb_class_new(_mrb, super));
}

mrb_value
RubyState::class_path(RClass *c)
{
	return WRAP_CALL(_api.mrb_class_path(_mrb, c));
}

RClass *
RubyState::class_real(RClass *cl)
{
	return WRAP_CALL(_api.mrb_class_real(cl));
}

void
RubyState::close()
{
	WRAP_CALL(_api.mrb_close(_mrb));
	_mrb = nullptr;
}

RProc *
RubyState::closure_new_cfunc(const mrb_func_t func, const int nlocals)
{
	return WRAP_CALL(_api.mrb_closure_new_cfunc(_mrb, func, nlocals));
}

mrb_int
RubyState::cmp(const mrb_value obj1, const mrb_value obj2)
{
	return WRAP_CALL(_api.mrb_cmp(_mrb, obj1, obj2));
}

mrb_bool
RubyState::const_defined(const mrb_value value, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_const_defined(_mrb, value, sym));
}

mrb_bool
RubyState::const_defined_at(const mrb_value mod, const mrb_sym id)
{
	return WRAP_CALL(_api.mrb_const_defined_at(_mrb, mod, id));
}

mrb_value
RubyState::const_get(const mrb_value value, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_const_get(_mrb, value, sym));
}

void
RubyState::const_remove(const mrb_value value, const mrb_sym sym)
{
	WRAP_CALL(_api.mrb_const_remove(_mrb, value, sym));
}

void
RubyState::const_set(const mrb_value value, const mrb_sym sym,
    const mrb_value new_value)
{
	WRAP_CALL(_api.mrb_const_set(_mrb, value, sym, new_value));
}

double
RubyState::cstr_to_dbl(const char *s, const mrb_bool badcheck)
{
	return WRAP_CALL(_api.mrb_cstr_to_dbl(_mrb, s, badcheck));
}

mrb_value
RubyState::cstr_to_inum(const char *s, const mrb_int base,
    const mrb_bool badcheck)
{
	return WRAP_CALL(_api.mrb_cstr_to_inum(_mrb, s, base, badcheck));
}

mrb_bool
RubyState::cv_defined(const mrb_value mod, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_cv_defined(_mrb, mod, sym));
}

mrb_value
RubyState::cv_get(const mrb_value mod, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_cv_get(_mrb, mod, sym));
}

void
RubyState::cv_set(const mrb_value mod, const mrb_sym sym, const mrb_value v)
{
	WRAP_CALL(_api.mrb_cv_set(_mrb, mod, sym, v));
}

void *
RubyState::data_check_get_ptr(const mrb_value value, const mrb_data_type *type)
{
	return WRAP_CALL(_api.mrb_data_check_get_ptr(_mrb, value, type));
}

void
RubyState::data_check_type(const mrb_value value, const mrb_data_type *type)
{
	WRAP_CALL(_api.mrb_data_check_type(_mrb, value, type));
}

void *
RubyState::data_get_ptr(const mrb_value value, const mrb_data_type *type)
{
	return WRAP_CALL(_api.mrb_data_get_ptr(_mrb, value, type));
}

const char *
RubyState::debug_get_filename(const mrb_irep *irep, const uint32_t pc)
{
	return WRAP_CALL(_api.mrb_debug_get_filename(_mrb, irep, pc));
}

int32_t
RubyState::debug_get_line(const mrb_irep *irep, const uint32_t pc)
{
	return WRAP_CALL(_api.mrb_debug_get_line(_mrb, irep, pc));
}

mrb_irep_debug_info *
RubyState::debug_info_alloc(mrb_irep *irep)
{
	return WRAP_CALL(_api.mrb_debug_info_alloc(_mrb, irep));
}

mrb_irep_debug_info_file *
RubyState::debug_info_append_file(mrb_irep_debug_info *info,
    const char *filename, uint16_t *lines, const uint32_t start_pos,
    const uint32_t end_pos)
{
	return WRAP_CALL(_api.mrb_debug_info_append_file(_mrb, info, filename,
	    lines, start_pos, end_pos));
}

void
RubyState::debug_info_free(mrb_irep_debug_info *d)
{
	WRAP_CALL(_api.mrb_debug_info_free(_mrb, d));
}

void
RubyState::define_alias(RClass *c, const char *a, const char *b)
{
	WRAP_CALL(_api.mrb_define_alias(_mrb, c, a, b));
}

void
RubyState::define_alias_id(RClass *c, const mrb_sym a, const mrb_sym b)
{
	WRAP_CALL(_api.mrb_define_alias_id(_mrb, c, a, b));
}

RClass *
RubyState::define_class(const char *name, RClass *super)
{
	return WRAP_CALL(_api.mrb_define_class(_mrb, name, super));
}

RClass *
RubyState::define_class_id(const mrb_sym name, RClass *super)
{
	return WRAP_CALL(_api.mrb_define_class_id(_mrb, name, super));
}

void
RubyState::define_class_method_id(RClass *cla, const mrb_sym name,
    const mrb_func_t fun, const mrb_aspec aspec)
{
	WRAP_CALL(_api.mrb_define_class_method_id(_mrb, cla, name, fun, aspec));
}

RClass *
RubyState::define_class_under_id(RClass *outer, const mrb_sym name,
    RClass *super)
{
	return WRAP_CALL(
	    _api.mrb_define_class_under_id(_mrb, outer, name, super));
}

void
RubyState::define_const(RClass *cla, const char *name, const mrb_value val)
{
	WRAP_CALL(_api.mrb_define_const(_mrb, cla, name, val));
}

void
RubyState::define_const_id(RClass *cla, const mrb_sym name, const mrb_value val)
{
	WRAP_CALL(_api.mrb_define_const_id(_mrb, cla, name, val));
}

void
RubyState::define_global_const(const char *name, const mrb_value val)
{
	WRAP_CALL(_api.mrb_define_global_const(_mrb, name, val));
}

void
RubyState::define_method_id(RClass *c, const mrb_sym mid, const mrb_func_t func,
    const mrb_aspec aspec)
{
	WRAP_CALL(_api.mrb_define_method_id(_mrb, c, mid, func, aspec));
}

void
RubyState::define_method_raw(RClass *c, const mrb_sym mid,
    const mrb_method_t meth)
{
	WRAP_CALL(_api.mrb_define_method_raw(_mrb, c, mid, meth));
}

RClass *
RubyState::define_module(const char *name)
{
	return WRAP_CALL(_api.mrb_define_module(_mrb, name));
}

void
RubyState::define_module_function_id(RClass *cla, const mrb_sym name,
    const mrb_func_t fun, const mrb_aspec aspec)
{
	WRAP_CALL(
	    _api.mrb_define_module_function_id(_mrb, cla, name, fun, aspec));
}

RClass *
RubyState::define_module_id(const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_define_module_id(_mrb, name));
}

RClass *
RubyState::define_module_under_id(RClass *outer, const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_define_module_under_id(_mrb, outer, name));
}

void
RubyState::define_singleton_method(RObject *cla, const char *name,
    const mrb_func_t fun, const mrb_aspec aspec)
{
	WRAP_CALL(
	    _api.mrb_define_singleton_method(_mrb, cla, name, fun, aspec));
}

void
RubyState::define_singleton_method_id(RObject *cla, const mrb_sym name,
    const mrb_func_t fun, const mrb_aspec aspec)
{
	WRAP_CALL(
	    _api.mrb_define_singleton_method_id(_mrb, cla, name, fun, aspec));
}

mrb_value
RubyState::ensure(const mrb_func_t body, const mrb_value b_data,
    const mrb_func_t ensure, const mrb_value e_data)
{
	return WRAP_CALL(_api.mrb_ensure(_mrb, body, b_data, ensure, e_data));
}

mrb_value
RubyState::ensure_array_type(const mrb_value self)
{
	return WRAP_CALL(_api.mrb_ensure_array_type(_mrb, self));
}

mrb_value
RubyState::ensure_hash_type(const mrb_value hash)
{
	return WRAP_CALL(_api.mrb_ensure_hash_type(_mrb, hash));
}

mrb_value
RubyState::ensure_string_type(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_ensure_string_type(_mrb, str));
}

mrb_bool
RubyState::eql(const mrb_value obj1, const mrb_value obj2)
{
	return WRAP_CALL(_api.mrb_eql(_mrb, obj1, obj2));
}

mrb_bool
RubyState::equal(const mrb_value obj1, const mrb_value obj2)
{
	return WRAP_CALL(_api.mrb_equal(_mrb, obj1, obj2));
}

mrb_value
RubyState::exc_backtrace(const mrb_value exc)
{
	return WRAP_CALL(_api.mrb_exc_backtrace(_mrb, exc));
}

RClass *
RubyState::exc_get_id(const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_exc_get_id(_mrb, name));
}

mrb_value
RubyState::exc_new(RClass *c, const char *ptr, const size_t len)
{
	return WRAP_CALL(_api.mrb_exc_new(_mrb, c, ptr, len));
}

mrb_value
RubyState::exc_new_str(RClass *c, const mrb_value str)
{
	return WRAP_CALL(_api.mrb_exc_new_str(_mrb, c, str));
}

void
RubyState::exc_raise(const mrb_value exc)
{
	WRAP_CALL(_api.mrb_exc_raise(_mrb, exc));
}

mrb_value
RubyState::f_raise(const mrb_value val)
{
	return WRAP_CALL(_api.mrb_f_raise(_mrb, val));
}

mrb_value
RubyState::fiber_alive_p(const mrb_value fib)
{
	return WRAP_CALL(_api.mrb_fiber_alive_p(_mrb, fib));
}

mrb_value
RubyState::fiber_resume(const mrb_value fib, const mrb_int argc,
    const mrb_value *argv)
{
	return WRAP_CALL(_api.mrb_fiber_resume(_mrb, fib, argc, argv));
}

mrb_value
RubyState::fiber_yield(const mrb_int argc, const mrb_value *argv)
{
	return WRAP_CALL(_api.mrb_fiber_yield(_mrb, argc, argv));
}

void
RubyState::field_write_barrier(RBasic *b1, RBasic *b2)
{
	WRAP_CALL(_api.mrb_field_write_barrier(_mrb, b1, b2));
}

mrb_value
RubyState::integer_to_str(mrb_value x, mrb_int base)
{
	return WRAP_CALL(_api.mrb_fixnum_to_str(_mrb, x, base));
}

mrb_value
RubyState::float_to_integer(mrb_value val)
{
	return WRAP_CALL(_api.mrb_flo_to_fixnum(_mrb, val));
}

double
RubyState::float_read(const char *str, char **endptr)
{
	return WRAP_CALL(_api.mrb_float_read(str, endptr));
}

int
RubyState::float_to_cstr(char *buf, const size_t len, const char *fmt,
    const mrb_float f)
{
	return WRAP_CALL(_api.mrb_float_to_cstr(_mrb, buf, len, fmt, f));
}

mrb_value
RubyState::float_to_str(const mrb_value x, const char *fmt)
{
	return WRAP_CALL(_api.mrb_float_to_str(_mrb, x, fmt));
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
	WRAP_CALL(_api.mrb_free(_mrb, ptr));
}

void
RubyState::free_context(mrb_context *c)
{
	WRAP_CALL(_api.mrb_free_context(_mrb, c));
}

void
RubyState::frozen_error(void *frozen_obj)
{
	WRAP_CALL(_api.mrb_frozen_error(_mrb, frozen_obj));
}

void
RubyState::full_gc()
{
	WRAP_CALL(_api.mrb_full_gc(_mrb));
}

mrb_bool
RubyState::func_basic_p(const mrb_value obj, const mrb_sym mid,
    const mrb_func_t func)
{
	return WRAP_CALL(_api.mrb_func_basic_p(_mrb, obj, mid, func));
}

mrb_value
RubyState::funcall(const mrb_value val, const char *name, const mrb_int argc,
    ...)
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
RubyState::funcall_argv(const mrb_value val, const mrb_sym name,
    const mrb_int argc, const mrb_value *argv)
{
	return WRAP_CALL(_api.mrb_funcall_argv(_mrb, val, name, argc, argv));
}

// mrb_value
// RubyState::funcall_id(const mrb_value val, const mrb_sym mid,
//     const mrb_int argc, ...)
// {
// 	va_list ap;
// 	va_start(ap, argc);
// 	std::vector<mrb_value> argv;
// 	argv.reserve(argc);
// 	for (mrb_int i = 0; i < argc; ++i)
// 		argv.push_back(va_arg(ap, mrb_value));
// 	return funcall_argv(val, mid, argc, argv.data());
// }

mrb_value
RubyState::funcall_with_block(const mrb_value val, const mrb_sym name,
    const mrb_int argc, const mrb_value *argv, const mrb_value block)
{
	return WRAP_CALL(
	    _api.mrb_funcall_with_block(_mrb, val, name, argc, argv, block));
}

void
RubyState::garbage_collect()
{
	WRAP_CALL(_api.mrb_garbage_collect(_mrb));
}

void
RubyState::gc_mark(RBasic *obj)
{
	WRAP_CALL(_api.mrb_gc_mark(_mrb, obj));
}

void
RubyState::gc_protect(const mrb_value obj)
{
	WRAP_CALL(_api.mrb_gc_protect(_mrb, obj));
}

void
RubyState::gc_register(const mrb_value obj)
{
	WRAP_CALL(_api.mrb_gc_register(_mrb, obj));
}

void
RubyState::gc_unregister(const mrb_value obj)
{
	WRAP_CALL(_api.mrb_gc_unregister(_mrb, obj));
}

RProc *
RubyState::generate_code(mrb_parser_state *p)
{
	return WRAP_CALL(_api.mrb_generate_code(_mrb, p));
}

mrb_value
RubyState::get_arg1()
{
	return WRAP_CALL(_api.mrb_get_arg1(_mrb));
}

mrb_int
RubyState::get_argc()
{
	return WRAP_CALL(_api.mrb_get_argc(_mrb));
}

const mrb_value *
RubyState::get_argv()
{
	return WRAP_CALL(_api.mrb_get_argv(_mrb));
}

mrb_value
RubyState::get_backtrace()
{
	return WRAP_CALL(_api.mrb_get_backtrace(_mrb));
}

mrb_value
RubyState::gv_get(const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_gv_get(_mrb, sym));
}

void
RubyState::gv_remove(const mrb_sym sym)
{
	WRAP_CALL(_api.mrb_gv_remove(_mrb, sym));
}

void
RubyState::gv_set(const mrb_sym sym, const mrb_value val)
{
	WRAP_CALL(_api.mrb_gv_set(_mrb, sym, val));
}

void
RubyState::hash_check_kdict(const mrb_value self)
{
	const auto keys = hash_keys(self);
	const mrb_int len = RARRAY_LEN(keys);
	for (mrb_int i = 0; i < len; ++i) {
		const auto key = ary_entry(keys, i);
		if (mrb_symbol_p(key)) continue;
		throw util::ArgumentError(state()->mrb(),
		    "keyword argument with non symbol keys");
	}
}

mrb_value
RubyState::hash_clear(const mrb_value hash)
{
	return WRAP_CALL(_api.mrb_hash_clear(_mrb, hash));
}

mrb_value
RubyState::hash_delete_key(const mrb_value hash, const mrb_value key)
{
	return WRAP_CALL(_api.mrb_hash_delete_key(_mrb, hash, key));
}

mrb_value
RubyState::hash_dup(const mrb_value hash)
{
	return WRAP_CALL(_api.mrb_hash_dup(_mrb, hash));
}

mrb_bool
RubyState::hash_empty_p(const mrb_value self)
{
	return WRAP_CALL(_api.mrb_hash_empty_p(_mrb, self));
}

mrb_value
RubyState::hash_fetch(const mrb_value hash, const mrb_value key,
    const mrb_value def)
{
	return WRAP_CALL(_api.mrb_hash_fetch(_mrb, hash, key, def));
}

void
RubyState::hash_foreach(RHash *hash, mrb_hash_foreach_func *func, void *p)
{
	return WRAP_CALL(_api.mrb_hash_foreach(_mrb, hash, func, p));
}

mrb_value
RubyState::hash_get(const mrb_value hash, const mrb_value key)
{
	return WRAP_CALL(_api.mrb_hash_get(_mrb, hash, key));
}

mrb_bool
RubyState::hash_key_p(const mrb_value hash, const mrb_value key)
{
	return WRAP_CALL(_api.mrb_hash_key_p(_mrb, hash, key));
}

mrb_value
RubyState::hash_keys(const mrb_value hash)
{
	return WRAP_CALL(_api.mrb_hash_keys(_mrb, hash));
}

void
RubyState::hash_merge(const mrb_value hash1, const mrb_value hash2)
{
	return WRAP_CALL(_api.mrb_hash_merge(_mrb, hash1, hash2));
}

mrb_value
RubyState::hash_new()
{
	return WRAP_CALL(_api.mrb_hash_new(_mrb));
}

mrb_value
RubyState::hash_new_capa(const mrb_int capa)
{
	return WRAP_CALL(_api.mrb_hash_new_capa(_mrb, capa));
}

void
RubyState::hash_set(const mrb_value hash, const mrb_value key,
    const mrb_value val)
{
	WRAP_CALL(_api.mrb_hash_set(_mrb, hash, key, val));
}

mrb_int
RubyState::hash_size(const mrb_value hash)
{
	return WRAP_CALL(_api.mrb_hash_size(_mrb, hash));
}

mrb_value
RubyState::hash_values(const mrb_value hash)
{
	return WRAP_CALL(_api.mrb_hash_values(_mrb, hash));
}

void
RubyState::include_module(RClass *cla, RClass *included)
{
	WRAP_CALL(_api.mrb_include_module(_mrb, cla, included));
}

void
RubyState::incremental_gc()
{
	WRAP_CALL(_api.mrb_incremental_gc(_mrb));
}

mrb_value
RubyState::inspect(const mrb_value obj)
{
	return WRAP_CALL(_api.mrb_inspect(_mrb, obj));
}

mrb_value
RubyState::instance_new(const mrb_value cv)
{
	return WRAP_CALL(_api.mrb_instance_new(_mrb, cv));
}

mrb_sym
RubyState::intern(const char *str, const size_t len)
{
	return WRAP_CALL(_api.mrb_intern(_mrb, str, len));
}

mrb_sym
RubyState::intern_check(const char *str, const size_t len)
{
	return WRAP_CALL(_api.mrb_intern_check(_mrb, str, len));
}

mrb_sym
RubyState::intern_check_cstr(const char *str)
{
	return WRAP_CALL(_api.mrb_intern_check_cstr(_mrb, str));
}

mrb_sym
RubyState::intern_check_str(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_intern_check_str(_mrb, str));
}

mrb_sym
RubyState::intern_cstr(const char *str)
{
	return WRAP_CALL(_api.mrb_intern_cstr(_mrb, str));
}

mrb_sym
RubyState::intern_static(const char *str, const size_t len)
{
	return WRAP_CALL(_api.mrb_intern_static(_mrb, str, len));
}

mrb_sym
RubyState::intern_str(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_intern_str(_mrb, str));
}

void
RubyState::iv_copy(const mrb_value dst, const mrb_value src)
{
	WRAP_CALL(_api.mrb_iv_copy(_mrb, dst, src));
}

mrb_bool
RubyState::iv_defined(const mrb_value obj, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_iv_defined(_mrb, obj, sym));
}

void
RubyState::iv_foreach(const mrb_value obj, mrb_iv_foreach_func *func, void *p)
{
	WRAP_CALL(_api.mrb_iv_foreach(_mrb, obj, func, p));
}

mrb_value
RubyState::iv_get(const mrb_value obj, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_iv_get(_mrb, obj, sym));
}

void
RubyState::iv_name_sym_check(const mrb_sym sym)
{
	WRAP_CALL(_api.mrb_iv_name_sym_check(_mrb, sym));
}

mrb_bool
RubyState::iv_name_sym_p(const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_iv_name_sym_p(_mrb, sym));
}

mrb_value
RubyState::iv_remove(const mrb_value obj, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_iv_remove(_mrb, obj, sym));
}

void
RubyState::iv_set(const mrb_value obj, const mrb_sym sym, const mrb_value v)
{
	WRAP_CALL(_api.mrb_iv_set(_mrb, obj, sym, v));
}

mrb_value
RubyState::load_detect_file_cxt(FILE *fp, mrbc_context *c)
{
	return WRAP_CALL(_api.mrb_load_detect_file_cxt(_mrb, fp, c));
}

mrb_value
RubyState::load_exec(mrb_parser_state *p, mrbc_context *c)
{
	return WRAP_CALL(_api.mrb_load_exec(_mrb, p, c));
}

mrb_value
RubyState::load_file(FILE *file)
{
	return WRAP_CALL(_api.mrb_load_file(_mrb, file));
}

mrb_value
RubyState::load_file_cxt(FILE *file, mrbc_context *cxt)
{
	return WRAP_CALL(_api.mrb_load_file_cxt(_mrb, file, cxt));
}

mrb_value
RubyState::load_irep(const uint8_t *data)
{
	return WRAP_CALL(_api.mrb_load_irep(_mrb, data));
}

mrb_value
RubyState::load_irep_buf(const void *data, const size_t len)
{
	return WRAP_CALL(_api.mrb_load_irep_buf(_mrb, data, len));
}

mrb_value
RubyState::load_irep_buf_cxt(const void *data, const size_t len,
    mrbc_context *ctx)
{
	return WRAP_CALL(_api.mrb_load_irep_buf_cxt(_mrb, data, len, ctx));
}

mrb_value
RubyState::load_irep_cxt(const uint8_t *data, mrbc_context *ctx)
{
	return WRAP_CALL(_api.mrb_load_irep_cxt(_mrb, data, ctx));
}

mrb_value
RubyState::load_irep_file(FILE *file)
{
	return WRAP_CALL(_api.mrb_load_irep_file(_mrb, file));
}

mrb_value
RubyState::load_irep_file_cxt(FILE *data, mrbc_context *ctx)
{
	return WRAP_CALL(_api.mrb_load_irep_file_cxt(_mrb, data, ctx));
}

mrb_value
RubyState::load_nstring(const char *s, const size_t len)
{
	return WRAP_CALL(_api.mrb_load_nstring(_mrb, s, len));
}

mrb_value
RubyState::load_nstring_cxt(const char *s, const size_t len, mrbc_context *cxt)
{
	return WRAP_CALL(_api.mrb_load_nstring_cxt(_mrb, s, len, cxt));
}

mrb_value
RubyState::load_proc(const RProc *proc)
{
	return WRAP_CALL(_api.mrb_load_proc(_mrb, proc));
}

mrb_value
RubyState::load_string(const char *s)
{
	return WRAP_CALL(_api.mrb_load_string(_mrb, s));
}

mrb_value
RubyState::load_string_cxt(const char *s, mrbc_context *cxt)
{
	return WRAP_CALL(_api.mrb_load_string_cxt(_mrb, s, cxt));
}

void *
RubyState::malloc(const size_t size)
{
	return WRAP_CALL(_api.mrb_malloc(_mrb, size));
}

void *
RubyState::malloc_simple(const size_t size)
{
	return WRAP_CALL(_api.mrb_malloc_simple(_mrb, size));
}

mrb_method_t
RubyState::method_search(RClass *cl, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_method_search(_mrb, cl, sym));
}

mrb_method_t
RubyState::method_search_vm(RClass **ptr, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_method_search_vm(_mrb, ptr, sym));
}

void
RubyState::mod_cv_set(RClass *c, const mrb_sym sym, const mrb_value v)
{
	WRAP_CALL(_api.mrb_mod_cv_set(_mrb, c, sym, v));
}

RClass *
RubyState::module_get_id(const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_module_get_id(_mrb, name));
}

RClass *
RubyState::module_get_under_id(RClass *outer, const mrb_sym name)
{
	return WRAP_CALL(_api.mrb_module_get_under_id(_mrb, outer, name));
}

RClass *
RubyState::module_new()
{
	return WRAP_CALL(_api.mrb_module_new(_mrb));
}

void
RubyState::mt_foreach(RClass *cls, mrb_mt_foreach_func *fn, void *ptr)
{
	WRAP_CALL(_api.mrb_mt_foreach(_mrb, cls, fn, ptr));
}

void
RubyState::name_error(const mrb_sym id, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto message = vformat(fmt, ap);
	va_end(ap);
	WRAP_CALL(_api.mrb_name_error(_mrb, id, string_cstr(message)));
}

void
RubyState::no_method_error(const mrb_sym id, const mrb_value args,
    const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto message = vformat(fmt, ap);
	va_end(ap);
	WRAP_CALL(
	    _api.mrb_no_method_error(_mrb, id, args, string_cstr(message)));
}

void
RubyState::notimplement()
{
	WRAP_CALL(_api.mrb_notimplement(_mrb));
}

mrb_value
RubyState::notimplement_m(const mrb_value m)
{
	return WRAP_CALL(_api.mrb_notimplement_m(_mrb, m));
}

mrb_value
RubyState::num_minus(const mrb_value x, const mrb_value y)
{
	return WRAP_CALL(_api.mrb_num_minus(_mrb, x, y));
}

mrb_value
RubyState::num_mul(const mrb_value x, const mrb_value y)
{
	return WRAP_CALL(_api.mrb_num_mul(_mrb, x, y));
}

mrb_value
RubyState::num_plus(const mrb_value x, const mrb_value y)
{
	return WRAP_CALL(_api.mrb_num_plus(_mrb, x, y));
}

RBasic *
RubyState::obj_alloc(const mrb_vtype type, RClass *cls)
{
	return WRAP_CALL(_api.mrb_obj_alloc(_mrb, type, cls));
}

mrb_value
RubyState::obj_as_string(const mrb_value obj)
{
	return WRAP_CALL(_api.mrb_obj_as_string(_mrb, obj));
}

RClass *
RubyState::obj_class(const mrb_value obj)
{
	return WRAP_CALL(_api.mrb_obj_class(_mrb, obj));
}

const char *
RubyState::obj_classname(const mrb_value obj)
{
	return WRAP_CALL(_api.mrb_obj_classname(_mrb, obj));
}

mrb_value
RubyState::obj_clone(const mrb_value self)
{
	return WRAP_CALL(_api.mrb_obj_clone(_mrb, self));
}

mrb_value
RubyState::obj_dup(const mrb_value obj)
{
	return WRAP_CALL(_api.mrb_obj_dup(_mrb, obj));
}

mrb_bool
RubyState::obj_eq(const mrb_value a, const mrb_value b)
{
	return WRAP_CALL(_api.mrb_obj_eq(_mrb, a, b));
}

mrb_bool
RubyState::obj_equal(const mrb_value a, const mrb_value b)
{
	return WRAP_CALL(_api.mrb_obj_equal(_mrb, a, b));
}

mrb_value
RubyState::obj_freeze(const mrb_value obj)
{
	return WRAP_CALL(_api.mrb_obj_freeze(_mrb, obj));
}

mrb_int
RubyState::obj_id(const mrb_value obj)
{
	return WRAP_CALL(_api.mrb_obj_id(obj));
}

mrb_value
RubyState::obj_inspect(const mrb_value self)
{
	return WRAP_CALL(_api.mrb_obj_inspect(_mrb, self));
}

mrb_bool
RubyState::obj_is_instance_of(const mrb_value obj, RClass *c)
{
	return WRAP_CALL(_api.mrb_obj_is_instance_of(_mrb, obj, c));
}

mrb_bool
RubyState::obj_is_kind_of(const mrb_value obj, RClass *c)
{
	return WRAP_CALL(_api.mrb_obj_is_kind_of(_mrb, obj, c));
}

mrb_bool
RubyState::obj_iv_defined(RObject *obj, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_obj_iv_defined(_mrb, obj, sym));
}

mrb_value
RubyState::obj_iv_get(RObject *obj, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_obj_iv_get(_mrb, obj, sym));
}

void
RubyState::obj_iv_set(RObject *obj, const mrb_sym sym, const mrb_value v)
{
	WRAP_CALL(_api.mrb_obj_iv_set(_mrb, obj, sym, v));
}

mrb_value
RubyState::obj_new(RClass *c, const mrb_int argc, const mrb_value *argv)
{
	return WRAP_CALL(_api.mrb_obj_new(_mrb, c, argc, argv));
}

mrb_bool
RubyState::obj_respond_to(RClass *c, const mrb_sym mid)
{
	return WRAP_CALL(_api.mrb_obj_respond_to(_mrb, c, mid));
}

mrb_sym
RubyState::obj_to_sym(const mrb_value name)
{
	return WRAP_CALL(_api.mrb_obj_to_sym(_mrb, name));
}

mrb_bool
RubyState::object_dead_p(RBasic *object)
{
	return WRAP_CALL(_api.mrb_object_dead_p(_mrb, object));
}

void
RubyState::p(const mrb_value str)
{
	WRAP_CALL(_api.mrb_p(_mrb, str));
}

mrb_parser_state *
RubyState::parse_file(FILE *file, mrbc_context *ctx)
{
	return WRAP_CALL(_api.mrb_parse_file(_mrb, file, ctx));
}

mrb_parser_state *
RubyState::parse_nstring(const char *str, const size_t len, mrbc_context *ctx)
{
	return WRAP_CALL(_api.mrb_parse_nstring(_mrb, str, len, ctx));
}

mrb_parser_state *
RubyState::parse_string(const char *str, mrbc_context *ctx)
{
	return WRAP_CALL(_api.mrb_parse_string(_mrb, str, ctx));
}

void
RubyState::parser_free(mrb_parser_state *ctx)
{
	WRAP_CALL(_api.mrb_parser_free(ctx));
}

mrb_sym
RubyState::parser_get_filename(mrb_parser_state *p, const uint16_t idx)
{
	return WRAP_CALL(_api.mrb_parser_get_filename(p, idx));
}

mrb_parser_state *
RubyState::parser_new()
{
	return WRAP_CALL(_api.mrb_parser_new(_mrb));
}

void
RubyState::parser_parse(mrb_parser_state *p, mrbc_context *ctx)
{
	WRAP_CALL(_api.mrb_parser_parse(p, ctx));
}

void
RubyState::parser_set_filename(mrb_parser_state *p, const char *str)
{
	WRAP_CALL(_api.mrb_parser_set_filename(p, str));
}

#if 0
void*
RubyState::pool_alloc(mrb_pool* pool, const size_t len)
{
	return WRAP_CALL(_api.mrb_pool_alloc(pool, len));
}

mrb_bool
RubyState::pool_can_realloc(mrb_pool* pool, void* ptr, const size_t len)
{
	return WRAP_CALL(_api.mrb_pool_can_realloc(pool, ptr, len));
}

void
RubyState::pool_close(mrb_pool* poo)
{
	return WRAP_CALL(_api.mrb_pool_close(poo));
}

mrb_pool*
RubyState::pool_open()
{
	return WRAP_CALL(_api.mrb_pool_open(_mrb));
}

void*
RubyState::pool_realloc(mrb_pool* pool, void* ptr, const size_t oldlen,
                        const size_t newlen)
{
	return WRAP_CALL(_api.mrb_pool_realloc(pool, ptr, oldlen, newlen));
}
#endif

void
RubyState::prepend_module(RClass *cla, RClass *prepended)
{
	WRAP_CALL(_api.mrb_prepend_module(_mrb, cla, prepended));
}

void
RubyState::print_backtrace()
{
	WRAP_CALL(_api.mrb_print_backtrace(_mrb));
}

void
RubyState::print_error()
{
	WRAP_CALL(_api.mrb_print_error(_mrb));
}

mrb_value
RubyState::proc_cfunc_env_get(const mrb_int idx)
{
	return WRAP_CALL(_api.mrb_proc_cfunc_env_get(_mrb, idx));
}

RProc *
RubyState::proc_new_cfunc(const mrb_func_t fn)
{
	return WRAP_CALL(_api.mrb_proc_new_cfunc(_mrb, fn));
}

RProc *
RubyState::proc_new_cfunc_with_env(const mrb_func_t func, const mrb_int argc,
    const mrb_value *argv)
{
	return WRAP_CALL(
	    _api.mrb_proc_new_cfunc_with_env(_mrb, func, argc, argv));
}

mrb_value
RubyState::protect(const mrb_func_t body, const mrb_value data, mrb_bool *state)
{
	return WRAP_CALL(_api.mrb_protect(_mrb, body, data, state));
}

mrb_value
RubyState::ptr_to_str(void *p)
{
	return WRAP_CALL(_api.mrb_ptr_to_str(_mrb, p));
}

enum mrb_range_beg_len
RubyState::range_beg_len(const mrb_value range, mrb_int *begp, mrb_int *lenp,
    const mrb_int len, const mrb_bool trunc)
{
	return WRAP_CALL(
	    _api.mrb_range_beg_len(_mrb, range, begp, lenp, len, trunc));
}

mrb_value
RubyState::range_new(const mrb_value start, const mrb_value end,
    const mrb_bool exclude)
{
	return WRAP_CALL(_api.mrb_range_new(_mrb, start, end, exclude));
}

RRange *
RubyState::range_ptr(const mrb_value range)
{
	return WRAP_CALL(_api.mrb_range_ptr(_mrb, range));
}

mrb_irep *
RubyState::read_irep(const uint8_t *data)
{
	return WRAP_CALL(_api.mrb_read_irep(_mrb, data));
}

mrb_irep *
RubyState::read_irep_buf(const void *data, const size_t len)
{
	return WRAP_CALL(_api.mrb_read_irep_buf(_mrb, data, len));
}

void *
RubyState::realloc(void *data, const size_t len)
{
	return WRAP_CALL(_api.mrb_realloc(_mrb, data, len));
}

void *
RubyState::realloc_simple(void *data, const size_t len)
{
	return WRAP_CALL(_api.mrb_realloc_simple(_mrb, data, len));
}

void
RubyState::remove_method(RClass *c, const mrb_sym sym)
{
	WRAP_CALL(_api.mrb_remove_method(_mrb, c, sym));
}

mrb_value
RubyState::rescue(const mrb_func_t body, const mrb_value b_data,
    const mrb_func_t rescue, const mrb_value r_data)
{
	return WRAP_CALL(_api.mrb_rescue(_mrb, body, b_data, rescue, r_data));
}

mrb_value
RubyState::rescue_exceptions(const mrb_func_t body, const mrb_value b_data,
    const mrb_func_t rescue, const mrb_value r_data, const mrb_int len,
    RClass **classes)
{
	return WRAP_CALL(_api.mrb_rescue_exceptions(_mrb, body, b_data, rescue,
	    r_data, len, classes));
}

mrb_bool
RubyState::respond_to(const mrb_value obj, const mrb_sym mid)
{
	return WRAP_CALL(_api.mrb_respond_to(_mrb, obj, mid));
}

void
RubyState::show_copyright()
{
	WRAP_CALL(_api.mrb_show_copyright(_mrb));
}

void
RubyState::show_version()
{
	WRAP_CALL(_api.mrb_show_version(_mrb));
}

mrb_value
RubyState::singleton_class(const mrb_value val)
{
	return WRAP_CALL(_api.mrb_singleton_class(_mrb, val));
}

RClass *
RubyState::singleton_class_ptr(const mrb_value val)
{
	return WRAP_CALL(_api.mrb_singleton_class_ptr(_mrb, val));
}

void
RubyState::stack_extend(const mrb_int n)
{
	WRAP_CALL(_api.mrb_stack_extend(_mrb, n));
}

void
RubyState::state_atexit(const mrb_atexit_func func)
{
	WRAP_CALL(_api.mrb_state_atexit(_mrb, func));
}

mrb_value
RubyState::str_append(const mrb_value str, const mrb_value str2)
{
	return WRAP_CALL(_api.mrb_str_append(_mrb, str, str2));
}

mrb_value
RubyState::str_cat(const mrb_value str, const char *ptr, const size_t len)
{
	return WRAP_CALL(_api.mrb_str_cat(_mrb, str, ptr, len));
}

mrb_value
RubyState::str_cat_cstr(const mrb_value str, const char *ptr)
{
	return WRAP_CALL(_api.mrb_str_cat_cstr(_mrb, str, ptr));
}

mrb_value
RubyState::str_cat_str(const mrb_value str, const mrb_value str2)
{
	return WRAP_CALL(_api.mrb_str_cat_str(_mrb, str, str2));
}

int
RubyState::str_cmp(const mrb_value str1, const mrb_value str2)
{
	return WRAP_CALL(_api.mrb_str_cmp(_mrb, str1, str2));
}

void
RubyState::str_concat(const mrb_value self, const mrb_value other)
{
	WRAP_CALL(_api.mrb_str_concat(_mrb, self, other));
}

mrb_value
RubyState::str_dup(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_str_dup(_mrb, str));
}

mrb_bool
RubyState::str_equal(const mrb_value str1, const mrb_value str2)
{
	return WRAP_CALL(_api.mrb_str_equal(_mrb, str1, str2));
}

mrb_int
RubyState::str_index(const mrb_value str, const char *p, const mrb_int len,
    const mrb_int offset)
{
	return WRAP_CALL(_api.mrb_str_index(_mrb, str, p, len, offset));
}

mrb_value
RubyState::str_intern(const mrb_value self)
{
	return WRAP_CALL(_api.mrb_str_intern(_mrb, self));
}

void
RubyState::str_modify(RString *s)
{
	WRAP_CALL(_api.mrb_str_modify(_mrb, s));
}

void
RubyState::str_modify_keep_ascii(RString *s)
{
	WRAP_CALL(_api.mrb_str_modify_keep_ascii(_mrb, s));
}

mrb_value
RubyState::str_new(const char *p, const size_t len)
{
	return WRAP_CALL(_api.mrb_str_new(_mrb, p, len));
}

mrb_value
RubyState::str_new_capa(const size_t capa)
{
	return WRAP_CALL(_api.mrb_str_new_capa(_mrb, capa));
}

mrb_value
RubyState::str_new_static(const char *p, const size_t len)
{
	return WRAP_CALL(_api.mrb_str_new_static(_mrb, p, len));
}

mrb_value
RubyState::str_plus(const mrb_value a, const mrb_value b)
{
	return WRAP_CALL(_api.mrb_str_plus(_mrb, a, b));
}

mrb_value
RubyState::str_resize(const mrb_value str, const mrb_int len)
{
	return WRAP_CALL(_api.mrb_str_resize(_mrb, str, len));
}

mrb_int
RubyState::str_strlen(RString *str)
{
	return WRAP_CALL(_api.mrb_str_strlen(_mrb, str));
}

mrb_value
RubyState::str_substr(const mrb_value str, const mrb_int beg, const mrb_int len)
{
	return WRAP_CALL(_api.mrb_str_substr(_mrb, str, beg, len));
}

char *
RubyState::str_to_cstr(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_str_to_cstr(_mrb, str));
}

double
RubyState::str_to_dbl(const mrb_value str, const mrb_bool badcheck)
{
	return WRAP_CALL(_api.mrb_str_to_dbl(_mrb, str, badcheck));
}

mrb_value
RubyState::str_to_integer(mrb_value str, mrb_int base, mrb_bool badcheck)
{
	return WRAP_CALL(_api.mrb_str_to_inum(_mrb, str, base, badcheck));
}

const char *
RubyState::string_cstr(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_string_cstr(_mrb, str));
}

mrb_value
RubyState::string_type(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_string_type(_mrb, str));
}

const char *
RubyState::string_value_cstr(mrb_value *str)
{
	return WRAP_CALL(_api.mrb_string_value_cstr(_mrb, str));
}

mrb_int
RubyState::string_value_len(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_string_value_len(_mrb, str));
}

const char *
RubyState::string_value_ptr(const mrb_value str)
{
	return WRAP_CALL(_api.mrb_string_value_ptr(_mrb, str));
}

const char *
RubyState::sym_dump(const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_sym_dump(_mrb, sym));
}

const char *
RubyState::sym_name(const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_sym_name(_mrb, sym));
}

const char *
RubyState::sym_name_len(const mrb_sym sym, mrb_int *len)
{
	return WRAP_CALL(_api.mrb_sym_name_len(_mrb, sym, len));
}

mrb_value
RubyState::sym_str(const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_sym_str(_mrb, sym));
}

void
RubyState::sys_fail(const char *mesg)
{
	WRAP_CALL(_api.mrb_sys_fail(_mrb, mesg));
}

mrb_float
RubyState::to_flo(const mrb_value x)
{
	return WRAP_CALL(_api.mrb_to_flo(_mrb, x));
}

mrb_value
RubyState::to_int(const mrb_value val)
{
	return WRAP_CALL(_api.mrb_to_int(_mrb, val));
}

mrb_value
RubyState::to_str(const mrb_value val)
{
	return WRAP_CALL(_api.mrb_to_str(_mrb, val));
}

mrb_value
RubyState::top_run(const RProc *proc, const mrb_value self,
    const mrb_int stack_keep)
{
	return WRAP_CALL(_api.mrb_top_run(_mrb, proc, self, stack_keep));
}

mrb_value
RubyState::top_self()
{
	return WRAP_CALL(_api.mrb_top_self(_mrb));
}

mrb_value
RubyState::type_convert(const mrb_value val, const mrb_vtype type,
    const mrb_sym method)
{
	return WRAP_CALL(_api.mrb_type_convert(_mrb, val, type, method));
}

mrb_value
RubyState::type_convert_check(const mrb_value val, const mrb_vtype type,
    const mrb_sym method)
{
	return WRAP_CALL(_api.mrb_type_convert_check(_mrb, val, type, method));
}

void
RubyState::undef_class_method(RClass *cls, const char *name)
{
	WRAP_CALL(_api.mrb_undef_class_method(_mrb, cls, name));
}

void
RubyState::undef_class_method_id(RClass *cls, const mrb_sym name)
{
	WRAP_CALL(_api.mrb_undef_class_method_id(_mrb, cls, name));
}

void
RubyState::undef_method(RClass *cla, const char *name)
{
	WRAP_CALL(_api.mrb_undef_method(_mrb, cla, name));
}

void
RubyState::undef_method_id(RClass *cla, const mrb_sym name)
{
	WRAP_CALL(_api.mrb_undef_method_id(_mrb, cla, name));
}

/* ReSharper disable once CppParameterMayBeConst */
mrb_value
RubyState::vformat(const char *format, va_list ap)
{
	return WRAP_CALL(_api.mrb_vformat(_mrb, format, ap));
}

mrb_value
RubyState::vm_const_get(const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_vm_const_get(_mrb, sym));
}

mrb_value
RubyState::vm_cv_get(const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_vm_cv_get(_mrb, sym));
}

void
RubyState::vm_cv_set(const mrb_sym sym, const mrb_value val)
{
	WRAP_CALL(_api.mrb_vm_cv_set(_mrb, sym, val));
}

RClass *
RubyState::vm_define_class(const mrb_value v1, const mrb_value v2,
    const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_vm_define_class(_mrb, v1, v2, sym));
}

RClass *
RubyState::vm_define_module(const mrb_value val, const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_vm_define_module(_mrb, val, sym));
}

mrb_value
RubyState::vm_exec(const RProc *proc, const mrb_code *iseq)
{
	return WRAP_CALL(_api.mrb_vm_exec(_mrb, proc, iseq));
}

mrb_value
RubyState::vm_run(const RProc *proc, const mrb_value self,
    const mrb_int stack_keep)
{
	return WRAP_CALL(_api.mrb_vm_run(_mrb, proc, self, stack_keep));
}

mrb_value
RubyState::vm_special_get(const mrb_sym sym)
{
	return WRAP_CALL(_api.mrb_vm_special_get(_mrb, sym));
}

void
RubyState::vm_special_set(const mrb_sym sym, const mrb_value val)
{
	WRAP_CALL(_api.mrb_vm_special_set(_mrb, sym, val));
}

void
RubyState::warn(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const auto message = vformat(fmt, ap);
	va_end(ap);
	WRAP_CALL(_api.mrb_warn(_mrb, string_cstr(message)));
}

mrb_value
RubyState::word_boxing_cptr_value(void *ptr)
{
	return WRAP_CALL(_api.mrb_word_boxing_cptr_value(_mrb, ptr));
}

mrb_value
RubyState::word_boxing_float_value(const mrb_float value)
{
	return WRAP_CALL(_api.mrb_word_boxing_float_value(_mrb, value));
}

mrb_value
RubyState::word_boxing_int_value(const mrb_int value)
{
	return WRAP_CALL(_api.mrb_word_boxing_int_value(_mrb, value));
}

void
RubyState::write_barrier(RBasic *b)
{
	WRAP_CALL(_api.mrb_write_barrier(_mrb, b));
}

mrb_value
RubyState::yield(const mrb_value b, const mrb_value arg)
{
	return WRAP_CALL(_api.mrb_yield(_mrb, b, arg));
}

mrb_value
RubyState::yield_argv(const mrb_value b, const mrb_int argc,
    const mrb_value *argv)
{
	return WRAP_CALL(_api.mrb_yield_argv(_mrb, b, argc, argv));
}

mrb_value
RubyState::yield_with_class(const mrb_value b, const mrb_int argc,
    const mrb_value *argv, const mrb_value self, RClass *c)
{
	return WRAP_CALL(
	    _api.mrb_yield_with_class(_mrb, b, argc, argv, self, c));
}

mrb_value
RubyState::obj_value(void *p)
{
	return WRAP_CALL(_api.mrb_obj_value(p));
}

mrb_value
RubyState::int_value(const mrb_int i)
{
	return WRAP_CALL(_api.mrb_int_value(_mrb, i));
}

mrb_value
RubyState::float_value(const mrb_float f)
{
	return WRAP_CALL(_api.mrb_float_value(_mrb, f));
}

mrb_value
RubyState::symbol_value(const mrb_sym i)
{
	return WRAP_CALL(_api.mrb_symbol_value(i));
}

euler::util::Reference<euler::app::dragonruby::State>
RubyState::state() const
{
	return _state.strengthen();
}

mrb_state *
RubyState::mrb() const
{
	return _mrb;
}

void
RubyState::raise_on_error()
{
	if (_mrb->exc == nullptr) return;
	const auto exc = obj_value(_mrb->exc);
	exc_raise(exc);
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
	return exc_get_id(intern_cstr("RegExpError"));
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

static int
env_bidx(struct REnv *e)
{
	int bidx;

	/* use saved block arg position */
	bidx = MRB_ENV_BIDX(e);
	/* bidx may be useless (e.g. define_method) */
	if (bidx >= MRB_ENV_LEN(e)) return -1;
	return bidx;
}

static mrb_value
mrb_f_block_given_p_m(mrb_state *mrb, mrb_value)
{
	mrb_callinfo *ci = &mrb->c->ci[-1];
	mrb_callinfo *cibase = mrb->c->cibase;
	mrb_value *bp;
	int bidx;
	struct REnv *e = NULL;
	const struct RProc *p;

	if (ci <= cibase) {
		/* toplevel does not have block */
		return mrb_false_value();
	}
	p = ci->proc;
	/* search method/class/module proc */
	while (p) {
		if (MRB_PROC_SCOPE_P(p)) break;
		e = MRB_PROC_ENV(p);
		p = p->upper;
	}
	if (p == NULL) return mrb_false_value();
	if (e) {
		bidx = env_bidx(e);
		if (bidx < 0) return mrb_false_value();
		bp = &e->stack[bidx];
		goto block_given;
	}
	/* search ci corresponding to proc */
	while (cibase < ci) {
		if (ci->proc == p) break;
		ci--;
	}
	if (ci == cibase) {
		/* proc is closure */
		if (!MRB_PROC_ENV_P(p)) return mrb_false_value();
		e = MRB_PROC_ENV(p);
		bidx = env_bidx(e);
		if (bidx < 0) return mrb_false_value();
		bp = &e->stack[bidx];
	} else if ((e = mrb_vm_ci_env(ci)) != NULL) {
		/* top-level does not have block slot (always false) */
		if (e->stack == mrb->c->stbase) return mrb_false_value();
		bidx = env_bidx(e);
		/* bidx may be useless (e.g. define_method) */
		if (bidx < 0) return mrb_false_value();
		bp = &e->stack[bidx];
	} else {
		bp = ci->stack + 1;
		if (ci->argc >= 0) {
			bp += ci->argc;
		} else {
			bp++;
		}
	}
block_given:
	if (mrb_nil_p(*bp)) return mrb_false_value();
	return mrb_true_value();
}

bool
RubyState::block_given_p()
{
	return mrb_bool(mrb_f_block_given_p_m(_mrb, mrb_nil_value()));
}

euler::util::Error::TypeInfo
RubyState::error_type_info(RObject *exc)
{
	using util::Error;
	const auto value = mrb_obj_value(exc);
	if (!_api.mrb_obj_is_kind_of(_mrb, value, exception()))
		throw std::invalid_argument("Not an exception");
	if (_api.mrb_obj_is_kind_of(_mrb, value, script_error())) {
		if (_api.mrb_obj_is_kind_of(_mrb, value,
		        not_implemented_error())) {
			return {
				.kind = Error::Kind::NotImplemented,
				.is_custom = !_api.mrb_obj_is_instance_of(_mrb,
				    value, not_implemented_error()),
			};
		}
		if (_api.mrb_obj_is_kind_of(_mrb, value, syntax_error())) {
			return {
				.kind = Error::Kind::Syntax,
				.is_custom = !_api.mrb_obj_is_instance_of(_mrb,
				    value, syntax_error()),
			};
		}
		return {
			.kind = Error::Kind::Script,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    script_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, index_error())) {
		return {
			.kind = Error::Kind::Index,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    index_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, key_error())) {
		return {
			.kind = Error::Kind::Key,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    key_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, no_method_error())) {
		return {
			.kind = Error::Kind::NoMethod,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    no_method_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, float_domain_error())) {
		return {
			.kind = Error::Kind::FloatDomain,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    float_domain_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, frozen_error())) {
		return {
			.kind = Error::Kind::Frozen,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    frozen_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, argument_error())) {
		return {
			.kind = Error::Kind::Argument,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    argument_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, local_jump_error())) {
		return {
			.kind = Error::Kind::LocalJump,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    local_jump_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, name_error())) {
		return {
			.kind = Error::Kind::Name,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    name_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, range_error())) {

		return {
			.kind = Error::Kind::Range,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    range_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, regexp_error())) {
		return {
			.kind = Error::Kind::Regexp,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    regexp_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, runtime_error())) {
		return {
			.kind = Error::Kind::Runtime,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    runtime_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, type_error())) {
		return {
			.kind = Error::Kind::Type,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    type_error()),
		};
	}
	if (_api.mrb_obj_is_kind_of(_mrb, value, zero_division_error())) {
		return {
			.kind = Error::Kind::ZeroDivision,
			.is_custom = !_api.mrb_obj_is_instance_of(_mrb, value,
			    zero_division_error()),
		};
	}
	return {
		.kind = Error::Kind::Standard,
		.is_custom
		= !_api.mrb_obj_is_instance_of(_mrb, value, standard_error()),
	};
}

static mrb_value
obj_attr_reader(const drb_api_t &_api, mrb_state *mrb, RObject *exc,
    mrb_sym name)
{
	const auto value = _api.mrb_obj_value(exc);
	return _api.mrb_funcall_argv(mrb, value, name, 0, nullptr);
}

static mrb_sym
intern_static(const drb_api_t &_api, mrb_state *mrb, const char *s)
{
	return _api.mrb_intern_cstr(mrb, s);
}

std::string
RubyState::error_cause(RObject *exc)
{
	const auto mrb = _mrb;
	const auto sym = ::intern_static(_api, mrb, "message");
	const auto result = obj_attr_reader(_api, mrb, exc, sym);
	return _api.mrb_string_cstr(_mrb, result);
}

std::string
RubyState::error_backtrace(RObject *exc)
{
	const auto mrb = _mrb;
	const auto sym = ::intern_static(_api, mrb, "backtrace");
	const auto bt = obj_attr_reader(_api, mrb, exc, sym);
	std::stringstream ss;
	if (mrb_nil_p(bt)) return {};
	for (mrb_int i = 0, len = RARRAY_LEN(bt); i < len; ++i) {
		const auto item = _api.mrb_ary_entry(bt, i);
		const auto line = _api.mrb_string_cstr(_mrb, item);
		ss << "  " << line << "\n";
	}
	return ss.str();
}
