#pragma once
#include "renderer.hpp"
#include "debugOverlay.hpp"

class TetOverlay : public DebugOverlay
{
public:
	void update( VkCommandBuffer commandBuffer ) override;
};

class TetRenderer : public Renderer
{
	void		draw() override;
public:
	TetOverlay	overlay;

	void		childInit() override;
	void		childShutdown() override;

	void		drawFrame() override;
};