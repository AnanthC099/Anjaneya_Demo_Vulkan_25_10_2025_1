# Multi-Pass Offscreen Rendering System

This document describes the multi-pass offscreen rendering system implemented in the Quad project, which allows rendering Scene0 and Scene1 to offscreen textures and compositing them in a final pass.

## Overview

The system implements the following concept:
- **Multi-pass offscreen rendering to texture**: Scene0 and Scene1 are rendered to separate offscreen textures
- **Mixing/compositing in the final pass**: The offscreen textures are blended together in a final compositing pass

## Key Features

### Render Modes
- **RENDER_QUAD_ONLY (0)**: Renders only the original textured quad
- **RENDER_SCENE0 (1)**: Renders Scene0 to offscreen texture and composites it
- **RENDER_SCENE1 (2)**: Renders Scene1 to offscreen texture and composites it
- **RENDER_BOTH (3)**: Renders both scenes to offscreen textures and composites them

### Key Controls
- **'0' key**: Switch to quad-only rendering
- **'1' key**: Switch to Scene0 rendering
- **'2' key**: Switch to Scene1 rendering
- **'3' key**: Switch to both scenes rendering
- **'F' key**: Toggle fullscreen (existing functionality)
- **ESC key**: Exit application (existing functionality)

## Architecture

### Data Structures

#### GlobalContext_SceneCompositor
The main context structure has been extended with:
- `currentRenderMode`: Current rendering mode (enum)
- `scene0Offscreen`: Offscreen render target for Scene0
- `scene1Offscreen`: Offscreen render target for Scene1
- `compositingPipeline`: Pipeline for final compositing pass
- `compositingDescriptorSetLayout`: Descriptor set layout for compositing
- `compositingDescriptorPool`: Descriptor pool for compositing
- `compositingDescriptorSet`: Descriptor set for compositing
- `offscreenSampler`: Sampler for offscreen textures

#### OffscreenRenderTarget
Structure for offscreen rendering:
- `colorImage`: Color attachment image
- `colorImageMemory`: Memory for color image
- `colorImageView`: Image view for color attachment
- `depthImage`: Depth attachment image
- `depthImageMemory`: Memory for depth image
- `depthImageView`: Image view for depth attachment
- `framebuffer`: Framebuffer for offscreen rendering
- `renderPass`: Render pass for offscreen rendering
- `commandBuffer`: Command buffer for offscreen rendering

### Shaders

#### Shader_Compositor.vert
Vertex shader for compositing pass:
- Takes vertex position and texture coordinates
- Outputs transformed position and texture coordinates
- Uses MVP matrices and compositing parameters

#### Shader_Compositor.frag
Fragment shader for compositing pass:
- Samples Scene0, Scene1, and Quad textures
- Applies fade factors and blend modes
- Supports additive, alpha blend, and multiply blending
- Falls back to quad texture when both scenes are invisible

### Key Functions

#### createOffscreenRenderTargets()
Creates offscreen render targets for Scene0 and Scene1:
- Creates color and depth images
- Creates image views
- Creates framebuffers
- Creates render passes
- Creates command buffers

#### createCompositingPipeline()
Creates the compositing pipeline:
- Creates descriptor set layout for multiple textures
- Creates descriptor pool and descriptor sets
- Creates compositing pipeline using new shaders
- Creates offscreen sampler

#### renderScene0ToOffscreen()
Renders Scene0 to offscreen texture:
- Sets up offscreen framebuffer
- Calls Scene0 rendering functions
- Records commands to render Scene0 to offscreen texture

#### renderScene1ToOffscreen()
Renders Scene1 to offscreen texture:
- Sets up offscreen framebuffer
- Calls Scene1 rendering functions
- Records commands to render Scene1 to offscreen texture

#### renderCompositingPass()
Renders the final compositing pass:
- Binds compositing pipeline
- Binds offscreen textures as input
- Renders fullscreen quad with compositing shader

#### setRenderMode(int mode)
Sets the current rendering mode:
- Validates mode parameter
- Updates current render mode
- Logs the change

## Implementation Status

### Completed
- ✅ Basic architecture and data structures
- ✅ Key handling for render mode switching
- ✅ Shader creation and compilation
- ✅ Function stubs and placeholders
- ✅ Display function modification for multi-pass rendering

### Pending Implementation
- ⏳ Actual offscreen render target creation
- ⏳ Compositing pipeline implementation
- ⏳ Scene0 and Scene1 integration
- ⏳ Descriptor set management
- ⏳ Memory management and cleanup

## Usage

1. **Build the project**: Use the existing Build.bat script
2. **Run the application**: Execute SceneCompositor.exe
3. **Switch render modes**: Use keys 0-3 to switch between different rendering modes
4. **Toggle fullscreen**: Use F key to toggle fullscreen mode
5. **Exit**: Use ESC key to exit the application

## Future Enhancements

- **Animation support**: Add time-based animations for fade factors
- **Advanced blending**: Implement more blend modes (screen, overlay, etc.)
- **Performance optimization**: Optimize offscreen rendering performance
- **Memory management**: Implement proper cleanup of offscreen resources
- **Error handling**: Add comprehensive error handling and validation

## Notes

This implementation provides a foundation for multi-pass offscreen rendering. The actual rendering integration with Scene0 and Scene1 would require:
1. Linking with Scene0 and Scene1 libraries
2. Implementing actual offscreen render target creation
3. Integrating Scene0 and Scene1 rendering functions
4. Implementing proper descriptor set management
5. Adding memory management and cleanup functions

The current implementation serves as a working framework that can be extended with the actual rendering functionality.