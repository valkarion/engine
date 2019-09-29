#pragma once

#include "renderer.hpp"

/*
	The render of the walking simulator will provide a debug overlay next
	to the default functionalities.
*/
class WsRenderer : public Renderer
{
public:
	void childInit() override;
	void childShutdown() override;
};