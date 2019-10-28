#include "tetRenderer.hpp"
#include "tetBoard.hpp"
#include "camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const uint32_t vertexCountPerSquare = 4;
const uint32_t indexCountPerSquare = 6;

void TetRenderer::drawSingleCell( const VkCommandBuffer cmdBuf,
	const VkPipeline pipeline, const VkDescriptorSet dSet,
	const glm::vec3& position, const glm::vec2 scale )
{
	VkDeviceSize modelOffset = modelMatrixBuffer.offset;
	VkDeviceSize indexOffset = dynamicIndexBuffer.offset;

	setupSquare( allocSquareMemory() );

	vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );

	vkCmdBindIndexBuffer( cmdBuf, dynamicIndexBuffer.buffer, indexOffset, VK_INDEX_TYPE_UINT32 );
	vkCmdBindVertexBuffers( cmdBuf, 1, 1, &modelMatrixBuffer.buffer, &modelOffset );

	glm::mat4x4* model = ( glm::mat4x4* )modelMatrixBuffer.allocate( sizeof( glm::mat4x4 ) );
	*model = glm::translate( glm::mat4x4( 1.f ), position );
	*model = glm::scale( *model, glm::vec3( scale, 1.f ) );

	vkCmdBindDescriptorSets( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout,
		0, 1, &dSet, 0, nullptr );

	vkCmdDrawIndexed( cmdBuf, 6, 1, 0, 0, 0 );
}

std::string TexIndexToTexName( const int index )
{
	if ( index < 0 )
	{
		return "gray";
	}

	static std::array<std::string, (size_t)enu_BLOCK_TYPE::size> texNames = {
		"orange", "brown", "pink", "yellow", "blue", "green", "red" };

	return texNames[index];
}

void TetRenderer::draw()
{
	VkCommandBuffer cmdBuf = commandBuffers[currentImageIndex];

	Board* board = Board::instance();

	VkDeviceSize vOffset[1] = { 0 };
	vkCmdBindVertexBuffers( cmdBuf, 0, 1, &dynamicVertexBuffer.buffer, vOffset );

	// draw the board and all fix cells
	for ( size_t y = 0; y < board->height; y++ )
	{
		for ( size_t x = 0; x < board->width; x++ )
		{
			VkPipeline pipeline = wireframePipeline;
			int texIndex = -1;
			
			if ( board->getCell( x, y ).hasEntity )
			{
				pipeline = graphicsPipeline;
				texIndex = board->getCell( x, y ).textureIndex;
			}

			drawSingleCell( cmdBuf, pipeline,
				getTexture( TexIndexToTexName( texIndex ) )->descriptor,
				glm::vec3( (float)x, (float)y, 0.f )
			);
		}
	}

	// draw our current block
	CurrentBlock& cBlock = board->cBlock;
	int texIndex = (int)cBlock.type;

	for ( size_t y = 0; y < 4; y++ )
	{
		for ( size_t x = 0; x < 4; x++ )
		{
			if ( cBlock.isFilled( x, y ) )
			{
				glm::vec3 p = glm::vec3( cBlock.px + x, cBlock.py + y, 0.f );

				drawSingleCell( cmdBuf, graphicsPipeline,
					getTexture( TexIndexToTexName( texIndex ) )->descriptor,
					p );
			}
		}
	}
}

void TetRenderer::debugDraw()
{
	VkCommandBuffer cmdBuf = commandBuffers[currentImageIndex];

	Board* b = Board::instance();

	VkDeviceSize vOffset[1] = { 0 };
	vkCmdBindVertexBuffers( cmdBuf, 0, 1, &dynamicVertexBuffer.buffer, vOffset );

	for ( size_t y = 0; y < b->height; y++ )
	{
		for ( size_t x = 0; x < b->width; x++ )
		{
			drawSingleCell(
				cmdBuf, graphicsPipeline,
				getTexture(
					TexIndexToTexName(
					( y * b->width + x ) % (size_t)enu_BLOCK_TYPE::size )
				)->descriptor,
				glm::vec3( x, y, 0.f ) );
		}
	}
}

SquareMemInfo TetRenderer::allocSquareMemory()
{
	SquareMemInfo sma;

	Vertex* vertecies = (Vertex*)dynamicVertexBuffer.allocate(
		vertexCountPerSquare * sizeof( Vertex ) );

	uint32_t offset = dynamicIndexBuffer.offset / sizeof( uint32_t );

	uint32_t* indecies = (uint32_t*)dynamicIndexBuffer.allocate(
		indexCountPerSquare * sizeof( uint32_t ) );

	sma.idxAddr = indecies;
	sma.idxOffset = offset;
	sma.vertAddr = vertecies;

	return sma;
}

void TetRenderer::setupSquare( const SquareMemInfo& memory ) const
{
	memory.vertAddr[0].position = glm::vec3( -0.5f, -0.5f, 0.f );
	memory.vertAddr[0].textureCoordinates = glm::vec2( 0.f, 0.f );

	memory.vertAddr[1].position = glm::vec3( 0.5f, -0.5f, 0.f );
	memory.vertAddr[1].textureCoordinates = glm::vec2( 1.f, 0.f );

	memory.vertAddr[2].position = glm::vec3( 0.5f, 0.5f, 0.f );
	memory.vertAddr[2].textureCoordinates = glm::vec2( 1.f, 1.f );

	memory.vertAddr[3].position = glm::vec3( -0.5f, 0.5f, 0.f );
	memory.vertAddr[3].textureCoordinates = glm::vec2( 0.f, 1.f );
	
	memory.idxAddr[0] = 0;
	memory.idxAddr[1] = 1;
	memory.idxAddr[2] = 2;
	memory.idxAddr[3] = 2;
	memory.idxAddr[4] = 3;
	memory.idxAddr[5] = 0;
}

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
	overlay.display = false;
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

void TetRenderer::childShutdown()
{
	overlay.shutdown();
	dynamicVertexBuffer.destroy();
}

void TetRenderer::drawFrame()
{
	beginDraw();

	Board* board = Board::instance();

	Camera* cam = Camera::instance();
	// set the camera to the center of the board, and back enough to see it all 
	cam->setPosition( glm::vec3(
		float( board->width / 2 ),
		float( board->height / 2 ),
		(float)board->height / 1.5f )
	);
	cam->setDirection( glm::vec3( 0.f, 0.f, -1.f ) );

	// push the static camera data into the shader data.
	VkCommandBuffer cmdBuf = commandBuffers[currentImageIndex];
	glm::mat4x4 pushConstant = cam->getProjection() * cam->getView();
	vkCmdPushConstants( cmdBuf, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof( glm::mat4x4 ), &pushConstant );

	draw();

	overlay.update( commandBuffers[currentImageIndex] );

	endDraw();

	modelMatrixBuffer.offset = 0;
	dynamicIndexBuffer.offset = 0;
	dynamicVertexBuffer.offset = 0;
}