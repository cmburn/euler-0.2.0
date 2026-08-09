/* SPDX-License-Identifier: ISC */

#include <cstdio>
#include <cstdlib>
#include <exception>

#include "euler/app/native/state.h"
#include "euler/app/state.h"
#include "euler/util/logger.h"

int
main(const int argc, const char **argv)
{
	try {
		const auto state
		    = euler::util::make_reference<euler::app::State>();
		const auto cfg = euler::app::native::parse_config(argc, argv);
		if (!state->initialize(cfg)) {
			state->log()->error("Failed to initialize state");
			return EXIT_FAILURE;
		}
		state->log()->info("Initialization complete");
		int exit_code = EXIT_SUCCESS;
		while (state->loop(exit_code)) {
			/* ReSharper disable once CppRedundantControlFlowJump */
			continue; /* NOLINT(*-redundant-control-flow) */
		}
		state->log()->info("Exiting with code {}", exit_code);
		return exit_code;
		/* ReSharper disable once CppDFAUnreachableCode */
	} catch (const std::exception &e) {
		fprintf(stderr, "Unhandled exception: %s\n", e.what());
		return EXIT_FAILURE;
	} catch (...) {
		fprintf(stderr, "Unhandled unknown exception\n");
		return EXIT_FAILURE;
	}
}
