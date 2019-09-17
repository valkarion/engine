#include "vulkanPipelineHelpers.hpp"
#include "vulkanCommon.hpp"
#include "fileSystem.hpp"

VkPipelineShaderStageCreateInfo CreatePipelineShaderStageCreateInfo( const char* path, 
	VkShaderStageFlagBits stages, VkDevice device )
{
	std::vector<char> code = ReadBinaryFile( path );

	VkShaderModuleCreateInfo smci = {};
	smci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	smci.codeSize = code.size();
	smci.pCode = (const uint32_t*)code.data();

	VkShaderModule module;
	VKCHECK( vkCreateShaderModule( device, &smci, nullptr, &module ) );

	VkPipelineShaderStageCreateInfo sci = {};
	sci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	sci.stage = stages;
	sci.module = module;
	sci.pName = "main";

	return sci;
}

VkVertexInputAttributeDescription CreateVertexInputAttributeDescription( uint32_t bindingNumber, 
	uint32_t location, VkFormat typeFormat, uint32_t offset )
{
	VkVertexInputAttributeDescription desc = {};

	desc.binding = bindingNumber;
	desc.location = location;
	desc.format = typeFormat;
	desc.offset = offset;

	return desc;
}

VkVertexInputBindingDescription	CreateVertexInputBindingDescription(
	uint32_t bindingNumber, uint32_t stride, VkVertexInputRate rate )
{
	VkVertexInputBindingDescription desc = {};

	desc.binding = bindingNumber;
	desc.stride = stride;
	desc.inputRate = rate;

	return desc;
}

VkPipelineInputAssemblyStateCreateInfo CreatePipelineInputAssemblyStateCreateInfo( VkPrimitiveTopology topology )
{
	VkPipelineInputAssemblyStateCreateInfo iac = {};
	iac.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	iac.topology = topology;
	iac.primitiveRestartEnable = VK_FALSE;

	return iac;
}

VkPipelineViewportStateCreateInfo CreatePipelineViewportStateCreateInfo( VkViewport& viewport, VkRect2D& scissor, uint32_t width, uint32_t height )
{
	// what part of the framebuffer should be rendered to? (all of it)
	viewport.x = 0.f;
	viewport.y = 0.f;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.0f;
	viewport.width = (float)width;
	viewport.height = (float)height;

	// what part of the frame buffer should we keep? (all of it)
	scissor.offset = VkOffset2D{ 0, 0 };
	scissor.extent = VkExtent2D{ width, height };

	VkPipelineViewportStateCreateInfo vsci = {};
	vsci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vsci.pScissors = &scissor;
	vsci.scissorCount = 1;
	vsci.pViewports = &viewport;
	vsci.viewportCount = 1;

	return vsci;
}

VkPipelineRasterizationStateCreateInfo CreatePipelineRasterizationStateCreateInfo( VkPolygonMode polygonMode, VkCullModeFlags cullMode )
{
	VkPipelineRasterizationStateCreateInfo rsci = {};
	rsci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// keep fragments that are not between the near and far plane? 
	rsci.depthClampEnable = VK_FALSE;
	// disable frame buffer output?
	rsci.rasterizerDiscardEnable = VK_FALSE;
	// how to generate fragments for the polygon? 	
	rsci.polygonMode = polygonMode;
	rsci.lineWidth = 1.f;
	rsci.cullMode = cullMode;
	// GLM matricies need this to be CCW
	rsci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rsci.depthBiasEnable = VK_FALSE;

	return rsci;
}

VkPipelineMultisampleStateCreateInfo CreatePipelineMultisampleStateCreateInfo()
{
	VkPipelineMultisampleStateCreateInfo msc = {};
	
	msc.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	msc.sampleShadingEnable = VK_FALSE;
	msc.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	return msc;
}

VkPipelineDepthStencilStateCreateInfo CreatePipelineDepthStencilStateCreateInfo()
{
	VkPipelineDepthStencilStateCreateInfo dsci = {};
	
	dsci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	dsci.depthTestEnable = VK_TRUE;
	dsci.depthWriteEnable = VK_TRUE;
	dsci.depthCompareOp = VK_COMPARE_OP_LESS;
	dsci.depthBoundsTestEnable = VK_FALSE;
	dsci.stencilTestEnable = VK_FALSE;

	return dsci;
}

VkPipelineColorBlendStateCreateInfo CreatePipelineColorBlendStateCreateInfo( VkPipelineColorBlendAttachmentState& attachmentState )
{
	attachmentState = {};
	attachmentState.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	attachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo cbsci = {};

	cbsci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	cbsci.pAttachments = &attachmentState;
	cbsci.attachmentCount = 1;
	cbsci.logicOp = VK_LOGIC_OP_COPY;
	cbsci.logicOpEnable = VK_FALSE;

	return cbsci;
};