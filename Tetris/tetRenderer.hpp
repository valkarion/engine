#pragma once
#include "renderer.hpp"
#include "debugOverlay.hpp"
#include <glm/glm.hpp>

struct Cell;
class Board;

// allocated memory address from vertex and index buffer 
// there are always 4 vertecies and 6 indecies in CCW fashion
// idxOffset is the starting index buffer offset position
struct SquareMemInfo
{
	Vertex*		vertAddr;
	uint32_t*	idxAddr;
	uint32_t	idxOffset;
};

class TetOverlay : public DebugOverlay
{
public:
	void update( VkCommandBuffer commandBuffer ) override;
};

class TetRenderer : public Renderer
{
	void			draw() override;
public:
	TetOverlay		overlay;

	VulkanBuffer	dynamicVertexBuffer;
		
	void			drawSingleCell( const VkCommandBuffer cmdBuf,
		const glm::vec3& position, const VkDescriptorSet dSet,
		const glm::vec2 scale = glm::vec2( 1.f, 1.f ) );

	void			drawBackground( VkCommandBuffer cmdBuf, Board* board );
	void			drawCells( VkCommandBuffer cmdBuf, Board* board );
	void			drawCurrentBlock( VkCommandBuffer cmdBuf, Board* board );
		
	SquareMemInfo	allocSquareMemory();
	void			setupSquare( const SquareMemInfo& memory ) const;
		   
	void			childInit() override;
	void			childShutdown() override;

	void			drawFrame() override;
};