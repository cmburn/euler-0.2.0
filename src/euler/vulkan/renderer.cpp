/* SPDX-License-Identifier: ISC */

#include "euler/vulkan/renderer.h"

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <unordered_map>

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_vulkan.h>

#include "euler/util/logger.h"
#include "euler/util/version.h"

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

static std::vector<const char *>
instance_extensions(const LogRef &log)
{
	Renderer::global_init();
	static bool initialized = false;
	static std::mutex vec_mutex;
	static std::vector<const char *> vec;
	/* could cause trouble if we're trying to create two at once */
	std::lock_guard lock(vec_mutex);
	if (initialized) return vec;
	uint32_t count;
	const auto *exts = SDL_Vulkan_GetInstanceExtensions(&count);
	if (exts == nullptr) {
		if (log != nullptr) {
			log->fatal(
			    "Failed to get SDL Vulkan instance extensions");
		}
		fprintf(stderr,
		    "Failed to get SDL Vulkan instance extensions\n");
		std::exit(EXIT_FAILURE);
	}
	vec.reserve(count);
	for (uint32_t i = 0; i < count; i++) vec.emplace_back(exts[i]);
	initialized = true;
	if (vec.empty()) return vec;
	if (log != nullptr)
		log->info("Available SDL Vulkan instance extensions:");
	for (const auto s : vec) log->info("\t- {}", s);
	log->info("");
	return vec;
}

Renderer::Renderer(const util::Reference<util::State> &state)
    : _log(state->log())
    , _state(state.weaken())
    , _preferred_gpu(state->preferred_gpu())
    , _context(get_instance_proc_addr())
    , _instance(create_instance())
    , _physical_device(select_physical_device())
    , _graphics_index(find_graphics_present_queue())
    , _compute_index(find_compute_queue())
    , _device(create_logical_device())
    , _allocator(make_allocator())
    , _graphics_queue(_device, _graphics_index, 0)
{
}

Renderer::~Renderer() { vmaDestroyAllocator(_allocator); }

euler::util::Reference<euler::util::State>
Renderer::state()
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
	    vk::PhysicalDeviceVulkan14Features,
	    vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
	if (!features.get<vk::PhysicalDeviceVulkan11Features>()
	        .shaderDrawParameters) {
		return false;
	}
	if (!features.get<vk::PhysicalDeviceVulkan13Features>()
	        .dynamicRendering) {
		return false;
	}
	if (!features.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
	        .extendedDynamicState) {
		return false;
	}
	return true;
}

static std::optional<vk::raii::PhysicalDevice>
first_compatible(VkInstance instance,
    const std::vector<vk::raii::PhysicalDevice> &devices)
{
	/* TODO: Need to verify with SDL_Vulkan that device supports surfaces */
	for (const auto &pd : devices) {
		if (is_compatible(instance, pd)) return pd;
	}
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
	bool found_preferred = rank_physical_devices(devices, _preferred_gpu);
	if (_preferred_gpu.has_value() && !found_preferred) {
		_log->warn("Unable to find user-specified graphics "
		           "device with ID {}",
		    _preferred_gpu.value());
	}
	const auto pd = first_compatible(*_instance, devices);
	if (!pd.has_value())
		_log->fatal("No compatible Vulkan physical device found");
	const auto props = pd->getProperties();
	_log->info("Selected device with ID {} ({})", props.deviceID,
	    props.deviceName.data());
	return pd.value();
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
