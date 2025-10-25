del VK.exe
del Log.txt
del VK.res
del VK.obj
cls

cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\include VK.cpp

rc.exe VK.rc

link.exe VK.obj VK.res /LIBPATH:C:\VulkanSDK\Anjaneya\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

VK.exe


