#include <stdio.h>		
#include <stdlib.h>	
#include <windows.h>	
#include <math.h>	

#include "Scene0.h"
#include "Sphere.h"			
#define LOG_FILE (char*)"Log.txt" 

//Vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR // XLIB_KHR, MACOS_KHR & MOLTEN something
#include <vulkan/vulkan.h> //(Only those members are enabled connected with above macro {conditional compilation using #ifdef internally})

//GLM related macro and header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

//Vulkan related libraries
#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "Sphere.lib")

struct AppFunctionTable_Scene0
{
        LRESULT (CALLBACK *wndProc)(HWND, UINT, WPARAM, LPARAM);
        VkResult (*initialize)(void);
        void (*uninitialize)(void);
        VkResult (*display)(void);
        void (*update)(void);
        VkResult (*resize)(int, int);
        void (*toggleFullscreen)(void);
};

struct VulkanFunctionTable_Scene0
{
        VkResult (*createVulkanInstance)(void);
        VkResult (*fillInstanceExtensionNames)(void);
        VkResult (*fillValidationLayerNames)(void);
        VkResult (*createValidationCallbackFunction)(void);
        VkResult (*getSupportedSurface)(void);
        VkResult (*getPhysicalDevice)(void);
        VkResult (*printVulkanInfo)(void);
        VkResult (*fillDeviceExtensionNames)(void);
        VkResult (*createVulKanDevice)(void);
        void (*getDeviceQueque)(void);
        VkResult (*getPhysicalDeviceSurfaceFormatAndColorSpace)(void);
        VkResult (*getPhysicalDevicePresentMode)(void);
        VkResult (*createSwapChain)(VkBool32);
        VkResult (*createImagesAndImageViews)(void);
        VkResult (*getSupportedDepthFormat)(void);
        VkResult (*createCommandPool)(void);
        VkResult (*createCommandBuffers)(void);
        VkResult (*createVertexBuffer)(void);
        VkResult (*createIndexBuffer)(void);
        VkResult (*createUniformBuffer)(void);
        VkResult (*createShaders)(void);
        VkResult (*createDescriptorSetLayout)(void);
        VkResult (*createPipelineLayout)(void);
        VkResult (*createDescriptorPool)(void);
        VkResult (*createDescriptorSet)(void);
        VkResult (*createRenderPass)(void);
        VkResult (*createPipeline)(void);
        VkResult (*createFramebuffers)(void);
        VkResult (*createSemaphores)(void);
        VkResult (*createFences)(void);
        VkResult (*buildCommandBuffers)(void);
        VkResult (*updateUniformBuffer)(void);
};

extern AppFunctionTable_Scene0 gAppFunctions_Scene0;
extern VulkanFunctionTable_Scene0 gVulkanFunctions_Scene0;

struct WindowState_Scene0
{
	const char* appName = "ARTR";
	HWND hwnd = NULL;
	BOOL isActive = FALSE;
	DWORD style = 0;
	WINDOWPLACEMENT previousPlacement{};
	BOOL isFullscreen = FALSE;
	BOOL isMinimized = FALSE;
};

struct GlobalState_Scene0
{
        struct MyUniformData
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

        struct UniformData
        {
                VkBuffer vkBuffer; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBuffer.html
                VkDeviceMemory vkDeviceMemory; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceMemory.html
        };

        struct VertexData
        {
                VkBuffer vkBuffer; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBuffer.html
                VkDeviceMemory vkDeviceMemory; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceMemory.html
        };

        struct Light
        {
                float ambient[4];
                float diffuse[4];
                float specular[4];
                float position[4];
        };

	WindowState_Scene0 window{};

        FILE* gFILE = NULL;

        uint32_t enabledInstanceExtensionsCount = 0;
        const char* enabledInstanceExtensionNames_array[3] = {};

        VkInstance vkInstance = VK_NULL_HANDLE;
        VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

        VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
        uint32_t graphicsQuequeFamilyIndex_selected = UINT32_MAX;
        VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties{};
        uint32_t physicalDeviceCount = 0;
        VkPhysicalDevice* vkPhysicalDevice_array = NULL;

        uint32_t enabledDeviceExtensionsCount = 0;
        const char* enabledDeviceExtensionNames_array[1] = {};

        VkDevice vkDevice = VK_NULL_HANDLE;
        VkQueue vkQueue = VK_NULL_HANDLE;

        VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
        VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

        int winWidth = WIN_WIDTH;
        int winHeight = WIN_HEIGHT;

        VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
        VkExtent2D vkExtent2D_SwapChain{};

        uint32_t swapchainImageCount = UINT32_MAX;
        VkImage* swapChainImage_array = NULL;
        VkImageView* swapChainImageView_array = NULL;

        VkFormat vkFormat_depth = VK_FORMAT_UNDEFINED;
        VkImage vkImage_depth = VK_NULL_HANDLE;
        VkDeviceMemory vkDeviceMemory_depth = VK_NULL_HANDLE;
        VkImageView vkImageView_depth = VK_NULL_HANDLE;

        VkCommandPool vkCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer* vkCommandBuffer_array = NULL;

        VkRenderPass vkRenderPass = VK_NULL_HANDLE;
        VkFramebuffer* vkFramebuffer_array = NULL;

        VkSemaphore vkSemaphore_BackBuffer = VK_NULL_HANDLE;
        VkSemaphore vkSemaphore_RenderComplete = VK_NULL_HANDLE;
        VkFence* vkFence_array = NULL;

        VkClearColorValue vkClearColorValue{};
        VkClearDepthStencilValue vkClearDepthStencilValue{};

        BOOL bInitialized = FALSE;
        uint32_t currentImageIndex = UINT32_MAX;

        BOOL bValidation = TRUE;
        uint32_t enabledValidationLayerCount = 0;
        const char* enabledValidationlayerNames_array[1] = {};
        VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

        unsigned int numVertices = 0;
        unsigned int numElements = 0;
        float sphere_vertices[1146]{};
        float sphere_normals[1146]{};
        float sphere_textures[764]{};
        unsigned short sphere_elements[2280]{};

        VertexData vertexdata_position{};
        VertexData vertexdata_normals{};
        VertexData vertexdata_texcoord{};
        VertexData vertexdata_index{};

        MyUniformData uniformBufferData{};
        UniformData uniformData{};

        VkShaderModule vkShaderMoudule_vertex_shader = VK_NULL_HANDLE;
        VkShaderModule vkShaderMoudule_fragment_shader = VK_NULL_HANDLE;

        VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;
        VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;
        VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

        VkViewport vkViewPort{};
        VkRect2D vkRect2D_scissor{};
        VkPipeline vkPipeline = VK_NULL_HANDLE;

        BOOL bLight = FALSE;
        float lightAngle = 0.0f;
        static constexpr float lightRadius = 100.0f;
        Light lights[3] =
        {
                {
                        { 0.0f, 0.0f, 0.0f, 1.0f },
                        { 1.0f, 0.0f, 0.0f, 1.0f },
                        { 1.0f, 0.0f, 0.0f, 1.0f },
                        { 0.0f, lightRadius, 0.0f, 1.0f }
                },
                {
                        { 0.0f, 0.0f, 0.0f, 1.0f },
                        { 0.0f, 1.0f, 0.0f, 1.0f },
                        { 0.0f, 1.0f, 0.0f, 1.0f },
                        { lightRadius, 0.0f, 0.0f, 1.0f }
                },
                {
                        { 0.0f, 0.0f, 0.0f, 1.0f },
                        { 0.0f, 0.0f, 1.0f, 1.0f },
                        { 0.0f, 0.0f, 1.0f, 1.0f },
                        { lightRadius, 0.0f, 0.0f, 1.0f }
                }
        };
};

static GlobalState_Scene0 gState_scene0;


// Entry-Point Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
        // Local Variable Declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[256];
	int iResult = 0;

	int SW = GetSystemMetrics(SM_CXSCREEN);
	int SH = GetSystemMetrics(SM_CYSCREEN);
	int xCoordinate = ((SW / 2) - (WIN_WIDTH / 2));
	int yCoordinate = ((SH / 2) - (WIN_HEIGHT / 2));

	BOOL bDone = FALSE;
	VkResult vkResult = VK_SUCCESS;

	// Code

	// Log File
	gState_scene0.gFILE = fopen(LOG_FILE, "w");
	if (!gState_scene0.gFILE)
	{
		MessageBox(NULL, TEXT("Program cannot open log file!"), TEXT("Error"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gState_scene0.gFILE, "WinMain()-> Program started successfully\n");
	}
	
	wsprintf(szAppName, TEXT("%s"), gState_scene0.window.appName);

	// WNDCLASSEX Initilization 
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wndclass.cbClsExtra = 0;
        wndclass.cbWndExtra = 0;
        wndclass.lpfnWndProc = gAppFunctions_Scene0.wndProc;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// Register WNDCLASSEX
	RegisterClassEx(&wndclass);


	// Create Window								// glutCreateWindow
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,			// to above of taskbar for fullscreen
						szAppName,
						TEXT("05_PhysicalDevice"),
						WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
						xCoordinate,				// glutWindowPosition 1st Parameter
						yCoordinate,				// glutWindowPosition 2nd Parameter
						WIN_WIDTH,					// glutWindowSize 1st Parameter
						WIN_HEIGHT,					// glutWindowSize 2nd Parameter
						NULL,
						NULL,
						hInstance,
						NULL);

	gState_scene0.window.hwnd = hwnd;

        // Initialization
        vkResult = gAppFunctions_Scene0.initialize();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "WinMain(): initialize()  function failed\n");
		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "WinMain(): initialize() succedded\n");
	}

	// Show The Window
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game Loop
	while (bDone == FALSE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gState_scene0.window.isActive == TRUE)
			{
				if(gState_scene0.window.isMinimized == FALSE)
				{
                                        vkResult = gAppFunctions_Scene0.display();
					if ((vkResult != VK_FALSE) && (vkResult != VK_SUCCESS) && (vkResult != VK_ERROR_OUT_OF_DATE_KHR) && ((vkResult != VK_SUBOPTIMAL_KHR))) //VK_ERROR_OUT_OF_DATE_KHR and VK_SUBOPTIMAL_KHR are meant for future issues.You can remove them.
					{
						fprintf(gState_scene0.gFILE, "WinMain(): display() function failed\n");
						bDone = TRUE;
					}
					
                                        if(gState_scene0.window.isActive == TRUE)
                                        {
                                                gAppFunctions_Scene0.update();
                                        }
				}
			}
		}
	}

        // Uninitialization
        gAppFunctions_Scene0.uninitialize();

	return((int)msg.wParam);
}

// CALLBACK Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
        //Variable Declarations
        VkResult vkResult;

	// Code
	switch (iMsg)
	{
		case WM_CREATE:
			memset((void*)&gState_scene0.window.previousPlacement, 0 , sizeof(WINDOWPLACEMENT));
			gState_scene0.window.previousPlacement.length = sizeof(WINDOWPLACEMENT);
		break;
		
		case WM_SETFOCUS:
			gState_scene0.window.isActive = TRUE;
			break;

		case WM_KILLFOCUS:
			gState_scene0.window.isActive = FALSE;
			break;

		case WM_SIZE:
			if(wParam == SIZE_MINIMIZED)
			{
				gState_scene0.window.isMinimized = TRUE;
			}
			else
			{
                                gState_scene0.window.isMinimized = FALSE; //Any sequence is OK
                                vkResult = gAppFunctions_Scene0.resize(LOWORD(lParam), HIWORD(lParam)); //No need of error checking
				if (vkResult != VK_SUCCESS)
				{
					fprintf(gState_scene0.gFILE, "WndProc(): resize() function failed with error code %d\n", vkResult);
					return vkResult;
				}
				else
				{
					fprintf(gState_scene0.gFILE, "WndProc(): resize() succedded\n");
				}
			}
			break;

		/*
		case WM_ERASEBKGND:
			return(0);
		*/

		case WM_KEYDOWN:
			switch (LOWORD(wParam))
			{
			case VK_ESCAPE:
				fprintf(gState_scene0.gFILE, "WndProc() VK_ESCAPE-> Program ended successfully.\n");
				fclose(gState_scene0.gFILE);
				gState_scene0.gFILE = NULL;
				DestroyWindow(hwnd);
				break;
			}
			break;

		case WM_CHAR:
			switch (LOWORD(wParam))
			{
			case 'F':
			case 'f':
				if (gState_scene0.window.isFullscreen == FALSE)
				{
                                        gAppFunctions_Scene0.toggleFullscreen();
					gState_scene0.window.isFullscreen = TRUE;
					fprintf(gState_scene0.gFILE, "WndProc() WM_CHAR(F key)-> Program entered Fullscreen.\n");
				}
				else
				{
                                        gAppFunctions_Scene0.toggleFullscreen();
					gState_scene0.window.isFullscreen = FALSE;
					fprintf(gState_scene0.gFILE, "WndProc() WM_CHAR(F key)-> Program ended Fullscreen.\n");
				}
				break;
				
			case 'L':
			case 'l':
				if (gState_scene0.bLight == FALSE)
				{
					gState_scene0.bLight = TRUE;
					fprintf(gState_scene0.gFILE, "WndProc() WM_CHAR(L key)-> gState_scene0.bLight = TRUE\n");
				}
				else
				{
					gState_scene0.bLight = FALSE;
					fprintf(gState_scene0.gFILE, "WndProc() WM_CHAR(L key)-> gState_scene0.bLight = FALSE\n");
				}
				break;
				
			default:
				break;
			}
			break;

		case WM_RBUTTONDOWN:								
			DestroyWindow(hwnd);
			break;

                case WM_CLOSE:
                        gAppFunctions_Scene0.uninitialize();
			DestroyWindow(hwnd);
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		
		default:
			break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void ToggleFullscreen(void)
{
	// Local Variable Declarations
	MONITORINFO mi = { sizeof(MONITORINFO) };

	// Code
	if (gState_scene0.window.isFullscreen == FALSE)
	{
		gState_scene0.window.style = GetWindowLong(gState_scene0.window.hwnd, GWL_STYLE);

		if (gState_scene0.window.style & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(gState_scene0.window.hwnd, &gState_scene0.window.previousPlacement) && GetMonitorInfo(MonitorFromWindow(gState_scene0.window.hwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(gState_scene0.window.hwnd, GWL_STYLE, gState_scene0.window.style & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(gState_scene0.window.hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOZORDER | SWP_FRAMECHANGED);
				// HWND_TOP ~ WS_OVERLAPPED, rc ~ RECT, SWP_FRAMECHANGED ~ WM_NCCALCSIZE msg
			}
		}

		ShowCursor(FALSE);
	}
	else {
		SetWindowPlacement(gState_scene0.window.hwnd, &gState_scene0.window.previousPlacement);
		SetWindowLong(gState_scene0.window.hwnd, GWL_STYLE, gState_scene0.window.style | WS_OVERLAPPEDWINDOW);
		SetWindowPos(gState_scene0.window.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		// SetWindowPos has greater priority than SetWindowPlacement and SetWindowStyle for Z-Order
		ShowCursor(TRUE);
	}
}

VkResult initialize(void)
{
        //Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	// Code
        vkResult = gVulkanFunctions_Scene0.createVulkanInstance();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateVulkanInstance() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateVulkanInstance() succedded\n");
	}
	
	//Create Vulkan Presentation Surface
        vkResult = gVulkanFunctions_Scene0.getSupportedSurface();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): GetSupportedSurface() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): GetSupportedSurface() succedded\n");
	}
	
	//Enumerate and select physical device and its queque family index
        vkResult = gVulkanFunctions_Scene0.getPhysicalDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): GetPhysicalDevice() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): GetPhysicalDevice() succedded\n");
	}
	
	//Print Vulkan Info ;
        vkResult = gVulkanFunctions_Scene0.printVulkanInfo();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): PrintVulkanInfo() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): PrintVulkanInfo() succedded\n");
	}
	
	//Create Vulkan Device (Logical Device)
        vkResult = gVulkanFunctions_Scene0.createVulKanDevice();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateVulKanDevice() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateVulKanDevice() succedded\n");
	}
	
	//get Device Queque
        gVulkanFunctions_Scene0.getDeviceQueque();
	
        vkResult = gVulkanFunctions_Scene0.createSwapChain(VK_FALSE); //https://registry.khronos.org/vulkan/specs/latest/man/html/VK_FALSE.html
	if (vkResult != VK_SUCCESS)
	{
		/*
		Why are we giving hardcoded error when returbn value is vkResult?
		Answer sir will give in swapchain
		*/
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "initialize(): CreateSwapChain() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateSwapChain() succedded\n");
	}
	
	//1. Get Swapchain image count in a global variable using vkGetSwapchainImagesKHR() API (https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetSwapchainImagesKHR.html).
	//Create Vulkan images and image views
        vkResult =  gVulkanFunctions_Scene0.createImagesAndImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateImagesAndImageViews() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateImagesAndImageViews() succedded with SwapChain Image count as %d\n", gState_scene0.swapchainImageCount);
	}
	
        vkResult = gVulkanFunctions_Scene0.createCommandPool();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateCommandPool() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateCommandPool() succedded\n");
	}
	
        vkResult  = gVulkanFunctions_Scene0.createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateCommandBuffers() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateCommandBuffers() succedded\n");
	}
	
	getSphereVertexData(gState_scene0.sphere_vertices, gState_scene0.sphere_normals, gState_scene0.sphere_textures, gState_scene0.sphere_elements);
    gState_scene0.numVertices = getNumberOfSphereVertices();
    gState_scene0.numElements = getNumberOfSphereElements();
	
	/*
	22.2. Declare User defined function CreateVertexBuffer().
	Write its prototype below CreateCommandBuffers() and above CreateRenderPass() and also call it between the calls of these two.
	*/
        vkResult  = gVulkanFunctions_Scene0.createVertexBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateVertexBuffer() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateVertexBuffer() succedded\n");
	}
	
        vkResult  = gVulkanFunctions_Scene0.createIndexBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateIndexBuffer() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateIndexBuffer() succedded\n");
	}
	
	/*
	31.3 CreateUniformBuffer()
	*/
        vkResult  = gVulkanFunctions_Scene0.createUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateUniformBuffer() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateUniformBuffer() succedded\n");
	}
	
	/*
	23.4. Using same above convention, call CreateShaders() between calls of above two.
	*/
        vkResult = gVulkanFunctions_Scene0.createShaders();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateShaders() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateShaders() succedded\n");
	}
	
	/*
	24.2. In initialize(), declare and call UDF CreateDescriptorSetLayout() maintaining the convention of declaring and calling it after CreateShaders() and before CreateRenderPass().
	*/
        vkResult = gVulkanFunctions_Scene0.createDescriptorSetLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateDescriptorSetLayout() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateDescriptorSetLayout() succedded\n");
	}
	
        vkResult = gVulkanFunctions_Scene0.createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreatePipelineLayout() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreatePipelineLayout() succedded\n");
	}
	
	//31.4 CreateDescriptorPool
        vkResult = gVulkanFunctions_Scene0.createDescriptorPool();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateDescriptorPool() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateDescriptorPool() succedded\n");
	}
	
	//31.5 CreateDescriptorSet
        vkResult = gVulkanFunctions_Scene0.createDescriptorSet();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateDescriptorSet() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateDescriptorSet() succedded\n");
	}
	
        vkResult =  gVulkanFunctions_Scene0.createRenderPass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateRenderPass() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateRenderPass() succedded\n");
	}
	
        vkResult = gVulkanFunctions_Scene0.createPipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreatePipeline() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreatePipeline() succedded\n");
	}
		
        vkResult = gVulkanFunctions_Scene0.createFramebuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateFramebuffers() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateFramebuffers() succedded\n");
	}
	
        vkResult = gVulkanFunctions_Scene0.createSemaphores();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateSemaphores() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateSemaphores() succedded\n");
	}
	
        vkResult = gVulkanFunctions_Scene0.createFences();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateFences() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): CreateFences() succedded\n");
	}
	
	/*
	Initialize Clear Color values
	*/
	memset((void*)&gState_scene0.vkClearColorValue, 0, sizeof(VkClearColorValue));
	//Following step is analogus to glClearColor. This is more analogus to DirectX 11.
	gState_scene0.vkClearColorValue.float32[0] = 0.0f;
	gState_scene0.vkClearColorValue.float32[1] = 0.0f;
	gState_scene0.vkClearColorValue.float32[2] = 0.0f;
	gState_scene0.vkClearColorValue.float32[3] = 1.0f;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkClearDepthStencilValue.html
	memset((void*)&gState_scene0.vkClearDepthStencilValue, 0, sizeof(VkClearDepthStencilValue));
	//Set default clear depth value
	gState_scene0.vkClearDepthStencilValue.depth = 1.0f; //type float
	//Set default clear stencil value
	gState_scene0.vkClearDepthStencilValue.stencil = 0; //type uint32_t
	
        vkResult = gVulkanFunctions_Scene0.buildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "initialize(): buildCommandBuffers() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "initialize(): buildCommandBuffers() succedded\n");
	}
	
	/*
	Initialization is completed here..........................
	*/
	gState_scene0.bInitialized = TRUE;
	
	fprintf(gState_scene0.gFILE, "initialize(): initialize() completed sucessfully");
	
	return vkResult;
}

VkResult resize(int width, int height)
{
        //Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	// Code
	if(height <= 0)
	{
		height = 1;
	}
	
	//30.1
	//Check the gState_scene0.bInitialized variable
	if(gState_scene0.bInitialized == FALSE)
	{
		//throw error
		fprintf(gState_scene0.gFILE, "resize(): initialization yet not completed or failed\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	
	//30.2 
	//As recreation of swapchain is needed, we are going to repeat many steps of initialize() again.
	//Hence set gState_scene0.bInitialized = FALSE again.
	gState_scene0.bInitialized = FALSE;
	
	/*
	call can go to display() and code for resize() here
	*/
	
	//30.4 
	//Set global WIN_WIDTH and WIN_HEIGHT variables
	gState_scene0.winWidth = width;
	gState_scene0.winHeight = height;
	
	//30.5
	//Wait for device to complete in-hand tasks
	if(gState_scene0.vkDevice)
	{
		vkDeviceWaitIdle(gState_scene0.vkDevice);
		fprintf(gState_scene0.gFILE, "resize(): vkDeviceWaitIdle() is done\n");
	}
	
	//Destroy and recreate Swapchain, Swapchain image and image views functions, Swapchain count functions, Renderpass, Framebuffer, Pipeline, Pipeline Layout, CommandBuffer
	
	//30.6
	//Check presence of swapchain
	if(gState_scene0.vkSwapchainKHR == VK_NULL_HANDLE)
	{
		fprintf(gState_scene0.gFILE, "resize(): gState_scene0.vkSwapchainKHR is already NULL, cannot proceed\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	
	//30.7
	//Destroy framebuffer: destroy framebuffers in a loop for gState_scene0.swapchainImageCount
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyFramebuffer.html
	for(uint32_t i =0; i < gState_scene0.swapchainImageCount; i++)
	{
		vkDestroyFramebuffer(gState_scene0.vkDevice, gState_scene0.vkFramebuffer_array[i], NULL);
		gState_scene0.vkFramebuffer_array[i] = NULL;
		fprintf(gState_scene0.gFILE, "resize(): vkDestroyFramebuffer() is done\n");
	}
	
	if(gState_scene0.vkFramebuffer_array)
	{
		free(gState_scene0.vkFramebuffer_array);
		gState_scene0.vkFramebuffer_array = NULL;
		fprintf(gState_scene0.gFILE, "resize(): gState_scene0.vkFramebuffer_array is freed\n");
	}
	
	//30.11
	//Destroy Commandbuffer: In unitialize(), free each command buffer by using vkFreeCommandBuffers()(https://registry.khronos.org/vulkan/specs/latest/man/html/vkFreeCommandBuffers.html) in a loop of size swapchainImage count.
	for(uint32_t i =0; i < gState_scene0.swapchainImageCount; i++)
	{
		vkFreeCommandBuffers(gState_scene0.vkDevice, gState_scene0.vkCommandPool, 1, &gState_scene0.vkCommandBuffer_array[i]);
		fprintf(gState_scene0.gFILE, "resize(): vkFreeCommandBuffers() is done\n");
	}
			
	//Free actual command buffer array.
	if(gState_scene0.vkCommandBuffer_array)
	{
		free(gState_scene0.vkCommandBuffer_array);
		gState_scene0.vkCommandBuffer_array = NULL;
		fprintf(gState_scene0.gFILE, "resize(): gState_scene0.vkCommandBuffer_array is freed\n");
	}
	
	//30.9
	//Destroy Pipeline
	if(gState_scene0.vkPipeline)
	{
		vkDestroyPipeline(gState_scene0.vkDevice, gState_scene0.vkPipeline, NULL);
		gState_scene0.vkPipeline = VK_NULL_HANDLE;
		fprintf(gState_scene0.gFILE, "resize(): gState_scene0.vkPipeline is freed\n");
	}
	
	//30.10
	//Destroy PipelineLayout
	if(gState_scene0.vkPipelineLayout)
	{
		vkDestroyPipelineLayout(gState_scene0.vkDevice, gState_scene0.vkPipelineLayout, NULL);
		gState_scene0.vkPipelineLayout = VK_NULL_HANDLE;
		fprintf(gState_scene0.gFILE, "resize(): gState_scene0.vkPipelineLayout is freed\n");
	}
	
	//30.8
	//Destroy Renderpass : In uninitialize , destroy the renderpass by 
	//using vkDestrorRenderPass() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyRenderPass.html).
	if(gState_scene0.vkRenderPass)
	{
		vkDestroyRenderPass(gState_scene0.vkDevice, gState_scene0.vkRenderPass, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyRenderPass.html
		gState_scene0.vkRenderPass = VK_NULL_HANDLE;
		fprintf(gState_scene0.gFILE, "resize(): vkDestroyRenderPass() is done\n");
	}
	
	//destroy depth image view
	if(gState_scene0.vkImageView_depth)
	{
		vkDestroyImageView(gState_scene0.vkDevice, gState_scene0.vkImageView_depth, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyImageView.html
		gState_scene0.vkImageView_depth = VK_NULL_HANDLE;
	}
			
	//destroy device memory for depth image
	if(gState_scene0.vkDeviceMemory_depth)
	{
		vkFreeMemory(gState_scene0.vkDevice, gState_scene0.vkDeviceMemory_depth, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkFreeMemory.html
		gState_scene0.vkDeviceMemory_depth = VK_NULL_HANDLE;
	}
			
	//destroy depth image
	if(gState_scene0.vkImage_depth)
	{
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyImage.html
		vkDestroyImage(gState_scene0.vkDevice, gState_scene0.vkImage_depth, NULL);
		gState_scene0.vkImage_depth = VK_NULL_HANDLE;
	}
	
	//30.12
	//Destroy Swapchain image and image view: Keeping the "destructor logic aside" for a while , first destroy image views from imagesViews array in a loop using vkDestroyImageViews() api.
	//(https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyImageView.html)
	for(uint32_t i =0; i < gState_scene0.swapchainImageCount; i++)
	{
		vkDestroyImageView(gState_scene0.vkDevice, gState_scene0.swapChainImageView_array[i], NULL);
		fprintf(gState_scene0.gFILE, "resize(): vkDestroyImageView() is done\n");
	}
	
	//Now actually free imageView array using free().
	//free imageView array
	if(gState_scene0.swapChainImageView_array)
	{
		free(gState_scene0.swapChainImageView_array);
		gState_scene0.swapChainImageView_array = NULL;
		fprintf(gState_scene0.gFILE, "resize(): gState_scene0.swapChainImageView_array is freed\n");
	}
	
	//Now actually free swapchain image array using free().
	/*
	for(uint32_t i = 0; i < gState_scene0.swapchainImageCount; i++)
	{
		vkDestroyImage(gState_scene0.vkDevice, gState_scene0.swapChainImage_array[i], NULL);
		fprintf(gState_scene0.gFILE, "resize(): vkDestroyImage() is done\n");
	}
	*/
	
	if(gState_scene0.swapChainImage_array)
	{
		free(gState_scene0.swapChainImage_array);
		gState_scene0.swapChainImage_array = NULL;
		fprintf(gState_scene0.gFILE, "resize(): gState_scene0.swapChainImage_array is freed\n");
	}
	
	//30.13
	//Destroy swapchain : destroy it uninitilialize() by using vkDestroySwapchainKHR() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroySwapchainKHR.html) Vulkan API.
	vkDestroySwapchainKHR(gState_scene0.vkDevice, gState_scene0.vkSwapchainKHR, NULL);
	gState_scene0.vkSwapchainKHR = VK_NULL_HANDLE;
	fprintf(gState_scene0.gFILE, "resize(): vkDestroySwapchainKHR() is done\n");
	
	//RECREATE FOR RESIZE
	
	//30.14 Create Swapchain
        vkResult = gVulkanFunctions_Scene0.createSwapChain(VK_FALSE); //https://registry.khronos.org/vulkan/specs/latest/man/html/VK_FALSE.html
	if (vkResult != VK_SUCCESS)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "resize(): CreateSwapChain() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	//30.15 Create Swapchain image and Image Views
        vkResult =  gVulkanFunctions_Scene0.createImagesAndImageViews();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "resize(): CreateImagesAndImageViews() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	//30.18 Create renderPass
        vkResult =  gVulkanFunctions_Scene0.createRenderPass();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "resize(): CreateRenderPass() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	//30.16 Create PipelineLayout
        vkResult = gVulkanFunctions_Scene0.createPipelineLayout();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "resize(): CreatePipelineLayout() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	//30.17 Create Pipeline
        vkResult = gVulkanFunctions_Scene0.createPipeline();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "resize(): CreatePipeline() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	//30.19 Create framebuffers
        vkResult = gVulkanFunctions_Scene0.createFramebuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "resize(): CreateFramebuffers() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	
	//30.16 Create CommandBuffers
        vkResult  = gVulkanFunctions_Scene0.createCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "resize(): CreateCommandBuffers() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	//30.20 Build Commandbuffers
        vkResult = gVulkanFunctions_Scene0.buildCommandBuffers();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "resize(): buildCommandBuffers() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	//30.3
	//Do this
	gState_scene0.bInitialized = TRUE;
	
	return vkResult;
}

//31.12
VkResult UpdateUniformBuffer(void)
{
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	//Code
	GlobalState_Scene0::MyUniformData myUniformData;
	memset((void*)&myUniformData, 0, sizeof(GlobalState_Scene0::MyUniformData));
	
	//Update matrices
	myUniformData.modelMatrix = glm::mat4(1.0f);
	glm::mat4 translationMatrix = glm::mat4(1.0f);
	translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -2.0f));
	
	//glm::mat4 scaleMatrix = glm::mat4(1.0f);
	//scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.75f));
	//myUniformData.modelMatrix = translationMatrix * scaleMatrix;
	myUniformData.modelMatrix = translationMatrix;
	
	myUniformData.viewMatrix = glm::mat4(1.0f);
	myUniformData.projectionMatrix = glm::mat4(1.0f); //Not Required
	
	glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f);
	perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)gState_scene0.winWidth/(float)gState_scene0.winHeight, 0.1f, 100.0f);
	perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * (-1.0f); //2n/t-d member multiplied by -1 
	
	myUniformData.projectionMatrix = perspectiveProjectionMatrix;
	
        //Update lighting related uniform
        for(int i = 0; i < 3; i++)
        {
                for(int j = 0; j < 4; j++)
                {
                        myUniformData.lightAmbient[i][j] = gState_scene0.lights[i].ambient[j];
                        myUniformData.lightDiffuse[i][j] = gState_scene0.lights[i].diffuse[j];
                        myUniformData.lightSpecular[i][j] = gState_scene0.lights[i].specular[j];
                        myUniformData.lightPosition[i][j] = gState_scene0.lights[i].position[j];
                }
        }

        //Update material related uniform
        myUniformData.materialAmbient[0] = 0.0f;
        myUniformData.materialAmbient[1] = 0.0f;
        myUniformData.materialAmbient[2] = 0.0f;
        myUniformData.materialAmbient[3] = 1.0f;

        myUniformData.materialDiffuse[0] = 1.0f;
        myUniformData.materialDiffuse[1] = 1.0f;
        myUniformData.materialDiffuse[2] = 1.0f;
        myUniformData.materialDiffuse[3] = 1.0f;

        myUniformData.materialSpecular[0] = 1.0f;
        myUniformData.materialSpecular[1] = 1.0f;
        myUniformData.materialSpecular[2] = 1.0f;
        myUniformData.materialSpecular[3] = 1.0f;

        myUniformData.materialShininess = 128.0f;
	
	//Update key press related uniform
	if(gState_scene0.bLight == TRUE)
	{
		myUniformData.lKeyIsPressed = 1;
	}
	else
	{
		myUniformData.lKeyIsPressed = 0;
	}
	
	//Map Uniform Buffer
	/*
	This will allow us to do memory mapped IO means when we write on void* buffer data, 
	it will get automatically written/copied on to device memory represented by device memory object handle.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkMapMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData);
	*/
	void* data = NULL;
	vkResult = vkMapMemory(gState_scene0.vkDevice, gState_scene0.uniformData.vkDeviceMemory, 0, sizeof(GlobalState_Scene0::MyUniformData), 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "UpdateUniformBuffer(): vkMapMemory() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	//Copy the data to the mapped buffer
	/*
	31.12. Now to do actual memory mapped IO, call memcpy.
	*/
	memcpy(data, &myUniformData, sizeof(GlobalState_Scene0::MyUniformData));
	
	/*
	31.12. To complete this memory mapped IO. finally call vkUmmapMemory() API.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkUnmapMemory.html
	// Provided by VK_VERSION_1_0
	void vkUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory);
	*/
	vkUnmapMemory(gState_scene0.vkDevice, gState_scene0.uniformData.vkDeviceMemory);
	
	return vkResult;
}

VkResult display(void)
{
        //Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	// Code
	
	// If control comes here , before initialization is completed , return false
	if(gState_scene0.bInitialized == FALSE)
	{
		fprintf(gState_scene0.gFILE, "display(): initialization not completed yet\n");
		return (VkResult)VK_FALSE;
	}
	
	// Acquire index of next swapChainImage
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkAcquireNextImageKHR.html
	/*
	// Provided by VK_KHR_swapchain
	VkResult vkAcquireNextImageKHR(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint64_t                                    timeout, // Waiting time from our side for swapchain to give the image for device. (Time in nanoseconds)
    VkSemaphore                                 semaphore, // Waiting for another queque to release the image held by another queque demanded by swapchain
    VkFence                                     fence, // ask host to wait image to be given by swapchain
    uint32_t*                                   pImageIndex);
	
	If this function  will not get image index from swapchain within gven time or timeout, then vkAcquireNextImageKHR() will return VK_NOT_READY
	4th paramater is waiting for another queque to release the image held by another queque demanded by swapchain
	*/
	vkResult = vkAcquireNextImageKHR(gState_scene0.vkDevice, gState_scene0.vkSwapchainKHR, UINT64_MAX, gState_scene0.vkSemaphore_BackBuffer, VK_NULL_HANDLE, &gState_scene0.currentImageIndex);
	if(vkResult != VK_SUCCESS) 
	{
                if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
                {
                        gAppFunctions_Scene0.resize(gState_scene0.winWidth, gState_scene0.winHeight);
		}
		else
		{
			fprintf(gState_scene0.gFILE, "display(): vkAcquireNextImageKHR() failed\n");
			return vkResult;
		}
	}
	
	/*
	Use fence to allow host to wait for completion of execution of previous commandbuffer.
	Magacha command buffer cha operation complete vhava mhanun vaprat aahe he fence
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkWaitForFences.html
	/*
	// Provided by VK_VERSION_1_0
	VkResult vkWaitForFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences,
    VkBool32                                    waitAll,
    uint64_t                                    timeout);
	*/
	vkResult = vkWaitForFences(gState_scene0.vkDevice, 1, &gState_scene0.vkFence_array[gState_scene0.currentImageIndex], VK_TRUE, UINT64_MAX);
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "display(): vkWaitForFences() failed\n");
		return vkResult;
	}
	
	//Now ready the fences for next commandbuffer.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkResetFences.html
	/*
	// Provided by VK_VERSION_1_0
	VkResult vkResetFences(
    VkDevice                                    device,
    uint32_t                                    fenceCount,
    const VkFence*                              pFences);
	*/
	vkResult = vkResetFences(gState_scene0.vkDevice, 1, &gState_scene0.vkFence_array[gState_scene0.currentImageIndex]);
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "display(): vkResetFences() failed\n");
		return vkResult;
	}
	
	//One of the memebers of VkSubmitInfo structure requires array of pipeline stages. We have only one of completion of color attachment output.
	//Still we need 1 member array.
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineStageFlags.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineStageFlagBits.html
	const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		
	// https://registry.khronos.org/vulkan/specs/latest/man/html/VkSubmitInfo.html
	// Declare, memset and initialize VkSubmitInfo structure
	VkSubmitInfo vkSubmitInfo;
	memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
	vkSubmitInfo.waitSemaphoreCount = 1;
	vkSubmitInfo.pWaitSemaphores = &gState_scene0.vkSemaphore_BackBuffer;
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &gState_scene0.vkCommandBuffer_array[gState_scene0.currentImageIndex];
	vkSubmitInfo.signalSemaphoreCount = 1;
	vkSubmitInfo.pSignalSemaphores = &gState_scene0.vkSemaphore_RenderComplete;
	
	//Now submit above work to the queque
	vkResult = vkQueueSubmit(gState_scene0.vkQueue, 1, &vkSubmitInfo, gState_scene0.vkFence_array[gState_scene0.currentImageIndex]); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkQueueSubmit.html
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "display(): vkQueueSubmit() failed\n");
		return vkResult;
	}
	
	//We are going to present the rendered image after declaring  and initializing VkPresentInfoKHR struct
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPresentInfoKHR.html
	VkPresentInfoKHR  vkPresentInfoKHR;
	memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
	vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	vkPresentInfoKHR.pNext = NULL;
	vkPresentInfoKHR.swapchainCount = 1;
	vkPresentInfoKHR.pSwapchains = &gState_scene0.vkSwapchainKHR;
	vkPresentInfoKHR.pImageIndices = &gState_scene0.currentImageIndex;
	vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &gState_scene0.vkSemaphore_RenderComplete;
	vkPresentInfoKHR.pResults = NULL;
	
	//Present the queque
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkQueuePresentKHR.html
	vkResult =  vkQueuePresentKHR(gState_scene0.vkQueue, &vkPresentInfoKHR);
	if(vkResult != VK_SUCCESS)
	{
                if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
                {
                        gAppFunctions_Scene0.resize(gState_scene0.winWidth, gState_scene0.winHeight);
		}
		else
		{
			fprintf(gState_scene0.gFILE, "display(): vkQueuePresentKHR() failed\n");
			return vkResult;
		}
	}
	
	//31.7
        vkResult = gVulkanFunctions_Scene0.updateUniformBuffer();
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "display(): updateUniformBuffer() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	
	vkDeviceWaitIdle(gState_scene0.vkDevice);
	return vkResult;
}

void update(void)
{
        // Code
        float radians = glm::radians(gState_scene0.lightAngle);
        float cosA = cosf(radians);
        float sinA = sinf(radians);

        gState_scene0.lights[0].position[0] = 0.0f;
        gState_scene0.lights[0].position[1] = GlobalState_Scene0::lightRadius * cosA;
        gState_scene0.lights[0].position[2] = GlobalState_Scene0::lightRadius * sinA;
        gState_scene0.lights[0].position[3] = 1.0f;

        gState_scene0.lights[1].position[0] = GlobalState_Scene0::lightRadius * cosA;
        gState_scene0.lights[1].position[1] = 0.0f;
        gState_scene0.lights[1].position[2] = GlobalState_Scene0::lightRadius * sinA;
        gState_scene0.lights[1].position[3] = 1.0f;

        gState_scene0.lights[2].position[0] = GlobalState_Scene0::lightRadius * cosA;
        gState_scene0.lights[2].position[1] = GlobalState_Scene0::lightRadius * sinA;
        gState_scene0.lights[2].position[2] = 0.0f;
        gState_scene0.lights[2].position[3] = 1.0f;

        gState_scene0.lightAngle += 1.0f;
        if(gState_scene0.lightAngle >= 360.0f)
        {
                gState_scene0.lightAngle -= 360.0f;
        }
}

/*
void uninitialize(void)
{
                if (gState_scene0.window.isFullscreen == TRUE)
                {
                        gAppFunctions_Scene0.toggleFullscreen();
                        gState_scene0.window.isFullscreen = FALSE;
                }

		// Destroy Window
		if (gState_scene0.window.hwnd)
		{
			DestroyWindow(gState_scene0.window.hwnd);
			gState_scene0.window.hwnd = NULL;
		}
		
		
		//10. When done destroy it uninitilialize() by using vkDestroySwapchainKHR() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroySwapchainKHR.html) Vulkan API.
		//Destroy swapchain
		vkDestroySwapchainKHR(gState_scene0.vkDevice, gState_scene0.vkSwapchainKHR, NULL);
		gState_scene0.vkSwapchainKHR = VK_NULL_HANDLE;
		fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroySwapchainKHR() is done\n");
		
		//Destroy Vulkan device
		
		//No need to destroy/uninitialize device queque
		
		//No need to destroy selected physical device
		if(gState_scene0.vkDevice)
		{
			vkDeviceWaitIdle(gState_scene0.vkDevice); //First synchronization function
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDeviceWaitIdle() is done\n");
			vkDestroyDevice(gState_scene0.vkDevice, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyDevice.html
			gState_scene0.vkDevice = VK_NULL_HANDLE;
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyDevice() is done\n");
		}
		
		if(gState_scene0.vkSurfaceKHR)
		{
			// The destroy() of vkDestroySurfaceKHR() generic not platform specific
			vkDestroySurfaceKHR(gState_scene0.vkInstance, gState_scene0.vkSurfaceKHR, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroySurfaceKHR.html
			gState_scene0.vkSurfaceKHR = VK_NULL_HANDLE;
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroySurfaceKHR() sucedded\n");
		}

		// Destroy VkInstance in uninitialize()
		if(gState_scene0.vkInstance)
		{
			vkDestroyInstance(gState_scene0.vkInstance, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyInstance.html
			gState_scene0.vkInstance = VK_NULL_HANDLE;
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyInstance() sucedded\n");
		}

		// Close the log file
		if (gState_scene0.gFILE)
		{
			fprintf(gState_scene0.gFILE, "uninitialize()-> Program ended successfully.\n");
			fclose(gState_scene0.gFILE);
			gState_scene0.gFILE = NULL;
		}

}
*/

void uninitialize(void)
{
                if (gState_scene0.window.isFullscreen == TRUE)
                {
                        gAppFunctions_Scene0.toggleFullscreen();
                        gState_scene0.window.isFullscreen = FALSE;
                }

		// Destroy Window
		if (gState_scene0.window.hwnd)
		{
			DestroyWindow(gState_scene0.window.hwnd);
			gState_scene0.window.hwnd = NULL;
		}
		
		//Destroy Vulkan device
		if(gState_scene0.vkDevice)
		{
			vkDeviceWaitIdle(gState_scene0.vkDevice); //First synchronization function
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDeviceWaitIdle() is done\n");
			
			/*
			18_7. In uninitialize(), first in a loop with swapchain image count as counter, destroy frnce array objects using vkDestroyFence() {https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyFence.html} and then
				actually free allocated fences array by using free().
			*/
			//Destroying fences
			for(uint32_t i = 0; i< gState_scene0.swapchainImageCount; i++)
			{
				vkDestroyFence(gState_scene0.vkDevice, gState_scene0.vkFence_array[i], NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyFence.html
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkFence_array[%d] is freed\n", i);
			}
			
			if(gState_scene0.vkFence_array)
			{
				free(gState_scene0.vkFence_array);
				gState_scene0.vkFence_array = NULL;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkFence_array is freed\n");
			}
			
			/*
			18_8. Destroy both global semaphore objects  with two separate calls to vkDestroySemaphore() {https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroySemaphore.html}.
			*/
			//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroySemaphore.html
			if(gState_scene0.vkSemaphore_RenderComplete)
			{
				vkDestroySemaphore(gState_scene0.vkDevice, gState_scene0.vkSemaphore_RenderComplete, NULL);
				gState_scene0.vkSemaphore_RenderComplete = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkSemaphore_RenderComplete is freed\n");
			}
			
			if(gState_scene0.vkSemaphore_BackBuffer)
			{
				vkDestroySemaphore(gState_scene0.vkDevice, gState_scene0.vkSemaphore_BackBuffer, NULL);
				gState_scene0.vkSemaphore_RenderComplete = VK_NULL_HANDLE;
					fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkSemaphore_BackBuffer is freed\n");
			}
			
			/*
			Step_17_3. In unitialize destroy framebuffers in a loop for gState_scene0.swapchainImageCount.
			https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyFramebuffer.html
			*/
			for(uint32_t i =0; i < gState_scene0.swapchainImageCount; i++)
			{
				vkDestroyFramebuffer(gState_scene0.vkDevice, gState_scene0.vkFramebuffer_array[i], NULL);
				gState_scene0.vkFramebuffer_array[i] = NULL;
				fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyFramebuffer() is done\n");
			}
			
			if(gState_scene0.vkFramebuffer_array)
			{
				free(gState_scene0.vkFramebuffer_array);
				gState_scene0.vkFramebuffer_array = NULL;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkFramebuffer_array is freed\n");
			}
			
			if(gState_scene0.vkPipeline)
			{
				vkDestroyPipeline(gState_scene0.vkDevice, gState_scene0.vkPipeline, NULL);
				gState_scene0.vkPipeline = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkPipeline is freed\n");
			}
			
			/*
			24.5. In uninitialize, call vkDestroyDescriptorSetlayout() Vulkan API to destroy this Vulkan object.
			//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyDescriptorSetLayout.html
			// Provided by VK_VERSION_1_0
			void vkDestroyDescriptorSetLayout(
			VkDevice                                    device,
			VkDescriptorSetLayout                       descriptorSetLayout,
			const VkAllocationCallbacks*                pAllocator);
			*/
			if(gState_scene0.vkDescriptorSetLayout)
			{
				vkDestroyDescriptorSetLayout(gState_scene0.vkDevice, gState_scene0.vkDescriptorSetLayout, NULL);
				gState_scene0.vkDescriptorSetLayout = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkDescriptorSetLayout is freed\n");
			}
			
			/*
			25.5. In uninitialize, call vkDestroyPipelineLayout() Vulkan API to destroy this gState_scene0.vkPipelineLayout Vulkan object.
			//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyPipelineLayout.html
			// Provided by VK_VERSION_1_0
			void vkDestroyPipelineLayout(
				VkDevice                                    device,
				VkPipelineLayout                            pipelineLayout,
				const VkAllocationCallbacks*                pAllocator);
			*/
			if(gState_scene0.vkPipelineLayout)
			{
				vkDestroyPipelineLayout(gState_scene0.vkDevice, gState_scene0.vkPipelineLayout, NULL);
				gState_scene0.vkPipelineLayout = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkPipelineLayout is freed\n");
			}
			
			//Step_16_6. In uninitialize , destroy the renderpass by using vkDestrorRenderPass() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyRenderPass.html).
			if(gState_scene0.vkRenderPass)
			{
				vkDestroyRenderPass(gState_scene0.vkDevice, gState_scene0.vkRenderPass, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyRenderPass.html
				gState_scene0.vkRenderPass = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyRenderPass() is done\n");
			}
			
			//31.8 Destroy descriptorpool (When descriptor pool is destroyed, descriptor sets created by that pool are also destroyed implicitly)
			if(gState_scene0.vkDescriptorPool)
			{
				//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyDescriptorPool.html
				vkDestroyDescriptorPool(gState_scene0.vkDevice, gState_scene0.vkDescriptorPool, NULL);
				gState_scene0.vkDescriptorPool = VK_NULL_HANDLE;
				gState_scene0.vkDescriptorSet = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyDescriptorPool() is done for gState_scene0.vkDescriptorPool and gState_scene0.vkDescriptorSet both\n");
			}
			
			/*
			23.11. In uninitialize , destroy both global shader objects using vkDestroyShaderModule() Vulkan API.
			//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyShaderModule.html
			// Provided by VK_VERSION_1_0
			void vkDestroyShaderModule(
			VkDevice device,
			VkShaderModule shaderModule,
			const VkAllocationCallbacks* pAllocator);
			*/
			if(gState_scene0.vkShaderMoudule_fragment_shader)
			{
				vkDestroyShaderModule(gState_scene0.vkDevice, gState_scene0.vkShaderMoudule_fragment_shader, NULL);
				gState_scene0.vkShaderMoudule_fragment_shader = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): VkShaderMoudule for fragment shader is done\n");
			}
			
			if(gState_scene0.vkShaderMoudule_vertex_shader)
			{
				vkDestroyShaderModule(gState_scene0.vkDevice, gState_scene0.vkShaderMoudule_vertex_shader, NULL);
				gState_scene0.vkShaderMoudule_vertex_shader = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): VkShaderMoudule for vertex shader is done\n");
			}
			
			//31.9 Destroy uniform buffer
			if(gState_scene0.uniformData.vkBuffer)
			{
				vkDestroyBuffer(gState_scene0.vkDevice, gState_scene0.uniformData.vkBuffer, NULL);
				gState_scene0.uniformData.vkBuffer = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.uniformData.vkBuffer is freed\n");
			}
			
			if(gState_scene0.uniformData.vkDeviceMemory)
			{
				vkFreeMemory(gState_scene0.vkDevice, gState_scene0.uniformData.vkDeviceMemory, NULL);
				gState_scene0.uniformData.vkDeviceMemory = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.uniformData.vkDeviceMemory is freed\n");
			}
			
			/*
			22.14. In uninitialize()
			First Free the ".vkDeviceMemory" memory of our global structure using vkFreeMemory() and then destroy ".vkBuffer" member of our global structure by using vkDestroyBuffer().
			
			//https://registry.khronos.org/vulkan/specs/latest/man/html/vkFreeMemory.html
			// Provided by VK_VERSION_1_0
			void vkFreeMemory(
				VkDevice                                    device,
				VkDeviceMemory                              memory,
				const VkAllocationCallbacks*                pAllocator);
				
			https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyBuffer.html
			// Provided by VK_VERSION_1_0
			void vkDestroyBuffer(
				VkDevice                                    device,
				VkBuffer                                    buffer,
				const VkAllocationCallbacks*                pAllocator);
			*/
			if(gState_scene0.vertexdata_index.vkDeviceMemory)
			{
				vkFreeMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_index.vkDeviceMemory, NULL);
				gState_scene0.vertexdata_index.vkDeviceMemory = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vertexdata_index.vkDeviceMemory is freed\n");
			}
			
			if(gState_scene0.vertexdata_index.vkBuffer)
			{
				vkDestroyBuffer(gState_scene0.vkDevice, gState_scene0.vertexdata_index.vkBuffer, NULL);
				gState_scene0.vertexdata_index.vkBuffer = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vertexdata_index.vkBuffer is freed\n");
			}
			
			if(gState_scene0.vertexdata_texcoord.vkDeviceMemory)
			{
				vkFreeMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_texcoord.vkDeviceMemory, NULL);
				gState_scene0.vertexdata_texcoord.vkDeviceMemory = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vertexdata_texcoord.vkDeviceMemory is freed\n");
			}
			
			if(gState_scene0.vertexdata_texcoord.vkBuffer)
			{
				vkDestroyBuffer(gState_scene0.vkDevice, gState_scene0.vertexdata_texcoord.vkBuffer, NULL);
				gState_scene0.vertexdata_texcoord.vkBuffer = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vertexdata_texcoord.vkBuffer is freed\n");
			}
			
			if(gState_scene0.vertexdata_normals.vkDeviceMemory)
			{
				vkFreeMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_normals.vkDeviceMemory, NULL);
				gState_scene0.vertexdata_normals.vkDeviceMemory = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vertexdata_normals.vkDeviceMemory is freed\n");
			}
			
			if(gState_scene0.vertexdata_normals.vkBuffer)
			{
				vkDestroyBuffer(gState_scene0.vkDevice, gState_scene0.vertexdata_normals.vkBuffer, NULL);
				gState_scene0.vertexdata_normals.vkBuffer = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vertexdata_normals.vkBuffer is freed\n");
			}
			
			if(gState_scene0.vertexdata_position.vkDeviceMemory)
			{
				vkFreeMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_position.vkDeviceMemory, NULL);
				gState_scene0.vertexdata_position.vkDeviceMemory = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vertexdata_position.vkDeviceMemory is freed\n");
			}
			
			if(gState_scene0.vertexdata_position.vkBuffer)
			{
				vkDestroyBuffer(gState_scene0.vkDevice, gState_scene0.vertexdata_position.vkBuffer, NULL);
				gState_scene0.vertexdata_position.vkBuffer = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vertexdata_position.vkBuffer is freed\n");
			}
			
			//Step_15_4. In unitialize(), free each command buffer by using vkFreeCommandBuffers()(https://registry.khronos.org/vulkan/specs/latest/man/html/vkFreeCommandBuffers.html) in a loop of size swapchainImage count.
			for(uint32_t i =0; i < gState_scene0.swapchainImageCount; i++)
			{
				vkFreeCommandBuffers(gState_scene0.vkDevice, gState_scene0.vkCommandPool, 1, &gState_scene0.vkCommandBuffer_array[i]);
				fprintf(gState_scene0.gFILE, "uninitialize(): vkFreeCommandBuffers() is done\n");
			}
			
				//Step_15_5. Free actual command buffer array.
			if(gState_scene0.vkCommandBuffer_array)
			{
				free(gState_scene0.vkCommandBuffer_array);
				gState_scene0.vkCommandBuffer_array = NULL;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkCommandBuffer_array is freed\n");
			}	

			//Step_14_3 In uninitialize(), destroy commandpool using VkDestroyCommandPool.
			// https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyCommandPool.html
			if(gState_scene0.vkCommandPool)
			{
				vkDestroyCommandPool(gState_scene0.vkDevice, gState_scene0.vkCommandPool, NULL);
				gState_scene0.vkCommandPool = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyCommandPool() is done\n");
			}
			
			//destroy depth image view
			if(gState_scene0.vkImageView_depth)
			{
				vkDestroyImageView(gState_scene0.vkDevice, gState_scene0.vkImageView_depth, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyImageView.html
				gState_scene0.vkImageView_depth = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkImageView_depth is done\n");
			}
			
			//destroy device memory for depth image
			if(gState_scene0.vkDeviceMemory_depth)
			{
				vkFreeMemory(gState_scene0.vkDevice, gState_scene0.vkDeviceMemory_depth, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkFreeMemory.html
				gState_scene0.vkDeviceMemory_depth = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkDeviceMemory_depth is done\n");
			}
			
			//destroy depth image
			if(gState_scene0.vkImage_depth)
			{
				//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyImage.html
				vkDestroyImage(gState_scene0.vkDevice, gState_scene0.vkImage_depth, NULL);
				gState_scene0.vkImage_depth = VK_NULL_HANDLE;
				fprintf(gState_scene0.gFILE, "uninitialize(): gState_scene0.vkImage_depth is done\n");
			}
			
			/*
			9. In unitialize(), keeping the "destructor logic aside" for a while , first destroy image views from imagesViews array in a loop using vkDestroyImageViews() api.
			(https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyImageView.html)
			*/
			for(uint32_t i =0; i < gState_scene0.swapchainImageCount; i++)
			{
				vkDestroyImageView(gState_scene0.vkDevice, gState_scene0.swapChainImageView_array[i], NULL);
				fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyImageView() is done\n");
			}
			
			/*
			10. In uninitialize() , now actually free imageView array using free().
			free imageView array
			*/
			if(gState_scene0.swapChainImageView_array)
			{
				free(gState_scene0.swapChainImageView_array);
				gState_scene0.swapChainImageView_array = NULL;
				fprintf(gState_scene0.gFILE, "uninitialize():gState_scene0.swapChainImageView_array is freed\n");
			}
			
			/*
			7. In unitialize(), keeping the "destructor logic aside" for a while , first destroy swapchain images from swap chain images array in a loop using vkDestroyImage() api. 
			(https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyImage.html)
			//Free swap chain images
			*/
			/*
			for(uint32_t i = 0; i < gState_scene0.swapchainImageCount; i++)
			{
				vkDestroyImage(gState_scene0.vkDevice, gState_scene0.swapChainImage_array[i], NULL);
				fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyImage() is done\n");
			}
			*/
			
			/*
			8. In uninitialize() , now actually free swapchain image array using free().
			*/
			if(gState_scene0.swapChainImage_array)
			{
				free(gState_scene0.swapChainImage_array);
				gState_scene0.swapChainImage_array = NULL;
				fprintf(gState_scene0.gFILE, "uninitialize():gState_scene0.swapChainImage_array is freed\n");
			}
			
			/*
			10. When done destroy it uninitilialize() by using vkDestroySwapchainKHR() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroySwapchainKHR.html) Vulkan API.
			Destroy swapchain
			*/
			vkDestroySwapchainKHR(gState_scene0.vkDevice, gState_scene0.vkSwapchainKHR, NULL);
			gState_scene0.vkSwapchainKHR = VK_NULL_HANDLE;
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroySwapchainKHR() is done\n");
			
			
			vkDestroyDevice(gState_scene0.vkDevice, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyDevice.html
			gState_scene0.vkDevice = VK_NULL_HANDLE;
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyDevice() is done\n");
		}
		
		//No need to destroy/uninitialize device queque
		
		//No need to destroy selected physical device
		
		if(gState_scene0.vkSurfaceKHR)
		{
			/*
			The destroy() of vkDestroySurfaceKHR() generic not platform specific
			*/
			vkDestroySurfaceKHR(gState_scene0.vkInstance, gState_scene0.vkSurfaceKHR, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroySurfaceKHR.html
			gState_scene0.vkSurfaceKHR = VK_NULL_HANDLE;
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroySurfaceKHR() sucedded\n");
		}

		//21_Validation
		if(gState_scene0.vkDebugReportCallbackEXT && gState_scene0.vkDestroyDebugReportCallbackEXT_fnptr)
		{
			gState_scene0.vkDestroyDebugReportCallbackEXT_fnptr(gState_scene0.vkInstance, gState_scene0.vkDebugReportCallbackEXT, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyDebugReportCallbackEXT.html
			gState_scene0.vkDebugReportCallbackEXT = VK_NULL_HANDLE;
			gState_scene0.vkDestroyDebugReportCallbackEXT_fnptr = NULL; //Nahi kel tari chalel
		}

		/*
		Destroy VkInstance in uninitialize()
		*/
		if(gState_scene0.vkInstance)
		{
			vkDestroyInstance(gState_scene0.vkInstance, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyInstance.html
			gState_scene0.vkInstance = VK_NULL_HANDLE;
			fprintf(gState_scene0.gFILE, "uninitialize(): vkDestroyInstance() sucedded\n");
		}

		// Close the log file
		if (gState_scene0.gFILE)
		{
			fprintf(gState_scene0.gFILE, "uninitialize()-> Program ended successfully.\n");
			fclose(gState_scene0.gFILE);
			gState_scene0.gFILE = NULL;
		}
}

//Definition of Vulkan related functions

VkResult CreateVulkanInstance(void)
{
	/*
		As explained before fill and initialize required extension names and count in 2 respective global variables (Lasst 8 steps mhanje instance cha first step)
	*/
        //Variable declarations
        VkResult vkResult = VK_SUCCESS;

	// Code
        vkResult = gVulkanFunctions_Scene0.fillInstanceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): FillInstanceExtensionNames()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): FillInstanceExtensionNames() succedded\n");
	}
	
	//21_Validation
	if(gState_scene0.bValidation == TRUE)
	{
		//21_Validation
                vkResult = gVulkanFunctions_Scene0.fillValidationLayerNames();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): FillValidationLayerNames()  function failed\n");
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): FillValidationLayerNames() succedded\n");
		}
	}
	
	/*
	Initialize struct VkApplicationInfo (Somewhat limbu timbu)
	*/
	struct VkApplicationInfo vkApplicationInfo;
	memset((void*)&vkApplicationInfo, 0, sizeof(struct VkApplicationInfo)); //Dont use ZeroMemory to keep parity across all OS
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkApplicationInfo.html/
	vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //First member of all Vulkan structure, for genericness and typesafety
	vkApplicationInfo.pNext = NULL;
	vkApplicationInfo.pApplicationName = gState_scene0.window.appName; //any string will suffice
	vkApplicationInfo.applicationVersion = 1; //any number will suffice
	vkApplicationInfo.pEngineName = gState_scene0.window.appName; //any string will suffice
	vkApplicationInfo.engineVersion = 1; //any number will suffice
	/*
	Mahatavacha aahe, 
	on fly risk aahe Sir used VK_API_VERSION_1_3 as installed 1.3.296 version
	Those using 1.4.304 must use VK_API_VERSION_1_4
	*/
	vkApplicationInfo.apiVersion = VK_API_VERSION_1_4; 
	
	/*
	Initialize struct VkInstanceCreateInfo by using information from Step1 and Step2 (Important)
	*/
	struct VkInstanceCreateInfo vkInstanceCreateInfo;
	memset((void*)&vkInstanceCreateInfo, 0, sizeof(struct VkInstanceCreateInfo));
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkInstanceCreateInfo.html
	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pNext = NULL;
	vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
	//folowing 2 members important
	vkInstanceCreateInfo.enabledExtensionCount = gState_scene0.enabledInstanceExtensionsCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames = gState_scene0.enabledInstanceExtensionNames_array;
	//21_Validation
	if(gState_scene0.bValidation == TRUE)
	{
		vkInstanceCreateInfo.enabledLayerCount = gState_scene0.enabledValidationLayerCount;
		vkInstanceCreateInfo.ppEnabledLayerNames = gState_scene0.enabledValidationlayerNames_array;
	}
	else
	{
		vkInstanceCreateInfo.enabledLayerCount = 0;
		vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
	}

	/*
	Call vkCreateInstance() to get VkInstance in a global variable and do error checking
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateInstance.html
	//2nd parameters is NULL as saying tuza memory allocator vapar , mazyakade custom memory allocator nahi
	vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &gState_scene0.vkInstance);
	if (vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): vkCreateInstance() function failed due to incompatible driver with error code %d\n", vkResult);
		return vkResult;
	}
	else if (vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): vkCreateInstance() function failed due to required extension not present with error code %d\n", vkResult);
		return vkResult;
	}
	else if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): vkCreateInstance() function failed due to unknown reason with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): vkCreateInstance() succedded\n");
	}
	
	//21_validation: do for validation callbacks
	if(gState_scene0.bValidation)
	{
		//21_Validation
                vkResult = gVulkanFunctions_Scene0.createValidationCallbackFunction();
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): CreateValidationCallbackFunction()  function failed\n");
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "CreateVulkanInstance(): CreateValidationCallbackFunction() succedded\n");
		}
	}
	
	return vkResult;
}

VkResult FillInstanceExtensionNames(void)
{
	// Code
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;

	/*
	1. Find how many instance extensions are supported by Vulkan driver of/for this version and keept the count in a local variable.
	1.3.296 madhe ek instance navta , je aata add zala aahe 1.4.304 madhe , VK_NV_DISPLAY_STEREO
	*/
	uint32_t instanceExtensionCount = 0;

	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkEnumerateInstanceExtensionProperties.html
	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	/* like in OpenCL
	1st - which layer extension required, as want all so NULL (akha driver supported kelleli extensions)
	2nd - count de mala
	3rd - Extension cha property cha array, NULL aahe karan count nahi ajun aplyakade
	*/
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): First call to vkEnumerateInstanceExtensionProperties()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): First call to vkEnumerateInstanceExtensionProperties() succedded\n");
	}

	/*
	 Allocate and fill struct VkExtensionProperties 
	 (https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtensionProperties.html) structure array, 
	 corresponding to above count
	*/
	VkExtensionProperties* vkExtensionProperties_array = NULL;
	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
	if (vkExtensionProperties_array != NULL)
	{
		//Add log here later for failure
		//exit(-1);
	}
	else
	{
		//Add log here later for success
	}

	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): Second call to vkEnumerateInstanceExtensionProperties()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): Second call to vkEnumerateInstanceExtensionProperties() succedded\n");
	}

	/*
	Fill and display a local string array of extension names obtained from VkExtensionProperties structure array
	*/
	char** instanceExtensionNames_array = NULL;
	instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
	if (instanceExtensionNames_array != NULL)
	{
		//Add log here later for failure
		//exit(-1);
	}
	else
	{
		//Add log here later for success
	}

	for (uint32_t i =0; i < instanceExtensionCount; i++)
	{
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtensionProperties.html
		instanceExtensionNames_array[i] = (char*)malloc( sizeof(char) * (strlen(vkExtensionProperties_array[i].extensionName) + 1));
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, (strlen(vkExtensionProperties_array[i].extensionName) + 1));
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): Vulkan Instance Extension Name = %s\n", instanceExtensionNames_array[i]);
	}

	/*
	As not required here onwards, free VkExtensionProperties array
	*/
	if (vkExtensionProperties_array)
	{
		free(vkExtensionProperties_array);
		vkExtensionProperties_array = NULL;
	}

	/*
	Find whether above extension names contain our required two extensions
	VK_KHR_SURFACE_EXTENSION_NAME
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME (added for 21_Validation)
	Accordingly set two global variables, "required extension count" and "required extension names array"
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkBool32.html -> Vulkan cha bool
	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 vulkanWin32SurfaceExtensionFound = VK_FALSE;
	
	//21_Validation
	VkBool32 debugReportExtensionFound = VK_FALSE;
	
	for (uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if (strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanSurfaceExtensionFound = VK_TRUE;
			gState_scene0.enabledInstanceExtensionNames_array[gState_scene0.enabledInstanceExtensionsCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}

		if (strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanWin32SurfaceExtensionFound = VK_TRUE;
			gState_scene0.enabledInstanceExtensionNames_array[gState_scene0.enabledInstanceExtensionsCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		}
		
		if (strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
		{
			debugReportExtensionFound = VK_TRUE;
			if(gState_scene0.bValidation == TRUE)
			{
				gState_scene0.enabledInstanceExtensionNames_array[gState_scene0.enabledInstanceExtensionsCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			}
			else
			{
				//array will not have entry so no code here
				//gState_scene0.enabledInstanceExtensionNames_array[gState_scene0.enabledInstanceExtensionsCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			}
		}
	}

	/*
	As not needed hence forth , free local string array
	*/
	for (uint32_t i =0 ; i < instanceExtensionCount; i++)
	{
		free(instanceExtensionNames_array[i]);
	}
	free(instanceExtensionNames_array);

	/*
	Print whether our required instance extension names or not (He log madhe yenar. Jithe print asel sarv log madhe yenar)
	*/
	if (vulkanSurfaceExtensionFound == VK_FALSE)
	{
		//Type mismatch in return VkResult and VKBool32, so return hardcoded failure
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): VK_KHR_SURFACE_EXTENSION_NAME not found\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): VK_KHR_SURFACE_EXTENSION_NAME is found\n");
	}

	if (vulkanWin32SurfaceExtensionFound == VK_FALSE)
	{
		//Type mismatch in return VkResult and VKBool32, so return hardcoded failure
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): VK_KHR_WIN32_SURFACE_EXTENSION_NAME is found\n");
	}
	
	if (debugReportExtensionFound == VK_FALSE)
	{
		if(gState_scene0.bValidation == TRUE)
		{
			//Type mismatch in return VkResult and VKBool32, so return hardcoded failure
			vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
			fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): Validation is ON, but required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): Validation is OFF, but VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
		}
	}
	else
	{
		if(gState_scene0.bValidation == TRUE)
		{
			//Type mismatch in return VkResult and VKBool32, so return hardcoded failure
			//vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
			fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): Validation is ON, but required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is also supported\n");
			//return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): Validation is OFF, but VK_EXT_DEBUG_REPORT_EXTENSION_NAME is also supported\n");
		}
	}

	/*
	Print only enabled extension names
	*/
	for (uint32_t i = 0; i < gState_scene0.enabledInstanceExtensionsCount; i++)
	{
		fprintf(gState_scene0.gFILE, "FillInstanceExtensionNames(): Enabled Vulkan Instance Extension Name = %s\n", gState_scene0.enabledInstanceExtensionNames_array[i]);
	}

	return vkResult;
}

VkResult FillValidationLayerNames(void)
{
	//Code
	
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	uint32_t validationLayerCount = 0;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkEnumerateInstanceLayerProperties.html
	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "FillValidationLayerNames(): First call to vkEnumerateInstanceLayerProperties()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillValidationLayerNames(): First call to vkEnumerateInstanceLayerProperties() succedded\n");
	}
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkLayerProperties.html
	VkLayerProperties* vkLayerProperties_array = NULL;
	vkLayerProperties_array = (VkLayerProperties*)malloc(sizeof(VkLayerProperties) * validationLayerCount);
	if (vkLayerProperties_array != NULL)
	{
		//Add log here later for failure
		//exit(-1);
	}
	else
	{
		//Add log here later for success
	}
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkEnumerateInstanceLayerProperties.html
	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "FillValidationLayerNames(): Second call to vkEnumerateInstanceLayerProperties()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillValidationLayerNames(): Second call to vkEnumerateInstanceLayerProperties() succedded\n");
	}
	
	char** validationLayerNames_array = NULL;
	validationLayerNames_array = (char**)malloc(sizeof(char*) * validationLayerCount);
	if (validationLayerNames_array != NULL)
	{
		//Add log here later for failure
		//exit(-1);
	}
	else
	{
		//Add log here later for success
	}

	for (uint32_t i =0; i < validationLayerCount; i++)
	{
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkLayerProperties.html
		validationLayerNames_array[i] = (char*)malloc( sizeof(char) * (strlen(vkLayerProperties_array[i].layerName) + 1));
		memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, (strlen(vkLayerProperties_array[i].layerName) + 1));
		fprintf(gState_scene0.gFILE, "FillValidationLayerNames(): Vulkan Instance Layer Name = %s\n", validationLayerNames_array[i]);
	}

	if (vkLayerProperties_array)
	{
		free(vkLayerProperties_array);
		vkLayerProperties_array = NULL;
	}
	
	//For required 1 validation layer VK_LAYER_KHRONOS_validation
	VkBool32 validationLayerFound = VK_FALSE;
	
	for (uint32_t i = 0; i < validationLayerCount; i++)
	{
		if (strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
		{
			validationLayerFound = VK_TRUE;
			gState_scene0.enabledValidationlayerNames_array[gState_scene0.enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
		}
	}
	
	for (uint32_t i =0 ; i < validationLayerCount; i++)
	{
		free(validationLayerNames_array[i]);
	}
	free(validationLayerNames_array);
	
	if(validationLayerFound == VK_FALSE)
	{
		//Type mismatch in return VkResult and VKBool32, so return hardcoded failure
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "FillValidationLayerNames(): VK_LAYER_KHRONOS_validation not supported\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillValidationLayerNames(): VK_LAYER_KHRONOS_validation is supported\n");
	}
	
	/*
	Print only enabled extension names
	*/
	for (uint32_t i = 0; i < gState_scene0.enabledValidationLayerCount; i++)
	{
		fprintf(gState_scene0.gFILE, "FillValidationLayerNames(): Enabled Vulkan Validation Layer Name = %s\n", gState_scene0.enabledValidationlayerNames_array[i]);
	}
	
	return vkResult;
}

VkResult CreateValidationCallbackFunction(void)
{
	//Function declaration
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDebugReportFlagsEXT.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VKAPI_ATTR.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDebugReportObjectTypeEXT.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/PFN_vkDebugReportCallbackEXT.html
	*/
	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t, int32_t, const char*, const char*, void*);
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDebugReportCallbackEXT.html
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;
	
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	
	//Code
	//get required function pointers
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetInstanceProcAddr.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDebugReportCallbackEXT.html
	*/
	vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(gState_scene0.vkInstance, "vkCreateDebugReportCallbackEXT");
	if(vkCreateDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gState_scene0.gFILE, "CreateValidationCallbackFunction(): vkGetInstanceProcAddr() failed to get function pointer for vkCreateDebugReportCallbackEXT\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateValidationCallbackFunction(): vkGetInstanceProcAddr() suceeded getting function pointer for vkCreateDebugReportCallbackEXT\n");
	}
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyDebugReportCallbackEXT.html
	gState_scene0.vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(gState_scene0.vkInstance, "vkDestroyDebugReportCallbackEXT");
	if(gState_scene0.vkDestroyDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gState_scene0.gFILE, "CreateValidationCallbackFunction(): vkGetInstanceProcAddr() failed to get function pointer for vkDestroyDebugReportCallbackEXT\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateValidationCallbackFunction(): vkGetInstanceProcAddr() suceeded getting function pointer for vkDestroyDebugReportCallbackEXT\n");
	}
	
	//get VulkanDebugReportCallback object
	/*
	VkDebugReportCallbackEXT *gState_scene0.vkDebugReportCallbackEXT = VK_NULL_HANDLE; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDebugReportCallbackEXT.html

	//https://registry.khronos.org/vulkan/specs/latest/man/html/PFN_vkDebugReportCallbackEXT.html 
	PFN_vkDestroyDebugReportCallbackEXT gState_scene0.vkDestroyDebugReportCallbackEXT_fnptr = NULL; 
	*/
	VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT ; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDebugReportCallbackCreateInfoEXT.html
	memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
	/*
	// Provided by VK_EXT_debug_report
	typedef struct VkDebugReportCallbackCreateInfoEXT {
		VkStructureType                 sType;
		const void*                     pNext;
		VkDebugReportFlagsEXT           flags;
		PFN_vkDebugReportCallbackEXT    pfnCallback;
		void*                           pUserData;
	} VkDebugReportCallbackCreateInfoEXT;
	*/
	vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
	vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT|VK_DEBUG_REPORT_WARNING_BIT_EXT|VK_DEBUG_REPORT_INFORMATION_BIT_EXT|VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT|VK_DEBUG_REPORT_DEBUG_BIT_EXT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDebugReportFlagBitsEXT.html
	vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
	vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDebugReportCallbackEXT.html
	vkResult = vkCreateDebugReportCallbackEXT_fnptr(gState_scene0.vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &gState_scene0.vkDebugReportCallbackEXT);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateValidationCallbackFunction(): vkCreateDebugReportCallbackEXT_fnptr()  function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateValidationCallbackFunction(): vkCreateDebugReportCallbackEXT_fnptr() succedded\n");
	}
	
	return vkResult;
}

VkResult GetSupportedSurface(void)
{
	//Code
	
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Declare and memset a platform(Windows, Linux , Android etc) specific SurfaceInfoCreate structure
	*/
	VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
	memset((void*)&vkWin32SurfaceCreateInfoKHR, 0 , sizeof(struct VkWin32SurfaceCreateInfoKHR));
	
	/*
	Initialize it , particularly its HINSTANCE and HWND members
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkWin32SurfaceCreateInfoKHR.html
	vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkWin32SurfaceCreateInfoKHR.pNext = NULL;
	vkWin32SurfaceCreateInfoKHR.flags = 0;
	vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(gState_scene0.window.hwnd, GWLP_HINSTANCE); //This member can also be initialized by using (HINSTANCE)GetModuleHandle(NULL); {typecasted HINSTANCE}
	vkWin32SurfaceCreateInfoKHR.hwnd = gState_scene0.window.hwnd;
	
	/*
	Now call VkCreateWin32SurfaceKHR() to create the presentation surface object
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateWin32SurfaceKHR.html
	vkResult = vkCreateWin32SurfaceKHR(gState_scene0.vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &gState_scene0.vkSurfaceKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "GetSupportedSurface(): vkCreateWin32SurfaceKHR()  function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "GetSupportedSurface(): vkCreateWin32SurfaceKHR() succedded\n");
	}
	
	return vkResult;
}

VkResult GetPhysicalDevice(void)
{
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	/*
	2. Call vkEnumeratePhysicalDevices() to get Physical device count
	*/
	vkResult = vkEnumeratePhysicalDevices(gState_scene0.vkInstance, &gState_scene0.physicalDeviceCount, NULL); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkEnumeratePhysicalDevices.html (first call)
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): vkEnumeratePhysicalDevices() first call failed with error code %d\n", vkResult);
		return vkResult;
	}
	else if (gState_scene0.physicalDeviceCount == 0)
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): vkEnumeratePhysicalDevices() first call resulted in 0 physical devices\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): vkEnumeratePhysicalDevices() first call succedded\n");
	}
	
	/*
	3. Allocate VkPhysicalDeviceArray object according to above count
	*/
	gState_scene0.vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gState_scene0.physicalDeviceCount);
	//for sake of brevity no error checking
	
	/*
	4. Call vkEnumeratePhysicalDevices() again to fill above array
	*/
	vkResult = vkEnumeratePhysicalDevices(gState_scene0.vkInstance, &gState_scene0.physicalDeviceCount, gState_scene0.vkPhysicalDevice_array); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkEnumeratePhysicalDevices.html (seocnd call)
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): vkEnumeratePhysicalDevices() second call failed with error code %d\n", vkResult);
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): vkEnumeratePhysicalDevices() second call succedded\n");
	}
	
	/*
	5. Start a loop using physical device count and physical device, array obtained above (Note: declare a boolean bFound variable before this loop which will decide whether we found desired physical device or not)
	Inside this loop, 
	a. Declare a local variable to hold queque count
	b. Call vkGetPhysicalDeviceQuequeFamilyProperties() to initialize above queque count variable
	c. Allocate VkQuequeFamilyProperties array according to above count
	d. Call vkGetPhysicalDeviceQuequeFamilyProperties() again to fill above array
	e. Declare VkBool32 type array and allocate it using the same above queque count
	f. Start a nested loop and fill above VkBool32 type array by calling vkGetPhysicalDeviceSurfaceSupportKHR()
	g. Start another nested loop(not inside above loop , but nested in main loop) and check whether physical device
	   in its array with its queque family "has"(Sir told to underline) graphics bit or not. 
	   If yes then this is a selected physical device and assign it to global variable. 
	   Similarly this index is the selected queque family index and assign it to global variable too and set bFound to true
	   and break out from second nested loop
	h. Now we are back in main loop, so free queque family array and VkBool32 type array
	i. Still being in main loop, acording to bFound variable break out from main loop
	j. free physical device array 
	*/
	VkBool32 bFound = VK_FALSE; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBool32.html
	for(uint32_t i = 0; i < gState_scene0.physicalDeviceCount; i++)
	{
		/*
		a. Declare a local variable to hold queque count
		*/
		uint32_t quequeCount = UINT32_MAX;
		
		
		/*
		b. Call vkGetPhysicalDeviceQuequeFamilyProperties() to initialize above queque count variable
		*/
		//Strange call returns void
		//Error checking not done above as yacha VkResult nahi aahe
		//Kiti physical devices denar , jevde array madhe aahet tevda -> Second parameter
		//If physical device is present , then it must separate atleast one qurque family
		vkGetPhysicalDeviceQueueFamilyProperties(gState_scene0.vkPhysicalDevice_array[i], &quequeCount, NULL);//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceQueueFamilyProperties.html
		
		/*
		c. Allocate VkQuequeFamilyProperties array according to above count
		*/
		struct VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;//https://registry.khronos.org/vulkan/specs/latest/man/html/VkQueueFamilyProperties.html
		vkQueueFamilyProperties_array = (struct VkQueueFamilyProperties*) malloc(sizeof(struct VkQueueFamilyProperties) * quequeCount);
		//for sake of brevity no error checking
		
		/*
		d. Call vkGetPhysicalDeviceQuequeFamilyProperties() again to fill above array
		*/
		vkGetPhysicalDeviceQueueFamilyProperties(gState_scene0.vkPhysicalDevice_array[i], &quequeCount, vkQueueFamilyProperties_array);//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceQueueFamilyProperties.html
		
		/*
		e. Declare VkBool32 type array and allocate it using the same above queque count
		*/
		VkBool32 *isQuequeSurfaceSupported_array = NULL;
		isQuequeSurfaceSupported_array = (VkBool32*) malloc(sizeof(VkBool32) * quequeCount);
		//for sake of brevity no error checking
		
		/*
		f. Start a nested loop and fill above VkBool32 type array by calling vkGetPhysicalDeviceSurfaceSupportKHR()
		*/
		for(uint32_t j =0; j < quequeCount ; j++)
		{
			//vkGetPhysicalDeviceSurfaceSupportKHR ->Supported surface la tumhi dilela surface support karto ka?
			//gState_scene0.vkPhysicalDevice_array[i] -> ya device cha
			//j -> ha index
			//gState_scene0.vkSurfaceKHR -> ha surface
			//isQuequeSurfaceSupported_array-> support karto ki nahi bhar
			vkResult = vkGetPhysicalDeviceSurfaceSupportKHR(gState_scene0.vkPhysicalDevice_array[i], j, gState_scene0.vkSurfaceKHR, &isQuequeSurfaceSupported_array[j]); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
		}
		
		/*
		g. Start another nested loop(not inside above loop , but nested in main loop) and check whether physical device
		   in its array with its queque family "has"(Sir told to underline) graphics bit or not. 
		   If yes then this is a selected physical device and assign it to global variable. 
		   Similarly this index is the selected queque family index and assign it to global variable too and set bFound to true
		   and break out from second nested loop
		*/
		for(uint32_t j =0; j < quequeCount ; j++)
		{
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkQueueFamilyProperties.html
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkQueueFlagBits.html
			if(vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				//select ith graphic card, queque familt at j, bFound la TRUE karun break vha
				if(isQuequeSurfaceSupported_array[j] == VK_TRUE)
				{
					gState_scene0.vkPhysicalDevice_selected = gState_scene0.vkPhysicalDevice_array[i];
					gState_scene0.graphicsQuequeFamilyIndex_selected = j;
					bFound = VK_TRUE;
					break;
				}
			}
		}
		
		/*
		h. Now we are back in main loop, so free queque family array and VkBool32 type array
		*/
		if(isQuequeSurfaceSupported_array)
		{
			free(isQuequeSurfaceSupported_array);
			isQuequeSurfaceSupported_array = NULL;
			fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): succedded to free isQuequeSurfaceSupported_array\n");
		}
		
		
		if(vkQueueFamilyProperties_array)
		{
			free(vkQueueFamilyProperties_array);
			vkQueueFamilyProperties_array = NULL;
			fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): succedded to free vkQueueFamilyProperties_array\n");
		}
		
		/*
		i. Still being in main loop, acording to bFound variable break out from main loop
		*/
		if(bFound == VK_TRUE)
		{
			break;
		}
	}
	
	/*
	6. Do error checking according to value of bFound
	*/
	if(bFound == VK_TRUE)
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): GetPhysicalDevice() suceeded to select required physical device with graphics enabled\n");
		
		/*
		PrintVulkanInfo() changes
		2. Accordingly remove physicaldevicearray freeing block from if(bFound == VK_TRUE) block and we will later write this freeing block in printVkInfo().
		*/
		
		/*
		//j. free physical device array 
		if(gState_scene0.vkPhysicalDevice_array)
		{
			free(gState_scene0.vkPhysicalDevice_array);
			gState_scene0.vkPhysicalDevice_array = NULL;
			fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): succedded to free gState_scene0.vkPhysicalDevice_array\n");
		}
		*/
	}
	else
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): GetPhysicalDevice() failed to obtain graphics supported physical device\n");
		
		/*
		j. free physical device array 
		*/
		if(gState_scene0.vkPhysicalDevice_array)
		{
			free(gState_scene0.vkPhysicalDevice_array);
			gState_scene0.vkPhysicalDevice_array = NULL;
			fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): succedded to free gState_scene0.vkPhysicalDevice_array\n");
		}
		
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	
	/*
	7. memset the global physical device memory property structure
	*/
	memset((void*)&gState_scene0.vkPhysicalDeviceMemoryProperties, 0, sizeof(struct VkPhysicalDeviceMemoryProperties)); //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceMemoryProperties.html
	
	/*
	8. initialize above structure by using vkGetPhysicalDeviceMemoryProperties() //https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceMemoryProperties.html
	No need of error checking as we already have physical device
	*/
	vkGetPhysicalDeviceMemoryProperties(gState_scene0.vkPhysicalDevice_selected, &gState_scene0.vkPhysicalDeviceMemoryProperties);
	
	/*
	9. Declare a local structure variable VkPhysicalDeviceFeatures, memset it  and initialize it by calling vkGetPhysicalDeviceFeatures() 
	// https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceFeatures.html
	// //https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceFeatures.html
	*/
	VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
	memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
	vkGetPhysicalDeviceFeatures(gState_scene0.vkPhysicalDevice_selected, &vkPhysicalDeviceFeatures);
	
	/*
	10. By using "tescellation shader" member of above structure check selected device's tescellation shader support
	11. By using "geometry shader" member of above structure check selected device's geometry shader support
	*/
	if(vkPhysicalDeviceFeatures.tessellationShader)
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): Supported physical device supports tessellation shader\n");
	}
	else
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): Supported physical device does not support tessellation shader\n");
	}
	
	if(vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): Supported physical device supports geometry shader\n");
	}
	else
	{
		fprintf(gState_scene0.gFILE, "GetPhysicalDevice(): Supported physical device does not support geometry shader\n");
	}
	
	/*
	12. There is no need to free/uninitialize/destroy selected physical device?
	Bcoz later we will create Vulkan logical device which need to be destroyed and its destruction will automatically destroy selected physical device.
	*/
	
	return vkResult;
}

/*
PrintVkInfo() changes
3. Write printVkInfo() user defined function with following steps
3a. Start a loop using global physical device count and inside it declare  and memset VkPhysicalDeviceProperties struct variable (https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceProperties.html).
3b. Initialize this struct variable by calling vkGetPhysicalDeviceProperties() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceProperties.html) vulkan api.
3c. Print Vulkan API version using apiVersion member of above struct.
	This requires 3 Vulkan macros.
3d. Print device name by using "deviceName" member of above struct.
3e. Use "deviceType" member of above struct in a switch case block and accordingly print device type.
3f. Print hexadecimal Vendor Id of device using "vendorId" member of above struct.
3g. Print hexadecimal deviceID of device using "deviceId" member of struct.
Note*: For sake of completeness, we can repeat a to h points from GetPhysicalDevice() {05-GetPhysicalDevice notes},
but now instead of assigning selected queque and selected device, print whether this device supports graphic bit, compute bit, transfer bit using if else if else if blocks
Similarly we also can repeat device features from GetPhysicalDevice() and can print all around 50 plus device features including support to tescellation shader and geometry shader.
3h. Free physicaldevice array here which we removed from if(bFound == VK_TRUE) block of GetPhysicalDevice().
*/
VkResult PrintVulkanInfo(void)
{
	VkResult vkResult = VK_SUCCESS;
	
	//Code
	fprintf(gState_scene0.gFILE, "************************* Shree Ganesha******************************\n");
	
	/*
	PrintVkInfo() changes
	3a. Start a loop using global physical device count and inside it declare  and memset VkPhysicalDeviceProperties struct variable
	*/
	for(uint32_t i = 0; i < gState_scene0.physicalDeviceCount; i++)
	{
		/*
		PrintVkInfo() changes
		3b. Initialize this struct variable by calling vkGetPhysicalDeviceProperties()
		*/
		VkPhysicalDeviceProperties vkPhysicalDeviceProperties; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceProperties.html
		memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
		vkGetPhysicalDeviceProperties(gState_scene0.vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties ); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceProperties.html
		
		/*
		PrintVkInfo() changes
		3c. Print Vulkan API version using apiVersion member of above struct.
		This requires 3 Vulkan macros.
		*/
		//uint32_t majorVersion,minorVersion,patchVersion;
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VK_VERSION_MAJOR.html -> api deprecation for which we changed to VK_API_VERSION_XXXXX
		uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion); //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceProperties.html
		uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);
		
		//API Version
		fprintf(gState_scene0.gFILE,"apiVersion = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);
		
		/*
		PrintVkInfo() changes
		3d. Print device name by using "deviceName" member of above struct.
		*/
		fprintf(gState_scene0.gFILE,"deviceName = %s\n", vkPhysicalDeviceProperties.deviceName);
		
		/*
		PrintVkInfo() changes
		3e. Use "deviceType" member of above struct in a switch case block and accordingly print device type.
		*/
		switch(vkPhysicalDeviceProperties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				fprintf(gState_scene0.gFILE,"deviceType = Integrated GPU (iGPU)\n");
			break;
			
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				fprintf(gState_scene0.gFILE,"deviceType = Discrete GPU (dGPU)\n");
			break;
			
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				fprintf(gState_scene0.gFILE,"deviceType = Virtual GPU (vGPU)\n");
			break;
			
			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				fprintf(gState_scene0.gFILE,"deviceType = CPU\n");
			break;
			
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
				fprintf(gState_scene0.gFILE,"deviceType = Other\n");
			break;
			
			default:
				fprintf(gState_scene0.gFILE, "deviceType = UNKNOWN\n");
			break;
		}
		
		/*
		PrintVkInfo() changes
		3f. Print hexadecimal Vendor Id of device using "vendorId" member of above struct.
		*/
		fprintf(gState_scene0.gFILE,"vendorID = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);
		
		/*
		PrintVkInfo() changes
		3g. Print hexadecimal deviceID of device using "deviceId" member of struct.
		*/
		fprintf(gState_scene0.gFILE,"deviceID = 0x%04x\n", vkPhysicalDeviceProperties.deviceID);
	}
	
	/*
	PrintVkInfo() changes
	3h. Free physicaldevice array here which we removed from if(bFound == VK_TRUE) block of GetPhysicalDevice().
	*/
	if(gState_scene0.vkPhysicalDevice_array)
	{
		free(gState_scene0.vkPhysicalDevice_array);
		gState_scene0.vkPhysicalDevice_array = NULL;
		fprintf(gState_scene0.gFILE, "PrintVkInfo(): succedded to free gState_scene0.vkPhysicalDevice_array\n");
	}
	
	return vkResult;
}

VkResult FillDeviceExtensionNames(void)
{
	// Code
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;

	/*
	1. Find how many device extensions are supported by Vulkan driver of/for this version and keept the count in a local variable.
	*/
	uint32_t deviceExtensionCount = 0;

	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkEnumerateDeviceExtensionProperties.html
	vkResult = vkEnumerateDeviceExtensionProperties(gState_scene0.vkPhysicalDevice_selected, NULL, &deviceExtensionCount, NULL );
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "FillDeviceExtensionNames(): First call to vkEnumerateDeviceExtensionProperties()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillDeviceExtensionNames(): First call to vkEnumerateDeviceExtensionProperties() succedded and returned %u count\n", deviceExtensionCount);
	}

	/*
	 Allocate and fill struct VkExtensionProperties 
	 (https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtensionProperties.html) structure array, 
	 corresponding to above count
	*/
	VkExtensionProperties* vkExtensionProperties_array = NULL;
	vkExtensionProperties_array = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
	if (vkExtensionProperties_array != NULL)
	{
		//Add log here later for failure
		//exit(-1);
	}
	else
	{
		//Add log here later for success
	}

	vkResult = vkEnumerateDeviceExtensionProperties(gState_scene0.vkPhysicalDevice_selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "FillDeviceExtensionNames(): Second call to vkEnumerateDeviceExtensionProperties()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillDeviceExtensionNames(): Second call to vkEnumerateDeviceExtensionProperties() succedded\n");
	}

	/*
	Fill and display a local string array of extension names obtained from VkExtensionProperties structure array
	*/
	char** deviceExtensionNames_array = NULL;
	deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
	if (deviceExtensionNames_array != NULL)
	{
		//Add log here later for failure
		//exit(-1);
	}
	else
	{
		//Add log here later for success
	}

	for (uint32_t i =0; i < deviceExtensionCount; i++)
	{
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtensionProperties.html
		deviceExtensionNames_array[i] = (char*)malloc( sizeof(char) * (strlen(vkExtensionProperties_array[i].extensionName) + 1));
		memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, (strlen(vkExtensionProperties_array[i].extensionName) + 1));
		fprintf(gState_scene0.gFILE, "FillDeviceExtensionNames(): Vulkan Device Extension Name = %s\n", deviceExtensionNames_array[i]);
	}

	/*
	As not required here onwards, free VkExtensionProperties array
	*/
	if (vkExtensionProperties_array)
	{
		free(vkExtensionProperties_array);
		vkExtensionProperties_array = NULL;
	}

	/*
	Find whether above extension names contain our required two extensions
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	Accordingly set two global variables, "required extension count" and "required extension names array"
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkBool32.html -> Vulkan cha bool
	VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;
	for (uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if (strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			vulkanSwapchainExtensionFound = VK_TRUE;
			gState_scene0.enabledDeviceExtensionNames_array[gState_scene0.enabledDeviceExtensionsCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}
	}

	/*
	As not needed hence forth , free local string array
	*/
	for (uint32_t i =0 ; i < deviceExtensionCount; i++)
	{
		free(deviceExtensionNames_array[i]);
	}
	free(deviceExtensionNames_array);

	/*
	Print whether our required device extension names or not (He log madhe yenar. Jithe print asel sarv log madhe yenar)
	*/
	if (vulkanSwapchainExtensionFound == VK_FALSE)
	{
		//Type mismatch in return VkResult and VKBool32, so return hardcoded failure
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "FillDeviceExtensionNames(): VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "FillDeviceExtensionNames(): VK_KHR_SWAPCHAIN_EXTENSION_NAME is found\n");
	}

	/*
	Print only enabled device extension names
	*/
	for (uint32_t i = 0; i < gState_scene0.enabledDeviceExtensionsCount; i++)
	{
		fprintf(gState_scene0.gFILE, "FillDeviceExtensionNames(): Enabled Vulkan Device Extension Name = %s\n", gState_scene0.enabledDeviceExtensionNames_array[i]);
	}

	return vkResult;
}

VkResult CreateVulKanDevice(void)
{
        //Variable declarations
        VkResult vkResult = VK_SUCCESS;
	
	/*
	fill device extensions
	2. Call previously created FillDeviceExtensionNames() in it.
	*/
        vkResult = gVulkanFunctions_Scene0.fillDeviceExtensionNames();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVulKanDevice(): FillDeviceExtensionNames()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVulKanDevice(): FillDeviceExtensionNames() succedded\n");
	}
	
	/*
	Newly added code
	*/
	//float queuePriorities[1]  = {1.0};
	float queuePriorities[1];
	queuePriorities[0] = 1.0f;
	VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceQueueCreateInfo.html
	memset(&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	
	vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkDeviceQueueCreateInfo.pNext = NULL;
	vkDeviceQueueCreateInfo.flags = 0;
	vkDeviceQueueCreateInfo.queueFamilyIndex = gState_scene0.graphicsQuequeFamilyIndex_selected;
	vkDeviceQueueCreateInfo.queueCount = 1;
	vkDeviceQueueCreateInfo.pQueuePriorities = queuePriorities;
	
	/*
	3. Declare and initialize VkDeviceCreateInfo structure (https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceCreateInfo.html).
	*/
	VkDeviceCreateInfo vkDeviceCreateInfo;
	memset(&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
	
	/*
	4. Use previously obtained device extension count and device extension array to initialize this structure.
	*/
	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pNext = NULL;
	vkDeviceCreateInfo.flags = 0;
	vkDeviceCreateInfo.enabledExtensionCount = gState_scene0.enabledDeviceExtensionsCount;
	vkDeviceCreateInfo.ppEnabledExtensionNames = gState_scene0.enabledDeviceExtensionNames_array;
	vkDeviceCreateInfo.enabledLayerCount = 0;
	vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
	vkDeviceCreateInfo.pEnabledFeatures = NULL;
	vkDeviceCreateInfo.queueCreateInfoCount = 1;
	vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;
	
	/*
	5. Now call vkCreateDevice to create actual Vulkan device and do error checking.
	*/
	vkResult = vkCreateDevice(gState_scene0.vkPhysicalDevice_selected, &vkDeviceCreateInfo, NULL, &gState_scene0.vkDevice); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDevice.html
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVulKanDevice(): vkCreateDevice()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVulKanDevice(): vkCreateDevice() succedded\n");
	}
	
	return vkResult;
}

void GetDeviceQueque(void)
{
	//Code
	vkGetDeviceQueue(gState_scene0.vkDevice, gState_scene0.graphicsQuequeFamilyIndex_selected, 0, &gState_scene0.vkQueue); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetDeviceQueue.html
	if(gState_scene0.vkQueue == VK_NULL_HANDLE)
	{
		fprintf(gState_scene0.gFILE, "GetDeviceQueque(): vkGetDeviceQueue() returned NULL for gState_scene0.vkQueue\n");
		return;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "GetDeviceQueque(): vkGetDeviceQueue() succedded\n");
	}
}

VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	//Code
	//Get count of supported surface color formats
	uint32_t FormatCount = 0;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceSurfaceFormatsKHR.html
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gState_scene0.vkPhysicalDevice_selected, gState_scene0.vkSurfaceKHR, &FormatCount, NULL);
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace(): First call to vkGetPhysicalDeviceSurfaceFormatsKHR() failed\n");
		return vkResult;
	}
	else if(FormatCount == 0)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "vkGetPhysicalDeviceSurfaceFormatsKHR():: First call to vkGetPhysicalDeviceSurfaceFormatsKHR() returned FormatCount as 0\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace(): First call to vkGetPhysicalDeviceSurfaceFormatsKHR() succedded\n");
	}
	
	//Declare and allocate VkSurfaceKHR array
	VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*)malloc(FormatCount * sizeof(VkSurfaceFormatKHR)); //https://registry.khronos.org/vulkan/specs/latest/man/html/VkSurfaceFormatKHR.html
	//For sake of brevity  no error checking
	
	//Filling the array
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gState_scene0.vkPhysicalDevice_selected, gState_scene0.vkSurfaceKHR, &FormatCount, vkSurfaceFormatKHR_array); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceSurfaceFormatsKHR.html
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace(): Second call to vkGetPhysicalDeviceSurfaceFormatsKHR()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace():  Second call to vkGetPhysicalDeviceSurfaceFormatsKHR() succedded\n");
	}
	
	//According to contents of array , we have to decide surface format and color space
	//Decide surface format first
	if( (1 == FormatCount) && (vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED) )
	{
		gState_scene0.vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html
	}
	else 
	{
		gState_scene0.vkFormat_color = vkSurfaceFormatKHR_array[0].format; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html
	}
	
	//Decide color space second
	gState_scene0.vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;
	
	//free the array
	if(vkSurfaceFormatKHR_array)
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDeviceSurfaceFormatAndColorSpace(): vkSurfaceFormatKHR_array is freed\n");
		free(vkSurfaceFormatKHR_array);
		vkSurfaceFormatKHR_array = NULL;
	}
	
	return vkResult;
}

VkResult getPhysicalDevicePresentMode(void)
{
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	//Code
	//mailbox bhetel aata , fifo milel android la kadachit
	uint32_t presentModeCount = 0;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceSurfacePresentModesKHR.html
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gState_scene0.vkPhysicalDevice_selected, gState_scene0.vkSurfaceKHR, &presentModeCount, NULL);
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDevicePresentMode(): First call to vkGetPhysicalDeviceSurfaceFormatsKHR() failed\n");
		return vkResult;
	}
	else if(presentModeCount == 0)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "getPhysicalDevicePresentMode():: First call to vkGetPhysicalDeviceSurfaceFormatsKHR() returned presentModeCount as 0\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDevicePresentMode(): First call to vkGetPhysicalDeviceSurfaceFormatsKHR() succedded\n");
	}
	
	//Declare and allocate VkPresentModeKHR array
	VkPresentModeKHR  *vkPresentModeKHR_array = (VkPresentModeKHR*)malloc(presentModeCount * sizeof(VkPresentModeKHR)); //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPresentModeKHR.html
	//For sake of brevity  no error checking
	
	//Filling the array
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gState_scene0.vkPhysicalDevice_selected, gState_scene0.vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceSurfaceFormatsKHR.html
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDevicePresentMode(): Second call to vkGetPhysicalDeviceSurfacePresentModesKHR()  function failed\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDevicePresentMode():  Second call to vkGetPhysicalDeviceSurfacePresentModesKHR() succedded\n");
	}
	
	//According to contents of array , we have to decide presentation mode
	for(uint32_t i=0; i < presentModeCount; i++)
	{
		if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			gState_scene0.vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPresentModeKHR.html
			break;
		}
	}
	
	if(gState_scene0.vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		gState_scene0.vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPresentModeKHR.html
	}
	
	fprintf(gState_scene0.gFILE, "getPhysicalDevicePresentMode(): gState_scene0.vkPresentModeKHR is %d\n", gState_scene0.vkPresentModeKHR);
	
	//free the array
	if(vkPresentModeKHR_array)
	{
		fprintf(gState_scene0.gFILE, "getPhysicalDevicePresentMode(): vkPresentModeKHR_array is freed\n");
		free(vkPresentModeKHR_array);
		vkPresentModeKHR_array = NULL;
	}
	
	return vkResult;
}

VkResult CreateSwapChain(VkBool32 vsync)
{
        //Variable declarations
        VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	
	/*
	Surface Format and Color Space
	1. Get Physical Device Surface supported color format and physical device surface supported color space , using Step 10.
	*/
        vkResult = gVulkanFunctions_Scene0.getPhysicalDeviceSurfaceFormatAndColorSpace();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): getPhysicalDeviceSurfaceFormatAndColorSpace() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): getPhysicalDeviceSurfaceFormatAndColorSpace() succedded\n");
	}
	
	/*
	2. Get Physical Device Surface capabilities by using Vulkan API vkGetPhysicalDeviceSurfaceCapabilitiesKHR (https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceSurfaceCapabilitiesKHR.html)
    and accordingly initialize VkSurfaceCapabilitiesKHR structure (https://registry.khronos.org/vulkan/specs/latest/man/html/VkSurfaceCapabilitiesKHR.html).
	*/
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gState_scene0.vkPhysicalDevice_selected, gState_scene0.vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): vkGetPhysicalDeviceSurfaceCapabilitiesKHR() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): vkGetPhysicalDeviceSurfaceCapabilitiesKHR() succedded\n");
	}
	
	/*
	3. By using minImageCount and maxImageCount members of above structure , decide desired ImageCount for swapchain.
	*/
	uint32_t testingNumerOfSwapChainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
	uint32_t desiredNumerOfSwapChainImages = 0; //To find this
	if( (vkSurfaceCapabilitiesKHR.maxImageCount > 0) && (vkSurfaceCapabilitiesKHR.maxImageCount < testingNumerOfSwapChainImages) )
	{
		desiredNumerOfSwapChainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
	}
	else
	{
		desiredNumerOfSwapChainImages = vkSurfaceCapabilitiesKHR.minImageCount;
	}
		
	/*
	4. By using currentExtent.width and currentExtent.height members of above structure and comparing them with current width and height of window, decide image width and image height of swapchain.
	Choose size of swapchain image
	*/
	memset((void*)&gState_scene0.vkExtent2D_SwapChain, 0 , sizeof(VkExtent2D));
	if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
	{
		gState_scene0.vkExtent2D_SwapChain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		gState_scene0.vkExtent2D_SwapChain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): Swapchain Image Width x SwapChain  Image Height = %d X %d\n", gState_scene0.vkExtent2D_SwapChain.width, gState_scene0.vkExtent2D_SwapChain.height);
	}
	else
	{
		gState_scene0.vkExtent2D_SwapChain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		gState_scene0.vkExtent2D_SwapChain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): Swapchain Image Width x SwapChain  Image Height = %d X %d\n", gState_scene0.vkExtent2D_SwapChain.width, gState_scene0.vkExtent2D_SwapChain.height);
	
		/*
		If surface size is already defined, then swapchain image size must match with it.
		*/
		VkExtent2D vkExtent2D;
		memset((void*)&vkExtent2D, 0, sizeof(VkExtent2D));
		vkExtent2D.width = (uint32_t)gState_scene0.winWidth;
		vkExtent2D.height = (uint32_t)gState_scene0.winHeight;
		
		gState_scene0.vkExtent2D_SwapChain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
		gState_scene0.vkExtent2D_SwapChain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): Swapchain Image Width x SwapChain  Image Height = %d X %d\n", gState_scene0.vkExtent2D_SwapChain.width, gState_scene0.vkExtent2D_SwapChain.height);
	}
	
	/*
	5. Decide how we are going to use swapchain images, means whether we we are going to store image data and use it later (Deferred Rendering) or we are going to use it immediatly as color attachment.
	Set Swapchain image usage flag
	Image usage flag hi concept aahe
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageUsageFlagBits.html
	VkImageUsageFlagBits vkImageUsageFlagBits = (VkImageUsageFlagBits) (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT|VK_IMAGE_USAGE_TRANSFER_SRC_BIT); // VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT -> Imp, VK_IMAGE_USAGE_TRANSFER_SRC_BIT->Optional
	/*
	Although VK_IMAGE_USAGE_TRANSFER_SRC_BIT is not usefule here for triangle application.
	It is useful for texture, fbo, compute shader
	*/
	
	
	/*
	6. Swapchain  is capable of storing transformed image before presentation, which is called as PreTransform. 
    While creating swapchain , we can decide whether to pretransform or not the swapchain images. (Pre transform also includes flipping of image)
   
    Whether to consider pretransform/flipping or not?
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkSurfaceTransformFlagBitsKHR.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkSurfaceCapabilitiesKHR.html
	VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
	if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
	}
	
	/*
	Presentation Mode
	7. Get Presentation mode for swapchain images using Step 11.
	*/
        vkResult = gVulkanFunctions_Scene0.getPhysicalDevicePresentMode();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): getPhysicalDevicePresentMode() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): getPhysicalDevicePresentMode() succedded\n");
	}
	
	/*
	8. According to above data, declare ,memset and initialize VkSwapchainCreateInfoKHR  structure (https://registry.khronos.org/vulkan/specs/latest/man/html/VkSwapchainCreateInfoKHR.html)
	bas aata structure bharaycha aahe
	*/
	struct VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
	memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(struct VkSwapchainCreateInfoKHR));
	vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapchainCreateInfoKHR.pNext = NULL;
	vkSwapchainCreateInfoKHR.flags = 0;
	vkSwapchainCreateInfoKHR.surface = gState_scene0.vkSurfaceKHR;
	vkSwapchainCreateInfoKHR.minImageCount = desiredNumerOfSwapChainImages;
	vkSwapchainCreateInfoKHR.imageFormat = gState_scene0.vkFormat_color;
	vkSwapchainCreateInfoKHR.imageColorSpace = gState_scene0.vkColorSpaceKHR;
	vkSwapchainCreateInfoKHR.imageExtent.width = gState_scene0.vkExtent2D_SwapChain.width;
	vkSwapchainCreateInfoKHR.imageExtent.height = gState_scene0.vkExtent2D_SwapChain.height;
	vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlagBits;
	vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
	vkSwapchainCreateInfoKHR.imageArrayLayers = 1; //concept
	vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkSharingMode.html
	vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkCompositeAlphaFlagBitsKHR.html
	vkSwapchainCreateInfoKHR.presentMode = gState_scene0.vkPresentModeKHR;
	vkSwapchainCreateInfoKHR.clipped = VK_TRUE;
	//vkSwapchainCreateInfoKHR.oldSwapchain is of no use in this application. Will be used in resize.
	
	/*
	9. At the end , call vkCreateSwapchainKHR() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateSwapchainKHR.html) Vulkan API to create the swapchain
	*/
	vkResult = vkCreateSwapchainKHR(gState_scene0.vkDevice, &vkSwapchainCreateInfoKHR, NULL, &gState_scene0.vkSwapchainKHR);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): vkCreateSwapchainKHR() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateSwapChain(): vkCreateSwapchainKHR() succedded\n");
	}
	
	return vkResult;
}

VkResult CreateImagesAndImageViews(void)
{
        //Variable declarations
        VkResult vkResult = VK_SUCCESS;
	
	//Code
	
	//1. Get Swapchain image count in a global variable using vkGetSwapchainImagesKHR() API (https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetSwapchainImagesKHR.html).
	vkResult = vkGetSwapchainImagesKHR(gState_scene0.vkDevice, gState_scene0.vkSwapchainKHR, &gState_scene0.swapchainImageCount, NULL);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): first call to vkGetSwapchainImagesKHR() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else if(gState_scene0.swapchainImageCount == 0)
	{
		vkResult = vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): first call to vkGetSwapchainImagesKHR() function returned swapchain Image Count as 0\n");
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): first call to vkGetSwapchainImagesKHR() succedded with gState_scene0.swapchainImageCount as %d\n", gState_scene0.swapchainImageCount);
	}
	
	//2. Declare a global VkImage type array and allocate it to swapchain image count using malloc. (https://registry.khronos.org/vulkan/specs/latest/man/html/VkImage.html)
	// Allocate swapchain image array
	gState_scene0.swapChainImage_array = (VkImage*)malloc(sizeof(VkImage) * gState_scene0.swapchainImageCount);
	if(gState_scene0.swapChainImage_array == NULL)
	{
			fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): gState_scene0.swapChainImage_array is NULL. malloc() failed\n");
	}
	
	//3. Now call same function again which we called in Step 1 and fill this array.
	//Fill this array by swapchain images
	vkResult = vkGetSwapchainImagesKHR(gState_scene0.vkDevice, gState_scene0.vkSwapchainKHR, &gState_scene0.swapchainImageCount, gState_scene0.swapChainImage_array);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): second call to vkGetSwapchainImagesKHR() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): second call to vkGetSwapchainImagesKHR() succedded with gState_scene0.swapchainImageCount as %d\n", gState_scene0.swapchainImageCount);
	}
	
	//4. Declare another global array of type VkImageView(https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageView.html) and allocate it to sizeof Swapchain image count.
	// Allocate array of swapchain image view
	gState_scene0.swapChainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * gState_scene0.swapchainImageCount);
	if(gState_scene0.swapChainImageView_array == NULL)
	{
			fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): gState_scene0.swapChainImageView_array is NULL. malloc() failed\n");
	}
	
	//5. Declare  and initialize VkImageViewCreateInfo struct (https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageViewCreateInfo.html) except its ".image" member.
	//Initialize VkImageViewCreateInfo struct
	VkImageViewCreateInfo vkImageViewCreateInfo;
	memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
	
	/*
	typedef struct VkImageViewCreateInfo {
    VkStructureType            sType;
    const void*                pNext;
    VkImageViewCreateFlags     flags;
    VkImage                    image;
    VkImageViewType            viewType;
    VkFormat                   format;
    VkComponentMapping         components;
    VkImageSubresourceRange    subresourceRange;
	} VkImageViewCreateInfo;
	*/
	
	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html
	vkImageViewCreateInfo.format = gState_scene0.vkFormat_color;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkComponentMapping.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkComponentSwizzle.html
	vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageSubresourceRange.html
	/*
	typedef struct VkImageSubresourceRange {
    VkImageAspectFlags    aspectMask;
    uint32_t              baseMipLevel;
    uint32_t              levelCount;
    uint32_t              baseArrayLayer;
    uint32_t              layerCount;
	} VkImageSubresourceRange;
	*/
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageAspectFlags.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageAspectFlagBits.html
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageViewType.html
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	
	
	//6. Now start a loop for swapchain image count and inside this loop, initialize above ".image" member to swapchain image array index we obtained above and then call vkCreateImage() to fill  above ImageView array.
	//Fill image view array using above struct
	for(uint32_t i = 0; i < gState_scene0.swapchainImageCount; i++)
	{
		vkImageViewCreateInfo.image = gState_scene0.swapChainImage_array[i];
		
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateImageView.html
		vkResult = vkCreateImageView(gState_scene0.vkDevice, &vkImageViewCreateInfo, NULL, &gState_scene0.swapChainImageView_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkCreateImageView() function failed with error code %d at iteration %d\n", vkResult, i);
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkCreateImageView() succedded for iteration %d\n", i);
		}
	}
	
	//For depth image
        vkResult = gVulkanFunctions_Scene0.getSupportedDepthFormat();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): GetSupportedDepthFormat() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): GetSupportedDepthFormat() succedded\n");
	}
	
	//For depth image, initialize VkImageCreateInfo
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageCreateInfo.html
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkImageCreateInfo {
		VkStructureType          sType;
		const void*              pNext;
		VkImageCreateFlags       flags;
		VkImageType              imageType;
		VkFormat                 format;
		VkExtent3D               extent;
		uint32_t                 mipLevels;
		uint32_t                 arrayLayers;
		VkSampleCountFlagBits    samples;
		VkImageTiling            tiling;
		VkImageUsageFlags        usage;
		VkSharingMode            sharingMode;
		uint32_t                 queueFamilyIndexCount;
		const uint32_t*          pQueueFamilyIndices;
		VkImageLayout            initialLayout;
	} VkImageCreateInfo;
	*/
	VkImageCreateInfo vkImageCreateInfo;
	memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));
	vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vkImageCreateInfo.pNext = NULL;
	vkImageCreateInfo.flags = 0;
	vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageType.html
	vkImageCreateInfo.format = gState_scene0.vkFormat_depth;
	
	vkImageCreateInfo.extent.width = (uint32_t)gState_scene0.winWidth; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtent3D.html
	vkImageCreateInfo.extent.height = (uint32_t)gState_scene0.winHeight; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtent3D.html
	vkImageCreateInfo.extent.depth = 1; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtent3D.html
	
	vkImageCreateInfo.mipLevels = 1;
	vkImageCreateInfo.arrayLayers = 1;
	vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkSampleCountFlagBits.html
	vkImageCreateInfo.tiling =  VK_IMAGE_TILING_OPTIMAL; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageTiling.html
	vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageUsageFlags.html
	//vkImageCreateInfo.sharingMode = ;
	//vkImageCreateInfo.queueFamilyIndexCount = ;
	//vkImageCreateInfo.pQueueFamilyIndices = ;
	//vkImageCreateInfo.initialLayout = ;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateImage.html
	/*
	// Provided by VK_VERSION_1_0
	VkResult vkCreateImage(
    VkDevice                                    device,
    const VkImageCreateInfo*                    pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkImage*                                    pImage);
	*/
	vkResult = vkCreateImage(gState_scene0.vkDevice, &vkImageCreateInfo, NULL, &gState_scene0.vkImage_depth);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkCreateImage() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkCreateImage() succedded\n");
	}
	
	//Memory requirements for depth Image
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryRequirements {
		VkDeviceSize    size;
		VkDeviceSize    alignment;
		uint32_t        memoryTypeBits;
	} VkMemoryRequirements;
	*/
	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetBufferMemoryRequirements.html
	/*
	// Provided by VK_VERSION_1_0
	void vkGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements);
	*/
	vkGetImageMemoryRequirements(gState_scene0.vkDevice, gState_scene0.vkImage_depth, &vkMemoryRequirements);
	
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryAllocateInfo.html
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryAllocateInfo {
		VkStructureType    sType;
		const void*        pNext;
		VkDeviceSize       allocationSize;
		uint32_t           memoryTypeIndex;
	} VkMemoryAllocateInfo;
	*/
	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceSize.html (vkMemoryRequirements allocates memory in regions.)
	
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //Initial value before entering into the loop
	for(uint32_t i =0; i < gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceMemoryProperties.html
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryRequirements.html
		{
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryType.html
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryPropertyFlagBits.html
			if(gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}			
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	
	/*
	// Provided by VK_VERSION_1_0
	VkResult vkAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory);
	*/
	vkResult = vkAllocateMemory(gState_scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gState_scene0.vkDeviceMemory_depth); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateMemory.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkAllocateMemory() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkAllocateMemory() succedded\n");
	}
	
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkBindBufferMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer, //whom to bind
    VkDeviceMemory                              memory, //what to bind
    VkDeviceSize                                memoryOffset);
	*/
	vkResult = vkBindImageMemory(gState_scene0.vkDevice, gState_scene0.vkImage_depth, gState_scene0.vkDeviceMemory_depth, 0); // We are binding device memory object handle with Vulkan buffer object handle. 
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkBindBufferMemory() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkBindBufferMemory() succedded\n");
	}
	
	//Create ImageView for above depth image
	//Declare  and initialize VkImageViewCreateInfo struct (https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageViewCreateInfo.html) except its ".image" member.
	//Initialize VkImageViewCreateInfo struct
	memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
	
	/*
	typedef struct VkImageViewCreateInfo {
    VkStructureType            sType;
    const void*                pNext;
    VkImageViewCreateFlags     flags;
    VkImage                    image;
    VkImageViewType            viewType;
    VkFormat                   format;
    VkComponentMapping         components;
    VkImageSubresourceRange    subresourceRange;
	} VkImageViewCreateInfo;
	*/
	
	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html
	vkImageViewCreateInfo.format = gState_scene0.vkFormat_depth;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkComponentMapping.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkComponentSwizzle.html
	//vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	//vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	//vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	//vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageSubresourceRange.html
	/*
	typedef struct VkImageSubresourceRange {
    VkImageAspectFlags    aspectMask;
    uint32_t              baseMipLevel;
    uint32_t              levelCount;
    uint32_t              baseArrayLayer;
    uint32_t              layerCount;
	} VkImageSubresourceRange;
	*/
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageAspectFlags.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageAspectFlagBits.html
	vkImageViewCreateInfo.subresourceRange.aspectMask =  VK_IMAGE_ASPECT_DEPTH_BIT|VK_IMAGE_ASPECT_STENCIL_BIT;
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageViewType.html
	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vkImageViewCreateInfo.image = gState_scene0.vkImage_depth;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateImageView.html
	vkResult = vkCreateImageView(gState_scene0.vkDevice, &vkImageViewCreateInfo, NULL, &gState_scene0.vkImageView_depth);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkCreateImageView() function failed with error code %d for depth image\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateImagesAndImageViews(): vkCreateImageView() succedded for depth image\n");
	}
	
	return vkResult;
}

VkResult GetSupportedDepthFormat(void)
{
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	////https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html
	VkFormat vkFormat_depth_array[] = 
	{ 
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};
	
	for(uint32_t i =0;i < (sizeof(vkFormat_depth_array)/sizeof(vkFormat_depth_array[0])); i++)
	{
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormatProperties.html
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormatFeatureFlags.html
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormatFeatureFlagBits.html
		VkFormatProperties vkFormatProperties;
		memset((void*)&vkFormatProperties, 0, sizeof(vkFormatProperties));
		
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormatProperties.html
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetPhysicalDeviceFormatProperties.html
		vkGetPhysicalDeviceFormatProperties(gState_scene0.vkPhysicalDevice_selected, vkFormat_depth_array[i], &vkFormatProperties);
		if(vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			gState_scene0.vkFormat_depth = vkFormat_depth_array[i];
			vkResult = VK_SUCCESS;
			break;
		}
	}
	
	return vkResult;
}

VkResult CreateCommandPool()
{
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	/*
	1. Declare and initialize VkCreateCommandPoolCreateInfo structure.
	https://registry.khronos.org/vulkan/specs/latest/man/html/VkCommandPoolCreateInfo.html
	
	typedef struct VkCommandPoolCreateInfo {
    VkStructureType             sType;
    const void*                 pNext;
    VkCommandPoolCreateFlags    flags;
    uint32_t                    queueFamilyIndex;
	} VkCommandPoolCreateInfo;
	
	*/
	VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
	memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
	
	vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommandPoolCreateInfo.pNext = NULL;
	/*
	This flag states that Vulkan should create such command pools which will contain such command buffers capable of reset and restart.
	These command buffers are long lived.
	Other transient one{transfer one} is short lived.
	*/
	vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkCommandPoolCreateFlagBits.html
	vkCommandPoolCreateInfo.queueFamilyIndex = gState_scene0.graphicsQuequeFamilyIndex_selected;
	
	/*
	2. Call VkCreateCommandPool to create command pool.
	https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/vkCreateCommandPool.html
	*/
	vkResult = vkCreateCommandPool(gState_scene0.vkDevice, &vkCommandPoolCreateInfo, NULL, &gState_scene0.vkCommandPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateCommandPool(): vkCreateCommandPool() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateCommandPool(): vkCreateCommandPool() succedded\n");
	}
	
	return vkResult;
}

VkResult CreateCommandBuffers(void)
{
	//Variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	
	/*
	1. Declare and initialize struct VkCommandBufferAllocateInfo (https://registry.khronos.org/vulkan/specs/latest/man/html/VkCommandBufferAllocateInfo.html)
	The number of command buffers are coventionally equal to number of swapchain images.
	
	typedef struct VkCommandBufferAllocateInfo {
    VkStructureType         sType;
    const void*             pNext;
    VkCommandPool           commandPool;
    VkCommandBufferLevel    level;
    uint32_t                commandBufferCount;
	} VkCommandBufferAllocateInfo;
	*/
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	//vkCommandBufferAllocateInfo.flags = 0;
	vkCommandBufferAllocateInfo.commandPool = gState_scene0.vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; //https://docs.vulkan.org/spec/latest/chapters/cmdbuffers.html#VkCommandBufferAllocateInfo
	vkCommandBufferAllocateInfo.commandBufferCount = 1;
	
	/*
	2. Declare command buffer array globally and allocate it to swapchain image count.
	*/
	gState_scene0.vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * gState_scene0.swapchainImageCount);
	//skipping error check for brevity
	
	/*
	3. In a loop , which is equal to gState_scene0.swapchainImageCount, allocate each command buffer in above array by using vkAllocateCommandBuffers(). //https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateCommandBuffers.html
   Remember at time of allocation all commandbuffers will be empty.
   Later we will record graphic/compute commands into them.
	*/
	for(uint32_t i = 0; i < gState_scene0.swapchainImageCount; i++)
	{
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateCommandBuffers.html
		vkResult = vkAllocateCommandBuffers(gState_scene0.vkDevice, &vkCommandBufferAllocateInfo, &gState_scene0.vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "CreateCommandBuffers(): vkAllocateCommandBuffers() function failed with error code %d at iteration %d\n", vkResult, i);
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "CreateCommandBuffers(): vkAllocateCommandBuffers() succedded for iteration %d\n", i);
		}
	}
	
	return vkResult;
}

/*
2. Declare User defined function CreateVertexBuffer().
   Write its prototype below CreateCommandBuffers() and above CreateRenderPass() and also call it between the calls of these two.
*/
VkResult CreateVertexBuffer(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code for Position
	*/

	/*
	22.4. memset our global vertexData_position.
	*/
	memset((void*)&gState_scene0.vertexdata_position, 0, sizeof(GlobalState_Scene0::VertexData));
	
	/*
	22.5. Declare and memset VkBufferCreateInfo struct.
	It has 8 members, we will use 5
	Out of them, 2 are very important (Usage and Size)
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferCreateInfo.html
	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
	
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkBufferCreateInfo {
		VkStructureType        sType;
		const void*            pNext;
		VkBufferCreateFlags    flags;
		VkDeviceSize           size;
		VkBufferUsageFlags     usage;
		VkSharingMode          sharingMode;
		uint32_t               queueFamilyIndexCount;
		const uint32_t*        pQueueFamilyIndices;
	} VkBufferCreateInfo;
	*/
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //Valid flags are used in scattered(sparse) buffer
	vkBufferCreateInfo.size = sizeof(gState_scene0.sphere_vertices);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferUsageFlagBits.html;
	/* //when one buffer shared in multiple queque's
	vkBufferCreateInfo.sharingMode =;
	vkBufferCreateInfo.queueFamilyIndexCount =;
	vkBufferCreateInfo.pQueueFamilyIndices =; 
	*/
	
	
	/*
	22.6. Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct
	// Provided by VK_VERSION_1_0
	VkResult vkCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer);
	*/
	vkResult = vkCreateBuffer(gState_scene0.vkDevice, &vkBufferCreateInfo, NULL, &gState_scene0.vertexdata_position.vkBuffer); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkCreateBuffer() function for gState_scene0.vertexdata_position.vkBuffer failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkCreateBuffer() succedded for gState_scene0.vertexdata_position.vkBuffer\n");
	}
	
	/*
	22.7. Declare and member memset struct VkMemoryRequirements and then call vkGetBufferMemoryRequirements() API to get the memory requirements.
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryRequirements {
		VkDeviceSize    size;
		VkDeviceSize    alignment;
		uint32_t        memoryTypeBits;
	} VkMemoryRequirements;
	*/
	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetBufferMemoryRequirements.html
	/*
	// Provided by VK_VERSION_1_0
	void vkGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements);
	*/
	vkGetBufferMemoryRequirements(gState_scene0.vkDevice, gState_scene0.vertexdata_position.vkBuffer, &vkMemoryRequirements);
	
	/*
	   22.8. To actually allocate the required memory, we need to call vkAllocateMemory().
	   But before that we need to declare and memset VkMemoryAllocateInfo structure.
	   Important members of this structure are ".memoryTypeIndex" and ".allocationSize".
	   For ".allocationSize", use the size obtained from vkGetBufferMemoryRequirements().
	   For ".memoryTypeIndex" : 
	   a. Start a loop with count as vkPkysicalDeviceMemoryProperties.memoryTypeCount.
	   b. Inside the loop check vkMemoryRequiremts.memoryTypeBits contain 1 or not.
	   c. If yes, Check gState_scene0.vkPhysicalDeviceMemoryProperties.memeoryTypes[i].propertyFlags member contains enum VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
	   d. Then this ith index will be our ".memoryTypeIndex".
		  If found, break out of the loop.
	   e. If not continue the loop by right shifting VkMemoryRequirements.memoryTypeBits by 1, over each iteration.
	*/
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryAllocateInfo.html
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryAllocateInfo {
		VkStructureType    sType;
		const void*        pNext;
		VkDeviceSize       allocationSize;
		uint32_t           memoryTypeIndex;
	} VkMemoryAllocateInfo;
	*/
	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceSize.html (vkMemoryRequirements allocates memory in regions.)
	
	/*
	   22.8. To actually allocate the required memory, we need to call vkAllocateMemory().
	   But before that we need to declare and memset VkMemoryAllocateInfo structure.
	   Important members of this structure are ".memoryTypeIndex" and ".allocationSize".
	   For ".allocationSize", use the size obtained from vkGetBufferMemoryRequirements().
	   For ".memoryTypeIndex" : 
	   a. Start a loop with count as gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount.
	   b. Inside the loop check vkMemoryRequiremts.memoryTypeBits contain 1 or not.
	   c. If yes, Check gState_scene0.vkPhysicalDeviceMemoryProperties.memeoryTypes[i].propertyFlags member contains enum VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
	   d. Then this ith index will be our ".memoryTypeIndex".
		  If found, break out of the loop.
	   e. If not continue the loop by right shifting VkMemoryRequirements.memoryTypeBits by 1, over each iteration.
	*/
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //Initial value before entering into the loop
	for(uint32_t i =0; i < gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceMemoryProperties.html
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryRequirements.html
		{
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryType.html
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryPropertyFlagBits.html
			if(gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}			
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	
	/*
	22.9. Now call vkAllocateMemory()  and get the required Vulkan memory objects handle into the ".vkDeviceMemory" member of put global structure.
	// Provided by VK_VERSION_1_0
	VkResult vkAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory);
	*/
	vkResult = vkAllocateMemory(gState_scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gState_scene0.vertexdata_position.vkDeviceMemory); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateMemory.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkAllocateMemory() function failed for gState_scene0.vertexdata_position.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkAllocateMemory() succedded for gState_scene0.vertexdata_position.vkBuffer\n");
	}
	
	/*
	22.10. Now we have our required deviceMemory handle as well as VkBuffer Handle.
	Bind this device memory handle to VkBuffer Handle by using vkBindBufferMemory().
	Declare a void* buffer say "data" and call vkMapMemory() to map our device memory object handle to this void* buffer data.
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkBindBufferMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer, //whom to bind
    VkDeviceMemory                              memory, //what to bind
    VkDeviceSize                                memoryOffset);
	*/
	vkResult = vkBindBufferMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_position.vkBuffer, gState_scene0.vertexdata_position.vkDeviceMemory, 0); // We are binding device memory object handle with Vulkan buffer object handle. 
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkBindBufferMemory() function failed for gState_scene0.vertexdata_position.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkBindBufferMemory() succedded for gState_scene0.vertexdata_position.vkBuffer\n");
	}
	
	/*
	22.11. This will allow us to do memory mapped IO means when we write on void* buffer data, it will get automatically written/copied on to device memory represented by device memory object handle.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkMapMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData);
	*/
	void* data = NULL;
	vkResult = vkMapMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkMapMemory() function failed for gState_scene0.vertexdata_position.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkMapMemory() succedded for gState_scene0.vertexdata_position.vkBuffer\n");
	}
	
	/*
	22.12. Now to do actual memory mapped IO, call memcpy.
	*/
	memcpy(data, gState_scene0.sphere_vertices, sizeof(gState_scene0.sphere_vertices));
	
	/*
	22.13. To complete this memory mapped IO. finally call vkUmmapMemory() API.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkUnmapMemory.html
	// Provided by VK_VERSION_1_0
	void vkUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory);
	*/
	vkUnmapMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_position.vkDeviceMemory);
	
	////////////////////////////////////////////////////////////////////////////////////////
	/*
	Code for Normals
	*/

	/*
	22.4. memset our global vertexData_position.
	*/
	memset((void*)&gState_scene0.vertexdata_normals, 0, sizeof(GlobalState_Scene0::VertexData));
	
	/*
	22.5. Declare and memset VkBufferCreateInfo struct.
	It has 8 members, we will use 5
	Out of them, 2 are very important (Usage and Size)
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferCreateInfo.html
	//VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
	
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkBufferCreateInfo {
		VkStructureType        sType;
		const void*            pNext;
		VkBufferCreateFlags    flags;
		VkDeviceSize           size;
		VkBufferUsageFlags     usage;
		VkSharingMode          sharingMode;
		uint32_t               queueFamilyIndexCount;
		const uint32_t*        pQueueFamilyIndices;
	} VkBufferCreateInfo;
	*/
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //Valid flags are used in scattered(sparse) buffer
	vkBufferCreateInfo.size = sizeof(gState_scene0.sphere_normals);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferUsageFlagBits.html;
	/* //when one buffer shared in multiple queque's
	vkBufferCreateInfo.sharingMode =;
	vkBufferCreateInfo.queueFamilyIndexCount =;
	vkBufferCreateInfo.pQueueFamilyIndices =; 
	*/
	
	
	/*
	22.6. Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct
	// Provided by VK_VERSION_1_0
	VkResult vkCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer);
	*/
	vkResult = vkCreateBuffer(gState_scene0.vkDevice, &vkBufferCreateInfo, NULL, &gState_scene0.vertexdata_normals.vkBuffer); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkCreateBuffer() function for gState_scene0.vertexdata_normals.vkBuffer failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkCreateBuffer() succedded for gState_scene0.vertexdata_normals.vkBuffer\n");
	}
	
	/*
	22.7. Declare and member memset struct VkMemoryRequirements and then call vkGetBufferMemoryRequirements() API to get the memory requirements.
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryRequirements {
		VkDeviceSize    size;
		VkDeviceSize    alignment;
		uint32_t        memoryTypeBits;
	} VkMemoryRequirements;
	*/
	//VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetBufferMemoryRequirements.html
	/*
	// Provided by VK_VERSION_1_0
	void vkGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements);
	*/
	vkGetBufferMemoryRequirements(gState_scene0.vkDevice, gState_scene0.vertexdata_normals.vkBuffer, &vkMemoryRequirements);
	
	/*
	   22.8. To actually allocate the required memory, we need to call vkAllocateMemory().
	   But before that we need to declare and memset VkMemoryAllocateInfo structure.
	   Important members of this structure are ".memoryTypeIndex" and ".allocationSize".
	   For ".allocationSize", use the size obtained from vkGetBufferMemoryRequirements().
	   For ".memoryTypeIndex" : 
	   a. Start a loop with count as vkPkysicalDeviceMemoryProperties.memoryTypeCount.
	   b. Inside the loop check vkMemoryRequiremts.memoryTypeBits contain 1 or not.
	   c. If yes, Check gState_scene0.vkPhysicalDeviceMemoryProperties.memeoryTypes[i].propertyFlags member contains enum VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
	   d. Then this ith index will be our ".memoryTypeIndex".
		  If found, break out of the loop.
	   e. If not continue the loop by right shifting VkMemoryRequirements.memoryTypeBits by 1, over each iteration.
	*/
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryAllocateInfo.html
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryAllocateInfo {
		VkStructureType    sType;
		const void*        pNext;
		VkDeviceSize       allocationSize;
		uint32_t           memoryTypeIndex;
	} VkMemoryAllocateInfo;
	*/
	//VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceSize.html (vkMemoryRequirements allocates memory in regions.)
	
	/*
	   22.8. To actually allocate the required memory, we need to call vkAllocateMemory().
	   But before that we need to declare and memset VkMemoryAllocateInfo structure.
	   Important members of this structure are ".memoryTypeIndex" and ".allocationSize".
	   For ".allocationSize", use the size obtained from vkGetBufferMemoryRequirements().
	   For ".memoryTypeIndex" : 
	   a. Start a loop with count as gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount.
	   b. Inside the loop check vkMemoryRequiremts.memoryTypeBits contain 1 or not.
	   c. If yes, Check gState_scene0.vkPhysicalDeviceMemoryProperties.memeoryTypes[i].propertyFlags member contains enum VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
	   d. Then this ith index will be our ".memoryTypeIndex".
		  If found, break out of the loop.
	   e. If not continue the loop by right shifting VkMemoryRequirements.memoryTypeBits by 1, over each iteration.
	*/
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //Initial value before entering into the loop
	for(uint32_t i =0; i < gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceMemoryProperties.html
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryRequirements.html
		{
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryType.html
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryPropertyFlagBits.html
			if(gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}			
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	
	/*
	22.9. Now call vkAllocateMemory()  and get the required Vulkan memory objects handle into the ".vkDeviceMemory" member of put global structure.
	// Provided by VK_VERSION_1_0
	VkResult vkAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory);
	*/
	vkResult = vkAllocateMemory(gState_scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gState_scene0.vertexdata_normals.vkDeviceMemory); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateMemory.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkAllocateMemory() function failed for gState_scene0.vertexdata_normals.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkAllocateMemory() succedded for gState_scene0.vertexdata_normals.vkBuffer\n");
	}
	
	/*
	22.10. Now we have our required deviceMemory handle as well as VkBuffer Handle.
	Bind this device memory handle to VkBuffer Handle by using vkBindBufferMemory().
	Declare a void* buffer say "data" and call vkMapMemory() to map our device memory object handle to this void* buffer data.
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkBindBufferMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer, //whom to bind
    VkDeviceMemory                              memory, //what to bind
    VkDeviceSize                                memoryOffset);
	*/
	vkResult = vkBindBufferMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_normals.vkBuffer, gState_scene0.vertexdata_normals.vkDeviceMemory, 0); // We are binding device memory object handle with Vulkan buffer object handle. 
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkBindBufferMemory() function failed for gState_scene0.vertexdata_normals.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkBindBufferMemory() succedded for gState_scene0.vertexdata_normals.vkBuffer\n");
	}
	
	/*
	22.11. This will allow us to do memory mapped IO means when we write on void* buffer data, it will get automatically written/copied on to device memory represented by device memory object handle.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkMapMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData);
	*/
	//void* data = NULL;
	data = NULL;
	vkResult = vkMapMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_normals.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkMapMemory() function failed for gState_scene0.vertexdata_normals.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkMapMemory() succedded for gState_scene0.vertexdata_normals.vkBuffer\n");
	}
	
	/*
	22.12. Now to do actual memory mapped IO, call memcpy.
	*/
	memcpy(data, gState_scene0.sphere_normals, sizeof(gState_scene0.sphere_normals));
	
	/*
	22.13. To complete this memory mapped IO. finally call vkUmmapMemory() API.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkUnmapMemory.html
	// Provided by VK_VERSION_1_0
	void vkUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory);
	*/
	vkUnmapMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_normals.vkDeviceMemory);
	////////////////////////////////////////////////////////////////////////////////////////
	/*
	Code for Texcoords
	*/

	/*
	22.4. memset our global vertexData_position.
	*/
	memset((void*)&gState_scene0.vertexdata_texcoord, 0, sizeof(GlobalState_Scene0::VertexData));
	
	/*
	22.5. Declare and memset VkBufferCreateInfo struct.
	It has 8 members, we will use 5
	Out of them, 2 are very important (Usage and Size)
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferCreateInfo.html
	//VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
	
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkBufferCreateInfo {
		VkStructureType        sType;
		const void*            pNext;
		VkBufferCreateFlags    flags;
		VkDeviceSize           size;
		VkBufferUsageFlags     usage;
		VkSharingMode          sharingMode;
		uint32_t               queueFamilyIndexCount;
		const uint32_t*        pQueueFamilyIndices;
	} VkBufferCreateInfo;
	*/
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //Valid flags are used in scattered(sparse) buffer
	vkBufferCreateInfo.size = sizeof(gState_scene0.sphere_textures);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferUsageFlagBits.html;
	/* //when one buffer shared in multiple queque's
	vkBufferCreateInfo.sharingMode =;
	vkBufferCreateInfo.queueFamilyIndexCount =;
	vkBufferCreateInfo.pQueueFamilyIndices =; 
	*/
	
	
	/*
	22.6. Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct
	// Provided by VK_VERSION_1_0
	VkResult vkCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer);
	*/
	vkResult = vkCreateBuffer(gState_scene0.vkDevice, &vkBufferCreateInfo, NULL, &gState_scene0.vertexdata_texcoord.vkBuffer); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkCreateBuffer() function for gState_scene0.vertexdata_texcoord.vkBuffer failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkCreateBuffer() succedded for gState_scene0.vertexdata_texcoord.vkBuffer\n");
	}
	
	/*
	22.7. Declare and member memset struct VkMemoryRequirements and then call vkGetBufferMemoryRequirements() API to get the memory requirements.
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryRequirements {
		VkDeviceSize    size;
		VkDeviceSize    alignment;
		uint32_t        memoryTypeBits;
	} VkMemoryRequirements;
	*/
	//VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetBufferMemoryRequirements.html
	/*
	// Provided by VK_VERSION_1_0
	void vkGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements);
	*/
	vkGetBufferMemoryRequirements(gState_scene0.vkDevice, gState_scene0.vertexdata_texcoord.vkBuffer, &vkMemoryRequirements);
	
	/*
	   22.8. To actually allocate the required memory, we need to call vkAllocateMemory().
	   But before that we need to declare and memset VkMemoryAllocateInfo structure.
	   Important members of this structure are ".memoryTypeIndex" and ".allocationSize".
	   For ".allocationSize", use the size obtained from vkGetBufferMemoryRequirements().
	   For ".memoryTypeIndex" : 
	   a. Start a loop with count as vkPkysicalDeviceMemoryProperties.memoryTypeCount.
	   b. Inside the loop check vkMemoryRequiremts.memoryTypeBits contain 1 or not.
	   c. If yes, Check gState_scene0.vkPhysicalDeviceMemoryProperties.memeoryTypes[i].propertyFlags member contains enum VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
	   d. Then this ith index will be our ".memoryTypeIndex".
		  If found, break out of the loop.
	   e. If not continue the loop by right shifting VkMemoryRequirements.memoryTypeBits by 1, over each iteration.
	*/
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryAllocateInfo.html
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryAllocateInfo {
		VkStructureType    sType;
		const void*        pNext;
		VkDeviceSize       allocationSize;
		uint32_t           memoryTypeIndex;
	} VkMemoryAllocateInfo;
	*/
	//VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceSize.html (vkMemoryRequirements allocates memory in regions.)
	
	/*
	   22.8. To actually allocate the required memory, we need to call vkAllocateMemory().
	   But before that we need to declare and memset VkMemoryAllocateInfo structure.
	   Important members of this structure are ".memoryTypeIndex" and ".allocationSize".
	   For ".allocationSize", use the size obtained from vkGetBufferMemoryRequirements().
	   For ".memoryTypeIndex" : 
	   a. Start a loop with count as gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount.
	   b. Inside the loop check vkMemoryRequiremts.memoryTypeBits contain 1 or not.
	   c. If yes, Check gState_scene0.vkPhysicalDeviceMemoryProperties.memeoryTypes[i].propertyFlags member contains enum VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
	   d. Then this ith index will be our ".memoryTypeIndex".
		  If found, break out of the loop.
	   e. If not continue the loop by right shifting VkMemoryRequirements.memoryTypeBits by 1, over each iteration.
	*/
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //Initial value before entering into the loop
	for(uint32_t i =0; i < gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceMemoryProperties.html
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryRequirements.html
		{
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryType.html
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryPropertyFlagBits.html
			if(gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}			
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	
	/*
	22.9. Now call vkAllocateMemory()  and get the required Vulkan memory objects handle into the ".vkDeviceMemory" member of put global structure.
	// Provided by VK_VERSION_1_0
	VkResult vkAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory);
	*/
	vkResult = vkAllocateMemory(gState_scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gState_scene0.vertexdata_texcoord.vkDeviceMemory); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateMemory.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkAllocateMemory() function failed for gState_scene0.vertexdata_texcoord.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkAllocateMemory() succedded for gState_scene0.vertexdata_texcoord.vkBuffer\n");
	}
	
	/*
	22.10. Now we have our required deviceMemory handle as well as VkBuffer Handle.
	Bind this device memory handle to VkBuffer Handle by using vkBindBufferMemory().
	Declare a void* buffer say "data" and call vkMapMemory() to map our device memory object handle to this void* buffer data.
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkBindBufferMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer, //whom to bind
    VkDeviceMemory                              memory, //what to bind
    VkDeviceSize                                memoryOffset);
	*/
	vkResult = vkBindBufferMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_texcoord.vkBuffer, gState_scene0.vertexdata_texcoord.vkDeviceMemory, 0); // We are binding device memory object handle with Vulkan buffer object handle. 
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkBindBufferMemory() function failed for gState_scene0.vertexdata_texcoord.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkBindBufferMemory() succedded for gState_scene0.vertexdata_texcoord.vkBuffer\n");
	}
	
	/*
	22.11. This will allow us to do memory mapped IO means when we write on void* buffer data, it will get automatically written/copied on to device memory represented by device memory object handle.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkMapMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData);
	*/
	//void* data = NULL;
	data = NULL;
	vkResult = vkMapMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_texcoord.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkMapMemory() function failed for gState_scene0.vertexdata_texcoord.vkBuffer with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateVertexBuffer(): vkMapMemory() succedded for gState_scene0.vertexdata_texcoord.vkBuffer\n");
	}
	
	/*
	22.12. Now to do actual memory mapped IO, call memcpy.
	*/
	memcpy(data, gState_scene0.sphere_textures, sizeof(gState_scene0.sphere_textures));
	
	/*
	22.13. To complete this memory mapped IO. finally call vkUmmapMemory() API.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkUnmapMemory.html
	// Provided by VK_VERSION_1_0
	void vkUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory);
	*/
	vkUnmapMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_texcoord.vkDeviceMemory);
	
	return vkResult;
}

VkResult CreateIndexBuffer(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	//Code
	/*
	Vertex Index Buffer
	*/

	memset((void*)&gState_scene0.vertexdata_index, 0, sizeof(GlobalState_Scene0::VertexData));
	
	/*
	Declare and memset VkBufferCreateInfo struct.
	It has 8 members, we will use 5
	Out of them, 2 are very important (Usage and Size)
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferCreateInfo.html
	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
	
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkBufferCreateInfo {
		VkStructureType        sType;
		const void*            pNext;
		VkBufferCreateFlags    flags;
		VkDeviceSize           size;
		VkBufferUsageFlags     usage;
		VkSharingMode          sharingMode;
		uint32_t               queueFamilyIndexCount;
		const uint32_t*        pQueueFamilyIndices;
	} VkBufferCreateInfo;
	*/
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //Valid flags are used in scattered(sparse) buffer
	vkBufferCreateInfo.size = sizeof(gState_scene0.sphere_elements);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferUsageFlagBits.html
	/* //when one buffer shared in multiple queque's
	vkBufferCreateInfo.sharingMode =;
	vkBufferCreateInfo.queueFamilyIndexCount =;
	vkBufferCreateInfo.pQueueFamilyIndices =; 
	*/
	
	/*
	Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct
	// Provided by VK_VERSION_1_0
	VkResult vkCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer);
	*/
	vkResult = vkCreateBuffer(gState_scene0.vkDevice, &vkBufferCreateInfo, NULL, &gState_scene0.vertexdata_index.vkBuffer); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateIndexBuffer(): vkCreateBuffer() function failed with error code %d for position index buffer\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateIndexBuffer(): vkCreateBuffer() succedded for position index buffer\n");
	}
	
	/*
	Declare and member memset struct VkMemoryRequirements and then call vkGetBufferMemoryRequirements() API to get the memory requirements.
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryRequirements {
		VkDeviceSize    size;
		VkDeviceSize    alignment;
		uint32_t        memoryTypeBits;
	} VkMemoryRequirements;
	*/
	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetBufferMemoryRequirements.html
	/*
	// Provided by VK_VERSION_1_0
	void vkGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements);
	*/
	vkGetBufferMemoryRequirements(gState_scene0.vkDevice, gState_scene0.vertexdata_index.vkBuffer, &vkMemoryRequirements);
	
	/*
	   22.8. To actually allocate the required memory, we need to call vkAllocateMemory().
	   But before that we need to declare and memset VkMemoryAllocateInfo structure.
	   Important members of this structure are ".memoryTypeIndex" and ".allocationSize".
	   For ".allocationSize", use the size obtained from vkGetBufferMemoryRequirements().
	   For ".memoryTypeIndex" : 
	   a. Start a loop with count as vkPkysicalDeviceMemoryProperties.memoryTypeCount.
	   b. Inside the loop check vkMemoryRequiremts.memoryTypeBits contain 1 or not.
	   c. If yes, Check gState_scene0.vkPhysicalDeviceMemoryProperties.memeoryTypes[i].propertyFlags member contains enum VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
	   d. Then this ith index will be our ".memoryTypeIndex".
		  If found, break out of the loop.
	   e. If not continue the loop by right shifting VkMemoryRequirements.memoryTypeBits by 1, over each iteration.
	*/
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryAllocateInfo.html
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryAllocateInfo {
		VkStructureType    sType;
		const void*        pNext;
		VkDeviceSize       allocationSize;
		uint32_t           memoryTypeIndex;
	} VkMemoryAllocateInfo;
	*/
	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceSize.html (vkMemoryRequirements allocates memory in regions.)
	
	/*
	   To actually allocate the required memory, we need to call vkAllocateMemory().
	   But before that we need to declare and memset VkMemoryAllocateInfo structure.
	   Important members of this structure are ".memoryTypeIndex" and ".allocationSize".
	   For ".allocationSize", use the size obtained from vkGetBufferMemoryRequirements().
	   For ".memoryTypeIndex" : 
	   a. Start a loop with count as gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount.
	   b. Inside the loop check vkMemoryRequiremts.memoryTypeBits contain 1 or not.
	   c. If yes, Check gState_scene0.vkPhysicalDeviceMemoryProperties.memeoryTypes[i].propertyFlags member contains enum VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT.
	   d. Then this ith index will be our ".memoryTypeIndex".
		  If found, break out of the loop.
	   e. If not continue the loop by right shifting VkMemoryRequirements.memoryTypeBits by 1, over each iteration.
	*/
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //Initial value before entering into the loop
	for(uint32_t i =0; i < gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceMemoryProperties.html
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryRequirements.html
		{
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryType.html
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryPropertyFlagBits.html
			if(gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}			
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	
	/*
	Now call vkAllocateMemory()  and get the required Vulkan memory objects handle into the ".vkDeviceMemory" member of put global structure.
	// Provided by VK_VERSION_1_0
	VkResult vkAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory);
	*/
	vkResult = vkAllocateMemory(gState_scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gState_scene0.vertexdata_index.vkDeviceMemory); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateMemory.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateIndexBuffer(): vkAllocateMemory() function failed with error code %d for position index buffer\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateIndexBuffer(): vkAllocateMemory() succedded for position index buffer\n");
	}
	
	/*
	Now we have our required deviceMemory handle as well as VkBuffer Handle.
	Bind this device memory handle to VkBuffer Handle by using vkBindBufferMemory().
	Declare a void* buffer say "data" and call vkMapMemory() to map our device memory object handle to this void* buffer data.
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkBindBufferMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer, //whom to bind
    VkDeviceMemory                              memory, //what to bind
    VkDeviceSize                                memoryOffset);
	*/
	vkResult = vkBindBufferMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_index.vkBuffer, gState_scene0.vertexdata_index.vkDeviceMemory, 0); // We are binding device memory object handle with Vulkan buffer object handle. 
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateIndexBuffer(): vkBindBufferMemory() function failed with error code %d for position index buffer\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateIndexBuffer(): vkBindBufferMemory() succedded for position index buffer\n");
	}
	
	/*
	This will allow us to do memory mapped IO means when we write on void* buffer data, it will get automatically written/copied on to device memory represented by device memory object handle.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkMapMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkMapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory,
    VkDeviceSize                                offset,
    VkDeviceSize                                size,
    VkMemoryMapFlags                            flags,
    void**                                      ppData);
	*/
	void *data = NULL;
	vkResult = vkMapMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_index.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateIndexBuffer(): vkMapMemory() function failed with error code %d for position index buffer\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateIndexBuffer(): vkMapMemory() succedded for position index buffer\n");
	}
	
	/*
	Now to do actual memory mapped IO, call memcpy.
	*/
	memcpy(data, gState_scene0.sphere_elements, sizeof(gState_scene0.sphere_elements));
	
	/*
	To complete this memory mapped IO. finally call vkUmmapMemory() API.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkUnmapMemory.html
	// Provided by VK_VERSION_1_0
	void vkUnmapMemory(
    VkDevice                                    device,
    VkDeviceMemory                              memory);
	*/
	vkUnmapMemory(gState_scene0.vkDevice, gState_scene0.vertexdata_index.vkDeviceMemory);
	
	return vkResult;
}

//31.11
VkResult CreateUniformBuffer()
{
        //Variable declarations
        VkResult vkResult = VK_SUCCESS;
	
	//Code
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferCreateInfo.html
	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void*)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));
	
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkBufferCreateInfo {
		VkStructureType        sType;
		const void*            pNext;
		VkBufferCreateFlags    flags;
		VkDeviceSize           size;
		VkBufferUsageFlags     usage;
		VkSharingMode          sharingMode;
		uint32_t               queueFamilyIndexCount;
		const uint32_t*        pQueueFamilyIndices;
	} VkBufferCreateInfo;
	*/
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //Valid flags are used in scattered(sparse) buffer
	vkBufferCreateInfo.size = sizeof(GlobalState_Scene0::MyUniformData);
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBufferUsageFlagBits.html;
	/* //when one buffer shared in multiple queque's
	vkBufferCreateInfo.sharingMode =;
	vkBufferCreateInfo.queueFamilyIndexCount =;
	vkBufferCreateInfo.pQueueFamilyIndices =; 
	*/
	
	memset((void*)&gState_scene0.uniformData, 0, sizeof(GlobalState_Scene0::UniformData));
	
	/*
	// Provided by VK_VERSION_1_0
	VkResult vkCreateBuffer(
    VkDevice                                    device,
    const VkBufferCreateInfo*                   pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkBuffer*                                   pBuffer);
	*/
	vkResult = vkCreateBuffer(gState_scene0.vkDevice, &vkBufferCreateInfo, NULL, &gState_scene0.uniformData.vkBuffer); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateBuffer.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateUniformBuffer(): vkCreateBuffer() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateUniformBuffer(): vkCreateBuffer() succedded\n");
	}
	
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryRequirements {
		VkDeviceSize    size;
		VkDeviceSize    alignment;
		uint32_t        memoryTypeBits;
	} VkMemoryRequirements;
	*/
	VkMemoryRequirements vkMemoryRequirements;
	memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkGetBufferMemoryRequirements.html
	/*
	// Provided by VK_VERSION_1_0
	void vkGetBufferMemoryRequirements(
    VkDevice                                    device,
    VkBuffer                                    buffer,
    VkMemoryRequirements*                       pMemoryRequirements);
	*/
	vkGetBufferMemoryRequirements(gState_scene0.vkDevice, gState_scene0.uniformData.vkBuffer, &vkMemoryRequirements);
	
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryAllocateInfo.html
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkMemoryAllocateInfo {
		VkStructureType    sType;
		const void*        pNext;
		VkDeviceSize       allocationSize;
		uint32_t           memoryTypeIndex;
	} VkMemoryAllocateInfo;
	*/
	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void*)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));
	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceSize.html (vkMemoryRequirements allocates memory in regions.)
	
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //Initial value before entering into the loop
	for(uint32_t i =0; i < gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPhysicalDeviceMemoryProperties.html
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1) //https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryRequirements.html
		{
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryType.html
			//https://registry.khronos.org/vulkan/specs/latest/man/html/VkMemoryPropertyFlagBits.html
			if(gState_scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				break;
			}			
		}
		vkMemoryRequirements.memoryTypeBits >>= 1;
	}
	
	/*
	// Provided by VK_VERSION_1_0
	VkResult vkAllocateMemory(
    VkDevice                                    device,
    const VkMemoryAllocateInfo*                 pAllocateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDeviceMemory*                             pMemory);
	*/
	vkResult = vkAllocateMemory(gState_scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gState_scene0.uniformData.vkDeviceMemory); //https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateMemory.html
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateUniformBuffer(): vkAllocateMemory() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateUniformBuffer(): vkAllocateMemory() succedded\n");
	}
	
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkBindBufferMemory.html
	// Provided by VK_VERSION_1_0
	VkResult vkBindBufferMemory(
    VkDevice                                    device,
    VkBuffer                                    buffer, //whom to bind
    VkDeviceMemory                              memory, //what to bind
    VkDeviceSize                                memoryOffset);
	*/
	vkResult = vkBindBufferMemory(gState_scene0.vkDevice, gState_scene0.uniformData.vkBuffer, gState_scene0.uniformData.vkDeviceMemory, 0); // We are binding device memory object handle with Vulkan buffer object handle. 
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateUniformBuffer(): vkBindBufferMemory() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateUniformBuffer(): vkBindBufferMemory() succedded\n");
	}
	
	//Call updateUniformBuffer() here
        vkResult = gVulkanFunctions_Scene0.updateUniformBuffer();
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateUniformBuffer(): updateUniformBuffer() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateUniformBuffer(): updateUniformBuffer() succedded\n");
	}
	
	return vkResult;
}

/*
23.5. Maintaining the same baove convention while defining CreateShaders() between definition of above two.
*/
VkResult CreateShaders(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code for Vertex Shader
	*/
	
	/*
	6. Inside our function, 
	first open shader file, 
	set the file pointer at end of file,
	find the byte size of shader file data,
	reset the file pointer at begining of the file,
	allocate a character buffer of file size and read Shader file data into it,
	and finally close the file.
	Do all these things using conventional fileIO.
	*/
	const char* szFileName = "Shader_Scene0.vert.spv";
	FILE* fp = NULL;
	size_t size;
	
	fp = fopen(szFileName, "rb");
	if(fp == NULL)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): failed to open Vertex Shader SPIRV file\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): sucedded to open Vertex Shader SPIRV file\n");
	}
	
	fseek(fp, 0L, SEEK_END);
	
	size = ftell(fp);
	if(size == 0)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): Vertex Shader SPIRV file size is 0\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	
	fseek(fp, 0L, SEEK_SET);
	
	char* shaderData = (char*)malloc(sizeof(char) * size);
	if(shaderData == NULL)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): malloc for Vertex Shader SPIRV file failed\n");
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): malloc for Vertex Shader SPIRV file done\n");
	}
	
	size_t retVal = fread(shaderData, size, 1, fp);
	if(retVal != 1)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): failed to read Vertex Shader SPIRV file\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): sucedded to read Vertex Shader SPIRV file\n");
	}
	
	fclose(fp);
	
	/*
	23.7. Declare and memset struct VkShaderModuleCreateInfo and specify above file size and buffer while initializing it.
	// Provided by VK_VERSION_1_0
	typedef struct VkShaderModuleCreateInfo {
		VkStructureType              sType;
		const void*                  pNext;
		VkShaderModuleCreateFlags    flags;
		size_t                       codeSize;
		const uint32_t*              pCode;
	} VkShaderModuleCreateInfo;
	*/
	VkShaderModuleCreateInfo vkShaderModuleCreateInfo; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkShaderModuleCreateInfo.html
	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0; //Reserved for future use. Hence must be 0
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;
	
	/*
	8. Call vkCreateShaderModule() Vulkan API, pass above struct's pointer to it as parameter and obtain shader module object in global variable, that we declared in Step 2.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateShaderModule.html
	// Provided by VK_VERSION_1_0
	VkResult vkCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule);
	*/
	vkResult = vkCreateShaderModule(gState_scene0.vkDevice, &vkShaderModuleCreateInfo, NULL, &gState_scene0.vkShaderMoudule_vertex_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): vkCreateShaderModule() function for vertex SPIRV shader file failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): vkCreateShaderModule() for vertex SPIRV shader file succedded\n");
	}
	
	/*
	9. Free the ShaderCode buffer which we allocated in Step 6.
	*/
	if(shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}
	fprintf(gState_scene0.gFILE, "CreateShaders(): vertex Shader module successfully created\n");
	
	/*
	23.10. Assuming we did above 4 steps 6 to 9 for Vertex Shader, Repeat them all for fragment shader too.
	Code for Fragment Shader
	*/
	
	/*
	6. Inside our function, 
	first open shader file, 
	set the file pointer at end of file,
	find the byte size of shader file data,
	reset the file pointer at begining of the file,
	allocate a character buffer of file size and read Shader file data into it,
	and finally close the file.
	Do all these things using conventional fileIO.
	*/
	szFileName = "Shader_Scene0.frag.spv";
	size = 0;
	fp = NULL;
	
	fp = fopen(szFileName, "rb");
	if(fp == NULL)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): failed to open Fragment Shader SPIRV file\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): sucedded to open Fragment Shader SPIRV file\n");
	}
	
	fseek(fp, 0L, SEEK_END);
	
	size = ftell(fp);
	if(size == 0)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): Fragment Shader SPIRV file size is 0\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	
	fseek(fp, 0L, SEEK_SET);
	
	shaderData = (char*)malloc(sizeof(char) * size);
	if(shaderData == NULL)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): malloc for Fragment Shader SPIRV file failed\n");
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): malloc for Fragment Shader SPIRV file done\n");
	}
	
	retVal = fread(shaderData, size, 1, fp);
	if(retVal != 1)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): failed to read Fragment Shader SPIRV file\n");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): sucedded to read Fragment Shader SPIRV file\n");
	}
	
	fclose(fp);
	
	/*
	23.7. Declare and memset struct VkShaderModuleCreateInfo and specify above file size and buffer while initializing it.
	// Provided by VK_VERSION_1_0
	typedef struct VkShaderModuleCreateInfo {
		VkStructureType              sType;
		const void*                  pNext;
		VkShaderModuleCreateFlags    flags;
		size_t                       codeSize;
		const uint32_t*              pCode;
	} VkShaderModuleCreateInfo;
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkShaderModuleCreateInfo.html
	memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0; //Reserved for future use. Hence must be 0
	vkShaderModuleCreateInfo.codeSize = size;
	vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;
	
	/*
	8. Call vkCreateShaderModule() Vulkan API, pass above struct's pointer to it as parameter and obtain shader module object in global variable, that we declared in Step 2.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateShaderModule.html
	// Provided by VK_VERSION_1_0
	VkResult vkCreateShaderModule(
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule);
	*/
	vkResult = vkCreateShaderModule(gState_scene0.vkDevice, &vkShaderModuleCreateInfo, NULL, &gState_scene0.vkShaderMoudule_fragment_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): vkCreateShaderModule() function for fragment SPIRV shader file failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateShaders(): vkCreateShaderModule() function for fragment SPIRV shader file succedded\n");
	}
	
	/*
	9. Free the ShaderCode buffer which we allocated in Step 6.
	*/
	if(shaderData)
	{
		free(shaderData);
		shaderData = NULL;
	}
	fprintf(gState_scene0.gFILE, "CreateShaders(): fragment Shader module successfully created\n");

	
	return vkResult;
}

/*
24.2. In initialize(), declare and call UDF CreateDescriptorSetLayout() maintaining the convention of declaring and calling it after CreateShaders() and before CreateRenderPass().
*/
VkResult CreateDescriptorSetLayout()
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	
	//Initialize descriptor set binding : //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorSetLayoutBinding.html
	VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding; 
	memset((void*)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorSetLayoutBinding {
		uint32_t              binding;
		VkDescriptorType      descriptorType;
		uint32_t              descriptorCount;
		VkShaderStageFlags    stageFlags;
		const VkSampler*      pImmutableSamplers;
	} VkDescriptorSetLayoutBinding;
	*/
	vkDescriptorSetLayoutBinding.binding = 0; //binding point kay aahe shader madhe. This 0 is related to binding =0 in vertex shader
	vkDescriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorType.html
	vkDescriptorSetLayoutBinding.descriptorCount = 1;
	vkDescriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkShaderStageFlagBits.html
	vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL;
	
	/*
	24.3. While writing this UDF, declare, memset and initialize struct VkDescriptorSetLayoutCreateInfo, particularly its two members 
	   1. bindingCount
	   2. pBindings array
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorSetLayoutCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorSetLayoutCreateInfo {
    VkStructureType                        sType;
    const void*                            pNext;
    VkDescriptorSetLayoutCreateFlags       flags;
    uint32_t                               bindingCount;
    const VkDescriptorSetLayoutBinding*    pBindings;
	} VkDescriptorSetLayoutCreateInfo;
	*/
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
	memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0; /*Since reserved*/
	
	/*
	pBindings is actually array of struct VkDescriptorSetLayoutBinding having 5 members
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorSetLayoutBinding.html
	// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorSetLayoutBinding {
    uint32_t              binding; //RTR madhe glGenBuffers(); glBindBuffer(binding point(1st parameter), ); //An interger value where you want to bind descriptor/descriptor set. (descriptor set expected)
    VkDescriptorType      descriptorType; 
    uint32_t              descriptorCount;
    VkShaderStageFlags    stageFlags;
    const VkSampler*      pImmutableSamplers;
	} VkDescriptorSetLayoutBinding;
	*/
	
	vkDescriptorSetLayoutCreateInfo.bindingCount = 1; //binding aahe ka
	vkDescriptorSetLayoutCreateInfo.pBindings = &vkDescriptorSetLayoutBinding;
	
	/*
	24.4. Then call vkCreateDescriptorSetLayout() Vulkan API with adress of above initialized structure and get the required global Vulkan object gState_scene0.vkDescriptorSetLayout in its last parameter.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDescriptorSetLayout.html
	// Provided by VK_VERSION_1_0
	VkResult vkCreateDescriptorSetLayout(
    VkDevice                                    device,
    const VkDescriptorSetLayoutCreateInfo*      pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorSetLayout*                      pSetLayout);
	*/
	vkResult = vkCreateDescriptorSetLayout(gState_scene0.vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &gState_scene0.vkDescriptorSetLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateDescriptorSetLayout(): vkCreateDescriptorSetLayout() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateDescriptorSetLayout(): vkCreateDescriptorSetLayout() function succedded\n");
	}
	
	return vkResult;
}

/*
25.2. In initialize(), declare and call UDF CreatePipelineLayout() maintaining the convention of declaring and calling it after CreatDescriptorSetLayout() and before CreateRenderPass().
*/
VkResult CreatePipelineLayout(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	
	/*
	25.3. While writing the definition of UDF, declare, memset and initialize struct VkPipelineLayoutCreateInfo , particularly its 4 important members 
	   1. .setLayoutCount
	   2. .pSetLayouts array
	   3. .pushConstantRangeCount
	   4. .pPushConstantRanges array
	//https://registry.khronos.org/VulkanSC/specs/1.0-extensions/man/html/VkPipelineLayoutCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineLayoutCreateInfo {
		VkStructureType                 sType;
		const void*                     pNext;
		VkPipelineLayoutCreateFlags     flags;
		uint32_t                        setLayoutCount;
		const VkDescriptorSetLayout*    pSetLayouts;
		uint32_t                        pushConstantRangeCount;
		const VkPushConstantRange*      pPushConstantRanges;
	} VkPipelineLayoutCreateInfo;
	*/
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.flags = 0; /* Reserved*/
	vkPipelineLayoutCreateInfo.setLayoutCount = 1;
	vkPipelineLayoutCreateInfo.pSetLayouts = &gState_scene0.vkDescriptorSetLayout;
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
	
	/*
	25.4. Then call vkCreatePipelineLayout() Vulkan API with adress of above initialized structure and get the required global Vulkan object gState_scene0.vkPipelineLayout in its last parameter.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreatePipelineLayout.html
	// Provided by VK_VERSION_1_0
	VkResult vkCreatePipelineLayout(
    VkDevice                                    device,
    const VkPipelineLayoutCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineLayout*                           pPipelineLayout);
	*/
	vkResult = vkCreatePipelineLayout(gState_scene0.vkDevice, &vkPipelineLayoutCreateInfo, NULL, &gState_scene0.vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreatePipelineLayout(): vkCreatePipelineLayout() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreatePipelineLayout(): vkCreatePipelineLayout() function succedded\n");
	}
	
	return vkResult;
}

//31.13
VkResult CreateDescriptorPool(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	/*
	//Before creating actual descriptor pool, Vulkan expects descriptor pool size
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorPoolSize.html
	// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorPoolSize {
		VkDescriptorType    type;
		uint32_t            descriptorCount;
	} VkDescriptorPoolSize;
	*/
	VkDescriptorPoolSize vkDescriptorPoolSize;
	memset((void*)&vkDescriptorPoolSize, 0, sizeof(VkDescriptorPoolSize));
	vkDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorType.html
	vkDescriptorPoolSize.descriptorCount = 1;
	
	/*
	//Create the pool
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorPoolCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorPoolCreateInfo {
		VkStructureType                sType;
		const void*                    pNext;
		VkDescriptorPoolCreateFlags    flags;
		uint32_t                       maxSets;
		uint32_t                       poolSizeCount;
		const VkDescriptorPoolSize*    pPoolSizes;
	} VkDescriptorPoolCreateInfo;
	*/
	VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
	memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
	vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkStructureType.html
	vkDescriptorPoolCreateInfo.pNext = NULL;
	vkDescriptorPoolCreateInfo.flags = 0;
	vkDescriptorPoolCreateInfo.maxSets = 1; //kiti sets pahije tumhala
	vkDescriptorPoolCreateInfo.poolSizeCount =  1;
	vkDescriptorPoolCreateInfo.pPoolSizes = &vkDescriptorPoolSize;
	
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateDescriptorPool.html
	// Provided by VK_VERSION_1_0
	VkResult vkCreateDescriptorPool(
    VkDevice                                    device,
    const VkDescriptorPoolCreateInfo*           pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkDescriptorPool*                           pDescriptorPool);
	*/
	vkResult = vkCreateDescriptorPool(gState_scene0.vkDevice, &vkDescriptorPoolCreateInfo, NULL, &gState_scene0.vkDescriptorPool);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateDescriptorPool(): vkCreateDescriptorPool() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateDescriptorPool(): vkCreateDescriptorPool() succedded\n");
	}
	
	return vkResult;
}

//31.14
VkResult CreateDescriptorSet(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	/*
	//Initialize descriptor set allocation info
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorSetAllocateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorSetAllocateInfo {
		VkStructureType                 sType;
		const void*                     pNext;
		VkDescriptorPool                descriptorPool;
		uint32_t                        descriptorSetCount;
		const VkDescriptorSetLayout*    pSetLayouts;
	} VkDescriptorSetAllocateInfo;
	*/
	VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
	memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
	vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	vkDescriptorSetAllocateInfo.pNext = NULL;
	vkDescriptorSetAllocateInfo.descriptorPool = gState_scene0.vkDescriptorPool;
	
	vkDescriptorSetAllocateInfo.descriptorSetCount = 1; //We are passing only 1 struct so put 1 here
	//we are giving descriptor setlayout's here for first time after Pipeline
	//Now plate is not empty, it has 1 descriptor
	//to bharnyasathi allocate karun de , 1 descriptor set bharnya sathi
	vkDescriptorSetAllocateInfo.pSetLayouts = &gState_scene0.vkDescriptorSetLayout; 
	
	/*
	//Jitha structure madhe point ani counter ekatra astat, tithe array expected astoch
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkAllocateDescriptorSets.html
	// Provided by VK_VERSION_1_0
	VkResult vkAllocateDescriptorSets(
    VkDevice                                    device,
    const VkDescriptorSetAllocateInfo*          pAllocateInfo,
    VkDescriptorSet*                            pDescriptorSets);
	*/
	vkResult = vkAllocateDescriptorSets(gState_scene0.vkDevice, &vkDescriptorSetAllocateInfo, &gState_scene0.vkDescriptorSet);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateDescriptorSet(): vkAllocateDescriptorSets() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateDescriptorSet(): vkAllocateDescriptorSets() succedded\n");
	}

	/*
	//Describe whether we want buffer as uniform /or image as uniform
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorBufferInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorBufferInfo {
		VkBuffer        buffer;
		VkDeviceSize    offset;
		VkDeviceSize    range;
	} VkDescriptorBufferInfo;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorImageInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkDescriptorImageInfo {
		VkSampler        sampler;
		VkImageView      imageView;
		VkImageLayout    imageLayout;
	} VkDescriptorImageInfo;
	*/
	VkDescriptorBufferInfo vkDescriptorBufferInfo;
	memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
	vkDescriptorBufferInfo.buffer = gState_scene0.uniformData.vkBuffer;
	vkDescriptorBufferInfo.offset = 0;
	vkDescriptorBufferInfo.range = sizeof(GlobalState_Scene0::MyUniformData);
	
	/*
	//Now update the above descriptor set directly to the shader
	//There are two ways to update 1. Writing directly to shader 2.Copying from one shader to another shader
	//We will prepare directly writing to the shader
	//This requires initialization of following structure
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkWriteDescriptorSet.html
	// Provided by VK_VERSION_1_0
	typedef struct VkWriteDescriptorSet {
		VkStructureType                  sType;
		const void*                      pNext;
		VkDescriptorSet                  dstSet;
		uint32_t                         dstBinding;
		uint32_t                         dstArrayElement;
		uint32_t                         descriptorCount;
		VkDescriptorType                 descriptorType;
		const VkDescriptorImageInfo*     pImageInfo;
		const VkDescriptorBufferInfo*    pBufferInfo;
		const VkBufferView*              pTexelBufferView; //Used for Texture tiling
	} VkWriteDescriptorSet;
	*/
	VkWriteDescriptorSet vkWriteDescriptorSet;
	memset((void*)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));
	vkWriteDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vkWriteDescriptorSet.pNext = NULL;
	vkWriteDescriptorSet.dstSet = gState_scene0.vkDescriptorSet;
	vkWriteDescriptorSet.dstBinding = 0; //because our uniform is at binding 0 index in shader
	vkWriteDescriptorSet.dstArrayElement = 0;
	vkWriteDescriptorSet.descriptorCount = 1;
	vkWriteDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkDescriptorType.html
	vkWriteDescriptorSet.pImageInfo = NULL;
	vkWriteDescriptorSet.pBufferInfo =  &vkDescriptorBufferInfo;
	vkWriteDescriptorSet.pTexelBufferView = NULL;
	
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkUpdateDescriptorSets.html
	// Provided by VK_VERSION_1_0
	void vkUpdateDescriptorSets(
    VkDevice                                    device,
    uint32_t                                    descriptorWriteCount,
    const VkWriteDescriptorSet*                 pDescriptorWrites,
    uint32_t                                    descriptorCopyCount,
    const VkCopyDescriptorSet*                  pDescriptorCopies);
	*/
	vkUpdateDescriptorSets(gState_scene0.vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);
	
	fprintf(gState_scene0.gFILE, "CreateDescriptorSet(): vkUpdateDescriptorSets() succedded\n");
	
	return vkResult;
}

VkResult CreateRenderPass(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	
	/*
	1. Declare and initialize VkAttachmentDescription Struct array. (https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentDescription.html)
    Number of elements in Array depends on number of attachments.
   (Although we have only 1 attachment i.e color attachment in this example, we will consider it as array)
   
   typedef struct VkAttachmentDescription {
    VkAttachmentDescriptionFlags    flags;
    VkFormat                        format;
    VkSampleCountFlagBits           samples;
    VkAttachmentLoadOp              loadOp;
    VkAttachmentStoreOp             storeOp;
    VkAttachmentLoadOp              stencilLoadOp;
    VkAttachmentStoreOp             stencilStoreOp;
    VkImageLayout                   initialLayout;
    VkImageLayout                   finalLayout;
	} VkAttachmentDescription;
	*/
	VkAttachmentDescription  vkAttachmentDescription_array[2]; //color and depth when added array will be of 2
	memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));
	
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentDescriptionFlagBits.html
	
	// Provided by VK_VERSION_1_0
	typedef enum VkAttachmentDescriptionFlagBits {
		VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT = 0x00000001,
	} VkAttachmentDescriptionFlagBits;
	
	Info on Sony japan company documentation of paper presentation.
	Mostly 0 , only for manging memory in embedded devices
	Multiple attachments jar astil , tar eka mekanchi memory vapru shaktat.
	*/
	vkAttachmentDescription_array[0].flags = 0; 
	
	vkAttachmentDescription_array[0].format = gState_scene0.vkFormat_color;

	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkSampleCountFlagBits.html
	/*
	// Provided by VK_VERSION_1_0
	typedef enum VkSampleCountFlagBits {
    VK_SAMPLE_COUNT_1_BIT = 0x00000001,
    VK_SAMPLE_COUNT_2_BIT = 0x00000002,
    VK_SAMPLE_COUNT_4_BIT = 0x00000004,
    VK_SAMPLE_COUNT_8_BIT = 0x00000008,
    VK_SAMPLE_COUNT_16_BIT = 0x00000010,
    VK_SAMPLE_COUNT_32_BIT = 0x00000020,
    VK_SAMPLE_COUNT_64_BIT = 0x00000040,
	} VkSampleCountFlagBits;
	
	https://www.google.com/search?q=sampling+meaning+in+texturw&oq=sampling+meaning+in+texturw&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBCTYzMjlqMGoxNagCCLACAQ&sourceid=chrome&ie=UTF-8
	*/
	vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT; // No MSAA
	
	// https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentLoadOp.html
	/*
	// Provided by VK_VERSION_1_0
	typedef enum VkAttachmentLoadOp {
		VK_ATTACHMENT_LOAD_OP_LOAD = 0,
		VK_ATTACHMENT_LOAD_OP_CLEAR = 1,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE = 2,
	  // Provided by VK_VERSION_1_4
		VK_ATTACHMENT_LOAD_OP_NONE = 1000400000,
	  // Provided by VK_EXT_load_store_op_none
		VK_ATTACHMENT_LOAD_OP_NONE_EXT = VK_ATTACHMENT_LOAD_OP_NONE,
	  // Provided by VK_KHR_load_store_op_none
		VK_ATTACHMENT_LOAD_OP_NONE_KHR = VK_ATTACHMENT_LOAD_OP_NONE,
	} VkAttachmentLoadOp;
	
	ya structure chi mahiti direct renderpass la jata.
	*/
	vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Render pass madhe aat aalyavar kay karu attachment cha image data sobat
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentStoreOp.html
	/*
	// Provided by VK_VERSION_1_0
	typedef enum VkAttachmentStoreOp {
    VK_ATTACHMENT_STORE_OP_STORE = 0,
    VK_ATTACHMENT_STORE_OP_DONT_CARE = 1,
  // Provided by VK_VERSION_1_3
    VK_ATTACHMENT_STORE_OP_NONE = 1000301000,
  // Provided by VK_KHR_dynamic_rendering, VK_KHR_load_store_op_none
    VK_ATTACHMENT_STORE_OP_NONE_KHR = VK_ATTACHMENT_STORE_OP_NONE,
  // Provided by VK_QCOM_render_pass_store_ops
    VK_ATTACHMENT_STORE_OP_NONE_QCOM = VK_ATTACHMENT_STORE_OP_NONE,
  // Provided by VK_EXT_load_store_op_none
    VK_ATTACHMENT_STORE_OP_NONE_EXT = VK_ATTACHMENT_STORE_OP_NONE,
	} VkAttachmentStoreOp;
	*/
	vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE; //Render pass madhun baher gelyavar kay karu attachment image data sobat
	
	vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // For both depth and stencil, dont go on name
	vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // For both depth and stencil, dont go on name
	
	/*
	https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageLayout.html
	he sarv attachment madhla data cha arrangement cha aahe
	Unpacking athva RTR cha , karan color attachment mhnaje mostly texture
	*/
	vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //Renderpass cha aat aalyavar , attachment cha data arrangemnent cha kay karu
	vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; //Renderpass cha baher gelyavar , attachment cha data arrangemnent cha kay karu
	/*
	jya praname soure image aage , taasach layout thevun present kar.
	Madhe kahi changes zale, source praname thev
	*/
	
	//For Depth
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentDescriptionFlagBits.html
	
	// Provided by VK_VERSION_1_0
	typedef enum VkAttachmentDescriptionFlagBits {
		VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT = 0x00000001,
	} VkAttachmentDescriptionFlagBits;
	
	Info on Sony japan company documentation of paper presentation.
	Mostly 0 , only for manging memory in embedded devices
	Multiple attachments jar astil , tar eka mekanchi memory vapru shaktat.
	*/
	vkAttachmentDescription_array[1].flags = 0; 
	
	vkAttachmentDescription_array[1].format = gState_scene0.vkFormat_depth;

	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkSampleCountFlagBits.html
	/*
	// Provided by VK_VERSION_1_0
	typedef enum VkSampleCountFlagBits {
    VK_SAMPLE_COUNT_1_BIT = 0x00000001,
    VK_SAMPLE_COUNT_2_BIT = 0x00000002,
    VK_SAMPLE_COUNT_4_BIT = 0x00000004,
    VK_SAMPLE_COUNT_8_BIT = 0x00000008,
    VK_SAMPLE_COUNT_16_BIT = 0x00000010,
    VK_SAMPLE_COUNT_32_BIT = 0x00000020,
    VK_SAMPLE_COUNT_64_BIT = 0x00000040,
	} VkSampleCountFlagBits;
	
	https://www.google.com/search?q=sampling+meaning+in+texturw&oq=sampling+meaning+in+texturw&gs_lcrp=EgZjaHJvbWUyBggAEEUYOdIBCTYzMjlqMGoxNagCCLACAQ&sourceid=chrome&ie=UTF-8
	*/
	vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT; // No MSAA
	
	// https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentLoadOp.html
	/*
	// Provided by VK_VERSION_1_0
	typedef enum VkAttachmentLoadOp {
		VK_ATTACHMENT_LOAD_OP_LOAD = 0,
		VK_ATTACHMENT_LOAD_OP_CLEAR = 1,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE = 2,
	  // Provided by VK_VERSION_1_4
		VK_ATTACHMENT_LOAD_OP_NONE = 1000400000,
	  // Provided by VK_EXT_load_store_op_none
		VK_ATTACHMENT_LOAD_OP_NONE_EXT = VK_ATTACHMENT_LOAD_OP_NONE,
	  // Provided by VK_KHR_load_store_op_none
		VK_ATTACHMENT_LOAD_OP_NONE_KHR = VK_ATTACHMENT_LOAD_OP_NONE,
	} VkAttachmentLoadOp;
	
	ya structure chi mahiti direct renderpass la jata.
	*/
	vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; //Render pass madhe aat aalyavar kay karu attachment cha image data sobat
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentStoreOp.html
	/*
	// Provided by VK_VERSION_1_0
	typedef enum VkAttachmentStoreOp {
    VK_ATTACHMENT_STORE_OP_STORE = 0,
    VK_ATTACHMENT_STORE_OP_DONT_CARE = 1,
  // Provided by VK_VERSION_1_3
    VK_ATTACHMENT_STORE_OP_NONE = 1000301000,
  // Provided by VK_KHR_dynamic_rendering, VK_KHR_load_store_op_none
    VK_ATTACHMENT_STORE_OP_NONE_KHR = VK_ATTACHMENT_STORE_OP_NONE,
  // Provided by VK_QCOM_render_pass_store_ops
    VK_ATTACHMENT_STORE_OP_NONE_QCOM = VK_ATTACHMENT_STORE_OP_NONE,
  // Provided by VK_EXT_load_store_op_none
    VK_ATTACHMENT_STORE_OP_NONE_EXT = VK_ATTACHMENT_STORE_OP_NONE,
	} VkAttachmentStoreOp;
	*/
	vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE; //Render pass madhun baher gelyavar kay karu attachment image data sobat
	
	vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // For both depth and stencil, dont go on name
	vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // For both depth and stencil, dont go on name
	
	/*
	https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageLayout.html
	he sarv attachment madhla data cha arrangement cha aahe
	Unpacking athva RTR cha , karan color attachment mhnaje mostly texture
	*/
	vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; //Renderpass cha aat aalyavar , attachment cha data arrangemnent cha kay karu
	vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //Renderpass cha baher gelyavar , attachment cha data arrangemnent cha kay karu
	/*
	jya praname soure image aage , taasach layout thevun present kar.
	Madhe kahi changes zale, source praname thev
	*/
	
	/*
	/////////////////////////////////
	//For Color attachment
	2. Declare and initialize VkAttachmentReference struct (https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentReference.html) , which will have information about the attachment we described above.
	(jevha depth baghu , tevha proper ek extra element add hoil array madhe)
	*/
	VkAttachmentReference vkAttachmentReference_color;
	memset((void*)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));
	vkAttachmentReference_color.attachment = 0; //It is index. 0th is color attchment , 1st will be depth attachment
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageLayout.html
	//he image ksa vapraycha aahe , sang mala
	vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; //layout kasa thevaycha aahe , vapraycha aahe ? i.e yacha layout asa thev ki mi he attachment , color attachment mhanun vapru shakel
	
	/*
	/////////////////////////////////
	//For Depth attachmnent
	Declare and initialize VkAttachmentReference struct (https://registry.khronos.org/vulkan/specs/latest/man/html/VkAttachmentReference.html) , which will have information about the attachment we described above.
	(jevha depth baghu , tevha proper ek extra element add hoil array madhe)
	*/
	VkAttachmentReference vkAttachmentReference_depth;
	memset((void*)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));
	vkAttachmentReference_depth.attachment = 1; //It is index. 0th is color attchment , 1st will be depth attachment
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageLayout.html
	//he image ksa vapraycha aahe , sang mala
	vkAttachmentReference_depth.layout =  VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //layout kasa thevaycha aahe , vapraycha aahe ? i.e yacha layout asa thev ki mi he attachment , color attachment mhanun vapru shakel
	
	/*
	/////////////////////////////////
	3. Declare and initialize VkSubpassDescription struct (https://registry.khronos.org/vulkan/specs/latest/man/html/VkSubpassDescription.html) and keep reference about above VkAttachmentReference structe in it.
	*/
	VkSubpassDescription vkSubpassDescription; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkSubpassDescription.html
	memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
	
	vkSubpassDescription.flags = 0;
	vkSubpassDescription.pipelineBindPoint =  VK_PIPELINE_BIND_POINT_GRAPHICS; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineBindPoint.html
	vkSubpassDescription.inputAttachmentCount = 0;
	vkSubpassDescription.pInputAttachments = NULL;
	vkSubpassDescription.colorAttachmentCount = 1; //This count should be count of VkAttachmentReference used for color
	vkSubpassDescription.pColorAttachments = (const VkAttachmentReference*)&vkAttachmentReference_color;
	vkSubpassDescription.pResolveAttachments = NULL;
	vkSubpassDescription.pDepthStencilAttachment = (const VkAttachmentReference*)&vkAttachmentReference_depth;
	vkSubpassDescription.preserveAttachmentCount = 0;
	vkSubpassDescription.pPreserveAttachments = NULL;
	
	/*
	/////////////////////////////////
	4. Declare and initialize VkRenderPassCreatefo struct (https://registry.khronos.org/vulkan/specs/latest/man/html/VkRenderPassCreateInfo.html)  and referabove VkAttachmentDescription struct and VkSubpassDescription struct into it.
    Remember here also we need attachment information in form of Image Views, which will be used by framebuffer later.
    We also need to specify interdependancy between subpasses if needed.
	*/
	// https://registry.khronos.org/vulkan/specs/latest/man/html/VkRenderPassCreateInfo.html
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
	
	/*
	/////////////////////////////////
	5. Now call vkCreateRenderPass() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateRenderPass.html) to create actual RenderPass.
	*/
	vkResult = vkCreateRenderPass(gState_scene0.vkDevice, &vkRenderPassCreateInfo, NULL, &gState_scene0.vkRenderPass);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateRenderPass(): vkCreateRenderPass() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateRenderPass(): vkCreateRenderPass() succedded\n");
	}
	
	return vkResult;
}

VkResult CreatePipeline(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkVertexInputBindingDescription.html
	// Provided by VK_VERSION_1_0
	typedef struct VkVertexInputBindingDescription {
		uint32_t             binding;
		uint32_t             stride;
		VkVertexInputRate    inputRate; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkVertexInputRate.html
	} VkVertexInputBindingDescription;
	
	// Provided by VK_VERSION_1_0
	typedef enum VkVertexInputRate {
		VK_VERTEX_INPUT_RATE_VERTEX = 0,
		VK_VERTEX_INPUT_RATE_INSTANCE = 1,
	} VkVertexInputRate;
	*/
	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[3]; //Position,Normal and Texcoord
	memset((void*)vkVertexInputBindingDescription_array, 0,  sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

	//For Position
	vkVertexInputBindingDescription_array[0].binding = 0; //Equivalent to GL_ARRAY_BUFFER
	vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //vertices maan, indices nako
	
	//For Normal
	vkVertexInputBindingDescription_array[1].binding = 1; //Equivalent to GL_ARRAY_BUFFER
	vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 3;
	vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //vertices maan, indices nako
	
	//For Texcoords
	vkVertexInputBindingDescription_array[2].binding = 2; //Equivalent to GL_ARRAY_BUFFER
	vkVertexInputBindingDescription_array[2].stride = sizeof(float) * 2;
	vkVertexInputBindingDescription_array[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; //vertices maan, indices nako
	
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkVertexInputAttributeDescription.html
	// Provided by VK_VERSION_1_0
	typedef struct VkVertexInputAttributeDescription {
		uint32_t    location;
		uint32_t    binding;
		VkFormat    format; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkFormat.html
		uint32_t    offset;
	} VkVertexInputAttributeDescription;
	*/
	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[3]; //Position,Normal and Texcoord
	memset((void*)vkVertexInputAttributeDescription_array, 0,  sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

	//For Position
	vkVertexInputAttributeDescription_array[0].location = 0;
	vkVertexInputAttributeDescription_array[0].binding = 0;
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vkVertexInputAttributeDescription_array[0].offset = 0;
	
	//For Normal
	vkVertexInputAttributeDescription_array[1].location = 1;
	vkVertexInputAttributeDescription_array[1].binding = 1;
	vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vkVertexInputAttributeDescription_array[1].offset = 0;
	
	//For Texcoord
	vkVertexInputAttributeDescription_array[2].location = 2;
	vkVertexInputAttributeDescription_array[2].binding = 2;
	vkVertexInputAttributeDescription_array[2].format = VK_FORMAT_R32G32_SFLOAT;
	vkVertexInputAttributeDescription_array[2].offset = 0;
	
	/*
	Vertex Input State PSO
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineVertexInputStateCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineVertexInputStateCreateInfo {
		VkStructureType                             sType;
		const void*                                 pNext;
		VkPipelineVertexInputStateCreateFlags       flags;
		uint32_t                                    vertexBindingDescriptionCount;
		const VkVertexInputBindingDescription*      pVertexBindingDescriptions;
		uint32_t                                    vertexAttributeDescriptionCount;
		const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
	} VkPipelineVertexInputStateCreateInfo;
	*/
	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
	memset((void*)&vkPipelineVertexInputStateCreateInfo, 0,  sizeof(VkPipelineVertexInputStateCreateInfo));
	vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkPipelineVertexInputStateCreateInfo.pNext = NULL;
	vkPipelineVertexInputStateCreateInfo.flags = 0;
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;
	
	/*
	Input Assembly State
	https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineInputAssemblyStateCreateInfo.html/
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineInputAssemblyStateCreateInfo {
		VkStructureType                            sType;
		const void*                                pNext;
		VkPipelineInputAssemblyStateCreateFlags    flags; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineInputAssemblyStateCreateFlags.html
		VkPrimitiveTopology                        topology;
		VkBool32                                   primitiveRestartEnable;
	} VkPipelineInputAssemblyStateCreateInfo;
	
	https://registry.khronos.org/vulkan/specs/latest/man/html/VkPrimitiveTopology.html
	// Provided by VK_VERSION_1_0
	typedef enum VkPrimitiveTopology {
		VK_PRIMITIVE_TOPOLOGY_POINT_LIST = 0,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST = 1,
		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP = 2,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST = 3,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP = 4,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN = 5,
		
		//For Geometry Shader
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY = 6,
		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY = 7,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY = 8,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY = 9,
		
		//For Tescellation Shader
		VK_PRIMITIVE_TOPOLOGY_PATCH_LIST = 10,
	} VkPrimitiveTopology;
	
	*/
	VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
	memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0,  sizeof(VkPipelineInputAssemblyStateCreateInfo));
	vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
	vkPipelineInputAssemblyStateCreateInfo.flags = 0;
	vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; //Not needed here. Only for geometry shader and for indexed drawing for strip and fan
	
	/*
	//Rasterizer State
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineRasterizationStateCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineRasterizationStateCreateInfo {
		VkStructureType                            sType;
		const void*                                pNext;
		VkPipelineRasterizationStateCreateFlags    flags; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineRasterizationStateCreateFlags.html
		VkBool32                                   depthClampEnable;
		VkBool32                                   rasterizerDiscardEnable;
		VkPolygonMode                              polygonMode;
		VkCullModeFlags                            cullMode; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkCullModeFlags.html
		VkFrontFace                                frontFace;
		VkBool32                                   depthBiasEnable;
		float                                      depthBiasConstantFactor;
		float                                      depthBiasClamp;
		float                                      depthBiasSlopeFactor;
		float                                      lineWidth;
	} VkPipelineRasterizationStateCreateInfo;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPolygonMode.html
	// Provided by VK_VERSION_1_0
	typedef enum VkPolygonMode {
		VK_POLYGON_MODE_FILL = 0,
		VK_POLYGON_MODE_LINE = 1,
		VK_POLYGON_MODE_POINT = 2,
	  // Provided by VK_NV_fill_rectangle
		VK_POLYGON_MODE_FILL_RECTANGLE_NV = 1000153000,
	} VkPolygonMode;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkFrontFace.html
	// Provided by VK_VERSION_1_0
	typedef enum VkFrontFace {
		VK_FRONT_FACE_COUNTER_CLOCKWISE = 0,
		VK_FRONT_FACE_CLOCKWISE = 1,
	} VkFrontFace;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkCullModeFlags.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkCullModeFlagBits.html
	// Provided by VK_VERSION_1_0
	typedef enum VkCullModeFlagBits {
		VK_CULL_MODE_NONE = 0,
		VK_CULL_MODE_FRONT_BIT = 0x00000001,
		VK_CULL_MODE_BACK_BIT = 0x00000002,
		VK_CULL_MODE_FRONT_AND_BACK = 0x00000003,
	} VkCullModeFlagBits;
	*/
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
	memset((void*)&vkPipelineRasterizationStateCreateInfo, 0,  sizeof(VkPipelineRasterizationStateCreateInfo));
	vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vkPipelineRasterizationStateCreateInfo.pNext = NULL;
	vkPipelineRasterizationStateCreateInfo.flags = 0;
	//vkPipelineRasterizationStateCreateInfo.depthClampEnable =;
	//vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable =;
	vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT was here originally
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //Triangle winding order 
	//vkPipelineRasterizationStateCreateInfo.depthBiasEnable =;
	//vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor =;
	//vkPipelineRasterizationStateCreateInfo.depthBiasClamp =;
	//vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor =;
	vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; //This is implementation dependant. So giving it is compulsary. Atleast give it 1.0
	
	/*
	//Color Blend state
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineColorBlendAttachmentState.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineColorBlendAttachmentState {
		VkBool32                 blendEnable;
		VkBlendFactor            srcColorBlendFactor; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBlendFactor.html
		VkBlendFactor            dstColorBlendFactor; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBlendFactor.html
		VkBlendOp                colorBlendOp; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkBlendOp.html
		VkBlendFactor            srcAlphaBlendFactor;
		VkBlendFactor            dstAlphaBlendFactor;
		VkBlendOp                alphaBlendOp;
		VkColorComponentFlags    colorWriteMask; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkColorComponentFlags.html
	} VkPipelineColorBlendAttachmentState;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkColorComponentFlags.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkColorComponentFlagBits.html
	// Provided by VK_VERSION_1_0
	typedef enum VkColorComponentFlagBits {
		VK_COLOR_COMPONENT_R_BIT = 0x00000001,
		VK_COLOR_COMPONENT_G_BIT = 0x00000002,
		VK_COLOR_COMPONENT_B_BIT = 0x00000004,
		VK_COLOR_COMPONENT_A_BIT = 0x00000008,
	} VkColorComponentFlagBits;
	*/
	VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
	memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
	vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
	/*
	vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor =;
	vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor =;
	vkPipelineColorBlendAttachmentState_array[0].colorBlendOp =;
	vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor =;
	vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor =;
	vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp=;
	*/
	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0xF;
	
	/*
	//Color Blend state
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineColorBlendStateCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineColorBlendStateCreateInfo {
		VkStructureType                               sType;
		const void*                                   pNext;
		VkPipelineColorBlendStateCreateFlags          flags; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineColorBlendStateCreateFlags.html
		VkBool32                                      logicOpEnable;
		VkLogicOp                                     logicOp;
		uint32_t                                      attachmentCount;
		const VkPipelineColorBlendAttachmentState*    pAttachments;
		float                                         blendConstants[4];
	} VkPipelineColorBlendStateCreateInfo;
	*/
	VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
	memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
	vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkPipelineColorBlendStateCreateInfo.pNext = NULL;
	vkPipelineColorBlendStateCreateInfo.flags = 0;
	//vkPipelineColorBlendStateCreateInfo.logicOpEnable =;
	//vkPipelineColorBlendStateCreateInfo.logicOp = ;
	vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
	//vkPipelineColorBlendStateCreateInfo.blendConstants =;
	
	/*Viewport Scissor State
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineViewportStateCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineViewportStateCreateInfo {
		VkStructureType                       sType;
		const void*                           pNext;
		VkPipelineViewportStateCreateFlags    flags;
		uint32_t                              viewportCount;
		const VkViewport*                     pViewports;
		uint32_t                              scissorCount;
		const VkRect2D*                       pScissors;
	} VkPipelineViewportStateCreateInfo;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkViewport.html
	// Provided by VK_VERSION_1_0
	typedef struct VkViewport {
		float    x;
		float    y;
		float    width;
		float    height;
		float    minDepth;
		float    maxDepth;
	} VkViewport;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkRect2D.html
	// Provided by VK_VERSION_1_0
	typedef struct VkRect2D {
		VkOffset2D    offset;
		VkExtent2D    extent;
	} VkRect2D;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkOffset2D.html
	// Provided by VK_VERSION_1_0
	typedef struct VkOffset2D {
		int32_t    x;
		int32_t    y;
	} VkOffset2D;

	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtent2D.html
	// Provided by VK_VERSION_1_0
	typedef struct VkExtent2D {
		uint32_t    width;
		uint32_t    height;
	} VkExtent2D;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateGraphicsPipelines.html
	// Provided by VK_VERSION_1_0
	VkResult vkCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines);
	
	We can create multiple pipelines.
	The viewport and scissor count members of this structure must be same.
	*/
	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
	memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
	vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkPipelineViewportStateCreateInfo.pNext = NULL;
	vkPipelineViewportStateCreateInfo.flags = 0;
	
	////////////////
	vkPipelineViewportStateCreateInfo.viewportCount = 1; //We can specify multiple viewport as array;
	memset((void*)&gState_scene0.vkViewPort, 0 , sizeof(VkViewport));
	gState_scene0.vkViewPort.x = 0;
	gState_scene0.vkViewPort.y = 0;
	gState_scene0.vkViewPort.width = (float)gState_scene0.vkExtent2D_SwapChain.width;
	gState_scene0.vkViewPort.height = (float)gState_scene0.vkExtent2D_SwapChain.height;
	
	//done link following parameters with glClearDepth()
	//viewport cha depth max kiti asu shakto deto ithe
	//depth buffer ani viewport cha depth cha sambandh nahi
	gState_scene0.vkViewPort.minDepth = 0.0f;
	gState_scene0.vkViewPort.maxDepth = 1.0f;
	
	vkPipelineViewportStateCreateInfo.pViewports = &gState_scene0.vkViewPort;
	////////////////
	
	////////////////
	vkPipelineViewportStateCreateInfo.scissorCount = 1;
	memset((void*)&gState_scene0.vkRect2D_scissor, 0 , sizeof(VkRect2D));
	gState_scene0.vkRect2D_scissor.offset.x = 0;
	gState_scene0.vkRect2D_scissor.offset.y = 0;
	gState_scene0.vkRect2D_scissor.extent.width = gState_scene0.vkExtent2D_SwapChain.width;
	gState_scene0.vkRect2D_scissor.extent.height = gState_scene0.vkExtent2D_SwapChain.height;
	
	vkPipelineViewportStateCreateInfo.pScissors = &gState_scene0.vkRect2D_scissor;
	////////////////
	
	/* Depth Stencil State
	As we dont have depth yet, we can omit this step.
	*/
	
	/* Dynamic State
	Those states of PSO, which can be changed dynamically without recreating pipeline.
	ViewPort, Scissor, Depth Bias, Blend constants, Stencil Mask, LineWidth etc are some states which can be changed dynamically.
	We dont have any dynamic state in this code.
	*/
	
	/*
	MultiSampling State
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineMultisampleStateCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineMultisampleStateCreateInfo {
		VkStructureType                          sType;
		const void*                              pNext;
		VkPipelineMultisampleStateCreateFlags    flags;
		VkSampleCountFlagBits                    rasterizationSamples;
		VkBool32                                 sampleShadingEnable;
		float                                    minSampleShading;
		const VkSampleMask*                      pSampleMask;
		VkBool32                                 alphaToCoverageEnable;
		VkBool32                                 alphaToOneEnable;
	} VkPipelineMultisampleStateCreateInfo;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkSampleCountFlagBits.html
	// Provided by VK_VERSION_1_0
	typedef enum VkSampleCountFlagBits {
		VK_SAMPLE_COUNT_1_BIT = 0x00000001,
		VK_SAMPLE_COUNT_2_BIT = 0x00000002,
		VK_SAMPLE_COUNT_4_BIT = 0x00000004,
		VK_SAMPLE_COUNT_8_BIT = 0x00000008,
		VK_SAMPLE_COUNT_16_BIT = 0x00000010,
		VK_SAMPLE_COUNT_32_BIT = 0x00000020,
		VK_SAMPLE_COUNT_64_BIT = 0x00000040,
	} VkSampleCountFlagBits;
	*/
	VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
	memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
	vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vkPipelineMultisampleStateCreateInfo.pNext = NULL;
	vkPipelineMultisampleStateCreateInfo.flags = 0; //Reserved and kept for future use, so 0
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // Need to give or validation error will come
	/*
	vkPipelineMultisampleStateCreateInfo.sampleShadingEnable =;
	vkPipelineMultisampleStateCreateInfo.minSampleShading =;
	vkPipelineMultisampleStateCreateInfo.pSampleMask =;
	vkPipelineMultisampleStateCreateInfo.alphaToCoverageEnable =;
	vkPipelineMultisampleStateCreateInfo.alphaToOneEnable =;
	*/
	
	/*
	Shader Stage
	Ithe array karava lagto (2/5 count cha)
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineShaderStageCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineShaderStageCreateInfo {
		VkStructureType                     sType;
		const void*                         pNext;
		VkPipelineShaderStageCreateFlags    flags;
		VkShaderStageFlagBits               stage; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkShaderStageFlagBits.html
		VkShaderModule                      module; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkShaderModule.html
		const char*                         pName;
		const VkSpecializationInfo*         pSpecializationInfo;
	} VkPipelineShaderStageCreateInfo;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkSpecializationInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkSpecializationInfo {
		uint32_t                           mapEntryCount;
		const VkSpecializationMapEntry*    pMapEntries;
		size_t                             dataSize;
		const void*                        pData;
	} VkSpecializationInfo;
	*/
	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
	memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));
	//Vertex Shader
	vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[0].pNext = NULL; //validation error is not given (If any structure(shader stage in this case) having extensions is not given pNext as NULL, then validation error comes)
	vkPipelineShaderStageCreateInfo_array[0].flags = 0;
	vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	vkPipelineShaderStageCreateInfo_array[0].module = gState_scene0.vkShaderMoudule_vertex_shader;
	vkPipelineShaderStageCreateInfo_array[0].pName = "main"; //entry point cha address
	vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL; //If any constants, precompile in SPIRV inline fashion.
	
	//Fragment Shader
	vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[1].pNext = NULL; //validation error is not given (If any structure(shader stage in this case) having extensions is not given pNext as NULL, then validation error comes)
	vkPipelineShaderStageCreateInfo_array[1].flags = 0;
	vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	vkPipelineShaderStageCreateInfo_array[1].module = gState_scene0.vkShaderMoudule_fragment_shader;
	vkPipelineShaderStageCreateInfo_array[1].pName = "main"; //entry point cha address;
	vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL; //If any constants, precompile in SPIRV inline fashion.
	
	/*
	Tescellation State
	We dont have tescellation shaders. So we can omit this state.
	*/
	
	/*
	As pipelines are created from pipeline cache, we will now create pipeline cache object.
	Not in red book. But in spec.
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineCacheCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineCacheCreateInfo {
		VkStructureType               sType;
		const void*                   pNext;
		VkPipelineCacheCreateFlags    flags;
		size_t                        initialDataSize;
		const void*                   pInitialData;
	} VkPipelineCacheCreateInfo;
	*/
	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0;
	/*
	vkPipelineCacheCreateInfo.initialDataSize =;
	vkPipelineCacheCreateInfo.pInitialData =;
	*/
	
	/*
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreatePipelineCache.html
	// Provided by VK_VERSION_1_0
	VkResult vkCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkPipelineCache*                            pPipelineCache);
	*/
	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
	vkResult = vkCreatePipelineCache(gState_scene0.vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreatePipeline(): vkCreatePipelineCache() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreatePipeline(): vkCreatePipelineCache() succedded\n");
	}
	
	/*
	Create actual graphics pipeline
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkGraphicsPipelineCreateInfo.html
	// Provided by VK_VERSION_1_0
	typedef struct VkGraphicsPipelineCreateInfo {
		VkStructureType                                  sType;
		const void*                                      pNext;
		VkPipelineCreateFlags                            flags;
		uint32_t                                         stageCount;
		const VkPipelineShaderStageCreateInfo*           pStages;
		const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
		const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
		const VkPipelineTessellationStateCreateInfo*     pTessellationState;
		const VkPipelineViewportStateCreateInfo*         pViewportState;
		const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
		const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
		const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
		const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
		const VkPipelineDynamicStateCreateInfo*          pDynamicState;
		VkPipelineLayout                                 layout;
		VkRenderPass                                     renderPass;
		uint32_t                                         subpass;
		VkPipeline                                       basePipelineHandle;
		int32_t                                          basePipelineIndex;
	} VkGraphicsPipelineCreateInfo;
	*/
	VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
	memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
	vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = NULL;
	vkGraphicsPipelineCreateInfo.flags = 0;
	vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array); //8
	vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array; //9
	vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo; //1
	vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo; //2
	vkGraphicsPipelineCreateInfo.pTessellationState = NULL; //10
	vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo; //5
	vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo; //3
	vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo; //8
	//vkGraphicsPipelineCreateInfo.pDepthStencilState = NULL; //6
	/*
	// Provided by VK_VERSION_1_0
	typedef struct VkPipelineDepthStencilStateCreateInfo {
		VkStructureType                           sType;
		const void*                               pNext;
		VkPipelineDepthStencilStateCreateFlags    flags;
		VkBool32                                  depthTestEnable;
		VkBool32                                  depthWriteEnable;
		VkCompareOp                               depthCompareOp;
		VkBool32                                  depthBoundsTestEnable;
		VkBool32                                  stencilTestEnable;
		VkStencilOpState                          front;
		VkStencilOpState                          back;
		float                                     minDepthBounds;
		float                                     maxDepthBounds;
	} VkPipelineDepthStencilStateCreateInfo;
	*/
	VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
	memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
	vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
	vkPipelineDepthStencilStateCreateInfo.flags = 0;
	vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	vkPipelineDepthStencilStateCreateInfo.depthWriteEnable= VK_TRUE; 
	vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkCompareOp.html
	vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable= VK_FALSE;
	vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	//vkPipelineDepthStencilStateCreateInfo.minDepthBounds = ;
	//vkPipelineDepthStencilStateCreateInfo.maxDepthBounds= ;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOpState.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOp.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkCompareOp.html
	vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP; 
	vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
	vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS; // one of 8 tests 
	//vkPipelineDepthStencilStateCreateInfo.back.depthFailOp = ;
	//vkPipelineDepthStencilStateCreateInfo.back.compareMask = ;
	//vkPipelineDepthStencilStateCreateInfo.back.writeMask = ;
	//vkPipelineDepthStencilStateCreateInfo.back.reference = ;
	
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOpState.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkStencilOp.html
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkCompareOp.html
	vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back; 
	
	vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo; //6

	vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo; //4
	vkGraphicsPipelineCreateInfo.pDynamicState = NULL; //7
	vkGraphicsPipelineCreateInfo.layout = gState_scene0.vkPipelineLayout; //11
	vkGraphicsPipelineCreateInfo.renderPass = gState_scene0.vkRenderPass; //12
	vkGraphicsPipelineCreateInfo.subpass = 0; //13. 0 as no subpass as wehave only 1 renderpass and its default subpass(In Redbook)
	vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;
	
	/*
	Now create the pipeline
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateGraphicsPipelines.html
	// Provided by VK_VERSION_1_0
	VkResult vkCreateGraphicsPipelines(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    uint32_t                                    createInfoCount,
    const VkGraphicsPipelineCreateInfo*         pCreateInfos,
    const VkAllocationCallbacks*                pAllocator,
    VkPipeline*                                 pPipelines);
	*/
	vkResult = vkCreateGraphicsPipelines(gState_scene0.vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &gState_scene0.vkPipeline);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "vkCreateGraphicsPipelines(): vkCreatePipelineCache() function failed with error code %d\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "vkCreateGraphicsPipelines(): vkCreatePipelineCache() succedded\n");
	}
	
	/*
	We are done with pipeline cache . So destroy it
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkDestroyPipelineCache.html
	// Provided by VK_VERSION_1_0
	void vkDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    const VkAllocationCallbacks*                pAllocator);
	*/
	if(vkPipelineCache != VK_NULL_HANDLE)
	{
		vkDestroyPipelineCache(gState_scene0.vkDevice, vkPipelineCache, NULL);
		vkPipelineCache = VK_NULL_HANDLE;
		fprintf(gState_scene0.gFILE, "vkCreateGraphicsPipelines(): vkPipelineCache is freed\n");
	}
	
	return vkResult;
}

VkResult CreateFramebuffers(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	gState_scene0.vkFramebuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * gState_scene0.swapchainImageCount);
		//for sake of brevity, no error checking
	
	for(uint32_t i = 0 ; i < gState_scene0.swapchainImageCount; i++)
	{
		/*
		1. Declare an array of VkImageView (https://registry.khronos.org/vulkan/specs/latest/man/html/VkImageView.html) equal to number of attachments i.e in our example array of member.
		*/
		VkImageView vkImageView_attachment_array[2];
		memset((void*)vkImageView_attachment_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachment_array));
		
		/*
		2. Declare and initialize VkFramebufferCreateInfo structure (https://registry.khronos.org/vulkan/specs/latest/man/html/VkFramebufferCreateInfo.html).
		Allocate the framebuffer array by malloc eqal size to gState_scene0.swapchainImageCount.
		 Start loop for  gState_scene0.swapchainImageCount and call vkCreateFramebuffer() (https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateFramebuffer.html) to create framebuffers.
		*/
		VkFramebufferCreateInfo vkFramebufferCreateInfo;
		memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
		
		vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		vkFramebufferCreateInfo.pNext = NULL;
		vkFramebufferCreateInfo.flags = 0;
		vkFramebufferCreateInfo.renderPass = gState_scene0.vkRenderPass;
		vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
		vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
		vkFramebufferCreateInfo.width = gState_scene0.vkExtent2D_SwapChain.width;
		vkFramebufferCreateInfo.height = gState_scene0.vkExtent2D_SwapChain.height;
		vkFramebufferCreateInfo.layers = 1;
		
		vkImageView_attachment_array[0] = gState_scene0.swapChainImageView_array[i];
		vkImageView_attachment_array[1] = gState_scene0.vkImageView_depth;
		
		vkResult = vkCreateFramebuffer(gState_scene0.vkDevice, &vkFramebufferCreateInfo, NULL, &gState_scene0.vkFramebuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "CreateFramebuffers(): vkCreateFramebuffer() function failed with error code %d\n", vkResult);
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "CreateFramebuffers(): vkCreateFramebuffer() succedded\n");
		}	
	}
	
	return vkResult;
}

VkResult CreateSemaphores(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	
	/*
	18_2. In CreateSemaphore() UDF(User defined function) , declare, memset and initialize VkSemaphoreCreateInfo  struct (https://registry.khronos.org/vulkan/specs/latest/man/html/VkSemaphoreCreateInfo.html)
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkSemaphoreCreateInfo.html
	VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
	memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
	vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSemaphoreCreateInfo.pNext = NULL; //If no type is specified , the type of semaphore created is binary semaphore
	vkSemaphoreCreateInfo.flags = 0; //must be 0 as reserved
	
	/*
	18_3. Now call vkCreateSemaphore() {https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateSemaphore.html} 2 times to create our 2 semaphore objects.
    Remember both will use same  VkSemaphoreCreateInfo struct as defined in 2nd step.
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateSemaphore.html
	vkResult = vkCreateSemaphore(gState_scene0.vkDevice, &vkSemaphoreCreateInfo, NULL, &gState_scene0.vkSemaphore_BackBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateSemaphores(): vkCreateSemaphore() function failed with error code %d for gState_scene0.vkSemaphore_BackBuffer\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateSemaphores(): vkCreateSemaphore() succedded for gState_scene0.vkSemaphore_BackBuffer\n");
	}

	//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateSemaphore.html
	vkResult = vkCreateSemaphore(gState_scene0.vkDevice, &vkSemaphoreCreateInfo, NULL, &gState_scene0.vkSemaphore_RenderComplete);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gState_scene0.gFILE, "CreateSemaphores(): vkCreateSemaphore() function failed with error code %d for gState_scene0.vkSemaphore_RenderComplete\n", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gState_scene0.gFILE, "CreateSemaphores(): vkCreateSemaphore() succedded for gState_scene0.vkSemaphore_RenderComplete\n");
	}	
	
	return vkResult;
}

VkResult CreateFences(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	
	/*
	18_4. In CreateFences() UDF(User defined function) declare, memset and initialize VkFenceCreateInfo struct (https://registry.khronos.org/vulkan/specs/latest/man/html/VkFenceCreateInfo.html).
	*/
	//https://registry.khronos.org/vulkan/specs/latest/man/html/VkFenceCreateInfo.html
	VkFenceCreateInfo  vkFenceCreateInfo;
	memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));
	vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceCreateInfo.pNext = NULL;
	vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //https://registry.khronos.org/vulkan/specs/latest/man/html/VkFenceCreateFlagBits.html
	
	/*
	18_5. In this function, CreateFences() allocate our global fence array to size of swapchain image count using malloc.
	*/
	gState_scene0.vkFence_array = (VkFence*)malloc(sizeof(VkFence) * gState_scene0.swapchainImageCount);
	//error checking skipped due to brevity
	
	/*
	18_6. Now in a loop, call vkCreateFence() {https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateFence.html} to initialize our global fences array.
	*/
	for(uint32_t i =0; i < gState_scene0.swapchainImageCount; i++)
	{
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCreateFence.html
		vkResult = vkCreateFence(gState_scene0.vkDevice, &vkFenceCreateInfo, NULL, &gState_scene0.vkFence_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "CreateFences(): vkCreateFence() function failed with error code %d at %d iteration\n", vkResult, i);
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "CreateFences(): vkCreateFence() succedded at %d iteration\n", i);
		}	
	}
	
	return vkResult;
}

VkResult buildCommandBuffers(void)
{
	//Variable declarations	
	VkResult vkResult = VK_SUCCESS;
	
	/*
	Code
	*/
	
	/*
	1. Start a loop with gState_scene0.swapchainImageCount as counter.
	   loop per swapchainImage
	*/
	for(uint32_t i =0; i< gState_scene0.swapchainImageCount; i++)
	{
		/*
		2. Inside loop, call vkResetCommandBuffer to reset contents of command buffers.
		0 says dont release resource created by command pool for these command buffers, because we may reuse
		*/
		vkResult = vkResetCommandBuffer(gState_scene0.vkCommandBuffer_array[i], 0);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "buildCommandBuffers(): vkResetCommandBuffer() function failed with error code %d at %d iteration\n", vkResult, i);
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "buildCommandBuffers(): vkResetCommandBuffer() succedded at %d iteration\n", i);
		}	
		
		/*
		3. Then declare, memset and initialize VkCommandBufferBeginInfo struct.
		*/
		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
		memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = NULL;
		vkCommandBufferBeginInfo.flags = 0; 
		
		/*
		pInheritanceInfo is a pointer to a VkCommandBufferInheritanceInfo structure, used if commandBuffer is a secondary command buffer. If this is a primary command buffer, then this value is ignored.
		We are not going to use this command buffer simultaneouly between multiple threads.
		*/
		vkCommandBufferBeginInfo.pInheritanceInfo = NULL;
		
		/*
		4. Call vkBeginCommandBuffer() to record different Vulkan drawing related commands.
		Do Error Checking.
		*/
		vkResult = vkBeginCommandBuffer(gState_scene0.vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "buildCommandBuffers(): vkBeginCommandBuffer() function failed with error code %d at %d iteration\n", vkResult, i);
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "buildCommandBuffers(): vkBeginCommandBuffer() succedded at %d iteration\n", i);
		}
		
		/*
		5. Declare, memset and initialize struct array of VkClearValue type
		*/
		VkClearValue vkClearValue_array[2];
		memset((void*)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));
		vkClearValue_array[0].color = gState_scene0.vkClearColorValue;
		vkClearValue_array[1].depthStencil = gState_scene0.vkClearDepthStencilValue;
		
		/*
		6. Then declare , memset and initialize VkRenderPassBeginInfo struct.
		*/
		VkRenderPassBeginInfo vkRenderPassBeginInfo;
		memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));
		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkRenderPassBeginInfo.pNext = NULL;
		vkRenderPassBeginInfo.renderPass = gState_scene0.vkRenderPass;
		
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkRect2D.html
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkOffset2D.html
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkExtent2D.html
		//THis is like D3DViewport/glViewPort
		vkRenderPassBeginInfo.renderArea.offset.x = 0;
		vkRenderPassBeginInfo.renderArea.offset.y = 0;
		vkRenderPassBeginInfo.renderArea.extent.width = gState_scene0.vkExtent2D_SwapChain.width;	
		vkRenderPassBeginInfo.renderArea.extent.height = gState_scene0.vkExtent2D_SwapChain.height;	
		
		vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
		vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
		
		vkRenderPassBeginInfo.framebuffer = gState_scene0.vkFramebuffer_array[i];
		
		/*
		7. Begin RenderPass by vkCmdBeginRenderPass() API.
		Remember, the code writtrn inside "BeginRenderPass" and "EndRenderPass" itself is code for subpass , if no subpass is explicitly created.
		In other words even if no subpass is declared explicitly , there is one subpass for renderpass.
		
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkSubpassContents.html
		//VK_SUBPASS_CONTENTS_INLINE specifies that the contents of the subpass will be recorded inline in the primary command buffer, and secondary command buffers must not be executed within the subpass.
		*/
		vkCmdBeginRenderPass(gState_scene0.vkCommandBuffer_array[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE); 
		
		/*
		Bind with the pipeline
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdBindPipeline.html
		// Provided by VK_VERSION_1_0
		void vkCmdBindPipeline(
			VkCommandBuffer                             commandBuffer,
			VkPipelineBindPoint                         pipelineBindPoint,
			VkPipeline                                  pipeline);
			
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkPipelineBindPoint.html
		// Provided by VK_VERSION_1_0
		typedef enum VkPipelineBindPoint {
			VK_PIPELINE_BIND_POINT_GRAPHICS = 0,
			VK_PIPELINE_BIND_POINT_COMPUTE = 1,
		#ifdef VK_ENABLE_BETA_EXTENSIONS
		  // Provided by VK_AMDX_shader_enqueue
			VK_PIPELINE_BIND_POINT_EXECUTION_GRAPH_AMDX = 1000134000,
		#endif
		  // Provided by VK_KHR_ray_tracing_pipeline
			VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR = 1000165000,
		  // Provided by VK_HUAWEI_subpass_shading
			VK_PIPELINE_BIND_POINT_SUBPASS_SHADING_HUAWEI = 1000369003,
		  // Provided by VK_NV_ray_tracing
			VK_PIPELINE_BIND_POINT_RAY_TRACING_NV = VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		} VkPipelineBindPoint;
		*/
		vkCmdBindPipeline(gState_scene0.vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gState_scene0.vkPipeline);
		
		
		/*
		Bind our descriptor set with pipeline
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdBindDescriptorSets.html
		// Provided by VK_VERSION_1_0
		void vkCmdBindDescriptorSets(
		VkCommandBuffer                             commandBuffer,
		VkPipelineBindPoint                         pipelineBindPoint,
		VkPipelineLayout                            layout,
		uint32_t                                    firstSet,
		uint32_t                                    descriptorSetCount,
		const VkDescriptorSet*                      pDescriptorSets,
		uint32_t                                    dynamicOffsetCount, // Used for dynamic shader stages
		const uint32_t*                             pDynamicOffsets); // Used for dynamic shader stages
		*/
		vkCmdBindDescriptorSets(gState_scene0.vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, gState_scene0.vkPipelineLayout, 0, 1, &gState_scene0.vkDescriptorSet, 0, NULL);
		
		/*
		Bind with vertex buffer
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkDeviceSize.html
		
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdBindVertexBuffers.html
		// Provided by VK_VERSION_1_0
		void vkCmdBindVertexBuffers(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    firstBinding,
			uint32_t                                    bindingCount,
			const VkBuffer*                             pBuffers,
			const VkDeviceSize*                         pOffsets);
		*/
		VkBuffer vertexBuffers[3] = {
			gState_scene0.vertexdata_position.vkBuffer,
			gState_scene0.vertexdata_normals.vkBuffer,
			gState_scene0.vertexdata_texcoord.vkBuffer
		};
		VkDeviceSize vkDeviceSize_offset_array_position[1];
		memset((void*)vkDeviceSize_offset_array_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_array_position));
		vkCmdBindVertexBuffers(gState_scene0.vkCommandBuffer_array[i], 0, 1, &(vertexBuffers[0]), vkDeviceSize_offset_array_position); //Here recording
		
		VkDeviceSize vkDeviceSize_offset_array_normal[1];
		memset((void*)vkDeviceSize_offset_array_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_array_normal));
		vkCmdBindVertexBuffers(gState_scene0.vkCommandBuffer_array[i], 1, 1, &(vertexBuffers[1]), vkDeviceSize_offset_array_normal); //Here recording
		
		VkDeviceSize vkDeviceSize_offset_array_texcoords[1];
		memset((void*)vkDeviceSize_offset_array_texcoords, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_array_texcoords));
		vkCmdBindVertexBuffers(gState_scene0.vkCommandBuffer_array[i], 2, 1, &(vertexBuffers[2]), vkDeviceSize_offset_array_texcoords); //Here recording
		
		//Binding Index Buffer
		/*
		Here we should call Vulkan drawing functions.
		*/
		/*
		Bind our index buffer
		//https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
		void glDrawElements(	GLenum mode,
								GLsizei count,
								GLenum type,
								const void * indices);

		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdBindIndexBuffer.html
		//https://registry.khronos.org/vulkan/specs/latest/man/html/VkIndexType.html
		// Provided by VK_VERSION_1_0
		void vkCmdBindIndexBuffer(
			VkCommandBuffer                             commandBuffer,
			VkBuffer                                    buffer, //corresponds to fourth parameter of glDrawElements
			VkDeviceSize                                offset,
			VkIndexType                                 indexType); //Corresponds to third parameter of glDrawElements
			
		//4th parameter VK_INDEX_TYPE_UINT32 is matching with our triangle indices array of uint32_t 
		*/
		vkCmdBindIndexBuffer(gState_scene0.vkCommandBuffer_array[i], gState_scene0.vertexdata_index.vkBuffer, 0,  VK_INDEX_TYPE_UINT16); //Use 16-bit index type to match gState_scene0.sphere_elements[] definition
		
		/*
		//https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
		void glDrawElements(	GLenum mode,
								GLsizei count,
								GLenum type,
								const void * indices);
		
		//https://registry.khronos.org/vulkan/specs/latest/man/html/vkCmdDrawIndexed.html
		// Provided by VK_VERSION_1_0
		void vkCmdDrawIndexed(
			VkCommandBuffer                             commandBuffer,
			uint32_t                                    indexCount, //corresponds to 2nd parameter of glDrawElements
			uint32_t                                    instanceCount,
			uint32_t                                    firstIndex,
			int32_t                                     vertexOffset, //If we want to add any offset from starting index in Vertex Buffer
			uint32_t                                    firstInstance); //nth instance
		*/
		vkCmdDrawIndexed(gState_scene0.vkCommandBuffer_array[i], gState_scene0.numElements, 1, 0, 0, 0); //last parameter was 1 earlier , then changed to 0, output comes in both
		/*
		8. End the renderpass by calling vkCmdEndRenderpass.
		*/
		vkCmdEndRenderPass(gState_scene0.vkCommandBuffer_array[i]);
		
		/*
		9. End the recording of commandbuffer by calling vkEndCommandBuffer() API.
		*/
		vkResult = vkEndCommandBuffer(gState_scene0.vkCommandBuffer_array[i]);
		if (vkResult != VK_SUCCESS)
		{
			fprintf(gState_scene0.gFILE, "buildCommandBuffers(): vkEndCommandBuffer() function failed with error code %d at %d iteration\n", vkResult, i);
			return vkResult;
		}
		else
		{
			fprintf(gState_scene0.gFILE, "buildCommandBuffers(): vkEndCommandBuffer() succedded at %d iteration\n", i);
		}
		
		/*
		10. Close the loop.
		*/
	}
	
	return vkResult;
}

/*
VKAPI_ATTR VkBOOL32 VKAPI_CALL debugReportCallback(
	VkDebugReportFlagsEXT vkDebugReportFlagsEXT, //which flags gave this callback
	VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT, //jyana ha callback trigger kela , tya object cha type
	uint64_t object, //Proper object
	size_t location,  //warning/error kutha aali tyacha location
	int32_t messageCode, // message cha id -> message code in hex 
	const char* pLayerPrefix, // kontya layer na ha dila (Purvi 5 layer hote, aata ek kila. So ekach yeil atta)
	const char* pMessage, //actual error message
	void* pUserData) //jar tumhi callback function la kahi parameter pass kela asel tar
{
	//Code
	fprintf(gState_scene0.gFILE, "Anjaneya_VALIDATION:debugReportCallback():%s(%d) = %s\n", pLayerPrefix, messageCode, pMessage);  
    return (VK_FALSE);
}
*/

VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT, uint64_t object, size_t location,  int32_t messageCode,const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	//Code
	fprintf(gState_scene0.gFILE, "Anjaneya_VALIDATION:debugReportCallback():%s(%d) = %s\n", pLayerPrefix, messageCode, pMessage);  
    return (VK_FALSE);
}

AppFunctionTable_Scene0 gAppFunctions_Scene0 =
{
        WndProc,
        initialize,
        uninitialize,
        display,
        update,
        resize,
        ToggleFullscreen
};

VulkanFunctionTable_Scene0 gVulkanFunctions_Scene0 =
{
        CreateVulkanInstance,
        FillInstanceExtensionNames,
        FillValidationLayerNames,
        CreateValidationCallbackFunction,
        GetSupportedSurface,
        GetPhysicalDevice,
        PrintVulkanInfo,
        FillDeviceExtensionNames,
        CreateVulKanDevice,
        GetDeviceQueque,
        getPhysicalDeviceSurfaceFormatAndColorSpace,
        getPhysicalDevicePresentMode,
        CreateSwapChain,
        CreateImagesAndImageViews,
        GetSupportedDepthFormat,
        CreateCommandPool,
        CreateCommandBuffers,
        CreateVertexBuffer,
        CreateIndexBuffer,
        CreateUniformBuffer,
        CreateShaders,
        CreateDescriptorSetLayout,
        CreatePipelineLayout,
        CreateDescriptorPool,
        CreateDescriptorSet,
        CreateRenderPass,
        CreatePipeline,
        CreateFramebuffers,
        CreateSemaphores,
        CreateFences,
        buildCommandBuffers,
        UpdateUniformBuffer
};
