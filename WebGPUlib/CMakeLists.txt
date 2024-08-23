cmake_minimum_required(VERSION 3.29)

project( WebGPUlib VERSION 0.1.0 LANGUAGES CXX C)

set( TARGET_NAME WebGPUlib )

set( INC
	inc/bitmask_operators.hpp
	inc/WebGPUlib/BindGroup.hpp
	inc/WebGPUlib/Buffer.hpp
	inc/WebGPUlib/CommandBuffer.hpp
	inc/WebGPUlib/ComputeCommandBuffer.hpp
	inc/WebGPUlib/ComputePipelineState.hpp
	inc/WebGPUlib/Device.hpp
	inc/WebGPUlib/GenerateMipsPipelineState.hpp
	inc/WebGPUlib/GraphicsCommandBuffer.hpp
	inc/WebGPUlib/GraphicsPipelineState.hpp
	inc/WebGPUlib/Hash.hpp
	inc/WebGPUlib/IndexBuffer.hpp
	inc/WebGPUlib/Material.hpp
	inc/WebGPUlib/Mesh.hpp
	inc/WebGPUlib/Queue.hpp
	inc/WebGPUlib/RenderTarget.hpp
	inc/WebGPUlib/Sampler.hpp
	inc/WebGPUlib/Scene.hpp
	inc/WebGPUlib/SceneNode.hpp
	inc/WebGPUlib/Surface.hpp
	inc/WebGPUlib/Texture.hpp
	inc/WebGPUlib/TextureView.hpp
	inc/WebGPUlib/UniformBuffer.hpp
	inc/WebGPUlib/Vertex.hpp
	inc/WebGPUlib/VertexBuffer.hpp
)

set( SRC
	src/BindGroup.cpp
	src/Buffer.cpp
	src/CommandBuffer.cpp
	src/ComputeCommandBuffer.cpp
	src/ComputePipelineState.cpp
	src/Device.cpp
	src/GenerateMipsPipelineState.cpp
	src/GraphicsCommandBuffer.cpp
	src/GraphicsPipelineState.cpp
	src/IndexBuffer.cpp
	src/Material.cpp
	src/Mesh.cpp
	src/Queue.cpp
	src/RenderTarget.cpp
	src/Sampler.cpp
	src/Scene.cpp
	src/SceneNode.cpp
	src/Surface.cpp
	src/Texture.cpp
	src/TextureView.cpp
	src/UniformBuffer.cpp
	src/Vertex.cpp
	src/VertexBuffer.cpp
)

set( SHADERS
	shaders/GenerateMips.wgsl
)

add_library( ${TARGET_NAME} STATIC ${INC} ${SRC} ${SHADERS} )

source_group( "Shaders" FILES ${SHADERS} )

target_compile_features( WebGPUlib
PUBLIC
	cxx_std_17
)

target_include_directories( ${TARGET_NAME}
PUBLIC
	inc
)

target_link_libraries( ${TARGET_NAME}
PUBLIC
	SDL2::SDL2 glm::glm webgpu sdl2webgpu stb_image tinyobjloader
)
