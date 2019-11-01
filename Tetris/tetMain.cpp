#include <Windows.h>
#include "application.hpp"
#include "tetRenderer.hpp"
#include "luaStateController.hpp"
#include "tetBoard.hpp"

void SetupLuaStuff( sol::state& state )
{
	state["GetTetrisBoard"] = []() -> Board*
	{
		return Board::instance();
	};

	state.new_usertype<Board>( "Board",
		"update", &Board::update,
		"moveLeft", &Board::shiftLeft,
		"moveRight", &Board::shiftRight,
		"rotate", &Board::rotateBlock,
		"moveDown", &Board::shiftDown,
		"save", &Board::save,
		"load", &Board::load );
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	Renderer::instance()->setInstanceType<TetRenderer>();
		
	if ( !Application::instance()->init() )
	{
		return -1;
	}
	
	SetupLuaStuff( LuaStateController::instance()->state );

	LuaStateController::instance()->safeRunScriptFile( "startup.lua" );
	   
	Board::instance()->initialize();
	
	Board::instance()->trySpawnBlock();

	Application::instance()->run();

	return 0;
}