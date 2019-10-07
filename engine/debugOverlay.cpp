#include "debugOverlay.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanDevice.hpp"
#include "vulkanPipelineHelpers.hpp"
#include <algorithm>

#include "playerController.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "entityManager.hpp"
#include "components.hpp"

bool DebugOverlay::checkBuffers()
{
	ImDrawData* drawData = ImGui::GetDrawData();

	VkDeviceSize vSize = drawData->TotalVtxCount * sizeof( ImDrawVert );
	VkDeviceSize iSize = drawData->TotalIdxCount * sizeof( ImDrawIdx );

	bool buffersRebuilt = false;

	if ( vSize == 0 || iSize == 0 )
	{
		return buffersRebuilt;
	}

	// update buffer sizes if necessary
	if ( vertexBuffer.buffer == VK_NULL_HANDLE || vertexCount != drawData->TotalVtxCount )
	{
		vertexBuffer.unmap();
		vertexBuffer.destroy();
		CreateBuffer( vSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer, *device );

		vertexBuffer.unmap();
		vertexBuffer.map();

		vertexCount = drawData->TotalVtxCount;
		buffersRebuilt = true;
	}

	if ( indexBuffer.buffer == VK_NULL_HANDLE || indexCount != drawData->TotalIdxCount )
	{
		indexBuffer.unmap();
		indexBuffer.destroy();
		CreateBuffer( iSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBuffer, *device );

		indexBuffer.map();

		indexCount = drawData->TotalIdxCount;
		buffersRebuilt = true;
	}

	// upload data to buffer
	vertexBuffer.offset = 0;
	indexBuffer.offset = 0;

	for ( int i = 0; i < drawData->CmdListsCount; i++ )
	{
		ImDrawList* commands = drawData->CmdLists[i];
		size_t vAllocSize = commands->VtxBuffer.Size * sizeof( ImDrawVert );
		size_t iAllocSize = commands->IdxBuffer.Size * sizeof( ImDrawIdx );

		ImDrawVert* vData = (ImDrawVert*)vertexBuffer.allocate( vAllocSize );
		ImDrawIdx* iData = (ImDrawIdx*)indexBuffer.allocate( iAllocSize );

		memcpy( vData, commands->VtxBuffer.Data, vAllocSize );
		memcpy( iData, commands->IdxBuffer.Data, iAllocSize );
	}

	vertexBuffer.flush();
	indexBuffer.flush();

	return !buffersRebuilt;
}

void DebugOverlay::createFontResources()
{
	ImGuiIO& io = ImGui::GetIO();

	// setup font 
	unsigned char* fData;
	int texWidth;
	int texHeight;
	io.Fonts->AddFontFromFileTTF( "core\\Roboto-Medium.ttf", 16.f );
	io.Fonts->GetTexDataAsRGBA32( &fData, &texWidth, &texHeight );
	VkDeviceSize fSize = texWidth * texHeight * 4 * sizeof( char );

	CreateImageProperties imageProps = {};
	imageProps.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageProps.width = (uint32_t)texWidth;
	imageProps.height = (uint32_t)texHeight;
	imageProps.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageProps.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageProps.memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	// Create Font resources
	CreateImage( imageProps, fontTexture.image, fontTexture.memory,
		*device );

	CreateImageView( device->logicalDevice, fontTexture.image,
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT,
		&fontTexture.view );

	// upload the image to device memory
	VulkanBuffer stagingBuffer;
	CreateBuffer( fSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, *device );

	stagingBuffer.map();
	memcpy( stagingBuffer.data, fData, fSize );
	stagingBuffer.unmap();

	TransitionImageLayout( fontTexture.image, imageProps.format, VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, device, graphicsQueue );

	CopyBufferToImage( stagingBuffer.buffer, fontTexture.image, texWidth,
		texHeight, device, graphicsQueue );

	TransitionImageLayout( fontTexture.image, imageProps.format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, device, graphicsQueue );

	stagingBuffer.destroy();

	// create sampler to read from image 
	VkSamplerCreateInfo sci = {};
	sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sci.magFilter = VK_FILTER_LINEAR;
	sci.minFilter = VK_FILTER_LINEAR;
	sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sci.maxAnisotropy = 1.f;
	sci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

	VKCHECK( vkCreateSampler( device->logicalDevice, &sci, nullptr, &fontSampler ) );

	// descriptor pools, layouts, and sets
	VkDescriptorPoolSize poolSize;
	poolSize.descriptorCount = 1;
	poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	VkDescriptorPoolCreateInfo poolCI = {};
	poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCI.maxSets = 2;
	poolCI.poolSizeCount = 1;
	poolCI.pPoolSizes = &poolSize;

	vkCreateDescriptorPool( device->logicalDevice, &poolCI, nullptr, &descriptorPool );

	VkDescriptorSetLayoutBinding descSetBinding = {};
	descSetBinding.descriptorCount = 1;
	descSetBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descSetBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo descLayout = {};
	descLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descLayout.pBindings = &descSetBinding;
	descLayout.bindingCount = 1;

	vkCreateDescriptorSetLayout( device->logicalDevice, &descLayout, nullptr,
		&descriptorSetLayout );

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;

	vkAllocateDescriptorSets( device->logicalDevice, &allocInfo, &descriptorSet );

	// update the set
	VkDescriptorImageInfo dii = {};
	dii.sampler = fontSampler;
	dii.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	dii.imageView = fontTexture.view;

	VkWriteDescriptorSet writeDescriptorSet = {};
	writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.pImageInfo = &dii;

	vkUpdateDescriptorSets( device->logicalDevice, 1, &writeDescriptorSet,
		0, nullptr );
}

void DebugOverlay::createPipeline()
{
	VkPushConstantRange pcRange = {};
	pcRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pcRange.size = sizeof( OverlayConstants );

	VkPipelineLayoutCreateInfo pci = {};
	pci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pci.setLayoutCount = 1;
	pci.pSetLayouts = &descriptorSetLayout;
	pci.pushConstantRangeCount = 1;
	pci.pPushConstantRanges = &pcRange;

	VKCHECK( vkCreatePipelineLayout( device->logicalDevice, &pci, nullptr, &graphicsPipelineLayout ) );

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		CreatePipelineInputAssemblyStateCreateInfo( VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST );

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		CreatePipelineRasterizationStateCreateInfo( VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE );

	VkPipelineColorBlendAttachmentState cbas = {};
	cbas.blendEnable = VK_TRUE;

	cbas.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	
	cbas.colorBlendOp = VK_BLEND_OP_ADD;
	cbas.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	cbas.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

	cbas.alphaBlendOp = VK_BLEND_OP_ADD;
	cbas.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	cbas.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		CreatePipelineColorBlendStateCreateInfo( cbas );

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		CreatePipelineDepthStencilStateCreateInfo( VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL );

	VkPipelineMultisampleStateCreateInfo multisampleState =
		CreatePipelineMultisampleStateCreateInfo();

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.scissorCount = 1;
	viewportState.viewportCount = 1;

	std::vector<VkDynamicState> dynStates = { VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicStates = {};
	dynamicStates.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStates.pDynamicStates = dynStates.data();
	dynamicStates.dynamicStateCount = 2;

	VkPipelineShaderStageCreateInfo vShader = CreatePipelineShaderStageCreateInfo(
		"core\\uioverlay.vspv", VK_SHADER_STAGE_VERTEX_BIT, device->logicalDevice
	);

	VkPipelineShaderStageCreateInfo fShader = CreatePipelineShaderStageCreateInfo(
		"core\\uioverlay.fspv", VK_SHADER_STAGE_FRAGMENT_BIT, device->logicalDevice
	);

	std::vector<VkPipelineShaderStageCreateInfo> shaders = {
		vShader, fShader
	};

	std::vector<VkVertexInputBindingDescription> vertexBindings = {
		CreateVertexInputBindingDescription( 0, sizeof( ImDrawVert ), VK_VERTEX_INPUT_RATE_VERTEX )
	};

	std::vector<VkVertexInputAttributeDescription> vertexAttributes = {
		CreateVertexInputAttributeDescription( 0, 0, VK_FORMAT_R32G32_SFLOAT,
		offsetof( ImDrawVert, ImDrawVert::pos ) ),
		CreateVertexInputAttributeDescription( 0, 1, VK_FORMAT_R32G32_SFLOAT,
		offsetof( ImDrawVert, ImDrawVert::uv ) ),
		CreateVertexInputAttributeDescription( 0, 2, VK_FORMAT_R8G8B8A8_UNORM,
		offsetof( ImDrawVert, ImDrawVert::col ) )
	};

	VkPipelineVertexInputStateCreateInfo vertexInputState = {};
	vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputState.pVertexAttributeDescriptions = vertexAttributes.data();
	vertexInputState.vertexAttributeDescriptionCount = (uint32_t)vertexAttributes.size();
	vertexInputState.pVertexBindingDescriptions = vertexBindings.data();
	vertexInputState.vertexBindingDescriptionCount = (uint32_t)vertexBindings.size();

	VkGraphicsPipelineCreateInfo gpci = {};
	gpci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	gpci.pColorBlendState = &colorBlendState;
	gpci.pDepthStencilState = &depthStencilState;
	gpci.pDynamicState = &dynamicStates;
	gpci.pInputAssemblyState = &inputAssemblyState;
	gpci.pMultisampleState = &multisampleState;
	gpci.pRasterizationState = &rasterizationState;
	gpci.pStages = shaders.data();
	gpci.pViewportState = &viewportState;
	gpci.pVertexInputState = &vertexInputState;
	gpci.stageCount = 2;
	gpci.subpass = 0;
	gpci.layout = graphicsPipelineLayout;
	gpci.renderPass = renderPass;
	gpci.basePipelineIndex = -1;

	VKCHECK( vkCreateGraphicsPipelines( device->logicalDevice, VK_NULL_HANDLE, 1,
		&gpci, nullptr, &graphicsPipeline ) );

	vkDestroyShaderModule( device->logicalDevice, vShader.module, nullptr );
	vkDestroyShaderModule( device->logicalDevice, fShader.module, nullptr );
}

void DebugOverlay::draw( VkCommandBuffer commandBuffer )
{
	ImDrawData* drawData = ImGui::GetDrawData();
	if ( drawData == nullptr || drawData->CmdListsCount == 0 )
	{
		return;
	}

	VkViewport viewport = {};
	viewport.width = window_width.floatValue;
	viewport.height = window_height.floatValue;
	viewport.minDepth = 0.f;
	viewport.maxDepth = 1.f;

	VkRect2D scissor = {};
	scissor.extent = { (uint32_t)viewport.width, (uint32_t)viewport.height };

	vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipelineLayout, 0, 1, &descriptorSet, 0, nullptr );
	vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipeline );

	vkCmdSetScissor( commandBuffer, 0, 1, &scissor );
	vkCmdSetViewport( commandBuffer, 0, 1, &viewport );

	ImGuiIO& io = ImGui::GetIO();
	pushConstants.scale = glm::vec2( 2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y );
	pushConstants.translate = glm::vec3( -1.f );
	vkCmdPushConstants( commandBuffer, graphicsPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof( OverlayConstants ), &pushConstants );

	int32_t vertexOffset = 0;
	int32_t indexOffset = 0;

	if ( drawData->CmdListsCount == 0 )
	{
		return;
	}

	VkDeviceSize offset[1] = { 0 };
	vkCmdBindVertexBuffers( commandBuffer, 0, 1, &vertexBuffer.buffer, offset );
	vkCmdBindIndexBuffer( commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16 );

	for ( size_t i = 0; i < drawData->CmdListsCount; i++ )
	{
		ImDrawList* commands = drawData->CmdLists[i];
		for ( int32_t j = 0; j < commands->CmdBuffer.Size; j++ )
		{
			ImDrawCmd* cmd = &commands->CmdBuffer[j];

			VkRect2D scissor;
			scissor.offset.x = std::max( int32_t( cmd->ClipRect.x ), 0 );
			scissor.offset.y = std::max( int32_t( cmd->ClipRect.y ), 0 );
			scissor.extent.width = uint32_t( cmd->ClipRect.z - cmd->ClipRect.x );
			scissor.extent.height = uint32_t( cmd->ClipRect.w - cmd->ClipRect.y );

			vkCmdSetScissor( commandBuffer, 0, 1, &scissor );
			vkCmdDrawIndexed( commandBuffer, cmd->ElemCount, 1,
				indexOffset, vertexOffset, 0 );

			indexOffset += cmd->ElemCount;
		}

		vertexOffset += commands->VtxBuffer.Size;
	}
}

void DebugOverlay::update( VkCommandBuffer commandBuffer ) {}

void DebugOverlay::init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO().DisplaySize = ImVec2( window_width.floatValue, window_height.floatValue );
	ImGui::GetIO().DisplayFramebufferScale = ImVec2( 1.f, 1.f );

	createFontResources();
	createPipeline();
}

void DebugOverlay::shutdown()
{
	ImGui::DestroyContext();

	vertexBuffer.destroy();
	indexBuffer.destroy();

	vkDestroyDescriptorSetLayout( device->logicalDevice, descriptorSetLayout, nullptr );
	vkDestroyDescriptorPool( device->logicalDevice, descriptorPool, nullptr );
	vkDestroySampler( device->logicalDevice, fontSampler, nullptr );

	vkDestroyPipelineLayout( device->logicalDevice, graphicsPipelineLayout, nullptr );
	vkDestroyPipeline( device->logicalDevice, graphicsPipeline, nullptr );

	vkDestroyImageView( device->logicalDevice, fontTexture.view, nullptr );
	vkDestroyImage( device->logicalDevice, fontTexture.image, nullptr );
	vkFreeMemory( device->logicalDevice, fontTexture.memory, nullptr );
}