/* SPDX-License-Identifier: ISC */

#include <unordered_set>

#include "euler/vulkan/renderer.h"
#include "euler/vulkan/surface.h"
#include "euler/vulkan/swapchain.h"

static vk::SurfaceFormatKHR
select_surface_format(const vk::raii::PhysicalDevice &pd,
    const vk::raii::SurfaceKHR &surface)
{
	auto formats = pd.getSurfaceFormatsKHR(*surface);
	std::ranges::stable_sort(formats, [](const auto &a, const auto &b) {
		/* TODO: more criteria */
		if (a.colorSpace != b.colorSpace) {
			if (a.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return true;
			if (b.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
				return false;
		}
		if (a.format != b.format) {
			if (a.format == vk::Format::eB8G8R8A8Srgb) return true;
			if (b.format == vk::Format::eB8G8R8A8Srgb) return false;
		}
		return false;
	});
	return formats.front();
}

static vk::PresentModeKHR
select_present_mode(const vk::raii::PhysicalDevice &pd,
    const vk::raii::SurfaceKHR &surface)
{
	static constexpr std::array PRESENT_MODES = {
		vk::PresentModeKHR::eMailbox,
		vk::PresentModeKHR::eFifo, /* guaranteed */
	};
	std::unordered_set<vk::PresentModeKHR> available_modes;
	for (auto pm : pd.getSurfacePresentModesKHR(*surface))
		available_modes.insert(pm);
	for (auto pm : PRESENT_MODES)
		if (available_modes.contains(pm)) return pm;
	return vk::PresentModeKHR::eFifo;
}

static uint32_t
select_image_count(const vk::SurfaceCapabilitiesKHR &sc)
{
	auto n = std::max(static_cast<uint32_t>(3), sc.minImageCount);
	const auto max = sc.maxImageCount;
	if (max > 0 && n > max) n = max;
	return n;
}

euler::vulkan::Swapchain::Capabilities::Capabilities(
    const vk::raii::PhysicalDevice &pd, const vk::raii::SurfaceKHR &surface)
    : capabilities(pd.getSurfaceCapabilitiesKHR(*surface))
    , image_count(select_image_count(capabilities))
    , surface_format(select_surface_format(pd, surface))
    , present_mode(select_present_mode(pd, surface))
{
}

euler::vulkan::Swapchain::Swapchain(Surface *surface,
    const Capabilities *capabilities, vk::raii::SwapchainKHR &&sc)
    : _surface(surface)
    , _capabilities(capabilities)
    , _extent(surface->extent())
    , _swapchain(create_swapchain(std::move(sc)))
    , _images(create_images())
    , _image_views(create_image_views())
{
}

vk::raii::SwapchainKHR
euler::vulkan::Swapchain::create_swapchain(vk::raii::SwapchainKHR &&sc) const
{
	vk::SwapchainCreateInfoKHR create_info = {
		.surface = *_surface->surface(),
		.minImageCount = _capabilities->image_count,
		.imageFormat = _capabilities->surface_format.format,
		.imageColorSpace = _capabilities->surface_format.colorSpace,
		.imageExtent = _extent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = _capabilities->capabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = _capabilities->present_mode,
		.clipped = VK_TRUE,
	};
	if (sc != nullptr) create_info.oldSwapchain = *sc;
	return vk::raii::SwapchainKHR(renderer()->device(), create_info);
}

std::vector<vk::raii::Image>
euler::vulkan::Swapchain::create_images() const
{
	std::vector<vk::raii::Image> images;
	const auto swap_images = _swapchain.getImages();
	images.reserve(swap_images.size());
	for (const auto &image : swap_images)
		images.emplace_back(this->renderer()->device(), image);
	return images;
}

std::vector<vk::raii::ImageView>
euler::vulkan::Swapchain::create_image_views() const
{
	std::vector<vk::raii::ImageView> views;
	vk::ImageViewCreateInfo create_info = {
		.viewType = vk::ImageViewType::e2D,
		.format = _capabilities->surface_format.format,
		.components = {
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity,
		},
		.subresourceRange ={
			.aspectMask =vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		},
	};
	views.reserve(_images.size());
	for (const auto &image : _images) {
		create_info.image = *image;
		views.emplace_back(renderer()->device(), create_info);
	}
	return views;
}

euler::util::Reference<euler::vulkan::Renderer>
euler::vulkan::Swapchain::renderer() const
{
	return _surface->renderer();
}