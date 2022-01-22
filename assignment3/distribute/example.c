
#include <string.h>
#include <unistd.h>

#include "console.h"
#include "example.h"

#define GAME_ROWS 24
#define GAME_COLS 80

/**** DIMENSIONS MUST MATCH the ROWS/COLS */
char *GAME_BOARD[] = {
"                   Score:          Lives:",
"=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-centipiede!=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"",
"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"\"",
"",
"",
"",
"",
"",
"", 
"" };


#define ENEMY_HEIGHT 2
#define ENEMY_BODY_ANIM_TILES 4 
char* ENEMY_BODY[ENEMY_BODY_ANIM_TILES][ENEMY_HEIGHT] = 
{
  {"1",
   "1"},
  {"2",
   "2"},
  {"3",
   "3"},
  {"4",
   "4"}
};


void exampleRun()
{
	if (consoleInit(GAME_ROWS, GAME_COLS, GAME_BOARD))
  {
		for (int i = 0; i<ENEMY_BODY_ANIM_TILES; i++)
		{
			char** tile = ENEMY_BODY[i];

			consoleClearImage(10,10,ENEMY_HEIGHT, strlen(tile[0]));
			consoleDrawImage(10, 10, tile, ENEMY_HEIGHT);
			consoleRefresh();
			sleep(1);
		}		

  finalKeypress(); /* wait for final key before killing curses and game */
  }       
  consoleFinish();        	
}

