/* SPDX-License-Identifier: ISC */

#include "euler/vulkan/renderer.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>

#include "euler/util/config.h"
#include "euler/util/logger.h"
#include "euler/util/version.h"
#include "euler/vulkan/compound_shader.h"
#include "euler/vulkan/compute_shader.h"
#include "euler/vulkan/fragment_shader.h"
#include "euler/vulkan/graphics_pipeline.h"
#include "euler/vulkan/shader.h"
#include "euler/vulkan/swapchain.h"
#include "euler/vulkan/vertex_shader.h"

#ifndef ENGINE_NAME
#define EULER_ENGINE_NAME "euler"
#endif

#ifndef ENGINE_VERSION_MAJOR
#define ENGINE_VERSION_MAJOR 0
#define AUTO_SUPPLIED_VERSION
#else
#define USER_SUPPLIED_VERSION
#endif

#ifndef ENGINE_VERSION_MINOR
#define ENGINE_VERSION_MINOR 1
#define AUTO_SUPPLIED_VERSION
#else
#define USER_SUPPLIED_VERSION
#endif

#ifndef ENGINE_VERSION_PATCH
#define ENGINE_VERSION_PATCH 0
#define AUTO_SUPPLIED_VERSION
#else
#define USER_SUPPLIED_VERSION
#endif

#if defined(AUTO_SUPPLIED_VERSION) && defined(USER_SUPPLIED_VERSION)
#error "Major, minor, and patch numbers must be supplied together"
#endif
#undef AUTO_SUPPLIED_VERSION
#undef USER_SUPPLIED_VERSION

using euler::vulkan::Renderer;

using LogRef = euler::util::Reference<euler::util::Logger>;

static constexpr std::array VALIDATION_LAYER_NAMES = {
	"VK_LAYER_KHRONOS_validation",
};

/* ReSharper disable once CppDFAConstantFunctionResult */
static consteval int
sdl_init_flags()
{
	int flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO;
#ifdef EULER_USE_JOYSTICK
	flags |= SDL_INIT_JOYSTICK;
#endif
#ifdef EULER_USE_HAPTIC
	flags |= SDL_INIT_HAPTIC;
#endif
#ifdef EULER_USE_GAMEPAD
	flags |= SDL_INIT_GAMEPAD;
#endif
#ifdef EULER_USE_SENSOR
	flags |= SDL_INIT_SENSOR;
#endif
#ifdef EULER_USE_CAMERA
	flags |= SDL_INIT_CAMERA;
#endif
	return flags;
}

void
Renderer::global_init()
{
	static std::once_flag flag;
	std::call_once(flag, []() {
		static constexpr auto SDL_FLAGS = sdl_init_flags();
		if (SDL_Init(SDL_FLAGS) && SDL_Vulkan_LoadLibrary(nullptr))
			return;
		fprintf(stderr, "SDL initialization failed: %s\n",
		    SDL_GetError());
		std::exit(EXIT_FAILURE);
	});
}

uint32_t
Renderer::window_count() const
{
	return static_cast<uint32_t>(_windows.size());
}

static PFN_vkGetInstanceProcAddr
get_instance_proc_addr()
{
	Renderer::global_init();
	if (const auto p = SDL_Vulkan_GetVkGetInstanceProcAddr(); p != nullptr)
		return reinterpret_cast<PFN_vkGetInstanceProcAddr>(p);
	fprintf(stderr, "Failed to get Vulkan instance proc addr: %s\n",
	    SDL_GetError());
	std::exit(EXIT_FAILURE);
}

static bool
have_validation_layer_support(const LogRef &log)
{
	/* TODO: always returns false? */

	static bool checked = false;
	static bool supported = false;
	if (checked) return supported;
	for (const auto &prop : vk::enumerateInstanceLayerProperties()) {
		log->info("Found layer '{}'", prop.layerName.data());
		const auto name = std::string_view(prop.layerName.data(),
		    prop.layerName.size());
		// TODO: fix this for multiple layers
		for (const std::string_view layer : VALIDATION_LAYER_NAMES) {
			if (layer != name) continue;
			supported = true;
			break;
		}
	}
	checked = true;
	// return supported;
	return true;
}

static int
vma_allocator_flags(vk::raii::PhysicalDevice &device)
{
	static const std::unordered_map<std::string_view, int> map = {
		{
		    "VK_KHR_dedicated_allocation",
		    VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT,
		},
		{
		    "VK_KHR_bind_memory2",
		    VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT,
		},
		{
		    "VK_KHR_maintenance4",
		    VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE4_BIT,
		},
		{
		    "VK_KHR_maintenance5",
		    VMA_ALLOCATOR_CREATE_KHR_MAINTENANCE5_BIT,
		},
		{
		    "VK_EXT_memory_budget",
		    VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
		},
		{
		    "VK_KHR_buffer_device_address",
		    VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
		},
		{
		    "VK_EXT_memory_priority",
		    VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT,
		},
		{
		    "VK_AMD_device_coherent_memory",
		    VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT,
		},
		{
		    "VK_KHR_external_memory_win32",
		    VMA_ALLOCATOR_CREATE_KHR_EXTERNAL_MEMORY_WIN32_BIT,
		},
	};
	int flags = 0;
	for (const auto &[str, v] :
	    device.enumerateDeviceExtensionProperties()) {
		const auto name = std::string_view(str.data(), str.size());
		const auto it = map.find(name);
		if (it == map.end()) continue;
		flags |= it->second;
	}
	return flags;
}

static const std::vector<const char *> &
instance_extensions(const LogRef &log)
{
	static std::vector<const char *> vec;
	static std::once_flag once_flag;
	std::call_once(once_flag, [&]() {
		Renderer::global_init();
		uint32_t count;
		const auto *exts = SDL_Vulkan_GetInstanceExtensions(&count);
		if (exts == nullptr) {
			if (log != nullptr) {
				log->fatal("Failed to get SDL Vulkan instance "
				           "extensions");
			}
			fprintf(stderr,
			    "Failed to get SDL Vulkan instance extensions\n");
			std::exit(EXIT_FAILURE);
		}
		vec.reserve(count);
		for (uint32_t i = 0; i < count; i++) vec.emplace_back(exts[i]);
		if (vec.empty()) return;
		if (log != nullptr)
			log->info("Using Vulkan instance extensions:");
		for (const auto s : vec) log->info("\t- {}", s);
	});
	return vec;
}

Renderer::Renderer(const util::Reference<util::State> &state)
    : _log(state->log())
    , _state(state.weaken())
    , _context(get_instance_proc_addr())
    , _instance(create_instance())
    , _physical_device(select_physical_device())
    , _graphics_index(find_graphics_present_queue())
    , _compute_index(find_compute_queue())
    , _device(create_logical_device())
    , _allocator(make_allocator())
    , _graphics_queue(_device, _graphics_index, 0)
    , _fragment_shader(load_fragment_shader("shaders/shader.frag.spv"))
    , _vertex_shader(load_vertex_shader("shaders/shader.vert.spv"))
    , _command_pool(create_command_pool())
    , _command_buffers(create_command_buffers())
{
}

Renderer::~Renderer() { vmaDestroyAllocator(_allocator); }

euler::util::Reference<euler::vulkan::Window>
Renderer::create_window(const char *title, int16_t w, int16_t h,
    Window::Flags flags)
{
	const auto ptr = new Window(util::Reference(this), title, w, h, flags);
	auto window = util::Reference(ptr);
	_windows.emplace_back(window);
	_window_indices.emplace(std::string(title), _windows.size() - 1);
	rebuild_command_buffers();
	return window;
}

euler::util::Reference<euler::vulkan::FragmentShader>
Renderer::load_fragment_shader(const std::string_view path)
{
	return load_shader<FragmentShader>(path);
}

euler::util::Reference<euler::vulkan::VertexShader>
Renderer::load_vertex_shader(const std::string_view path)
{
	return load_shader<VertexShader>(path);
}

euler::util::Reference<euler::vulkan::ComputeShader>
Renderer::load_compute_shader(const std::string_view path)
{
	return load_shader<ComputeShader>(path);
}

euler::util::Reference<euler::vulkan::CompoundShader>
Renderer::load_compound_shader(std::string_view,
    const std::vector<std::pair<std::string_view, Shader::Type>> &)
{
	/* TODO */
	return nullptr;
}

euler::util::Reference<euler::vulkan::FragmentShader>
Renderer::fragment_shader() const
{
	return _fragment_shader;
}

euler::util::Reference<euler::vulkan::VertexShader>
Renderer::vertex_shader() const
{
	return _vertex_shader;
}

void
Renderer::draw()
{
	record_command_buffers();
}

euler::util::Reference<euler::util::State>
Renderer::state() const
{
	return _state.strengthen();
}

vk::raii::Instance
Renderer::create_instance()
{
	auto exts = instance_extensions(state()->log());
	const auto app_name = state()->progname();
	const vk::ApplicationInfo app_info = {
		.pApplicationName = app_name.c_str(),
		.applicationVersion = state()->app_version().to_vulkan(),
		.pEngineName = EULER_ENGINE_NAME,
		.engineVersion = state()->engine_version().to_vulkan(),
		.apiVersion = VK_API_VERSION_1_4,
	};
	vk::InstanceCreateInfo info = {
		.pApplicationInfo = &app_info,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = static_cast<uint32_t>(exts.size()),
		.ppEnabledExtensionNames = exts.data(),
	};
	if (!have_validation_layer_support(_log)) {
		_log->info("Validation layers not supported.");
	} else {
		_log->info("Validation enabled.");
		info.enabledLayerCount = VALIDATION_LAYER_NAMES.size();
		info.ppEnabledLayerNames = VALIDATION_LAYER_NAMES.data();
	}
	return vk::raii::Instance(_context, info);
}

static constexpr std::array REQUIRED_DEVICE_EXTS = {
	vk::KHRSwapchainExtensionName,
};

static bool
is_compatible(VkInstance instance, const vk::raii::PhysicalDevice &pd)
{
	if (pd.getProperties().apiVersion < VK_API_VERSION_1_4) return false;
	const auto queue_families = pd.getQueueFamilyProperties();
	bool have_graphics = false;
	for (uint32_t i = 0; i < queue_families.size(); ++i) {
		const auto &qf = queue_families[i];
		if (!(qf.queueFlags & vk::QueueFlagBits::eGraphics)) {
			continue;
		}
		if (SDL_Vulkan_GetPresentationSupport(instance, *pd, i)) {
			have_graphics = true;
			break;
		}
	}
	if (!have_graphics) return false;
	auto compute = std::ranges::any_of(queue_families, [](const auto &qf) {
		return static_cast<bool>(
		    qf.queueFlags & vk::QueueFlagBits::eCompute);
	});
	if (!compute) return false;
	const auto extensions = pd.enumerateDeviceExtensionProperties();
	const auto has_required_exts = std::ranges::all_of(REQUIRED_DEVICE_EXTS,
	    [&](const auto &ext) {
		    return std::ranges::any_of(extensions, [&](const auto &e) {
			    return std::strcmp(e.extensionName, ext) == 0;
		    });
	    });
	if (!has_required_exts) return false;
	auto features = pd.getFeatures2<vk::PhysicalDeviceFeatures2,
	    vk::PhysicalDeviceVulkan11Features,
	    vk::PhysicalDeviceVulkan13Features,
	    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
#define REQUIRE_FEATURE(FEATURE_SET, FEATURE)                                  \
	do {                                                                   \
		if (!features.get<vk::PhysicalDevice##FEATURE_SET>().FEATURE)  \
			return false;                                          \
	} while (0)
	REQUIRE_FEATURE(Vulkan11Features, shaderDrawParameters);
	REQUIRE_FEATURE(Vulkan13Features, dynamicRendering);
	REQUIRE_FEATURE(ExtendedDynamicStateFeaturesEXT, extendedDynamicState);
#undef REQUIRE_FEATURE
	return true;
}

static std::optional<vk::raii::PhysicalDevice>
first_compatible(const VkInstance instance,
    const std::vector<vk::raii::PhysicalDevice> &devices)
{
	for (const auto &pd : devices)
		if (is_compatible(instance, pd)) return pd;
	return std::nullopt;
}

static bool
rank_physical_devices(std::vector<vk::raii::PhysicalDevice> &devices,
    std::optional<uint32_t> preferred_gpu)
{
	bool found_preferred = false;
	std::ranges::stable_sort(devices, [&](const auto &a, const auto &b) {
		auto a_props = a.getProperties();
		auto b_props = b.getProperties();
		if (a_props.deviceID == b_props.deviceID) return false;
		if (preferred_gpu.has_value()) {
			if (a_props.deviceID == preferred_gpu.value()) {
				found_preferred = true;
				return true;
			}
			if (b_props.deviceID == preferred_gpu.value()) {
				found_preferred = true;
				return false;
			}
		}
		if (a_props.deviceType != b_props.deviceType) {
			if (a_props.deviceType
			    == vk::PhysicalDeviceType::eDiscreteGpu)
				return true;
			if (b_props.deviceType
			    == vk::PhysicalDeviceType::eDiscreteGpu)
				return false;
		}
		return false;
	});
	return found_preferred;
}

vk::raii::PhysicalDevice
Renderer::select_physical_device()
{
	auto devices = _instance.enumeratePhysicalDevices();
	for (const auto &d : devices) {
		auto properties = d.getProperties();
		_log->info("Found device with ID {} ({})", properties.deviceID,
		    properties.deviceName.data());
	}
	const auto preferred = state()->config().preferred_gpu;
	bool found_preferred = rank_physical_devices(devices, preferred);
	if (preferred.has_value() && !found_preferred) {
		_log->warn("Unable to find user-specified graphics "
		           "device with ID {}",
		    preferred.value());
	}
	const auto pd = first_compatible(*_instance, devices);
	if (!pd.has_value())
		_log->fatal("No compatible Vulkan physical device found");
	const auto props = pd->getProperties();
	_log->info("Selected device with ID {} ({})", props.deviceID,
	    props.deviceName.data());
	return pd.value();
}

template <std::size_t N, typename F>
static constexpr auto
generate_array(F &&f)
{
	return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
		return std::array<std::invoke_result_t<F &>, N> { (
		    static_cast<void>(Is), f())... };
	}(std::make_index_sequence<N> {});
}

Renderer::CommandBufferSet
Renderer::make_command_buffer_set()
{
	return generate_array<MAX_FRAMES_IN_FLIGHT>([]() {
		//
		return vk::raii::CommandBuffer(nullptr);
	});
}

uint32_t
Renderer::find_graphics_present_queue() const
{
	const auto props = _physical_device.getQueueFamilyProperties();
	for (uint32_t i = 0; i < props.size(); ++i) {
		const auto &qf = props[i];
		if (!(qf.queueFlags & vk::QueueFlagBits::eGraphics)) continue;
		if (SDL_Vulkan_GetPresentationSupport(*_instance,
		        *_physical_device, i))
			return i;
	}
	return static_cast<uint32_t>(-1);
}

uint32_t
Renderer::find_compute_queue() const
{
	const auto props = _physical_device.getQueueFamilyProperties();
	for (uint32_t i = 0; i < props.size(); ++i) {
		const auto &qf = props[i];
		if (qf.queueFlags & vk::QueueFlagBits::eCompute) return i;
	}
	return static_cast<uint32_t>(-1);
}

vk::raii::Device
Renderer::create_logical_device()
{
	static constexpr float priority = 0.5f;
	const vk::DeviceQueueCreateInfo queue_info = {
		.queueFamilyIndex = _graphics_index,
		.queueCount = 1,
		.pQueuePriorities = &priority,
	};
	const vk::StructureChain feature_chain = {
		vk::PhysicalDeviceFeatures2 {},
		vk::PhysicalDeviceVulkan11Features {
		    .shaderDrawParameters = true,
		},
		vk::PhysicalDeviceVulkan13Features {
		    .dynamicRendering = true,
		},
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT {
		    .extendedDynamicState = true,
		},
	};
	const vk::DeviceCreateInfo device_info = {
		.pNext = &feature_chain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queue_info,
		.enabledExtensionCount = REQUIRED_DEVICE_EXTS.size(),
		.ppEnabledExtensionNames = REQUIRED_DEVICE_EXTS.data(),
	};
	return vk::raii::Device(_physical_device, device_info);
}

VmaAllocator
Renderer::make_allocator()
{
	VmaVulkanFunctions vk_funcs = {};
	vk_funcs.vkGetInstanceProcAddr = get_instance_proc_addr();
	vk_funcs.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.flags = vma_allocator_flags(_physical_device);
	allocator_info.physicalDevice = *_physical_device;
	allocator_info.device = *_device;
	allocator_info.instance = *_instance;
	allocator_info.vulkanApiVersion = VK_API_VERSION_1_4;
	allocator_info.pVulkanFunctions = &vk_funcs;
	VmaAllocator allocator;
	if (const auto result = vmaCreateAllocator(&allocator_info, &allocator);
	    result != VK_SUCCESS) {
		_log->fatal("Failed to create Vulkan memory allocator");
	}
	return allocator;
}

static std::vector<uint32_t>
read_file(const std::filesystem::path &path)
{
	assert(std::filesystem::exists(path));
	std::ifstream ifs(path, std::ios::binary | std::ios::ate);
	if (!ifs) {
		throw std::runtime_error(
		    "failed to open file: " + path.string());
	}
	const auto size = static_cast<std::streamsize>(ifs.tellg());
	if (size < 0 || size % sizeof(uint32_t) != 0) {
		throw std::runtime_error(
		    "file size is not a multiple of 4 bytes: " + path.string());
	}
	std::vector<uint32_t> buffer(
	    static_cast<size_t>(size) / sizeof(uint32_t));
	ifs.seekg(0);
	ifs.read(reinterpret_cast<char *>(buffer.data()), size);
	return buffer;
}

std::vector<uint32_t>
Renderer::load_shader_data(std::string_view path) const
{
	for (const auto &shader_path : state()->config().shader_paths) {
		const auto full_path
		    = std::filesystem::path(shader_path).append(path);
		if (!std::filesystem::exists(full_path)) continue;
		return read_file(full_path);
	}
	std::stringstream ss;
	ss << "Shader file '" << path
	   << "' not found in any shader path. Searched paths:\n";
	for (const auto &shader_path : state()->config().shader_paths) {
		ss << "  - '" << shader_path << "'\n";
	}

	log()->error("{}", ss.str());
	const auto msg = std::format(
	    "Shader file '{}' not found in any shader path", path);

	throw std::invalid_argument(msg);
}

vk::raii::CommandPool
Renderer::create_command_pool()
{
	const vk::CommandPoolCreateInfo info = {
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = _graphics_index,
	};
	return vk::raii::CommandPool(_device, info);
}

std::vector<Renderer::CommandBufferSet>
Renderer::create_command_buffers()
{
	const uint32_t n = window_count();
	if (n == 0) return {};
	const auto cmd_buf_count = MAX_FRAMES_IN_FLIGHT * n;
	const vk::CommandBufferAllocateInfo info = {
		.commandPool = *_command_pool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = cmd_buf_count,
	};
	auto bufs = vk::raii::CommandBuffers(_device, info);
	std::vector<CommandBufferSet> v;
	v.reserve(n);
	for (uint32_t i = 0; i < n; ++i) {
		CommandBufferSet set = make_command_buffer_set();
		for (uint32_t j = 0; j < MAX_FRAMES_IN_FLIGHT; ++j)
			set[j] = std::move(bufs[i * MAX_FRAMES_IN_FLIGHT + j]);
		v.emplace_back(std::move(set));
	}
	return v;
}

void
Renderer::rebuild_command_buffers()
{
	_command_buffers = create_command_buffers();
}

void
Renderer::rebuild_window_cache()
{
	const uint32_t n = std::ranges::count(_windows, nullptr);
	const auto old = std::move(_windows);
	_windows = {};
	_window_indices = {};
	_windows.reserve(n);
	_window_indices.reserve(n);
	for (uint32_t i = 0, j = 0; i < old.size(); ++i) {
		const auto &w = old[i].strengthen();
		if (w == nullptr) continue;
		_windows.emplace_back(w);
		_window_indices.emplace(std::string(w->title()), j++);
	}
	rebuild_command_buffers();
}
void
Renderer::record_command_buffer(const vk::raii::CommandBuffer &cmd,
    Window &window, const uint32_t image_index)
{
	using StageFlag = vk::PipelineStageFlagBits;
	cmd.begin({});
	auto &sc = window.surface().swapchain();
	const ImageLayoutTransition start_ilt = {
		.swapchain = sc,
		.image_index = image_index,
		.old_layout = vk::ImageLayout::eUndefined,
		.new_layout = vk::ImageLayout::eColorAttachmentOptimal,
		.src_access_mask = {},
		.dst_access_mask = vk::AccessFlagBits::eColorAttachmentWrite,
		.src_stage_mask = StageFlag::eColorAttachmentOutput,
		.dst_stage_mask = StageFlag::eColorAttachmentOutput,
	};
	transition_image_layout(cmd, start_ilt);
	static constexpr vk::ClearValue clearColor
	    = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	const vk::RenderingAttachmentInfo attachment_info = {
		.imageView = sc.image_view(image_index),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor,
	};
	const auto ext = sc.extent();
	const vk::Rect2D render_area = {
		.offset = vk::Offset2D { 0, 0 },
		.extent = ext,
	};
	const vk::RenderingInfo rendering_info = {
		.renderArea = render_area,
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachment_info,
	};
	const auto gp = sc.graphics_pipeline();
	cmd.beginRendering(rendering_info);
	cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, gp->pipeline());
	const vk::Viewport viewport = {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(ext.width),
		.height = static_cast<float>(ext.height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};
	cmd.setViewport(0, viewport);
	cmd.setScissor(0, render_area);
	window.surface().record_commands(cmd, image_index);
	cmd.draw(3, 1, 0, 0);
	cmd.endRendering();
	const ImageLayoutTransition end_ilt = {
		.swapchain = sc,
		.image_index = image_index,
		.old_layout = vk::ImageLayout::eColorAttachmentOptimal,
		.new_layout = vk::ImageLayout::ePresentSrcKHR,
		.src_access_mask = vk::AccessFlagBits::eColorAttachmentWrite,
		.dst_access_mask = {},
		.src_stage_mask = StageFlag::eColorAttachmentOutput,
		.dst_stage_mask = StageFlag::eBottomOfPipe,
	};
	transition_image_layout(cmd, end_ilt);
	cmd.end();
}

void
Renderer::record_command_buffers()
{
	for (const auto [i, wr] : std::views::enumerate(_windows)) {
		if (wr == nullptr) continue;
		auto w = wr.strengthen();
		for (uint32_t j = 0; j < MAX_FRAMES_IN_FLIGHT; ++j) {
			const auto &buf = _command_buffers[i][j];
			record_command_buffer(buf, *w, j);
		}
	}
}

void
Renderer::transition_image_layout(const vk::raii::CommandBuffer &buf,
    const ImageLayoutTransition &ilt)
{
	const auto &i = ilt.swapchain.image(ilt.image_index);
	const vk::ImageMemoryBarrier barrier = {
		.srcAccessMask = ilt.src_access_mask,
		.dstAccessMask = ilt.dst_access_mask,
		.oldLayout = ilt.old_layout,
		.newLayout = ilt.new_layout,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = i,
		.subresourceRange = {
		    .aspectMask = vk::ImageAspectFlagBits::eColor,
		    .baseMipLevel = 0,
		    .levelCount = 1,
		    .baseArrayLayer = 0,
		    .layerCount = 1,
		},
	};
	buf.pipelineBarrier(ilt.src_stage_mask, ilt.dst_stage_mask, {}, nullptr,
	    nullptr, barrier);
}
