#include "debugOverlay.hpp"
#include "vulkanBuffer.hpp"
#include "vulkanDevice.hpp"
#include "vulkanPipelineHelpers.hpp"


void DebugOverlay::init()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	   
	ImGuiIO& io = ImGui::GetIO();

// setup font 
	unsigned char* fData;
	int texWidth;
	int texHeight;
	io.Fonts->AddFontFromFileTTF( "core\\Roboto-Medium.ttf", 16.f );
	io.Fonts->GetTexDataAsRGBA32( &fData, &texWidth, &texHeight );
	VkDeviceSize fSize = texWidth * texHeight * 4; // 4 - RGBA

	CreateImageProperties imageProps = {};
	imageProps.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageProps.width = (uint32_t)texWidth;
	imageProps.height = (uint32_t)texHeight;
	imageProps.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageProps.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageProps.memProps = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	CreateImage( imageProps, fontTexture.image, fontTexture.memory, 
		*device );

	CreateImageView( device->logicalDevice, fontTexture.image, 
		VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 
		&fontTexture.view );

	VulkanBuffer stagingBuffer;
	CreateBuffer( fSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, *device );

	stagingBuffer.map();
	memcpy( stagingBuffer.data, fData, fSize );
	stagingBuffer.unmap();
}

void DebugOverlay::shutdown()
{
	vkDestroyImageView( device->logicalDevice, fontTexture.view, nullptr );
	vkDestroyImage( device->logicalDevice, fontTexture.image, nullptr );
}