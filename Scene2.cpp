//Header file
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//header file for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Scene2.h"
//This inclusion of <vulkan/vulkan.h> is needed in all platforms
//to distinguish the platfrom, below macro is needed
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

//glm related macros and header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE   //clipping values between 0 and 1 for depth
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
// Vulkan related library
#pragma comment(lib, "vulkan-1.lib")

//Macros
/* History of Graphics adapter
Aspect ratio od 4:3 is supported by all monitors
Monochrome
CGA -> Color Graphics Adapter
EGA -> Extended Graphics Adapter
VGA -> Video Graphics Adapter
SVGA -> Super Video Graphics Adapter (4:3 Aspect ratio)
*/
#define WIN_WIDTH  800
#define WIN_HEIGHT 600

//Callback function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

typedef struct Win32WindowContext_Scene2 Win32WindowContext_Scene2;
typedef struct GlobalContext_Scene2 GlobalContext_Scene2;

typedef struct Scene2Win32FunctionTable
{
    LRESULT (CALLBACK *WndProc)(HWND, UINT, WPARAM, LPARAM);
    void (*ToggleFullscreen)(void);
    VkResult (*Initialize)(void);
    VkResult (*Resize)(int, int);
    VkResult (*Display)(void);
    void (*Update)(void);
    void (*Uninitialize)(void);
} Scene2Win32FunctionTable;

typedef struct Scene2FunctionTable
{
    void (*InitializeWin32WindowContext_Scene2)(Win32WindowContext_Scene2*);
    void (*InitializeGlobalContext_Scene2)(GlobalContext_Scene2*);
    int (WINAPI *WinMain)(HINSTANCE, HINSTANCE, LPSTR, int);
    VkResult (*createVulkanInstance)(void);
    VkResult (*fillInstanceExtensionNames)(void);
    VkResult (*fillValidationLayerNames)(void);
    VkResult (*createValidationCallbackFunction)(void);
    VkResult (*getSupportedSurface)(void);
    VkResult (*getPhysicalDevice)(void);
    VkResult (*printVkInfo)(void);
    VkResult (*fillDeviceExtensionNames)(void);
    VkResult (*createVulkanDevice)(void);
    void (*getDeviceQueue)(void);
    VkResult (*getPhysicalDeviceSurfaceFormatAndColorSpace)(void);
    VkResult (*getPhysicalDevicePresentMode)(void);
    VkResult (*createSwapchain)(VkBool32);
    VkResult (*createImagesAndImageViews)(void);
    VkResult (*GetSupportedDepthFormat)(void);
    VkResult (*createCommandPool)(void);
    VkResult (*createCommandBuffers)(void);
    VkResult (*createVertexBuffer)(void);
    VkResult (*createTexture)(const char*);
    VkResult (*createUniformBuffer)(void);
    VkResult (*updateUniformBuffer)(void);
    VkResult (*createShaders)(void);
    VkResult (*createDescriptorSetLayout)(void);
    VkResult (*createPipelineLayout)(void);
    VkResult (*createDescriptorPool)(void);
    VkResult (*createDescriptorSet)(void);
    VkResult (*createRenderPass)(void);
    VkResult (*createPipeline)(void);
    VkResult (*createFrameBuffers)(void);
    VkResult (*createSemaphores)(void);
    VkResult (*createFences)(void);
    VkResult (*buildCommandBuffers)(void);
    VkBool32 (VKAPI_PTR *debugReportCallback)(VkDebugReportFlagsEXT,
                                              VkDebugReportObjectTypeEXT,
                                              uint64_t,
                                              size_t,
                                              int32_t,
                                              const char*,
                                              const char*,
                                              void*);
} Scene2FunctionTable;

static void PopulateFunctionTable_Scene2(void);

Scene2Win32FunctionTable Win32FunctionTable_Scene2;
Scene2FunctionTable FunctionTable_Scene2;

typedef struct Win32WindowContext_Scene2
{
    const char* appName;
    HWND hwnd;
    HDC hdc;
    HGLRC hrc;
    DWORD windowStyle;
    WINDOWPLACEMENT previousPlacement;
    int isActiveWindow;
    int isFullscreen;
    int isWindowMinimized;
    int height;
    int width;
} Win32WindowContext_Scene2;

static void InitializeWin32WindowContext_Scene2(Win32WindowContext_Scene2* context)
{
    if (context == NULL)
    {
        return;
    }

    context->appName = "ARTR";
    context->hwnd = NULL;
    context->hdc = NULL;
    context->hrc = NULL;
    context->windowStyle = 0;
    memset(&context->previousPlacement, 0, sizeof(WINDOWPLACEMENT));
    context->isActiveWindow = 0;
    context->isFullscreen = 0;
    context->isWindowMinimized = FALSE;
    context->height = 0;
    context->width = 0;
}

typedef struct GlobalContext_Scene2
{
    Win32WindowContext_Scene2 win32Context;
    FILE* logFile;
    uint32_t instanceExtensionCount;
    const char* instanceExtensionNames[3];
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    uint32_t graphicsQueueFamilyIndex;
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    VkPhysicalDevice* physicalDevices;
    uint32_t physicalDeviceCount;
    uint32_t deviceExtensionCount;
    const char* deviceExtensionNames[1];
    VkDevice device;
    VkQueue queue;
    VkFormat colorFormat;
    VkColorSpaceKHR colorSpace;
    VkPresentModeKHR presentMode;
    int windowWidth;
    int windowHeight;
    VkSwapchainKHR swapchain;
    VkExtent2D swapchainExtent;
    uint32_t swapchainImageCount;
    VkImage* swapchainImages;
    VkImageView* swapchainImageViews;
    VkFormat depthFormat;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;
    VkCommandPool commandPool;
    VkCommandBuffer* commandBuffers;
    VkRenderPass renderPass;
    VkFramebuffer* framebuffers;
    VkSemaphore backBufferSemaphore;
    VkSemaphore renderCompleteSemaphore;
    VkFence* fences;
    VkClearColorValue clearColor;
    VkClearDepthStencilValue clearDepthStencil;
    VkBool32 isInitialized;
    uint32_t currentImageIndex;
    BOOL enableValidation;
    uint32_t validationLayerCount;
    const char* validationLayerNames[1];
    VkDebugReportCallbackEXT debugReportCallback;
    PFN_vkDestroyDebugReportCallbackEXT destroyDebugReportCallback;
    struct VertexBufferData
    {
        VkBuffer vkBuffer;
        VkDeviceMemory vkDeviceMemory;
    } positionVertexData,
      texcoordVertexData;
    struct UniformBufferData
    {
        VkBuffer vkBuffer;
        VkDeviceMemory vkDeviceMemory;
    } uniformData;
    struct UniformBufferObject
    {
        glm::mat4 modelMatrix;
        glm::mat4 viewMatrix;
        glm::mat4 projectionMatrix;
    } uniformBufferObject;
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSet descriptorSet;
    VkViewport viewport;
    VkRect2D scissor;
    VkPipeline pipeline;
    float angle;
    VkImage textureImage;
    VkDeviceMemory textureMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    PFN_vkCreateDebugReportCallbackEXT createDebugReportCallback;
} GlobalContext_Scene2;

static void InitializeGlobalContext_Scene2(GlobalContext_Scene2* context)
{
    int i = 0;

    if (context == NULL)
    {
        return;
    }

    memset(context, 0, sizeof(GlobalContext_Scene2));
    FunctionTable_Scene2.InitializeWin32WindowContext_Scene2(&context->win32Context);

    context->instanceExtensionCount = 0;
    for (i = 0; i < 3; ++i)
    {
        context->instanceExtensionNames[i] = NULL;
    }

    context->instance = VK_NULL_HANDLE;
    context->surface = VK_NULL_HANDLE;
    context->physicalDevice = VK_NULL_HANDLE;
    context->graphicsQueueFamilyIndex = UINT32_MAX;
    context->physicalDevices = NULL;
    context->physicalDeviceCount = 0;
    context->deviceExtensionCount = 0;
    context->deviceExtensionNames[0] = NULL;
    context->device = VK_NULL_HANDLE;
    context->queue = VK_NULL_HANDLE;
    context->colorFormat = VK_FORMAT_UNDEFINED;
    context->colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    context->presentMode = VK_PRESENT_MODE_FIFO_KHR;
    context->windowWidth = WIN_WIDTH;
    context->windowHeight = WIN_HEIGHT;
    context->swapchain = VK_NULL_HANDLE;
    context->swapchainExtent.width = 0;
    context->swapchainExtent.height = 0;
    context->swapchainImageCount = UINT32_MAX;
    context->swapchainImages = NULL;
    context->swapchainImageViews = NULL;
    context->depthFormat = VK_FORMAT_UNDEFINED;
    context->depthImage = VK_NULL_HANDLE;
    context->depthImageMemory = VK_NULL_HANDLE;
    context->depthImageView = VK_NULL_HANDLE;
    context->commandPool = VK_NULL_HANDLE;
    context->commandBuffers = NULL;
    context->renderPass = VK_NULL_HANDLE;
    context->framebuffers = NULL;
    context->backBufferSemaphore = VK_NULL_HANDLE;
    context->renderCompleteSemaphore = VK_NULL_HANDLE;
    context->fences = NULL;
    context->isInitialized = VK_FALSE;
    context->currentImageIndex = UINT32_MAX;
    context->enableValidation = TRUE;
    context->validationLayerCount = 0;
    context->validationLayerNames[0] = NULL;
    context->debugReportCallback = VK_NULL_HANDLE;
    context->destroyDebugReportCallback = NULL;
    context->positionVertexData.vkBuffer = VK_NULL_HANDLE;
    context->positionVertexData.vkDeviceMemory = VK_NULL_HANDLE;
    context->texcoordVertexData.vkBuffer = VK_NULL_HANDLE;
    context->texcoordVertexData.vkDeviceMemory = VK_NULL_HANDLE;
    context->uniformData.vkBuffer = VK_NULL_HANDLE;
    context->uniformData.vkDeviceMemory = VK_NULL_HANDLE;
    context->vertexShaderModule = VK_NULL_HANDLE;
    context->fragmentShaderModule = VK_NULL_HANDLE;
    context->descriptorSetLayout = VK_NULL_HANDLE;
    context->pipelineLayout = VK_NULL_HANDLE;
    context->descriptorPool = VK_NULL_HANDLE;
    context->descriptorSet = VK_NULL_HANDLE;
    context->viewport.x = 0.0f;
    context->viewport.y = 0.0f;
    context->viewport.width = 0.0f;
    context->viewport.height = 0.0f;
    context->viewport.minDepth = 0.0f;
    context->viewport.maxDepth = 0.0f;
    context->scissor.offset.x = 0;
    context->scissor.offset.y = 0;
    context->scissor.extent.width = 0;
    context->scissor.extent.height = 0;
    context->pipeline = VK_NULL_HANDLE;
    context->angle = 0.0f;
    context->textureImage = VK_NULL_HANDLE;
    context->textureMemory = VK_NULL_HANDLE;
    context->textureImageView = VK_NULL_HANDLE;
    context->textureSampler = VK_NULL_HANDLE;
    context->createDebugReportCallback = NULL;
}

GlobalContext_Scene2 gContext_Scene2;

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
        //local variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[255];
	int bDone = 0;
    VkResult vkResult = VK_SUCCESS;

    PopulateFunctionTable_Scene2();

    InitializeGlobalContext_Scene2(&gContext_Scene2);


	//code
    gContext_Scene2.logFile = fopen("SSA_Log.txt", "w+");
	if (gContext_Scene2.logFile == NULL)
	{
		MessageBox(NULL, TEXT("Cannot create/open SSA_Log.txt file"), TEXT("FILE IO ERROR"), MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gContext_Scene2.logFile, "WinMain() --> Program started successfully\n");
	}

    wsprintf(szAppName, TEXT("%s"), gContext_Scene2.win32Context.appName);

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
        wndclass.lpfnWndProc = Win32FunctionTable_Scene2.WndProc;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hInstance = hInstance;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("SSA_VULKAN"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		GetSystemMetrics(SM_CXSCREEN)/2 - WIN_WIDTH/2,
		GetSystemMetrics(SM_CYSCREEN)/2 - WIN_HEIGHT/2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL, //Parent Window Handle
		NULL, //Menu Handle
		hInstance,
		NULL);

    gContext_Scene2.win32Context.hwnd = hwnd;

        vkResult = Win32FunctionTable_Scene2.Initialize();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "WinMain() --> Initialize() is failed\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "WinMain() --> Initialize() is succedded\n");
    }
    //Error checking of Initialize

	ShowWindow(hwnd, iCmdShow);
    Win32FunctionTable_Scene2.Update();
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Game loop
	while (bDone == 0)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = 1;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
                        if (gContext_Scene2.win32Context.isActiveWindow == 1)
			{
				//Here you should call update() for OpenGL rendering
                Win32FunctionTable_Scene2.Update();
				//Here you should call display() for OpenGL rendering
                if(gContext_Scene2.win32Context.isWindowMinimized == FALSE)
                {
                    vkResult = Win32FunctionTable_Scene2.Display();
                    if((vkResult != VK_FALSE) && (vkResult != VK_SUCCESS) && (vkResult != VK_ERROR_OUT_OF_DATE_KHR) && (vkResult != VK_SUBOPTIMAL_KHR))
                    {
                        fprintf(gContext_Scene2.logFile, "WinMain() --> call to Display() failed\n");
                        bDone = TRUE;
                    }
                }
			}
		}
	}

        Win32FunctionTable_Scene2.Uninitialize();

	return((int)(msg.wParam));
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

	int height = 0;
    int width = 0;
    int length = sizeof(WINDOWPLACEMENT);
    
    switch (iMsg)
	{

    case WM_CREATE:
        memset(&gContext_Scene2.win32Context.previousPlacement, 0, length);
        break;

	case WM_SETFOCUS:
                gContext_Scene2.win32Context.isActiveWindow = 1;
		break;

	case WM_KILLFOCUS:
                gContext_Scene2.win32Context.isActiveWindow = 0;
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_SIZE:
        if(wParam == SIZE_MINIMIZED)
        {
            gContext_Scene2.win32Context.isWindowMinimized = TRUE;
        }
        else
        {
            gContext_Scene2.win32Context.isWindowMinimized = FALSE;
            Win32FunctionTable_Scene2.Resize(LOWORD(lParam), HIWORD(lParam));
        }
        
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
		case 0x66:
                        Win32FunctionTable_Scene2.ToggleFullscreen();
			break;

		default:
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
                Win32FunctionTable_Scene2.Uninitialize();
		PostQuitMessage(0);
		break;

	default: break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void ToggleFullscreen(void)
{
	//Local variable declaration
	MONITORINFO mi = { sizeof(MONITORINFO) };

	//Code
        if (gContext_Scene2.win32Context.isFullscreen == 0) //If current window is normal(Not Fullscreen)
        {
                gContext_Scene2.win32Context.windowStyle = GetWindowLong(gContext_Scene2.win32Context.hwnd, GWL_STYLE); //gContext_Scene2.windowStyle will get WS_OVERLAPPEDWINDOW information
                if (gContext_Scene2.win32Context.windowStyle & WS_OVERLAPPEDWINDOW)
                {
                        if (GetWindowPlacement(gContext_Scene2.win32Context.hwnd, &gContext_Scene2.win32Context.previousPlacement) && GetMonitorInfo(MonitorFromWindow(gContext_Scene2.win32Context.hwnd, MONITORINFOF_PRIMARY), &mi))
                        {
                                SetWindowLong(gContext_Scene2.win32Context.hwnd, GWL_STYLE, (gContext_Scene2.win32Context.windowStyle & ~WS_OVERLAPPEDWINDOW)); //Removing flags of WS_OVERLAPPEDWINDOW
                                SetWindowPos(gContext_Scene2.win32Context.hwnd,
                                        HWND_TOP, //HWND_TOP -> for making it on top order
                                        mi.rcMonitor.left, // left coordinate of monitor
                                        mi.rcMonitor.top, // top coordinate of monitor
					mi.rcMonitor.right - mi.rcMonitor.left, // width coordinate of monitor
					mi.rcMonitor.bottom - mi.rcMonitor.top, // height coordinate of monitor
					SWP_NOZORDER | //Window flag --> don't change the Z order
					SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
			}
                }
                ShowCursor(FALSE);  //disappear the cursor in full screen or Game mode
                gContext_Scene2.win32Context.isFullscreen = 1;
        }
        else
        {
                SetWindowLong(gContext_Scene2.win32Context.hwnd, GWL_STYLE, (gContext_Scene2.win32Context.windowStyle | WS_OVERLAPPEDWINDOW));//Restoring flags of WS_OVERLAPPEDWINDOW
                SetWindowPlacement(gContext_Scene2.win32Context.hwnd, &gContext_Scene2.win32Context.previousPlacement); //setting placement of window
                SetWindowPos(gContext_Scene2.win32Context.hwnd,
                        HWND_TOP,
                        0,  //already set in gContext_Scene2.win32Context.previousPlacement
                        0,  //already set in gContext_Scene2.win32Context.previousPlacement
                        0,  //already set in gContext_Scene2.win32Context.previousPlacement
                        0,  //already set in gContext_Scene2.win32Context.previousPlacement
                        SWP_NOMOVE | // do not change x, y cordinates for starting position
                        SWP_NOSIZE | // do not change height and width cordinates
			SWP_NOOWNERZORDER | //do not change the position even if its parent window is changed
			SWP_NOZORDER | //Window flag --> don't change the Z order
			SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
                ShowCursor(TRUE); //Appear the cursor in full screen or Game mode
                gContext_Scene2.win32Context.isFullscreen = 0;
        }
}


VkResult Initialize(void)
{
    //function declaration
    VkResult createVulkanInstance(void);
    VkResult getSupportedSurface(void);
    VkResult getPhysicalDevice(void);
    VkResult printVkInfo(void);
    VkResult createVulkanDevice(void);
    void getDeviceQueue(void);
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandPool(void);
    VkResult createCommandBuffers(void);
    VkResult createVertexBuffer(void);
    VkResult createTexture(const char*);
    VkResult createUniformBuffer(void);
    VkResult createShaders(void);
    VkResult createDescriptorSetLayout(void);
    VkResult createPipelineLayout(void);
    VkResult createDescriptorPool(void);
    VkResult createDescriptorSet(void);
    VkResult createRenderPass(void);
    VkResult createPipeline(void);
    VkResult createFrameBuffers(void);
    VkResult createSemaphores(void);
    VkResult createFences(void);
    VkResult buildCommandBuffers(void);
        
    //variable delaration
    VkResult vkResult = VK_SUCCESS;

    //code
    vkResult = FunctionTable_Scene2.createVulkanInstance();
    /*Main points:
        vkEnumerateInstanceExtensionProperties()
        
        struct --> vkApplicationInfo
        struct --> vkInstanceCreateInfo
        vkCreateInstance()
    */
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createVulkanInstance() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createVulkanInstance() is succedded\n");
    }

    //CreateVulkan Presntation Surface
    vkResult = FunctionTable_Scene2.getSupportedSurface();
    /* Main points:
        struct --> vkWin32SurfaceCreateInfoKHR
        vkCreateWin32SurfaceKHR()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> getSupportedSurface() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> getSupportedSurface() is succedded\n");
    }

    //Get required Physical Device and its Queue family index
    vkResult = FunctionTable_Scene2.getPhysicalDevice();
    /* Main points:
        vkEnumeratePhysicalDevices()
        vkGetPhysicalDeviceQueueFamilyProperties()
        vkGetPhysicalDeviceSurfaceSupportKHR()
        vkGetPhysicalDeviceMemoryProperties()
        vkGetPhysicalDeviceFeatures()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> getPhysicalDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> getPhysicalDevice() is succedded\n");
    }
    
    //Printing Vulkan Infomration
    vkResult = FunctionTable_Scene2.printVkInfo();
    /* Main points:
        vkGetPhysicalDeviceProperties()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> printVkInfo() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> printVkInfo() is succedded\n");
    }
    
    vkResult = FunctionTable_Scene2.createVulkanDevice();
    /* Main Points:
        vkEnumerateDeviceExtensionProperties()
        struct --> vkDeviceQueueCreateInfo
        struct --> vkDeviceCreateInfo
        vkCreateDevice()
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createVulkanDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createVulkanDevice() is succedded\n");
    }
    
    //getDeviceQueue
    FunctionTable_Scene2.getDeviceQueue();
    /* Main Points
        vkGetDeviceQueue()
    */

        
    //Swapchain
    vkResult = FunctionTable_Scene2.createSwapchain(VK_FALSE);
    /*Main Points:
        vkGetPhysicalDeviceSurfaceFormatsKHR()
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
        vkGetPhysicalDeviceSurfacePresentModesKHR()
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createSwapchain() is failed %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createSwapchain() is succedded\n");
    }
    
    //Create Vulkan Images and ImageViews
    vkResult = FunctionTable_Scene2.createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createImagesAndImageViews() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createImagesAndImageViews() is succedded\n");
    }
    
    //Create Command Pool
    vkResult = FunctionTable_Scene2.createCommandPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createCommandPool() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createCommandPool() is succedded\n");
    }
    
    //Command Buffers
    vkResult = FunctionTable_Scene2.createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createCommandBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createCommandBuffers() is succedded\n");
    }
    
    //create VertexBUffer
    vkResult = FunctionTable_Scene2.createVertexBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createVertexBuffer() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createVertexBuffer() is succedded\n");
    }
    
    //create texture
    vkResult = FunctionTable_Scene2.createTexture("EndCredits.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createTexture() is failed for Stone texture %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createTexture() is succedded for Stone texture\n");
    }
    
    //create Uniform Buffer
    vkResult = FunctionTable_Scene2.createUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createUniformBuffer() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createUniformBuffer() is succedded\n");
    }
    
    //Create RenderPass
    vkResult = FunctionTable_Scene2.createShaders();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createShaders() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createShaders() is succedded\n");
    }
    
    
    
    //Create Dewscriptor Set Layout
    vkResult = FunctionTable_Scene2.createDescriptorSetLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createDescriptorSetLayout() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createDescriptorSetLayout() is succedded\n");
    }
    
    //Create Descriptor Set Layout
    vkResult = FunctionTable_Scene2.createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createPipelineLayout() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createPipelineLayout() is succedded\n");
    }
    
    //create Descriptor pool
    vkResult = FunctionTable_Scene2.createDescriptorPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createDescriptorPool() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createDescriptorPool() is succedded\n");
    }
    
    //create Descriptor set
    vkResult = FunctionTable_Scene2.createDescriptorSet();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createDescriptorSet() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createDescriptorPool() is succedded\n");
    }
    
    //Create RenderPass
    vkResult = FunctionTable_Scene2.createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createRenderPass() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createRenderPass() is succedded\n");
    }
    
    //Create Pipeline
    vkResult = FunctionTable_Scene2.createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createPipeline() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createPipeline() is succedded\n");
    }
    
    
    //CreateBuffer
    vkResult = FunctionTable_Scene2.createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createFrameBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createFrameBuffers() is succedded\n");
    }
    
    //CreateSemaphores
    vkResult = FunctionTable_Scene2.createSemaphores();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createSemaphores() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createSemaphores() is succedded\n");
    }
    
    //CreateFences
    vkResult = FunctionTable_Scene2.createFences();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createFences() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> createFences() is succedded\n");
    }
    
    //Initlaize gContext_Scene2.clearColor
    memset((void*)&gContext_Scene2.clearColor, 0, sizeof(VkClearColorValue));
    gContext_Scene2.clearColor.float32[0] = 0.0f;// R
    gContext_Scene2.clearColor.float32[1] = 0.0f;// G
    gContext_Scene2.clearColor.float32[2] = 0.0f;// B
    gContext_Scene2.clearColor.float32[3] = 1.0f;//Analogous to glClearColor()
    
    memset((void*)&gContext_Scene2.clearDepthStencil, 0, sizeof(VkClearDepthStencilValue));
    //setDefaultClearDepth
    gContext_Scene2.clearDepthStencil.depth = 1.0f; //flaot value
    //setDeafaultStencilValue
    gContext_Scene2.clearDepthStencil.stencil = 0; //uint32_t value
    
    //BuildCommandBuffers
    vkResult = FunctionTable_Scene2.buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> buildCommandBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "Initialize() --> buildCommandBuffers() is succedded\n");
    }
    
    //Initialization is completed
    gContext_Scene2.isInitialized = TRUE;
    fprintf(gContext_Scene2.logFile, "Initialize() --> Initialization is completed successfully\n");
  
    return vkResult;
}


VkResult Resize(int width, int height)
{
    //Function declarations
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandBuffers(void);
    VkResult createPipelineLayout(void);
    VkResult createPipeline(void);
    VkResult createRenderPass(void);
    VkResult createFrameBuffers(void);
    VkResult buildCommandBuffers(void);
    
    
    //variables
    VkResult vkResult = VK_SUCCESS;
	//code
	if (height == 0)
		height = 1;
    
    gContext_Scene2.win32Context.height = height;
    gContext_Scene2.win32Context.width = width;
    

    //check the gContext_Scene2.isInitialized variable
    if(gContext_Scene2.isInitialized == FALSE)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> Initialization yet not completed or failed\n");  
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    
    //As recreation of swapchain is needed we are goring to repeat many steps of initialized again, hence set bINitialized to \False again
    gContext_Scene2.isInitialized = FALSE;

    //Set gloabl winwidth and winheight variable
    gContext_Scene2.windowWidth = width;
    gContext_Scene2.windowHeight = height;
    
    //wait for device to complete inhand tasks
    vkDeviceWaitIdle(gContext_Scene2.device);
    fprintf(gContext_Scene2.logFile, "Resize() --> vkDeviceWaitIdle() is done\n");
    
    //check presence of swapchain
    if(gContext_Scene2.swapchain == VK_NULL_HANDLE)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> swapchain is already NULL cannot proceed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
    }
    
    //Destroy vkframebuffer
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(gContext_Scene2.device, gContext_Scene2.framebuffers[i], NULL);
    }
    if(gContext_Scene2.framebuffers)
    {
        free(gContext_Scene2.framebuffers);
        gContext_Scene2.framebuffers = NULL;
        fprintf(gContext_Scene2.logFile, "Resize() --> gContext_Scene2.framebuffers() is done\n");
    }
    
    //Destroy Commandbuffer
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(gContext_Scene2.device, gContext_Scene2.commandPool, 1, &gContext_Scene2.commandBuffers[i]);
        fprintf(gContext_Scene2.logFile, "Resize() --> vkFreeCommandBuffers() is done\n");
    }
    
    if(gContext_Scene2.commandBuffers)
    {
        free(gContext_Scene2.commandBuffers);
        gContext_Scene2.commandBuffers = NULL;
        fprintf(gContext_Scene2.logFile, "Resize() --> gContext_Scene2.commandBuffers is freed\n");
    }
    
    
    //Destroy Pipeline
    if(gContext_Scene2.pipeline)
    {
        vkDestroyPipeline(gContext_Scene2.device, gContext_Scene2.pipeline, NULL);
        gContext_Scene2.pipeline = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Resize() --> vkDestroyPipeline() is done\n");
    }
    
    //Destroy gContext_Scene2.pipelineLayout
    if(gContext_Scene2.pipelineLayout)
    {
        vkDestroyPipelineLayout(gContext_Scene2.device, gContext_Scene2.pipelineLayout, NULL);
        gContext_Scene2.pipelineLayout = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Resize() --> gContext_Scene2.pipelineLayout() is done\n");
    }
    
    //Destroy Renderpass
    if(gContext_Scene2.renderPass)
    {
        vkDestroyRenderPass(gContext_Scene2.device, gContext_Scene2.renderPass, NULL);
        gContext_Scene2.renderPass = VK_NULL_HANDLE;
    }
    
    //destroy depth image view
    if(gContext_Scene2.depthImageView)
    {
        vkDestroyImageView(gContext_Scene2.device, gContext_Scene2.depthImageView, NULL);
        gContext_Scene2.depthImageView = VK_NULL_HANDLE;
    }
    
    //destroy device memory for depth image
    if(gContext_Scene2.depthImageMemory)
    {
        vkFreeMemory(gContext_Scene2.device, gContext_Scene2.depthImageMemory, NULL);
        gContext_Scene2.depthImageMemory = VK_NULL_HANDLE;
    }
    
    if(gContext_Scene2.depthImage)
    {
        vkDestroyImage(gContext_Scene2.device, gContext_Scene2.depthImage, NULL);
        gContext_Scene2.depthImage = VK_NULL_HANDLE;
    }
    //destory image views
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        vkDestroyImageView(gContext_Scene2.device, gContext_Scene2.swapchainImageViews[i], NULL);
        fprintf(gContext_Scene2.logFile, "Resize() --> vkDestoryImageView() is done\n");
    }
    
    if(gContext_Scene2.swapchainImageViews)
    {
        free(gContext_Scene2.swapchainImageViews);
        gContext_Scene2.swapchainImageViews = NULL;
        fprintf(gContext_Scene2.logFile, "Resize() --> gContext_Scene2.swapchainImageViews is freed\n");
    }
    
    //free swapchainImages
    // for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    // {
        // vkDestroyImage(gContext_Scene2.device, gContext_Scene2.swapchainImages[i], NULL);
        // fprintf(gContext_Scene2.logFile, "Resize() --> vkDestroyImage() is done\n");
    // }
    
    
    if(gContext_Scene2.swapchainImages)
    {
        free(gContext_Scene2.swapchainImages);
        gContext_Scene2.swapchainImages = NULL;
        fprintf(gContext_Scene2.logFile, "Resize() --> gContext_Scene2.swapchainImages is freed\n");
    }
    
    //Destory Swapchain
    if(gContext_Scene2.swapchain)
    {
        vkDestroySwapchainKHR(gContext_Scene2.device, gContext_Scene2.swapchain, NULL);
        gContext_Scene2.swapchain = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Resize() --> vkSwapchainCreateInfoKHR() is done\n");
    }
    
    //RECREATE FOR RESIZE
    
    //Swapchain
    vkResult = FunctionTable_Scene2.createSwapchain(VK_FALSE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> createSwapchain() is failed %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    //Create Vulkan Images and ImageViews
    vkResult = FunctionTable_Scene2.createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> createImagesAndImageViews() is failed %d\n", vkResult);
    }
    
    //Create RenderPass
    vkResult = FunctionTable_Scene2.createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> createRenderPass() is failed %d\n", vkResult);
    }
    
    //Create Descriptor Set Layout
    vkResult = FunctionTable_Scene2.createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> createPipelineLayout() is failed %d\n", vkResult);
    }
    
    //Create Pipeline
    vkResult = FunctionTable_Scene2.createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> createPipeline() is failed %d\n", vkResult);
    }
      
    //CreateBuffer
    vkResult = FunctionTable_Scene2.createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> createFrameBuffers() is failed %d\n", vkResult);
    }
    
    //Command Buffers
    vkResult = FunctionTable_Scene2.createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> createCommandBuffers() is failed %d\n", vkResult);
    }
    
    //BuildCommandBuffers
    vkResult = FunctionTable_Scene2.buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Resize() --> buildCommandBuffers() is failed %d\n", vkResult);
    }

    gContext_Scene2.isInitialized = TRUE;

    return vkResult;
}


VkResult Display(void)
{
    //Function Declarations
    VkResult Resize(int, int);
    VkResult updateUniformBuffer(void);
    
    //variable declarations
    VkResult vkResult = VK_SUCCESS; 

    //code
    // if control comes here before initlaization is completed, return FALSE
    if(gContext_Scene2.isInitialized == FALSE)
    {
        fprintf(gContext_Scene2.logFile, "Display() --> Initialization yet not completed\n");
        return (VkResult)VK_FALSE;
    }
    
    //acquire index of next swapchin image
    //if timour occurs, then function returns VK_NOT_READY
    vkResult = vkAcquireNextImageKHR(gContext_Scene2.device,
                                     gContext_Scene2.swapchain,
                                     UINT64_MAX, //waiting time in nanaoseconds for swapchain to get the image
                                     gContext_Scene2.backBufferSemaphore, //semaphore, waiting for another queue to relaease the image held by another queue demanded by swapchain, (InterQueue semaphore)
                                     VK_NULL_HANDLE, //Fence, when you want to halt host also, for device::: (Use Semaphore and fences exclusively, using both is not recommended(Redbook)
                                     &gContext_Scene2.currentImageIndex);
    if(vkResult != VK_SUCCESS)
    {
        if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
        {
            Win32FunctionTable_Scene2.Resize(gContext_Scene2.windowWidth, gContext_Scene2.windowHeight);
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "Display() --> vkAcquireNextImageKHR() is failed errorcode = %d\n", vkResult);
            return vkResult;   
        }
    }
    
    //use fence to allow host to wait for completion of execution of previous commandbuffer
    vkResult = vkWaitForFences(gContext_Scene2.device,
                               1, //waiting for how many fences
                               &gContext_Scene2.fences[gContext_Scene2.currentImageIndex], //Which fence
                               VK_TRUE, // wait till all fences get signalled(Blocking and unblocking function)
                               UINT64_MAX); //waiting time in nanaoseconds
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Display() --> vkWaitForFences() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }

    //Now make Fences execution of next command buffer
    vkResult = vkResetFences(gContext_Scene2.device,
                             1, //How many fences to reset
                             &gContext_Scene2.fences[gContext_Scene2.currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Display() --> vkResetFences() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    
    //One of the mmeber of vkSubmitinfo structure requires array of pipeline stages, we have only one of the completion of color attachment output, still we need 1 member array
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    //Declare memset and initialize VkSubmitInfo structure
    VkSubmitInfo vkSubmitInfo;
    memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
    
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &gContext_Scene2.backBufferSemaphore;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &gContext_Scene2.commandBuffers[gContext_Scene2.currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &gContext_Scene2.renderCompleteSemaphore;
    
    //Now submit our work to the Queue
    vkResult = vkQueueSubmit(gContext_Scene2.queue,
                             1,
                             &vkSubmitInfo,
                             gContext_Scene2.fences[gContext_Scene2.currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Display() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    
    //We are going to present rendered image after declaring and initlaizing VkPresentInfoKHR structure
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
    
    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &gContext_Scene2.swapchain;
    vkPresentInfoKHR.pImageIndices = &gContext_Scene2.currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &gContext_Scene2.renderCompleteSemaphore;
    
    //Now present the Queue
    vkResult = vkQueuePresentKHR(gContext_Scene2.queue, &vkPresentInfoKHR);
    if(vkResult != VK_SUCCESS)
    {
        if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
        {
            Win32FunctionTable_Scene2.Resize(gContext_Scene2.windowWidth, gContext_Scene2.windowHeight);
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "Display() --> vkQueuePresentKHR() is failed errorcode = %d\n", vkResult);
            return vkResult;
        }
    }
    
    vkResult = FunctionTable_Scene2.updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "Display() --> updateUniformBuffer() is failed errorcode = %d\n", vkResult);
    }
        
    //validation
    vkDeviceWaitIdle(gContext_Scene2.device);    
    
    return vkResult;
}
    

void Update(void)
{
    gContext_Scene2.angle = gContext_Scene2.angle + 0.0f;
}


void Uninitialize(void)
{
    //code
    if (gContext_Scene2.win32Context.isFullscreen == 1)
    {
        gContext_Scene2.win32Context.windowStyle = GetWindowLong(gContext_Scene2.win32Context.hwnd, GWL_STYLE);
        SetWindowLong(gContext_Scene2.win32Context.hwnd, GWL_STYLE, (gContext_Scene2.win32Context.windowStyle | WS_OVERLAPPEDWINDOW));//Restoring flags of WS_OVERLAPPEDWINDOW
        SetWindowPlacement(gContext_Scene2.win32Context.hwnd, &gContext_Scene2.win32Context.previousPlacement); //setting placement of window
        SetWindowPos(gContext_Scene2.win32Context.hwnd,
            HWND_TOP,
            0,  //already set in gContext_Scene2.win32Context.previousPlacement
            0,  //already set in gContext_Scene2.win32Context.previousPlacement
            0,  //already set in gContext_Scene2.win32Context.previousPlacement
            0,  //already set in gContext_Scene2.win32Context.previousPlacement
            SWP_NOMOVE | // do not change x, y cordinates for starting position
            SWP_NOSIZE | // do not change height and width cordinates
            SWP_NOOWNERZORDER | //do not change the position even if its parent window is changed
            SWP_NOZORDER | //Window flag --> don't change the Z order
            SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
        ShowCursor(TRUE); //Appear the cursor in full screen or Game mode
        gContext_Scene2.win32Context.isFullscreen = 0;
    }

    /**************Shader removal code*******************/


    if (gContext_Scene2.win32Context.hdc)
    {
        ReleaseDC(gContext_Scene2.win32Context.hwnd, gContext_Scene2.win32Context.hdc);
        gContext_Scene2.win32Context.hdc = NULL;
    }

    //synchronisation function
    vkDeviceWaitIdle(gContext_Scene2.device);
    fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDeviceWaitIdle() is done\n");
    
    //DestroyFences
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        vkDestroyFence(gContext_Scene2.device, gContext_Scene2.fences[i], NULL);
    }
    
    if(gContext_Scene2.fences)
    {
        free(gContext_Scene2.fences);
        gContext_Scene2.fences = NULL;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyFence() is done\n");
    }
    
    //DestroySemaphore
    if(gContext_Scene2.renderCompleteSemaphore)
    {
        vkDestroySemaphore(gContext_Scene2.device, gContext_Scene2.renderCompleteSemaphore, NULL);
        gContext_Scene2.renderCompleteSemaphore = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroySemaphore() for gContext_Scene2.renderCompleteSemaphore is done\n");
    }
    
    if(gContext_Scene2.backBufferSemaphore)
    {
        vkDestroySemaphore(gContext_Scene2.device, gContext_Scene2.backBufferSemaphore, NULL);
        gContext_Scene2.backBufferSemaphore = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroySemaphore() for gContext_Scene2.backBufferSemaphore is done\n");
    }
    
    //vkframebuffer
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(gContext_Scene2.device, gContext_Scene2.framebuffers[i], NULL);
    }
    if(gContext_Scene2.framebuffers)
    {
        free(gContext_Scene2.framebuffers);
        gContext_Scene2.framebuffers = NULL;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> gContext_Scene2.framebuffers() is done\n");
    }
    
    //Destroy Descriptor Pool
    //When Descriptor pool is destroyed, descriptor set created  by that pool get destroyed implicitly
    if(gContext_Scene2.descriptorPool)
    {
        vkDestroyDescriptorPool(gContext_Scene2.device, gContext_Scene2.descriptorPool, NULL);        
        gContext_Scene2.descriptorPool = VK_NULL_HANDLE;
        gContext_Scene2.descriptorSet = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> gContext_Scene2.descriptorPool and DescriptorSet() is done\n");
    }
    
    //VkDescritporSetLayout
    if(gContext_Scene2.pipelineLayout)
    {
        vkDestroyPipelineLayout(gContext_Scene2.device, gContext_Scene2.pipelineLayout, NULL);
        gContext_Scene2.pipelineLayout = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> gContext_Scene2.pipelineLayout() is done\n");
    }
    
    //VkDescritporSetLayout
    if(gContext_Scene2.descriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(gContext_Scene2.device, gContext_Scene2.descriptorSetLayout, NULL);
        gContext_Scene2.descriptorSetLayout = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> gContext_Scene2.descriptorSetLayout() is done\n");
    }
    
    if(gContext_Scene2.pipeline)
    {
        vkDestroyPipeline(gContext_Scene2.device, gContext_Scene2.pipeline, NULL);
        gContext_Scene2.pipeline = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyPipeline() is done\n");
    }
    
    //Renderpass
    if(gContext_Scene2.renderPass)
    {
        vkDestroyRenderPass(gContext_Scene2.device, gContext_Scene2.renderPass, NULL);
        gContext_Scene2.renderPass = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyRenderPass() is done\n");
    }
    
    //destroy shader modules
    if(gContext_Scene2.fragmentShaderModule)
    {
        vkDestroyShaderModule(gContext_Scene2.device, gContext_Scene2.fragmentShaderModule, NULL);
        gContext_Scene2.fragmentShaderModule = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyShaderModule() is done\n");
    }
    
    if(gContext_Scene2.vertexShaderModule)
    {
        vkDestroyShaderModule(gContext_Scene2.device, gContext_Scene2.vertexShaderModule, NULL);
        gContext_Scene2.vertexShaderModule = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyShaderModule() is done\n");
    }
    
    //Destroy uniform buffer
    if(gContext_Scene2.uniformData.vkBuffer)
    {
        vkDestroyBuffer(gContext_Scene2.device, gContext_Scene2.uniformData.vkBuffer, NULL);
        gContext_Scene2.uniformData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyBuffer()  for gContext_Scene2.uniformData.vkBuffer is done\n");
    }
    
    if(gContext_Scene2.uniformData.vkDeviceMemory)
    {
        vkFreeMemory(gContext_Scene2.device, gContext_Scene2.uniformData.vkDeviceMemory, NULL);
        gContext_Scene2.uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkFreeMemory()  for gContext_Scene2.uniformData.vkDeviceMemory is done\n");
        
    }
    
    if(gContext_Scene2.textureSampler)
    {
        vkDestroySampler(gContext_Scene2.device, gContext_Scene2.textureSampler, NULL);
        gContext_Scene2.textureSampler = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroySampler() for gContext_Scene2.textureSampler is done\n");
    }
    
    if(gContext_Scene2.textureImageView)
    {
        vkDestroyImageView(gContext_Scene2.device, gContext_Scene2.textureImageView, NULL);
        gContext_Scene2.textureImageView = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroySampler() for gContext_Scene2.textureImageView is done\n");
    }
    
    if(gContext_Scene2.textureMemory)
    {
        vkFreeMemory(gContext_Scene2.device, gContext_Scene2.textureMemory, NULL);
        gContext_Scene2.textureMemory = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkFreeMemory() for gContext_Scene2.textureMemory is done\n");
    }
    
    if(gContext_Scene2.textureImage)
    {
        vkDestroyImage(gContext_Scene2.device, gContext_Scene2.textureImage, NULL);
        gContext_Scene2.textureImage = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyImage() for gContext_Scene2.textureImage is done\n");
    }

    
    //Vertex Tecoord buffer
    if(gContext_Scene2.texcoordVertexData.vkDeviceMemory)
    {
        vkFreeMemory(gContext_Scene2.device, gContext_Scene2.texcoordVertexData.vkDeviceMemory, NULL);
        gContext_Scene2.texcoordVertexData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkFreeMemory() is done\n");
    }
    
    if(gContext_Scene2.texcoordVertexData.vkBuffer)
    {
        vkDestroyBuffer(gContext_Scene2.device, gContext_Scene2.texcoordVertexData.vkBuffer, NULL);
        gContext_Scene2.texcoordVertexData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyBuffer() is done\n");
    }

    //Vertex position BUffer
    if(gContext_Scene2.positionVertexData.vkDeviceMemory)
    {
        vkFreeMemory(gContext_Scene2.device, gContext_Scene2.positionVertexData.vkDeviceMemory, NULL);
        gContext_Scene2.positionVertexData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkFreeMemory() is done\n");
    }
    
    if(gContext_Scene2.positionVertexData.vkBuffer)
    {
        vkDestroyBuffer(gContext_Scene2.device, gContext_Scene2.positionVertexData.vkBuffer, NULL);
        gContext_Scene2.positionVertexData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyBuffer() is done\n");
    }

    
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(gContext_Scene2.device, gContext_Scene2.commandPool, 1, &gContext_Scene2.commandBuffers[i]);
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkFreeCommandBuffers() is done\n");
    }
    
    if(gContext_Scene2.commandBuffers)
    {
        free(gContext_Scene2.commandBuffers);
        gContext_Scene2.commandBuffers = NULL;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> gContext_Scene2.commandBuffers is freed\n");
    }
    
    
    if(gContext_Scene2.commandPool)
    {
        vkDestroyCommandPool(gContext_Scene2.device, gContext_Scene2.commandPool, NULL);
        gContext_Scene2.commandPool = NULL;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyCommandPool() is done\n");
    }
    
    //destroy depth image view
    if(gContext_Scene2.depthImageView)
    {
        vkDestroyImageView(gContext_Scene2.device, gContext_Scene2.depthImageView, NULL);
        gContext_Scene2.depthImageView = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestoryImageView() is done for depth\n");
    }
    
    //destroy device memory for depth image
    if(gContext_Scene2.depthImageMemory)
    {
        vkFreeMemory(gContext_Scene2.device, gContext_Scene2.depthImageMemory, NULL);
        gContext_Scene2.depthImageMemory = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkFreeMemory() is done for depth\n");
    }
    
    if(gContext_Scene2.depthImage)
    {
        vkDestroyImage(gContext_Scene2.device, gContext_Scene2.depthImage, NULL);
        gContext_Scene2.depthImage = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestoryImage() is done for depth\n");
    }
    
    //destory image views
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        vkDestroyImageView(gContext_Scene2.device, gContext_Scene2.swapchainImageViews[i], NULL);
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestoryImageView() is done for color\n");
    }
    
    if(gContext_Scene2.swapchainImageViews)
    {
        free(gContext_Scene2.swapchainImageViews);
        gContext_Scene2.swapchainImageViews = NULL;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> gContext_Scene2.swapchainImageViews is freed\n");
    }
    
    //free swapchainImages
    // for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    // {
        // vkDestroyImage(gContext_Scene2.device, gContext_Scene2.swapchainImages[i], NULL);
        // fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyImage() is done\n");
    // }
    
    if(gContext_Scene2.swapchainImages)
    {
        free(gContext_Scene2.swapchainImages);
        gContext_Scene2.swapchainImages = NULL;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> gContext_Scene2.swapchainImages is freed\n");
    }
    
  
    
    //Destory Swapchain
    if(gContext_Scene2.swapchain)
    {
        vkDestroySwapchainKHR(gContext_Scene2.device, gContext_Scene2.swapchain, NULL);
        gContext_Scene2.swapchain = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkSwapchainCreateInfoKHR() is done\n");
    }

    //No need to Destroy/Uninitialize the DeviceQueue

    //Destroy vulkan device
    if(gContext_Scene2.device)
    {
        vkDestroyDevice(gContext_Scene2.device, NULL);
        gContext_Scene2.device = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestoryDevice() is done\n");
    }

    //No need to free slected physical device

    //Destroy gContext_Scene2.surface:This function is generic and not platform specific
    if(gContext_Scene2.surface)
    {
        vkDestroySurfaceKHR(gContext_Scene2.instance, gContext_Scene2.surface, NULL);
        gContext_Scene2.surface = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroySurfaceKHR() is done\n");
    }
    
    // ***** MODIFIED: make sure we actually destroy the debug report callback
    if (gContext_Scene2.debugReportCallback != VK_NULL_HANDLE)
    {
        // If the global function pointer wasn't set (due to earlier shadowing),
        // fetch it safely now so the callback can be destroyed.
        if (!gContext_Scene2.destroyDebugReportCallback)
        {
            gContext_Scene2.destroyDebugReportCallback =
                (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
                    gContext_Scene2.instance, "vkDestroyDebugReportCallbackEXT");
        }
        if (gContext_Scene2.destroyDebugReportCallback)
        {
            gContext_Scene2.destroyDebugReportCallback(gContext_Scene2.instance, 
                                                  gContext_Scene2.debugReportCallback,
                                                  NULL);
            gContext_Scene2.debugReportCallback = VK_NULL_HANDLE;
            gContext_Scene2.destroyDebugReportCallback = NULL;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "Uninitialize() --> WARNING: vkDestroyDebugReportCallbackEXT function pointer unavailable; callback not destroyed\n");
        }
    }
    
    
    //Destroy Vulkan Instance
    if(gContext_Scene2.instance)
    {
        vkDestroyInstance(gContext_Scene2.instance, NULL);
        gContext_Scene2.instance = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> vkDestroyInstance() is done\n");
    }
    
    if (gContext_Scene2.logFile)
    {
        fprintf(gContext_Scene2.logFile, "Uninitialize() --> Program terminated successfully\n");
        fclose(gContext_Scene2.logFile);
        gContext_Scene2.logFile = NULL;
    }
}

/***************Definition of Vulkan functions***********************/
/********************************************************************/

VkResult createVulkanInstance(void)
{
    //Function declarations
    VkResult fillInstanceExtensionNames(void);
    VkResult fillValidationLayerNames(void);
    VkResult createValidationCallbackFunction(void);

    //variable declarations
    VkResult vkResult = VK_SUCCESS;

    //code
    //Step1: Fill and initialize required extension names and count in global variable
    vkResult = FunctionTable_Scene2.fillInstanceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> fillInstanceExtensionNames() is failed\n");
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> fillInstanceExtensionNames() is succedded\n");
    }

    if(gContext_Scene2.enableValidation == TRUE)
    {
        //Fill validation Layer Names
        vkResult = FunctionTable_Scene2.fillValidationLayerNames();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> fillValidationLayerNames() is failed\n");
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> fillValidationLayerNames() is succedded\n");
        }
    }


    //Step1: Initialize struct VkApplicationInfo
    VkApplicationInfo vkApplicationInfo;
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //type safety, generic names
    vkApplicationInfo.pNext = NULL; //Linked List
    vkApplicationInfo.pApplicationName = gContext_Scene2.win32Context.appName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gContext_Scene2.win32Context.appName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;


    //Step3: Initialize struct VkInstanceCreateInfo by using information in Step1 and Step2
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = gContext_Scene2.instanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = gContext_Scene2.instanceExtensionNames;
    
    if(gContext_Scene2.enableValidation == TRUE)
    {
        vkInstanceCreateInfo.enabledLayerCount = gContext_Scene2.validationLayerCount;
        vkInstanceCreateInfo.ppEnabledLayerNames = gContext_Scene2.validationLayerNames;
    }
    else
    {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
    }

    /*
	// Provided by VK_VERSION_1_0
		VkResult vkCreateInstance(
		const VkInstanceCreateInfo* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkInstance* pInstance);

		• pCreateInfo is a pointer to a VkInstanceCreateInfo structure controlling creation of the instance.
		• pAllocator controls host memory allocation as described in the Memory Allocation chapter.
		• pInstance points a VkInstance handle in which the resulting instance is returned.
	 */
    //Step4: Call VkCreateInstance() to get gContext_Scene2.instance in a global variable and do error checking
    vkResult = vkCreateInstance(&vkInstanceCreateInfo,
                                 NULL,  //no custom Memory allocater
                                 &gContext_Scene2.instance);
    if(vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to incompatible driver %d\n", vkResult);
        return vkResult;
    }
    else if(vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to required extension not present %d\n", vkResult);
        return vkResult;
    }
    else if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to unknown reason %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance succedded\n");
    }
    
    //do for validation callbacks
    if(gContext_Scene2.enableValidation == TRUE)
    {
        vkResult = FunctionTable_Scene2.createValidationCallbackFunction();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> createValidationCallbackFunction() is failed\n");
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "createVulkanInstance() --> createValidationCallbackFunction() is succedded\n");
        }
    }


   //Step5: Destroy

    return vkResult;
}


VkResult fillInstanceExtensionNames(void)
{
    //variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step1: Find how many Instacne Extension are supported by the vulkan driver of this version and keep the count in local variable
    uint32_t instanceExtensionCount = 0;

    /*
    // Provided by VK_VERSION_1_0
        VkResult vkEnumerateInstanceExtensionProperties(
        const char* pLayerName,
        uint32_t* pPropertyCount,
        VkExtensionProperties* pProperties);
    
        • pLayerName is either NULL or a pointer to a null-terminated UTF-8 string naming the layer to
          retrieve extensions from.
        • pPropertyCount is a pointer to an integer related to the number of extension properties available
          or queried, as described below.
        • pProperties is either NULL or a pointer to an array of VkExtensionProperties structures
    */

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, //Which layer's extenion is needed: Mention extension name: For all driver's extension use NULL
                                                      &instanceExtensionCount,
                                                      NULL); // Instance Extensions Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is failed\n");
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is succedded\n");
    }



    //Step2: Allocate and fill VkExtensionProperties corresponding to above count
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * instanceExtensionCount);
    //Should be error checking for malloc: assert() can also be used

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, //Which layer's extenion is needed: Mention extension name: For all driver's extension use NULL
                                                      &instanceExtensionCount,
                                                      vkExtensionProperties_array); // Instance Extensions Properties array: As we have count, so it is value
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is failed\n");
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is succedded\n");
    }



    //Step3: Fill and Display a local string array of extension names obtained from vkExtensionProperties
    char** instanceExtensionNames_array = NULL;

    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> Vulkan Instance Extension names = %s\n", instanceExtensionNames_array[i]);
    }



   //Step4: As not required henceforth, free the vkExtensionProperties_array;
   free(vkExtensionProperties_array);
   vkExtensionProperties_array = NULL;



   // Step5: Find whether below extension names contains our required two extensions
   //VK_KHR_SURFACE_EXTENSION_NAME
   //VK_KHR_WIN32_SURFACE_EXTENSION_NAME
   VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
   VkBool32 vulkanWin32SurfaceExtensionFound = VK_FALSE;
   VkBool32 vulkanDebugReportExtensionFound = VK_FALSE;

    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            gContext_Scene2.instanceExtensionNames[gContext_Scene2.instanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
        
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanWin32SurfaceExtensionFound = VK_TRUE;
            gContext_Scene2.instanceExtensionNames[gContext_Scene2.instanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
        
        if(strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
        {
            vulkanDebugReportExtensionFound = VK_TRUE;
            if(gContext_Scene2.enableValidation == TRUE)
            {
                gContext_Scene2.instanceExtensionNames[gContext_Scene2.instanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }
            else
            {
                //array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }
        }
    }



    //Step 6:
    //As not required henceforth, free the local string array
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        free(instanceExtensionNames_array[i]);
    }
    free(instanceExtensionNames_array);



    //Step7:Print whether our vulkan driver supports our required extension names or not
    if(vulkanSurfaceExtensionFound == VK_FALSE)
    {
        // return hardcoded failure
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME found\n");
    }

    if(vulkanWin32SurfaceExtensionFound == VK_FALSE)
    {
        // return hardcoded failure
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME found\n");
    }
    
    if(vulkanDebugReportExtensionFound == VK_FALSE)
    {
        if(gContext_Scene2.enableValidation == TRUE)
        {
            // return hardcoded failure
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is ON But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is OFF But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
        }
    }
    else
    {
        if(gContext_Scene2.enableValidation == TRUE)
        {
            fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is ON and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is OFF and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
        }
    }


    //Step8: Print only Enabled Extension Names 
    for(uint32_t i = 0; i < gContext_Scene2.instanceExtensionCount; i++)
    {
         fprintf(gContext_Scene2.logFile, "fillInstanceExtensionNames() --> Enabled vulkan Instance extension Names = %s\n", gContext_Scene2.instanceExtensionNames[i]);
    }

    return vkResult;
}


VkResult fillValidationLayerNames(void)
{
    //code
    //variables
    VkResult vkResult = VK_SUCCESS;
    uint32_t validationLayerCount = 0;
    
    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  NULL); // Instance Validation Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is succedded\n");
    }
    
    VkLayerProperties* vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*) malloc (sizeof(VkLayerProperties) * validationLayerCount);
    //Should be error checking for malloc: assert() can also be used

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  vkLayerProperties_array); // Instance Validation Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is succedded\n");
    }
    
    char** validationLayerNames_array = NULL;
    validationLayerNames_array = (char**) malloc(sizeof(char*) * validationLayerCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        validationLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
        //Should be error checking for malloc: assert() can also be used
        memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
        fprintf(gContext_Scene2.logFile, "fillValidationLayerNames() --> Vulkan Validation Layer names = %s\n", vkLayerProperties_array[i].layerName);
    }
    
    if(vkLayerProperties_array) 
        free(vkLayerProperties_array);
    vkLayerProperties_array = NULL;

    // Step5: Find whether below layer names contains our required two extensions
    //VK_KHR_SURFACE_EXTENSION_NAME
    VkBool32 vulkanValidationLayerFound = VK_FALSE;
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        if(strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
        {
            vulkanValidationLayerFound = VK_TRUE;
            gContext_Scene2.validationLayerNames[gContext_Scene2.validationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }
    
     //As not required henceforth, free the local string array
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        free(validationLayerNames_array[i]);
    }
    free(validationLayerNames_array);
    
    if(gContext_Scene2.enableValidation == TRUE)
    {
        //Step7:Print whether our vulkan driver supports our required extension names or not
        if(vulkanValidationLayerFound == VK_FALSE)
        {
            // return hardcoded failure
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gContext_Scene2.logFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation not supported\n");
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation is supported\n");
        }
    }
    
    //Step8: Print only Enabled validation layer Names
    for(uint32_t i = 0; i < gContext_Scene2.validationLayerCount; i++)
    {
         fprintf(gContext_Scene2.logFile, "fillValidationLayerNames() --> Enabled vulkan validation layer Names = %s\n", gContext_Scene2.validationLayerNames[i]);
    }
    
    return (vkResult);    
}


VkResult createValidationCallbackFunction(void)
{
    //code
    //function declarations
    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, 
                                                       VkDebugReportObjectTypeEXT,
                                                       uint64_t,
                                                       size_t,
                                                       int32_t,
                                                       const char*,
                                                       const char*,
                                                       void*);
    
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    gContext_Scene2.createDebugReportCallback = NULL;
    
    //Get the required function pointers
    gContext_Scene2.createDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(gContext_Scene2.instance, "vkCreateDebugReportCallbackEXT");
    if(gContext_Scene2.createDebugReportCallback == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gContext_Scene2.logFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is failed to get function pointer for vkCreateDebugReportCallbackEXT \n");
        return (vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is succedded to get function pointer for vkCreateDebugReportCallbackEXT \n");
    }
    
    // ***** MODIFIED: do NOT redeclare a local variable with the same name.
    // Assign the function pointer to the GLOBAL gContext_Scene2.destroyDebugReportCallback.
    gContext_Scene2.destroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(gContext_Scene2.instance, "vkDestroyDebugReportCallbackEXT");
    if(gContext_Scene2.destroyDebugReportCallback == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gContext_Scene2.logFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is failed to get function pointer for vkDestroyDebugReportCallbackEXT \n");
        return (vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is succedded to get function pointer for vkDestroyDebugReportCallbackEXT \n");
    }
    
    
    //Get the vulkanDebugReportCallback object
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
    memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;
    
    vkResult = gContext_Scene2.createDebugReportCallback(gContext_Scene2.instance,
                                                    &vkDebugReportCallbackCreateInfoEXT,
                                                    NULL,
                                                    &gContext_Scene2.debugReportCallback);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createValidationCallbackFunction() --> gContext_Scene2.createDebugReportCallback() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createValidationCallbackFunction() --> gContext_Scene2.createDebugReportCallback() is succedded\n");
    }
    
    return (vkResult);
}



//Create Vulkan Presentation Surface
VkResult getSupportedSurface(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step2
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
    memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    //one way
    // vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetModuleHandle(NULL);
    //another way for 64bit
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(gContext_Scene2.win32Context.hwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = gContext_Scene2.win32Context.hwnd;

    //Step3:
    vkResult = vkCreateWin32SurfaceKHR(gContext_Scene2.instance,
                                       &vkWin32SurfaceCreateInfoKHR,
                                       NULL, //Memory mamnagement function is default
                                       &gContext_Scene2.surface);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is succedded\n");
    }

    return vkResult;
}


VkResult getPhysicalDevice(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;
    

    //code
    vkResult = vkEnumeratePhysicalDevices(gContext_Scene2.instance,
                                          &gContext_Scene2.physicalDeviceCount,
                                          NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(gContext_Scene2.physicalDeviceCount == 0)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() resulted in zero devices\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is succedded\n");
    }

    gContext_Scene2.physicalDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gContext_Scene2.physicalDeviceCount);
    //error checking to be done
    
    vkResult = vkEnumeratePhysicalDevices(gContext_Scene2.instance, 
                                          &gContext_Scene2.physicalDeviceCount,
                                          gContext_Scene2.physicalDevices);
     if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is succedded\n");
    }

    VkBool32 bFound = VK_FALSE;
    for(uint32_t i = 0; i < gContext_Scene2.physicalDeviceCount; i++)
    {
        uint32_t qCount = UINT32_MAX;
        
        //If physical device is present then it must support at least 1 queue family
        vkGetPhysicalDeviceQueueFamilyProperties(gContext_Scene2.physicalDevices[i], 
                                               &qCount, 
                                               NULL);
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * qCount);
        //error checking to be done
        
        vkGetPhysicalDeviceQueueFamilyProperties(gContext_Scene2.physicalDevices[i], 
                                               &qCount, 
                                               vkQueueFamilyProperties_array);
        
        VkBool32* isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * qCount);
        //error checking to be done
        
        for(uint32_t j = 0; j < qCount; j++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(gContext_Scene2.physicalDevices[i], 
                                                 j,
                                                 gContext_Scene2.surface,
                                                 &isQueueSurfaceSupported_array[j]);
        }
        
        for(uint32_t j = 0; j < qCount; j++)
        {
            if(vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if(isQueueSurfaceSupported_array[j] == VK_TRUE)
                {
                    gContext_Scene2.physicalDevice = gContext_Scene2.physicalDevices[i];
                    gContext_Scene2.graphicsQueueFamilyIndex = j;
                    bFound = VK_TRUE;
                    break;
                }
            }
        }
        
        if(isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
            fprintf(gContext_Scene2.logFile, "getPhysicalDevice() --> isQueueSurfaceSupported_array succedded to free\n");
        }
        
        if(vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            fprintf(gContext_Scene2.logFile, "getPhysicalDevice() --> vkQueueFamilyProperties_array succedded to free\n");
        }
        
        if(bFound == VK_TRUE)
        {
            break;
        }
    }
    
    if(bFound == VK_TRUE)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() -->is succedded to select the required device with graphics enabled\n");
    }
    else
    {
        if(gContext_Scene2.physicalDevices)
        {
            free(gContext_Scene2.physicalDevices);
            gContext_Scene2.physicalDevices = NULL;
            fprintf(gContext_Scene2.logFile, "getPhysicalDevice() --> gContext_Scene2.physicalDevices succedded to free\n");
        }
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() -->is failed to select the required device with graphics enabled\n");
    }
    
    
    memset((void*)&gContext_Scene2.physicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
    vkGetPhysicalDeviceMemoryProperties(gContext_Scene2.physicalDevice, 
                                        &gContext_Scene2.physicalDeviceMemoryProperties);
                                        
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    
    vkGetPhysicalDeviceFeatures(gContext_Scene2.physicalDevice, 
                                &vkPhysicalDeviceFeatures);
                                
    if(vkPhysicalDeviceFeatures.tessellationShader)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() -->selected device supports tessellationShader\n");
    }
    else
    {
          fprintf(gContext_Scene2.logFile, "getPhysicalDevice() -->selected device not supports tessellationShader\n");
    }
    
    if(vkPhysicalDeviceFeatures.geometryShader)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() -->selected device supports geometryShader\n");
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevice() -->selected device not supports geometryShader\n");
    }
    
    return vkResult;
}


VkResult printVkInfo(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;
    
    //code
    fprintf(gContext_Scene2.logFile, "*******************VULKAN INFORMATION*********************\n");
    for(uint32_t i = 0; i < gContext_Scene2.physicalDeviceCount; i++)    
    {
        fprintf(gContext_Scene2.logFile, "Infomration of Device = %d\n", i);
        
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        
        vkGetPhysicalDeviceProperties(gContext_Scene2.physicalDevices[i], &vkPhysicalDeviceProperties);
        
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);;
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);;
        //API Version
        fprintf(gContext_Scene2.logFile, "apiVersion = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);
        
        //Device Name
        fprintf(gContext_Scene2.logFile, "DeviceName = %s\n", vkPhysicalDeviceProperties.deviceName);
        
        //DeviceType
        switch(vkPhysicalDeviceProperties.deviceType)
        {
            case(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
                fprintf(gContext_Scene2.logFile, "DeviceType = Integrated GPU(iGPU)\n");
                break;
            
            case(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
                fprintf(gContext_Scene2.logFile, "DeviceType = Discrete GPU(dGPU)\n");
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU):
                fprintf(gContext_Scene2.logFile, "DeviceType = Virtual GPU(vGPU)\n");
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_CPU):
                fprintf(gContext_Scene2.logFile, "DeviceType = CPU\n");
                break;    
                
            case(VK_PHYSICAL_DEVICE_TYPE_OTHER):
                fprintf(gContext_Scene2.logFile, "DeviceType = Other\n");
                break; 
                
            default: 
                fprintf(gContext_Scene2.logFile, "DeviceType = UNKNOWN\n");                
        }
        
        //Vendor Id
        fprintf(gContext_Scene2.logFile, "VendorId = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);
        
        //DeviceId
        fprintf(gContext_Scene2.logFile, "DeviceId = 0x%04x\n\n", vkPhysicalDeviceProperties.deviceID);
   }
   
   fprintf(gContext_Scene2.logFile, "****************END OF VULKAN INFORMATION********************\n");
   
    //Freephysical device array
    if(gContext_Scene2.physicalDevices)
    {
        free(gContext_Scene2.physicalDevices);
        gContext_Scene2.physicalDevices = NULL;
        fprintf(gContext_Scene2.logFile, "printVkInfo() --> gContext_Scene2.physicalDevices succedded to free\n");
    }
    
    return vkResult;
}


VkResult fillDeviceExtensionNames(void)
{
    //variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step1: Find how many Device Extension are supported by the vulkan driver of this version and keep the count in local variable
    uint32_t deviceExtensionCount = 0;

    vkResult = vkEnumerateDeviceExtensionProperties(gContext_Scene2.physicalDevice,
                                                    NULL,  //Layer name: All layers
                                                    &deviceExtensionCount,
                                                    NULL); // Device Extensions Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is succedded\n");
        fprintf(gContext_Scene2.logFile, "deviceExtensionCount is %u\n", deviceExtensionCount);
    }


    //Step2: Allocate and fill VkExtensionProperties corresponding to above count
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * deviceExtensionCount);
    //Should be error checking for malloc: assert() can also be used

    vkResult = vkEnumerateDeviceExtensionProperties(gContext_Scene2.physicalDevice,
                                                    NULL, //Which layer's extenion is needed: Mention extension name: For all driver's extension use NULL
                                                    &deviceExtensionCount,
                                                    vkExtensionProperties_array); // Instance Extensions Properties array: As we have count, so it is value
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is succedded\n");
    }



    //Step3: Fill and Display a local string array of extension names obtained from vkExtensionProperties
    char** deviceExtensionNames_array = NULL;

    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gContext_Scene2.logFile, "fillDeviceExtensionNames() --> Vulkan Device Extension names = %s\n", deviceExtensionNames_array[i]);
    }



   //Step4: As not required henceforth, free the vkExtensionProperties_array;
   free(vkExtensionProperties_array);
   vkExtensionProperties_array = NULL;



   // Step5: Find whether below extension names contains our required two extensions
   //VK_KHR_SWAPCHAIN_EXTENSION_NAME
   VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;
  
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        if(strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapChainExtensionFound = VK_TRUE;
            gContext_Scene2.deviceExtensionNames[gContext_Scene2.deviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }



    //Step 6:
    //As not required henceforth, free the local string array
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        free(deviceExtensionNames_array[i]);
    }
    free(deviceExtensionNames_array);



    //Step7:Print whether our vulkan driver supports our required extension names or not
    if(vulkanSwapChainExtensionFound == VK_FALSE)
    {
        // return hardcoded failure
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gContext_Scene2.logFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME found\n");
    }


    //Step8: Print only Enabled Extension Names
    for(uint32_t i = 0; i < gContext_Scene2.deviceExtensionCount; i++)
    {
         fprintf(gContext_Scene2.logFile, "fillDeviceExtensionNames() --> Enabled vulkan Device extension Names = %s\n", gContext_Scene2.deviceExtensionNames[i]);
    }

    return vkResult;
}


VkResult createVulkanDevice(void)
{  
    //Function declarations
    VkResult fillDeviceExtensionNames(void);
    
    //variable declaration
    VkResult vkResult = VK_SUCCESS;
    
    //Fill Device Extensions
    vkResult = FunctionTable_Scene2.fillDeviceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVulkanDevice() --> fillDeviceExtensionNames() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVulkanDevice() --> fillDeviceExtensionNames() is succedded\n");
    }
    
    /////Newly added code//////
    
    float QueuePriorities[] = {1.0};
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = gContext_Scene2.graphicsQueueFamilyIndex;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = QueuePriorities;
    
    //Initialize VkDeviceCreateinfo structure
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = gContext_Scene2.deviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = gContext_Scene2.deviceExtensionNames;
    vkDeviceCreateInfo.enabledLayerCount = 0;  // Deprecated
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;  // Deprecated
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;
        
    vkResult = vkCreateDevice(gContext_Scene2.physicalDevice,
                              &vkDeviceCreateInfo,
                              NULL,
                              &gContext_Scene2.device);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVulkanDevice() --> vkCreateDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVulkanDevice() --> vkCreateDevice() is succedded\n");
    }                     
    
    return vkResult;
}


void getDeviceQueue(void)
{
    //code
    vkGetDeviceQueue(gContext_Scene2.device, 
                     gContext_Scene2.graphicsQueueFamilyIndex,
                     0, //0th Queue index in that family queue
                     &gContext_Scene2.queue);
    if(gContext_Scene2.queue == VK_NULL_HANDLE) //rarest possibility
    {
        fprintf(gContext_Scene2.logFile, "getDeviceQueue() --> vkGetDeviceQueue() returned NULL for gContext_Scene2.queue\n");
        return;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getDeviceQueue() --> vkGetDeviceQueue() is succedded\n");
    }
}


VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    //variable declarations
    VkResult vkResult = VK_SUCCESS;   
    uint32_t formatCount = 0;
    
    //code
    //get the count of supported SurfaceColorFormats
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gContext_Scene2.physicalDevice,
                                                    gContext_Scene2.surface,
                                                    &formatCount,
                                                    NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(formatCount == 0)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed as formatCount is zero:: %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is succedded\n");
    }
    
    fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> total formatCount are:: %d\n", formatCount);
    
    VkSurfaceFormatKHR* vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*) malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    //Malloc error checking
    
    //Fillig the array
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gContext_Scene2.physicalDevice,
                                                    gContext_Scene2.surface,
                                                    &formatCount,
                                                    vkSurfaceFormatKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is succedded\n");
    }
    
    if(formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED) //bydefault it is not there
    {
        gContext_Scene2.colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
        fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> gContext_Scene2.colorFormat is VK_FORMAT_B8G8R8A8_UNORM\n");
    }
    else
    {
        gContext_Scene2.colorFormat = vkSurfaceFormatKHR_array[0].format;
        fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> gContext_Scene2.colorFormat is %d\n", gContext_Scene2.colorFormat);
    }
    
    //Decide the ColorSpace
    gContext_Scene2.colorSpace = vkSurfaceFormatKHR_array[0].colorSpace;
    
    if(vkSurfaceFormatKHR_array)
    {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        fprintf(gContext_Scene2.logFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkSurfaceFormatKHR_array is freed\n");
    }
    
    return vkResult;
}


VkResult getPhysicalDevicePresentMode(void)
{
    //variable declarations
    VkResult vkResult = VK_SUCCESS;   
    
    uint32_t presentModeCount = 0;
    
    //code
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gContext_Scene2.physicalDevice,
                                                         gContext_Scene2.surface,
                                                         &presentModeCount,
                                                         NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(presentModeCount == 0)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed as formatCount is zero:: %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
    }   

    fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> total presentModeCount are:: %d\n", presentModeCount);

    VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*) malloc(presentModeCount * sizeof(VkPresentModeKHR));
    //Malloc error checking
    
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gContext_Scene2.physicalDevice,
                                                         gContext_Scene2.surface,
                                                         &presentModeCount,
                                                         vkPresentModeKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
    }
    
    //Decide Presentation mode
    for(uint32_t i = 0; i < presentModeCount; i++)
    {
        if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            gContext_Scene2.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> gContext_Scene2.presentMode is VK_PRESENT_MODE_MAILBOX_KHR\n");
            break;
        }
    }
    
    if(gContext_Scene2.presentMode != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        gContext_Scene2.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> gContext_Scene2.presentMode is VK_PRESENT_MODE_FIFO_KHR\n");
    }
  
    
    if(vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        fprintf(gContext_Scene2.logFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR_array is freed\n");
    }
    
    return vkResult;
  
}


VkResult createSwapchain(VkBool32 vsync)  // vertical sync
{
    //fucntion Declarations
    VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult getPhysicalDevicePresentMode(void);
    
    //variables
    VkResult vkResult = VK_SUCCESS;
        
    //code
    //Color Format and ColorSpace
    vkResult = FunctionTable_Scene2.getPhysicalDeviceSurfaceFormatAndColorSpace();
    /*Main Points
        vkGetPhysicalDeviceSurfaceFormatsKHR()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> getPhysicalDeviceSurfaceFormatAndColorSpace() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> getPhysicalDeviceSurfaceFormatAndColorSpace() is succedded\n");
    }
   
   
    //Step 2:
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
    
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gContext_Scene2.physicalDevice,
                                                         gContext_Scene2.surface,
                                                         &vkSurfaceCapabilitiesKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() is succedded\n");
    }
   
    //Step3: Find out desired swapchain image count
    uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
    uint32_t desiredNumbeOfSwapchainImages = 0;
    
    
    if(vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
    {
        desiredNumbeOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    }
    else
    {
        desiredNumbeOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
    }
    
    //Step4: Choose size of swapchain image
    memset((void*)&gContext_Scene2.swapchainExtent, 0, sizeof(VkExtent2D));
    if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        gContext_Scene2.swapchainExtent.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        gContext_Scene2.swapchainExtent.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
        
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> Swapchain image width X height = %d X %d \n", gContext_Scene2.swapchainExtent.width, gContext_Scene2.swapchainExtent.height);
    }
    else
    {
        // if surface is already defined then swapchain image size must match with it
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0 , sizeof(VkExtent2D));
        vkExtent2D.width = (uint32_t)gContext_Scene2.windowWidth;
        vkExtent2D.height = (uint32_t)gContext_Scene2.windowHeight;
        
        gContext_Scene2.swapchainExtent.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        gContext_Scene2.swapchainExtent.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
        
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> Swapchain image width X height = %d X %d \n", gContext_Scene2.swapchainExtent.width, gContext_Scene2.swapchainExtent.height);
    }
    
    //step5: Set SwapchainImageUsageFlag
    VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT/*Texture, Compute, FBO*/; 
    
    //enum
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
    if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
    }
   
    
    //Step 7: Presentation mode
    vkResult = FunctionTable_Scene2.getPhysicalDevicePresentMode();
    /*Main Points
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> getPhysicalDevicePresentMode() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> getPhysicalDevicePresentMode() is succedded\n");
    }
    
    //Step 8: Initialie vkCreateSwapchinCreateInfoStructure
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset((void*)&vkSwapchainCreateInfoKHR, 0 , sizeof(VkSwapchainCreateInfoKHR));
    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = gContext_Scene2.surface;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumbeOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = gContext_Scene2.colorFormat;
    vkSwapchainCreateInfoKHR.imageColorSpace = gContext_Scene2.colorSpace;
    vkSwapchainCreateInfoKHR.imageExtent.width = gContext_Scene2.swapchainExtent.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = gContext_Scene2.swapchainExtent.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = gContext_Scene2.presentMode;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;
    
    //Step9:
    vkResult = vkCreateSwapchainKHR(gContext_Scene2.device,
                                    &vkSwapchainCreateInfoKHR,
                                    NULL,
                                    &gContext_Scene2.swapchain);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> vkCreateSwapchainKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createSwapchain() --> vkCreateSwapchainKHR() is succedded\n");
    }
    
    return vkResult;
}


VkResult createImagesAndImageViews(void)
{
    //fuction  declarations
    VkResult GetSupportedDepthFormat(void);
    
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //step1: Get desired SwapchainImage count
    vkResult = vkGetSwapchainImagesKHR(gContext_Scene2.device, 
                                       gContext_Scene2.swapchain,
                                       &gContext_Scene2.swapchainImageCount,
                                       NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> 1st call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(0 == gContext_Scene2.swapchainImageCount)
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> 1st call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> gives swapchainImagecount = %d\n", gContext_Scene2.swapchainImageCount);
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkGetSwapchainImagesKHR() is succedded\n");
    }
    
    
    //step2: Allocate the swapchain Image array
    gContext_Scene2.swapchainImages = (VkImage*)malloc(sizeof(VkImage) * gContext_Scene2.swapchainImageCount);
    //malloc check to be done

    //step3: fill this array by swapchain imagesize
    vkResult = vkGetSwapchainImagesKHR(gContext_Scene2.device, 
                                       gContext_Scene2.swapchain,
                                       &gContext_Scene2.swapchainImageCount,
                                       gContext_Scene2.swapchainImages);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> 2nd call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> 2nd call to vkGetSwapchainImagesKHR() is succedded\n");
    }
    
    //step4: allocate array of swapchainImageViews   
    gContext_Scene2.swapchainImageViews = (VkImageView*)malloc(sizeof(VkImageView) * gContext_Scene2.swapchainImageCount);
    //malloc check to be done
    
    //step5: Initialize vkImageViewCreateInfo structure
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = gContext_Scene2.colorFormat;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    
    //Step6: Fill Imageview Array by using above struct
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        vkImageViewCreateInfo.image = gContext_Scene2.swapchainImages[i];
        
        vkResult = vkCreateImageView(gContext_Scene2.device,
                                 &vkImageViewCreateInfo,
                                 NULL,
                                 &gContext_Scene2.swapchainImageViews[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkCreateImageViews() is failed for iteration %d and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkCreateImageViews() is succedded for iteration for %d\n", i);
        }
    }
    
    //for depth image
    vkResult = FunctionTable_Scene2.GetSupportedDepthFormat();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() is succedded\n");
    }
    
    //for depth image Initialize VkImageCreateInfo
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = gContext_Scene2.depthFormat;
    vkImageCreateInfo.extent.width = gContext_Scene2.windowWidth;
    vkImageCreateInfo.extent.height = gContext_Scene2.windowHeight;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vkResult = vkCreateImage(gContext_Scene2.device, &vkImageCreateInfo, NULL, &gContext_Scene2.depthImage);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkCreateImage() is failed error code is %d\n",vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkCreateImage() is succedded\n");
    }
    
    //Memory requirement for depth image
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetImageMemoryRequirements(gContext_Scene2.device, gContext_Scene2.depthImage, &vkMemoryRequirements);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < gContext_Scene2.physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(gContext_Scene2.physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gContext_Scene2.device, &vkMemoryAllocateInfo, NULL, &gContext_Scene2.depthImageMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindImageMemory(gContext_Scene2.device, gContext_Scene2.depthImage, gContext_Scene2.depthImageMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkBindBufferMemory() is succedded\n");
    }
    
    
    //create image view for above depth image
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = gContext_Scene2.depthFormat;
    // ***** MODIFIED: include STENCIL bit only if the chosen format has stencil
    VkImageAspectFlags depthAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (gContext_Scene2.depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        gContext_Scene2.depthFormat == VK_FORMAT_D24_UNORM_S8_UINT  ||
        gContext_Scene2.depthFormat == VK_FORMAT_D16_UNORM_S8_UINT)
    {
        depthAspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    vkImageViewCreateInfo.subresourceRange.aspectMask = depthAspect;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = gContext_Scene2.depthImage;
    
    vkResult = vkCreateImageView(gContext_Scene2.device,
                                 &vkImageViewCreateInfo,
                                 NULL,
                                 &gContext_Scene2.depthImageView);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkCreateImageView() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createImagesAndImageViews() --> vkCreateImageView() is succedded\n");
    }
    
    
    return vkResult;
}


VkResult GetSupportedDepthFormat(void)
{
    //code
    //variables
    VkResult vkResult = VK_SUCCESS;
    VkFormat vkFormat_depth_array[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, 
                                       VK_FORMAT_D32_SFLOAT,
                                       VK_FORMAT_D24_UNORM_S8_UINT,
                                       VK_FORMAT_D16_UNORM_S8_UINT,
                                       VK_FORMAT_D16_UNORM };
    
    for(uint32_t i = 0; i < (sizeof(vkFormat_depth_array)/sizeof(vkFormat_depth_array[0])); i++)
    {
        VkFormatProperties vkFormatProperties;
        memset((void*)&vkFormatProperties, 0, sizeof(VkFormatProperties));
        
        vkGetPhysicalDeviceFormatProperties(gContext_Scene2.physicalDevice, vkFormat_depth_array[i], &vkFormatProperties);
        
        if(vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
           gContext_Scene2.depthFormat = vkFormat_depth_array[i];
           vkResult = VK_SUCCESS;
           break;
        }
    }
    
   return vkResult;
}


VkResult createCommandPool(void)
{
    //variables
   VkResult vkResult = VK_SUCCESS;
    
   //code
   VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
   memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
   
   vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   vkCommandPoolCreateInfo.pNext = NULL;
   vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //such command buffers will be resetted and resatrted, and these command buffers are long lived
   vkCommandPoolCreateInfo.queueFamilyIndex = gContext_Scene2.graphicsQueueFamilyIndex;
   
   vkResult = vkCreateCommandPool(gContext_Scene2.device, 
                                  &vkCommandPoolCreateInfo,
                                  NULL,
                                  &gContext_Scene2.commandPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createCommandPool() --> vkCreateCommandPool() is failed and error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createCommandPool() --> vkCreateCommandPool() is succedded \n");
    }

   return vkResult;   
}


VkResult createCommandBuffers(void)
{
   //variables
   VkResult vkResult = VK_SUCCESS;
   
   //code
   //vkCommandBuffer allocate info structure initliazation
   VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo; 
   memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
   
   vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   vkCommandBufferAllocateInfo.pNext = NULL;
   vkCommandBufferAllocateInfo.commandPool = gContext_Scene2.commandPool;
   vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   vkCommandBufferAllocateInfo.commandBufferCount = 1;
   
   gContext_Scene2.commandBuffers = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * gContext_Scene2.swapchainImageCount);
   //malloc check to be done
   
   for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(gContext_Scene2.device, &vkCommandBufferAllocateInfo, &gContext_Scene2.commandBuffers[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is failed for %d iteration and error code is %d\n",i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is succedded for iteration %d\n", i);
        }
    }
   
   return vkResult;   
}

VkResult createVertexBuffer(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS; 
    
    // QUAD (two triangles)
    // position (CCW winding; centered at origin; Z = 0.0f)
    float quadPosition[] =
    {
        // triangle one
         1.0f,  1.0f,  0.0f,  // top-right
        -1.0f,  1.0f,  0.0f,  // top-left
         1.0f, -1.0f,  0.0f,  // bottom-right
        
        // triangle two
         1.0f, -1.0f,  0.0f,  // bottom-right
        -1.0f,  1.0f,  0.0f,  // top-left
        -1.0f, -1.0f,  0.0f,  // bottom-left
    };

    // texcoords (matching the "front" face mapping you used on the cube)
    float quadTexcoords[] =
    {
        // triangle one
        1.0f, 1.0f,   // top-right
        0.0f, 1.0f,   // top-left
        1.0f, 0.0f,   // bottom-right

        // triangle two
        1.0f, 0.0f,   // bottom-right
        0.0f, 1.0f,   // top-left
        0.0f, 0.0f,   // bottom-left
    };
    
    //VERTEX POSITION BUFFER 
    //#4 memset the global strucure variable
    memset((void*)&gContext_Scene2.positionVertexData, 0, sizeof(gContext_Scene2.positionVertexData));
    
    //#5 VkBufferCreateInfo structure filling
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    //valid flags are used in scatterred/sparse buffer
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(quadPosition);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    ///#6
    vkResult = vkCreateBuffer(gContext_Scene2.device, 
                              &vkBufferCreateInfo,
                              NULL,
                              &gContext_Scene2.positionVertexData.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(gContext_Scene2.device, gContext_Scene2.positionVertexData.vkBuffer, &vkMemoryRequirements);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < (int)gContext_Scene2.physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(gContext_Scene2.physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gContext_Scene2.device, &vkMemoryAllocateInfo, NULL, &gContext_Scene2.positionVertexData.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(gContext_Scene2.device, gContext_Scene2.positionVertexData.vkBuffer, gContext_Scene2.positionVertexData.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded\n");
    }
    
    //#11
    void* data = NULL;
    vkResult = vkMapMemory(gContext_Scene2.device, gContext_Scene2.positionVertexData.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkMapMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkMapMemory() is succedded\n");
    }
   
    //#12
    memcpy(data, quadPosition, sizeof(quadPosition));
    
    vkUnmapMemory(gContext_Scene2.device, gContext_Scene2.positionVertexData.vkDeviceMemory);
   
   
   //VERTEX TEXCOORD BUFFER 
    //#4 memset the global strucure variable
    memset((void*)&gContext_Scene2.texcoordVertexData, 0, sizeof(gContext_Scene2.texcoordVertexData));
    
    //#5 VkBufferCreateInfo structure filling
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    //valid flags are used in scatterred/sparse buffer
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(quadTexcoords);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    ///#6
    vkResult = vkCreateBuffer(gContext_Scene2.device, 
                              &vkBufferCreateInfo,
                              NULL,
                              &gContext_Scene2.texcoordVertexData.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord buffer \n");
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(gContext_Scene2.device, gContext_Scene2.texcoordVertexData.vkBuffer, &vkMemoryRequirements);
    
    //8" Allocate
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < (int)gContext_Scene2.physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(gContext_Scene2.physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gContext_Scene2.device, &vkMemoryAllocateInfo, NULL, &gContext_Scene2.texcoordVertexData.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord buffer\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(gContext_Scene2.device, gContext_Scene2.texcoordVertexData.vkBuffer, gContext_Scene2.texcoordVertexData.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord buffer\n");
    }
    
    //#11
    data = NULL;
    vkResult = vkMapMemory(gContext_Scene2.device, gContext_Scene2.texcoordVertexData.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord buffer\n");
    }
   
    //#12
    memcpy(data, quadTexcoords, sizeof(quadTexcoords));
    
    vkUnmapMemory(gContext_Scene2.device, gContext_Scene2.texcoordVertexData.vkDeviceMemory);
   
    return (vkResult);
}

VkResult createTexture(const char* textureFileName)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step 1
    FILE* fp = NULL;
    fp = fopen(textureFileName, "rb");
    if(fp == NULL)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> fOpen() failed to open Stone.png texture file\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    
    uint8_t* imageData = NULL;
    int texture_width, texture_height, texture_channels;

    imageData = stbi_load_from_file(fp, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    if(imageData == NULL || texture_width <= 0 || texture_height <= 0 || texture_channels <= 0)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> stbi_load_from_file() failed to read Stone.png texture file\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    
    /*uint64_t :: VkDeviceSize*/
    VkDeviceSize image_size = texture_width * texture_height * 4 /*RCBA*/ ;
    
    //step 2
    VkBuffer vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
    
    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));    
    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    //valid flags are used in scatterred/sparse buffer
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = image_size;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // this buffer is source
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // buffer can be used for concurrent usage, for multithreading
    
    vkResult = vkCreateBuffer(gContext_Scene2.device, 
                              &vkBufferCreateInfo_stagingBuffer,
                              NULL,
                              &vkBuffer_stagingBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));
    
    vkGetBufferMemoryRequirements(gContext_Scene2.device, vkBuffer_stagingBuffer, &vkMemoryRequirements_stagingBuffer);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
    memset((void*)&vkMemoryAllocateInfo_stagingBuffer , 0, sizeof(VkMemoryAllocateInfo));
    
    vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_stagingBuffer.pNext = NULL; 
    vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;
    
    for(uint32_t i = 0; i < gContext_Scene2.physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
        {
            if(gContext_Scene2.physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) //VK_MEMORY_PROPERTY_HOST_COHERENT_BIT--> No need to manage vulkan cache mechanism for flushing and mapping as we order vulkan to maintain coherency
            {
                vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gContext_Scene2.device, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vkDeviceMemory_stagingBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(gContext_Scene2.device, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBindBufferMemory() is succedded\n");
    }
    
    void* data = NULL;
    vkResult = vkMapMemory(gContext_Scene2.device, vkDeviceMemory_stagingBuffer, 0, image_size, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkMapMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkMapMemory() is succedded\n");
    }
    
    //#12
    memcpy(data, imageData, image_size);
    
    vkUnmapMemory(gContext_Scene2.device, vkDeviceMemory_stagingBuffer);
    
    //As copying of image data into the staging buffer is completed, we can Free the actual image data given by stb 
    stbi_image_free(imageData);
    imageData = NULL;
    fprintf(gContext_Scene2.logFile, "createTexture() --> stbi_image_free() Freeing of image data is succedded\n");
    
    
    /*
    Step# 3. 
    Create "Device only visible", empty, but enough sized Image equal to size of image(image width * image Height).
    */
    
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;  //SRGB can ber found in other tutorials
    vkImageCreateInfo.extent.width = texture_width;
    vkImageCreateInfo.extent.height = texture_height;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(gContext_Scene2.device, &vkImageCreateInfo, NULL, &gContext_Scene2.textureImage);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkCreateImage() is failed error code is %d\n",vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkCreateImage() is succedded\n");
        fflush(gContext_Scene2.logFile);
    }
    
    //Memory requirement for texture image
    VkMemoryRequirements vkMemoryRequirements_image;
    memset((void*)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(gContext_Scene2.device, gContext_Scene2.textureImage, &vkMemoryRequirements_image);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
    memset((void*)&vkMemoryAllocateInfo_image , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo_image.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_image.pNext = NULL; 
    vkMemoryAllocateInfo_image.allocationSize = vkMemoryRequirements_image.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo_image.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < gContext_Scene2.physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements_image.memoryTypeBits & 1) == 1)
        {
            if(gContext_Scene2.physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo_image.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_image.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gContext_Scene2.device, &vkMemoryAllocateInfo_image, NULL, &gContext_Scene2.textureMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateMemory() is succedded\n");
        fflush(gContext_Scene2.logFile);
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindImageMemory(gContext_Scene2.device, gContext_Scene2.textureImage, gContext_Scene2.textureMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBindImageMemory() is failed and error code is %d\n", vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBindImageMemory() is succedded\n");
        fflush(gContext_Scene2.logFile);
    }
    
    /*
    Step#4. 
    Send "image transition layout" to the Vulkan / GPU, before the actual staging buffer from step 2 to empty vkImage of Step 3, using Pipeline Barrier.
    */
    
    /*
    Steps of Staging buffer, when pushing data to GPU in initialize
    
    Command pool must be allocated before these steps
    
    AllocateCmd buffer
    begin cmd buffer
    vkCMD
    end CMD nbuffer
    summbmit queue
    wait idleQueue
    Free comamnd buffer
    */
    
    //#4.1
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_transition_image_layout; 
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));
    
    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = gContext_Scene2.commandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    VkCommandBuffer vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(gContext_Scene2.device, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateCommandBuffers() is failed and error code is %d\n", vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
        fflush(gContext_Scene2.logFile);
    }

    //#4.2
    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_transition_image_layout;
    memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));
    
    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  //1. we will use only primary command buffers, 2. we are not going to use this command buffer simultaneoulsy between multipple threads
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBeginCommandBuffer() is failed and error code is %d\n", vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBeginCommandBuffer() is succedded \n");
        fflush(gContext_Scene2.logFile);
    }

    //#4.3: Setting Barrier
    VkPipelineStageFlags vkPipelineStageFlags_source = 0;
    VkPipelineStageFlags vkPipelineStageFlags_destination = 0;
    VkImageMemoryBarrier vkImageMemoryBarrier;
    memset((void*)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = gContext_Scene2.textureImage;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;
    
    if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
        
        //Vulkan Pipeline
        //1. Top Stage
        //2. Drawindirect
        //3. Vertex input stage
        //4. Vertex shader stage
        //5. TSC shader stage
        //6. TSE shader stage
        //7. Geometry Shader
        //8. Fragment shader stage
        //9. Early pixel test stage(implementation dependent) (some of these post processing tests, pixel ownership, scissor, stencil, alpha, dither, blend , depth, logic op)
        //10. Late Pixel Stage(implementation dependent)
        //11. Color attachment output stage
        //12. Compute Shader stage
        //13. Transfer stage
        //14. Bottom stage
        //15. Host stage
        //16. All graphic stage
        //17. All command stage
        
        
    }
    else if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> Unsupported texture layout transition()\n");
        fflush(gContext_Scene2.logFile);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    vkCmdPipelineBarrier(vkCommandBuffer_transition_image_layout, vkPipelineStageFlags_source, vkPipelineStageFlags_destination, 0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);
    
    //#4.4: End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkEndCommandBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");
    }
    
    //#4.5: Submitting Queue
    VkSubmitInfo  vkSubmitInfo_transition_image_layout;
    memset((void*)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
    
    vkSubmitInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_transition_image_layout.pNext = NULL;
    vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
    vkSubmitInfo_transition_image_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;
    // As there is no need of synchrnization for waitDstStageMask and Semaphore is not needed
    
    //Now submit our work to the Queue
    vkResult = vkQueueSubmit(gContext_Scene2.queue,
                             1,
                             &vkSubmitInfo_transition_image_layout,
                             VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueSubmit() succeded\n", vkResult);
    }

    //#4.6: Waiting
    vkResult = vkQueueWaitIdle(gContext_Scene2.queue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueWaitIdle() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueWaitIdle() succeded\n", vkResult);
    }
    
    //#4.7: Freeing
    vkFreeCommandBuffers(gContext_Scene2.device, gContext_Scene2.commandPool, 1, &vkCommandBuffer_transition_image_layout);
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    
    /*
    Step #5 
    Now actually copy the image data from staging buffer to the empty vkImage.
    */
    
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_buffer_to_image_copy; 
    memset((void*)&vkCommandBufferAllocateInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferAllocateInfo));
    
    vkCommandBufferAllocateInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandPool = gContext_Scene2.commandPool;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    VkCommandBuffer vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(gContext_Scene2.device, &vkCommandBufferAllocateInfo_buffer_to_image_copy, &vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateCommandBuffers() is failed for buffer_to_image_copy and error code is %d \n", vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateCommandBuffers() is succedded for buffer_to_image_copy\n");
        fflush(gContext_Scene2.logFile);
    }

    //#5.2
    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_buffer_to_image_copy;
    memset((void*)&vkCommandBufferBeginInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferBeginInfo));
    
    vkCommandBufferBeginInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferBeginInfo_buffer_to_image_copy.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  //1. we will use only primary command buffers, 2. we are not going to use this command buffer simultaneoulsy between multipple threads
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_buffer_to_image_copy, &vkCommandBufferBeginInfo_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBeginCommandBuffer() is failed for buffer_to_image_copy and error code is %d\n", vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBeginCommandBuffer() is succedded for buffer_to_image_copy \n");
        fflush(gContext_Scene2.logFile);
    }
    
    //5.3
    VkBufferImageCopy vkBufferImageCopy;
    memset((void*)&vkBufferImageCopy, 0, sizeof(VkBufferImageCopy));
    vkBufferImageCopy.bufferOffset = 0; 
    vkBufferImageCopy.bufferRowLength = 0;
    vkBufferImageCopy.bufferImageHeight = 0;
    vkBufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkBufferImageCopy.imageSubresource.mipLevel = 0;
    vkBufferImageCopy.imageSubresource.baseArrayLayer = 0;
    vkBufferImageCopy.imageSubresource.layerCount = 1;
    // vkBufferImageCopy.imageSubresource.levelCount = 1;
    vkBufferImageCopy.imageOffset.x = 0;
    vkBufferImageCopy.imageOffset.y = 0;
    vkBufferImageCopy.imageOffset.z = 0;
    vkBufferImageCopy.imageExtent.width = texture_width;
    vkBufferImageCopy.imageExtent.height = texture_height;
    vkBufferImageCopy.imageExtent.depth = 1;
    
    vkCmdCopyBufferToImage(vkCommandBuffer_buffer_to_image_copy, 
                           vkBuffer_stagingBuffer, 
                           gContext_Scene2.textureImage, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                           1, 
                           &vkBufferImageCopy);
    
    //#5.4: End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkEndCommandBuffer() is failed for buffer_to_image_copy and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkEndCommandBuffer() is succedded for buffer_to_image_copy\n");
    }
    
    //#5.5: Submitting Queue
    VkSubmitInfo  vkSubmitInfo_buffer_to_image_copy;
    memset((void*)&vkSubmitInfo_buffer_to_image_copy, 0, sizeof(VkSubmitInfo));
    
    vkSubmitInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_buffer_to_image_copy.pNext = NULL;
    vkSubmitInfo_buffer_to_image_copy.commandBufferCount = 1;
    vkSubmitInfo_buffer_to_image_copy.pCommandBuffers = &vkCommandBuffer_buffer_to_image_copy;
    // As there is no need of synchrnization for waitDstStageMask and Semaphore is not needed
    
    //Now submit our work to the Queue
    vkResult = vkQueueSubmit(gContext_Scene2.queue,
                             1,
                             &vkSubmitInfo_buffer_to_image_copy,
                             VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueSubmit() is failed for buffer_to_image_copy errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueSubmit() is succedded for buffer_to_image_copy");
    }

    //#5.6: Waiting
    vkResult = vkQueueWaitIdle(gContext_Scene2.queue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueWaitIdle() is failed for buffer_to_image_copy errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueWaitIdle() succeded for buffer_to_image_copy\n");
    }
    
    //#5.7: Freeing
    vkFreeCommandBuffers(gContext_Scene2.device, gContext_Scene2.commandPool, 1, &vkCommandBuffer_buffer_to_image_copy);
    vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    
    
    /*
    Step 6. 
    Now again do image layout transition similar to the step 4, for the corrext reading/writing of object data by shaders.
    */
    
    //#6.1
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));
    
    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = gContext_Scene2.commandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(gContext_Scene2.device, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateCommandBuffers() is failed and error code is %d\n", vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
        fflush(gContext_Scene2.logFile);
    }

    //#6.2
    memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));
    
    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  //1. we will use only primary command buffers, 2. we are not going to use this command buffer simultaneoulsy between multipple threads
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBeginCommandBuffer() is failed and error code is %d\n", vkResult);
        fflush(gContext_Scene2.logFile);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkBeginCommandBuffer() is succedded \n");
        fflush(gContext_Scene2.logFile);
    }

    //#6.3: Setting Barrier
    vkPipelineStageFlags_source = 0;
    vkPipelineStageFlags_destination = 0;

    memset((void*)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = gContext_Scene2.textureImage;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;
    
    if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = 0;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> Unsupported texture layout transition for 2nd time in step 6\n");
        fflush(gContext_Scene2.logFile);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    vkCmdPipelineBarrier(vkCommandBuffer_transition_image_layout, vkPipelineStageFlags_source, vkPipelineStageFlags_destination, 0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);
    
    //#6.4: End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkEndCommandBuffer() is failed for 2nd time in step 6 and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");
    }
    
    //#6.5: Submitting Queue
    memset((void*)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
    
    vkSubmitInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_transition_image_layout.pNext = NULL;
    vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
    vkSubmitInfo_transition_image_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;
    // As there is no need of synchrnization for waitDstStageMask and Semaphore is not needed
    
    //Now submit our work to the Queue
    vkResult = vkQueueSubmit(gContext_Scene2.queue,
                             1,
                             &vkSubmitInfo_transition_image_layout,
                             VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueSubmit() is failed  for 2nd time in step 6 errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueSubmit() succeded  for 2nd time in step 6 \n", vkResult);
    }

    //#6.6: Waiting
    vkResult = vkQueueWaitIdle(gContext_Scene2.queue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueWaitIdle() is failed  for 2nd time in step 6 errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkQueueWaitIdle() succeded for 2nd time in step 6 \n", vkResult);
    }
    
    //#6.7: Freeing
    vkFreeCommandBuffers(gContext_Scene2.device, gContext_Scene2.commandPool, 1, &vkCommandBuffer_transition_image_layout);
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    
    /*
    Step #7. 
    Now staging buffer is not needed, hence release its memory and itself
    */

    if(vkBuffer_stagingBuffer)
    {
        vkDestroyBuffer(gContext_Scene2.device, vkBuffer_stagingBuffer, NULL);
        vkBuffer_stagingBuffer = VK_NULL_HANDLE;
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkDestroyBuffer() is done for vkBuffer_stagingBuffer of setp 7\n");
    }
    
    if(vkDeviceMemory_stagingBuffer)
    {
       vkFreeMemory(gContext_Scene2.device, vkDeviceMemory_stagingBuffer, NULL);
       vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
       fprintf(gContext_Scene2.logFile, "createTexture() --> vkFreeMemory() is done for vkBuffer_stagingBuffer of setp 7\n");
    }
    
    /* Step8. 
       Create imageview of above image
    */
    // Initialize vkImageViewCreateInfo structure
    VkImageViewCreateInfo vkImageViewCreateInfo;
    //create image view for above depth image
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    // ***** MODIFIED: this is a COLOR image; aspect must be COLOR only (removes validation error)
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = gContext_Scene2.textureImage;
    
    vkResult = vkCreateImageView(gContext_Scene2.device,
                                 &vkImageViewCreateInfo,
                                 NULL,
                                 &gContext_Scene2.textureImageView);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkCreateImageView() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkCreateImageView() is succedded\n");
    }
    
    
    
    /*Step 9. 
      Create texture sampler of above image
     */
     
    VkSamplerCreateInfo vkSamplerCreateInfo;
    memset((void*)&vkSamplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));
    vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.pNext = NULL;
    vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.anisotropyEnable = VK_FALSE;
    vkSamplerCreateInfo.maxAnisotropy = 16;
    vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    vkSamplerCreateInfo.compareEnable = VK_FALSE;
    vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    
    vkResult = vkCreateSampler(gContext_Scene2.device, &vkSamplerCreateInfo, NULL, &gContext_Scene2.textureSampler);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkCreateSampler() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createTexture() --> vkCreateSampler() is succedded\n");
    }
     
    return vkResult; 
}

VkResult createUniformBuffer(void)
{
    //Function declarations
    VkResult updateUniformBuffer(void);
    
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    
    //#4 memset the global strucure variable
    memset((void*)&gContext_Scene2.uniformData, 0, sizeof(gContext_Scene2.uniformData));
    
    //#5 VkBufferCreateInfo structure filling
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    //valid flags are used in scatterred/sparse buffer
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(gContext_Scene2.uniformBufferObject);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    //#6
    vkResult = vkCreateBuffer(gContext_Scene2.device, 
                              &vkBufferCreateInfo,
                              NULL,
                              &gContext_Scene2.uniformData.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(gContext_Scene2.device, gContext_Scene2.uniformData.vkBuffer, &vkMemoryRequirements);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < gContext_Scene2.physicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(gContext_Scene2.physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gContext_Scene2.device, &vkMemoryAllocateInfo, NULL, &gContext_Scene2.uniformData.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(gContext_Scene2.device, gContext_Scene2.uniformData.vkBuffer, gContext_Scene2.uniformData.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
    }
    
    //call updateUniformBuffer()
    vkResult = FunctionTable_Scene2.updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createUniformBuffer() --> updateUniformBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createUniformBuffer() --> updateUniformBuffer() is succedded\n");
    }
    
    return vkResult;
}

VkResult updateUniformBuffer(void)
{
    VkResult vkResult = VK_SUCCESS;

    // For a fullscreen quad: clip-space passthrough.
    // Positions in the VBO are already in the range [-1, +1],
    // so use identity matrices to avoid any scaling/translation/perspective.
    memset((void*)&gContext_Scene2.uniformBufferObject, 0, sizeof(gContext_Scene2.uniformBufferObject));

    gContext_Scene2.uniformBufferObject.modelMatrix      = glm::mat4(1.0f); // no scale/rotation/translation
    gContext_Scene2.uniformBufferObject.viewMatrix       = glm::mat4(1.0f); // no camera
    gContext_Scene2.uniformBufferObject.projectionMatrix = glm::mat4(1.0f); // identity (no perspective, no Y-flip)

    // Map → copy → unmap
    void* data = NULL;
    vkResult = vkMapMemory(gContext_Scene2.device, gContext_Scene2.uniformData.vkDeviceMemory, 0, sizeof(gContext_Scene2.uniformBufferObject), 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "updateUniformBuffer() --> vkMapMemory() failed, error code = %d\n", vkResult);
        return vkResult;
    }

    memcpy(data, &gContext_Scene2.uniformBufferObject, sizeof(gContext_Scene2.uniformBufferObject));
    vkUnmapMemory(gContext_Scene2.device, gContext_Scene2.uniformData.vkDeviceMemory);

    return vkResult;
}

VkResult createShaders(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //for vertex shader
    const char* szFileName = "Shader_Scene2.vert.spv";
    FILE* fp = NULL;
    size_t size;
    
    //#6a
    fp = fopen(szFileName, "rb"); //open for reading in binary format
    if(fp == NULL)
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> fopen() failed to open shader.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> fopen() succedded to open shader.vert.spv\n");
    }
    
    //#6b
    fseek(fp, 0L, SEEK_END);
    
    //#6c
    size = ftell(fp);
    if(size == 0)
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> ftell() failed to provide size of shader.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
   
    //#6d
    fseek(fp, 0L, SEEK_SET); //reset to start
    
    //#6e
    char* shaderData = (char*)malloc(sizeof(char) * size);
    size_t retVal = fread(shaderData, size, 1, fp);
    if(retVal != 1)
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> fread() failed to read shader.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> fread() succedded to read shader.vert.spv\n");
    }
    
    //#6f
    fclose(fp);
    
    //#7
    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; // reserved, hence must be zero
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    //8
    vkResult = vkCreateShaderModule(gContext_Scene2.device, &vkShaderModuleCreateInfo, NULL, &gContext_Scene2.vertexShaderModule);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> vkCreateShaderModule() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> vkCreateShaderModule() is succedded\n");
    }
    
    //#9
    if(shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }
    
    fprintf(gContext_Scene2.logFile, "createShaders() --> vertex Shader module successfully created\n");
    
    
    //for fragment shader
    szFileName = "Shader_Scene2.frag.spv";
    fp = NULL;
    size = 0;
    
    //#6a
    fp = fopen(szFileName, "rb"); //open for reading in binary format
    if(fp == NULL)
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> fopen() failed to open shader.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> fopen() succedded to open shader.frag.spv\n");
    }
    
    //#6b
    fseek(fp, 0L, SEEK_END);
    
    //#6c
    size = ftell(fp);
    if(size == 0)
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> ftell() failed to provide size of shader.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
   
    //#6d
    fseek(fp, 0L, SEEK_SET); //reset to start
    
    //#6e
    shaderData = (char*)malloc(sizeof(char) * size);
    retVal = fread(shaderData, size, 1, fp);
    if(retVal != 1)
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> fread() failed to read shader.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> fread() succedded to read shader.frag.spv\n");
    }
    
    //#6f
    fclose(fp);
    
    //#7
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; // reserved, hence must be zero
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    //8
    vkResult = vkCreateShaderModule(gContext_Scene2.device, &vkShaderModuleCreateInfo, NULL, &gContext_Scene2.fragmentShaderModule);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> vkCreateShaderModule() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createShaders() --> vkCreateShaderModule() is succedded\n");
    }
    
    //#9
    if(shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }
    
    fprintf(gContext_Scene2.logFile, "createShaders() --> fragment Shader module successfully created\n");
     
    return (vkResult);
}

VkResult createDescriptorSetLayout(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //Descriptor set binding 
    //0th index --> uniform
    //1st index --> texture image
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_array[2];
    memset((void*)vkDescriptorSetLayoutBinding_array, 0, sizeof(VkDescriptorSetLayoutBinding) * _ARRAYSIZE(vkDescriptorSetLayoutBinding_array));
    
    // for MVP Uniform
    vkDescriptorSetLayoutBinding_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorSetLayoutBinding_array[0].binding = 0;  // this zero related with zero binding in vertex shader
    vkDescriptorSetLayoutBinding_array[0].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  //shader stage
    vkDescriptorSetLayoutBinding_array[0].pImmutableSamplers = NULL;
    
    // for texture image and sampler
    vkDescriptorSetLayoutBinding_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorSetLayoutBinding_array[1].binding = 1;  // this one related with 1 binding in fragment shader
    vkDescriptorSetLayoutBinding_array[1].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  //shader stage
    vkDescriptorSetLayoutBinding_array[1].pImmutableSamplers = NULL;
    
    
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0; //reserved
    vkDescriptorSetLayoutCreateInfo.bindingCount = _ARRAYSIZE(vkDescriptorSetLayoutBinding_array); // one DescriptorSet available
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_array;
    
    vkResult = vkCreateDescriptorSetLayout(gContext_Scene2.device, &vkDescriptorSetLayoutCreateInfo, NULL, &gContext_Scene2.descriptorSetLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayour() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayour() is succedded\n");
    }
    
    return (vkResult);
}

VkResult createPipelineLayout(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0; //reserved
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &gContext_Scene2.descriptorSetLayout;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
    
    vkResult = vkCreatePipelineLayout(gContext_Scene2.device, &vkPipelineLayoutCreateInfo, NULL, &gContext_Scene2.pipelineLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createPipelineLayout() --> vkCreatePipelineLayout() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createPipelineLayout() --> vkCreatePipelineLayout() is succedded\n");
    }
    
    return (vkResult);
}

VkResult createDescriptorPool(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //before creating actual descriptor pool, vulkan expects descriptor pool size
    //0th index --> uniform
    //1st index --> texture image
    VkDescriptorPoolSize vkDescriptorPoolSize_array[2];
    memset((void*)vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize) * _ARRAYSIZE(vkDescriptorPoolSize_array));
    
    //for MVP uniform
    vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize_array[0].descriptorCount = 1;
    
    //for Texture and sampler uniform 
    vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolSize_array[1].descriptorCount = 1;
    
    
    //Create the pool
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = _ARRAYSIZE(vkDescriptorPoolSize_array);
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;
    vkDescriptorPoolCreateInfo.maxSets = 2;
    
    vkResult = vkCreateDescriptorPool(gContext_Scene2.device, &vkDescriptorPoolCreateInfo, NULL, &gContext_Scene2.descriptorPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createDescriptorPool() --> vkCreateDescriptorPool() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createDescriptorPool() --> vkCreateDescriptorPool() is succedded\n");
    }
    
    return (vkResult);
}


VkResult createDescriptorSet(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //Initialize descriptorset allocation info
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = gContext_Scene2.descriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;  //though we have 2 descriptors, 1 for MVP uniform and 1 for texture sampler, both are in 1 same descriptor set
    vkDescriptorSetAllocateInfo.pSetLayouts = &gContext_Scene2.descriptorSetLayout;
    
    vkResult = vkAllocateDescriptorSets(gContext_Scene2.device, &vkDescriptorSetAllocateInfo, &gContext_Scene2.descriptorSet);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded\n");
    }
    
    //Describe whether we want buffer as uniform or image as uniform
    //For Buffer Uniform
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = gContext_Scene2.uniformData.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(gContext_Scene2.uniformBufferObject);
    
    
    //for Texture image & sampler
    VkDescriptorImageInfo vkDescriptorImageInfo;
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = gContext_Scene2.textureImageView;
    vkDescriptorImageInfo.sampler = gContext_Scene2.textureSampler;
    
   
    
    //now update the above descriptor set directly to shader
    //There are two ways to update -->driectly writing in shader or copying from one shader to another shader
    //we will be using writing to shader option, this require initilaization of following structure
    VkWriteDescriptorSet vkWriteDescriptorSet_array[2];
    //2 VkWriteDescriptorSet for above two steructure
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    //for MVP
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = gContext_Scene2.descriptorSet;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; //where to bind in shader, and our uniform is at binding 0 index in shader
    
    //for texture image & sampler
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = gContext_Scene2.descriptorSet;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; //where to bind in shader, and our uniform is at binding 1 index in frag shader
    
    
    vkUpdateDescriptorSets(gContext_Scene2.device, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gContext_Scene2.logFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
    
    return (vkResult);
}

VkResult createRenderPass(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1:
    VkAttachmentDescription vkAttachmentDescription_array[2];  //for both color and depth
    memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));
    
    //for color
    vkAttachmentDescription_array[0].flags = 0; //For embedded devices
    vkAttachmentDescription_array[0].format = gContext_Scene2.colorFormat;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    //for depth
    vkAttachmentDescription_array[1].flags = 0; //For embedded devices
    vkAttachmentDescription_array[1].format = gContext_Scene2.depthFormat;
    vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    

    //Step2:
    //for color attachment
    VkAttachmentReference vkAttachmentReference_color;
    memset((void*)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));
    
    vkAttachmentReference_color.attachment = 0;  //index number
    vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //how to keep/desires/use the layout of image
    
    //for depth attachment
    VkAttachmentReference vkAttachmentReference_depth;
    memset((void*)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));
    
    vkAttachmentReference_depth.attachment = 1;  //index number
    vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //how to keep/desires/use the layout of image
    
    
    //step3:
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    // vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);  // earlier code
    vkSubpassDescription.colorAttachmentCount = 1;  //Recommended change for Depth:: This count should be of count vkAttachmentReference of color count
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;
    
    //step4:
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
    
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;
    
    
    //step5:
    vkResult = vkCreateRenderPass(gContext_Scene2.device, 
                                  &vkRenderPassCreateInfo,
                                  NULL,
                                  &gContext_Scene2.renderPass);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createRenderPass() --> vkCreateRenderPass() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createRenderPass() --> vkCreateRenderPass() is succedded\n");
    }
    
    return (vkResult);
}

VkResult createPipeline(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //#1: vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[2]; 
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));
    
    //for position
    vkVertexInputBindingDescription_array[0].binding = 0;//corresponding to location 0 in vertex shader
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    //for texcoord
    vkVertexInputBindingDescription_array[1].binding = 1; //corresponding to location 1 in vertex shader
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 2;
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  
    
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2]; 
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));
    
    //for position
    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;
    
    //for texcoord
    vkVertexInputAttributeDescription_array[1].binding = 1;
    vkVertexInputAttributeDescription_array[1].location = 1;
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT;
    vkVertexInputAttributeDescription_array[1].offset = 0;
    
    
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;
    
    
    //#2: Input assembly State
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = 0;
    
    
    //#3: Rasterizer state
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    
    //#4: Color Blend State
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0xF;
    
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    
    //#5: viewport sciessor state
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; //Means we can specify multiple viewport
    
    memset((void*)&gContext_Scene2.viewport, 0, sizeof(VkViewport));
    gContext_Scene2.viewport.x = 0;
    gContext_Scene2.viewport.y = 0;
    gContext_Scene2.viewport.width = (float)gContext_Scene2.swapchainExtent.width;
    gContext_Scene2.viewport.height = (float)gContext_Scene2.swapchainExtent.height;
    gContext_Scene2.viewport.minDepth = 0.0f;
    gContext_Scene2.viewport.maxDepth = 1.0f;
    
    vkPipelineViewportStateCreateInfo.pViewports = &gContext_Scene2.viewport;
    vkPipelineViewportStateCreateInfo.scissorCount = 1;
    
    memset((void*)&gContext_Scene2.scissor, 0, sizeof(VkRect2D));
    gContext_Scene2.scissor.offset.x = 0;
    gContext_Scene2.scissor.offset.y = 0;
    gContext_Scene2.scissor.extent.width = (float)gContext_Scene2.swapchainExtent.width;
    gContext_Scene2.scissor.extent.height = (float)gContext_Scene2.swapchainExtent.height;
    
    vkPipelineViewportStateCreateInfo.pScissors = &gContext_Scene2.scissor;
    
    //#6: Depth Stencil state
    //THIS STATE CAN BE OMMITTED AS WE DONT HAVE THE DEPTH
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back;
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;    
    
    //#7: Dynamic State
    //THIS STATE CAN BE OMMITTED AS WE DONT HAVE ANY DYNAMIC STATE
    
    //#8: Multi Sample State(needed for fragment shader)
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    //#9: Shader State
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));
    //Vertex Shader
    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[0].flags = 0;
    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkPipelineShaderStageCreateInfo_array[0].module = gContext_Scene2.vertexShaderModule;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main";
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;
    
    //fragment Shader
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = gContext_Scene2.fragmentShaderModule;
    vkPipelineShaderStageCreateInfo_array[1].pName = "main";
    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;
    
    //#10: Tessellation State
    //THIS STATE CAN BE OMMITTED AS WE DONT HAVE ANY TESSELLATION SHADER
    
    
    //As pipeline are created from pipeline cache, we will create the pipeline cache object
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;
    
    
    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE; 
    vkResult = vkCreatePipelineCache(gContext_Scene2.device, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createPipeline() --> vkCreatePipelineCache() is failed error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createPipeline() --> vkCreatePipelineCache() is succedded\n");
    }
    
    //create the actual graphics pipeline
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
    vkGraphicsPipelineCreateInfo.layout = gContext_Scene2.pipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass = gContext_Scene2.renderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0; //as we have only one renderpass
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;
    
    //Now create the pipeline
    vkResult = vkCreateGraphicsPipelines(gContext_Scene2.device,
                                         vkPipelineCache,
                                         1,
                                         &vkGraphicsPipelineCreateInfo,
                                         NULL,
                                         &gContext_Scene2.pipeline);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createPipeline() --> vkCreateGraphicsPipelines() is failed error code is %d\n", vkResult);
        vkDestroyPipelineCache(gContext_Scene2.device, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createPipeline() --> vkCreateGraphicsPipelines() is succedded\n");
    }
    
    //we ar done with pipeline cache so destroy it
    vkDestroyPipelineCache(gContext_Scene2.device, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;
    
    return (vkResult);
}



VkResult createFrameBuffers(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    VkImageView vkImageView_attachment_array[1];
    memset((void*)vkImageView_attachment_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachment_array));
    
    // step2
    VkFramebufferCreateInfo vkFramebufferCreateInfo;
    memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
    
    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.pNext = NULL;
    vkFramebufferCreateInfo.flags = 0;
    vkFramebufferCreateInfo.renderPass = gContext_Scene2.renderPass;
    vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
    vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
    vkFramebufferCreateInfo.width = gContext_Scene2.swapchainExtent.width;
    vkFramebufferCreateInfo.height = gContext_Scene2.swapchainExtent.height;
    vkFramebufferCreateInfo.layers = 1;
    
    //step3:
    gContext_Scene2.framebuffers = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * gContext_Scene2.swapchainImageCount);
    //check for malloc
    
    //step4:
    
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        // Recommended Change for depth
        /*********************************************************************************************************************/
        VkImageView vkImageView_attachment_array[2]; //for color and depth
        memset((void*)vkImageView_attachment_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachment_array));
        
        //step2
        VkFramebufferCreateInfo vkFramebufferCreateInfo;
        memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
        
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.pNext = NULL;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.renderPass = gContext_Scene2.renderPass;
        vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
        vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
        vkFramebufferCreateInfo.width = gContext_Scene2.swapchainExtent.width;
        vkFramebufferCreateInfo.height = gContext_Scene2.swapchainExtent.height;
        vkFramebufferCreateInfo.layers = 1;
        
        /*********************************************************************************************************************/
        
        vkImageView_attachment_array[0] = gContext_Scene2.swapchainImageViews[i];
        vkImageView_attachment_array[1] = gContext_Scene2.depthImageView;
        
        vkResult = vkCreateFramebuffer(gContext_Scene2.device, 
                                       &vkFramebufferCreateInfo,
                                       NULL,
                                       &gContext_Scene2.framebuffers[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "createFrameBuffers() --> vkCreateFramebuffer() is failed for %d iteration and error code is %d\n",i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "createFrameBuffers() --> vkCreateFramebuffer() is succedded for iteration %d\n", i);
        }
    }
    
    return vkResult;
}


VkResult createSemaphores(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
    
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;  //Binary and Timeline Semaphore info, bydefault it is Binary
    vkSemaphoreCreateInfo.flags = 0; //RESERVED: must be zero
    
    //backBuffer Semaphore
    vkResult = vkCreateSemaphore(gContext_Scene2.device, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &gContext_Scene2.backBufferSemaphore);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createSemaphores() --> vkCreateSemaphore() is failed for gContext_Scene2.backBufferSemaphore and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createSemaphores() --> vkCreateSemaphore() is succedded for gContext_Scene2.backBufferSemaphore\n");
    }
    
    
    //renderComplete Semaphore
    vkResult = vkCreateSemaphore(gContext_Scene2.device, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &gContext_Scene2.renderCompleteSemaphore);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gContext_Scene2.logFile, "createSemaphores() --> vkCreateSemaphore() is failed for gContext_Scene2.renderCompleteSemaphore and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gContext_Scene2.logFile, "createSemaphores() --> vkCreateSemaphore() is succedded for gContext_Scene2.renderCompleteSemaphore\n");
    }
    
    return vkResult;
    
}

VkResult createFences(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    VkFenceCreateInfo vkFenceCreateInfo;
    memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));
    
    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    // ***** MODIFIED: allocate for 'gContext_Scene2.swapchainImageCount' fences (not sizeof(gContext_Scene2.swapchainImageCount))
    gContext_Scene2.fences = (VkFence*)malloc(sizeof(VkFence) * gContext_Scene2.swapchainImageCount);
    //malloc error checking to be done
    
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        vkResult = vkCreateFence(gContext_Scene2.device, 
                                 &vkFenceCreateInfo,
                                 NULL,
                                 &gContext_Scene2.fences[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "createFences() --> vkCreateFence() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "createFences() --> vkCreateFence() is succedded for %d iteration\n", i);
        }
    }

    return vkResult;
}

VkResult buildCommandBuffers(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    //Loop per swapchainImagecount
    for(uint32_t i = 0; i < gContext_Scene2.swapchainImageCount; i++)
    {
        //ResetCommandBuffers
        vkResult = vkResetCommandBuffer(gContext_Scene2.commandBuffers[i], 0);  // second parameetr: this command buffer is created from command pool, so dont release the resouces created by this comand buffer
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "buildCommandBuffers() --> vkResetCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "buildCommandBuffers() --> vkResetCommandBuffer() is succedded for %d iteration\n", i);
        }
        
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        
        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0;  //1. we will use only primary command buffers, 2. we are not going to use this command buffer simultaneoulsy between multipple threads
        
        vkResult = vkBeginCommandBuffer(gContext_Scene2.commandBuffers[i], &vkCommandBufferBeginInfo);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "buildCommandBuffers() --> vkBeginCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "buildCommandBuffers() --> vkBeginCommandBuffer() is succedded for %d iteration\n", i);
        }
        
        //SetClearValues
        VkClearValue vkClearValue_array[2];
        memset((void*)vkClearValue_array, 0, (sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array)));
        
        vkClearValue_array[0].color = gContext_Scene2.clearColor;
        vkClearValue_array[1].depthStencil = gContext_Scene2.clearDepthStencil;
        
        //Fill REnderpass 
        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));
        
        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = gContext_Scene2.renderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = gContext_Scene2.swapchainExtent.width;
        vkRenderPassBeginInfo.renderArea.extent.height = gContext_Scene2.swapchainExtent.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = gContext_Scene2.framebuffers[i];
        
        //BeginRenderPass
        vkCmdBeginRenderPass(gContext_Scene2.commandBuffers[i], 
                             &vkRenderPassBeginInfo, 
                             VK_SUBPASS_CONTENTS_INLINE);

        //Bind with the pipeline
        vkCmdBindPipeline(gContext_Scene2.commandBuffers[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          gContext_Scene2.pipeline);
        //Bind our descriptor set with the pipeline
        vkCmdBindDescriptorSets(gContext_Scene2.commandBuffers[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                gContext_Scene2.pipelineLayout,
                                0,
                                1,
                                &gContext_Scene2.descriptorSet,
                                0, 
                                NULL);
        
        //bind vertex position buffer
        VkDeviceSize vkDeviceSize_offset_position[1];
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(gContext_Scene2.commandBuffers[i], 
                               0, //binding point
                               1, //how many buffer
                               &gContext_Scene2.positionVertexData.vkBuffer, //which buffer
                               vkDeviceSize_offset_position);
        
        //bind vertex texcoord buffer
        VkDeviceSize vkDeviceSize_offset_texcoord[1];
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(gContext_Scene2.commandBuffers[i], 
                               1, //binding point
                               1, //how many buffer
                               &gContext_Scene2.texcoordVertexData.vkBuffer, //which buffer
                               vkDeviceSize_offset_texcoord);
                                     
        // Draw a quad (2 triangles → 6 vertices)
        vkCmdDraw(gContext_Scene2.commandBuffers[i],
                  6,  // vertices
                  1,  // instances
                  0,  // firstVertex
                  0); // firstInstance
        
        //EndRenderPass
        vkCmdEndRenderPass(gContext_Scene2.commandBuffers[i]);
        
        //End Command Buffer recording
        vkResult = vkEndCommandBuffer(gContext_Scene2.commandBuffers[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gContext_Scene2.logFile, "buildCommandBuffers() --> vkEndCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gContext_Scene2.logFile, "buildCommandBuffers() --> vkEndCommandBuffer() is succedded for %d iteration\n", i);
        }
    }
    
    return vkResult;
}

 VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT,
                                                   VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,
                                                   uint64_t object,
                                                   size_t location,
                                                   int32_t messageCode,
                                                   const char* pLayerPrefix,
                                                   const char* pMessage,
                                                   void* pUserData)
{
    //code
    fprintf(gContext_Scene2.logFile, "SSA_Validation: debugReportCallback() --> %s (%d) = %s\n", pLayerPrefix, messageCode, pMessage);

    return(VK_FALSE);
}

static void PopulateFunctionTable_Scene2(void)
{
    Win32FunctionTable_Scene2.WndProc = WndProc;
    Win32FunctionTable_Scene2.ToggleFullscreen = ToggleFullscreen;
    Win32FunctionTable_Scene2.Initialize = Initialize;
    Win32FunctionTable_Scene2.Resize = Resize;
    Win32FunctionTable_Scene2.Display = Display;
    Win32FunctionTable_Scene2.Update = Update;
    Win32FunctionTable_Scene2.Uninitialize = Uninitialize;

    FunctionTable_Scene2.InitializeWin32WindowContext_Scene2 = InitializeWin32WindowContext_Scene2;
    FunctionTable_Scene2.InitializeGlobalContext_Scene2 = InitializeGlobalContext_Scene2;
    FunctionTable_Scene2.WinMain = WinMain;
    FunctionTable_Scene2.createVulkanInstance = createVulkanInstance;
    FunctionTable_Scene2.fillInstanceExtensionNames = fillInstanceExtensionNames;
    FunctionTable_Scene2.fillValidationLayerNames = fillValidationLayerNames;
    FunctionTable_Scene2.createValidationCallbackFunction = createValidationCallbackFunction;
    FunctionTable_Scene2.getSupportedSurface = getSupportedSurface;
    FunctionTable_Scene2.getPhysicalDevice = getPhysicalDevice;
    FunctionTable_Scene2.printVkInfo = printVkInfo;
    FunctionTable_Scene2.fillDeviceExtensionNames = fillDeviceExtensionNames;
    FunctionTable_Scene2.createVulkanDevice = createVulkanDevice;
    FunctionTable_Scene2.getDeviceQueue = getDeviceQueue;
    FunctionTable_Scene2.getPhysicalDeviceSurfaceFormatAndColorSpace = getPhysicalDeviceSurfaceFormatAndColorSpace;
    FunctionTable_Scene2.getPhysicalDevicePresentMode = getPhysicalDevicePresentMode;
    FunctionTable_Scene2.createSwapchain = createSwapchain;
    FunctionTable_Scene2.createImagesAndImageViews = createImagesAndImageViews;
    FunctionTable_Scene2.GetSupportedDepthFormat = GetSupportedDepthFormat;
    FunctionTable_Scene2.createCommandPool = createCommandPool;
    FunctionTable_Scene2.createCommandBuffers = createCommandBuffers;
    FunctionTable_Scene2.createVertexBuffer = createVertexBuffer;
    FunctionTable_Scene2.createTexture = createTexture;
    FunctionTable_Scene2.createUniformBuffer = createUniformBuffer;
    FunctionTable_Scene2.updateUniformBuffer = updateUniformBuffer;
    FunctionTable_Scene2.createShaders = createShaders;
    FunctionTable_Scene2.createDescriptorSetLayout = createDescriptorSetLayout;
    FunctionTable_Scene2.createPipelineLayout = createPipelineLayout;
    FunctionTable_Scene2.createDescriptorPool = createDescriptorPool;
    FunctionTable_Scene2.createDescriptorSet = createDescriptorSet;
    FunctionTable_Scene2.createRenderPass = createRenderPass;
    FunctionTable_Scene2.createPipeline = createPipeline;
    FunctionTable_Scene2.createFrameBuffers = createFrameBuffers;
    FunctionTable_Scene2.createSemaphores = createSemaphores;
    FunctionTable_Scene2.createFences = createFences;
    FunctionTable_Scene2.buildCommandBuffers = buildCommandBuffers;
    FunctionTable_Scene2.debugReportCallback = debugReportCallback;
}
