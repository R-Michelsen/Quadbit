#pragma once

#include <Windows.h>
#include <EASTL/hash_map.h>
#include <EASTL/string.h>
#include <EASTL/unique_ptr.h>

#include "Engine/Rendering/RenderTypes.h"
#include "Engine/Rendering/VulkanTypes.h"

namespace Quadbit {
	// Forward declarations
	class PBRPipeline;
	class ImGuiPipeline;
	class SkyPipeline;
	class EntityManager;
	struct Entity;

	class QbVkRenderer {
	public:
		QbVkRenderer(HINSTANCE hInstance, HWND hwnd, InputHandler* const inputHandler, EntityManager* const entityManager);
		~QbVkRenderer();

		void DrawFrame();
		void DestroyResource(QbVkBuffer buffer);
		void DestroyResource(QbVkTexture texture);

	private:
		// API Classes can access internals to provide functionality to the user
		friend class Graphics;
		friend class Compute;

		bool canRender_ = false;

		HINSTANCE localHandle_ = NULL;
		HWND windowHandle_ = NULL;

		VkInstance instance_ = VK_NULL_HANDLE;
		eastl::unique_ptr<QbVkContext> context_;
		eastl::unique_ptr<PBRPipeline> pbrPipeline_;
		eastl::unique_ptr<ImGuiPipeline> imGuiPipeline_;
		eastl::unique_ptr<SkyPipeline> skyPipeline_;


		// DEBUG BUILD ONLY
#ifndef NDEBUG
		VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
		void CreateDebugMessenger();
#endif

		void CreateInstance();
		void CreateSurface();
		void CreatePhysicalDevice();
		void CreateLogicalDeviceAndQueues();

		void CreateCommandPool();
		void CreateSyncObjects();
		void AllocateCommandBuffers();

		void CreateMultisamplingResources();
		void CreateDepthResources();
		void CreateShadowmapResources();

		void CreateSwapChain();
		void RecreateSwapchain();
		void CreateMainRenderPass();

		void ImGuiUpdateContent();
		void PrepareFrame(uint32_t resourceIndex, VkCommandBuffer commandBuffer, VkFramebuffer& framebuffer, VkImageView imageView);

		void Rebuild();
	};
}