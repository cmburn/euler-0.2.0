/* SPDX-License-Identifier: ISC */

#ifndef EULER_VULKAN_SWAPCHAIN_H
#define EULER_VULKAN_SWAPCHAIN_H

#include <vulkan/vulkan_raii.hpp>

#include "euler/util/object.h"

namespace euler::vulkan {
class Surface;
class Renderer;

/* Swapchain is embedded directly in Surface, so we don't need to worry about
 * inheriting util::Object, we're only a separate class for readability. */
class Swapchain final {
public:
	struct Capabilities {
		vk::SurfaceCapabilitiesKHR capabilities = {};
		uint32_t image_count;
		vk::SurfaceFormatKHR surface_format;
		vk::PresentModeKHR present_mode;
		Capabilities(const vk::raii::PhysicalDevice &pd,
		    const vk::raii::SurfaceKHR &surface);
	};

	Swapchain(Surface *surface, const Capabilities *capabilities,
	    vk::raii::SwapchainKHR &&sc = nullptr);

	Surface *
	surface() const
	{
		return _surface;
	}

	vk::raii::SwapchainKHR
	take_swapchain()
	{
		auto sc = std::move(_swapchain);
		_swapchain = nullptr;
		return sc;
	}

private:
	vk::raii::SwapchainKHR create_swapchain(
	    vk::raii::SwapchainKHR &&sc) const;
	std::vector<vk::raii::Image> create_images() const;
	std::vector<vk::raii::ImageView> create_image_views() const;
	util::Reference<Renderer> renderer() const;

	Surface *_surface;
	const Capabilities *_capabilities;
	vk::Extent2D _extent;
	vk::raii::SwapchainKHR _swapchain;
	std::vector<vk::raii::Image> _images;
	std::vector<vk::raii::ImageView> _image_views;
	vk::raii::PipelineLayout _pipeline_layout = nullptr;
	vk::raii::Pipeline _pipeline = nullptr;
};
} /* namespace euler::vulkan */

#endif /* EULER_VULKAN_SWAPCHAIN_H */
