#include "luaStateController.hpp"
#include "camera.hpp"

Camera* GetCamera()
{
	return Camera::instance();
}

void LuaStateController::registerFunctions()
{
	state["GetCamera"] = GetCamera;
}