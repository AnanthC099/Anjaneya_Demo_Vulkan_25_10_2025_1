cls

del *.exe *.txt *.spv *.bak *.res *.obj

glslangValidator.exe -V -H -o Shader_Scene2.vert.spv Shader_Scene2.vert

glslangValidator.exe -V -H -o Shader_Scene2.frag.spv Shader_Scene2.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\include Scene2.cpp

rc.exe Scene2.rc

link.exe Scene2.obj Scene2.res /LIBPATH:C:\VulkanSDK\Anjaneya\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

del *.bak *.res *.obj

Scene2.exe


