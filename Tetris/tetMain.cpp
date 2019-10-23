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
		"update", &Board::update );
}

void FillBoardRandomly( Board* board )
{
	const size_t board_height = board->field.size();
	const size_t board_width = board->field[0].size();

	for ( size_t y = 0; y < board_height; y++ )
	{
		for ( size_t x = 0; x < board_width; x++ )
		{
			Cell& c = board->getCell( x, y );

			c.hasEntity = true;
			c.textureIndex = rand() % (int)enu_BLOCK_TYPE::size;
		}
	}
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

	FillBoardRandomly( Board::instance() );

	Application::instance()->run();

	return 0;
}