/* SPDX-License-Identifier: ISC */

#include "euler/util/object.h"

#include "euler/util/state.h"

RData *
euler::util::safe_data_object_alloc(mrb_state *mrb, RClass *klass, void *datap,
    const mrb_data_type *type)
{
	return State::get(mrb)->rb().data_object_alloc(klass, datap, type);
}

void *
euler::util::checked_unwrap_ptr(mrb_state *mrb, const mrb_value value,
    const mrb_data_type *type)
{
	return State::get(mrb)->rb().data_check_get_ptr(value, type);
}