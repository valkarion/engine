#pragma once

#include <vulkan/vulkan.hpp>

/*
	Pipeline creation is a huge process and needs a lot of 
	submodules. Since the states are baked into the pipelines
	multiple pipelines are required to do different kinds of rending, 
	therefore its submodules must be parameterized differently.
	These are it's helper functions. Most of the things are still baked 
	but for wireframe and polygon rendering we need different things.
*/

// shader stage infos tell the pipeline in which stage it should run the shader code
VkPipelineShaderStageCreateInfo CreatePipelineShaderStageCreateInfo( const VkShaderModule shaderModule, VkShaderStageFlagBits stages, const char* entryPoint );

// input attribute describes the size and location of information within a given structure
VkVertexInputAttributeDescription CreateVertexInputAttributeDescription( uint32_t bindingNumber, uint32_t location, VkFormat typeFormat, uint32_t offset );

// binding description describes where to bind data so that the shader can read from those binding locations
VkVertexInputBindingDescription	CreateVertexInputBindingDescription( uint32_t bindingNumber, uint32_t stride, VkVertexInputRate rate );

// the kind of geometry will be drawn from the vertecies
VkPipelineInputAssemblyStateCreateInfo CreatePipelineInputAssemblyStateCreateInfo( VkPrimitiveTopology topology );

// the size of the displayed area ( usually the size of the window ) 
VkPipelineViewportStateCreateInfo CreatePipelineViewportStateCreateInfo( VkViewport& viewport, VkRect2D& scissor, uint32_t width, uint32_t height );

// how does the geometry get drawn? 
VkPipelineRasterizationStateCreateInfo CreatePipelineRasterizationStateCreateInfo( VkPolygonMode polygonMode, VkCullModeFlags cullMode );

// we dont use multisampling, but it needs to be defined as such anyway
VkPipelineMultisampleStateCreateInfo CreatePipelineMultisampleStateCreateInfo();

// depth stencil defines how to discard fragments that are above or below other fragments
VkPipelineDepthStencilStateCreateInfo CreatePipelineDepthStencilStateCreateInfo();

// What color combinations of the base RGBA colors should be displayed? This function will create a default RGBA display
VkPipelineColorBlendStateCreateInfo CreatePipelineColorBlendStateCreateInfo( VkPipelineColorBlendAttachmentState& attachmentState );
