#pragma once

#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <windows.h>

#ifndef RC_INVOKED
#include <vulkan/vulkan.h>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#endif

#define MYICON 101
#define MYAUDIO 102

struct GlobalContext_Scene1;
struct Win32WindowContext_Scene1;

struct Win32FunctionTable_Scene1
{
    LRESULT (CALLBACK *WndProc)(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
    void (*ToggleFullscreen)(void);
    VkResult (*initialize)(void);
    VkResult (*resize)(int width, int height);
    VkResult (*display)(void);
    void (*update)(void);
    void (*uninitialize)(void);
};

struct FunctionTable_Scene1
{
    void (*SetOverlaySizeFrac)(float frac);
    void (*NudgeOverlaySizeFrac)(float delta);
    float (*ease01)(float x);
    float (*ComputeOverlayFadeForPan)(int panIndex);
    void (*BindOverlayTexture)(int which);
    float (*frand01)();
    VkCommandBuffer (*BeginOneShotCommandBuffer)(void);
    void (*EndOneShotCommandBuffer)(VkCommandBuffer commandBuffer);
    DWORD (*CalcPanDurationMs)(const glm::quat* q0, const glm::quat* q1);
    void (*SetPanSpeedDegPerSec)(float s);
    void (*BeginNewPan)();
    void (*UpdateCameraAnim)();
    float (*ComputeOverlayFade)();
    int (WINAPI *WinMain)(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow);
    VkResult (*createTexture2D)(const char* path, VkImage* outImg, VkDeviceMemory* outMem, VkImageView* outView, VkSampler* outSampler);
    VkResult (*createVulkanInstance)(void);
    VkResult (*fillInstanceExtensionNames)(void);
    VkResult (*fillValidationLayerNames)(void);
    VkResult (*createValidationCallbackFunction)(void);
    VkResult (*getSupportedSurface)(void);
    VkResult (*getPhysicalDevice)(void);
    VkResult (*printVKInfo)(void);
    VkResult (*fillDeviceExtensionNames)(void);
    VkResult (*createVulkanDevice)(void);
    void (*getDeviceQueue)(void);
    VkResult (*getPhysicalDeviceSurfaceFormatAndColorSpace)(void);
    VkResult (*getPhysicalDevicePresentMode)(void);
    VkResult (*createSwapchain)(VkBool32 vsync);
    VkResult (*createImagesAndImageViews)(void);
    VkResult (*createCommandPool)(void);
    VkResult (*getSupportedDepthFormat)(void);
    VkResult (*createCommandBuffers)(void);
    VkResult (*createVertexBuffer)(void);
    VkResult (*createTexture)(const char* textureFileName[]);
    VkResult (*createUniformBuffer)(void);
    VkResult (*updateUniformBuffer)(void);
    VkResult (*createShaders)(void);
    VkResult (*createDescriptorSetLayout)(void);
    VkResult (*createPipelineLayout)(void);
    VkResult (*createDescriptorPool)(void);
    VkResult (*createDescriptorSet)(void);
    VkResult (*createRenderpass)(void);
    VkResult (*createPipeline)(void);
    VkResult (*createFrameBuffers)(void);
    VkResult (*createSemaphore)(void);
    VkResult (*createFences)(void);
    VkResult (*buildCommandBuffers)(void);
    PFN_vkDebugReportCallbackEXT debugReportCallback;
};

extern Win32WindowContext_Scene1 g_win32WindowCtxScene1;
extern GlobalContext_Scene1 g_ctxScene1;
extern Win32FunctionTable_Scene1 gWin32FunctionTable_Scene1;
extern FunctionTable_Scene1 gFunctionTable_Scene1;

void InitializeFunctionTable_Scene1(void);
