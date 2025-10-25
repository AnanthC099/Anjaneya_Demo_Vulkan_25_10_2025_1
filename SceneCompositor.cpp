// SceneCompositor.cpp
// Off-screen render of Scene0 and Scene1 (color+depth), then full-screen composite mix with parameter t.
// C-style Vulkan (no C++ except glm). Allman braces. Win32 + Vulkan KHR swapchain.

#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cstdint>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Sphere.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "Sphere.lib")

#define WIN_WIDTH   1280
#define WIN_HEIGHT   720
#define LOG_FILE   (char*)"Compositor_Log.txt"

#define APP_NAME   "ARTR_Compositor"

static const double SCENE_HOLD_DURATION_SECONDS = 5.0;
static const double SCENE_CROSSFADE_DURATION_SECONDS = 1.0;
static const float  SCENE0_ANGLE_SPEED_DEG_PER_SEC = 24.0f;
static const float  SCENE1_ANGLE_SPEED_DEG_PER_SEC = 36.0f;

enum ScenePhase
{
    ScenePhase_Scene0Hold = 0,
    ScenePhase_FadeToScene1,
    ScenePhase_Scene1Hold,
    ScenePhase_FadeToScene0
};

// == Forward Decls ==
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);

// == Helpers ==
static void FatalMsgA(const char* title, const char* msg)
{
    MessageBoxA(NULL, msg, title, MB_OK | MB_ICONERROR);
}

// ===================================================================================
// Global Types / State (C-style)
// ===================================================================================

struct WindowState
{
    HWND hwnd;
    BOOL active;
    BOOL minimized;
    BOOL fullscreen;
    DWORD style;
    WINDOWPLACEMENT wp;
};

struct Buffer
{
    VkBuffer        buf;
    VkDeviceMemory  mem;
    VkDeviceSize    size;
};

struct ImageRT
{
    VkImage         image;
    VkDeviceMemory  mem;
    VkImageView     view;
    VkFormat        format;
    VkExtent2D      extent;
    VkImageLayout   layout;
};

struct OffscreenPass
{
    // color + depth
    ImageRT         color;
    ImageRT         depth;
    VkFramebuffer   framebuffer;
};

struct SwapchainTargets
{
    VkSwapchainKHR      swapchain;
    VkImage*            images;
    VkImageView*        views;
    uint32_t            count;
    VkExtent2D          extent;
    VkFormat            format;
    ImageRT             depth;
    VkFramebuffer*      framebuffers;
};

struct Scene0UniformData
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    glm::vec4 lightAmbient[3];
    glm::vec4 lightDiffuse[3];
    glm::vec4 lightSpecular[3];
    glm::vec4 lightPosition[3];

    glm::vec4 materialAmbient;
    glm::vec4 materialDiffuse;
    glm::vec4 materialSpecular;
    float     materialShininess;
    uint32_t  lKeyIsPressed;
};

struct Scene1UniformData
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    glm::vec4 lightAmbient[2];
    glm::vec4 lightDiffuse[2];
    glm::vec4 lightSpecular[2];
    glm::vec4 lightPosition[2];

    glm::vec4 materialAmbient;
    glm::vec4 materialDiffuse;
    glm::vec4 materialSpecular;
    float     materialShininess;
    uint32_t  lKeyIsPressed;
    glm::vec2 padding;
};

struct Scene2UniformData
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};

static const float SCENE0_LIGHT_RADIUS = 100.0f;

// Global State
static struct Global
{
    FILE*               log;

    WindowState         win;

    uint32_t            winWidth;
    uint32_t            winHeight;
    BOOL                initialized;

    // Vulkan core
    VkInstance          instance;
    VkDebugReportCallbackEXT dbgCb;
    PFN_vkDestroyDebugReportCallbackEXT pfnDestroyDbg;

    VkSurfaceKHR        surface;

    VkPhysicalDevice    phys;
    VkDevice            device;
    VkQueue             queue;
    uint32_t            qFamIndex;

    VkPhysicalDeviceMemoryProperties memProps;

    // Swapchain & present
    SwapchainTargets    sc;
    VkRenderPass        rpPresent;     // color (+ optional depth)
    VkRenderPass        rpOffscreen;   // color+depth with final color layout = SHADER_READ_ONLY_OPTIMAL

    VkCommandPool       cmdPool;
    VkCommandBuffer*    cmdBufs;       // per swapchain image

    VkSemaphore*        semImageAvailable;
    VkSemaphore*        semRenderFinished;
    VkFence*            fences;        // per in-flight frame
    VkFence*            imagesInFlight;
    uint32_t            maxFramesInFlight;
    uint32_t            currentFrame;

    // Offscreen passes for the two scenes
    OffscreenPass       scene0;
    OffscreenPass       scene1;
    OffscreenPass       scene2;

    // Scene 0 geometry (sphere) and resources
    Buffer              s0_vboPos;
    Buffer              s0_vboNorm;
    Buffer              s0_vboTex;
    Buffer              s0_ibo;
    uint32_t            s0_vertexCount;
    uint32_t            s0_indexCount;

    // Scene 1 geometry (pyramid) and resources
    Buffer              s1_vboPos;
    Buffer              s1_vboNorm;
    uint32_t            s1_vertexCount;

    // UBOs (one per scene)
    Buffer              ubo0;
    Buffer              ubo1;
    Buffer              ubo2;

    // Scene pipelines (reusing your SPIR-V)
    VkShaderModule      s0_vs;
    VkShaderModule      s0_fs;
    VkShaderModule      s1_vs;
    VkShaderModule      s1_fs;
    VkShaderModule      s2_vs;
    VkShaderModule      s2_fs;

    VkDescriptorSetLayout sUboDSL;     // binding 0: uniform buffer
    VkPipelineLayout    s0_pLayout;
    VkPipelineLayout    s1_pLayout;
    VkPipelineLayout    s2_pLayout;
    VkDescriptorPool    sDescPool;
    VkDescriptorSet     s0_descSet;
    VkDescriptorSet     s1_descSet;
    VkDescriptorSetLayout s2_DSL;     // binding 0: UBO, binding 1: combined sampler
    VkDescriptorPool    s2DescPool;
    VkDescriptorSet     s2_descSet;
    VkPipeline          s0_pipe;
    VkPipeline          s1_pipe;
    VkPipeline          s2_pipe;

    // Composite (fullscreen)
    VkShaderModule      comp_vs;
    VkShaderModule      comp_fs;
    VkDescriptorSetLayout compDSL;     // binding 0: sampler2D scene0, 1: sampler2D scene1
    VkPipelineLayout    comp_pLayout;  // push constant float t
    VkDescriptorPool    compPool;
    VkDescriptorSet     compDescSet;
    VkPipeline          compPipe;
    VkSampler           compSampler;

    // Scene 2 resources (textured quad)
    Buffer              s2_vboPos;
    Buffer              s2_vboTex;
    uint32_t            s2_vertexCount;
    VkSampler           s2_sampler;
    ImageRT             s2_tex;

    // Animation/state
    float               angle0;
    float               angle1;
    float               t;
    int                 phase;
    double              phaseTime;
    LARGE_INTEGER       qpcFreq;
    LARGE_INTEGER       lastQpc;
    BOOL                lighting;
    uint32_t            selectedScene; // 0,1,2 by key press

    // Config
    BOOL                validation;
    VkBool32            hasDebugReportExt;

} g;

// ===================================================================================
// Validation helpers
// ===================================================================================

static BOOL HasInstanceExtension(const char* name)
{
    uint32_t count = 0;
    if (vkEnumerateInstanceExtensionProperties(NULL, &count, NULL) != VK_SUCCESS || count == 0)
    {
        return FALSE;
    }

    VkExtensionProperties* props = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * count);
    if (!props)
    {
        return FALSE;
    }

    BOOL found = FALSE;
    if (vkEnumerateInstanceExtensionProperties(NULL, &count, props) == VK_SUCCESS)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            if (strcmp(props[i].extensionName, name) == 0)
            {
                found = TRUE;
                break;
            }
        }
    }

    free(props);
    return found;
}

static BOOL HasValidationLayer(const char* name)
{
    uint32_t count = 0;
    if (vkEnumerateInstanceLayerProperties(&count, NULL) != VK_SUCCESS || count == 0)
    {
        return FALSE;
    }

    VkLayerProperties* props = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * count);
    if (!props)
    {
        return FALSE;
    }

    BOOL found = FALSE;
    if (vkEnumerateInstanceLayerProperties(&count, props) == VK_SUCCESS)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            if (strcmp(props[i].layerName, name) == 0)
            {
                found = TRUE;
                break;
            }
        }
    }

    free(props);
    return found;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(VkDebugReportFlagsEXT flags,
                                                   VkDebugReportObjectTypeEXT /*objType*/,
                                                   uint64_t /*object*/,
                                                   size_t /*location*/,
                                                   int32_t /*messageCode*/,
                                                   const char* pLayerPrefix,
                                                   const char* pMessage,
                                                   void* /*pUserData*/)
{
    if (g.log)
    {
        const char* severity = "INFO";
        if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) severity = "ERROR";
        else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) severity = "WARN";
        else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) severity = "PERF";
        fprintf(g.log, "[Validation][%s][%s]: %s\n",
                severity,
                pLayerPrefix ? pLayerPrefix : "",
                pMessage ? pMessage : "");
        fflush(g.log);
    }
    return VK_FALSE;
}

static VkResult SetupValidationDebug(void)
{
    if (!g.validation)
    {
        return VK_SUCCESS;
    }

    PFN_vkCreateDebugReportCallbackEXT pfnCreate =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(g.instance, "vkCreateDebugReportCallbackEXT");
    g.pfnDestroyDbg =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(g.instance, "vkDestroyDebugReportCallbackEXT");

    if (!pfnCreate || !g.pfnDestroyDbg)
    {
        if (g.log)
        {
            fprintf(g.log, "SetupValidationDebug(): Debug report functions unavailable. Disabling validation callback.\n");
        }
        g.pfnDestroyDbg = NULL;
        g.validation = FALSE;
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    VkDebugReportCallbackCreateInfoEXT ci;
    memset(&ci, 0, sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
               VK_DEBUG_REPORT_WARNING_BIT_EXT |
               VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
               VK_DEBUG_REPORT_DEBUG_BIT_EXT |
               VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
    ci.pfnCallback = DebugReportCallback;

    VkResult r = pfnCreate(g.instance, &ci, NULL, &g.dbgCb);
    if (r != VK_SUCCESS)
    {
        if (g.log)
        {
            fprintf(g.log, "SetupValidationDebug(): vkCreateDebugReportCallbackEXT failed (%d).\n", r);
        }
        g.pfnDestroyDbg = NULL;
        g.validation = FALSE;
        g.dbgCb = VK_NULL_HANDLE;
        return r;
    }

    if (g.log)
    {
        fprintf(g.log, "SetupValidationDebug(): Debug report callback created.\n");
    }
    return VK_SUCCESS;
}

// ===================================================================================
// Utility: Allman-style wrappers & helpers
// ===================================================================================

static void ZeroMem(void* p, size_t sz)
{
    memset(p, 0, sz);
}

static void SetImageLayout(VkCommandBuffer cmd,
                           VkImage image,
                           VkImageAspectFlags aspect,
                           VkImageLayout oldLayout,
                           VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier;
    ZeroMem(&barrier, sizeof(barrier));
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;

    barrier.subresourceRange.aspectMask = aspect;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

    vkCmdPipelineBarrier(cmd,
                         srcStage,
                         dstStage,
                         0,
                         0, NULL,
                         0, NULL,
                         1, &barrier);
}

static int ReadFileBytes(const char* path, char** outData, size_t* outSize)
{
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    if (sz <= 0)
    {
        fclose(f);
        return 0;
    }
    fseek(f, 0, SEEK_SET);

    char* data = (char*)malloc((size_t)sz);
    if (!data)
    {
        fclose(f);
        return 0;
    }

    size_t r = fread(data, 1, (size_t)sz, f);
    fclose(f);
    if (r != (size_t)sz)
    {
        free(data);
        return 0;
    }

    *outData = data;
    *outSize = (size_t)sz;
    return 1;
}

static VkShaderModule CreateShaderModuleFromFile(VkDevice dev, const char* path)
{
    char* data = NULL;
    size_t size = 0;
    if (!ReadFileBytes(path, &data, &size))
    {
        fprintf(g.log, "CreateShaderModuleFromFile(): failed to read %s\n", path);
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo ci;
    ZeroMem(&ci, sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = size;
    ci.pCode = (const uint32_t*)data;

    VkShaderModule mod = VK_NULL_HANDLE;
    VkResult r = vkCreateShaderModule(dev, &ci, NULL, &mod);
    free(data);

    if (r != VK_SUCCESS)
    {
        fprintf(g.log, "vkCreateShaderModule() failed for %s (%d)\n", path, r);
        return VK_NULL_HANDLE;
    }
    return mod;
}

static uint32_t FindMemoryType(uint32_t typeBits, VkMemoryPropertyFlags req)
{
    for (uint32_t i = 0; i < g.memProps.memoryTypeCount; ++i)
    {
        if ((typeBits & (1u << i)) && (g.memProps.memoryTypes[i].propertyFlags & req) == req)
        {
            return i;
        }
    }
    return UINT32_MAX;
}

// ===================================================================================
// Minimal geometry (position + normal) matching Scene1 layout (12 triangles / pyramid)
// ===================================================================================

static const float sPyramidPos[] =
{
     // front
     0.0f,  1.0f,  0.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,

     // right
     0.0f,  1.0f,  0.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,

     // back
     0.0f,  1.0f,  0.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,

     // left
     0.0f,  1.0f,  0.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
};

static const float sPyramidNorm[] =
{
    // front
     0.000000f, 0.447214f,  0.894427f,
     0.000000f, 0.447214f,  0.894427f,
     0.000000f, 0.447214f,  0.894427f,

    // right
     0.894427f, 0.447214f,  0.000000f,
     0.894427f, 0.447214f,  0.000000f,
     0.894427f, 0.447214f,  0.000000f,

    // back
     0.000000f, 0.447214f, -0.894427f,
     0.000000f, 0.447214f, -0.894427f,
     0.000000f, 0.447214f, -0.894427f,

    // left
    -0.894427f, 0.447214f,  0.000000f,
    -0.894427f, 0.447214f,  0.000000f,
    -0.894427f, 0.447214f,  0.000000f,
};

// ===================================================================================
// Win32 helpers
// ===================================================================================

static void ToggleFullscreen(void)
{
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);

    if (!g.win.fullscreen)
    {
        g.win.style = GetWindowLong(g.win.hwnd, GWL_STYLE);
        if (g.win.style & WS_OVERLAPPEDWINDOW)
        {
            if (GetWindowPlacement(g.win.hwnd, &g.win.wp) &&
                GetMonitorInfo(MonitorFromWindow(g.win.hwnd, MONITORINFOF_PRIMARY), &mi))
            {
                SetWindowLong(g.win.hwnd, GWL_STYLE, g.win.style & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(g.win.hwnd, HWND_TOP,
                             mi.rcMonitor.left, mi.rcMonitor.top,
                             mi.rcMonitor.right - mi.rcMonitor.left,
                             mi.rcMonitor.bottom - mi.rcMonitor.top,
                             SWP_NOZORDER | SWP_FRAMECHANGED);
            }
        }
        ShowCursor(FALSE);
        g.win.fullscreen = TRUE;
    }
    else
    {
        SetWindowPlacement(g.win.hwnd, &g.win.wp);
        SetWindowLong(g.win.hwnd, GWL_STYLE, g.win.style | WS_OVERLAPPEDWINDOW);
        SetWindowPos(g.win.hwnd, HWND_TOP, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
        ShowCursor(TRUE);
        g.win.fullscreen = FALSE;
    }
}

// ===================================================================================
// Vulkan: creation & teardown
// ===================================================================================

static VkResult CreateInstance(void)
{
    VkApplicationInfo ai;
    ZeroMem(&ai, sizeof(ai));
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName = APP_NAME;
    ai.applicationVersion = 1;
    ai.pEngineName = APP_NAME;
    ai.engineVersion = 1;
    ai.apiVersion = VK_API_VERSION_1_3;

    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    const char* layers[1];
    uint32_t layerCount = 0;

    if (g.validation)
    {
        if (HasValidationLayer(validationLayer))
        {
            layers[layerCount++] = validationLayer;
            if (g.log)
            {
                fprintf(g.log, "CreateInstance(): Enabling validation layer %s.\n", validationLayer);
            }
        }
        else
        {
            if (g.log)
            {
                fprintf(g.log, "CreateInstance(): Validation layer %s not available. Disabling validation.\n", validationLayer);
            }
            g.validation = FALSE;
        }
    }

    g.hasDebugReportExt = VK_FALSE;
    if (g.validation)
    {
        if (HasInstanceExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
        {
            g.hasDebugReportExt = VK_TRUE;
        }
        else
        {
            if (g.log)
            {
                fprintf(g.log, "CreateInstance(): %s not available. Disabling validation.\n", VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
            }
            g.validation = FALSE;
        }
    }

    const char* exts[4];
    uint32_t extCount = 0;
    exts[extCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
    exts[extCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    if (g.validation && g.hasDebugReportExt)
    {
        exts[extCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
    }

    VkInstanceCreateInfo ci;
    ZeroMem(&ci, sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &ai;
    ci.enabledExtensionCount = extCount;
    ci.ppEnabledExtensionNames = exts;
    ci.enabledLayerCount = layerCount;
    ci.ppEnabledLayerNames = (layerCount > 0) ? layers : NULL;

    VkResult r = vkCreateInstance(&ci, NULL, &g.instance);
    if (r == VK_SUCCESS && g.validation)
    {
        VkResult dbg = SetupValidationDebug();
        if (dbg != VK_SUCCESS && g.log)
        {
            fprintf(g.log, "CreateInstance(): SetupValidationDebug failed (%d). Continuing without callback.\n", dbg);
        }
    }

    return r;
}

static VkResult CreateSurface(void)
{
    VkWin32SurfaceCreateInfoKHR ci;
    ZeroMem(&ci, sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    ci.hinstance = (HINSTANCE)GetWindowLongPtr(g.win.hwnd, GWLP_HINSTANCE);
    ci.hwnd = g.win.hwnd;
    return vkCreateWin32SurfaceKHR(g.instance, &ci, NULL, &g.surface);
}

static VkResult PickPhysicalDeviceAndQueue(void)
{
    uint32_t count = 0;
    VkResult r = vkEnumeratePhysicalDevices(g.instance, &count, NULL);
    if (r != VK_SUCCESS || count == 0)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkPhysicalDevice* devs = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * count);
    vkEnumeratePhysicalDevices(g.instance, &count, devs);

    for (uint32_t i = 0; i < count; ++i)
    {
        uint32_t qCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(devs[i], &qCount, NULL);
        if (qCount == 0) continue;

        VkQueueFamilyProperties* qprops = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * qCount);
        vkGetPhysicalDeviceQueueFamilyProperties(devs[i], &qCount, qprops);

        for (uint32_t q = 0; q < qCount; ++q)
        {
            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(devs[i], q, g.surface, &present);

            if ((qprops[q].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present)
            {
                g.phys = devs[i];
                g.qFamIndex = q;
                free(qprops);
                free(devs);
                vkGetPhysicalDeviceMemoryProperties(g.phys, &g.memProps);
                return VK_SUCCESS;
            }
        }
        free(qprops);
    }
    free(devs);
    return VK_ERROR_INITIALIZATION_FAILED;
}

static VkResult CreateDeviceAndQueue(void)
{
    const char* devExts[1];
    uint32_t devExtCount = 0;
    devExts[devExtCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;

    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci;
    ZeroMem(&qci, sizeof(qci));
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = g.qFamIndex;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;

    VkDeviceCreateInfo dci;
    ZeroMem(&dci, sizeof(dci));
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    dci.enabledExtensionCount = devExtCount;
    dci.ppEnabledExtensionNames = devExts;
    dci.pEnabledFeatures = NULL;

    VkResult r = vkCreateDevice(g.phys, &dci, NULL, &g.device);
    if (r != VK_SUCCESS)
    {
        return r;
    }
    vkGetDeviceQueue(g.device, g.qFamIndex, 0, &g.queue);
    return VK_SUCCESS;
}

static VkSurfaceFormatKHR ChooseSurfaceFormat(void)
{
    VkSurfaceFormatKHR chosen;
    ZeroMem(&chosen, sizeof(chosen));

    uint32_t n = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(g.phys, g.surface, &n, NULL);
    VkSurfaceFormatKHR* arr = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * n);
    vkGetPhysicalDeviceSurfaceFormatsKHR(g.phys, g.surface, &n, arr);

    if (n == 1 && arr[0].format == VK_FORMAT_UNDEFINED)
    {
        chosen.format = VK_FORMAT_B8G8R8A8_UNORM;
        chosen.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    else
    {
        chosen = arr[0];
    }
    free(arr);
    return chosen;
}

static VkPresentModeKHR ChoosePresentMode(void)
{
    uint32_t n = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(g.phys, g.surface, &n, NULL);
    VkPresentModeKHR* arr = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * n);
    vkGetPhysicalDeviceSurfacePresentModesKHR(g.phys, g.surface, &n, arr);

    VkPresentModeKHR m = VK_PRESENT_MODE_FIFO_KHR;
    for (uint32_t i = 0; i < n; ++i)
    {
        if (arr[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            m = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }
    free(arr);
    return m;
}

static VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR* caps, uint32_t w, uint32_t h)
{
    if (caps->currentExtent.width != UINT32_MAX)
    {
        return caps->currentExtent;
    }

    VkExtent2D e;
    e.width = w;
    e.height = h;

    if (e.width < caps->minImageExtent.width)  e.width = caps->minImageExtent.width;
    if (e.width > caps->maxImageExtent.width)  e.width = caps->maxImageExtent.width;
    if (e.height < caps->minImageExtent.height) e.height = caps->minImageExtent.height;
    if (e.height > caps->maxImageExtent.height) e.height = caps->maxImageExtent.height;
    return e;
}

static VkResult CreateSwapchainAndTargets(uint32_t winW, uint32_t winH, SwapchainTargets* out)
{
    ZeroMem(out, sizeof(*out));

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g.phys, g.surface, &caps);

    VkSurfaceFormatKHR fmt = ChooseSurfaceFormat();
    VkPresentModeKHR pmode = ChoosePresentMode();
    VkExtent2D extent = ChooseExtent(&caps, winW, winH);

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0 && imageCount > caps.maxImageCount)
    {
        imageCount = caps.maxImageCount;
    }

    VkSwapchainCreateInfoKHR ci;
    ZeroMem(&ci, sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    ci.surface = g.surface;
    ci.minImageCount = imageCount;
    ci.imageFormat = fmt.format;
    ci.imageColorSpace = fmt.colorSpace;
    ci.imageExtent = extent;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform = (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) ?
                      VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = pmode;
    ci.clipped = VK_TRUE;
    ci.oldSwapchain = VK_NULL_HANDLE;

    VkResult r = vkCreateSwapchainKHR(g.device, &ci, NULL, &out->swapchain);
    if (r != VK_SUCCESS)
    {
        return r;
    }

    out->format = fmt.format;
    out->extent = extent;

    // images
    vkGetSwapchainImagesKHR(g.device, out->swapchain, &imageCount, NULL);
    out->count = imageCount;
    out->images = (VkImage*)malloc(sizeof(VkImage) * imageCount);
    vkGetSwapchainImagesKHR(g.device, out->swapchain, &imageCount, out->images);

    // views
    out->views = (VkImageView*)malloc(sizeof(VkImageView) * imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        VkImageViewCreateInfo vi;
        ZeroMem(&vi, sizeof(vi));
        vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        vi.image = out->images[i];
        vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vi.format = out->format;
        vi.components.r = VK_COMPONENT_SWIZZLE_R;
        vi.components.g = VK_COMPONENT_SWIZZLE_G;
        vi.components.b = VK_COMPONENT_SWIZZLE_B;
        vi.components.a = VK_COMPONENT_SWIZZLE_A;
        vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        vi.subresourceRange.baseMipLevel = 0;
        vi.subresourceRange.levelCount = 1;
        vi.subresourceRange.baseArrayLayer = 0;
        vi.subresourceRange.layerCount = 1;

        r = vkCreateImageView(g.device, &vi, NULL, &out->views[i]);
        if (r != VK_SUCCESS)
        {
            return r;
        }
    }

    return VK_SUCCESS;
}

static VkFormat PickDepthFormat(void)
{
    VkFormat cand[] =
    {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };
    for (uint32_t i = 0; i < sizeof(cand)/sizeof(cand[0]); ++i)
    {
        VkFormatProperties p;
        vkGetPhysicalDeviceFormatProperties(g.phys, cand[i], &p);
        if (p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
            return cand[i];
        }
    }
    return VK_FORMAT_UNDEFINED;
}

static VkResult CreateImageRT_Color(VkFormat fmt, VkExtent2D extent, VkImageUsageFlags usage, ImageRT* out)
{
    ZeroMem(out, sizeof(*out));
    out->format = fmt;
    out->extent = extent;

    VkImageCreateInfo ici;
    ZeroMem(&ici, sizeof(ici));
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = fmt;
    ici.extent.width = extent.width;
    ici.extent.height = extent.height;
    ici.extent.depth = 1;
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = usage; // color attachment + sampled for offscreen
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkResult r = vkCreateImage(g.device, &ici, NULL, &out->image);
    if (r != VK_SUCCESS)
    {
        return r;
    }

    VkMemoryRequirements mr;
    vkGetImageMemoryRequirements(g.device, out->image, &mr);

    uint32_t typeIndex = FindMemoryType(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (typeIndex == UINT32_MAX)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkMemoryAllocateInfo mai;
    ZeroMem(&mai, sizeof(mai));
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = mr.size;
    mai.memoryTypeIndex = typeIndex;

    r = vkAllocateMemory(g.device, &mai, NULL, &out->mem);
    if (r != VK_SUCCESS)
    {
        return r;
    }
    r = vkBindImageMemory(g.device, out->image, out->mem, 0);
    if (r != VK_SUCCESS)
    {
        return r;
    }

    VkImageViewCreateInfo vi;
    ZeroMem(&vi, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vi.image = out->image;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = fmt;
    vi.components.r = VK_COMPONENT_SWIZZLE_R;
    vi.components.g = VK_COMPONENT_SWIZZLE_G;
    vi.components.b = VK_COMPONENT_SWIZZLE_B;
    vi.components.a = VK_COMPONENT_SWIZZLE_A;
    vi.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vi.subresourceRange.baseMipLevel = 0;
    vi.subresourceRange.levelCount = 1;
    vi.subresourceRange.baseArrayLayer = 0;
    vi.subresourceRange.layerCount = 1;

    r = vkCreateImageView(g.device, &vi, NULL, &out->view);
    if (r != VK_SUCCESS)
    {
        return r;
    }

    out->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    return VK_SUCCESS;
}

static VkResult CreateImageRT_Depth(VkFormat fmt, VkExtent2D extent, ImageRT* out)
{
    ZeroMem(out, sizeof(*out));
    out->format = fmt;
    out->extent = extent;

    VkImageCreateInfo ici;
    ZeroMem(&ici, sizeof(ici));
    ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = fmt;
    ici.extent.width = extent.width;
    ici.extent.height = extent.height;
    ici.extent.depth = 1;
    ici.mipLevels = 1;
    ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VkResult r = vkCreateImage(g.device, &ici, NULL, &out->image);
    if (r != VK_SUCCESS)
    {
        return r;
    }

    VkMemoryRequirements mr;
    vkGetImageMemoryRequirements(g.device, out->image, &mr);

    uint32_t typeIndex = FindMemoryType(mr.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (typeIndex == UINT32_MAX)
    {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkMemoryAllocateInfo mai;
    ZeroMem(&mai, sizeof(mai));
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = mr.size;
    mai.memoryTypeIndex = typeIndex;

    r = vkAllocateMemory(g.device, &mai, NULL, &out->mem);
    if (r != VK_SUCCESS)
    {
        return r;
    }
    r = vkBindImageMemory(g.device, out->image, out->mem, 0);
    if (r != VK_SUCCESS)
    {
        return r;
    }

    VkImageAspectFlags aspect = (fmt == VK_FORMAT_D16_UNORM ||
                                 fmt == VK_FORMAT_D32_SFLOAT) ?
                                VK_IMAGE_ASPECT_DEPTH_BIT :
                                (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT);

    VkImageViewCreateInfo vi;
    ZeroMem(&vi, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vi.image = out->image;
    vi.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vi.format = fmt;
    vi.subresourceRange.aspectMask = aspect;
    vi.subresourceRange.baseMipLevel = 0;
    vi.subresourceRange.levelCount = 1;
    vi.subresourceRange.baseArrayLayer = 0;
    vi.subresourceRange.layerCount = 1;

    r = vkCreateImageView(g.device, &vi, NULL, &out->view);
    if (r != VK_SUCCESS)
    {
        return r;
    }
    out->layout = VK_IMAGE_LAYOUT_UNDEFINED;
    return VK_SUCCESS;
}

static VkResult CreatePresentDepth(SwapchainTargets* sc)
{
    VkFormat df = PickDepthFormat();
    return CreateImageRT_Depth(df, sc->extent, &sc->depth);
}

static VkResult CreateRenderPassOffscreen(VkFormat colorFmt, VkFormat depthFmt, VkRenderPass* out)
{
    VkAttachmentDescription atts[2];
    ZeroMem(atts, sizeof(atts));

    atts[0].format = colorFmt;
    atts[0].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    atts[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // for sampling later

    atts[1].format = depthFmt;
    atts[1].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef;
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef;
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub;
    ZeroMem(&sub, sizeof(sub));
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &colorRef;
    sub.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo rpci;
    ZeroMem(&rpci, sizeof(rpci));
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 2;
    rpci.pAttachments = atts;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &sub;

    return vkCreateRenderPass(g.device, &rpci, NULL, out);
}

static VkResult CreateRenderPassPresent(VkFormat colorFmt, VkFormat depthFmt, VkRenderPass* out)
{
    VkAttachmentDescription atts[2];
    ZeroMem(atts, sizeof(atts));

    atts[0].format = colorFmt;
    atts[0].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    atts[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    atts[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    atts[1].format = depthFmt;
    atts[1].samples = VK_SAMPLE_COUNT_1_BIT;
    atts[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;     // depth not used in composite pass
    atts[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    atts[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    atts[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    atts[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorRef;
    colorRef.attachment = 0;
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef;
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription sub;
    ZeroMem(&sub, sizeof(sub));
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &colorRef;
    sub.pDepthStencilAttachment = &depthRef;

    VkRenderPassCreateInfo rpci;
    ZeroMem(&rpci, sizeof(rpci));
    rpci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpci.attachmentCount = 2;
    rpci.pAttachments = atts;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &sub;

    return vkCreateRenderPass(g.device, &rpci, NULL, out);
}

static VkResult CreateOffscreenPass(OffscreenPass* p, VkFormat colorFmt, VkFormat depthFmt, VkExtent2D extent)
{
    VkResult r = CreateImageRT_Color(colorFmt, extent,
                                     VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                     &p->color);
    if (r != VK_SUCCESS) return r;

    r = CreateImageRT_Depth(depthFmt, extent, &p->depth);
    if (r != VK_SUCCESS) return r;

    VkImageView atts[2] = { p->color.view, p->depth.view };

    VkFramebufferCreateInfo fci;
    ZeroMem(&fci, sizeof(fci));
    fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fci.renderPass = g.rpOffscreen;
    fci.attachmentCount = 2;
    fci.pAttachments = atts;
    fci.width = extent.width;
    fci.height = extent.height;
    fci.layers = 1;

    return vkCreateFramebuffer(g.device, &fci, NULL, &p->framebuffer);
}

static VkResult CreatePresentFramebuffers(SwapchainTargets* sc)
{
    sc->framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * sc->count);
    for (uint32_t i = 0; i < sc->count; ++i)
    {
        VkImageView atts[2] =
        {
            sc->views[i],
            sc->depth.view
        };

        VkFramebufferCreateInfo fci;
        ZeroMem(&fci, sizeof(fci));
        fci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fci.renderPass = g.rpPresent;
        fci.attachmentCount = 2;
        fci.pAttachments = atts;
        fci.width = sc->extent.width;
        fci.height = sc->extent.height;
        fci.layers = 1;

        VkResult r = vkCreateFramebuffer(g.device, &fci, NULL, &sc->framebuffers[i]);
        if (r != VK_SUCCESS)
        {
            return r;
        }
    }
    return VK_SUCCESS;
}

static VkResult CreateCommandPoolAndBuffers(void)
{
    if (g.cmdPool)
    {
        vkDestroyCommandPool(g.device, g.cmdPool, NULL);
        g.cmdPool = VK_NULL_HANDLE;
    }

    VkCommandPoolCreateInfo pci;
    ZeroMem(&pci, sizeof(pci));
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.queueFamilyIndex = g.qFamIndex;
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VkResult r = vkCreateCommandPool(g.device, &pci, NULL, &g.cmdPool);
    if (r != VK_SUCCESS)
    {
        return r;
    }

    g.cmdBufs = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * g.sc.count);
    if (!g.cmdBufs)
    {
        vkDestroyCommandPool(g.device, g.cmdPool, NULL);
        g.cmdPool = VK_NULL_HANDLE;
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    VkCommandBufferAllocateInfo ai;
    ZeroMem(&ai, sizeof(ai));
    ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    ai.commandPool = g.cmdPool;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = g.sc.count;

    r = vkAllocateCommandBuffers(g.device, &ai, g.cmdBufs);
    if (r != VK_SUCCESS)
    {
        free(g.cmdBufs);
        g.cmdBufs = NULL;
        vkDestroyCommandPool(g.device, g.cmdPool, NULL);
        g.cmdPool = VK_NULL_HANDLE;
        return r;
    }
    return VK_SUCCESS;
}

static VkResult CreateSyncObjects(void)
{
    g.maxFramesInFlight = (g.sc.count > 0) ? g.sc.count : 1;
    g.currentFrame = 0;

    size_t semSz = sizeof(VkSemaphore) * g.maxFramesInFlight;
    g.semImageAvailable = (VkSemaphore*)malloc(semSz);
    g.semRenderFinished = (VkSemaphore*)malloc(semSz);
    g.fences = (VkFence*)malloc(sizeof(VkFence) * g.maxFramesInFlight);
    g.imagesInFlight = (VkFence*)malloc(sizeof(VkFence) * g.sc.count);

    if (!g.semImageAvailable || !g.semRenderFinished || !g.fences || !g.imagesInFlight)
    {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    ZeroMem(g.semImageAvailable, semSz);
    ZeroMem(g.semRenderFinished, semSz);
    ZeroMem(g.fences, sizeof(VkFence) * g.maxFramesInFlight);
    ZeroMem(g.imagesInFlight, sizeof(VkFence) * g.sc.count);

    VkSemaphoreCreateInfo si;
    ZeroMem(&si, sizeof(si));
    si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (uint32_t i = 0; i < g.maxFramesInFlight; ++i)
    {
        VkFenceCreateInfo fi;
        ZeroMem(&fi, sizeof(fi));
        fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        VkResult r = vkCreateSemaphore(g.device, &si, NULL, &g.semImageAvailable[i]);
        if (r != VK_SUCCESS) return r;
        r = vkCreateSemaphore(g.device, &si, NULL, &g.semRenderFinished[i]);
        if (r != VK_SUCCESS) return r;
        r = vkCreateFence(g.device, &fi, NULL, &g.fences[i]);
        if (r != VK_SUCCESS) return r;
    }

    for (uint32_t i = 0; i < g.sc.count; ++i)
    {
        g.imagesInFlight[i] = VK_NULL_HANDLE;
    }
    return VK_SUCCESS;
}

// ===================================================================================
// Buffers & descriptors
// ===================================================================================

static VkResult CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memFlags, Buffer* out)
{
    ZeroMem(out, sizeof(*out));
    out->size = size;

    VkBufferCreateInfo bci;
    ZeroMem(&bci, sizeof(bci));
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = size;
    bci.usage = usage;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkResult r = vkCreateBuffer(g.device, &bci, NULL, &out->buf);
    if (r != VK_SUCCESS) return r;

    VkMemoryRequirements mr;
    vkGetBufferMemoryRequirements(g.device, out->buf, &mr);

    uint32_t typeIndex = FindMemoryType(mr.memoryTypeBits, memFlags);
    if (typeIndex == UINT32_MAX) return VK_ERROR_MEMORY_MAP_FAILED;

    VkMemoryAllocateInfo mai;
    ZeroMem(&mai, sizeof(mai));
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = mr.size;
    mai.memoryTypeIndex = typeIndex;

    r = vkAllocateMemory(g.device, &mai, NULL, &out->mem);
    if (r != VK_SUCCESS) return r;

    r = vkBindBufferMemory(g.device, out->buf, out->mem, 0);
    return r;
}

static VkResult UploadBuffer(const void* src, VkDeviceSize size, Buffer* dst)
{
    void* p = NULL;
    VkResult r = vkMapMemory(g.device, dst->mem, 0, size, 0, &p);
    if (r != VK_SUCCESS)
    {
        return r;
    }
    memcpy(p, src, (size_t)size);
    vkUnmapMemory(g.device, dst->mem);
    return VK_SUCCESS;
}

static VkResult CreateSceneDescriptorStuff(void)
{
    // layout: binding 0 = uniform buffer (VS + FS)
    VkDescriptorSetLayoutBinding b;
    ZeroMem(&b, sizeof(b));
    b.binding = 0;
    b.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    b.descriptorCount = 1;
    b.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo dslci;
    ZeroMem(&dslci, sizeof(dslci));
    dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    dslci.bindingCount = 1;
    dslci.pBindings = &b;

    VkResult r = vkCreateDescriptorSetLayout(g.device, &dslci, NULL, &g.sUboDSL);
    if (r != VK_SUCCESS) return r;

    // pool
    VkDescriptorPoolSize psize;
    ZeroMem(&psize, sizeof(psize));
    psize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    psize.descriptorCount = 3; // scene0 + scene1 + scene2 ubo

    VkDescriptorPoolCreateInfo dpci;
    ZeroMem(&dpci, sizeof(dpci));
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.maxSets = 3;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes = &psize;

    r = vkCreateDescriptorPool(g.device, &dpci, NULL, &g.sDescPool);
    if (r != VK_SUCCESS) return r;

    // alloc sets
    VkDescriptorSetLayout layouts[3] = { g.sUboDSL, g.sUboDSL, g.sUboDSL };

    VkDescriptorSetAllocateInfo dsai;
    ZeroMem(&dsai, sizeof(dsai));
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = g.sDescPool;
    dsai.descriptorSetCount = 3;
    dsai.pSetLayouts = layouts;

    VkDescriptorSet sets[3];
    r = vkAllocateDescriptorSets(g.device, &dsai, sets);
    if (r != VK_SUCCESS) return r;
    g.s0_descSet = sets[0];
    g.s1_descSet = sets[1];
    g.s2_descSet = sets[2];

    // update
    VkDescriptorBufferInfo bi0;
    bi0.buffer = g.ubo0.buf;
    bi0.offset = 0;
    bi0.range  = sizeof(Scene0UniformData);

    VkDescriptorBufferInfo bi1;
    bi1.buffer = g.ubo1.buf;
    bi1.offset = 0;
    bi1.range  = sizeof(Scene1UniformData);

    VkDescriptorBufferInfo bi2;
    bi2.buffer = g.ubo2.buf;
    bi2.offset = 0;
    bi2.range  = sizeof(Scene2UniformData);

    VkWriteDescriptorSet writes[3];
    ZeroMem(writes, sizeof(writes));

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = g.s0_descSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[0].pBufferInfo = &bi0;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = g.s1_descSet;
    writes[1].dstBinding = 0;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[1].pBufferInfo = &bi1;

    writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[2].dstSet = g.s2_descSet;
    writes[2].dstBinding = 0;
    writes[2].descriptorCount = 1;
    writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writes[2].pBufferInfo = &bi2;

    vkUpdateDescriptorSets(g.device, 3, writes, 0, NULL);
    return VK_SUCCESS;
}

static VkResult CreateCompositeDescriptorStuff(void)
{
    VkResult r = VK_SUCCESS;

    if (g.compDSL == VK_NULL_HANDLE)
    {
        // layout: binding0 = combined image sampler (scene0), binding1 = combined image sampler (scene1)
        VkDescriptorSetLayoutBinding bindings[2];
        ZeroMem(bindings, sizeof(bindings));

        bindings[0].binding = 0;
        bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[0].descriptorCount = 1;
        bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        bindings[1].binding = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].descriptorCount = 1;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo dslci;
        ZeroMem(&dslci, sizeof(dslci));
        dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dslci.bindingCount = 2;
        dslci.pBindings = bindings;

        r = vkCreateDescriptorSetLayout(g.device, &dslci, NULL, &g.compDSL);
        if (r != VK_SUCCESS) return r;
    }

    if (g.compSampler == VK_NULL_HANDLE)
    {
        // sampler
        VkSamplerCreateInfo sci;
        ZeroMem(&sci, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sci.magFilter = VK_FILTER_LINEAR;
        sci.minFilter = VK_FILTER_LINEAR;
        sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sci.minLod = 0.0f;
        sci.maxLod = 0.0f;
        r = vkCreateSampler(g.device, &sci, NULL, &g.compSampler);
        if (r != VK_SUCCESS) return r;
    }

    // pool
    VkDescriptorPoolSize psize;
    ZeroMem(&psize, sizeof(psize));
    psize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    psize.descriptorCount = 2;

    VkDescriptorPoolCreateInfo dpci;
    ZeroMem(&dpci, sizeof(dpci));
    dpci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpci.maxSets = 2;
    dpci.poolSizeCount = 1;
    dpci.pPoolSizes = &psize;

    r = vkCreateDescriptorPool(g.device, &dpci, NULL, &g.compPool);
    if (r != VK_SUCCESS) return r;

    // alloc set
    VkDescriptorSetAllocateInfo dsai;
    ZeroMem(&dsai, sizeof(dsai));
    dsai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsai.descriptorPool = g.compPool;
    dsai.descriptorSetCount = 1;
    dsai.pSetLayouts = &g.compDSL;

    r = vkAllocateDescriptorSets(g.device, &dsai, &g.compDescSet);
    if (r != VK_SUCCESS) return r;

    // update set with two sampled images (scene0.color, scene1.color)
    VkDescriptorImageInfo ii0;
    ii0.sampler = g.compSampler;
    ii0.imageView = g.scene0.color.view;
    ii0.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorImageInfo ii1;
    ii1.sampler = g.compSampler;
    ii1.imageView = g.scene1.color.view;
    ii1.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet writes[2];
    ZeroMem(writes, sizeof(writes));

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = g.compDescSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].pImageInfo = &ii0;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = g.compDescSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[1].pImageInfo = &ii1;

    vkUpdateDescriptorSets(g.device, 2, writes, 0, NULL);
    return VK_SUCCESS;
}

// ===================================================================================
// Pipelines
// ===================================================================================

static void FillViewportScissor(VkExtent2D extent,
                                VkViewport* vp,
                                VkRect2D* sc)
{
    vp->x = 0.0f;
    vp->y = 0.0f;
    vp->width  = (float)extent.width;
    vp->height = (float)extent.height;
    vp->minDepth = 0.0f;
    vp->maxDepth = 1.0f;

    sc->offset.x = 0;
    sc->offset.y = 0;
    sc->extent = extent;
}

static VkPipeline CreateScenePipeline(VkShaderModule vs, VkShaderModule fs,
                                      VkPipelineLayout pLayout,
                                      VkRenderPass rp, VkExtent2D extent,
                                      const VkVertexInputBindingDescription* binds,
                                      uint32_t bindCount,
                                      const VkVertexInputAttributeDescription* attrs,
                                      uint32_t attrCount)
{
    VkPipelineVertexInputStateCreateInfo vi;
    ZeroMem(&vi, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = bindCount;
    vi.pVertexBindingDescriptions = binds;
    vi.vertexAttributeDescriptionCount = attrCount;
    vi.pVertexAttributeDescriptions = attrs;

    VkPipelineInputAssemblyStateCreateInfo ia;
    ZeroMem(&ia, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport vp;
    VkRect2D sc;
    FillViewportScissor(extent, &vp, &sc);

    VkPipelineViewportStateCreateInfo vpci;
    ZeroMem(&vpci, sizeof(vpci));
    vpci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpci.viewportCount = 1;
    vpci.pViewports = &vp;
    vpci.scissorCount = 1;
    vpci.pScissors = &sc;

    VkDynamicState dynStates[2];
    dynStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynStates[1] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dsci;
    ZeroMem(&dsci, sizeof(dsci));
    dsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dsci.dynamicStateCount = 2;
    dsci.pDynamicStates = dynStates;

    VkPipelineRasterizationStateCreateInfo rs;
    ZeroMem(&rs, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms;
    ZeroMem(&ms, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds;
    ZeroMem(&ds, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS;

    VkPipelineColorBlendAttachmentState cba;
    ZeroMem(&cba, sizeof(cba));
    cba.colorWriteMask = 0xF;
    cba.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo cb;
    ZeroMem(&cb, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &cba;

    VkPipelineShaderStageCreateInfo stages[2];
    ZeroMem(stages, sizeof(stages));
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vs;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fs;
    stages[1].pName = "main";

    VkGraphicsPipelineCreateInfo gpci;
    ZeroMem(&gpci, sizeof(gpci));
    gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpci.stageCount = 2;
    gpci.pStages = stages;
    gpci.pVertexInputState = &vi;
    gpci.pInputAssemblyState = &ia;
    gpci.pViewportState = &vpci;
    gpci.pRasterizationState = &rs;
    gpci.pMultisampleState = &ms;
    gpci.pDepthStencilState = &ds;
    gpci.pColorBlendState = &cb;
    gpci.pDynamicState = &dsci;
    gpci.layout = pLayout;
    gpci.renderPass = rp;
    gpci.subpass = 0;

    VkPipeline pipe = VK_NULL_HANDLE;
    VkResult r = vkCreateGraphicsPipelines(g.device, VK_NULL_HANDLE, 1, &gpci, NULL, &pipe);
    if (r != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }
    return pipe;
}

static VkPipeline CreateCompositePipeline(VkShaderModule vs, VkShaderModule fs,
                                          VkPipelineLayout pLayout, VkRenderPass rp, VkExtent2D extent)
{
    // full-screen triangle: no vertex input
    VkPipelineVertexInputStateCreateInfo vi;
    ZeroMem(&vi, sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    VkPipelineInputAssemblyStateCreateInfo ia;
    ZeroMem(&ia, sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkViewport vp;
    VkRect2D sc;
    FillViewportScissor(extent, &vp, &sc);

    VkPipelineViewportStateCreateInfo vpci;
    ZeroMem(&vpci, sizeof(vpci));
    vpci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vpci.viewportCount = 1;
    vpci.pViewports = &vp;
    vpci.scissorCount = 1;
    vpci.pScissors = &sc;

    VkDynamicState dynStates[2];
    dynStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    dynStates[1] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDynamicStateCreateInfo dsci;
    ZeroMem(&dsci, sizeof(dsci));
    dsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dsci.dynamicStateCount = 2;
    dsci.pDynamicStates = dynStates;

    VkPipelineRasterizationStateCreateInfo rs;
    ZeroMem(&rs, sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms;
    ZeroMem(&ms, sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds;
    ZeroMem(&ds, sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable = VK_FALSE;
    ds.depthWriteEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState cba;
    ZeroMem(&cba, sizeof(cba));
    cba.colorWriteMask = 0xF;
    cba.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo cb;
    ZeroMem(&cb, sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = &cba;

    VkPipelineShaderStageCreateInfo stages[2];
    ZeroMem(stages, sizeof(stages));
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vs;
    stages[0].pName  = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fs;
    stages[1].pName  = "main";

    VkGraphicsPipelineCreateInfo gpci;
    ZeroMem(&gpci, sizeof(gpci));
    gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    gpci.stageCount = 2;
    gpci.pStages = stages;
    gpci.pVertexInputState = &vi;
    gpci.pInputAssemblyState = &ia;
    gpci.pViewportState = &vpci;
    gpci.pRasterizationState = &rs;
    gpci.pMultisampleState = &ms;
    gpci.pDepthStencilState = &ds;
    gpci.pColorBlendState = &cb;
    gpci.pDynamicState = &dsci;
    gpci.layout = pLayout;
    gpci.renderPass = rp;
    gpci.subpass = 0;

    VkPipeline pipe = VK_NULL_HANDLE;
    VkResult r = vkCreateGraphicsPipelines(g.device, VK_NULL_HANDLE, 1, &gpci, NULL, &pipe);
    if (r != VK_SUCCESS)
    {
        return VK_NULL_HANDLE;
    }
    return pipe;
}

// ===================================================================================
// Uniform Updates
// ===================================================================================

static void FillScene0Uniform(Scene0UniformData* u, float angle, uint32_t winW, uint32_t winH, BOOL lighting)
{
    ZeroMem(u, sizeof(*u));
    glm::mat4 I = glm::mat4(1.0f);

    glm::mat4 model = glm::translate(I, glm::vec3(0.0f, 0.0f, -2.0f));
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)winW / (float)winH, 0.1f, 100.0f);
    proj[1][1] *= -1.0f;

    u->modelMatrix = model;
    u->viewMatrix = view;
    u->projectionMatrix = proj;

    // Lighting colours follow Scene0.cpp defaults (three rotating RGB lights)
    const glm::vec4 ambientBase[3] =
    {
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    };
    const glm::vec4 diffuseBase[3] =
    {
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
    };
    const glm::vec4 specularBase[3] =
    {
        glm::vec4(1.0f, 0.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
        glm::vec4(0.0f, 0.0f, 1.0f, 1.0f)
    };

    float rad = glm::radians(angle);
    float cosA = cosf(rad);
    float sinA = sinf(rad);

    glm::vec4 positions[3];
    positions[0] = glm::vec4(0.0f, SCENE0_LIGHT_RADIUS * cosA, SCENE0_LIGHT_RADIUS * sinA, 1.0f);
    positions[1] = glm::vec4(SCENE0_LIGHT_RADIUS * cosA, 0.0f, SCENE0_LIGHT_RADIUS * sinA, 1.0f);
    positions[2] = glm::vec4(SCENE0_LIGHT_RADIUS * cosA, SCENE0_LIGHT_RADIUS * sinA, 0.0f, 1.0f);

    for (int i = 0; i < 3; ++i)
    {
        u->lightAmbient[i] = ambientBase[i];
        u->lightDiffuse[i] = diffuseBase[i];
        u->lightSpecular[i] = specularBase[i];
        u->lightPosition[i] = positions[i];
    }

    u->materialAmbient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    u->materialDiffuse = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    u->materialSpecular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    u->materialShininess = 128.0f;
    u->lKeyIsPressed = lighting ? 1u : 0u;
}

static void FillScene1Uniform(Scene1UniformData* u, float angle, uint32_t winW, uint32_t winH, BOOL lighting)
{
    ZeroMem(u, sizeof(*u));
    glm::mat4 I = glm::mat4(1.0f);

    glm::mat4 model = glm::translate(I, glm::vec3(0.0f, 0.0f, -5.0f));
    model = model * glm::rotate(I, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)winW / (float)winH, 0.1f, 100.0f);
    proj[1][1] *= -1.0f;

    u->modelMatrix = model;
    u->viewMatrix = view;
    u->projectionMatrix = proj;

    // Scene1.cpp uses two coloured spot lights oscillating around the model
    for (int i = 0; i < 2; ++i)
    {
        u->lightAmbient[i]  = glm::vec4(0.0f);
        u->lightDiffuse[i]  = glm::vec4(0.0f);
        u->lightSpecular[i] = glm::vec4(0.0f);
    }
    u->lightPosition[0] = glm::vec4( 2.0f, 0.0f, 0.0f, 1.0f);
    u->lightPosition[1] = glm::vec4(-2.0f, 0.0f, 0.0f, 1.0f);

    if (lighting)
    {
        u->lightAmbient[0]  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        u->lightDiffuse[0]  = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        u->lightSpecular[0] = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

        u->lightAmbient[1]  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        u->lightDiffuse[1]  = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
        u->lightSpecular[1] = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }

    u->materialAmbient  = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    u->materialDiffuse  = glm::vec4(0.5f, 0.2f, 0.7f, 1.0f);
    u->materialSpecular = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
    u->materialShininess = 128.0f;
    u->lKeyIsPressed = lighting ? 1u : 0u;
    u->padding = glm::vec2(0.0f);
}

static void FillScene2Uniform(Scene2UniformData* u, uint32_t winW, uint32_t winH)
{
    ZeroMem(u, sizeof(*u));
    glm::mat4 I = glm::mat4(1.0f);
    u->modelMatrix = I;
    u->viewMatrix = I;
    glm::mat4 proj = glm::mat4(1.0f);
    // Ortho covering NDC since geometry is already in [-1,1]
    u->projectionMatrix = proj;
}

// ===================================================================================
// Frame Recording
// ===================================================================================

static VkResult RecordFrameCmd(uint32_t imgIndex)
{
    VkCommandBuffer cmd = g.cmdBufs[imgIndex];

    VkCommandBufferBeginInfo bi;
    ZeroMem(&bi, sizeof(bi));
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkResult r = vkBeginCommandBuffer(cmd, &bi);
    if (r != VK_SUCCESS) return r;

    // ----- Offscreen: Scene 0 -----
    VkClearValue clears0[2];
    clears0[0].color.float32[0] = 0.05f;
    clears0[0].color.float32[1] = 0.05f;
    clears0[0].color.float32[2] = 0.08f;
    clears0[0].color.float32[3] = 1.0f;
    clears0[1].depthStencil.depth = 1.0f;
    clears0[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo rpbi0;
    ZeroMem(&rpbi0, sizeof(rpbi0));
    rpbi0.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi0.renderPass = g.rpOffscreen;
    rpbi0.framebuffer = g.scene0.framebuffer;
    rpbi0.renderArea.offset.x = 0;
    rpbi0.renderArea.offset.y = 0;
    rpbi0.renderArea.extent = g.sc.extent;
    rpbi0.clearValueCount = 2;
    rpbi0.pClearValues = clears0;

    vkCmdBeginRenderPass(cmd, &rpbi0, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.s0_pipe);
    VkViewport vp0;
    VkRect2D sc0;
    FillViewportScissor(g.scene0.color.extent, &vp0, &sc0);
    vkCmdSetViewport(cmd, 0, 1, &vp0);
    vkCmdSetScissor(cmd, 0, 1, &sc0);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.s0_pLayout, 0, 1, &g.s0_descSet, 0, NULL);
    VkBuffer vbs0[3] = { g.s0_vboPos.buf, g.s0_vboNorm.buf, g.s0_vboTex.buf };
    VkDeviceSize offs0[3] = { 0, 0, 0 };
    vkCmdBindVertexBuffers(cmd, 0, 3, vbs0, offs0);
    vkCmdBindIndexBuffer(cmd, g.s0_ibo.buf, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, g.s0_indexCount, 1, 0, 0, 0);

    vkCmdEndRenderPass(cmd);

    // ----- Offscreen: Scene 1 -----
    VkClearValue clears1[2];
    clears1[0].color.float32[0] = 0.04f;
    clears1[0].color.float32[1] = 0.03f;
    clears1[0].color.float32[2] = 0.06f;
    clears1[0].color.float32[3] = 1.0f;
    clears1[1].depthStencil.depth = 1.0f;
    clears1[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo rpbi1;
    ZeroMem(&rpbi1, sizeof(rpbi1));
    rpbi1.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi1.renderPass = g.rpOffscreen;
    rpbi1.framebuffer = g.scene1.framebuffer;
    rpbi1.renderArea.offset.x = 0;
    rpbi1.renderArea.offset.y = 0;
    rpbi1.renderArea.extent = g.sc.extent;
    rpbi1.clearValueCount = 2;
    rpbi1.pClearValues = clears1;

    vkCmdBeginRenderPass(cmd, &rpbi1, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.s1_pipe);
    VkViewport vp1;
    VkRect2D sc1;
    FillViewportScissor(g.scene1.color.extent, &vp1, &sc1);
    vkCmdSetViewport(cmd, 0, 1, &vp1);
    vkCmdSetScissor(cmd, 0, 1, &sc1);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.s1_pLayout, 0, 1, &g.s1_descSet, 0, NULL);
    VkBuffer vbs1[2] = { g.s1_vboPos.buf, g.s1_vboNorm.buf };
    VkDeviceSize offs1[2] = { 0, 0 };
    vkCmdBindVertexBuffers(cmd, 0, 2, vbs1, offs1);
    vkCmdDraw(cmd, g.s1_vertexCount, 1, 0, 0);

    vkCmdEndRenderPass(cmd);

    // ----- Offscreen: Scene 2 -----
    VkClearValue clears2[2];
    clears2[0].color.float32[0] = 0.02f;
    clears2[0].color.float32[1] = 0.02f;
    clears2[0].color.float32[2] = 0.02f;
    clears2[0].color.float32[3] = 1.0f;
    clears2[1].depthStencil.depth = 1.0f;
    clears2[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo rpbi2;
    ZeroMem(&rpbi2, sizeof(rpbi2));
    rpbi2.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi2.renderPass = g.rpOffscreen;
    rpbi2.framebuffer = g.scene2.framebuffer;
    rpbi2.renderArea.offset.x = 0;
    rpbi2.renderArea.offset.y = 0;
    rpbi2.renderArea.extent = g.sc.extent;
    rpbi2.clearValueCount = 2;
    rpbi2.pClearValues = clears2;

    vkCmdBeginRenderPass(cmd, &rpbi2, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.s2_pipe);
    VkViewport vp2;
    VkRect2D sc2;
    FillViewportScissor(g.scene2.color.extent, &vp2, &sc2);
    vkCmdSetViewport(cmd, 0, 1, &vp2);
    vkCmdSetScissor(cmd, 0, 1, &sc2);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.s2_pLayout, 0, 1, &g.s2_descSet, 0, NULL);
    VkBuffer vbs2[2] = { g.s2_vboPos.buf, g.s2_vboTex.buf };
    VkDeviceSize offs2[2] = { 0, 0 };
    vkCmdBindVertexBuffers(cmd, 0, 2, vbs2, offs2);
    vkCmdDraw(cmd, g.s2_vertexCount, 1, 0, 0);

    vkCmdEndRenderPass(cmd);

    // ----- Present composite -----
    VkClearValue clears[2];
    clears[0].color.float32[0] = 0.0f;
    clears[0].color.float32[1] = 0.0f;
    clears[0].color.float32[2] = 0.0f;
    clears[0].color.float32[3] = 1.0f;
    clears[1].depthStencil.depth = 1.0f;
    clears[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo rpbi;
    ZeroMem(&rpbi, sizeof(rpbi));
    rpbi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpbi.renderPass = g.rpPresent;
    rpbi.framebuffer = g.sc.framebuffers[imgIndex];
    rpbi.renderArea.offset.x = 0;
    rpbi.renderArea.offset.y = 0;
    rpbi.renderArea.extent = g.sc.extent;
    rpbi.clearValueCount = 2;
    rpbi.pClearValues = clears;

    vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.compPipe);
    VkViewport vpPresent;
    VkRect2D scPresent;
    FillViewportScissor(g.sc.extent, &vpPresent, &scPresent);
    vkCmdSetViewport(cmd, 0, 1, &vpPresent);
    vkCmdSetScissor(cmd, 0, 1, &scPresent);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, g.comp_pLayout, 0, 1, &g.compDescSet, 0, NULL);

    // push constant float t or override based on selection
    float t = g.t;
    if (g.selectedScene == 0) t = 0.0f; else if (g.selectedScene == 1) t = 1.0f; else if (g.selectedScene == 2) {
        // Blend between scene1 and scene2: reuse descriptor by updating elsewhere; here keep t=1 and swap descriptors.
        t = 1.0f;
    }
    vkCmdPushConstants(cmd, g.comp_pLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(float), &t);

    // full-screen triangle (3 verts, no vbo)
    vkCmdDraw(cmd, 3, 1, 0, 0);

    vkCmdEndRenderPass(cmd);

    r = vkEndCommandBuffer(cmd);
    return r;
}

// ===================================================================================
// Initialization sequence
// ===================================================================================

static VkResult InitVulkan(uint32_t winW, uint32_t winH)
{
    g.initialized = FALSE;
    g.winWidth = winW;
    g.winHeight = winH;

    VkResult r = CreateInstance();
    if (r != VK_SUCCESS) return r;

    r = CreateSurface();
    if (r != VK_SUCCESS) return r;

    r = PickPhysicalDeviceAndQueue();
    if (r != VK_SUCCESS) return r;

    r = CreateDeviceAndQueue();
    if (r != VK_SUCCESS) return r;

    r = CreateSwapchainAndTargets(winW, winH, &g.sc);
    if (r != VK_SUCCESS) return r;

    r = CreatePresentDepth(&g.sc);
    if (r != VK_SUCCESS) return r;

    // render passes
    r = CreateRenderPassOffscreen(g.sc.format, g.sc.depth.format, &g.rpOffscreen);
    if (r != VK_SUCCESS) return r;

    r = CreateRenderPassPresent(g.sc.format, g.sc.depth.format, &g.rpPresent);
    if (r != VK_SUCCESS) return r;

    // offscreen passes
    r = CreateOffscreenPass(&g.scene0, g.sc.format, g.sc.depth.format, g.sc.extent);
    if (r != VK_SUCCESS) return r;

    r = CreateOffscreenPass(&g.scene1, g.sc.format, g.sc.depth.format, g.sc.extent);
    if (r != VK_SUCCESS) return r;

    r = CreateOffscreenPass(&g.scene2, g.sc.format, g.sc.depth.format, g.sc.extent);
    if (r != VK_SUCCESS) return r;

    // framebuffers for present
    r = CreatePresentFramebuffers(&g.sc);
    if (r != VK_SUCCESS) return r;

    // command pool & buffers
    r = CreateCommandPoolAndBuffers();
    if (r != VK_SUCCESS) return r;

    // sync
    r = CreateSyncObjects();
    if (r != VK_SUCCESS) return r;

    // geometry buffers - Scene 0 (matches Scene0.cpp expectations)
    float spherePos[1146];
    float sphereNorm[1146];
    float sphereTex[764];
    unsigned short sphereIdx[2280];
    getSphereVertexData(spherePos, sphereNorm, sphereTex, sphereIdx);
    g.s0_vertexCount = getNumberOfSphereVertices();
    g.s0_indexCount = getNumberOfSphereElements();

    r = CreateBuffer(sizeof(spherePos), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.s0_vboPos);
    if (r != VK_SUCCESS) return r;
    r = UploadBuffer(spherePos, sizeof(spherePos), &g.s0_vboPos);
    if (r != VK_SUCCESS) return r;

    r = CreateBuffer(sizeof(sphereNorm), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.s0_vboNorm);
    if (r != VK_SUCCESS) return r;
    r = UploadBuffer(sphereNorm, sizeof(sphereNorm), &g.s0_vboNorm);
    if (r != VK_SUCCESS) return r;

    r = CreateBuffer(sizeof(sphereTex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.s0_vboTex);
    if (r != VK_SUCCESS) return r;
    r = UploadBuffer(sphereTex, sizeof(sphereTex), &g.s0_vboTex);
    if (r != VK_SUCCESS) return r;

    r = CreateBuffer(sizeof(sphereIdx), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.s0_ibo);
    if (r != VK_SUCCESS) return r;
    r = UploadBuffer(sphereIdx, sizeof(sphereIdx), &g.s0_ibo);
    if (r != VK_SUCCESS) return r;

    // geometry buffers - Scene 1 (pyramid from Scene1.cpp)
    g.s1_vertexCount = (uint32_t)(sizeof(sPyramidPos) / (sizeof(float) * 3));
    r = CreateBuffer(sizeof(sPyramidPos), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.s1_vboPos);
    if (r != VK_SUCCESS) return r;
    r = UploadBuffer(sPyramidPos, sizeof(sPyramidPos), &g.s1_vboPos);
    if (r != VK_SUCCESS) return r;

    r = CreateBuffer(sizeof(sPyramidNorm), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.s1_vboNorm);
    if (r != VK_SUCCESS) return r;
    r = UploadBuffer(sPyramidNorm, sizeof(sPyramidNorm), &g.s1_vboNorm);
    if (r != VK_SUCCESS) return r;

    // UBOs
    r = CreateBuffer(sizeof(Scene0UniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.ubo0);
    if (r != VK_SUCCESS) return r;
    r = CreateBuffer(sizeof(Scene1UniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.ubo1);
    if (r != VK_SUCCESS) return r;
    r = CreateBuffer(sizeof(Scene2UniformData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.ubo2);
    if (r != VK_SUCCESS) return r;

    // scene descriptor set layout/pool/sets
    r = CreateSceneDescriptorStuff();
    if (r != VK_SUCCESS) return r;

    // load shaders (Scene0/Scene1 from your existing SPIR-V)
    g.s0_vs = CreateShaderModuleFromFile(g.device, "Shader_Scene0.vert.spv");
    g.s0_fs = CreateShaderModuleFromFile(g.device, "Shader_Scene0.frag.spv");
    g.s1_vs = CreateShaderModuleFromFile(g.device, "Shader_Scene1.vert.spv");
    g.s1_fs = CreateShaderModuleFromFile(g.device, "Shader_Scene1.frag.spv");
    if (!g.s0_vs || !g.s0_fs || !g.s1_vs || !g.s1_fs) return VK_ERROR_INITIALIZATION_FAILED;

    // Scene2 uses Scene2 shaders existing in workspace
    g.s2_vs = CreateShaderModuleFromFile(g.device, "Shader_Scene2.vert.spv");
    g.s2_fs = CreateShaderModuleFromFile(g.device, "Shader_Scene2.frag.spv");
    if (!g.s2_vs || !g.s2_fs)
    {
        // try alternate names
        g.s2_vs = g.s2_vs ? g.s2_vs : CreateShaderModuleFromFile(g.device, "shader_Scene2.vert.spv");
        g.s2_fs = g.s2_fs ? g.s2_fs : CreateShaderModuleFromFile(g.device, "shader_Scene2.frag.spv");
    }
    // Scene2 is optional; if shaders unavailable, we will skip scene2 drawing

    // scene pipeline layouts
    VkPipelineLayoutCreateInfo plci;
    ZeroMem(&plci, sizeof(plci));
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &g.sUboDSL;

    r = vkCreatePipelineLayout(g.device, &plci, NULL, &g.s0_pLayout);
    if (r != VK_SUCCESS) return r;
    r = vkCreatePipelineLayout(g.device, &plci, NULL, &g.s1_pLayout);
    if (r != VK_SUCCESS) return r;

    // For scene2, we will have a separate DSL with binding0=UBO, binding1=combined sampler
    VkDescriptorSetLayoutBinding s2Bindings[2];
    ZeroMem(s2Bindings, sizeof(s2Bindings));
    s2Bindings[0].binding = 0;
    s2Bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    s2Bindings[0].descriptorCount = 1;
    s2Bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    s2Bindings[1].binding = 1;
    s2Bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    s2Bindings[1].descriptorCount = 1;
    s2Bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo s2dslci;
    ZeroMem(&s2dslci, sizeof(s2dslci));
    s2dslci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    s2dslci.bindingCount = 2;
    s2dslci.pBindings = s2Bindings;
    vkCreateDescriptorSetLayout(g.device, &s2dslci, NULL, &g.s2_DSL);

    ZeroMem(&plci, sizeof(plci));
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &g.s2_DSL;
    r = vkCreatePipelineLayout(g.device, &plci, NULL, &g.s2_pLayout);
    if (r != VK_SUCCESS) return r;

    // scene pipelines (render into offscreen pass)
    VkVertexInputBindingDescription s0Binds[3];
    ZeroMem(s0Binds, sizeof(s0Binds));
    s0Binds[0].binding = 0;
    s0Binds[0].stride = sizeof(float) * 3;
    s0Binds[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    s0Binds[1].binding = 1;
    s0Binds[1].stride = sizeof(float) * 3;
    s0Binds[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    s0Binds[2].binding = 2;
    s0Binds[2].stride = sizeof(float) * 2;
    s0Binds[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription s0Attrs[3];
    ZeroMem(s0Attrs, sizeof(s0Attrs));
    s0Attrs[0].location = 0;
    s0Attrs[0].binding = 0;
    s0Attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    s0Attrs[0].offset = 0;
    s0Attrs[1].location = 1;
    s0Attrs[1].binding = 1;
    s0Attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    s0Attrs[1].offset = 0;
    s0Attrs[2].location = 2;
    s0Attrs[2].binding = 2;
    s0Attrs[2].format = VK_FORMAT_R32G32_SFLOAT;
    s0Attrs[2].offset = 0;

    g.s0_pipe = CreateScenePipeline(g.s0_vs, g.s0_fs, g.s0_pLayout, g.rpOffscreen, g.sc.extent,
                                    s0Binds, 3, s0Attrs, 3);

    VkVertexInputBindingDescription s1Binds[2];
    ZeroMem(s1Binds, sizeof(s1Binds));
    s1Binds[0].binding = 0;
    s1Binds[0].stride = sizeof(float) * 3;
    s1Binds[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    s1Binds[1].binding = 1;
    s1Binds[1].stride = sizeof(float) * 3;
    s1Binds[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription s1Attrs[2];
    ZeroMem(s1Attrs, sizeof(s1Attrs));
    s1Attrs[0].location = 0;
    s1Attrs[0].binding = 0;
    s1Attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    s1Attrs[0].offset = 0;
    s1Attrs[1].location = 1;
    s1Attrs[1].binding = 1;
    s1Attrs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    s1Attrs[1].offset = 0;

    g.s1_pipe = CreateScenePipeline(g.s1_vs, g.s1_fs, g.s1_pLayout, g.rpOffscreen, g.sc.extent,
                                    s1Binds, 2, s1Attrs, 2);
    if (!g.s0_pipe || !g.s1_pipe) return VK_ERROR_INITIALIZATION_FAILED;

    // Scene2 geometry (fullscreen quad positions + texcoords)
    static const float sQuadPos[] = {
        // tri1
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        // tri2
        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
    };
    static const float sQuadUV[] = {
        0.0f, 1.0f,
        1.0f, 1.0f,
        0.0f, 0.0f,
        0.0f, 0.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
    };
    g.s2_vertexCount = 6;
    r = CreateBuffer(sizeof(sQuadPos), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.s2_vboPos);
    if (r != VK_SUCCESS) return r;
    r = UploadBuffer(sQuadPos, sizeof(sQuadPos), &g.s2_vboPos);
    if (r != VK_SUCCESS) return r;
    r = CreateBuffer(sizeof(sQuadUV), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g.s2_vboTex);
    if (r != VK_SUCCESS) return r;
    r = UploadBuffer(sQuadUV, sizeof(sQuadUV), &g.s2_vboTex);
    if (r != VK_SUCCESS) return r;

    // Simple texture for Scene2 from EndCredits.png
    int tw=0, th=0, tc=0;
    unsigned char* pixels = stbi_load("EndCredits.png", &tw, &th, &tc, 4);
    if (pixels)
    {
        // create image and upload via a host-visible linear tiling buffer (simplified: reuse color RT as sampled not suitable). Here, skip upload to keep sample concise.
        stbi_image_free(pixels);
    }

    // Scene2 pipeline if shaders exist
    if (g.s2_vs && g.s2_fs)
    {
        VkVertexInputBindingDescription s2Binds[2];
        ZeroMem(s2Binds, sizeof(s2Binds));
        s2Binds[0].binding = 0; s2Binds[0].stride = sizeof(float)*3; s2Binds[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        s2Binds[1].binding = 1; s2Binds[1].stride = sizeof(float)*2; s2Binds[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        VkVertexInputAttributeDescription s2Attrs[2];
        ZeroMem(s2Attrs, sizeof(s2Attrs));
        s2Attrs[0].location=0; s2Attrs[0].binding=0; s2Attrs[0].format=VK_FORMAT_R32G32B32_SFLOAT; s2Attrs[0].offset=0;
        s2Attrs[1].location=1; s2Attrs[1].binding=1; s2Attrs[1].format=VK_FORMAT_R32G32_SFLOAT; s2Attrs[1].offset=0;

        g.s2_pipe = CreateScenePipeline(g.s2_vs, g.s2_fs, g.s2_pLayout, g.rpOffscreen, g.sc.extent,
                                        s2Binds, 2, s2Attrs, 2);
    }

    // composite shaders (compile the provided GLSL to SPIR-V and place files)
    g.comp_vs = CreateShaderModuleFromFile(g.device, "composite_fullscreen.vert.spv");
    g.comp_fs = CreateShaderModuleFromFile(g.device, "composite_blend.frag.spv");
    if (!g.comp_vs || !g.comp_fs) return VK_ERROR_INITIALIZATION_FAILED;

    // composite descriptor set (two sampled images)
    r = CreateCompositeDescriptorStuff();
    if (r != VK_SUCCESS) return r;

    // composite pipeline layout (add push constant range for float t)
    VkPushConstantRange pcr;
    pcr.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pcr.offset = 0;
    pcr.size = sizeof(float);

    ZeroMem(&plci, sizeof(plci));
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.setLayoutCount = 1;
    plci.pSetLayouts = &g.compDSL;
    plci.pushConstantRangeCount = 1;
    plci.pPushConstantRanges = &pcr;

    r = vkCreatePipelineLayout(g.device, &plci, NULL, &g.comp_pLayout);
    if (r != VK_SUCCESS) return r;

    // composite pipeline (present render pass)
    g.compPipe = CreateCompositePipeline(g.comp_vs, g.comp_fs, g.comp_pLayout, g.rpPresent, g.sc.extent);
    if (!g.compPipe) return VK_ERROR_INITIALIZATION_FAILED;

    // init anim
    g.angle0 = 0.0f;
    g.angle1 = 0.0f;
    g.t = 0.0f;
    g.phase = ScenePhase_Scene0Hold;
    g.phaseTime = 0.0;
    if (!QueryPerformanceFrequency(&g.qpcFreq))
    {
        g.qpcFreq.QuadPart = 0;
    }
    if (!QueryPerformanceCounter(&g.lastQpc))
    {
        g.lastQpc.QuadPart = 0;
    }
    g.lighting = TRUE;
    g.selectedScene = 0u;

    g.winWidth = g.sc.extent.width;
    g.winHeight = g.sc.extent.height;
    g.initialized = TRUE;

    return VK_SUCCESS;
}

static void CleanupVulkan(void)
{
    g.initialized = FALSE;
    g.winWidth = 0;
    g.winHeight = 0;
    g.angle0 = 0.0f;
    g.angle1 = 0.0f;
    g.t = 0.0f;
    g.phase = ScenePhase_Scene0Hold;
    g.phaseTime = 0.0;
    g.lastQpc.QuadPart = 0;
    g.qpcFreq.QuadPart = 0;

    if (g.device)
    {
        vkDeviceWaitIdle(g.device);

        if (g.compSampler) vkDestroySampler(g.device, g.compSampler, NULL);

        if (g.compPipe) vkDestroyPipeline(g.device, g.compPipe, NULL);
        if (g.comp_pLayout) vkDestroyPipelineLayout(g.device, g.comp_pLayout, NULL);
        if (g.compPool) vkDestroyDescriptorPool(g.device, g.compPool, NULL);
        if (g.compDSL) vkDestroyDescriptorSetLayout(g.device, g.compDSL, NULL);
        if (g.comp_vs) vkDestroyShaderModule(g.device, g.comp_vs, NULL);
        if (g.comp_fs) vkDestroyShaderModule(g.device, g.comp_fs, NULL);

        if (g.s0_pipe) vkDestroyPipeline(g.device, g.s0_pipe, NULL);
        if (g.s1_pipe) vkDestroyPipeline(g.device, g.s1_pipe, NULL);
        if (g.s0_pLayout) vkDestroyPipelineLayout(g.device, g.s0_pLayout, NULL);
        if (g.s1_pLayout) vkDestroyPipelineLayout(g.device, g.s1_pLayout, NULL);
        if (g.sDescPool) vkDestroyDescriptorPool(g.device, g.sDescPool, NULL);
        if (g.sUboDSL) vkDestroyDescriptorSetLayout(g.device, g.sUboDSL, NULL);
        if (g.s0_vs) vkDestroyShaderModule(g.device, g.s0_vs, NULL);
        if (g.s0_fs) vkDestroyShaderModule(g.device, g.s0_fs, NULL);
        if (g.s1_vs) vkDestroyShaderModule(g.device, g.s1_vs, NULL);
        if (g.s1_fs) vkDestroyShaderModule(g.device, g.s1_fs, NULL);

        if (g.ubo0.buf) { vkDestroyBuffer(g.device, g.ubo0.buf, NULL); vkFreeMemory(g.device, g.ubo0.mem, NULL); }
        if (g.ubo1.buf) { vkDestroyBuffer(g.device, g.ubo1.buf, NULL); vkFreeMemory(g.device, g.ubo1.mem, NULL); }
        if (g.s0_vboPos.buf) { vkDestroyBuffer(g.device, g.s0_vboPos.buf, NULL); vkFreeMemory(g.device, g.s0_vboPos.mem, NULL); }
        if (g.s0_vboNorm.buf) { vkDestroyBuffer(g.device, g.s0_vboNorm.buf, NULL); vkFreeMemory(g.device, g.s0_vboNorm.mem, NULL); }
        if (g.s0_vboTex.buf) { vkDestroyBuffer(g.device, g.s0_vboTex.buf, NULL); vkFreeMemory(g.device, g.s0_vboTex.mem, NULL); }
        if (g.s0_ibo.buf) { vkDestroyBuffer(g.device, g.s0_ibo.buf, NULL); vkFreeMemory(g.device, g.s0_ibo.mem, NULL); }
        if (g.s1_vboPos.buf) { vkDestroyBuffer(g.device, g.s1_vboPos.buf, NULL); vkFreeMemory(g.device, g.s1_vboPos.mem, NULL); }
        if (g.s1_vboNorm.buf) { vkDestroyBuffer(g.device, g.s1_vboNorm.buf, NULL); vkFreeMemory(g.device, g.s1_vboNorm.mem, NULL); }

        if (g.fences)
        {
            for (uint32_t i = 0; i < g.maxFramesInFlight; ++i)
            {
                if (g.fences[i]) vkDestroyFence(g.device, g.fences[i], NULL);
            }
            free(g.fences);
            g.fences = NULL;
        }

        if (g.imagesInFlight)
        {
            free(g.imagesInFlight);
            g.imagesInFlight = NULL;
        }

        if (g.semRenderFinished)
        {
            for (uint32_t i = 0; i < g.maxFramesInFlight; ++i)
            {
                if (g.semRenderFinished[i]) vkDestroySemaphore(g.device, g.semRenderFinished[i], NULL);
            }
            free(g.semRenderFinished);
            g.semRenderFinished = NULL;
        }

        if (g.semImageAvailable)
        {
            for (uint32_t i = 0; i < g.maxFramesInFlight; ++i)
            {
                if (g.semImageAvailable[i]) vkDestroySemaphore(g.device, g.semImageAvailable[i], NULL);
            }
            free(g.semImageAvailable);
            g.semImageAvailable = NULL;
        }

        g.maxFramesInFlight = 0;
        g.currentFrame = 0;

        if (g.cmdBufs)
        {
            vkFreeCommandBuffers(g.device, g.cmdPool, g.sc.count, g.cmdBufs);
            free(g.cmdBufs);
            g.cmdBufs = NULL;
        }
        if (g.cmdPool)
        {
            vkDestroyCommandPool(g.device, g.cmdPool, NULL);
            g.cmdPool = VK_NULL_HANDLE;
        }

        if (g.sc.framebuffers)
        {
            for (uint32_t i = 0; i < g.sc.count; ++i)
            {
                vkDestroyFramebuffer(g.device, g.sc.framebuffers[i], NULL);
            }
            free(g.sc.framebuffers);
            g.sc.framebuffers = NULL;
        }

        if (g.rpPresent) vkDestroyRenderPass(g.device, g.rpPresent, NULL);

        if (g.scene0.framebuffer) vkDestroyFramebuffer(g.device, g.scene0.framebuffer, NULL);
        if (g.scene1.framebuffer) vkDestroyFramebuffer(g.device, g.scene1.framebuffer, NULL);
        if (g.scene0.color.view) vkDestroyImageView(g.device, g.scene0.color.view, NULL);
        if (g.scene1.color.view) vkDestroyImageView(g.device, g.scene1.color.view, NULL);
        if (g.scene0.depth.view) vkDestroyImageView(g.device, g.scene0.depth.view, NULL);
        if (g.scene1.depth.view) vkDestroyImageView(g.device, g.scene1.depth.view, NULL);

        if (g.scene0.color.image) { vkDestroyImage(g.device, g.scene0.color.image, NULL); vkFreeMemory(g.device, g.scene0.color.mem, NULL); }
        if (g.scene1.color.image) { vkDestroyImage(g.device, g.scene1.color.image, NULL); vkFreeMemory(g.device, g.scene1.color.mem, NULL); }
        if (g.scene0.depth.image) { vkDestroyImage(g.device, g.scene0.depth.image, NULL); vkFreeMemory(g.device, g.scene0.depth.mem, NULL); }
        if (g.scene1.depth.image) { vkDestroyImage(g.device, g.scene1.depth.image, NULL); vkFreeMemory(g.device, g.scene1.depth.mem, NULL); }

        if (g.rpOffscreen) vkDestroyRenderPass(g.device, g.rpOffscreen, NULL);

        if (g.sc.views)
        {
            for (uint32_t i = 0; i < g.sc.count; ++i)
            {
                vkDestroyImageView(g.device, g.sc.views[i], NULL);
            }
            free(g.sc.views);
            g.sc.views = NULL;
        }

        if (g.sc.depth.view) vkDestroyImageView(g.device, g.sc.depth.view, NULL);
        if (g.sc.depth.image) { vkDestroyImage(g.device, g.sc.depth.image, NULL); vkFreeMemory(g.device, g.sc.depth.mem, NULL); }

        if (g.sc.swapchain) vkDestroySwapchainKHR(g.device, g.sc.swapchain, NULL);

        vkDestroyDevice(g.device, NULL);
        g.device = VK_NULL_HANDLE;
    }

    if (g.surface)
    {
        vkDestroySurfaceKHR(g.instance, g.surface, NULL);
        g.surface = VK_NULL_HANDLE;
    }

    if (g.instance)
    {
        if (g.dbgCb && g.pfnDestroyDbg)
        {
            g.pfnDestroyDbg(g.instance, g.dbgCb, NULL);
            g.dbgCb = VK_NULL_HANDLE;
            g.pfnDestroyDbg = NULL;
        }
        vkDestroyInstance(g.instance, NULL);
        g.instance = VK_NULL_HANDLE;
    }
}

// ===================================================================================
// Per-frame: update + draw
// ===================================================================================

static void UpdateUniforms(void)
{
    // Scene0
    Scene0UniformData u0;
    FillScene0Uniform(&u0, g.angle0, g.sc.extent.width, g.sc.extent.height, g.lighting);
    UploadBuffer(&u0, sizeof(u0), &g.ubo0);

    // Scene1
    Scene1UniformData u1;
    FillScene1Uniform(&u1, g.angle1, g.sc.extent.width, g.sc.extent.height, g.lighting);
    UploadBuffer(&u1, sizeof(u1), &g.ubo1);

    // Scene2
    Scene2UniformData u2;
    FillScene2Uniform(&u2, g.sc.extent.width, g.sc.extent.height);
    UploadBuffer(&u2, sizeof(u2), &g.ubo2);
}

static void UpdateAnim(void)
{
    double dt = 0.0;
    LARGE_INTEGER now;

    if (g.qpcFreq.QuadPart != 0 && QueryPerformanceCounter(&now))
    {
        if (g.lastQpc.QuadPart != 0)
        {
            dt = (double)(now.QuadPart - g.lastQpc.QuadPart) / (double)g.qpcFreq.QuadPart;
        }
        g.lastQpc = now;
    }

    if (dt <= 0.0)
    {
        dt = 1.0 / 60.0;
    }

    g.angle0 += SCENE0_ANGLE_SPEED_DEG_PER_SEC * (float)dt;
    while (g.angle0 > 360.0f)
    {
        g.angle0 -= 360.0f;
    }

    g.angle1 += SCENE1_ANGLE_SPEED_DEG_PER_SEC * (float)dt;
    while (g.angle1 > 360.0f)
    {
        g.angle1 -= 360.0f;
    }

    g.phaseTime += dt;

    double phaseTime = g.phaseTime;
    int phase = g.phase;
    BOOL repeat = TRUE;

    while (repeat)
    {
        repeat = FALSE;

        switch (phase)
        {
            case ScenePhase_Scene0Hold:
            {
                g.t = 0.0f;
                if (phaseTime >= SCENE_HOLD_DURATION_SECONDS)
                {
                    phaseTime -= SCENE_HOLD_DURATION_SECONDS;
                    phase = ScenePhase_FadeToScene1;
                    repeat = TRUE;
                }
            } break;

            case ScenePhase_FadeToScene1:
            {
                double duration = SCENE_CROSSFADE_DURATION_SECONDS;
                if (duration <= 0.0)
                {
                    g.t = 1.0f;
                    phase = ScenePhase_Scene1Hold;
                    repeat = TRUE;
                }
                else if (phaseTime >= duration)
                {
                    phaseTime -= duration;
                    g.t = 1.0f;
                    phase = ScenePhase_Scene1Hold;
                    repeat = TRUE;
                }
                else
                {
                    double ratio = phaseTime / duration;
                    if (ratio < 0.0) ratio = 0.0;
                    if (ratio > 1.0) ratio = 1.0;
                    g.t = (float)ratio;
                }
            } break;

            case ScenePhase_Scene1Hold:
            {
                g.t = 1.0f;
                if (phaseTime >= SCENE_HOLD_DURATION_SECONDS)
                {
                    phaseTime -= SCENE_HOLD_DURATION_SECONDS;
                    phase = ScenePhase_FadeToScene0;
                    repeat = TRUE;
                }
            } break;

            case ScenePhase_FadeToScene0:
            {
                double duration = SCENE_CROSSFADE_DURATION_SECONDS;
                if (duration <= 0.0)
                {
                    g.t = 0.0f;
                    phase = ScenePhase_Scene0Hold;
                    repeat = TRUE;
                }
                else if (phaseTime >= duration)
                {
                    phaseTime -= duration;
                    g.t = 0.0f;
                    phase = ScenePhase_Scene0Hold;
                    repeat = TRUE;
                }
                else
                {
                    double ratio = phaseTime / duration;
                    if (ratio < 0.0) ratio = 0.0;
                    if (ratio > 1.0) ratio = 1.0;
                    g.t = (float)(1.0 - ratio);
                }
            } break;

            default:
            {
                g.t = 0.0f;
                phase = ScenePhase_Scene0Hold;
                phaseTime = 0.0;
            } break;
        }
    }

    g.phase = phase;
    g.phaseTime = phaseTime;
}

static VkResult DrawFrame(void)
{
    if (!g.initialized)
    {
        if (g.log)
        {
            fprintf(g.log, "DrawFrame(): Initialization not completed yet.\n");
        }
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    uint32_t frame = g.currentFrame % g.maxFramesInFlight;

    vkWaitForFences(g.device, 1, &g.fences[frame], VK_TRUE, UINT64_MAX);

    uint32_t imgIndex = 0;
    VkResult r = vkAcquireNextImageKHR(g.device, g.sc.swapchain, UINT64_MAX, g.semImageAvailable[frame], VK_NULL_HANDLE, &imgIndex);
    if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR)
    {
        return r; // let caller recreate
    }
    else if (r != VK_SUCCESS)
    {
        return r;
    }

    if (g.imagesInFlight[imgIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(g.device, 1, &g.imagesInFlight[imgIndex], VK_TRUE, UINT64_MAX);
    }

    g.imagesInFlight[imgIndex] = g.fences[frame];

    vkResetFences(g.device, 1, &g.fences[frame]);

    UpdateUniforms();

    // record cmd for this frame with current t
    vkResetCommandBuffer(g.cmdBufs[imgIndex], 0);
    r = RecordFrameCmd(imgIndex);
    if (r != VK_SUCCESS) return r;

    const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo si;
    ZeroMem(&si, sizeof(si));
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &g.semImageAvailable[frame];
    si.pWaitDstStageMask = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &g.cmdBufs[imgIndex];
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &g.semRenderFinished[frame];

    r = vkQueueSubmit(g.queue, 1, &si, g.fences[frame]);
    if (r != VK_SUCCESS) return r;

    VkPresentInfoKHR pi;
    ZeroMem(&pi, sizeof(pi));
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &g.semRenderFinished[frame];
    pi.swapchainCount = 1;
    pi.pSwapchains = &g.sc.swapchain;
    pi.pImageIndices = &imgIndex;

    r = vkQueuePresentKHR(g.queue, &pi);
    g.currentFrame = (frame + 1) % g.maxFramesInFlight;
    return r;
}

// ===================================================================================
// Resize (swapchain recreation)
// ===================================================================================

static VkResult RecreateSwapchain(uint32_t w, uint32_t h)
{
    if (w == 0 || h == 0)
    {
        if (g.log)
        {
            fprintf(g.log, "RecreateSwapchain(): Skipping resize for zero extent (%u x %u).\n", w, h);
        }
        return VK_SUCCESS;
    }

    if (!g.device || g.sc.swapchain == VK_NULL_HANDLE)
    {
        if (g.log)
        {
            fprintf(g.log, "RecreateSwapchain(): Device or swapchain not ready.\n");
        }
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    g.initialized = FALSE;
    g.winWidth = w;
    g.winHeight = h;

    vkDeviceWaitIdle(g.device);

    // destroy per-swapchain stuff
    if (g.cmdBufs)
    {
        vkFreeCommandBuffers(g.device, g.cmdPool, g.sc.count, g.cmdBufs);
        free(g.cmdBufs);
        g.cmdBufs = NULL;
    }
    if (g.cmdPool)
    {
        vkDestroyCommandPool(g.device, g.cmdPool, NULL);
        g.cmdPool = VK_NULL_HANDLE;
    }
    if (g.fences)
    {
        for (uint32_t i = 0; i < g.maxFramesInFlight; ++i)
        {
            if (g.fences[i]) vkDestroyFence(g.device, g.fences[i], NULL);
        }
        free(g.fences);
        g.fences = NULL;
    }

    if (g.imagesInFlight)
    {
        free(g.imagesInFlight);
        g.imagesInFlight = NULL;
    }

    if (g.semRenderFinished)
    {
        for (uint32_t i = 0; i < g.maxFramesInFlight; ++i)
        {
            if (g.semRenderFinished[i]) vkDestroySemaphore(g.device, g.semRenderFinished[i], NULL);
        }
        free(g.semRenderFinished);
        g.semRenderFinished = NULL;
    }

    if (g.semImageAvailable)
    {
        for (uint32_t i = 0; i < g.maxFramesInFlight; ++i)
        {
            if (g.semImageAvailable[i]) vkDestroySemaphore(g.device, g.semImageAvailable[i], NULL);
        }
        free(g.semImageAvailable);
        g.semImageAvailable = NULL;
    }

    g.maxFramesInFlight = 0;
    g.currentFrame = 0;
    if (g.sc.framebuffers)
    {
        for (uint32_t i = 0; i < g.sc.count; ++i)
        {
            vkDestroyFramebuffer(g.device, g.sc.framebuffers[i], NULL);
        }
        free(g.sc.framebuffers);
        g.sc.framebuffers = NULL;
    }
    if (g.sc.views)
    {
        for (uint32_t i = 0; i < g.sc.count; ++i)
        {
            vkDestroyImageView(g.device, g.sc.views[i], NULL);
        }
        free(g.sc.views);
        g.sc.views = NULL;
    }
    if (g.sc.depth.view) vkDestroyImageView(g.device, g.sc.depth.view, NULL);
    if (g.sc.depth.image) { vkDestroyImage(g.device, g.sc.depth.image, NULL); vkFreeMemory(g.device, g.sc.depth.mem, NULL); }
    if (g.sc.swapchain) vkDestroySwapchainKHR(g.device, g.sc.swapchain, NULL);

    // offscreen framebuffers & images
    if (g.scene0.framebuffer) vkDestroyFramebuffer(g.device, g.scene0.framebuffer, NULL);
    if (g.scene1.framebuffer) vkDestroyFramebuffer(g.device, g.scene1.framebuffer, NULL);

    if (g.scene0.color.view) vkDestroyImageView(g.device, g.scene0.color.view, NULL);
    if (g.scene0.color.image) { vkDestroyImage(g.device, g.scene0.color.image, NULL); vkFreeMemory(g.device, g.scene0.color.mem, NULL); }

    if (g.scene1.color.view) vkDestroyImageView(g.device, g.scene1.color.view, NULL);
    if (g.scene1.color.image) { vkDestroyImage(g.device, g.scene1.color.image, NULL); vkFreeMemory(g.device, g.scene1.color.mem, NULL); }

    if (g.scene0.depth.view) vkDestroyImageView(g.device, g.scene0.depth.view, NULL);
    if (g.scene0.depth.image) { vkDestroyImage(g.device, g.scene0.depth.image, NULL); vkFreeMemory(g.device, g.scene0.depth.mem, NULL); }

    if (g.scene1.depth.view) vkDestroyImageView(g.device, g.scene1.depth.view, NULL);
    if (g.scene1.depth.image) { vkDestroyImage(g.device, g.scene1.depth.image, NULL); vkFreeMemory(g.device, g.scene1.depth.mem, NULL); }

    // recreate
    VkResult r = CreateSwapchainAndTargets(w, h, &g.sc);
    if (r != VK_SUCCESS) return r;

    r = CreatePresentDepth(&g.sc);
    if (r != VK_SUCCESS) return r;

    r = CreateOffscreenPass(&g.scene0, g.sc.format, g.sc.depth.format, g.sc.extent);
    if (r != VK_SUCCESS) return r;

    r = CreateOffscreenPass(&g.scene1, g.sc.format, g.sc.depth.format, g.sc.extent);
    if (r != VK_SUCCESS) return r;

    r = CreatePresentFramebuffers(&g.sc);
    if (r != VK_SUCCESS) return r;

    r = CreateCommandPoolAndBuffers();
    if (r != VK_SUCCESS) return r;

    r = CreateSyncObjects();
    if (r != VK_SUCCESS) return r;

    // rewire composite descriptors to new image views/layouts
    vkDestroyDescriptorPool(g.device, g.compPool, NULL);
    g.compPool = VK_NULL_HANDLE;
    g.compDescSet = VK_NULL_HANDLE;
    r = CreateCompositeDescriptorStuff();
    if (r != VK_SUCCESS) return r;

    g.winWidth = g.sc.extent.width;
    g.winHeight = g.sc.extent.height;
    g.initialized = TRUE;

    return VK_SUCCESS;
}

static VkResult HandleResize(uint32_t w, uint32_t h)
{
    if (h == 0) h = 1;
    if (w == 0) w = 1;

    if (!g.initialized)
    {
        if (g.log)
        {
            fprintf(g.log, "HandleResize(): Initialization yet not completed or failed.\n");
        }
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkResult vkResult = RecreateSwapchain(w, h);
    if (vkResult != VK_SUCCESS)
    {
        if (g.log)
        {
            fprintf(g.log, "HandleResize(): RecreateSwapchain failed with error code %d.\n", vkResult);
        }
    }
    else
    {
        if (g.log)
        {
            fprintf(g.log, "HandleResize(): resize succeeded (%u x %u).\n", g.winWidth, g.winHeight);
        }
    }
    return vkResult;
}

// ===================================================================================
// Win32 entry
// ===================================================================================

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            g.win.hwnd = hwnd;
            ZeroMem(&g.win.wp, sizeof(WINDOWPLACEMENT));
            g.win.wp.length = sizeof(WINDOWPLACEMENT);
        } break;

        case WM_SETFOCUS:
        {
            g.win.active = TRUE;
        } break;

        case WM_KILLFOCUS:
        {
            g.win.active = FALSE;
        } break;

        case WM_SIZE:
        {
            if (wp == SIZE_MINIMIZED)
            {
                g.win.minimized = TRUE;
            }
            else
            {
                g.win.minimized = FALSE;
                uint32_t w = LOWORD(lp);
                uint32_t h = HIWORD(lp);
                VkResult vkResult = HandleResize(w, h);
                if (vkResult != VK_SUCCESS && g.log)
                {
                    fprintf(g.log, "WndProc(): HandleResize failed with error code %d.\n", vkResult);
                }
            }
        } break;

        case WM_KEYDOWN:
        {
            if (wp == VK_ESCAPE)
            {
                DestroyWindow(hwnd);
            }
            else if (wp == 'F' || wp == 'f')
            {
                ToggleFullscreen();
            }
            else if (wp == '1')
            {
                g.selectedScene = 0u;
            }
            else if (wp == '2')
            {
                g.selectedScene = 1u;
            }
            else if (wp == '3')
            {
                g.selectedScene = 2u;
            }
        } break;

        case WM_CHAR:
        {
            if (wp == 'L' || wp == 'l')
            {
                g.lighting = (g.lighting == TRUE) ? FALSE : TRUE;
            }
        } break;

        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int cmdShow)
{
    // open log
    g.log = fopen(LOG_FILE, "w");
    if (!g.log)
    {
        FatalMsgA("Error", "Cannot open compositor log file");
        return 0;
    }

    // window
    WNDCLASSEX wc;
    ZeroMem(&wc, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = TEXT("ARTR_CompositorClass");
    RegisterClassEx(&wc);

    int sw = GetSystemMetrics(SM_CXSCREEN);
    int sh = GetSystemMetrics(SM_CYSCREEN);
    int x  = (sw - WIN_WIDTH) / 2;
    int y  = (sh - WIN_HEIGHT) / 2;

    HWND hwnd = CreateWindowEx(WS_EX_APPWINDOW,
                               wc.lpszClassName,
                               TEXT("Scene Compositor"),
                               WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                               x, y, WIN_WIDTH, WIN_HEIGHT,
                               NULL, NULL, hInst, NULL);
    g.win.hwnd = hwnd;

    ShowWindow(hwnd, cmdShow);
    UpdateWindow(hwnd);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);

    // enable validation by default (matches Scene0/Scene1 behaviour)
    g.validation = TRUE;

    // init Vulkan
    VkResult vkResult = InitVulkan(WIN_WIDTH, WIN_HEIGHT);
    if (vkResult != VK_SUCCESS)
    {
        FatalMsgA("Error", "InitVulkan failed");
        CleanupVulkan();
        if (g.log) { fprintf(g.log, "InitVulkan failed: %d\n", vkResult); fclose(g.log); g.log = NULL; }
        return 0;
    }

    // game loop
    MSG msg;
    BOOL run = TRUE;
    while (run)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                run = FALSE;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!run) break;

        if (!g.win.minimized && g.win.active)
        {
            UpdateAnim();

            vkResult = DrawFrame();
            if (vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR)
            {
                RECT rc;
                GetClientRect(hwnd, &rc);
                uint32_t w = (uint32_t)(rc.right - rc.left);
                uint32_t h = (uint32_t)(rc.bottom - rc.top);
                vkResult = HandleResize(w, h);
                if (vkResult != VK_SUCCESS)
                {
                    if (g.log)
                    {
                        fprintf(g.log, "WinMain(): HandleResize failed with error code %d.\n", vkResult);
                    }
                    run = FALSE;
                }
            }
            else if (vkResult != VK_SUCCESS)
            {
                // break on fatal
                run = FALSE;
            }
        }
    }

    vkDeviceWaitIdle(g.device);
    CleanupVulkan();

    if (g.log)
    {
        fprintf(g.log, "Compositor terminated gracefully.\n");
        fclose(g.log);
        g.log = NULL;
    }
    return 0;
}
