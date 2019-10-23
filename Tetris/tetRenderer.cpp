#include "tetRenderer.hpp"
#include "tetBoard.hpp"
#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


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

	const int boardVertecies = 5000;

	VkDeviceSize bufferSize = boardVertecies * sizeof( Vertex );
	VkBufferUsageFlags flags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VkMemoryPropertyFlags memProps = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	CreateBuffer( bufferSize, flags, memProps, dynamicVertexBuffer, device );

	dynamicVertexBuffer.map();

	Camera::instance()->initCamera();
}

glm::mat4x4 GetModelMatrix( glm::vec3 position )
{
	glm::mat4x4 model( 1.f );

	// displacement
	glm::mat4 translation = glm::translate( model, position );
	
	return translation;
}

void TetRenderer::childShutdown()
{
	overlay.shutdown();

	dynamicVertexBuffer.destroy();
}

SquareMemoryAddr_t TetRenderer::allocSquareMemory()
{
	const uint32_t vertexCountPerSquare = 4;
	const uint32_t indexCountPerSquare = 6;

	Vertex* vertecies = (Vertex*)dynamicVertexBuffer.allocate( vertexCountPerSquare * sizeof( Vertex ) );
	uint32_t* indecies = (uint32_t*)dynamicIndexBuffer.allocate( indexCountPerSquare * sizeof( uint32_t ) );

	return std::make_pair( vertecies, indecies );
}

void TetRenderer::setupSquare( const SquareMemoryAddr_t& memory, 
	const glm::vec2& position, uint32_t indexOffset ) const
{
	memory.first[0].position = glm::vec3( -0.5f + position.x, -0.5f + position.y, 0.f );
	memory.first[0].color = glm::vec3( 1.f, 1.f, 1.f );
	memory.first[0].textureCoordinates = glm::vec2( 0.f, 0.f );

	memory.first[1].position = glm::vec3( 0.5f + position.x, -0.5f + position.y, 0.f );
	memory.first[1].color = glm::vec3( 1.f, 1.f, 1.f );
	memory.first[1].textureCoordinates = glm::vec2( 1.f, 0.f );

	memory.first[2].position = glm::vec3( 0.5f + position.x, 0.5f + position.y, 0.f );
	memory.first[2].color = glm::vec3( 1.f, 1.f, 1.f );
	memory.first[2].textureCoordinates = glm::vec2( 1.f, 1.f );

	memory.first[3].position = glm::vec3( -0.5f + position.x, 0.5f + position.y, 0.f );
	memory.first[3].color = glm::vec3( 1.f, 1.f, 1.f );
	memory.first[3].textureCoordinates = glm::vec2( 0.f, 1.f );

	memory.second[0] = indexOffset + 0;
	memory.second[1] = indexOffset + 1;
	memory.second[2] = indexOffset + 2;
	memory.second[3] = indexOffset + 2;
	memory.second[4] = indexOffset + 3;
	memory.second[5] = indexOffset + 0;
}

void TetRenderer::draw()
{
	Camera* cam = Camera::instance();	
	VkCommandBuffer cmdBuf = commandBuffers[currentImageIndex];

	cam->setPosition( glm::vec3( 0.f, 0.f, -1.f ) );
	cam->setDirection( glm::vec3( 0.f, 0.f, 1.f ) );

	// push the static camera data into the shader data.
	glm::mat4x4 pushConstant = cam->getProjection() * cam->getView();
	vkCmdPushConstants( cmdBuf, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof( glm::mat4x4 ), &pushConstant );

	Board* board = Board::instance();

	VkDeviceSize offsets[] = { 0 };
	
	uint32_t vertexOffset = 0;
	uint32_t indexOffset = 0;
	
	vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, wireframePipeline );
	vkCmdBindVertexBuffers( cmdBuf, 1, 1, &modelMatrixBuffer.buffer, offsets );
	vkCmdBindVertexBuffers( cmdBuf, 0, 1, &dynamicVertexBuffer.buffer, offsets );

	const size_t board_height = board->field.size();
	const size_t board_width = board->field[0].size();
	
	for ( size_t y = 0; y < board_height; y++ )
	{
		for ( size_t x = 0; x < board_width; x++ )
		{
			
		}
	}
	
	dynamicVertexBuffer.offset = 0;
	dynamicIndexBuffer.offset = 0;
	modelMatrixBuffer.offset = 0;
}

void TetRenderer::drawFrame()
{
	beginDraw();
	draw();
	overlay.update( commandBuffers[currentImageIndex] );
	endDraw();
}