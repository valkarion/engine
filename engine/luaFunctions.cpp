#include "luaStateController.hpp"
#include "camera.hpp"

Camera* GetCamera()
{
	return Camera::instance();
}

void LuaStateController::luaRegisterFunctions()
{
	state["GetCamera"] = GetCamera;
}