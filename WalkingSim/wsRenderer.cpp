#include "wsRenderer.hpp"

#include "camera.hpp"
#include "playerController.hpp"
#include "components.hpp"
#include "entityManager.hpp"

void WsOverlay::update( VkCommandBuffer commandBuffer )
{
	if ( !display )
	{
		return;
	}

	Camera* cam = Camera::instance();
	PlayerController* pc = PlayerController::instance();
	TransformComponent* tc = EntityManager::instance()->get<TransformComponent>( pc->getPlayerId() );
	RigidbodyComponent* rc = EntityManager::instance()->get<RigidbodyComponent>( pc->getPlayerId() );

	ImGui::NewFrame();

	ImGui::SetNextWindowSize( ImVec2( 200.f, 450.f ) );
	ImGui::Begin( "Debug Overlay", nullptr, ImGuiWindowFlags_NoSavedSettings );

	ImGui::Text( "Camera position:\n x:\t%f\n y:\t%f\n z:\t%f",
		cam->getPosition().x, cam->getPosition().y, cam->getPosition().z );

	ImGui::Text( "Camera look at:\n x:\t%f\n y:\t%f\n z:\t%f",
		cam->getDirection().x, cam->getDirection().y, cam->getDirection().z );

	ImGui::Text( "Player position:\n x:\t%f\n y:\t%f\n z:\t%f",
		tc->position.x, tc->position.y, tc->position.z );

	ImGui::Text( "Player facing direction:\n x:\t%f\n y:\t%f\n z:\t%f",
		tc->facingDirection.x, tc->facingDirection.y, tc->facingDirection.z );

	ImGui::Text( "Clipping: %s", ( rc->collidable ) ? "on" : "off" );
	ImGui::Text( "Gravity:  %s", ( rc->affectedByGravity ) ? "affected" : "unaffected" );

	ImGui::Text( "Rotation:\n x:\t%f\n y:\t%f\n z:\t%f",
		tc->rotation.x, tc->rotation.y, tc->rotation.z );

	ImGui::End();

	ImGui::Render();

	if ( checkBuffers() )
	{
		draw( commandBuffer );
	}
}

void WsRenderer::childInit()
{
	overlay.display = true;
	overlay.device = &device;
	overlay.graphicsQueue = graphicsQueue;
	overlay.renderPass = renderPass;
	overlay.init();
}

void WsRenderer::childShutdown()
{
	overlay.shutdown();
}

void WsRenderer::drawFrame()
{
	beginDraw();
	draw();
	overlay.update( commandBuffers[currentImageIndex] );
	endDraw();
}