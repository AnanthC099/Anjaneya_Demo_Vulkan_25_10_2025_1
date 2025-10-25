cls

del Scene0.exe Scene1.exe SceneCompositor.exe Log.txt Shader_Scene0.vert.spv Shader_Scene0.frag.spv Shader_Scene1.vert.spv Shader_Scene1.frag.spv Shader_SceneCompositor.vert.spv Shader_SceneCompositor.frag.spv Scene0.embed.obj Scene1.embed.obj SceneCompositor.obj SceneCompositor.res SceneCompositor.pdb

glslangValidator.exe -V -H -o Shader_Scene0.vert.spv Shader_Scene0.vert

glslangValidator.exe -V -H -o Shader_Scene0.frag.spv Shader_Scene0.frag

glslangValidator.exe -V -H -o Shader_Scene1.vert.spv Shader_Scene1.vert

glslangValidator.exe -V -H -o Shader_Scene1.frag.spv Shader_Scene1.frag

glslangValidator.exe -V -H -o composite_fullscreen.vert.spv composite_fullscreen.vert

glslangValidator.exe -V -H -o composite_blend.frag.spv composite_blend.frag

cl /nologo /I"C:\VulkanSDK\Anjaneya\Include" /DSCENE_EMBED /c Scene0.cpp /Fo"Scene0.embed.obj"

cl /nologo /I"C:\VulkanSDK\Anjaneya\Include" /DSCENE_EMBED /c Scene1.cpp /Fo"Scene1.embed.obj"

cl /nologo /I"C:\VulkanSDK\Anjaneya\Include" /c SceneCompositor.cpp /Fo"SceneCompositor.obj"

rc.exe SceneCompositor.rc

link SceneCompositor.obj SceneCompositor.res /LIBPATH:"C:\VulkanSDK\Anjaneya\Lib" vulkan-1.lib user32.lib gdi32.lib kernel32.lib Sphere.lib /OUT:SceneCompositor.exe

SceneCompositor.exe
