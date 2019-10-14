#include "tetRenderer.hpp"
#include "tetBoard.hpp"
#include "camera.hpp"

void TetOverlay::update( VkCommandBuffer commandBuffer )
{
	if ( !display )
	{
		return;
	}


	ImGui::NewFrame();

	ImGui::SetNextWindowSize( ImVec2( 200.f, 100.f ) );
	ImGui::Begin( "Debug Overlay", nullptr, ImGuiWindowFlags_NoSavedSettings );
	
	ImGui::End();

	ImGui::Render();

	if ( checkBuffers() )
	{
		draw( commandBuffer );
	}
}

void TetRenderer::childInit()
{
	overlay.display = true;
	overlay.device = &device;
	overlay.graphicsQueue = graphicsQueue;
	overlay.renderPass = renderPass;
	overlay.init();


	
}

void TetRenderer::childShutdown()
{
	overlay.shutdown();
}

void TetRenderer::draw()
{
	Camera* cam = Camera::instance();
	VkCommandBuffer cmdBuf = commandBuffers[currentImageIndex];

	// push the static camera data into the shader data.
	glm::mat4x4 pushConstant = cam->getProjection() * cam->getView();
	vkCmdPushConstants( cmdBuf, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof( glm::mat4x4 ), &pushConstant );

	Board* board = Board::instance();

	size_t width = board->field.size();
	size_t height = board->field[0].size();

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;
	
	VkBuffer vertexBuffers[] = { vertexBuffer.buffer };
	VkDeviceSize offsets[] = { 0 };

	uint32_t vertexOffset = 0;
	
	vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );
		
	const VulkanTexture* texture = getTexture( "notexture" );
	vkCmdBindDescriptorSets( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout, 0, 1, &texture->descriptor, 0, &vertexOffset );

	for ( size_t y = 0; y < height; y++ )
	{
		for ( size_t x = 0; x < width; x++ )
		{
			Cell& c = board->field[x][y];

			if ( c.hasEntity )
			{

			}
		}
	}

	vkCmdBindIndexBuffer( commandBuffers[currentImageIndex], indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32 );
	vkCmdBindVertexBuffers( cmdBuf, 1, 1, &transformBuffer.buffer, offsets );
	vkCmdBindVertexBuffers( cmdBuf, 0, 1, vertexBuffers, offsets );

	vkCmdDrawIndexed( commandBuffers[currentImageIndex], indexCount, 1, 0, 0, 0 );
	
	vertexBuffer.offset = 0;
	indexBuffer.offset = 0;
}

void TetRenderer::drawFrame()
{
	beginDraw();
	draw();
	overlay.update( commandBuffers[currentImageIndex] );
	endDraw();
}