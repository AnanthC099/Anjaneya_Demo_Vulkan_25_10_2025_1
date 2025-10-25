cls

del Scene0.exe Scene1.exe Scene2.exe SceneCompositor.exe Log.txt Scene0.embed.obj Scene1.embed.obj SceneCompositor.obj SceneCompositor.res SceneCompositor.pdb Shader_Scene0.vert.spv Shader_Scene0.frag.spv Shader_Scene1.vert.spv Shader_Scene1.frag.spv shader_Scene2.vert.spv shader_Scene2.frag.spv composite_fullscreen.vert.spv composite_blend.frag.spv *.txt *.spv *.bak *.res *.obj

REM Scene 0
glslangValidator.exe -V -H -o shader_Scene0.vert.spv shader_Scene0.vert
glslangValidator.exe -V -H -o shader_Scene0.frag.spv shader_Scene0.frag
cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\Include Scene0.cpp
rc.exe Scene0.rc
link.exe Scene0.obj Scene0.res /LIBPATH:C:\VulkanSDK\Anjaneya\Lib /SUBSYSTEM:WINDOWS
Scene0.exe
del Scene0.obj Scene0.exe

REM Scene 1
glslangValidator.exe -V -H -o Shader_Scene1.vert.spv Shader_Scene1.vert
glslangValidator.exe -V -H -o Shader_Scene1.frag.spv Shader_Scene1.frag
cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\include Scene1.cpp
rc.exe Scene1.rc
link.exe Scene1.obj Scene1.res /LIBPATH:C:\VulkanSDK\Anjaneya\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
del *.bak *.res *.obj
Scene1.exe

REM Scene 2
glslangValidator.exe -V -H -o shader_Scene2.vert.spv shader_Scene2.vert
glslangValidator.exe -V -H -o shader_Scene2.frag.spv shader_Scene2.frag
cl.exe /c /EHsc /I C:\VulkanSDK\Anjaneya\include Scene2.cpp
rc.exe Scene2.rc
link.exe Scene2.obj Scene2.res /LIBPATH:C:\VulkanSDK\Anjaneya\lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS
del *.bak *.res *.obj
Scene2.exe

REM SceneCompositor
glslangValidator.exe -V -H -o composite_fullscreen.vert.spv Reference/composite_fullscreen.vert
glslangValidator.exe -V -H -o composite_blend.frag.spv Reference/composite_blend.frag
cl /nologo /I"C:\VulkanSDK\Anjaneya\Include" /DSCENE_EMBED /c Scene0.cpp /Fo"Scene0.embed.obj"
cl /nologo /I"C:\VulkanSDK\Anjaneya\Include" /DSCENE_EMBED /c Scene1.cpp /Fo"Scene1.embed.obj"
cl /nologo /I"C:\VulkanSDK\Anjaneya\Include" /c SceneCompositor.cpp /Fo"SceneCompositor.obj"
rc.exe SceneCompositor.rc
link SceneCompositor.obj SceneCompositor.res /LIBPATH:"C:\VulkanSDK\Anjaneya\Lib" vulkan-1.lib user32.lib gdi32.lib kernel32.lib Sphere.lib /OUT:SceneCompositor.exe
SceneCompositor.exe