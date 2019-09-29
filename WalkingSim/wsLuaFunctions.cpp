#include "renderer.hpp"
#include "wsRenderer.hpp"
#include "luaStateController.hpp"

void ToggleDebugOverlay()
{
	WsRenderer* r = (WsRenderer*)Renderer::instance();
	r->overlay.display = !r->overlay.display;
}

void RegisterLuaFunctions()
{
	sol::state& s = LuaStateController::instance()->state;

	s["ToggleDebugOverlay"] = ToggleDebugOverlay;
}