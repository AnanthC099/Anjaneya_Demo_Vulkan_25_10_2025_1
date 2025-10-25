cls

del *.exe *.txt *.spv *.bak *.res *.obj

glslangValidator.exe -V -H -o Shader_SceneCompositor.vert.spv Shader_SceneCompositor.vert

glslangValidator.exe -V -H -o Shader_SceneCompositor.frag.spv Shader_SceneCompositor.frag

cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\include SceneCompositor.cpp

rc.exe SceneCompositor.rc

link.exe SceneCompositor.obj SceneCompositor.res /LIBPATH:C:\VulkanSDK\Anjaneya\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS

del *.bak *.res *.obj

SceneCompositor.exe


