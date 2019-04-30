#include "tests.hpp"
#include "application.hpp"
#include "inputSystem.hpp"
#include "camera.hpp"
#include <glm/glm.hpp>

extern Camera camera;

void AddInputCommands()
{
	const float cameraSens = 0.01f;
	InputSystem* is = InputSystem::instance();

	is->addKeyboardFunction( GLFW_KEY_D, [&, cameraSens]()
	{
		glm::vec3 right = glm::normalize( glm::cross( camera.position - camera.direction, camera.up ) );
		camera.displace( right * -cameraSens );
	} );

	is->addKeyboardFunction( GLFW_KEY_A, [&, cameraSens]()
	{
		glm::vec3 right = glm::normalize( glm::cross( camera.position - camera.direction, camera.up ) );
		camera.displace( right * cameraSens );
	} );

	is->addKeyboardFunction( GLFW_KEY_W, [&, cameraSens]()
	{
		camera.displace( glm::normalize( camera.position - camera.direction ) * -cameraSens );
	} );

	is->addKeyboardFunction( GLFW_KEY_S, [&, cameraSens]()
	{
		camera.displace( glm::normalize( camera.position - camera.direction ) * cameraSens );
	} );
}

void T_Renderer()
{
	if( !Application::instance()->init() )
	{		
		exit( -1 );
	}

	camera.initCamera();
	AddInputCommands();

	Application::instance()->run();
}