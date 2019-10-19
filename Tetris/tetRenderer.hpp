#pragma once
#include "renderer.hpp"
#include "debugOverlay.hpp"

#include <tuple>

// allocated memory address from vertex and index buffer 
// there are always 4 vertecies and 6 indecies in CCW fashion
using SquareMemoryAddr_t = std::pair<Vertex*, uint32_t*>;

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
	
	SquareMemoryAddr_t allocSquareMemory();
	void			setupSquare( const SquareMemoryAddr_t& memory, uint32_t indexOffset ) const;
		   
	void			childInit() override;
	void			childShutdown() override;

	void			drawFrame() override;
};