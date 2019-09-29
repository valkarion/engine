#pragma once

#include "renderer.hpp"
#include "debugOverlay.hpp"

class WsOverlay : public DebugOverlay
{
public:
	void update( VkCommandBuffer commandBuffer ) override;
};

/*
	The render of the walking simulator will provide a debug overlay next
	to the default functionalities.
*/
class WsRenderer : public Renderer
{
public:
	WsOverlay	overlay;

	void		childInit() override;
	void		childShutdown() override;

	void		drawFrame() override;
};