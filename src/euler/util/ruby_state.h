/* SPDX-License-Identifier: ISC */

#ifndef EULER_UTIL_MRUBY_STATE_H
#define EULER_UTIL_MRUBY_STATE_H

#include "euler/util/object.h"

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/compile.h>
#include <mruby/debug.h>
#include <mruby/hash.h>
#include <mruby/range.h>
#include <mruby/string.h>
#include <mruby/variable.h>

#include "error.h"

namespace euler::util {
class Error;

/* Note that the implementation of this interface must convert ruby exceptions
 * into C++ exceptions, if mRuby is not built with C++ exceptions enabled (such
 * as in DragonRuby builds) */
class RubyState {
public:
	virtual ~RubyState() = default;
	virtual const mrb_state *mrb() const = 0;
	virtual mrb_state *mrb() = 0;
	virtual void raise_on_error() = 0;
	[[noreturn]] virtual void raise(RClass *c, const char *msg) = 0;
	[[noreturn]] virtual void raisef(RClass *c, const char *fmt, ...) = 0;
	virtual RClass *module_get(const char *name) = 0;
	virtual RClass *module_get_under(RClass *outer, const char *name) = 0;
	virtual RClass *define_module_under(RClass *outer, const char *name)
	    = 0;
	virtual RClass *class_get_under(RClass *outer, const char *name) = 0;
	virtual mrb_bool class_ptr_p(mrb_value obj) = 0;
	virtual RClass *define_class_under(RClass *outer, const char *name,
	    RClass *super)
	    = 0;
	virtual void define_module_function(RClass *cla, const char *name,
	    mrb_func_t fun, mrb_aspec aspec)
	    = 0;
	virtual void define_method(RClass *cla, const char *name,
	    mrb_func_t func, mrb_aspec aspec)
	    = 0;
	virtual void define_class_method(RClass *cla, const char *name,
	    mrb_func_t fun, mrb_aspec aspec)
	    = 0;
	virtual mrb_int get_args(mrb_args_format format, ...) = 0;
	virtual mrb_value str_new_cstr(const char *) = 0;
	virtual RData *data_object_alloc(RClass *klass, void *datap,
	    const mrb_data_type *type)
	    = 0;
	virtual mrb_value ensure_float_type(mrb_value val) = 0;
	virtual mrb_value ensure_integer_type(mrb_value val) = 0;
	virtual mrb_irep *add_irep() = 0;
	virtual void alias_method(RClass *c, mrb_sym a, mrb_sym b) = 0;
	virtual mrb_value any_to_s(mrb_value obj) = 0;
	virtual void argnum_error(mrb_int argc, int min, int max) = 0;
	virtual mrb_value ary_clear(mrb_value self) = 0;
	virtual void ary_concat(mrb_value self, mrb_value other) = 0;
	virtual mrb_value ary_entry(mrb_value ary, mrb_int offset) = 0;
	virtual mrb_value ary_join(mrb_value ary, mrb_value sep) = 0;
	virtual void ary_modify(RArray *) = 0;
	virtual mrb_value ary_new() = 0;
	virtual mrb_value ary_new_capa(mrb_int) = 0;
	virtual mrb_value ary_new_from_values(mrb_int size,
	    const mrb_value *vals)
	    = 0;
	virtual mrb_value ary_pop(mrb_value ary) = 0;
	virtual void ary_push(mrb_value array, mrb_value value) = 0;
	virtual mrb_value ary_ref(mrb_value ary, mrb_int n) = 0;
	virtual void ary_replace(mrb_value self, mrb_value other) = 0;
	virtual mrb_value ary_resize(mrb_value ary, mrb_int new_len) = 0;
	virtual void ary_set(mrb_value ary, mrb_int n, mrb_value val) = 0;
	virtual mrb_value ary_shift(mrb_value self) = 0;
	virtual mrb_value ary_splat(mrb_value value) = 0;
	virtual mrb_value ary_splice(mrb_value self, mrb_int head, mrb_int len,
	    mrb_value rpl)
	    = 0;
	virtual mrb_value ary_unshift(mrb_value self, mrb_value item) = 0;
	virtual mrb_value assoc_new(mrb_value car, mrb_value cdr) = 0;
	virtual mrb_value attr_get(mrb_value obj, mrb_sym id) = 0;
	virtual void bug(const char *fmt, ...) = 0;
	virtual void *calloc(size_t, size_t) = 0;
	virtual mrb_value check_array_type(mrb_value self) = 0;
	virtual mrb_value check_hash_type(mrb_value hash) = 0;
	virtual mrb_value check_intern(const char *, size_t) = 0;
	virtual mrb_value check_intern_cstr(const char *) = 0;
	virtual mrb_value check_intern_str(mrb_value) = 0;
	virtual mrb_value check_string_type(mrb_value str) = 0;
	virtual void check_type(mrb_value x, mrb_vtype t) = 0;
	virtual mrb_bool class_defined(const char *name) = 0;
	virtual mrb_bool class_defined_id(mrb_sym name) = 0;
	virtual mrb_bool class_defined_under(RClass *outer, const char *name)
	    = 0;
	virtual mrb_bool class_defined_under_id(RClass *outer, mrb_sym name)
	    = 0;
	virtual RClass *class_get(const char *name) = 0;
	virtual RClass *class_get_id(mrb_sym name) = 0;
	virtual RClass *class_get_under_id(RClass *outer, mrb_sym name) = 0;
	virtual const char *class_name(RClass *klass) = 0;
	virtual RClass *class_new(RClass *super) = 0;
	virtual mrb_value class_path(RClass *c) = 0;
	virtual RClass *class_real(RClass *cl) = 0;
	virtual void close() = 0;
	virtual RProc *closure_new_cfunc(mrb_func_t func, int nlocals) = 0;
	virtual mrb_int cmp(mrb_value obj1, mrb_value obj2) = 0;
	virtual mrb_bool const_defined(mrb_value, mrb_sym) = 0;
	virtual mrb_bool const_defined_at(mrb_value mod, mrb_sym id) = 0;
	virtual mrb_value const_get(mrb_value, mrb_sym) = 0;
	virtual void const_remove(mrb_value, mrb_sym) = 0;
	virtual void const_set(mrb_value, mrb_sym, mrb_value) = 0;
	virtual double cstr_to_dbl(const char *s, mrb_bool badcheck) = 0;
	virtual mrb_value cstr_to_inum(const char *s, mrb_int base,
	    mrb_bool badcheck)
	    = 0;
	virtual mrb_bool cv_defined(mrb_value mod, mrb_sym sym) = 0;
	virtual mrb_value cv_get(mrb_value mod, mrb_sym sym) = 0;
	virtual void cv_set(mrb_value mod, mrb_sym sym, mrb_value v) = 0;
	virtual void *data_check_get_ptr(mrb_value, const mrb_data_type *) = 0;
	virtual void data_check_type(mrb_value, const mrb_data_type *) = 0;
	virtual void *data_get_ptr(mrb_value, const mrb_data_type *) = 0;
	virtual const char *debug_get_filename(const mrb_irep *irep,
	    uint32_t pc)
	    = 0;
	virtual int32_t debug_get_line(const mrb_irep *irep, uint32_t pc) = 0;
	virtual mrb_irep_debug_info *debug_info_alloc(mrb_irep *irep) = 0;
	virtual mrb_irep_debug_info_file *debug_info_append_file(
	    mrb_irep_debug_info *info, const char *filename, uint16_t *lines,
	    uint32_t start_pos, uint32_t end_pos)
	    = 0;
	virtual void debug_info_free(mrb_irep_debug_info *d) = 0;
	virtual void define_alias(RClass *c, const char *a, const char *b) = 0;
	virtual void define_alias_id(RClass *c, mrb_sym a, mrb_sym b) = 0;
	virtual RClass *define_class(const char *name, RClass *super) = 0;
	virtual RClass *define_class_id(mrb_sym name, RClass *super) = 0;
	virtual void define_class_method_id(RClass *cla, mrb_sym name,
	    mrb_func_t fun, mrb_aspec aspec)
	    = 0;
	virtual RClass *define_class_under_id(RClass *outer, mrb_sym name,
	    RClass *super)
	    = 0;
	virtual void define_const(RClass *cla, const char *name, mrb_value val)
	    = 0;
	virtual void define_const_id(RClass *cla, mrb_sym name, mrb_value val)
	    = 0;
	virtual void define_global_const(const char *name, mrb_value val) = 0;
	virtual void define_method_id(RClass *c, mrb_sym mid, mrb_func_t func,
	    mrb_aspec aspec)
	    = 0;
	virtual void define_method_raw(RClass *, mrb_sym, mrb_method_t) = 0;
	virtual RClass *define_module(const char *name) = 0;
	virtual void define_module_function_id(RClass *cla, mrb_sym name,
	    mrb_func_t fun, mrb_aspec aspec)
	    = 0;
	virtual RClass *define_module_id(mrb_sym name) = 0;
	virtual RClass *define_module_under_id(RClass *outer, mrb_sym name) = 0;
	virtual void define_singleton_method(RObject *cla, const char *name,
	    mrb_func_t fun, mrb_aspec aspec)
	    = 0;
	virtual void define_singleton_method_id(RObject *cla, mrb_sym name,
	    mrb_func_t fun, mrb_aspec aspec)
	    = 0;
	virtual mrb_value ensure(mrb_func_t body, mrb_value b_data,
	    mrb_func_t ensure, mrb_value e_data)
	    = 0;
	virtual mrb_value ensure_array_type(mrb_value self) = 0;
	virtual mrb_value ensure_hash_type(mrb_value hash) = 0;
	virtual mrb_value ensure_string_type(mrb_value str) = 0;
	virtual mrb_bool eql(mrb_value obj1, mrb_value obj2) = 0;
	virtual mrb_bool equal(mrb_value obj1, mrb_value obj2) = 0;
	virtual mrb_value exc_backtrace(mrb_value exc) = 0;
	virtual RClass *exc_get_id(mrb_sym name) = 0;
	virtual mrb_value exc_new(RClass *c, const char *ptr, size_t len) = 0;
	virtual mrb_value exc_new_str(RClass *c, mrb_value str) = 0;
	[[noreturn]] virtual void exc_raise(mrb_value exc) = 0;
	virtual mrb_value f_raise(mrb_value) = 0;
	virtual mrb_value fiber_alive_p(mrb_value fib) = 0;
	virtual mrb_value fiber_resume(mrb_value fib, mrb_int argc,
	    const mrb_value *argv)
	    = 0;
	virtual mrb_value fiber_yield(mrb_int argc, const mrb_value *argv) = 0;
	virtual void field_write_barrier(RBasic *, RBasic *) = 0;
	virtual mrb_value integer_to_str(mrb_value x, mrb_int base) = 0;
	virtual mrb_value float_to_integer(mrb_value val) = 0;
	virtual double float_read(const char *, char **) = 0;
	virtual int float_to_cstr(char *buf, size_t len, const char *fmt,
	    mrb_float f)
	    = 0;
	virtual mrb_value float_to_str(mrb_value x, const char *fmt) = 0;
	virtual mrb_value format(const char *format, ...) = 0;
	virtual void free(void *) = 0;
	virtual void free_context(mrb_context *c) = 0;
	virtual void frozen_error(void *frozen_obj) = 0;
	virtual void full_gc() = 0;
	virtual mrb_bool func_basic_p(mrb_value obj, mrb_sym mid,
	    mrb_func_t func)
	    = 0;
	virtual mrb_value funcall(mrb_value val, const char *name, mrb_int argc,
	    ...)
	    = 0;
	virtual mrb_value funcall_argv(mrb_value val, mrb_sym name,
	    mrb_int argc = 0, const mrb_value *argv = nullptr)
	    = 0;
	// virtual mrb_value funcall_id(mrb_value val, mrb_sym mid, mrb_int
	//     argc, ...)
	//     = 0;
	virtual mrb_value funcall_with_block(mrb_value val, mrb_sym name,
	    mrb_int argc, const mrb_value *argv, mrb_value block)
	    = 0;
	virtual void garbage_collect() = 0;
	virtual void gc_mark(RBasic *) = 0;
	virtual void gc_protect(mrb_value obj) = 0;
	virtual void gc_register(mrb_value obj) = 0;
	virtual void gc_unregister(mrb_value obj) = 0;
	virtual RProc *generate_code(mrb_parser_state *) = 0;
	virtual mrb_value get_arg1() = 0;
	virtual mrb_int get_argc() = 0;
	virtual const mrb_value *get_argv() = 0;
	virtual mrb_int get_args_a(mrb_args_format format, void **ptr) = 0;
	virtual mrb_value get_backtrace() = 0;
	virtual mrb_value gv_get(mrb_sym sym) = 0;
	virtual void gv_remove(mrb_sym sym) = 0;
	virtual void gv_set(mrb_sym sym, mrb_value val) = 0;
	virtual void hash_check_kdict(mrb_value self) = 0;
	virtual mrb_value hash_clear(mrb_value hash) = 0;
	virtual mrb_value hash_delete_key(mrb_value hash, mrb_value key) = 0;
	virtual mrb_value hash_dup(mrb_value hash) = 0;
	virtual mrb_bool hash_empty_p(mrb_value self) = 0;
	virtual mrb_value hash_fetch(mrb_value hash, mrb_value key,
	    mrb_value def)
	    = 0;
	virtual void hash_foreach(RHash *hash, mrb_hash_foreach_func *func,
	    void *p)
	    = 0;
	virtual mrb_value hash_get(mrb_value hash, mrb_value key) = 0;
	virtual mrb_bool hash_key_p(mrb_value hash, mrb_value key) = 0;
	virtual mrb_value hash_keys(mrb_value hash) = 0;
	virtual void hash_merge(mrb_value hash1, mrb_value hash2) = 0;
	virtual mrb_value hash_new() = 0;
	virtual mrb_value hash_new_capa(mrb_int capa) = 0;
	virtual void hash_set(mrb_value hash, mrb_value key, mrb_value val) = 0;
	void
	hash_set(mrb_value hash, const char *key, mrb_value val)
	{
		auto sym = intern_cstr(key);
		hash_set(hash, mrb_symbol_value(sym), val);
	}
	virtual mrb_int hash_size(mrb_value hash) = 0;
	virtual mrb_value hash_values(mrb_value hash) = 0;
	virtual void include_module(RClass *cla, RClass *included) = 0;
	virtual void incremental_gc() = 0;
	virtual mrb_value inspect(mrb_value obj) = 0;
	virtual mrb_value instance_new(mrb_value cv) = 0;
	virtual mrb_sym intern(const char *, size_t) = 0;
	virtual mrb_sym intern_check(const char *, size_t) = 0;
	virtual mrb_sym intern_check_cstr(const char *) = 0;
	virtual mrb_sym intern_check_str(mrb_value) = 0;
	virtual mrb_sym intern_cstr(const char *str) = 0;
	virtual mrb_sym intern_static(const char *, size_t) = 0;
	virtual mrb_sym intern_str(mrb_value) = 0;
	virtual void iv_copy(mrb_value dst, mrb_value src) = 0;
	virtual mrb_bool iv_defined(mrb_value, mrb_sym) = 0;
	virtual void iv_foreach(mrb_value obj, mrb_iv_foreach_func *func,
	    void *p)
	    = 0;
	virtual mrb_value iv_get(mrb_value obj, mrb_sym sym) = 0;
	virtual void iv_name_sym_check(mrb_sym sym) = 0;
	virtual mrb_bool iv_name_sym_p(mrb_sym sym) = 0;
	virtual mrb_value iv_remove(mrb_value obj, mrb_sym sym) = 0;
	virtual void iv_set(mrb_value obj, mrb_sym sym, mrb_value v) = 0;
	virtual mrb_value load_detect_file_cxt(FILE *fp, mrbc_context *c) = 0;
	virtual mrb_value load_exec(mrb_parser_state *p, mrbc_context *c) = 0;
	virtual mrb_value load_file(FILE *) = 0;
	virtual mrb_value load_file_cxt(FILE *, mrbc_context *cxt) = 0;
	virtual mrb_value load_irep(const uint8_t *) = 0;
	virtual mrb_value load_irep_buf(const void *, size_t) = 0;
	virtual mrb_value load_irep_buf_cxt(const void *, size_t,
	    mrbc_context *)
	    = 0;
	virtual mrb_value load_irep_cxt(const uint8_t *, mrbc_context *) = 0;
	virtual mrb_value load_irep_file(FILE *) = 0;
	virtual mrb_value load_irep_file_cxt(FILE *, mrbc_context *) = 0;
	virtual mrb_value load_nstring(const char *s, size_t len) = 0;
	virtual mrb_value load_nstring_cxt(const char *s, size_t len,
	    mrbc_context *cxt)
	    = 0;
	virtual mrb_value load_proc(const RProc *proc) = 0;
	virtual mrb_value load_string(const char *s) = 0;
	virtual mrb_value load_string_cxt(const char *s, mrbc_context *cxt) = 0;
	virtual void *malloc(size_t) = 0;
	virtual void *malloc_simple(size_t) = 0;
	virtual mrb_method_t method_search(RClass *, mrb_sym) = 0;
	virtual mrb_method_t method_search_vm(RClass **, mrb_sym) = 0;
	virtual void mod_cv_set(RClass *c, mrb_sym sym, mrb_value v) = 0;
	virtual RClass *module_get_id(mrb_sym name) = 0;
	virtual RClass *module_get_under_id(RClass *outer, mrb_sym name) = 0;
	virtual RClass *module_new() = 0;
	virtual void mt_foreach(RClass *, mrb_mt_foreach_func *, void *) = 0;
	virtual void name_error(mrb_sym id, const char *fmt, ...) = 0;
	virtual void no_method_error(mrb_sym id, mrb_value args,
	    const char *fmt, ...)
	    = 0;
	virtual void notimplement() = 0;
	virtual mrb_value notimplement_m(mrb_value) = 0;
	virtual mrb_value num_minus(mrb_value x, mrb_value y) = 0;
	virtual mrb_value num_mul(mrb_value x, mrb_value y) = 0;
	virtual mrb_value num_plus(mrb_value x, mrb_value y) = 0;
	virtual RBasic *obj_alloc(mrb_vtype, RClass *) = 0;
	virtual mrb_value obj_as_string(mrb_value obj) = 0;
	virtual RClass *obj_class(mrb_value obj) = 0;
	virtual const char *obj_classname(mrb_value obj) = 0;
	virtual mrb_value obj_clone(mrb_value self) = 0;
	virtual mrb_value obj_dup(mrb_value obj) = 0;
	virtual mrb_bool obj_eq(mrb_value a, mrb_value b) = 0;
	virtual mrb_bool obj_equal(mrb_value a, mrb_value b) = 0;
	virtual mrb_value obj_freeze(mrb_value) = 0;
	virtual mrb_int obj_id(mrb_value obj) = 0;
	virtual mrb_value obj_inspect(mrb_value self) = 0;
	virtual mrb_bool obj_is_instance_of(mrb_value obj, RClass *c) = 0;
	virtual mrb_bool obj_is_kind_of(mrb_value obj, RClass *c) = 0;
	virtual mrb_bool obj_iv_defined(RObject *obj, mrb_sym sym) = 0;
	virtual mrb_value obj_iv_get(RObject *obj, mrb_sym sym) = 0;
	virtual void obj_iv_set(RObject *obj, mrb_sym sym, mrb_value v) = 0;
	virtual mrb_value obj_new(RClass *c, mrb_int argc,
	    const mrb_value *argv)
	    = 0;
	virtual mrb_bool obj_respond_to(RClass *c, mrb_sym mid) = 0;
	virtual mrb_sym obj_to_sym(mrb_value name) = 0;
	virtual mrb_bool object_dead_p(RBasic *object) = 0;
	// virtual mrb_state *open_allocf(mrb_allocf f, void *ud) = 0;
	// virtual mrb_state *open_core(mrb_allocf f, void *ud) = 0;
	virtual void p(mrb_value) = 0;
	virtual mrb_parser_state *parse_file(FILE *, mrbc_context *) = 0;
	virtual mrb_parser_state *parse_nstring(const char *, size_t,
	    mrbc_context *)
	    = 0;
	virtual mrb_parser_state *parse_string(const char *, mrbc_context *)
	    = 0;
	virtual void parser_free(mrb_parser_state *) = 0;
	virtual mrb_sym parser_get_filename(mrb_parser_state *, uint16_t idx)
	    = 0;
	virtual mrb_parser_state *parser_new() = 0;
	virtual void parser_parse(mrb_parser_state *, mrbc_context *) = 0;
	virtual void parser_set_filename(mrb_parser_state *, const char *) = 0;
	virtual void prepend_module(RClass *cla, RClass *prepended) = 0;
	virtual void print_backtrace() = 0;
	virtual void print_error() = 0;
	virtual mrb_value proc_cfunc_env_get(mrb_int idx) = 0;
	virtual RProc *proc_new_cfunc(mrb_func_t) = 0;
	virtual RProc *proc_new_cfunc_with_env(mrb_func_t func, mrb_int argc,
	    const mrb_value *argv)
	    = 0;
	virtual mrb_value protect(mrb_func_t body, mrb_value data,
	    mrb_bool *state)
	    = 0;
	virtual mrb_value ptr_to_str(void *p) = 0;
	virtual enum mrb_range_beg_len range_beg_len(mrb_value range,
	    mrb_int *begp, mrb_int *lenp, mrb_int len, mrb_bool trunc)
	    = 0;
	virtual mrb_value range_new(mrb_value start, mrb_value end,
	    mrb_bool exclude)
	    = 0;
	virtual RRange *range_ptr(mrb_value range) = 0;
	virtual mrb_irep *read_irep(const uint8_t *) = 0;
	virtual mrb_irep *read_irep_buf(const void *, size_t) = 0;
	virtual void *realloc(void *, size_t) = 0;
	virtual void *realloc_simple(void *, size_t) = 0;
	virtual void remove_method(RClass *c, mrb_sym sym) = 0;
	virtual mrb_value rescue(mrb_func_t body, mrb_value b_data,
	    mrb_func_t rescue, mrb_value r_data)
	    = 0;
	virtual mrb_value rescue_exceptions(mrb_func_t body, mrb_value b_data,
	    mrb_func_t rescue, mrb_value r_data, mrb_int len, RClass **classes)
	    = 0;
	virtual mrb_bool respond_to(mrb_value obj, mrb_sym mid) = 0;
	virtual void show_copyright() = 0;
	virtual void show_version() = 0;
	virtual mrb_value singleton_class(mrb_value val) = 0;
	virtual RClass *singleton_class_ptr(mrb_value val) = 0;
	virtual void stack_extend(mrb_int) = 0;
	virtual void state_atexit(mrb_atexit_func func) = 0;
	virtual mrb_value str_append(mrb_value str, mrb_value str2) = 0;
	virtual mrb_value str_cat(mrb_value str, const char *ptr, size_t len)
	    = 0;
	virtual mrb_value str_cat_cstr(mrb_value str, const char *ptr) = 0;
	virtual mrb_value str_cat_str(mrb_value str, mrb_value str2) = 0;
	virtual int str_cmp(mrb_value str1, mrb_value str2) = 0;
	virtual void str_concat(mrb_value self, mrb_value other) = 0;
	virtual mrb_value str_dup(mrb_value str) = 0;
	virtual mrb_bool str_equal(mrb_value str1, mrb_value str2) = 0;
	virtual mrb_int str_index(mrb_value str, const char *p, mrb_int len,
	    mrb_int offset)
	    = 0;
	virtual mrb_value str_intern(mrb_value self) = 0;
	virtual void str_modify(struct RString *s) = 0;
	virtual void str_modify_keep_ascii(RString *s) = 0;
	virtual mrb_value str_new(const char *p, size_t len) = 0;
	virtual mrb_value str_new_capa(size_t capa) = 0;
	virtual mrb_value str_new_static(const char *p, size_t len) = 0;
	virtual mrb_value str_plus(mrb_value a, mrb_value b) = 0;
	virtual mrb_value str_resize(mrb_value str, mrb_int len) = 0;
	virtual mrb_int str_strlen(RString *) = 0;
	virtual mrb_value str_substr(mrb_value str, mrb_int beg, mrb_int len)
	    = 0;
	virtual char *str_to_cstr(mrb_value str) = 0;
	virtual double str_to_dbl(mrb_value str, mrb_bool badcheck) = 0;
	virtual mrb_value str_to_integer(mrb_value str, mrb_int base,
	    mrb_bool badcheck)
	    = 0;
	virtual const char *string_cstr(mrb_value str) = 0;
	virtual mrb_value string_type(mrb_value str) = 0;
	virtual const char *string_value_cstr(mrb_value *str) = 0;
	virtual mrb_int string_value_len(mrb_value str) = 0;
	virtual const char *string_value_ptr(mrb_value str) = 0;
	virtual const char *sym_dump(mrb_sym) = 0;
	virtual const char *sym_name(mrb_sym) = 0;
	virtual const char *sym_name_len(mrb_sym, mrb_int *) = 0;
	virtual mrb_value sym_str(mrb_sym) = 0;
	virtual void sys_fail(const char *mesg) = 0;
	virtual mrb_float to_flo(mrb_value x) = 0;
	virtual mrb_value to_int(mrb_value val) = 0;
	virtual mrb_value to_str(mrb_value val) = 0;
	virtual mrb_value top_run(const RProc *proc, mrb_value self,
	    mrb_int stack_keep)
	    = 0;
	virtual mrb_value top_self() = 0;
	virtual mrb_value type_convert(mrb_value val, mrb_vtype type,
	    mrb_sym method)
	    = 0;
	virtual mrb_value type_convert_check(mrb_value val, mrb_vtype type,
	    mrb_sym method)
	    = 0;
	virtual void undef_class_method(RClass *cls, const char *name) = 0;
	virtual void undef_class_method_id(RClass *cls, mrb_sym name) = 0;
	virtual void undef_method(RClass *cla, const char *name) = 0;
	virtual void undef_method_id(RClass *, mrb_sym) = 0;
	virtual mrb_value vformat(const char *format, va_list ap) = 0;
	virtual mrb_value vm_const_get(mrb_sym) = 0;
	virtual mrb_value vm_cv_get(mrb_sym) = 0;
	virtual void vm_cv_set(mrb_sym, mrb_value) = 0;
	virtual RClass *vm_define_class(mrb_value, mrb_value, mrb_sym) = 0;
	virtual RClass *vm_define_module(mrb_value, mrb_sym) = 0;
	virtual mrb_value vm_exec(const RProc *proc, const mrb_code *iseq) = 0;
	virtual mrb_value vm_run(const RProc *proc, mrb_value self,
	    mrb_int stack_keep)
	    = 0;
	virtual mrb_value vm_special_get(mrb_sym) = 0;
	virtual void vm_special_set(mrb_sym, mrb_value) = 0;
	virtual void warn(const char *fmt, ...) = 0;
	virtual mrb_value word_boxing_cptr_value(void *) = 0;
	virtual mrb_value word_boxing_float_value(mrb_float) = 0;
	virtual mrb_value word_boxing_int_value(mrb_int) = 0;
	virtual void write_barrier(RBasic *) = 0;
	virtual mrb_value yield(mrb_value b, mrb_value arg) = 0;
	virtual mrb_value yield_argv(mrb_value b, mrb_int argc,
	    const mrb_value *argv)
	    = 0;
	virtual mrb_value yield_with_class(mrb_value b, mrb_int argc,
	    const mrb_value *argv, mrb_value self, RClass *c)
	    = 0;
	virtual mrb_value obj_value(void *p) = 0;
	virtual mrb_value int_value(mrb_int i) = 0;
	virtual mrb_value float_value(mrb_float f) = 0;
	virtual mrb_value symbol_value(mrb_sym i) = 0;
	virtual RClass *exception() = 0;
	virtual RClass *standard_error() = 0;
	virtual RClass *runtime_error() = 0;
	virtual RClass *type_error() = 0;
	virtual RClass *zero_division_error() = 0;
	virtual RClass *argument_error() = 0;
	virtual RClass *index_error() = 0;
	virtual RClass *range_error() = 0;
	virtual RClass *name_error() = 0;
	virtual RClass *no_method_error() = 0;
	virtual RClass *script_error() = 0;
	virtual RClass *syntax_error() = 0;
	virtual RClass *local_jump_error() = 0;
	virtual RClass *regexp_error() = 0;
	virtual RClass *frozen_error() = 0;
	virtual RClass *not_implemented_error() = 0;
	virtual RClass *key_error() = 0;
	virtual RClass *float_domain_error() = 0;
	virtual bool block_given_p() = 0;
	virtual Error::TypeInfo error_type_info(RObject *exc) = 0;
	virtual std::string error_cause(RObject *exc) = 0;
	virtual std::string error_backtrace(RObject *exc) = 0;

	template <typename... Args>
	mrb_value
	call(mrb_value block, Args &&...args)
	{
		const mrb_int argc = sizeof...(Args);
		mrb_value argv[] = { std::forward<Args>(args)... };
		return funcall_argv(block, intern_cstr("call"), argc, argv);
	}
};

} /* namespace euler::util */

#endif /* EULER_UTIL_MRUBY_STATE_H */
