cls

del *.exe *.txt *.spv *.bak *.res *.obj

glslangValidator.exe -V -H -o Shader.vert.spv Shader.vert

glslangValidator.exe -V -H -o Shader.frag.spv Shader.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\include VK.cpp

rc.exe VK.rc

link.exe VK.obj VK.res /LIBPATH:C:\VulkanSDK\Anjaneya\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

del *.bak *.res *.obj

VK.exe


