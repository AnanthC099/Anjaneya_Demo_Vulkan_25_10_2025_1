#pragma once
#define MYICON 1001

#ifndef RC_INVOKED
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include <stdint.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

void SceneCompositor_SetSecondaryTexture(VkImageView imageView, VkSampler sampler);
void SceneCompositor_SetBlendFactor(float blendFactor);
VkResult SceneCompositor_BeginOffscreenRender(uint32_t index);
VkResult SceneCompositor_EndOffscreenRender(uint32_t index);
VkImage SceneCompositor_GetOffscreenColorImage(uint32_t index);
VkImageView SceneCompositor_GetOffscreenColorImageView(uint32_t index);
VkSampler SceneCompositor_GetSharedSampler(void);
VkExtent2D SceneCompositor_GetOffscreenExtent(void);
VkImage SceneCompositor_GetOffscreenDepthImage(void);
VkImageView SceneCompositor_GetOffscreenDepthImageView(void);

#ifdef __cplusplus
}
#endif
#endif

