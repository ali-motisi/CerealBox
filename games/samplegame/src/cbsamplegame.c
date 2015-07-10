#include "../../../common/src/oogame.h"

#include <string.h>
#include <math.h> // TODO remove


typedef struct {
	oofloat x;
	oofloat y;
} ooplayer;

typedef struct {
	oobool initialised;

	OOGameInput*  input;
	OOGameOutput* output;

	oofloat time;
	oofloat tSine;
	ooplayer p1;
	ooplayer p2;

	oofloat moveDelta;
} ootestgame;


void initGame(ootestgame * game) {
	game->initialised = ootrue;
	game->p2.x = 200.f;
}



void outputSine(ootestgame* game) {
	ooshort toneVolume = 4000;
	//oofloat freq = 440 + (game->p1.x-game->p1.y) * 5;
	oofloat freq = 440.f;


	ooint period = (ooint) (44100.f/freq);
	ooshort value;
	oouint k = 0;

	for(oouint i=0; i<game->output->audio.framesToWrite; i++) {
		value = (ooshort)(sinf(game->tSine)*toneVolume);

		game->output->audio.buffer[k++] = value;
		game->output->audio.buffer[k++] = value;

		game->tSine += (2.f*M_PIf)/period;
		if( game->tSine>2.f*M_PIf ) {
			game->tSine -= 2.f*M_PIf;
		}
	}

}

void clearScreen(ootestgame* game) {
	memset(game->output->graphic.buffer, 0, game->output->graphic.width*game->output->graphic.height*sizeof(oouint));
}

void drawRectangle(ootestgame* game, ooint x, ooint y, oouint width, oouint height, oouint colour) {
	// this is a very simple implementation
	// and needs optimisation
	// also alpha blending is missing

	ooint currentX, currentY;
	for(oouint r=0; r<height; r++) {
		currentY = y + r;
		for(oouint c=0; c<width; c++) {
			currentX = x + c;
			if( currentX>=0 && currentX<game->output->graphic.width &&
				currentY>=0 && currentY<game->output->graphic.height ) {
				game->output->graphic.buffer[currentY*game->output->graphic.width+currentX] = colour;
			}
		}
	}
}


void drawPlayer(ootestgame* game, ooplayer* player) {
	ooint playerSize = 50;
	oouint colour = &game->p1==player ? 0xff00ffff : 0xff00ff00;
	drawRectangle(game, (ooint) player->x, (ooint)player->y, playerSize, playerSize, colour);
}

void updateGame(ootestgame * game) {

	oofloat playerSpeed = game->input->dt * 300.f;
	
	if( game->input->keyboard['D'] ) {
		game->p1.x += playerSpeed;
	}
	if( game->input->keyboard['A'] ) {
		game->p1.x -= playerSpeed;
	}
	if( game->input->keyboard['W'] ) {
		game->p1.y -= playerSpeed;
	}
	if( game->input->keyboard['S'] ) {
		game->p1.y += playerSpeed;
	}

	if( game->input->keyboard['L'] ) {
		game->p2.x += playerSpeed;
	}
	if( game->input->keyboard['J'] ) {
		game->p2.x -= playerSpeed;
	}
	if( game->input->keyboard['I'] ) {
		game->p2.y -= playerSpeed;
	}
	if( game->input->keyboard['K'] ) {
		game->p2.y += playerSpeed;
	}

}





oouint64 gameMemorySize() {
	return sizeof(ootestgame); // TODO rename
}


void advanceGame(void* gameMemory, OOGameInput* input, OOGameOutput* output) {
	ootestgame * game = (ootestgame*) gameMemory;

	game->input = input;
	game->output = output;

	if( !game->initialised ) {
		initGame(game);
	}

	game->moveDelta = game->p1.x;
	updateGame(game);
	game->moveDelta -= game->p1.x;

	clearScreen(game);
	
	drawPlayer(game, &game->p2);
	drawPlayer(game, &game->p1);

	// draw a rectangle at the mouse position
	drawRectangle(game, (ooint)input->mouse.x, (ooint)input->mouse.y, 25, 25, 0xffff0000);

	outputSine(game);

	game->time += game->input->dt;
}

