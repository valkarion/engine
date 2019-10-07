#include <Windows.h>
#include "application.hpp"
#include "tetRenderer.hpp"
#include "luaStateController.hpp"
#include "tetBoard.hpp"

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	Renderer::instance()->setInstanceType<TetRenderer>();
		
	if ( !Application::instance()->init() )
	{
		return -1;
	}

	sol::state& state = LuaStateController::instance()->state;

	state["GetTetrisBoard"] = []() -> Board*
	{
		return Board::instance();
	};

	state.new_usertype<Board>("Board",
		"update", &Board::update );

	LuaStateController::instance()->safeRunScriptFile( "startup.lua" );
	   
	Board::instance()->initialize();

	Application::instance()->run();

	return 0;
}