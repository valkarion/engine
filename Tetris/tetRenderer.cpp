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

SquareMemInfo TetRenderer::allocSquareMemory()
{
	const uint32_t vertexCountPerSquare = 4;
	const uint32_t indexCountPerSquare = 6;

	SquareMemInfo sma;

	Vertex* vertecies = (Vertex*)dynamicVertexBuffer.allocate( 
		vertexCountPerSquare * sizeof( Vertex ) );
	
	uint32_t offset = dynamicIndexBuffer.offset / sizeof( uint32_t );
	
	uint32_t* indecies = (uint32_t*)dynamicIndexBuffer.allocate( 
		indexCountPerSquare * sizeof( uint32_t ) );

	sma.idxAddr	= indecies;
	sma.idxOffset = offset;
	sma.vertAddr = vertecies;

	return sma;
}

void TetRenderer::setupSquare( const SquareMemInfo& memory ) const
{
	memory.vertAddr[0].position = glm::vec3( -0.5f , -0.5f , 0.f );
	memory.vertAddr[0].color = glm::vec3( 1.f, 1.f, 1.f );
	memory.vertAddr[0].textureCoordinates = glm::vec2( 0.f, 0.f );

	memory.vertAddr[1].position = glm::vec3( 0.5f , -0.5f , 0.f );
	memory.vertAddr[1].color = glm::vec3( 1.f, 1.f, 1.f );
	memory.vertAddr[1].textureCoordinates = glm::vec2( 1.f, 0.f );

	memory.vertAddr[2].position = glm::vec3( 0.5f , 0.5f , 0.f );
	memory.vertAddr[2].color = glm::vec3( 1.f, 1.f, 1.f );
	memory.vertAddr[2].textureCoordinates = glm::vec2( 1.f, 1.f );

	memory.vertAddr[3].position = glm::vec3( -0.5f , 0.5f , 0.f );
	memory.vertAddr[3].color = glm::vec3( 1.f, 1.f, 1.f );
	memory.vertAddr[3].textureCoordinates = glm::vec2( 0.f, 1.f );

	memory.idxAddr[0] = memory.idxOffset + 0;
	memory.idxAddr[1] = memory.idxOffset + 1;
	memory.idxAddr[2] = memory.idxOffset + 2;
	memory.idxAddr[3] = memory.idxOffset + 2;
	memory.idxAddr[4] = memory.idxOffset + 3;
	memory.idxAddr[5] = memory.idxOffset + 0;
}

std::string TexIndexToTexName( const int index )
{
	static std::array<std::string, (size_t)enu_BLOCK_TYPE::size> texNames = {
		"orange", "brown", "pink", "yellow", "blue", "green", "red"	};

	return texNames[index];
}

void TetRenderer::drawBackground( VkCommandBuffer cmdBuf, Board* board )
{
	vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );

	VkDeviceSize offset = dynamicIndexBuffer.offset;

	setupSquare( allocSquareMemory() );

	vkCmdBindIndexBuffer( cmdBuf, dynamicIndexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32 );
	
	glm::mat4x4* model = ( glm::mat4x4* )modelMatrixBuffer.allocate( sizeof( glm::mat4x4 ) );
	*model = glm::translate( glm::mat4x4( 1.f ), glm::vec3( board->width / 2, board->height / 2, -1.f ) );
	*model = glm::scale( *model, glm::vec3( (float)board->width + 2.5f, (float)board->height + 2.5f, 0.f ) );

	const VulkanTexture* tex = getTexture( "gray" );
	vkCmdBindDescriptorSets( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout, 0, 1, &tex->descriptor, 0, nullptr );

	vkCmdDrawIndexed( cmdBuf, 6, 1, 0, 0, 0 );
}

void TetRenderer::drawCells( VkCommandBuffer cmdBuf, Board* board )
{
	uint32_t currentOffset = dynamicIndexBuffer.offset / sizeof( uint32_t );

	int nEntities = 0;
	vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );

	for ( int y = 0; y < board->height; y++ )
	{
		for ( int x = 0; x < board->width; x++ )
		{
			Cell& c = board->getCell( x, y );
			if ( c.hasEntity )
			{
				setupSquare( allocSquareMemory() );

				glm::mat4x4* model = ( glm::mat4x4* )modelMatrixBuffer.allocate( sizeof( glm::mat4x4 ) );
				*model = glm::translate(
					glm::mat4x4( 1.f ),
					glm::vec3( float( x + board->cBlock.px ),
						float( y + board->cBlock.py ), 0.f )
				);

				const VulkanTexture* tex = getTexture( TexIndexToTexName( c.textureIndex ) );
				vkCmdBindDescriptorSets( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
					pipelineLayout, 0, 1, &tex->descriptor, 0, nullptr );

				nEntities++;
			}
		}
	}

	vkCmdDrawIndexed( cmdBuf, 6 * nEntities, nEntities, currentOffset, 0, 0 );
}

void TetRenderer::drawCurrentBlock( VkCommandBuffer cmdBuf, Board* board )
{
	VkDeviceSize offset = dynamicIndexBuffer.offset;

	vkCmdBindPipeline( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline );
	
	for ( size_t y = 0; y < 4; y++ )
	{
		for ( size_t x = 0; x < 4; x++ )
		{
			if ( board->cBlock.getCell( x, y ) == CELL_FILLED )
			{
				setupSquare( allocSquareMemory() );

				glm::mat4x4* model = ( glm::mat4x4* )modelMatrixBuffer.allocate( sizeof( glm::mat4x4 ) );
				*model = glm::translate( 
					glm::mat4x4(1.f), 
					glm::vec3(	float( x + board->cBlock.px ),
								float( y + board->cBlock.py ), 0.f ) 
				);
			}
		}
	}
	
	const VulkanTexture* tex = getTexture( TexIndexToTexName( 0 ) );
	vkCmdBindDescriptorSets( cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout, 0, 1, &tex->descriptor, 0, nullptr );

	vkCmdBindIndexBuffer( cmdBuf, dynamicIndexBuffer.buffer, offset, VK_INDEX_TYPE_UINT32 );
	vkCmdDrawIndexed( cmdBuf, 6 * 4, 4, 0, 0, 0 );
}

void TetRenderer::draw()
{
	Camera* cam = Camera::instance();	
	VkCommandBuffer cmdBuf = commandBuffers[currentImageIndex];

	Board* board = Board::instance();

	// set the camera to the center of the board, and back enough to see it all 
	cam->setPosition( glm::vec3(
		float( board->width / 2 ),
		float( board->height / 2 ),
		(float)board->height / 1.5f )
	);

	cam->setDirection( glm::vec3( 0.f, 0.f, -1.f ) );

	// push the static camera data into the shader data.
	glm::mat4x4 pushConstant = cam->getProjection() * cam->getView();
	vkCmdPushConstants( cmdBuf, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT,
		0, sizeof( glm::mat4x4 ), &pushConstant );
	
	VkDeviceSize offsets[] = { 0 };	
	vkCmdBindVertexBuffers( cmdBuf, 1, 1, &modelMatrixBuffer.buffer, offsets );
	vkCmdBindVertexBuffers( cmdBuf, 0, 1, &dynamicVertexBuffer.buffer, offsets );
	vkCmdBindIndexBuffer( cmdBuf, dynamicIndexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32 );
	
	drawBackground( cmdBuf, board );
	//drawCells( cmdBuf, board );	
	drawCurrentBlock( cmdBuf, board );
	
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