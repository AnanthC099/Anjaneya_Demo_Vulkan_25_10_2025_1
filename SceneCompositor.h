#pragma once
#define MYICON 1001

#ifndef RC_INVOKED
#include <windows.h>
#include <vulkan/vulkan.h>
#include "glm/glm.hpp"

// Scene management
enum SceneType
{
    SCENE_NONE = -1,
    SCENE_0 = 0,
    SCENE_1 = 1,
    SCENE_2 = 2
};

// Transition states
enum TransitionState
{
    TRANSITION_NONE,
    TRANSITION_IN_PROGRESS,
    TRANSITION_COMPLETE
};

// Off-screen texture structure for each scene
struct OffScreenTexture
{
    VkImage image;
    VkDeviceMemory memory;
    VkImageView imageView;
    VkFramebuffer framebuffer;
    VkRenderPass renderPass;
    VkCommandBuffer commandBuffer;
    VkSemaphore renderCompleteSemaphore;
    VkFence renderCompleteFence;
};

// Scene transition data
struct SceneTransition
{
    SceneType currentScene;
    SceneType targetScene;
    TransitionState state;
    float transitionTime;
    float transitionDuration;
    float transitionParameter; // 0.0 = current scene, 1.0 = target scene
};

// Function declarations
void SwitchToScene(SceneType targetScene);
void UpdateSceneTransition();
VkResult CreateOffScreenTextures();
uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
VkResult RenderSceneToTexture(SceneType sceneType, OffScreenTexture* targetTexture);
VkResult RenderScene0Placeholder(OffScreenTexture* targetTexture);
VkResult RenderScene1Placeholder(OffScreenTexture* targetTexture);
VkResult RenderScene2Placeholder(OffScreenTexture* targetTexture);
VkResult CreateCompositorPipeline();
VkResult CreateCompositorDescriptorSet();
VkResult RenderCompositor();
VkShaderModule LoadShaderModule(const char* filename);

#endif // RC_INVOKED
