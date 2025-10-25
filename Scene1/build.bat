cls

del *.exe *.txt *.spv *.bak *.res *.obj

glslangValidator.exe -V -H -o Shader_Scene1.vert.spv Shader_Scene1.vert

glslangValidator.exe -V -H -o Shader_Scene1.frag.spv Shader_Scene1.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\include Scene1.cpp

rc.exe Scene1.rc

link.exe Scene1.obj Scene1.res /LIBPATH:C:\VulkanSDK\Anjaneya\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

del *.bak *.res *.obj

Scene1.exe


