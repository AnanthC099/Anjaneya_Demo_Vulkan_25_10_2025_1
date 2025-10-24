//Header file
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mmsystem.h>
//header file for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Scene0.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
// Vulkan related library
#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib,"winmm.lib")

//Callback function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void Update(void);


Win32WindowContext_Scene0 gWin32WindowCtx_Scene0;
GlobalContext_Scene0 gCtx_Scene0;
Win32FunctionTable_Scene0 gWin32FunctionTable_Scene0;
FunctionTable_Scene0 gFunctionTable_Scene0;

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


        //code
        gCtx_Scene0.gpFile = fopen("SSA_Log.txt", "w+");
        if (gCtx_Scene0.gpFile == NULL)
        {
                MessageBox(NULL, TEXT("Cannot create/open SSA_Log.txt file"), TEXT("FILE IO ERROR"), MB_ICONERROR);
                exit(0);
        }
        else
        {
                fprintf(gCtx_Scene0.gpFile, "WinMain() --> Program started successfully\n");
        }

        InitializeFunctionTable();

        wsprintf(szAppName, TEXT("%s"), gWin32WindowCtx_Scene0.gpszAppName);

        wndclass.cbSize = sizeof(WNDCLASSEX);
        wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wndclass.cbClsExtra = 0;
        wndclass.cbWndExtra = 0;
        wndclass.lpfnWndProc = gWin32FunctionTable_Scene0.WndProc;
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

	gWin32WindowCtx_Scene0.ghwnd = hwnd;

    vkResult = gWin32FunctionTable_Scene0.Initialize();
        if(vkResult != VK_SUCCESS)
        {
                fprintf(gCtx_Scene0.gpFile, "WinMain() --> Initialize() is failed\n");
                DestroyWindow(hwnd);
                hwnd = NULL;
        }
        else
        {
                fprintf(gCtx_Scene0.gpFile, "WinMain() --> Initialize() is succedded\n");
        }
        //Error checking of Initialize

	ShowWindow(hwnd, iCmdShow);
    gWin32FunctionTable_Scene0.Update();
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
			if (gWin32WindowCtx_Scene0.gbActiveWindow == 1)
			{
                                //Here you should call update() for OpenGL rendering
                                gWin32FunctionTable_Scene0.Update();
                                //Here you should call display() for OpenGL rendering
                                if(gWin32WindowCtx_Scene0.gbWindowMinimized == FALSE)
                                {
                                        vkResult = gWin32FunctionTable_Scene0.Display();
                                        if((vkResult != VK_FALSE) && (vkResult != VK_SUCCESS) && (vkResult != VK_ERROR_OUT_OF_DATE_KHR) && (vkResult != VK_SUBOPTIMAL_KHR))
                                        {
                                                fprintf(gCtx_Scene0.gpFile, "WinMain() --> call to Display() failed\n");
                                                bDone = TRUE;
                                        }
                                }
			}
		}
	}

        gWin32FunctionTable_Scene0.Uninitialize();

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
                memset(&gWin32WindowCtx_Scene0.wpPrev, 0, length);
                break;

        case WM_SETFOCUS:
                gWin32WindowCtx_Scene0.gbActiveWindow = 1;
                break;

        case WM_KILLFOCUS:
                gWin32WindowCtx_Scene0.gbActiveWindow = 0;
                break;

        case WM_ERASEBKGND:
                return(0);

        case WM_SIZE:
                if(wParam == SIZE_MINIMIZED)
                {
                        gWin32WindowCtx_Scene0.gbWindowMinimized = TRUE;
                }
                else
                {
                        gWin32WindowCtx_Scene0.gbWindowMinimized = FALSE;
                        gWin32FunctionTable_Scene0.Resize(LOWORD(lParam), HIWORD(lParam));
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
                        gWin32FunctionTable_Scene0.ToggleFullscreen();
                        break;

                default:
                        break;
                }
                break;

        case WM_CLOSE:
                DestroyWindow(hwnd);
                break;

        case WM_DESTROY:
                gWin32FunctionTable_Scene0.Uninitialize();
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
	if (gWin32WindowCtx_Scene0.gbFullscreen == 0) //If current window is normal(Not Fullscreen)
	{
		gWin32WindowCtx_Scene0.dwStyle = GetWindowLong(gWin32WindowCtx_Scene0.ghwnd, GWL_STYLE); //gWin32WindowCtx_Scene0.dwStyle will get WS_OVERLAPPEDWINDOW information
		if (gWin32WindowCtx_Scene0.dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(gWin32WindowCtx_Scene0.ghwnd, &gWin32WindowCtx_Scene0.wpPrev) && GetMonitorInfo(MonitorFromWindow(gWin32WindowCtx_Scene0.ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(gWin32WindowCtx_Scene0.ghwnd, GWL_STYLE, (gWin32WindowCtx_Scene0.dwStyle & ~WS_OVERLAPPEDWINDOW)); //Removing flags of WS_OVERLAPPEDWINDOW
				SetWindowPos(gWin32WindowCtx_Scene0.ghwnd,
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
		gWin32WindowCtx_Scene0.gbFullscreen = 1;
	}
	else
	{
		SetWindowLong(gWin32WindowCtx_Scene0.ghwnd, GWL_STYLE, (gWin32WindowCtx_Scene0.dwStyle | WS_OVERLAPPEDWINDOW));//Restoring flags of WS_OVERLAPPEDWINDOW
		SetWindowPlacement(gWin32WindowCtx_Scene0.ghwnd, &gWin32WindowCtx_Scene0.wpPrev); //setting placement of window
		SetWindowPos(gWin32WindowCtx_Scene0.ghwnd,
			HWND_TOP,
			0,  //already set in gWin32WindowCtx_Scene0.wpPrev
			0,  //already set in gWin32WindowCtx_Scene0.wpPrev
			0,  //already set in gWin32WindowCtx_Scene0.wpPrev
			0,  //already set in gWin32WindowCtx_Scene0.wpPrev
			SWP_NOMOVE | // do not change x, y cordinates for starting position
			SWP_NOSIZE | // do not change height and width cordinates
			SWP_NOOWNERZORDER | //do not change the position even if its parent window is changed
			SWP_NOZORDER | //Window flag --> don't change the Z order
			SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
		ShowCursor(TRUE); //Appear the cursor in full screen or Game mode
		gWin32WindowCtx_Scene0.gbFullscreen = 0;
	}
}



VkResult Initialize(void)
{
//variable delaration
VkResult vkResult = VK_SUCCESS;

//code
vkResult = gFunctionTable_Scene0.createVulkanInstance();
    /*Main points:
        vkEnumerateInstanceExtensionProperties()
        
        struct --> vkApplicationInfo
        struct --> gCtx_Scene0.vkInstanceCreateInfo
        vkCreateInstance()
    */
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createVulkanInstance() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createVulkanInstance() is succedded\n");
    }

//CreateVulkan Presntation Surface
vkResult = gFunctionTable_Scene0.getSupportedSurface();
    /* Main points:
        struct --> vkWin32SurfaceCreateInfoKHR
        vkCreateWin32SurfaceKHR()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> getSupportedSurface() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> getSupportedSurface() is succedded\n");
    }

//Get required Physical Device and its Queue family index
vkResult = gFunctionTable_Scene0.getPhysicalDevice();
    /* Main points:
        vkEnumeratePhysicalDevices()
        vkGetPhysicalDeviceQueueFamilyProperties()
        vkGetPhysicalDeviceSurfaceSupportKHR()
        vkGetPhysicalDeviceMemoryProperties()
        vkGetPhysicalDeviceFeatures()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> getPhysicalDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> getPhysicalDevice() is succedded\n");
    }
    
//Printing Vulkan Infomration
vkResult = gFunctionTable_Scene0.printVkInfo();
    /* Main points:
        vkGetPhysicalDeviceProperties()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> printVkInfo() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> printVkInfo() is succedded\n");
    }
    
vkResult = gFunctionTable_Scene0.createVulkanDevice();
    /* Main Points:
        vkEnumerateDeviceExtensionProperties()
        struct --> gCtx_Scene0.vkDeviceQueueCreateInfo
        struct --> gCtx_Scene0.vkDeviceCreateInfo
        vkCreateDevice()
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createVulkanDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createVulkanDevice() is succedded\n");
    }
    
//getDeviceQueue
gFunctionTable_Scene0.getDeviceQueue();
    /* Main Points
        vkGetDeviceQueue()
    */

        
//Swapchain
vkResult = gFunctionTable_Scene0.createSwapchain(VK_FALSE);
    /*Main Points:
        vkGetPhysicalDeviceSurfaceFormatsKHR()
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
        vkGetPhysicalDeviceSurfacePresentModesKHR()
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createSwapchain() is failed %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createSwapchain() is succedded\n");
    }
    
//Create Vulkan Images and ImageViews
vkResult = gFunctionTable_Scene0.createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createImagesAndImageViews() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createImagesAndImageViews() is succedded\n");
    }
    
//Create Command Pool
vkResult = gFunctionTable_Scene0.createCommandPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createCommandPool() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createCommandPool() is succedded\n");
    }
    
//Command Buffers
vkResult = gFunctionTable_Scene0.createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createCommandBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createCommandBuffers() is succedded\n");
    }
    
//create VertexBUffer
vkResult = gFunctionTable_Scene0.createVertexBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createVertexBuffer() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createVertexBuffer() is succedded\n");
    }
    
//create texture
vkResult = gFunctionTable_Scene0.createTexture("Vijay_Kundali.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createTexture() is failed for Stone texture %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createTexture() is succedded for Stone texture\n");
    }
    
//create Uniform Buffer
vkResult = gFunctionTable_Scene0.createUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createUniformBuffer() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createUniformBuffer() is succedded\n");
    }
    
//Create RenderPass
vkResult = gFunctionTable_Scene0.createShaders();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createShaders() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createShaders() is succedded\n");
    }
    
    
    
//Create Dewscriptor Set Layout
vkResult = gFunctionTable_Scene0.createDescriptorSetLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createDescriptorSetLayout() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createDescriptorSetLayout() is succedded\n");
    }
    
//Create Descriptor Set Layout
vkResult = gFunctionTable_Scene0.createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createPipelineLayout() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createPipelineLayout() is succedded\n");
    }
    
//create Descriptor pool
vkResult = gFunctionTable_Scene0.createDescriptorPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createDescriptorPool() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createDescriptorPool() is succedded\n");
    }
    
    //create Descriptor set
    vkResult = gFunctionTable_Scene0.createDescriptorSet();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createDescriptorSet() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createDescriptorPool() is succedded\n");
    }
    
    //Create RenderPass
    vkResult = gFunctionTable_Scene0.createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createRenderPass() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createRenderPass() is succedded\n");
    }
    
    //Create Pipeline
    vkResult = gFunctionTable_Scene0.createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createPipeline() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createPipeline() is succedded\n");
    }
    
    
    //CreateBuffer
    vkResult = gFunctionTable_Scene0.createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createFrameBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createFrameBuffers() is succedded\n");
    }
    
    //CreateSemaphores
    vkResult = gFunctionTable_Scene0.createSemaphores();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createSemaphores() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createSemaphores() is succedded\n");
    }
    
    //CreateFences
    vkResult = gFunctionTable_Scene0.createFences();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createFences() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> createFences() is succedded\n");
    }
    
    //Initlaize gCtx_Scene0.vkClearColorValue
    memset((void*)&gCtx_Scene0.vkClearColorValue, 0, sizeof(VkClearColorValue));
    gCtx_Scene0.vkClearColorValue.float32[0] = 0.0f;// R
    gCtx_Scene0.vkClearColorValue.float32[1] = 0.0f;// G
    gCtx_Scene0.vkClearColorValue.float32[2] = 0.0f;// B
    gCtx_Scene0.vkClearColorValue.float32[3] = 1.0f;//Analogous to glClearColor()
    
    memset((void*)&gCtx_Scene0.vkClearDepthStencilValue, 0, sizeof(VkClearDepthStencilValue));
    //setDefaultClearDepth
    gCtx_Scene0.vkClearDepthStencilValue.depth = 1.0f; //flaot value
    //setDeafaultStencilValue
    gCtx_Scene0.vkClearDepthStencilValue.stencil = 0; //uint32_t value
    
    //BuildCommandBuffers
    vkResult = gFunctionTable_Scene0.buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> buildCommandBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "Initialize() --> buildCommandBuffers() is succedded\n");
    }
    
    //Initialization is completed
    gCtx_Scene0.bInitialized = TRUE;
    fprintf(gCtx_Scene0.gpFile, "Initialize() --> Initialization is completed successfully\n");
    
    PlaySound(MAKEINTRESOURCE(MYAUDIO), NULL, SND_ASYNC | SND_RESOURCE);
  
    return vkResult;
}


VkResult Resize(int width, int height)
{
//variables
VkResult vkResult = VK_SUCCESS;
	//code
	if (height == 0)
		height = 1;
    
    gWin32WindowCtx_Scene0.giHeight = height;
    gWin32WindowCtx_Scene0.giWidth = width;
    

    //check the gCtx_Scene0.bInitialized variable
    if(gCtx_Scene0.bInitialized == FALSE)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> Initialization yet not completed or failed\n");  
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    
    //As recreation of swapchain is needed we are goring to repeat many steps of initialized again, hence set bINitialized to \False again
    gCtx_Scene0.bInitialized = FALSE;

    //Set gloabl winwidth and winheight variable
    gCtx_Scene0.winWidth = width;
    gCtx_Scene0.winHeight = height;
    
    //wait for device to complete inhand tasks
    vkDeviceWaitIdle(gCtx_Scene0.vkDevice);
    fprintf(gCtx_Scene0.gpFile, "Resize() --> vkDeviceWaitIdle() is done\n");
    
    //check presence of swapchain
    if(gCtx_Scene0.vkSwapchainKHR == VK_NULL_HANDLE)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> swapchain is already NULL cannot proceed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
    }
    
    //Destroy vkframebuffer
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(gCtx_Scene0.vkDevice, gCtx_Scene0.vkFrameBuffer_array[i], NULL);
    }
    if(gCtx_Scene0.vkFrameBuffer_array)
    {
        free(gCtx_Scene0.vkFrameBuffer_array);
        gCtx_Scene0.vkFrameBuffer_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Resize() --> gCtx_Scene0.vkFrameBuffer_array() is done\n");
    }
    
    //Destroy Commandbuffer
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(gCtx_Scene0.vkDevice, gCtx_Scene0.vkCommandPool, 1, &gCtx_Scene0.vkCommandBuffer_array[i]);
        fprintf(gCtx_Scene0.gpFile, "Resize() --> vkFreeCommandBuffers() is done\n");
    }
    
    if(gCtx_Scene0.vkCommandBuffer_array)
    {
        free(gCtx_Scene0.vkCommandBuffer_array);
        gCtx_Scene0.vkCommandBuffer_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Resize() --> gCtx_Scene0.vkCommandBuffer_array is freed\n");
    }
    
    
    //Destroy Pipeline
    if(gCtx_Scene0.vkPipeline)
    {
        vkDestroyPipeline(gCtx_Scene0.vkDevice, gCtx_Scene0.vkPipeline, NULL);
        gCtx_Scene0.vkPipeline = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Resize() --> vkDestroyPipeline() is done\n");
    }
    
    //Destroy gCtx_Scene0.vkPipelineLayout
    if(gCtx_Scene0.vkPipelineLayout)
    {
        vkDestroyPipelineLayout(gCtx_Scene0.vkDevice, gCtx_Scene0.vkPipelineLayout, NULL);
        gCtx_Scene0.vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Resize() --> gCtx_Scene0.vkPipelineLayout() is done\n");
    }
    
    //Destroy Renderpass
    if(gCtx_Scene0.vkRenderPass)
    {
        vkDestroyRenderPass(gCtx_Scene0.vkDevice, gCtx_Scene0.vkRenderPass, NULL);
        gCtx_Scene0.vkRenderPass = VK_NULL_HANDLE;
    }
    
    //destroy depth image view
    if(gCtx_Scene0.vkImageView_depth)
    {
        vkDestroyImageView(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImageView_depth, NULL);
        gCtx_Scene0.vkImageView_depth = VK_NULL_HANDLE;
    }
    
    //destroy device memory for depth image
    if(gCtx_Scene0.vkDeviceMemory_depth)
    {
        vkFreeMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vkDeviceMemory_depth, NULL);
        gCtx_Scene0.vkDeviceMemory_depth = VK_NULL_HANDLE;
    }
    
    if(gCtx_Scene0.vkImage_depth)
    {
        vkDestroyImage(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImage_depth, NULL);
        gCtx_Scene0.vkImage_depth = VK_NULL_HANDLE;
    }
    //destory image views
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        vkDestroyImageView(gCtx_Scene0.vkDevice, gCtx_Scene0.swapchainImageView_array[i], NULL);
        fprintf(gCtx_Scene0.gpFile, "Resize() --> vkDestoryImageView() is done\n");
    }
    
    if(gCtx_Scene0.swapchainImageView_array)
    {
        free(gCtx_Scene0.swapchainImageView_array);
        gCtx_Scene0.swapchainImageView_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Resize() --> gCtx_Scene0.swapchainImageView_array is freed\n");
    }
    
    //free swapchainImages
    // for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    // {
        // vkDestroyImage(gCtx_Scene0.vkDevice, gCtx_Scene0.swapchainImage_array[i], NULL);
        // fprintf(gCtx_Scene0.gpFile, "Resize() --> vkDestroyImage() is done\n");
    // }
    
    
    if(gCtx_Scene0.swapchainImage_array)
    {
        free(gCtx_Scene0.swapchainImage_array);
        gCtx_Scene0.swapchainImage_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Resize() --> gCtx_Scene0.swapchainImage_array is freed\n");
    }
    
    //Destory Swapchain
    if(gCtx_Scene0.vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(gCtx_Scene0.vkDevice, gCtx_Scene0.vkSwapchainKHR, NULL);
        gCtx_Scene0.vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Resize() --> vkSwapchainCreateInfoKHR() is done\n");
    }
    
    //RECREATE FOR RESIZE
    
//Swapchain
vkResult = gFunctionTable_Scene0.createSwapchain(VK_FALSE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> createSwapchain() is failed %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
//Create Vulkan Images and ImageViews
vkResult = gFunctionTable_Scene0.createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> createImagesAndImageViews() is failed %d\n", vkResult);
    }
    
//Create RenderPass
vkResult = gFunctionTable_Scene0.createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> createRenderPass() is failed %d\n", vkResult);
    }
    
    //Create Descriptor Set Layout
    vkResult = gFunctionTable_Scene0.createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> createPipelineLayout() is failed %d\n", vkResult);
    }
    
//Create Pipeline
vkResult = gFunctionTable_Scene0.createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> createPipeline() is failed %d\n", vkResult);
    }
      
//CreateBuffer
vkResult = gFunctionTable_Scene0.createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> createFrameBuffers() is failed %d\n", vkResult);
    }
    
//Command Buffers
vkResult = gFunctionTable_Scene0.createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> createCommandBuffers() is failed %d\n", vkResult);
    }
    
//BuildCommandBuffers
vkResult = gFunctionTable_Scene0.buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Resize() --> buildCommandBuffers() is failed %d\n", vkResult);
    }

    gCtx_Scene0.bInitialized = TRUE;

    return vkResult;
}



VkResult Display(void)
{
//variable declarations
VkResult vkResult = VK_SUCCESS;

    //code
    // if control comes here before initlaization is completed, return FALSE
    if(gCtx_Scene0.bInitialized == FALSE)
    {
        fprintf(gCtx_Scene0.gpFile, "Display() --> Initialization yet not completed\n");
        return (VkResult)VK_FALSE;
    }
    
    //acquire index of next swapchin image
    //if timour occurs, then function returns VK_NOT_READY
    vkResult = vkAcquireNextImageKHR(gCtx_Scene0.vkDevice,
                                     gCtx_Scene0.vkSwapchainKHR,
                                     UINT64_MAX, //waiting time in nanaoseconds for swapchain to get the image
                                     gCtx_Scene0.vkSemaphore_backBuffer, //semaphore, waiting for another queue to relaease the image held by another queue demanded by swapchain, (InterQueue semaphore)
                                     VK_NULL_HANDLE, //Fence, when you want to halt host also, for device::: (Use Semaphore and fences exclusively, using both is not recommended(Redbook)
                                     &gCtx_Scene0.currentImageIndex);
    if(vkResult != VK_SUCCESS)
    {
        if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
        {
            gWin32FunctionTable_Scene0.Resize(gCtx_Scene0.winWidth, gCtx_Scene0.winHeight);
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "Display() --> vkAcquireNextImageKHR() is failed errorcode = %d\n", vkResult);
            return vkResult;   
        }
    }
    
    //use fence to allow host to wait for completion of execution of previous commandbuffer
    vkResult = vkWaitForFences(gCtx_Scene0.vkDevice,
                               1, //waiting for how many fences
                               &gCtx_Scene0.vkFence_array[gCtx_Scene0.currentImageIndex], //Which fence
                               VK_TRUE, // wait till all fences get signalled(Blocking and unblocking function)
                               UINT64_MAX); //waiting time in nanaoseconds
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Display() --> vkWaitForFences() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }

    //Now make Fences execution of next command buffer
    vkResult = vkResetFences(gCtx_Scene0.vkDevice,
                             1, //How many fences to reset
                             &gCtx_Scene0.vkFence_array[gCtx_Scene0.currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Display() --> vkResetFences() is failed errorcode = %d\n", vkResult);
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
    vkSubmitInfo.pWaitSemaphores = &gCtx_Scene0.vkSemaphore_backBuffer;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &gCtx_Scene0.vkCommandBuffer_array[gCtx_Scene0.currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &gCtx_Scene0.vkSemaphore_renderComplete;
    
    //Now submit our work to the Queue
    vkResult = vkQueueSubmit(gCtx_Scene0.vkQueue,
                             1,
                             &vkSubmitInfo,
                             gCtx_Scene0.vkFence_array[gCtx_Scene0.currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Display() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    
    //We are going to present rendered image after declaring and initlaizing VkPresentInfoKHR structure
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
    
    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &gCtx_Scene0.vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &gCtx_Scene0.currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &gCtx_Scene0.vkSemaphore_renderComplete;
    
    //Now present the Queue
    vkResult = vkQueuePresentKHR(gCtx_Scene0.vkQueue, &vkPresentInfoKHR);
    if(vkResult != VK_SUCCESS)
    {
        if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
        {
            gWin32FunctionTable_Scene0.Resize(gCtx_Scene0.winWidth, gCtx_Scene0.winHeight);
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "Display() --> vkQueuePresentKHR() is failed errorcode = %d\n", vkResult);
            return vkResult;
        }
    }
    
vkResult = gFunctionTable_Scene0.updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "Display() --> updateUniformBuffer() is failed errorcode = %d\n", vkResult);
    }
        
    //validation
    vkDeviceWaitIdle(gCtx_Scene0.vkDevice);    
    
    return vkResult;
}
    
void Update(void)
{
    gCtx_Scene0.gFade = 1.0f;
}

void Uninitialize(void)
{
	//code
	if (gWin32WindowCtx_Scene0.gbFullscreen == 1)
	{
		gWin32WindowCtx_Scene0.dwStyle = GetWindowLong(gWin32WindowCtx_Scene0.ghwnd, GWL_STYLE);
		SetWindowLong(gWin32WindowCtx_Scene0.ghwnd, GWL_STYLE, (gWin32WindowCtx_Scene0.dwStyle | WS_OVERLAPPEDWINDOW));//Restoring flags of WS_OVERLAPPEDWINDOW
		SetWindowPlacement(gWin32WindowCtx_Scene0.ghwnd, &gWin32WindowCtx_Scene0.wpPrev); //setting placement of window
		SetWindowPos(gWin32WindowCtx_Scene0.ghwnd,
			HWND_TOP,
			0,  //already set in gWin32WindowCtx_Scene0.wpPrev
			0,  //already set in gWin32WindowCtx_Scene0.wpPrev
			0,  //already set in gWin32WindowCtx_Scene0.wpPrev
			0,  //already set in gWin32WindowCtx_Scene0.wpPrev
			SWP_NOMOVE | // do not change x, y cordinates for starting position
			SWP_NOSIZE | // do not change height and width cordinates
			SWP_NOOWNERZORDER | //do not change the position even if its parent window is changed
			SWP_NOZORDER | //Window flag --> don't change the Z order
			SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
		ShowCursor(TRUE); //Appear the cursor in full screen or Game mode
	}

	/**************Shader removal code*******************/


	if (gWin32WindowCtx_Scene0.ghdc)
	{
		ReleaseDC(gWin32WindowCtx_Scene0.ghwnd, gWin32WindowCtx_Scene0.ghdc);
		gWin32WindowCtx_Scene0.ghdc = NULL;
	}

    //synchronisation function
    vkDeviceWaitIdle(gCtx_Scene0.vkDevice);
    fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDeviceWaitIdle() is done\n");
    
    //DestroyFences
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        vkDestroyFence(gCtx_Scene0.vkDevice, gCtx_Scene0.vkFence_array[i], NULL);
    }
    
    if(gCtx_Scene0.vkFence_array)
    {
        free(gCtx_Scene0.vkFence_array);
        gCtx_Scene0.vkFence_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyFence() is done\n");
    }
    
    //DestroySemaphore
    if(gCtx_Scene0.vkSemaphore_renderComplete)
    {
        vkDestroySemaphore(gCtx_Scene0.vkDevice, gCtx_Scene0.vkSemaphore_renderComplete, NULL);
        gCtx_Scene0.vkSemaphore_renderComplete = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroySemaphore() for gCtx_Scene0.vkSemaphore_renderComplete is done\n");
    }
    
    if(gCtx_Scene0.vkSemaphore_backBuffer)
    {
        vkDestroySemaphore(gCtx_Scene0.vkDevice, gCtx_Scene0.vkSemaphore_backBuffer, NULL);
        gCtx_Scene0.vkSemaphore_backBuffer = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroySemaphore() for gCtx_Scene0.vkSemaphore_backBuffer is done\n");
    }
    
    //vkframebuffer
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(gCtx_Scene0.vkDevice, gCtx_Scene0.vkFrameBuffer_array[i], NULL);
    }
    if(gCtx_Scene0.vkFrameBuffer_array)
    {
        free(gCtx_Scene0.vkFrameBuffer_array);
        gCtx_Scene0.vkFrameBuffer_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> gCtx_Scene0.vkFrameBuffer_array() is done\n");
    }
    
    //Destroy Descriptor Pool
    //When Descriptor pool is destroyed, descriptor set created  by that pool get destroyed implicitly
    if(gCtx_Scene0.vkDescriptorPool)
    {
        vkDestroyDescriptorPool(gCtx_Scene0.vkDevice, gCtx_Scene0.vkDescriptorPool, NULL);        
        gCtx_Scene0.vkDescriptorPool = VK_NULL_HANDLE;
        gCtx_Scene0.vkDescriptorSet = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> gCtx_Scene0.vkDescriptorPool and DescriptorSet() is done\n");
    }
    
    //VkDescritporSetLayout
    if(gCtx_Scene0.vkPipelineLayout)
    {
        vkDestroyPipelineLayout(gCtx_Scene0.vkDevice, gCtx_Scene0.vkPipelineLayout, NULL);
        gCtx_Scene0.vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> gCtx_Scene0.vkPipelineLayout() is done\n");
    }
    
    //VkDescritporSetLayout
    if(gCtx_Scene0.vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(gCtx_Scene0.vkDevice, gCtx_Scene0.vkDescriptorSetLayout, NULL);
        gCtx_Scene0.vkDescriptorSetLayout = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> gCtx_Scene0.vkDescriptorSetLayout() is done\n");
    }
    
    if(gCtx_Scene0.vkPipeline)
    {
        vkDestroyPipeline(gCtx_Scene0.vkDevice, gCtx_Scene0.vkPipeline, NULL);
        gCtx_Scene0.vkPipeline = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyPipeline() is done\n");
    }
    
    //Renderpass
    if(gCtx_Scene0.vkRenderPass)
    {
        vkDestroyRenderPass(gCtx_Scene0.vkDevice, gCtx_Scene0.vkRenderPass, NULL);
        gCtx_Scene0.vkRenderPass = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyRenderPass() is done\n");
    }
    
    //destroy shader modules
    if(gCtx_Scene0.vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(gCtx_Scene0.vkDevice, gCtx_Scene0.vkShaderModule_fragment_shader, NULL);
        gCtx_Scene0.vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyShaderModule() is done\n");
    }
    
    if(gCtx_Scene0.vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(gCtx_Scene0.vkDevice, gCtx_Scene0.vkShaderModule_vertex_shader, NULL);
        gCtx_Scene0.vkShaderModule_vertex_shader = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyShaderModule() is done\n");
    }
    
    //Destroy uniform buffer
    if(gCtx_Scene0.uniformData.vkBuffer)
    {
        vkDestroyBuffer(gCtx_Scene0.vkDevice, gCtx_Scene0.uniformData.vkBuffer, NULL);
        gCtx_Scene0.uniformData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyBuffer()  for gCtx_Scene0.uniformData.vkBuffer is done\n");
    }
    
    if(gCtx_Scene0.uniformData.vkDeviceMemory)
    {
        vkFreeMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.uniformData.vkDeviceMemory, NULL);
        gCtx_Scene0.uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkFreeMemory()  for gCtx_Scene0.uniformData.vkDeviceMemory is done\n");
        
    }
    
    if(gCtx_Scene0.vkSampler_texture)
    {
        vkDestroySampler(gCtx_Scene0.vkDevice, gCtx_Scene0.vkSampler_texture, NULL);
        gCtx_Scene0.vkSampler_texture = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroySampler() for gCtx_Scene0.vkSampler_texture is done\n");
    }
    
    if(gCtx_Scene0.vkImageView_texture)
    {
        vkDestroyImageView(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImageView_texture, NULL);
        gCtx_Scene0.vkImageView_texture = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroySampler() for gCtx_Scene0.vkImageView_texture is done\n");
    }
    
    if(gCtx_Scene0.vkDeviceMemory_texture)
    {
        vkFreeMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vkDeviceMemory_texture, NULL);
        gCtx_Scene0.vkDeviceMemory_texture = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkFreeMemory() for gCtx_Scene0.vkDeviceMemory_texture is done\n");
    }
    
    if(gCtx_Scene0.vkImage_texture)
    {
        vkDestroyImage(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImage_texture, NULL);
        gCtx_Scene0.vkImage_texture = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyImage() for gCtx_Scene0.vkImage_texture is done\n");
    }

    
    //Vertex Tecoord buffer
    if(gCtx_Scene0.vertexData_texcoord.vkDeviceMemory)
    {
        vkFreeMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_texcoord.vkDeviceMemory, NULL);
        gCtx_Scene0.vertexData_texcoord.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkFreeMemory() is done\n");
    }
    
    if(gCtx_Scene0.vertexData_texcoord.vkBuffer)
    {
        vkDestroyBuffer(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_texcoord.vkBuffer, NULL);
        gCtx_Scene0.vertexData_texcoord.vkBuffer = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyBuffer() is done\n");
    }

    //Vertex position BUffer
    if(gCtx_Scene0.vertexData_position.vkDeviceMemory)
    {
        vkFreeMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_position.vkDeviceMemory, NULL);
        gCtx_Scene0.vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkFreeMemory() is done\n");
    }
    
    if(gCtx_Scene0.vertexData_position.vkBuffer)
    {
        vkDestroyBuffer(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_position.vkBuffer, NULL);
        gCtx_Scene0.vertexData_position.vkBuffer = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyBuffer() is done\n");
    }

    
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(gCtx_Scene0.vkDevice, gCtx_Scene0.vkCommandPool, 1, &gCtx_Scene0.vkCommandBuffer_array[i]);
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkFreeCommandBuffers() is done\n");
    }
    
    if(gCtx_Scene0.vkCommandBuffer_array)
    {
        free(gCtx_Scene0.vkCommandBuffer_array);
        gCtx_Scene0.vkCommandBuffer_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> gCtx_Scene0.vkCommandBuffer_array is freed\n");
    }
    
    
    if(gCtx_Scene0.vkCommandPool)
    {
        vkDestroyCommandPool(gCtx_Scene0.vkDevice, gCtx_Scene0.vkCommandPool, NULL);
        gCtx_Scene0.vkCommandPool = NULL;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyCommandPool() is done\n");
    }
    
    //destroy depth image view
    if(gCtx_Scene0.vkImageView_depth)
    {
        vkDestroyImageView(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImageView_depth, NULL);
        gCtx_Scene0.vkImageView_depth = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestoryImageView() is done for depth\n");
    }
    
    //destroy device memory for depth image
    if(gCtx_Scene0.vkDeviceMemory_depth)
    {
        vkFreeMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vkDeviceMemory_depth, NULL);
        gCtx_Scene0.vkDeviceMemory_depth = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkFreeMemory() is done for depth\n");
    }
    
    if(gCtx_Scene0.vkImage_depth)
    {
        vkDestroyImage(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImage_depth, NULL);
        gCtx_Scene0.vkImage_depth = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestoryImage() is done for depth\n");
    }
    
    //destory image views
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        vkDestroyImageView(gCtx_Scene0.vkDevice, gCtx_Scene0.swapchainImageView_array[i], NULL);
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestoryImageView() is done for color\n");
    }
    
    if(gCtx_Scene0.swapchainImageView_array)
    {
        free(gCtx_Scene0.swapchainImageView_array);
        gCtx_Scene0.swapchainImageView_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> gCtx_Scene0.swapchainImageView_array is freed\n");
    }
    
    //free swapchainImages
    // for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    // {
        // vkDestroyImage(gCtx_Scene0.vkDevice, gCtx_Scene0.swapchainImage_array[i], NULL);
        // fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyImage() is done\n");
    // }
    
    if(gCtx_Scene0.swapchainImage_array)
    {
        free(gCtx_Scene0.swapchainImage_array);
        gCtx_Scene0.swapchainImage_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> gCtx_Scene0.swapchainImage_array is freed\n");
    }
    
  
    
    //Destory Swapchain
    if(gCtx_Scene0.vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(gCtx_Scene0.vkDevice, gCtx_Scene0.vkSwapchainKHR, NULL);
        gCtx_Scene0.vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkSwapchainCreateInfoKHR() is done\n");
    }

    //No need to Destroy/Uninitialize the DeviceQueue

    //Destroy vulkan device
    if(gCtx_Scene0.vkDevice)
    {
        vkDestroyDevice(gCtx_Scene0.vkDevice, NULL);
        gCtx_Scene0.vkDevice = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestoryDevice() is done\n");
    }

    //No need to free slected physical device

    //Destroy gCtx_Scene0.vkSurfaceKHR:This function is generic and not platform specific
    if(gCtx_Scene0.vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(gCtx_Scene0.vkInstance, gCtx_Scene0.vkSurfaceKHR, NULL);
        gCtx_Scene0.vkSurfaceKHR = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroySurfaceKHR() is done\n");
    }
    
    //Validation destroying
    if(gCtx_Scene0.vkDebugReportCallbackEXT && gCtx_Scene0.vkDestroyDebugReportCallbackEXT_fnptr)
    {
        gCtx_Scene0.vkDestroyDebugReportCallbackEXT_fnptr(gCtx_Scene0.vkInstance, 
                                              gCtx_Scene0.vkDebugReportCallbackEXT,
                                              NULL);
        gCtx_Scene0.vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        gCtx_Scene0.vkDestroyDebugReportCallbackEXT_fnptr = NULL;
    }
    
    
    //Destroy Vulkan Instance
    if(gCtx_Scene0.vkInstance)
    {
        vkDestroyInstance(gCtx_Scene0.vkInstance, NULL);
        gCtx_Scene0.vkInstance = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> vkDestroyInstance() is done\n");
    }
    
    if (gCtx_Scene0.gpFile)
	{
		fprintf(gCtx_Scene0.gpFile, "Uninitialize() --> Program terminated successfully\n");
		fclose(gCtx_Scene0.gpFile);
		gCtx_Scene0.gpFile = NULL;
	}
}

/***************Definition of Vulkan functions***********************/
/********************************************************************/


VkResult createVulkanInstance(void)
{
//variable declarations
VkResult vkResult = VK_SUCCESS;

    //code
//Step1: Fill and initialize required extension names and count in global variable
vkResult = gFunctionTable_Scene0.fillInstanceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> fillInstanceExtensionNames() is failed\n");
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> fillInstanceExtensionNames() is succedded\n");
    }

if(gCtx_Scene0.bValidation == TRUE)
{
//Fill validation Layer Names
vkResult = gFunctionTable_Scene0.fillValidationLayerNames();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> fillValidationLayerNames() is failed\n");
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> fillValidationLayerNames() is succedded\n");
        }
    }


    //Step1: Initialize struct VkApplicationInfo
    VkApplicationInfo vkApplicationInfo;
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //type safety, generic names
    vkApplicationInfo.pNext = NULL; //Linked List
    vkApplicationInfo.pApplicationName = gWin32WindowCtx_Scene0.gpszAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gWin32WindowCtx_Scene0.gpszAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;


    //Step3: Initialize struct VkInstanceCreateInfo by using information in Step1 and Step2
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = gCtx_Scene0.enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = gCtx_Scene0.enabledInstanceExtensionNames_array;
    
    if(gCtx_Scene0.bValidation == TRUE)
    {
        vkInstanceCreateInfo.enabledLayerCount = gCtx_Scene0.enabledValidationLayerCount;
        vkInstanceCreateInfo.ppEnabledLayerNames = gCtx_Scene0.enabledValidationLayerNames_array;
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
    //Step4: Call VkCreateInstance() to get gCtx_Scene0.vkInstance in a global variable and do error checking
    vkResult = vkCreateInstance(&vkInstanceCreateInfo,
                                 NULL,  //no custom Memory allocater
                                 &gCtx_Scene0.vkInstance);
    if(vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to incompatible driver %d\n", vkResult);
        return vkResult;
    }
    else if(vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to required extension not present %d\n", vkResult);
        return vkResult;
    }
    else if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to unknown reason %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance succedded\n");
    }
    
    //do for validation callbacks
if(gCtx_Scene0.bValidation == TRUE)
{
vkResult = gFunctionTable_Scene0.createValidationCallbackFunction();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> createValidationCallbackFunction() is failed\n");
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "createVulkanInstance() --> createValidationCallbackFunction() is succedded\n");
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

    memset((void*)gCtx_Scene0.enabledInstanceExtensionNames_array, 0,
           sizeof(gCtx_Scene0.enabledInstanceExtensionNames_array));
    gCtx_Scene0.enabledInstanceExtensionCount = 0;

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
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is failed\n");
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is succedded\n");
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
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is failed\n");
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is succedded\n");
    }



    //Step3: Fill and Display a local string array of extension names obtained from vkExtensionProperties
    char** instanceExtensionNames_array = NULL;

    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> Vulkan Instance Extension names = %s\n", instanceExtensionNames_array[i]);
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
            gCtx_Scene0.enabledInstanceExtensionNames_array[gCtx_Scene0.enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
        
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanWin32SurfaceExtensionFound = VK_TRUE;
            gCtx_Scene0.enabledInstanceExtensionNames_array[gCtx_Scene0.enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
        
        if(strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
        {
            vulkanDebugReportExtensionFound = VK_TRUE;
            if(gCtx_Scene0.bValidation == TRUE)
            {
                gCtx_Scene0.enabledInstanceExtensionNames_array[gCtx_Scene0.enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
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
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME found\n");
    }

    if(vulkanWin32SurfaceExtensionFound == VK_FALSE)
    {
        // return hardcoded failure
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME found\n");
    }
    
    if(vulkanDebugReportExtensionFound == VK_FALSE)
    {
        if(gCtx_Scene0.bValidation == TRUE)
        {
            // return hardcoded failure
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is ON But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is OFF But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
        }
    }
    else
    {
        if(gCtx_Scene0.bValidation == TRUE)
        {
            fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is ON and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is OFF and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
        }
    }


    //Step8: Print only Enabled Extension Names 
    for(uint32_t i = 0; i < gCtx_Scene0.enabledInstanceExtensionCount; i++)
    {
         fprintf(gCtx_Scene0.gpFile, "fillInstanceExtensionNames() --> Enabled vulkan Instance extension Names = %s\n", gCtx_Scene0.enabledInstanceExtensionNames_array[i]);
    }

    return vkResult;
}


VkResult fillValidationLayerNames(void)
{
    //code
    //variables
    VkResult vkResult = VK_SUCCESS;
    uint32_t validationLayerCount = 0;

    memset((void*)gCtx_Scene0.enabledValidationLayerNames_array, 0,
           sizeof(gCtx_Scene0.enabledValidationLayerNames_array));
    gCtx_Scene0.enabledValidationLayerCount = 0;
    
    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  NULL); // Instance Validation Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is succedded\n");
    }
    
    VkLayerProperties* vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*) malloc (sizeof(VkLayerProperties) * validationLayerCount);
    //Should be error checking for malloc: assert() can also be used

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  vkLayerProperties_array); // Instance Validation Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is succedded\n");
    }
    
    char** validationLayerNames_array = NULL;
    validationLayerNames_array = (char**) malloc(sizeof(char*) * validationLayerCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        validationLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
        //Should be error checking for malloc: assert() can also be used
        memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
        fprintf(gCtx_Scene0.gpFile, "fillValidationLayerNames() --> Vulkan Validation Layer names = %s\n", vkLayerProperties_array[i].layerName);
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
            gCtx_Scene0.enabledValidationLayerNames_array[gCtx_Scene0.enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }
    
     //As not required henceforth, free the local string array
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        free(validationLayerNames_array[i]);
    }
    free(validationLayerNames_array);
    
    if(gCtx_Scene0.bValidation == TRUE)
    {
        //Step7:Print whether our vulkan driver supports our required extension names or not
        if(vulkanValidationLayerFound == VK_FALSE)
        {
            // return hardcoded failure
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gCtx_Scene0.gpFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation not supported\n");
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation is supported\n");
        }
    }
    
    //Step8: Print only Enabled validation layer Names
    for(uint32_t i = 0; i < gCtx_Scene0.enabledValidationLayerCount; i++)
    {
         fprintf(gCtx_Scene0.gpFile, "fillValidationLayerNames() --> Enabled vulkan validation layer Names = %s\n", gCtx_Scene0.enabledValidationLayerNames_array[i]);
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
    
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;
    
    //Get the required function pointers
    vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(gCtx_Scene0.vkInstance, "vkCreateDebugReportCallbackEXT");
    if(vkCreateDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gCtx_Scene0.gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is failed to get function pointer for vkCreateDebugReportCallbackEXT \n");
        return (vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is succedded to get function pointer for vkCreateDebugReportCallbackEXT \n");
    }
    
    
   // Assign to the global function pointer (no local shadowing)
    gCtx_Scene0.vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(gCtx_Scene0.vkInstance, "vkDestroyDebugReportCallbackEXT");
    if(gCtx_Scene0.vkDestroyDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gCtx_Scene0.gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is failed to get function pointer for vkDestroyDebugReportCallbackEXT \n");
        return (vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() is succedded to get function pointer for vkDestroyDebugReportCallbackEXT \n");
    }
    
    
    //Get the vulkanDebugReportCallback object
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
    memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = gFunctionTable_Scene0.debugReportCallback;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;
    
    vkResult = vkCreateDebugReportCallbackEXT_fnptr(gCtx_Scene0.vkInstance,
                                                    &vkDebugReportCallbackCreateInfoEXT,
                                                    NULL,
                                                    &gCtx_Scene0.vkDebugReportCallbackEXT);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createValidationCallbackFunction() --> vkCreateDebugReportCallbackEXT_fnptr() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createValidationCallbackFunction() --> vkCreateDebugReportCallbackEXT_fnptr() is succedded\n");
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
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(gWin32WindowCtx_Scene0.ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = gWin32WindowCtx_Scene0.ghwnd;

    //Step3:
    vkResult = vkCreateWin32SurfaceKHR(gCtx_Scene0.vkInstance,
                                       &vkWin32SurfaceCreateInfoKHR,
                                       NULL, //Memory mamnagement function is default
                                       &gCtx_Scene0.vkSurfaceKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is succedded\n");
    }

    return vkResult;
}


VkResult getPhysicalDevice(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;
    

    //code
    vkResult = vkEnumeratePhysicalDevices(gCtx_Scene0.vkInstance,
                                          &gCtx_Scene0.physicalDeviceCount,
                                          NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(gCtx_Scene0.physicalDeviceCount == 0)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() resulted in zero devices\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is succedded\n");
    }

    gCtx_Scene0.vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * gCtx_Scene0.physicalDeviceCount);
    //error checking to be done
    
    vkResult = vkEnumeratePhysicalDevices(gCtx_Scene0.vkInstance, 
                                          &gCtx_Scene0.physicalDeviceCount,
                                          gCtx_Scene0.vkPhysicalDevice_array);
     if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is succedded\n");
    }

    VkBool32 bFound = VK_FALSE;
    for(uint32_t i = 0; i < gCtx_Scene0.physicalDeviceCount; i++)
    {
        uint32_t qCount = UINT32_MAX;
        
        //If physical device is present then it must support at least 1 queue family
        vkGetPhysicalDeviceQueueFamilyProperties(gCtx_Scene0.vkPhysicalDevice_array[i], 
                                               &qCount, 
                                               NULL);
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * qCount);
        //error checking to be done
        
        vkGetPhysicalDeviceQueueFamilyProperties(gCtx_Scene0.vkPhysicalDevice_array[i], 
                                               &qCount, 
                                               vkQueueFamilyProperties_array);
        
        VkBool32* isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * qCount);
        //error checking to be done
        
        for(uint32_t j = 0; j < qCount; j++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(gCtx_Scene0.vkPhysicalDevice_array[i], 
                                                 j,
                                                 gCtx_Scene0.vkSurfaceKHR,
                                                 &isQueueSurfaceSupported_array[j]);
        }
        
        for(uint32_t j = 0; j < qCount; j++)
        {
            if(vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if(isQueueSurfaceSupported_array[j] == VK_TRUE)
                {
                    gCtx_Scene0.vkPhysicalDevice_selected = gCtx_Scene0.vkPhysicalDevice_array[i];
                    gCtx_Scene0.graphicsQueueFamilyIndex_selected = j;
                    bFound = VK_TRUE;
                    break;
                }
            }
        }
        
        if(isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
            fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() --> isQueueSurfaceSupported_array succedded to free\n");
        }
        
        if(vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() --> vkQueueFamilyProperties_array succedded to free\n");
        }
        
        if(bFound == VK_TRUE)
        {
            break;
        }
    }
    
    if(bFound == VK_TRUE)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() -->is succedded to select the required device with graphics enabled\n");
    }
    else
    {
        if(gCtx_Scene0.vkPhysicalDevice_array)
        {
            free(gCtx_Scene0.vkPhysicalDevice_array);
            gCtx_Scene0.vkPhysicalDevice_array = NULL;
            fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() --> gCtx_Scene0.vkPhysicalDevice_array succedded to free\n");
        }
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() -->is failed to select the required device with graphics enabled\n");
    }
    
    
    memset((void*)&gCtx_Scene0.vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
    vkGetPhysicalDeviceMemoryProperties(gCtx_Scene0.vkPhysicalDevice_selected, 
                                        &gCtx_Scene0.vkPhysicalDeviceMemoryProperties);
                                        
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    
    vkGetPhysicalDeviceFeatures(gCtx_Scene0.vkPhysicalDevice_selected, 
                                &vkPhysicalDeviceFeatures);
                                
    if(vkPhysicalDeviceFeatures.tessellationShader)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() -->selected device supports tessellationShader\n");
    }
    else
    {
          fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() -->selected device not supports tessellationShader\n");
    }
    
    if(vkPhysicalDeviceFeatures.geometryShader)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() -->selected device supports geometryShader\n");
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevice() -->selected device not supports geometryShader\n");
    }
    
    return vkResult;
}


VkResult printVkInfo(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;
    
    //code
    fprintf(gCtx_Scene0.gpFile, "*******************VULKAN INFORMATION*********************\n");
    for(uint32_t i = 0; i < gCtx_Scene0.physicalDeviceCount; i++)    
    {
        fprintf(gCtx_Scene0.gpFile, "Infomration of Device = %d\n", i);
        
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        
        vkGetPhysicalDeviceProperties(gCtx_Scene0.vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);
        
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);;
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);;
        //API Version
        fprintf(gCtx_Scene0.gpFile, "apiVersion = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);
        
        //Device Name
        fprintf(gCtx_Scene0.gpFile, "DeviceName = %s\n", vkPhysicalDeviceProperties.deviceName);
        
        //DeviceType
        switch(vkPhysicalDeviceProperties.deviceType)
        {
            case(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
                fprintf(gCtx_Scene0.gpFile, "DeviceType = Integrated GPU(iGPU)\n");
                break;
            
            case(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
                fprintf(gCtx_Scene0.gpFile, "DeviceType = Discrete GPU(dGPU)\n");
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU):
                fprintf(gCtx_Scene0.gpFile, "DeviceType = Virtual GPU(vGPU)\n");
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_CPU):
                fprintf(gCtx_Scene0.gpFile, "DeviceType = CPU\n");
                break;    
                
            case(VK_PHYSICAL_DEVICE_TYPE_OTHER):
                fprintf(gCtx_Scene0.gpFile, "DeviceType = Other\n");
                break; 
                
            default: 
                fprintf(gCtx_Scene0.gpFile, "DeviceType = UNKNOWN\n");                
        }
        
        //Vendor Id
        fprintf(gCtx_Scene0.gpFile, "VendorId = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);
        
        //DeviceId
        fprintf(gCtx_Scene0.gpFile, "DeviceId = 0x%04x\n\n", vkPhysicalDeviceProperties.deviceID);
   }
   
   fprintf(gCtx_Scene0.gpFile, "****************END OF VULKAN INFORMATION********************\n");
   
    //Freephysical device array
    if(gCtx_Scene0.vkPhysicalDevice_array)
    {
        free(gCtx_Scene0.vkPhysicalDevice_array);
        gCtx_Scene0.vkPhysicalDevice_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "printVkInfo() --> gCtx_Scene0.vkPhysicalDevice_array succedded to free\n");
    }
    
    return vkResult;
}


VkResult fillDeviceExtensionNames(void)
{
    //variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step1: Find how many Device Extension are supported by the vulkan driver of this version and keep the count in local variable
    uint32_t deviceExtensionCount = 0;

    memset((void*)gCtx_Scene0.enabledDeviceExtensionNames_array, 0,
           sizeof(gCtx_Scene0.enabledDeviceExtensionNames_array));
    gCtx_Scene0.enabledDeviceExtensionCount = 0;

    vkResult = vkEnumerateDeviceExtensionProperties(gCtx_Scene0.vkPhysicalDevice_selected,
                                                    NULL,  //Layer name: All layers
                                                    &deviceExtensionCount,
                                                    NULL); // Device Extensions Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is succedded\n");
        fprintf(gCtx_Scene0.gpFile, "deviceExtensionCount is %u\n", deviceExtensionCount);
    }


    //Step2: Allocate and fill VkExtensionProperties corresponding to above count
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * deviceExtensionCount);
    //Should be error checking for malloc: assert() can also be used

    vkResult = vkEnumerateDeviceExtensionProperties(gCtx_Scene0.vkPhysicalDevice_selected,
                                                    NULL, //Which layer's extenion is needed: Mention extension name: For all driver's extension use NULL
                                                    &deviceExtensionCount,
                                                    vkExtensionProperties_array); // Instance Extensions Properties array: As we have count, so it is value
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is succedded\n");
    }



    //Step3: Fill and Display a local string array of extension names obtained from vkExtensionProperties
    char** deviceExtensionNames_array = NULL;

    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gCtx_Scene0.gpFile, "fillDeviceExtensionNames() --> Vulkan Device Extension names = %s\n", deviceExtensionNames_array[i]);
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
            gCtx_Scene0.enabledDeviceExtensionNames_array[gCtx_Scene0.enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
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
        fprintf(gCtx_Scene0.gpFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME found\n");
    }


    //Step8: Print only Enabled Extension Names
    for(uint32_t i = 0; i < gCtx_Scene0.enabledDeviceExtensionCount; i++)
    {
         fprintf(gCtx_Scene0.gpFile, "fillDeviceExtensionNames() --> Enabled vulkan Device extension Names = %s\n", gCtx_Scene0.enabledDeviceExtensionNames_array[i]);
    }

    return vkResult;
}



VkResult createVulkanDevice(void)
{
//variable declaration
VkResult vkResult = VK_SUCCESS;
    
//Fill Device Extensions
vkResult = gFunctionTable_Scene0.fillDeviceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanDevice() --> fillDeviceExtensionNames() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanDevice() --> fillDeviceExtensionNames() is succedded\n");
    }
    
    /////Newly added code//////
    
    float QueuePriorities[] = {1.0};
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = gCtx_Scene0.graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = QueuePriorities;
    
    //Initialize VkDeviceCreateinfo structure
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));

    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = gCtx_Scene0.enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = gCtx_Scene0.enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.enabledLayerCount = 0;  // Deprecated
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;  // Deprecated
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;
        
    vkResult = vkCreateDevice(gCtx_Scene0.vkPhysicalDevice_selected,
                              &vkDeviceCreateInfo,
                              NULL,
                              &gCtx_Scene0.vkDevice);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanDevice() --> vkCreateDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVulkanDevice() --> vkCreateDevice() is succedded\n");
    }                     
    
    return vkResult;
}


void getDeviceQueue(void)
{
    //code
    vkGetDeviceQueue(gCtx_Scene0.vkDevice, 
                     gCtx_Scene0.graphicsQueueFamilyIndex_selected,
                     0, //0th Queue index in that family queue
                     &gCtx_Scene0.vkQueue);
    if(gCtx_Scene0.vkQueue == VK_NULL_HANDLE) //rarest possibility
    {
        fprintf(gCtx_Scene0.gpFile, "getDeviceQueue() --> vkGetDeviceQueue() returned NULL for gCtx_Scene0.vkQueue\n");
        return;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getDeviceQueue() --> vkGetDeviceQueue() is succedded\n");
    }
}


VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    //variable declarations
    VkResult vkResult = VK_SUCCESS;   
    uint32_t formatCount = 0;
    
    //code
    //get the count of supported SurfaceColorFormats
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gCtx_Scene0.vkPhysicalDevice_selected,
                                                    gCtx_Scene0.vkSurfaceKHR,
                                                    &formatCount,
                                                    NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(formatCount == 0)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed as formatCount is zero:: %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is succedded\n");
    }
    
    fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> total formatCount are:: %d\n", formatCount);
    
    VkSurfaceFormatKHR* vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*) malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    //Malloc error checking
    
    //Fillig the array
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(gCtx_Scene0.vkPhysicalDevice_selected,
                                                    gCtx_Scene0.vkSurfaceKHR,
                                                    &formatCount,
                                                    vkSurfaceFormatKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is succedded\n");
    }
    
    if(formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED) //bydefault it is not there
    {
        gCtx_Scene0.vkFormat_color = VK_FORMAT_R8G8B8A8_UNORM;
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> gCtx_Scene0.vkFormat_color is VK_FORMAT_B8G8R8A8_UNORM\n");
    }
    else
    {
        gCtx_Scene0.vkFormat_color = vkSurfaceFormatKHR_array[0].format;
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> gCtx_Scene0.vkFormat_color is %d\n", gCtx_Scene0.vkFormat_color);
    }
    
    //Decide the ColorSpace
    gCtx_Scene0.vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;
    
    if(vkSurfaceFormatKHR_array)
    {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkSurfaceFormatKHR_array is freed\n");
    }
    
    return vkResult;
}


VkResult getPhysicalDevicePresentMode(void)
{
    //variable declarations
    VkResult vkResult = VK_SUCCESS;   
    
    uint32_t presentModeCount = 0;
    
    //code
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gCtx_Scene0.vkPhysicalDevice_selected,
                                                         gCtx_Scene0.vkSurfaceKHR,
                                                         &presentModeCount,
                                                         NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(presentModeCount == 0)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed as formatCount is zero:: %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
    }   

    fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> total presentModeCount are:: %d\n", presentModeCount);

    VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*) malloc(presentModeCount * sizeof(VkPresentModeKHR));
    //Malloc error checking
    
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(gCtx_Scene0.vkPhysicalDevice_selected,
                                                         gCtx_Scene0.vkSurfaceKHR,
                                                         &presentModeCount,
                                                         vkPresentModeKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
    }
    
    //Decide Presentation mode
    for(uint32_t i = 0; i < presentModeCount; i++)
    {
        if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            gCtx_Scene0.vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> gCtx_Scene0.vkPresentModeKHR is VK_PRESENT_MODE_MAILBOX_KHR\n");
            break;
        }
    }
    
    if(gCtx_Scene0.vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        gCtx_Scene0.vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> gCtx_Scene0.vkPresentModeKHR is VK_PRESENT_MODE_FIFO_KHR\n");
    }
  
    
    if(vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        fprintf(gCtx_Scene0.gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR_array is freed\n");
    }
    
    return vkResult;
  
}



VkResult createSwapchain(VkBool32 vsync)  // vertical sync
{
//variables
VkResult vkResult = VK_SUCCESS;
        
    //code
//Color Format and ColorSpace
vkResult = gFunctionTable_Scene0.getPhysicalDeviceSurfaceFormatAndColorSpace();
    /*Main Points
        vkGetPhysicalDeviceSurfaceFormatsKHR()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> getPhysicalDeviceSurfaceFormatAndColorSpace() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> getPhysicalDeviceSurfaceFormatAndColorSpace() is succedded\n");
    }
   
   
    //Step 2:
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
    
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gCtx_Scene0.vkPhysicalDevice_selected,
                                                         gCtx_Scene0.vkSurfaceKHR,
                                                         &vkSurfaceCapabilitiesKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() is succedded\n");
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
    memset((void*)&gCtx_Scene0.vkExtent2D_swapchain, 0, sizeof(VkExtent2D));
    if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        gCtx_Scene0.vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        gCtx_Scene0.vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
        
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> Swapchain image width X height = %d X %d \n", gCtx_Scene0.vkExtent2D_swapchain.width, gCtx_Scene0.vkExtent2D_swapchain.height);
    }
    else
    {
        // if surface is already defined then swapchain image size must match with it
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0 , sizeof(VkExtent2D));
        vkExtent2D.width = (uint32_t)gCtx_Scene0.winWidth;
        vkExtent2D.height = (uint32_t)gCtx_Scene0.winHeight;
        
        gCtx_Scene0.vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        gCtx_Scene0.vkExtent2D_swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
        
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> Swapchain image width X height = %d X %d \n", gCtx_Scene0.vkExtent2D_swapchain.width, gCtx_Scene0.vkExtent2D_swapchain.height);
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
vkResult = gFunctionTable_Scene0.getPhysicalDevicePresentMode();
    /*Main Points
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> getPhysicalDevicePresentMode() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> getPhysicalDevicePresentMode() is succedded\n");
    }
    
    //Step 8: Initialie vkCreateSwapchinCreateInfoStructure
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset((void*)&vkSwapchainCreateInfoKHR, 0 , sizeof(VkSwapchainCreateInfoKHR));
    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = gCtx_Scene0.vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumbeOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = gCtx_Scene0.vkFormat_color;
    vkSwapchainCreateInfoKHR.imageColorSpace = gCtx_Scene0.vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = gCtx_Scene0.vkExtent2D_swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = gCtx_Scene0.vkExtent2D_swapchain.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = gCtx_Scene0.vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;
    
    //Step9:
    vkResult = vkCreateSwapchainKHR(gCtx_Scene0.vkDevice,
                                    &vkSwapchainCreateInfoKHR,
                                    NULL,
                                    &gCtx_Scene0.vkSwapchainKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> vkCreateSwapchainKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createSwapchain() --> vkCreateSwapchainKHR() is succedded\n");
    }
    
    return vkResult;
}

VkBool32 HasStencilComponent(VkFormat fmt) {
    return (fmt == VK_FORMAT_D32_SFLOAT_S8_UINT) ||
           (fmt == VK_FORMAT_D24_UNORM_S8_UINT)  ||
           (fmt == VK_FORMAT_D16_UNORM_S8_UINT);
}


VkResult createImagesAndImageViews(void)
{
//variables
    VkResult vkResult = VK_SUCCESS;
    
    //step1: Get desired SwapchainImage count
    vkResult = vkGetSwapchainImagesKHR(gCtx_Scene0.vkDevice, 
                                       gCtx_Scene0.vkSwapchainKHR,
                                       &gCtx_Scene0.swapchainImageCount,
                                       NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> 1st call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(0 == gCtx_Scene0.swapchainImageCount)
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> 1st call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> gives swapchainImagecount = %d\n", gCtx_Scene0.swapchainImageCount);
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkGetSwapchainImagesKHR() is succedded\n");
    }
    
    
    //step2: Allocate the swapchain Image array
    gCtx_Scene0.swapchainImage_array = (VkImage*)malloc(sizeof(VkImage) * gCtx_Scene0.swapchainImageCount);
    //malloc check to be done

    //step3: fill this array by swapchain imagesize
    vkResult = vkGetSwapchainImagesKHR(gCtx_Scene0.vkDevice, 
                                       gCtx_Scene0.vkSwapchainKHR,
                                       &gCtx_Scene0.swapchainImageCount,
                                       gCtx_Scene0.swapchainImage_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> 2nd call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> 2nd call to vkGetSwapchainImagesKHR() is succedded\n");
    }
    
    //step4: allocate array of swapchainImageViews   
    gCtx_Scene0.swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * gCtx_Scene0.swapchainImageCount);
    //malloc check to be done
    
    //step5: Initialize vkImageViewCreateInfo structure
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = gCtx_Scene0.vkFormat_color;
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
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        vkImageViewCreateInfo.image = gCtx_Scene0.swapchainImage_array[i];
        
        vkResult = vkCreateImageView(gCtx_Scene0.vkDevice,
                                 &vkImageViewCreateInfo,
                                 NULL,
                                 &gCtx_Scene0.swapchainImageView_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkCreateImageViews() is failed for iteration %d and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkCreateImageViews() is succedded for iteration for %d\n", i);
        }
    }
    
    //for depth image
    vkResult = gFunctionTable_Scene0.GetSupportedDepthFormat();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() is succedded\n");
    }
    
    //for depth image Initialize VkImageCreateInfo
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = gCtx_Scene0.vkFormat_depth;
    vkImageCreateInfo.extent.width = gCtx_Scene0.winWidth;
    vkImageCreateInfo.extent.height = gCtx_Scene0.winHeight;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vkResult = vkCreateImage(gCtx_Scene0.vkDevice, &vkImageCreateInfo, NULL, &gCtx_Scene0.vkImage_depth);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkCreateImage() is failed error code is %d\n",vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkCreateImage() is succedded\n");
    }
    
    //Memory requirement for depth image
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetImageMemoryRequirements(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImage_depth, &vkMemoryRequirements);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gCtx_Scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gCtx_Scene0.vkDeviceMemory_depth);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindImageMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImage_depth, gCtx_Scene0.vkDeviceMemory_depth, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkBindBufferMemory() is succedded\n");
    }
    
    
    //create image view for above depth image
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = gCtx_Scene0.vkFormat_depth;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
   if (gFunctionTable_Scene0.HasStencilComponent(gCtx_Scene0.vkFormat_depth)) {
		vkImageViewCreateInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = gCtx_Scene0.vkImage_depth;
    
    vkResult = vkCreateImageView(gCtx_Scene0.vkDevice,
                                 &vkImageViewCreateInfo,
                                 NULL,
                                 &gCtx_Scene0.vkImageView_depth);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkCreateImageView() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createImagesAndImageViews() --> vkCreateImageView() is succedded\n");
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

        vkGetPhysicalDeviceFormatProperties(gCtx_Scene0.vkPhysicalDevice_selected, vkFormat_depth_array[i], &vkFormatProperties);

        if(vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
           gCtx_Scene0.vkFormat_depth = vkFormat_depth_array[i];
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
   vkCommandPoolCreateInfo.queueFamilyIndex = gCtx_Scene0.graphicsQueueFamilyIndex_selected;

   vkResult = vkCreateCommandPool(gCtx_Scene0.vkDevice,
                                  &vkCommandPoolCreateInfo,
                                  NULL,
                                  &gCtx_Scene0.vkCommandPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createCommandPool() --> vkCreateCommandPool() is failed and error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createCommandPool() --> vkCreateCommandPool() is succedded \n");
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
   vkCommandBufferAllocateInfo.commandPool = gCtx_Scene0.vkCommandPool;
   vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   vkCommandBufferAllocateInfo.commandBufferCount = 1;
   
   gCtx_Scene0.vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * gCtx_Scene0.swapchainImageCount);
   //malloc check to be done
   
   for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(gCtx_Scene0.vkDevice, &vkCommandBufferAllocateInfo, &gCtx_Scene0.vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is failed for %d iteration and error code is %d\n",i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is succedded for iteration %d\n", i);
        }
    }
   
   return vkResult;   
}

VkResult createVertexBuffer(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS; 
    
    // CUBE
    // position
    float cubePosition[] =
    {
        // front
        // triangle one
         1.0f,  1.0f,  1.0f, // top-right of front
        -1.0f,  1.0f,  1.0f, // top-left of front
         1.0f, -1.0f,  1.0f, // bottom-right of front
        
        // triangle two
         1.0f, -1.0f,  1.0f, // bottom-right of front
        -1.0f,  1.0f,  1.0f, // top-left of front
        -1.0f, -1.0f,  1.0f, // bottom-left of front

        // right
        // triangle one
         1.0f,  1.0f, -1.0f, // top-right of right
         1.0f,  1.0f,  1.0f, // top-left of right
         1.0f, -1.0f, -1.0f, // bottom-right of right
         
        // triangle two
         1.0f, -1.0f, -1.0f, // bottom-right of right
         1.0f,  1.0f,  1.0f, // top-left of right
         1.0f, -1.0f,  1.0f, // bottom-left of right

        // back
        // triangle one
         1.0f,  1.0f, -1.0f, // top-right of back
        -1.0f,  1.0f, -1.0f, // top-left of back
         1.0f, -1.0f, -1.0f, // bottom-right of back
        
        // triangle two
         1.0f, -1.0f, -1.0f, // bottom-right of back
        -1.0f,  1.0f, -1.0f, // top-left of back
        -1.0f, -1.0f, -1.0f, // bottom-left of back

        // left
        // triangle one
        -1.0f,  1.0f,  1.0f, // top-right of left
        -1.0f,  1.0f, -1.0f, // top-left of left
        -1.0f, -1.0f,  1.0f, // bottom-right of left
        
        // triangle two
        -1.0f, -1.0f,  1.0f, // bottom-right of left
        -1.0f,  1.0f, -1.0f, // top-left of left
        -1.0f, -1.0f, -1.0f, // bottom-left of left

        // top
        // triangle one
         1.0f,  1.0f, -1.0f, // top-right of top
        -1.0f,  1.0f, -1.0f, // top-left of top
         1.0f,  1.0f,  1.0f, // bottom-right of top

        // triangle two
         1.0f,  1.0f,  1.0f, // bottom-right of top
        -1.0f,  1.0f, -1.0f, // top-left of top
        -1.0f,  1.0f,  1.0f, // bottom-left of top

        // bottom
        // triangle one
         1.0f, -1.0f,  1.0f, // top-right of bottom
        -1.0f, -1.0f,  1.0f, // top-left of bottom
         1.0f, -1.0f, -1.0f, // bottom-right of bottom
        
        // triangle two
         1.0f, -1.0f, -1.0f, // bottom-right of bottom
        -1.0f, -1.0f,  1.0f, // top-left of bottom
        -1.0f, -1.0f, -1.0f, // bottom-left of bottom
    };

    float cubeTexcoords[] =
	{
		// front
		// triangle one
		1.0f, 1.0f, // top-right of front
		0.0f, 1.0f, // top-left of front
		1.0f, 0.0f, // bottom-right of front

		// triangle two
		1.0f, 0.0f, // bottom-right of front
		0.0f, 1.0f, // top-left of front
		0.0f, 0.0f, // bottom-left of front

		// right
		// triangle one
		1.0f, 1.0f, // top-right of right
		0.0f, 1.0f, // top-left of right
		1.0f, 0.0f, // bottom-right of right
	
		// triangle two
		1.0f, 0.0f, // bottom-right of right
		0.0f, 1.0f, // top-left of right
		0.0f, 0.0f, // bottom-left of right

		// back
		// triangle one
		1.0f, 1.0f, // top-right of back
		0.0f, 1.0f, // top-left of back
		1.0f, 0.0f, // bottom-right of back

		// triangle two
		1.0f, 0.0f, // bottom-right of back
		0.0f, 1.0f, // top-left of back
		0.0f, 0.0f, // bottom-left of back

		// left
		// triangle one
		1.0f, 1.0f, // top-right of left
		0.0f, 1.0f, // top-left of left
		1.0f, 0.0f, // bottom-right of left
	
		// triangle two
		1.0f, 0.0f, // bottom-right of left
		0.0f, 1.0f, // top-left of left
		0.0f, 0.0f, // bottom-left of left

		// top
		// triangle one
		1.0f, 1.0f, // top-right of top
		0.0f, 1.0f, // top-left of top
		1.0f, 0.0f, // bottom-right of top

		// triangle two
		1.0f, 0.0f, // bottom-right of top
		0.0f, 1.0f, // top-left of top
		0.0f, 0.0f, // bottom-left of top

		// bottom
		// triangle one
		1.0f, 1.0f, // top-right of bottom
		0.0f, 1.0f, // top-left of bottom
		1.0f, 0.0f, // bottom-right of bottom

		// triangle two
		1.0f, 0.0f, // bottom-right of bottom
		0.0f, 1.0f, // top-left of bottom
		0.0f, 0.0f, // bottom-left of bottom
	};
    
    //VERTEX POSITION BUFFER 
    //#4 memset the global strucure variable
    memset((void*)&gCtx_Scene0.vertexData_position, 0, sizeof(GlobalContext_Scene0::VertexData));
    
    //#5 VkBufferCreateInfo structure filling
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    //valid flags are used in scatterred/sparse buffer
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(cubePosition);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    ///#6
    vkResult = vkCreateBuffer(gCtx_Scene0.vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &gCtx_Scene0.vertexData_position.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_position.vkBuffer, &vkMemoryRequirements);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gCtx_Scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gCtx_Scene0.vertexData_position.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_position.vkBuffer, gCtx_Scene0.vertexData_position.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded\n");
    }
    
    //#11
    void* data = NULL;
    vkResult = vkMapMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkMapMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkMapMemory() is succedded\n");
    }
   
    //#12
    memcpy(data, cubePosition, sizeof(cubePosition));
    
    vkUnmapMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_position.vkDeviceMemory);
   
   
   
   //VERTEX COLOR BUFFER 
    //#4 memset the global strucure variable
    memset((void*)&gCtx_Scene0.vertexData_texcoord, 0, sizeof(GlobalContext_Scene0::VertexData));
    
    //#5 VkBufferCreateInfo structure filling
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    //valid flags are used in scatterred/sparse buffer
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(cubeTexcoords);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    ///#6
    vkResult = vkCreateBuffer(gCtx_Scene0.vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &gCtx_Scene0.vertexData_texcoord.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord buffer \n");
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_texcoord.vkBuffer, &vkMemoryRequirements);
    
    //8" Allocate
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gCtx_Scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gCtx_Scene0.vertexData_texcoord.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord buffer\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_texcoord.vkBuffer, gCtx_Scene0.vertexData_texcoord.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord buffer\n");
    }
    
    //#11
    data = NULL;
    vkResult = vkMapMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_texcoord.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord buffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord buffer\n");
    }
   
    //#12
    memcpy(data, cubeTexcoords, sizeof(cubeTexcoords));
    
    vkUnmapMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vertexData_texcoord.vkDeviceMemory);
   
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
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> fOpen() failed to open Stone.png texture file\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    
    uint8_t* imageData = NULL;
    int texture_width, texture_height, texture_channels;

    imageData = stbi_load_from_file(fp, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    if(imageData == NULL || texture_width <= 0 || texture_height <= 0 || texture_channels <= 0)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> stbi_load_from_file() failed to read Stone.png texture file\n");
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
    
    vkResult = vkCreateBuffer(gCtx_Scene0.vkDevice, 
                              &vkBufferCreateInfo_stagingBuffer,
                              NULL,
                              &vkBuffer_stagingBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));
    
    vkGetBufferMemoryRequirements(gCtx_Scene0.vkDevice, vkBuffer_stagingBuffer, &vkMemoryRequirements_stagingBuffer);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
    memset((void*)&vkMemoryAllocateInfo_stagingBuffer , 0, sizeof(VkMemoryAllocateInfo));
    
    vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_stagingBuffer.pNext = NULL; 
    vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;
    
    for(uint32_t i = 0; i < gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
        {
            if(gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) //VK_MEMORY_PROPERTY_HOST_COHERENT_BIT--> No need to manage vulkan cache mechanism for flushing and mapping as we order vulkan to maintain coherency
            {
                vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gCtx_Scene0.vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vkDeviceMemory_stagingBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(gCtx_Scene0.vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBindBufferMemory() is succedded\n");
    }
    
    void* data = NULL;
    vkResult = vkMapMemory(gCtx_Scene0.vkDevice, vkDeviceMemory_stagingBuffer, 0, image_size, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkMapMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkMapMemory() is succedded\n");
    }
    
    //#12
    memcpy(data, imageData, image_size);
    
    vkUnmapMemory(gCtx_Scene0.vkDevice, vkDeviceMemory_stagingBuffer);
    
    //As copying of image data into the staging buffer is completed, we can Free the actual image data given by stb 
    stbi_image_free(imageData);
    imageData = NULL;
    fprintf(gCtx_Scene0.gpFile, "createTexture() --> stbi_image_free() Freeing of image data is succedded\n");
    
    
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

    vkResult = vkCreateImage(gCtx_Scene0.vkDevice, &vkImageCreateInfo, NULL, &gCtx_Scene0.vkImage_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkCreateImage() is failed error code is %d\n",vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkCreateImage() is succedded\n");
        fflush(gCtx_Scene0.gpFile);
    }
    
    //Memory requirement for texture image
    VkMemoryRequirements vkMemoryRequirements_image;
    memset((void*)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImage_texture, &vkMemoryRequirements_image);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
    memset((void*)&vkMemoryAllocateInfo_image , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo_image.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_image.pNext = NULL; 
    vkMemoryAllocateInfo_image.allocationSize = vkMemoryRequirements_image.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo_image.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements_image.memoryTypeBits & 1) == 1)
        {
            if(gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo_image.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_image.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gCtx_Scene0.vkDevice, &vkMemoryAllocateInfo_image, NULL, &gCtx_Scene0.vkDeviceMemory_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateMemory() is succedded\n");
        fflush(gCtx_Scene0.gpFile);
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindImageMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.vkImage_texture, gCtx_Scene0.vkDeviceMemory_texture, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBindImageMemory() is failed and error code is %d\n", vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBindImageMemory() is succedded\n");
        fflush(gCtx_Scene0.gpFile);
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
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = gCtx_Scene0.vkCommandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    VkCommandBuffer vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(gCtx_Scene0.vkDevice, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed and error code is %d\n", vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
        fflush(gCtx_Scene0.gpFile);
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
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBeginCommandBuffer() is failed and error code is %d\n", vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded \n");
        fflush(gCtx_Scene0.gpFile);
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
    vkImageMemoryBarrier.image = gCtx_Scene0.vkImage_texture;
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
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> Unsupported texture layout transition()\n");
        fflush(gCtx_Scene0.gpFile);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    vkCmdPipelineBarrier(vkCommandBuffer_transition_image_layout, vkPipelineStageFlags_source, vkPipelineStageFlags_destination, 0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);
    
    //#4.4: End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkEndCommandBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");
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
    vkResult = vkQueueSubmit(gCtx_Scene0.vkQueue,
                             1,
                             &vkSubmitInfo_transition_image_layout,
                             VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueSubmit() succeded\n", vkResult);
    }

    //#4.6: Waiting
    vkResult = vkQueueWaitIdle(gCtx_Scene0.vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueWaitIdle() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueWaitIdle() succeded\n", vkResult);
    }
    
    //#4.7: Freeing
    vkFreeCommandBuffers(gCtx_Scene0.vkDevice, gCtx_Scene0.vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    
    /*
    Step #5 
    Now actually copy the image data from staging buffer to the empty vkImage.
    */
    
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_buffer_to_image_copy; 
    memset((void*)&vkCommandBufferAllocateInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferAllocateInfo));
    
    vkCommandBufferAllocateInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandPool = gCtx_Scene0.vkCommandPool;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    VkCommandBuffer vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(gCtx_Scene0.vkDevice, &vkCommandBufferAllocateInfo_buffer_to_image_copy, &vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed for buffer_to_image_copy and error code is %d \n", vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded for buffer_to_image_copy\n");
        fflush(gCtx_Scene0.gpFile);
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
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBeginCommandBuffer() is failed for buffer_to_image_copy and error code is %d\n", vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded for buffer_to_image_copy \n");
        fflush(gCtx_Scene0.gpFile);
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
                           gCtx_Scene0.vkImage_texture, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                           1, 
                           &vkBufferImageCopy);
    
    //#5.4: End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkEndCommandBuffer() is failed for buffer_to_image_copy and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkEndCommandBuffer() is succedded for buffer_to_image_copy\n");
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
    vkResult = vkQueueSubmit(gCtx_Scene0.vkQueue,
                             1,
                             &vkSubmitInfo_buffer_to_image_copy,
                             VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueSubmit() is failed for buffer_to_image_copy errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueSubmit() is succedded for buffer_to_image_copy");
    }

    //#5.6: Waiting
    vkResult = vkQueueWaitIdle(gCtx_Scene0.vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueWaitIdle() is failed for buffer_to_image_copy errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueWaitIdle() succeded for buffer_to_image_copy\n");
    }
    
    //#5.7: Freeing
    vkFreeCommandBuffers(gCtx_Scene0.vkDevice, gCtx_Scene0.vkCommandPool, 1, &vkCommandBuffer_buffer_to_image_copy);
    vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    
    
    /*
    Step 6. 
    Now again do image layout transition similar to the step 4, for the corrext reading/writing of object data by shaders.
    */
    
    //#6.1
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));
    
    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = gCtx_Scene0.vkCommandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(gCtx_Scene0.vkDevice, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed and error code is %d\n", vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
        fflush(gCtx_Scene0.gpFile);
    }

    //#6.2
    memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));
    
    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;  //1. we will use only primary command buffers, 2. we are not going to use this command buffer simultaneoulsy between multipple threads
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBeginCommandBuffer() is failed and error code is %d\n", vkResult);
        fflush(gCtx_Scene0.gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded \n");
        fflush(gCtx_Scene0.gpFile);
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
    vkImageMemoryBarrier.image = gCtx_Scene0.vkImage_texture;
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
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> Unsupported texture layout transition for 2nd time in step 6\n");
        fflush(gCtx_Scene0.gpFile);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    vkCmdPipelineBarrier(vkCommandBuffer_transition_image_layout, vkPipelineStageFlags_source, vkPipelineStageFlags_destination, 0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);
    
    //#6.4: End Command Buffer
    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkEndCommandBuffer() is failed for 2nd time in step 6 and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");
    }
    
    //#6.5: Submitting Queue
    memset((void*)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
    
    vkSubmitInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_transition_image_layout.pNext = NULL;
    vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
    vkSubmitInfo_transition_image_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;
    // As there is no need of synchrnization for waitDstStageMask and Semaphore is not needed
    
    //Now submit our work to the Queue
    vkResult = vkQueueSubmit(gCtx_Scene0.vkQueue,
                             1,
                             &vkSubmitInfo_transition_image_layout,
                             VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueSubmit() is failed  for 2nd time in step 6 errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueSubmit() succeded  for 2nd time in step 6 \n", vkResult);
    }

    //#6.6: Waiting
    vkResult = vkQueueWaitIdle(gCtx_Scene0.vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueWaitIdle() is failed  for 2nd time in step 6 errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkQueueWaitIdle() succeded for 2nd time in step 6 \n", vkResult);
    }
    
    //#6.7: Freeing
    vkFreeCommandBuffers(gCtx_Scene0.vkDevice, gCtx_Scene0.vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    
    /*
    Step #7. 
    Now staging buffer is not needed, hence release its memory and itself
    */

    if(vkBuffer_stagingBuffer)
    {
        vkDestroyBuffer(gCtx_Scene0.vkDevice, vkBuffer_stagingBuffer, NULL);
        vkBuffer_stagingBuffer = VK_NULL_HANDLE;
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkDestroyBuffer() is done for vkBuffer_stagingBuffer of setp 7\n");
    }
    
    if(vkDeviceMemory_stagingBuffer)
    {
       vkFreeMemory(gCtx_Scene0.vkDevice, vkDeviceMemory_stagingBuffer, NULL);
       vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
       fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkFreeMemory() is done for vkBuffer_stagingBuffer of setp 7\n");
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
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = gCtx_Scene0.vkImage_texture;
    
    vkResult = vkCreateImageView(gCtx_Scene0.vkDevice,
                                 &vkImageViewCreateInfo,
                                 NULL,
                                 &gCtx_Scene0.vkImageView_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkCreateImageView() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkCreateImageView() is succedded\n");
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
    
    vkResult = vkCreateSampler(gCtx_Scene0.vkDevice, &vkSamplerCreateInfo, NULL, &gCtx_Scene0.vkSampler_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkCreateSampler() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createTexture() --> vkCreateSampler() is succedded\n");
    }
     
    return vkResult; 
}



VkResult createUniformBuffer(void)
{
//variable
VkResult vkResult = VK_SUCCESS;
    
    //code
    
    //#4 memset the global strucure variable
    memset((void*)&gCtx_Scene0.uniformData, 0, sizeof(GlobalContext_Scene0::UniformData));
    
    //#5 VkBufferCreateInfo structure filling
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    //valid flags are used in scatterred/sparse buffer
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(GlobalContext_Scene0::MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    //#6
    vkResult = vkCreateBuffer(gCtx_Scene0.vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &gCtx_Scene0.uniformData.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(gCtx_Scene0.vkDevice, gCtx_Scene0.uniformData.vkBuffer, &vkMemoryRequirements);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(gCtx_Scene0.vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(gCtx_Scene0.vkDevice, &vkMemoryAllocateInfo, NULL, &gCtx_Scene0.uniformData.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.uniformData.vkBuffer, gCtx_Scene0.uniformData.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
    }
    
//call updateUniformBuffer()
vkResult = gFunctionTable_Scene0.updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> updateUniformBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> updateUniformBuffer() is succedded\n");
    }
    
    return vkResult;
}

VkResult updateUniformBuffer(void)
{
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //Note:glm follows column major matrix just like OpenGL, BUT unlike OpenGL its matrix array is 2D array 4x4 and not 1D array of 16 elements 
    GlobalContext_Scene0::MyUniformData myUniformData;
    memset((void*)&myUniformData, 0, sizeof(GlobalContext_Scene0::MyUniformData));
    //update matrices
    myUniformData.modelMatrix = glm::mat4(1.0f);  //Identity matrix
    
    glm::mat4 translationMatrix = glm::mat4(1.0f);  //Identity matrix
    translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
    
    glm::mat4 rotationMatrix_X = glm::mat4(1.0f);  //Identity matrix
    glm::mat4 rotationMatrix_Y = glm::mat4(1.0f);  //Identity matrix
    glm::mat4 rotationMatrix_Z = glm::mat4(1.0f);  //Identity matrix
    
    glm::mat4 rotationMatrix = glm::mat4(1.0f);  //Identity matrix
    rotationMatrix_X = glm::rotate(glm::mat4(1.0f), glm::radians(gCtx_Scene0.angle), glm::vec3(1.0f, 0.0f, 0.0f)); //X axis rotation
    rotationMatrix_Y = glm::rotate(glm::mat4(1.0f), glm::radians(gCtx_Scene0.angle), glm::vec3(0.0f, 1.0f, 0.0f)); //Y axis rotation
    rotationMatrix_Z = glm::rotate(glm::mat4(1.0f), glm::radians(gCtx_Scene0.angle), glm::vec3(0.0f, 0.0f, 1.0f)); //Z axis rotation
    
    rotationMatrix = rotationMatrix_X * rotationMatrix_Y * rotationMatrix_Z;
    
    glm::mat4 ScaleMatrix = glm::mat4(1.0f);  //Identity matrix
    ScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.05f, 1.25f, 1.25f));
    
    myUniformData.modelMatrix = translationMatrix * ScaleMatrix * rotationMatrix;
    
    myUniformData.viewMatrix = glm::mat4(1.0f);  //Identity matrix
    //myUniformData.projectionMatrix = glm::mat4(1.0f);  //Identity matrix
    glm::mat4 perspectiveProjectionMatrix = glm::mat4(1.0f); //Identity matrix
    perspectiveProjectionMatrix = glm::mat4(1.0f); //Identity matrix
    
    perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)gCtx_Scene0.winWidth/(float)gCtx_Scene0.winHeight, 0.1f, 100.0f);
    
    perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * (-1.0f);
    
    myUniformData.projectionMatrix = perspectiveProjectionMatrix;
	
        myUniformData.fade = glm::vec4(gCtx_Scene0.gFade, 0.0f, 0.0f, 0.0f);
    
    //map uniform buffer
    void* data = NULL;
    vkResult = vkMapMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.uniformData.vkDeviceMemory, 0, sizeof(GlobalContext_Scene0::MyUniformData), 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createUniformBuffer() --> vkMapMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
   
    //#12 copy the data to maped buffer
    memcpy(data, &myUniformData, sizeof(GlobalContext_Scene0::MyUniformData));
    
    //Unmap memory
    vkUnmapMemory(gCtx_Scene0.vkDevice, gCtx_Scene0.uniformData.vkDeviceMemory);
   
    
    return vkResult;
}

VkResult createShaders(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //for vertex shader
    const char* szFileName = "shader_Scene0.vert.spv";
    FILE* fp = NULL;
    size_t size;
    
    //#6a
    fp = fopen(szFileName, "rb"); //open for reading in binary format
    if(fp == NULL)
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> fopen() failed to open shader_Scene0.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> fopen() succedded to open shader_Scene0.vert.spv\n");
    }
    
    //#6b
    fseek(fp, 0L, SEEK_END);
    
    //#6c
    size = ftell(fp);
    if(size == 0)
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> ftell() failed to provide size of shader_Scene0.vert.spv\n");
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
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> fread() failed to read shader_Scene0.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> fread() succedded to read shader_Scene0.vert.spv\n");
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
    vkResult = vkCreateShaderModule(gCtx_Scene0.vkDevice, &vkShaderModuleCreateInfo, NULL, &gCtx_Scene0.vkShaderModule_vertex_shader);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> vkCreateShaderModule() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> vkCreateShaderModule() is succedded\n");
    }
    
    //#9
    if(shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }
    
    fprintf(gCtx_Scene0.gpFile, "createShaders() --> vertex Shader module successfully created\n");
    
    
    //for fragment shader
    szFileName = "shader_Scene0.frag.spv";
    fp = NULL;
    size = 0;
    
    //#6a
    fp = fopen(szFileName, "rb"); //open for reading in binary format
    if(fp == NULL)
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> fopen() failed to open shader_Scene0.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> fopen() succedded to open shader_Scene0.frag.spv\n");
    }
    
    //#6b
    fseek(fp, 0L, SEEK_END);
    
    //#6c
    size = ftell(fp);
    if(size == 0)
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> ftell() failed to provide size of shader_Scene0.frag.spv\n");
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
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> fread() failed to read shader_Scene0.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> fread() succedded to read shader_Scene0.frag.spv\n");
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
    vkResult = vkCreateShaderModule(gCtx_Scene0.vkDevice, &vkShaderModuleCreateInfo, NULL, &gCtx_Scene0.vkShaderModule_fragment_shader);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> vkCreateShaderModule() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createShaders() --> vkCreateShaderModule() is succedded\n");
    }
    
    //#9
    if(shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }
    
    fprintf(gCtx_Scene0.gpFile, "createShaders() --> fragment Shader module successfully created\n");
     
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
    vkDescriptorSetLayoutBinding_array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;  //shader stage
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

    vkResult = vkCreateDescriptorSetLayout(gCtx_Scene0.vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &gCtx_Scene0.vkDescriptorSetLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayour() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayour() is succedded\n");
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
    vkPipelineLayoutCreateInfo.pSetLayouts = &gCtx_Scene0.vkDescriptorSetLayout;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    vkResult = vkCreatePipelineLayout(gCtx_Scene0.vkDevice, &vkPipelineLayoutCreateInfo, NULL, &gCtx_Scene0.vkPipelineLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createPipelineLayout() --> vkCreatePipelineLayout() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createPipelineLayout() --> vkCreatePipelineLayout() is succedded\n");
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

    vkResult = vkCreateDescriptorPool(gCtx_Scene0.vkDevice, &vkDescriptorPoolCreateInfo, NULL, &gCtx_Scene0.vkDescriptorPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createDescriptorPool() --> vkCreateDescriptorPool() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createDescriptorPool() --> vkCreateDescriptorPool() is succedded\n");
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
    vkDescriptorSetAllocateInfo.descriptorPool = gCtx_Scene0.vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;  //though we have 2 descriptors, 1 for MVP uniform and 1 for texture sampler, both are in 1 same descriptor set
    vkDescriptorSetAllocateInfo.pSetLayouts = &gCtx_Scene0.vkDescriptorSetLayout;

    vkResult = vkAllocateDescriptorSets(gCtx_Scene0.vkDevice, &vkDescriptorSetAllocateInfo, &gCtx_Scene0.vkDescriptorSet);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded\n");
    }
    
    //Describe whether we want buffer as uniform or image as uniform
    //For Buffer Uniform
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = gCtx_Scene0.uniformData.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(GlobalContext_Scene0::MyUniformData);
    
    
    //for Texture image & sampler
    VkDescriptorImageInfo vkDescriptorImageInfo;
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = gCtx_Scene0.vkImageView_texture;
    vkDescriptorImageInfo.sampler = gCtx_Scene0.vkSampler_texture;
    
   
    
    //now update the above descriptor set directly to shader
    //There are two ways to update -->driectly writing in shader or copying from one shader to another shader
    //we will be using writing to shader option, this require initilaization of following structure
    VkWriteDescriptorSet vkWriteDescriptorSet_array[2];
    //2 VkWriteDescriptorSet for above two steructure
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    //for MVP
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = gCtx_Scene0.vkDescriptorSet;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; //where to bind in shader, and our uniform is at binding 0 index in shader
    
    //for texture image & sampler
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = gCtx_Scene0.vkDescriptorSet;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; //where to bind in shader, and our uniform is at binding 1 index in frag shader
    
    
    vkUpdateDescriptorSets(gCtx_Scene0.vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gCtx_Scene0.gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
    
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
    vkAttachmentDescription_array[0].format = gCtx_Scene0.vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    //for depth
    vkAttachmentDescription_array[1].flags = 0; //For embedded devices
    vkAttachmentDescription_array[1].format = gCtx_Scene0.vkFormat_depth;
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
    vkResult = vkCreateRenderPass(gCtx_Scene0.vkDevice,
                                  &vkRenderPassCreateInfo,
                                  NULL,
                                  &gCtx_Scene0.vkRenderPass);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createRenderPass() --> vkCreateRenderPass() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createRenderPass() --> vkCreateRenderPass() is succedded\n");
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
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_TRUE;
	
	// Pre-multiplied alpha: final = src + dst*(1 - src.a)
        vkPipelineColorBlendAttachmentState_array[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        vkPipelineColorBlendAttachmentState_array[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        vkPipelineColorBlendAttachmentState_array[0].colorBlendOp        = VK_BLEND_OP_ADD;

        vkPipelineColorBlendAttachmentState_array[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        vkPipelineColorBlendAttachmentState_array[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        vkPipelineColorBlendAttachmentState_array[0].alphaBlendOp        = VK_BLEND_OP_ADD;
	
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
    
    memset((void*)&gCtx_Scene0.vkViewport, 0, sizeof(VkViewport));
    gCtx_Scene0.vkViewport.x = 0;
    gCtx_Scene0.vkViewport.y = 0;
    gCtx_Scene0.vkViewport.width = (float)gCtx_Scene0.vkExtent2D_swapchain.width;
    gCtx_Scene0.vkViewport.height = (float)gCtx_Scene0.vkExtent2D_swapchain.height;
    gCtx_Scene0.vkViewport.minDepth = 0.0f;
    gCtx_Scene0.vkViewport.maxDepth = 1.0f;
    
    vkPipelineViewportStateCreateInfo.pViewports = &gCtx_Scene0.vkViewport;
    vkPipelineViewportStateCreateInfo.scissorCount = 1;
    
    memset((void*)&gCtx_Scene0.vkRect2D_scissor, 0, sizeof(VkRect2D));
    gCtx_Scene0.vkRect2D_scissor.offset.x = 0;
    gCtx_Scene0.vkRect2D_scissor.offset.y = 0;
    gCtx_Scene0.vkRect2D_scissor.extent.width = (float)gCtx_Scene0.vkExtent2D_swapchain.width;
    gCtx_Scene0.vkRect2D_scissor.extent.height = (float)gCtx_Scene0.vkExtent2D_swapchain.height;
    
    vkPipelineViewportStateCreateInfo.pScissors = &gCtx_Scene0.vkRect2D_scissor;
    
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
    vkPipelineShaderStageCreateInfo_array[0].module = gCtx_Scene0.vkShaderModule_vertex_shader;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main";
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;
    
    //fragment Shader
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = gCtx_Scene0.vkShaderModule_fragment_shader;
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
    
    
    vkResult = vkCreatePipelineCache(gCtx_Scene0.vkDevice, &vkPipelineCacheCreateInfo, NULL, &gCtx_Scene0.vkPipelineCache);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createPipeline() --> vkCreatePipelineCache() is failed error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createPipeline() --> vkCreatePipelineCache() is succedded\n");
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
    vkGraphicsPipelineCreateInfo.layout = gCtx_Scene0.vkPipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass = gCtx_Scene0.vkRenderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0; //as we have only one renderpass
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;
    
    //Now create the pipeline
    vkResult = vkCreateGraphicsPipelines(gCtx_Scene0.vkDevice,
                                         gCtx_Scene0.vkPipelineCache,
                                         1,
                                         &vkGraphicsPipelineCreateInfo,
                                         NULL,
                                         &gCtx_Scene0.vkPipeline);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createPipeline() --> vkCreateGraphicsPipelines() is failed error code is %d\n", vkResult);
        vkDestroyPipelineCache(gCtx_Scene0.vkDevice, gCtx_Scene0.vkPipelineCache, NULL);
        gCtx_Scene0.vkPipelineCache = VK_NULL_HANDLE;
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createPipeline() --> vkCreateGraphicsPipelines() is succedded\n");
    }
    
    //we ar done with pipeline cache so destroy it
    vkDestroyPipelineCache(gCtx_Scene0.vkDevice, gCtx_Scene0.vkPipelineCache, NULL);
    gCtx_Scene0.vkPipelineCache = VK_NULL_HANDLE;
    
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
    vkFramebufferCreateInfo.renderPass = gCtx_Scene0.vkRenderPass;
    vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
    vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
    vkFramebufferCreateInfo.width = gCtx_Scene0.vkExtent2D_swapchain.width;
    vkFramebufferCreateInfo.height = gCtx_Scene0.vkExtent2D_swapchain.height;
    vkFramebufferCreateInfo.layers = 1;
    
    //step3:
    gCtx_Scene0.vkFrameBuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * gCtx_Scene0.swapchainImageCount);
    //check for malloc
    
    //step4:
    
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
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
        vkFramebufferCreateInfo.renderPass = gCtx_Scene0.vkRenderPass;
        vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
        vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
        vkFramebufferCreateInfo.width = gCtx_Scene0.vkExtent2D_swapchain.width;
        vkFramebufferCreateInfo.height = gCtx_Scene0.vkExtent2D_swapchain.height;
        vkFramebufferCreateInfo.layers = 1;
        
        /*********************************************************************************************************************/
        
        vkImageView_attachment_array[0] = gCtx_Scene0.swapchainImageView_array[i];
        vkImageView_attachment_array[1] = gCtx_Scene0.vkImageView_depth;
        
        vkResult = vkCreateFramebuffer(gCtx_Scene0.vkDevice, 
                                       &vkFramebufferCreateInfo,
                                       NULL,
                                       &gCtx_Scene0.vkFrameBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "createFrameBuffers() --> vkCreateFramebuffer() is failed for %d iteration and error code is %d\n",i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "createFrameBuffers() --> vkCreateFramebuffer() is succedded for iteration %d\n", i);
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
    vkResult = vkCreateSemaphore(gCtx_Scene0.vkDevice, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &gCtx_Scene0.vkSemaphore_backBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createSemaphores() --> vkCreateSemaphore() is failed for gCtx_Scene0.vkSemaphore_backBuffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createSemaphores() --> vkCreateSemaphore() is succedded for gCtx_Scene0.vkSemaphore_backBuffer\n");
    }
    
    
    //renderComplete Semaphore
    vkResult = vkCreateSemaphore(gCtx_Scene0.vkDevice, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &gCtx_Scene0.vkSemaphore_renderComplete);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "createSemaphores() --> vkCreateSemaphore() is failed for gCtx_Scene0.vkSemaphore_renderComplete and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gCtx_Scene0.gpFile, "createSemaphores() --> vkCreateSemaphore() is succedded for gCtx_Scene0.vkSemaphore_renderComplete\n");
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
    
    gCtx_Scene0.vkFence_array = (VkFence*)malloc(sizeof(VkFence) * gCtx_Scene0.swapchainImageCount);
    //malloc error checking to be done
    
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        vkResult = vkCreateFence(gCtx_Scene0.vkDevice, 
                                 &vkFenceCreateInfo,
                                 NULL,
                                 &gCtx_Scene0.vkFence_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "createFences() --> vkCreateFence() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "createFences() --> vkCreateFence() is succedded for %d iteration\n", i);
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
    for(uint32_t i = 0; i < gCtx_Scene0.swapchainImageCount; i++)
    {
        //ResetCommandBuffers
        vkResult = vkResetCommandBuffer(gCtx_Scene0.vkCommandBuffer_array[i], 0);  // second parameetr: this command buffer is created from command pool, so dont release the resouces created by this comand buffer
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "buildCommandBuffers() --> vkResetCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "buildCommandBuffers() --> vkResetCommandBuffer() is succedded for %d iteration\n", i);
        }
        
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        
        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0;  //1. we will use only primary command buffers, 2. we are not going to use this command buffer simultaneoulsy between multipple threads
        
        vkResult = vkBeginCommandBuffer(gCtx_Scene0.vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "buildCommandBuffers() --> vkBeginCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "buildCommandBuffers() --> vkBeginCommandBuffer() is succedded for %d iteration\n", i);
        }
        
        //SetClearValues
        VkClearValue vkClearValue_array[2];
        memset((void*)vkClearValue_array, 0, (sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array)));
        
        vkClearValue_array[0].color = gCtx_Scene0.vkClearColorValue;
        vkClearValue_array[1].depthStencil = gCtx_Scene0.vkClearDepthStencilValue;
        
        //Fill REnderpass 
        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = gCtx_Scene0.vkRenderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = gCtx_Scene0.vkExtent2D_swapchain.width;
        vkRenderPassBeginInfo.renderArea.extent.height = gCtx_Scene0.vkExtent2D_swapchain.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = gCtx_Scene0.vkFrameBuffer_array[i];
        
        //BeginRenderPass
        vkCmdBeginRenderPass(gCtx_Scene0.vkCommandBuffer_array[i],
                             &vkRenderPassBeginInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        //Bind with the pipeline
        vkCmdBindPipeline(gCtx_Scene0.vkCommandBuffer_array[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          gCtx_Scene0.vkPipeline);
        //Bind our descriptor set with the pipeline
        vkCmdBindDescriptorSets(gCtx_Scene0.vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                gCtx_Scene0.vkPipelineLayout,
                                0,
                                1,
                                &gCtx_Scene0.vkDescriptorSet,
                                0, 
                                NULL);
        
        //bind vertex position buffer
        VkDeviceSize vkDeviceSize_offset_position[1];
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(gCtx_Scene0.vkCommandBuffer_array[i], 
                               0, //binding point
                               1, //how many buffer
                               &gCtx_Scene0.vertexData_position.vkBuffer, //which buffer
                               vkDeviceSize_offset_position);
        
        //bind vertex color buffer
        VkDeviceSize vkDeviceSize_offset_texcoord[1];
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(gCtx_Scene0.vkCommandBuffer_array[i], 
                               1, //binding point
                               1, //how many buffer
                               &gCtx_Scene0.vertexData_texcoord.vkBuffer, //which buffer
                               vkDeviceSize_offset_texcoord);
                                     
        //Here we should call vulkan drawing functions
        vkCmdDraw(gCtx_Scene0.vkCommandBuffer_array[i],
                  36, //vertices
                  1, //instances
                  0, //which vertices to start
                  0);
        
        //EndRenderPass
        vkCmdEndRenderPass(gCtx_Scene0.vkCommandBuffer_array[i]);
        
        //End Command Buffer recording
        vkResult = vkEndCommandBuffer(gCtx_Scene0.vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gCtx_Scene0.gpFile, "buildCommandBuffers() --> vkEndCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gCtx_Scene0.gpFile, "buildCommandBuffers() --> vkEndCommandBuffer() is succedded for %d iteration\n", i);
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
    fprintf(gCtx_Scene0.gpFile, "SSA_Validation: debugReportCallback() --> %s (%d) = %s\n", pLayerPrefix, messageCode, pMessage);

    return(VK_FALSE);
}

void InitializeFunctionTable(void)
{
    memset(&gWin32FunctionTable_Scene0, 0, sizeof(gWin32FunctionTable_Scene0));
    memset(&gFunctionTable_Scene0, 0, sizeof(gFunctionTable_Scene0));

    gWin32FunctionTable_Scene0.WndProc = WndProc;
    gWin32FunctionTable_Scene0.Update = Update;
    gWin32FunctionTable_Scene0.Initialize = Initialize;
    gWin32FunctionTable_Scene0.Resize = Resize;
    gWin32FunctionTable_Scene0.Display = Display;
    gWin32FunctionTable_Scene0.Uninitialize = Uninitialize;
    gWin32FunctionTable_Scene0.ToggleFullscreen = ToggleFullscreen;
    gFunctionTable_Scene0.createVulkanInstance = createVulkanInstance;
    gFunctionTable_Scene0.fillInstanceExtensionNames = fillInstanceExtensionNames;
    gFunctionTable_Scene0.fillValidationLayerNames = fillValidationLayerNames;
    gFunctionTable_Scene0.createValidationCallbackFunction = createValidationCallbackFunction;
    gFunctionTable_Scene0.getSupportedSurface = getSupportedSurface;
    gFunctionTable_Scene0.getPhysicalDevice = getPhysicalDevice;
    gFunctionTable_Scene0.printVkInfo = printVkInfo;
    gFunctionTable_Scene0.fillDeviceExtensionNames = fillDeviceExtensionNames;
    gFunctionTable_Scene0.createVulkanDevice = createVulkanDevice;
    gFunctionTable_Scene0.getDeviceQueue = getDeviceQueue;
    gFunctionTable_Scene0.getPhysicalDeviceSurfaceFormatAndColorSpace = getPhysicalDeviceSurfaceFormatAndColorSpace;
    gFunctionTable_Scene0.getPhysicalDevicePresentMode = getPhysicalDevicePresentMode;
    gFunctionTable_Scene0.createSwapchain = createSwapchain;
    gFunctionTable_Scene0.HasStencilComponent = HasStencilComponent;
    gFunctionTable_Scene0.createImagesAndImageViews = createImagesAndImageViews;
    gFunctionTable_Scene0.GetSupportedDepthFormat = GetSupportedDepthFormat;
    gFunctionTable_Scene0.createCommandPool = createCommandPool;
    gFunctionTable_Scene0.createCommandBuffers = createCommandBuffers;
    gFunctionTable_Scene0.createVertexBuffer = createVertexBuffer;
    gFunctionTable_Scene0.createTexture = createTexture;
    gFunctionTable_Scene0.createUniformBuffer = createUniformBuffer;
    gFunctionTable_Scene0.updateUniformBuffer = updateUniformBuffer;
    gFunctionTable_Scene0.createShaders = createShaders;
    gFunctionTable_Scene0.createDescriptorSetLayout = createDescriptorSetLayout;
    gFunctionTable_Scene0.createPipelineLayout = createPipelineLayout;
    gFunctionTable_Scene0.createDescriptorPool = createDescriptorPool;
    gFunctionTable_Scene0.createDescriptorSet = createDescriptorSet;
    gFunctionTable_Scene0.createRenderPass = createRenderPass;
    gFunctionTable_Scene0.createPipeline = createPipeline;
    gFunctionTable_Scene0.createFrameBuffers = createFrameBuffers;
    gFunctionTable_Scene0.createSemaphores = createSemaphores;
    gFunctionTable_Scene0.createFences = createFences;
    gFunctionTable_Scene0.buildCommandBuffers = buildCommandBuffers;
    gFunctionTable_Scene0.debugReportCallback = debugReportCallback;
}

VkResult RenderToOffScreenTexture(VkImageView targetImageView, VkFramebuffer targetFramebuffer, VkRenderPass targetRenderPass)
{
    VkResult result = VK_SUCCESS;
    
    // Check if scene is initialized
    if (gCtx_Scene0.bInitialized == FALSE)
    {
        fprintf(gCtx_Scene0.gpFile, "RenderToOffScreenTexture() --> Scene0 not initialized\n");
        return VK_FALSE;
    }
    
    // Create command buffer for off-screen rendering
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = gCtx_Scene0.vkCommandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    
    VkCommandBuffer commandBuffer;
    result = vkAllocateCommandBuffers(gCtx_Scene0.vkDevice, &allocInfo, &commandBuffer);
    if (result != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "RenderToOffScreenTexture() --> Failed to allocate command buffer\n");
        return result;
    }
    
    // Begin command buffer
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (result != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "RenderToOffScreenTexture() --> Failed to begin command buffer\n");
        return result;
    }
    
    // Begin render pass
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = targetRenderPass;
    renderPassInfo.framebuffer = targetFramebuffer;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {gCtx_Scene0.winWidth, gCtx_Scene0.winHeight};
    
    VkClearValue clearValues[2];
    clearValues[0].color = gCtx_Scene0.vkClearColorValue;
    clearValues[1].depthStencil = gCtx_Scene0.vkClearDepthStencilValue;
    
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    
    // Bind pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gCtx_Scene0.vkPipeline);
    
    // Bind vertex buffers
    VkBuffer vertexBuffers[] = {gCtx_Scene0.vertexData_position.vkBuffer, gCtx_Scene0.vertexData_texcoord.vkBuffer};
    VkDeviceSize offsets[] = {0, 0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);
    
    // Bind descriptor set
    vkCmdBindDescriptorSet(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gCtx_Scene0.vkPipelineLayout, 0, 1, &gCtx_Scene0.vkDescriptorSet, 0, nullptr);
    
    // Set viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)gCtx_Scene0.winWidth;
    viewport.height = (float)gCtx_Scene0.winHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {gCtx_Scene0.winWidth, gCtx_Scene0.winHeight};
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    
    // Draw (assuming we have a quad or triangle)
    vkCmdDraw(commandBuffer, 6, 1, 0, 0); // Draw full-screen quad
    
    vkCmdEndRenderPass(commandBuffer);
    
    result = vkEndCommandBuffer(commandBuffer);
    if (result != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "RenderToOffScreenTexture() --> Failed to end command buffer\n");
        return result;
    }
    
    // Submit command buffer
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    
    result = vkQueueSubmit(gCtx_Scene0.vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        fprintf(gCtx_Scene0.gpFile, "RenderToOffScreenTexture() --> Failed to submit command buffer\n");
        return result;
    }
    
    // Wait for completion
    vkQueueWaitIdle(gCtx_Scene0.vkQueue);
    
    // Clean up command buffer
    vkFreeCommandBuffers(gCtx_Scene0.vkDevice, gCtx_Scene0.vkCommandPool, 1, &commandBuffer);
    
    return VK_SUCCESS;
}
