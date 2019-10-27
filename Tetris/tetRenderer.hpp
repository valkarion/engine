#pragma once
#include "renderer.hpp"
#include "debugOverlay.hpp"

#include <glm/glm.hpp>
#include <tuple>

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
	
	SquareMemInfo	allocSquareMemory();
	
	void			drawBackground( VkCommandBuffer cmdBuf, Board* board );
	void			drawCells( VkCommandBuffer cmdBuf, Board* board );
	void			drawCurrentBlock( VkCommandBuffer cmdBuf, Board* board );

	// sets up the buffer memory of the given square 
	void			setupSquare( const SquareMemInfo& memory ) const;
		   
	void			childInit() override;
	void			childShutdown() override;

	void			drawFrame() override;
};