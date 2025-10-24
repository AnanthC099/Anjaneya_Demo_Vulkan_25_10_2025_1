cls

del *.spv *.bak *.txt *.res *.obj *.exe

glslangValidator.exe -V -H -o shader_Scene0.vert.spv shader_Scene0.vert

glslangValidator.exe -V -H -o shader_Scene0.frag.spv shader_Scene0.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\Include Scene0.cpp

rc.exe Scene0.rc

link.exe  Scene0.obj Scene0.res /LIBPATH:C:\VulkanSDK\Anjaneya\Lib /SUBSYSTEM:WINDOWS

Scene0.exe

del Scene0.obj Scene0.exe

