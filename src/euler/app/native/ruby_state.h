/* SPDX-License-Identifier: ISC */

#ifndef EULER_APP_NATIVE_RUBY_STATE_H
#define EULER_APP_NATIVE_RUBY_STATE_H

#include <mruby.h>

#include "euler/util/object.h"
#include "euler/util/ruby_state.h"

namespace euler::app::native {
class State;
class RubyState final : public util::RubyState {
	friend class State;

public:
	RubyState();
	~RubyState() override;
	const mrb_state *mrb() const override;
	mrb_state *mrb() override;
	void raise_on_error() override;
	[[noreturn]] void raise(RClass *c, const char *msg) override;
	[[noreturn]] void raisef(RClass *c, const char *fmt, ...) override;
	RClass *module_get(const char *name) override;
	RClass *module_get_under(RClass *outer, const char *name) override;
	RClass *define_module_under(RClass *outer, const char *name) override;
	RClass *class_get_under(RClass *outer, const char *name) override;
	mrb_bool class_ptr_p(mrb_value obj) override;
	RClass *define_class_under(RClass *outer, const char *name,
	    RClass *super) override;
	void define_module_function(RClass *cla, const char *name,
	    mrb_func_t fun, mrb_aspec aspec) override;
	void define_method(RClass *cla, const char *name, mrb_func_t func,
	    mrb_aspec aspec) override;
	void define_class_method(RClass *cla, const char *name, mrb_func_t fun,
	    mrb_aspec aspec) override;
	mrb_int get_args(mrb_args_format format, ...) override;
	mrb_value str_new_cstr(const char *) override;
	RData *data_object_alloc(RClass *klass, void *datap,
	    const mrb_data_type *type) override;
	mrb_value ensure_float_type(mrb_value val) override;
	mrb_value ensure_integer_type(mrb_value val) override;
	mrb_irep *add_irep() override;
	void alias_method(RClass *c, mrb_sym a, mrb_sym b) override;
	mrb_value any_to_s(mrb_value obj) override;
	void argnum_error(mrb_int argc, int min, int max) override;
	mrb_value ary_clear(mrb_value self) override;
	void ary_concat(mrb_value self, mrb_value other) override;
	mrb_value ary_entry(mrb_value ary, mrb_int offset) override;
	mrb_value ary_join(mrb_value ary, mrb_value sep) override;
	void ary_modify(RArray *) override;
	mrb_value ary_new() override;
	mrb_value ary_new_capa(mrb_int) override;
	mrb_value ary_new_from_values(mrb_int size,
	    const mrb_value *vals) override;
	mrb_value ary_pop(mrb_value ary) override;
	void ary_push(mrb_value array, mrb_value value) override;
	mrb_value ary_ref(mrb_value ary, mrb_int n) override;
	void ary_replace(mrb_value self, mrb_value other) override;
	mrb_value ary_resize(mrb_value ary, mrb_int new_len) override;
	void ary_set(mrb_value ary, mrb_int n, mrb_value val) override;
	mrb_value ary_shift(mrb_value self) override;
	mrb_value ary_splat(mrb_value value) override;
	mrb_value ary_splice(mrb_value self, mrb_int head, mrb_int len,
	    mrb_value rpl) override;
	mrb_value ary_unshift(mrb_value self, mrb_value item) override;
	mrb_value assoc_new(mrb_value car, mrb_value cdr) override;
	mrb_value attr_get(mrb_value obj, mrb_sym id) override;
	void bug(const char *fmt, ...) override;
	void *calloc(size_t, size_t) override;
	mrb_value check_array_type(mrb_value self) override;
	mrb_value check_hash_type(mrb_value hash) override;
	mrb_value check_intern(const char *, size_t) override;
	mrb_value check_intern_cstr(const char *) override;
	mrb_value check_intern_str(mrb_value) override;
	mrb_value check_string_type(mrb_value str) override;
	void check_type(mrb_value x, mrb_vtype t) override;
	mrb_bool class_defined(const char *name) override;
	mrb_bool class_defined_id(mrb_sym name) override;
	mrb_bool class_defined_under(RClass *outer, const char *name) override;
	mrb_bool class_defined_under_id(RClass *outer, mrb_sym name) override;
	RClass *class_get(const char *name) override;
	RClass *class_get_id(mrb_sym name) override;
	RClass *class_get_under_id(RClass *outer, mrb_sym name) override;
	const char *class_name(RClass *cls) override;
	RClass *class_new(RClass *super) override;
	mrb_value class_path(RClass *c) override;
	RClass *class_real(RClass *cl) override;
	void close() override;
	RProc *closure_new_cfunc(mrb_func_t func, int nlocals) override;
	mrb_int cmp(mrb_value obj1, mrb_value obj2) override;
	mrb_bool const_defined(mrb_value, mrb_sym) override;
	mrb_bool const_defined_at(mrb_value mod, mrb_sym id) override;
	mrb_value const_get(mrb_value, mrb_sym) override;
	void const_remove(mrb_value, mrb_sym) override;
	void const_set(mrb_value, mrb_sym, mrb_value) override;
	double cstr_to_dbl(const char *s, mrb_bool badcheck) override;
	mrb_value cstr_to_inum(const char *s, mrb_int base,
	    mrb_bool badcheck) override;
	mrb_bool cv_defined(mrb_value mod, mrb_sym sym) override;
	mrb_value cv_get(mrb_value mod, mrb_sym sym) override;
	void cv_set(mrb_value mod, mrb_sym sym, mrb_value v) override;
	void *data_check_get_ptr(mrb_value, const mrb_data_type *) override;
	void data_check_type(mrb_value, const mrb_data_type *) override;
	void *data_get_ptr(mrb_value, const mrb_data_type *) override;
	const char *debug_get_filename(const mrb_irep *irep,
	    uint32_t pc) override;
	int32_t debug_get_line(const mrb_irep *irep, uint32_t pc) override;
	mrb_irep_debug_info *debug_info_alloc(mrb_irep *irep) override;
	mrb_irep_debug_info_file *debug_info_append_file(
	    mrb_irep_debug_info *info, const char *filename, uint16_t *lines,
	    uint32_t start_pos, uint32_t end_pos) override;
	void debug_info_free(mrb_irep_debug_info *d) override;
	void define_alias(RClass *c, const char *a, const char *b) override;
	void define_alias_id(RClass *c, mrb_sym a, mrb_sym b) override;
	RClass *define_class(const char *name, RClass *super) override;
	RClass *define_class_id(mrb_sym name, RClass *super) override;
	void define_class_method_id(RClass *cla, mrb_sym name, mrb_func_t fun,
	    mrb_aspec aspec) override;
	RClass *define_class_under_id(RClass *outer, mrb_sym name,
	    RClass *super) override;
	void define_const(RClass *cla, const char *name,
	    mrb_value val) override;
	void define_const_id(RClass *cla, mrb_sym name, mrb_value val) override;
	void define_global_const(const char *name, mrb_value val) override;
	void define_method_id(RClass *c, mrb_sym mid, mrb_func_t func,
	    mrb_aspec aspec) override;
	void define_method_raw(RClass *, mrb_sym, mrb_method_t) override;
	RClass *define_module(const char *name) override;
	void define_module_function_id(RClass *cla, mrb_sym name,
	    mrb_func_t fun, mrb_aspec aspec) override;
	RClass *define_module_id(mrb_sym name) override;
	RClass *define_module_under_id(RClass *outer, mrb_sym name) override;
	void define_singleton_method(RObject *cla, const char *name,
	    mrb_func_t fun, mrb_aspec aspec) override;
	void define_singleton_method_id(RObject *cla, mrb_sym name,
	    mrb_func_t fun, mrb_aspec aspec) override;
	mrb_value ensure(mrb_func_t body, mrb_value b_data, mrb_func_t ensure,
	    mrb_value e_data) override;
	mrb_value ensure_array_type(mrb_value self) override;
	mrb_value ensure_hash_type(mrb_value hash) override;
	mrb_value ensure_string_type(mrb_value str) override;
	mrb_bool eql(mrb_value obj1, mrb_value obj2) override;
	mrb_bool equal(mrb_value obj1, mrb_value obj2) override;
	mrb_value exc_backtrace(mrb_value exc) override;
	RClass *exc_get_id(mrb_sym name) override;
	mrb_value exc_new(RClass *c, const char *ptr, size_t len) override;
	mrb_value exc_new_str(RClass *c, mrb_value str) override;
	void exc_raise(mrb_value exc) override;
	mrb_value f_raise(mrb_value) override;
	mrb_value fiber_alive_p(mrb_value fib) override;
	mrb_value fiber_resume(mrb_value fib, mrb_int argc,
	    const mrb_value *argv) override;
	mrb_value fiber_yield(mrb_int argc, const mrb_value *argv) override;
	void field_write_barrier(RBasic *, RBasic *) override;
	mrb_value integer_to_str(mrb_value x, mrb_int base) override;
	mrb_value float_to_integer(mrb_value val) override;
	double float_read(const char *, char **) override;
	int float_to_cstr(char *buf, size_t len, const char *fmt,
	    mrb_float f) override;
	mrb_value float_to_str(mrb_value x, const char *fmt) override;
	mrb_value format(const char *format, ...) override;
	void free(void *) override;
	void free_context(mrb_context *c) override;
	void frozen_error(void *frozen_obj) override;
	void full_gc() override;
	mrb_bool func_basic_p(mrb_value obj, mrb_sym mid,
	    mrb_func_t func) override;
	mrb_value funcall(mrb_value val, const char *name, mrb_int argc,
	    ...) override;
	mrb_value funcall_argv(mrb_value val, mrb_sym name, mrb_int argc,
	    const mrb_value *argv) override;
	mrb_value funcall_with_block(mrb_value val, mrb_sym name, mrb_int argc,
	    const mrb_value *argv, mrb_value block) override;
	void garbage_collect() override;
	void gc_mark(RBasic *) override;
	void gc_protect(mrb_value obj) override;
	void gc_register(mrb_value obj) override;
	void gc_unregister(mrb_value obj) override;
	RProc *generate_code(mrb_parser_state *) override;
	mrb_value get_arg1() override;
	mrb_int get_argc() override;
	const mrb_value *get_argv() override;
	mrb_int get_args_a(mrb_args_format format, void **ptr) override;
	mrb_value get_backtrace() override;
	mrb_value gv_get(mrb_sym sym) override;
	void gv_remove(mrb_sym sym) override;
	void gv_set(mrb_sym sym, mrb_value val) override;
	void hash_check_kdict(mrb_value self) override;
	mrb_value hash_clear(mrb_value hash) override;
	mrb_value hash_delete_key(mrb_value hash, mrb_value key) override;
	mrb_value hash_dup(mrb_value hash) override;
	mrb_bool hash_empty_p(mrb_value self) override;
	mrb_value hash_fetch(mrb_value hash, mrb_value key,
	    mrb_value def) override;
	void hash_foreach(RHash *hash, mrb_hash_foreach_func *func,
	    void *p) override;
	mrb_value hash_get(mrb_value hash, mrb_value key) override;
	mrb_bool hash_key_p(mrb_value hash, mrb_value key) override;
	mrb_value hash_keys(mrb_value hash) override;
	void hash_merge(mrb_value hash1, mrb_value hash2) override;
	mrb_value hash_new() override;
	mrb_value hash_new_capa(mrb_int capa) override;
	void hash_set(mrb_value hash, mrb_value key, mrb_value val) override;
	mrb_int hash_size(mrb_value hash) override;
	mrb_value hash_values(mrb_value hash) override;
	void include_module(RClass *cla, RClass *included) override;
	void incremental_gc() override;
	mrb_value inspect(mrb_value obj) override;
	mrb_value instance_new(mrb_value cv) override;
	mrb_sym intern(const char *, size_t) override;
	mrb_sym intern_check(const char *, size_t) override;
	mrb_sym intern_check_cstr(const char *) override;
	mrb_sym intern_check_str(mrb_value) override;
	mrb_sym intern_cstr(const char *str) override;
	mrb_sym intern_static(const char *, size_t) override;
	mrb_sym intern_str(mrb_value) override;
	void iv_copy(mrb_value dst, mrb_value src) override;
	mrb_bool iv_defined(mrb_value, mrb_sym) override;
	void iv_foreach(mrb_value obj, mrb_iv_foreach_func *func,
	    void *p) override;
	mrb_value iv_get(mrb_value obj, mrb_sym sym) override;
	void iv_name_sym_check(mrb_sym sym) override;
	mrb_bool iv_name_sym_p(mrb_sym sym) override;
	mrb_value iv_remove(mrb_value obj, mrb_sym sym) override;
	void iv_set(mrb_value obj, mrb_sym sym, mrb_value v) override;
	mrb_value load_detect_file_cxt(FILE *fp, mrb_ccontext *c) override;
	mrb_value load_exec(mrb_parser_state *p, mrb_ccontext *c) override;
	mrb_value load_file(FILE *) override;
	mrb_value load_file_cxt(FILE *, mrb_ccontext *cxt) override;
	mrb_value load_irep(const uint8_t *) override;
	mrb_value load_irep_buf(const void *, size_t) override;
	mrb_value load_irep_buf_cxt(const void *, size_t,
	    mrb_ccontext *) override;
	mrb_value load_irep_cxt(const uint8_t *, mrb_ccontext *) override;
	mrb_value load_irep_file(FILE *) override;
	mrb_value load_irep_file_cxt(FILE *, mrb_ccontext *) override;
	mrb_value load_nstring(const char *s, size_t len) override;
	mrb_value load_nstring_cxt(const char *s, size_t len,
	    mrb_ccontext *cxt) override;
	mrb_value load_proc(const RProc *proc) override;
	mrb_value load_string(const char *s) override;
	mrb_value load_string_cxt(const char *s, mrb_ccontext *cxt) override;
	void *malloc(size_t) override;
	void *malloc_simple(size_t) override;
	mrb_method_t method_search(RClass *, mrb_sym) override;
	mrb_method_t method_search_vm(RClass **, mrb_sym) override;
	void mod_cv_set(RClass *c, mrb_sym sym, mrb_value v) override;
	RClass *module_get_id(mrb_sym name) override;
	RClass *module_get_under_id(RClass *outer, mrb_sym name) override;
	RClass *module_new() override;
	void mt_foreach(RClass *, mrb_mt_foreach_func *, void *) override;
	void name_error(mrb_sym id, const char *fmt, ...) override;
	void no_method_error(mrb_sym id, mrb_value args, const char *fmt,
	    ...) override;
	void notimplement() override;
	mrb_value notimplement_m(mrb_value) override;
	mrb_value num_minus(mrb_value x, mrb_value y) override;
	mrb_value num_mul(mrb_value x, mrb_value y) override;
	mrb_value num_plus(mrb_value x, mrb_value y) override;
	RBasic *obj_alloc(mrb_vtype, RClass *) override;
	mrb_value obj_as_string(mrb_value obj) override;
	RClass *obj_class(mrb_value obj) override;
	const char *obj_classname(mrb_value obj) override;
	mrb_value obj_clone(mrb_value self) override;
	mrb_value obj_dup(mrb_value obj) override;
	mrb_bool obj_eq(mrb_value a, mrb_value b) override;
	mrb_bool obj_equal(mrb_value a, mrb_value b) override;
	mrb_value obj_freeze(mrb_value) override;
	mrb_int obj_id(mrb_value obj) override;
	mrb_value obj_inspect(mrb_value self) override;
	mrb_bool obj_is_instance_of(mrb_value obj, RClass *c) override;
	mrb_bool obj_is_kind_of(mrb_value obj, RClass *c) override;
	mrb_bool obj_iv_defined(RObject *obj, mrb_sym sym) override;
	mrb_value obj_iv_get(RObject *obj, mrb_sym sym) override;
	void obj_iv_set(RObject *obj, mrb_sym sym, mrb_value v) override;
	mrb_value obj_new(RClass *c, mrb_int argc,
	    const mrb_value *argv) override;
	mrb_bool obj_respond_to(RClass *c, mrb_sym mid) override;
	mrb_sym obj_to_sym(mrb_value name) override;
	mrb_bool object_dead_p(RBasic *object) override;
	void p(mrb_value) override;
	mrb_parser_state *parse_file(FILE *, mrb_ccontext *) override;
	mrb_parser_state *parse_nstring(const char *, size_t,
	    mrb_ccontext *) override;
	mrb_parser_state *parse_string(const char *, mrb_ccontext *) override;
	void parser_free(mrb_parser_state *) override;
	mrb_sym parser_get_filename(mrb_parser_state *, uint16_t idx) override;
	mrb_parser_state *parser_new() override;
	void parser_parse(mrb_parser_state *, mrb_ccontext *) override;
	void parser_set_filename(mrb_parser_state *, const char *) override;
	void prepend_module(RClass *cla, RClass *prepended) override;
	void print_backtrace() override;
	void print_error() override;
	mrb_value proc_cfunc_env_get(mrb_int idx) override;
	RProc *proc_new_cfunc(mrb_func_t) override;
	RProc *proc_new_cfunc_with_env(mrb_func_t func, mrb_int argc,
	    const mrb_value *argv) override;
	mrb_value protect(mrb_func_t body, mrb_value data,
	    mrb_bool *state) override;
	mrb_value ptr_to_str(void *p) override;
	enum mrb_range_beg_len range_beg_len(mrb_value range, mrb_int *begp,
	    mrb_int *lenp, mrb_int len, mrb_bool trunc) override;
	mrb_value range_new(mrb_value start, mrb_value end,
	    mrb_bool exclude) override;
	RRange *range_ptr(mrb_value range) override;
	mrb_irep *read_irep(const uint8_t *) override;
	mrb_irep *read_irep_buf(const void *, size_t) override;
	void *realloc(void *, size_t) override;
	void *realloc_simple(void *, size_t) override;
	void remove_method(RClass *c, mrb_sym sym) override;
	mrb_value rescue(mrb_func_t body, mrb_value b_data, mrb_func_t rescue,
	    mrb_value r_data) override;
	mrb_value rescue_exceptions(mrb_func_t body, mrb_value b_data,
	    mrb_func_t rescue, mrb_value r_data, mrb_int len,
	    RClass **classes) override;
	mrb_bool respond_to(mrb_value obj, mrb_sym mid) override;
	void show_copyright() override;
	void show_version() override;
	mrb_value singleton_class(mrb_value val) override;
	RClass *singleton_class_ptr(mrb_value val) override;
	void stack_extend(mrb_int) override;
	void state_atexit(mrb_atexit_func func) override;
	mrb_value str_append(mrb_value str, mrb_value str2) override;
	mrb_value str_cat(mrb_value str, const char *ptr, size_t len) override;
	mrb_value str_cat_cstr(mrb_value str, const char *ptr) override;
	mrb_value str_cat_str(mrb_value str, mrb_value str2) override;
	int str_cmp(mrb_value str1, mrb_value str2) override;
	void str_concat(mrb_value self, mrb_value other) override;
	mrb_value str_dup(mrb_value str) override;
	mrb_bool str_equal(mrb_value str1, mrb_value str2) override;
	mrb_int str_index(mrb_value str, const char *p, mrb_int len,
	    mrb_int offset) override;
	mrb_value str_intern(mrb_value self) override;
	void str_modify(RString *s) override;
	void str_modify_keep_ascii(RString *s) override;
	mrb_value str_new(const char *p, size_t len) override;
	mrb_value str_new_capa(size_t capa) override;
	mrb_value str_new_static(const char *p, size_t len) override;
	mrb_value str_plus(mrb_value a, mrb_value b) override;
	mrb_value str_resize(mrb_value str, mrb_int len) override;
	mrb_int str_strlen(RString *) override;
	mrb_value str_substr(mrb_value str, mrb_int beg, mrb_int len) override;
	char *str_to_cstr(mrb_value str) override;
	double str_to_dbl(mrb_value str, mrb_bool badcheck) override;
	mrb_value str_to_integer(mrb_value str, mrb_int base,
	    mrb_bool badcheck) override;
	const char *string_cstr(mrb_value str) override;
	mrb_value string_type(mrb_value str) override;
	const char *string_value_cstr(mrb_value *str) override;
	mrb_int string_value_len(mrb_value str) override;
	const char *string_value_ptr(mrb_value str) override;
	const char *sym_dump(mrb_sym) override;
	const char *sym_name(mrb_sym) override;
	const char *sym_name_len(mrb_sym, mrb_int *) override;
	mrb_value sym_str(mrb_sym) override;
	void sys_fail(const char *mesg) override;
	mrb_float to_flo(mrb_value x) override;
	mrb_value to_int(mrb_value val) override;
	mrb_value to_str(mrb_value val) override;
	mrb_value top_run(const RProc *proc, mrb_value self,
	    mrb_int stack_keep) override;
	mrb_value top_self() override;
	mrb_value type_convert(mrb_value val, mrb_vtype type,
	    mrb_sym method) override;
	mrb_value type_convert_check(mrb_value val, mrb_vtype type,
	    mrb_sym method) override;
	void undef_class_method(RClass *cls, const char *name) override;
	void undef_class_method_id(RClass *cls, mrb_sym name) override;
	void undef_method(RClass *cla, const char *name) override;
	void undef_method_id(RClass *, mrb_sym) override;
	mrb_value vformat(const char *format, va_list ap) override;
	mrb_value vm_const_get(mrb_sym) override;
	mrb_value vm_cv_get(mrb_sym) override;
	void vm_cv_set(mrb_sym, mrb_value) override;
	RClass *vm_define_class(mrb_value, mrb_value, mrb_sym) override;
	RClass *vm_define_module(mrb_value, mrb_sym) override;
	mrb_value vm_exec(const RProc *proc, const mrb_code *iseq) override;
	mrb_value vm_run(const RProc *proc, mrb_value self,
	    mrb_int stack_keep) override;
	mrb_value vm_special_get(mrb_sym) override;
	void vm_special_set(mrb_sym, mrb_value) override;
	void warn(const char *fmt, ...) override;
	mrb_value word_boxing_cptr_value(void *) override;
	mrb_value word_boxing_float_value(mrb_float) override;
	mrb_value word_boxing_int_value(mrb_int) override;
	void write_barrier(RBasic *) override;
	mrb_value yield(mrb_value b, mrb_value arg) override;
	mrb_value yield_argv(mrb_value b, mrb_int argc,
	    const mrb_value *argv) override;
	mrb_value yield_with_class(mrb_value b, mrb_int argc,
	    const mrb_value *argv, mrb_value self, RClass *c) override;
	mrb_value obj_value(void *p) override;
	mrb_value int_value(mrb_int i) override;
	mrb_value float_value(mrb_float f) override;
	mrb_value symbol_value(mrb_sym i) override;
	RClass *exception() override;
	RClass *standard_error() override;
	RClass *runtime_error() override;
	RClass *type_error() override;
	RClass *zero_division_error() override;
	RClass *argument_error() override;
	RClass *index_error() override;
	RClass *range_error() override;
	RClass *name_error() override;
	RClass *no_method_error() override;
	RClass *script_error() override;
	RClass *syntax_error() override;
	RClass *local_jump_error() override;
	RClass *regexp_error() override;
	RClass *frozen_error() override;
	RClass *not_implemented_error() override;
	RClass *key_error() override;
	RClass *float_domain_error() override;
	bool block_given_p() override;
	util::Error::TypeInfo error_type_info(RObject *exc) override;
	std::string error_cause(RObject *exc) override;
	std::string error_backtrace(RObject *exc) override;

private:
	mrb_state *_mrb;
	bool _closed = false;
};
} /* namespace euler::app::native */

#endif /* EULER_APP_NATIVE_RUBY_STATE_H */
