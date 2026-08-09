/* SPDX-License-Identifier: ISC */

#include "euler/util/ext.h"

#include "euler/util/logger.h"
#include "euler/util/version.h"

RClass *
euler::util::init(const Reference<State> &state, RClass *, RClass *)
{
	auto &util = state->modules().util;
	const auto mod = util.mod;
	util.logger = Logger::init(state, mod);
	util.version = Version::init(state, mod);
	return mod;
}

float
euler::util::read_hash_float(mrb_state *mrb, const mrb_value hash,
    const mrb_sym key, const float default_value)
{
	const auto state = State::get(mrb);
	const auto sym = mrb_symbol_value(key);
	const auto value = state->mrb()->hash_get(hash, sym);
	if (mrb_nil_p(value)) return default_value;
	return static_cast<float>(state->mrb()->to_flo(value));
}