#pragma once
#define MYICON 1001

#ifndef RC_INVOKED
#include <windows.h>
#include <vulkan/vulkan.h>

// Off-screen rendering support
VkResult RenderToOffScreenTexture(VkImageView targetImageView, VkFramebuffer targetFramebuffer, VkRenderPass targetRenderPass);

#endif // RC_INVOKED
