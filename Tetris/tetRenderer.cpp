#include "tetRenderer.hpp"

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

}

void TetRenderer::drawFrame()
{
	beginDraw();
	draw();
	overlay.update( commandBuffers[currentImageIndex] );
	endDraw();
}