#include "Nxna.h"
#include "Game1.h"

int NxnaMain(int argc, const char* argv[])
{
	Game1* game = new Game1();
	game->Run();
	delete game;


	return 0;
}
