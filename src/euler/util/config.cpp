/* SPDX-License-Identifier: ISC */

#include "euler/util/config.h"

#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <unordered_map>

#include <nlohmann/json.hpp>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API [[maybe_unused]] static
#include "euler/util/optparse.h"

using euler::util::Config;

[[noreturn]] static void
usage(const char *progname, const char *errmsg = nullptr)
{
	const bool is_error = errmsg != nullptr;
	FILE *output = is_error ? stderr : stdout;
	if (is_error) fprintf(output, "Error: %s\n", errmsg);
	fprintf(output, "Usage: %s [options] [--] FILE\n", progname);
	static constexpr auto usage_message =
	    R"EOF(Options:
-h, --help              Display this help menu and exit
-c, --config PATH       Specify a JSONC configuration file. Must end in .json.
                        Option flags will override the settings here. Note that
                        the file is parsed as JSONC, so both comments and commas
                        are allowed.
-l, --log-level LEVEL   Set the logging verbosity level. Defaults to `debug` on
                        debug builds, `info` otherwise. Valid options are:
                        - debug
                        - info
                        - warn
                        - error
                        - fatal
-q, --quiet             Decrease the current logging level by one, down to
                        `debug`.
-S, --shader-path PATH  Add a shader search path. *.spv files found within can
                        be loaded as shaders. Multiple paths may be registered,
                        in which case they will be searched first to last when a
                        shader is loaded.
-t, --title TITLE       Specify a window title to use.
-V, --version           Print version information and exit.
-v, --verbose           Increase the current logging level by one, up to
                        `fatal`.
--preferred-gpu ID      Attempt to launch using GPU with the given ID.
--present-mode MODE     Use the given present mode. Defaults to `fifo`. Valid
                        options are:
                        - immediate
                        - fifo
                        - fifo_relaxed
                        - mailbox
)EOF";
	fprintf(output, "%s", usage_message);
	exit(is_error ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void
print_version()
{
	fprintf(stdout, "Euler Engine Version: %s\n",
	    euler::util::engine_version().to_string().c_str());
	exit(EXIT_SUCCESS);
}

static std::optional<Config::PresentMode>
parse_present_mode(const char *str)
{
	using PresentMode = Config::PresentMode;
	static const std::unordered_map<std::string_view, PresentMode> modes = {
		{ "immediate", PresentMode::Immediate },
		{ "fifo", PresentMode::Fifo },
		{ "fifo_relaxed", PresentMode::FifoRelaxed },
		{ "mailbox", PresentMode::Mailbox },
	};
	if (const auto it = modes.find(str); it != modes.end())
		return it->second;
	return std::nullopt;
}

Config
Config::from_json(const nlohmann::json &json)
{
	auto cfg = Config {};
	if (json.contains("title")) cfg.title = json["title"];
	if (json.contains("app_version")) {
		const auto str = json["app_version"].get<std::string_view>();
		cfg.app_version = Version::parse(str);
	}
	if (json.contains("log_level")) {
		const auto str = json["log_level"].get<std::string>();
		cfg.severity = Logger::parse_severity(str);
	}
#ifdef EULER_NATIVE
	if (json.contains("preferred_gpu"))
		cfg.preferred_gpu = json["preferred_gpu"];
	if (json.contains("present_mode")) {
		const auto str = json["present_mode"].get<std::string>();
		cfg.present_mode = parse_present_mode(str.c_str());
	}
	if (json.contains("shader_paths")) {
		for (const auto &path : json["shader_paths"])
			cfg.shader_paths.emplace_back(path.get<std::string>());
	}
#endif
	return cfg;
}

nlohmann::json
Config::to_json() const
{
	auto j = nlohmann::json::object();
	if (title.has_value()) j["title"] = title.value();
	if (app_version.has_value())
		j["app_version"] = app_version.value().to_string();
	if (severity.has_value())
		j["log_level"] = Logger::severity_to_string(severity.value());
	return j;
}

Config
Config::merge(const Config &other) const
{
	auto merged = *this;
	if (other.title.has_value()) merged.title = other.title;
	if (other.app_version.has_value())
		merged.app_version = other.app_version;
	if (other.severity.has_value()) merged.severity = other.severity;
	return merged;
}

Config
Config::from_argv(int, const char **argv)
{
	static constexpr int32_t UTF8_MAX = 0x10FFFF;
	enum class OptionFlag {
		Done = -1,
		Config = 'c',
		Help = 'h',
		LogLevel = 'l',
		ShaderPath = 'S',
		Title = 't',
		Version = 'V',
		Verbosity = 'v',
		PreferredGPU = UTF8_MAX + 1,
		PresentMode,
	};
	static constexpr std::array OPTIONS = {
		(struct optparse_long) {
		    "help",
		    static_cast<int>(OptionFlag::Help),
		    OPTPARSE_NONE,
		},
		(struct optparse_long) {
		    "shader-path",
		    static_cast<int>(OptionFlag::ShaderPath),
		    OPTPARSE_REQUIRED,
		},
		(struct optparse_long) {
		    "title",
		    static_cast<int>(OptionFlag::Title),
		    OPTPARSE_OPTIONAL,
		},
		(struct optparse_long) {
		    "version",
		    static_cast<int>(OptionFlag::Version),
		    OPTPARSE_NONE,
		},
		(struct optparse_long) {
		    "verbosity",
		    static_cast<int>(OptionFlag::Verbosity),
		    OPTPARSE_OPTIONAL,
		},
		(struct optparse_long) {
		    "preferred-gpu",
		    static_cast<int>(OptionFlag::PreferredGPU),
		    OPTPARSE_REQUIRED,
		},
		(struct optparse_long) {
		    "present-mode",
		    static_cast<int>(OptionFlag::PresentMode),
		    OPTPARSE_REQUIRED,
		},
		/* Optparse requires an empty argument at the end */
		(struct optparse_long) {
		    nullptr,
		    0,
		    OPTPARSE_NONE,
		},
	};
	Config flag_config;
	std::optional<Config> file_config;
	std::optional<std::string> config_path;
	/* okay to const cast, not modified if permute = 0 */
	struct optparse optparse = {};
	optparse_init(&optparse, const_cast<char **>(argv));
	optparse.permute = false;
	static constexpr auto parse = [](struct optparse &o) {
		return static_cast<OptionFlag>(
		    optparse_long(&o, OPTIONS.data(), nullptr));
	};
	for (auto opt = parse(optparse); opt != OptionFlag::Done;
	    opt = parse(optparse)) {
		switch (opt) {
		case OptionFlag::Config: {
			const auto path
			    = std::filesystem::path(optparse.optarg);
			if (!std::filesystem::exists(path)) {
				const auto msg = std::format(
				    "Configuration file does not exist: {}",
				    path.string());
				usage(argv[0], msg.c_str());
			}
			auto ifs = std::ifstream(path);
			if (!ifs.is_open()) {
				const auto msg = std::format(
				    "Failed to open configuration file: {}",
				    path.string());
				usage(path.c_str(), msg.c_str());
			}
			auto json = nlohmann::json::parse(ifs, nullptr, true,
			    true, true);
			break;
		}
		case OptionFlag::Help: usage(argv[0]);
		case OptionFlag::ShaderPath:
			flag_config.shader_paths.emplace_back(optparse.optarg);
			break;
		case OptionFlag::Title:
			flag_config.title = optparse.optarg;
			break;
		case OptionFlag::Version: print_version(); break;
		case OptionFlag::Verbosity: {
			if (optparse.optarg != nullptr) {
				flag_config.severity
				    = Logger::parse_severity(optparse.optarg);
				break;
			}
			const auto cur = flag_config.severity.value_or(
			    Logger::Severity::Debug);
			const auto next_value = static_cast<int>(cur) + 1;
			const auto next
			    = static_cast<Logger::Severity>(next_value);
			flag_config.severity = std::min<Logger::Severity>(next,
			    Logger::Severity::Fatal);
			break;
		}
		case OptionFlag::PreferredGPU:
			flag_config.preferred_gpu = static_cast<uint32_t>(
			    std::stoul(optparse.optarg));
			break;
		case OptionFlag::PresentMode:
			flag_config.present_mode
			    = parse_present_mode(optparse.optarg);
			if (!flag_config.present_mode.has_value())
				usage(argv[0], "Invalid present mode");
			break;
		default: {
			const auto errmsg = optparse.errmsg[0] != '\0'
			    ? optparse.errmsg
			    : "Unknown option";
			usage(argv[0], errmsg);
		}
		}
	}

	if (file_config.has_value())
		flag_config = file_config.value().merge(flag_config);

	// if (flag_config.shader_paths.empty())

	if (const auto bin_dir = std::filesystem::path(argv[0]);
	    std::filesystem::exists(bin_dir)) {
		const auto path = bin_dir.parent_path().string();
		flag_config.shader_paths.emplace_back(path);
	}

	/* TODO: parse command line arguments */
	return flag_config;
}