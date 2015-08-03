#include "../../../../common/src/oogame.h"

#define OOPlayer1_WidthFract 0.05
#define OONumBullets 5

#define OOTimeToNextBullet 0.5
#define OOTimeToLife 0.5
#define OOTimeToRestart 0.5

#define OOMaxPoints 5

#define OOBgColour 0xff548109
#define OOBorderColour 0xffE4A849


typedef struct {
    ooint x;
    ooint y;
    oouint size;
    oofloat angle;
    oofloat speed;
    oobyte visible;
}OOBullet;

typedef struct {
    ooint x;
    ooint y;
    ooint width;
    ooint height;
    oouint thickness;
}OOBorder;

static ooint num1[] = {
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 1, 1, 1, 0,
    0, 0, 1, 1, 1, 1, 0,
    0, 1, 1, 0, 1, 1, 0,
    1, 1, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0,
    0, 0, 0, 0, 1, 1, 0
};

typedef struct {
    ooint x;
    ooint y;
    ooint width;
    ooint height;
}OONumber;

typedef struct {
    ooint x;
    ooint y;
    oouint size;
    oouint centerSize;
    oouint wheelWidth;
    oouint wheelHeight;
    oouint cannonWidth;
    oouint cannonHeight;
    oouint colour;
    oofloat angle;
    oofloat speed;
    oofloat timeToNextBullet;
    OOBullet bullets[OONumBullets];
    ooint points;
    oofloat timeToLife;
}OOTank;

typedef struct {
	oobool init;
	OOGameInput* input;
	OOGameOutput* output;
    OOBorder border;
    OOTank players[2];
    OONumber points[2];
    oofloat timeToRestart;
    ooint winningPlayer;
} OOGame;


oouint64 gameMemorySize() {
	return sizeof(OOGame);
}

void initGame(OOGame * game) {
    
    game->border.x = game->border.thickness = game->output->graphic.height*0.03;
    game->border.y = game->output->graphic.height*0.2;
    game->border.width = game->output->graphic.width - game->border.thickness*2;
    game->border.height = game->output->graphic.height - game->border.y - game->border.thickness;
    
    game->timeToRestart = -1;
    game->winningPlayer = -1;
    
    for (ooint i=0; i<2; i++ ) {
        game->players[i].angle = 0;
        game->players[i].speed = 0;
        
        game->players[i].size = OOPlayer1_WidthFract*game->output->graphic.width;
        game->players[i].centerSize = game->players[i].size*0.8;
        game->players[i].wheelHeight = game->players[i].centerSize/4;
        game->players[i].wheelWidth = game->players[i].centerSize*1.4;
        game->players[i].cannonHeight = game->players[i].centerSize*0.2;
        game->players[i].cannonWidth = game->players[i].centerSize*0.6;
        game->players[i].colour = 0xffE55E39;
        game->players[i].timeToNextBullet = -1;
        game->players[i].points = 0;
        game->players[i].timeToLife = -1;
        
        for ( ooint b=0; b<OONumBullets; b++ ) {
            game->players[i].bullets[b].visible = oofalse;
            game->players[i].bullets[b].size = game->players[i].centerSize*0.15;
            game->players[i].bullets[b].speed = game->output->graphic.height;
        }
        
        game->points[i].x = game->output->graphic.width/4 + i * game->output->graphic.width/2;
        game->points[i].y = game->output->graphic.height*0.01;
        game->points[i].width = 7;
        game->points[i].height = 16;
    }
    
    game->players[0].x = game->output->graphic.width/4;
    game->players[0].y = game->output->graphic.height/2;
    
    game->players[1].x = game->output->graphic.width/4*3;
    game->players[1].y = game->output->graphic.height/2;
    game->players[1].angle = 180;
    game->players[1].colour = 0xff4A80BE;
    
	game->init = ootrue;
}

oofloat degToRad(oofloat deg) {
 
    return deg/180*M_PIf;
}

void drawRectangle(OOGame* game, ooint x, ooint y, oouint width, oouint height, oouint colour) {
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

void drawRotatedRectangle(OOGame* game, ooint x, ooint y, oouint width, oouint height, oouint colour, oofloat degrees, ooint rotationCenterX, ooint rotationCenterY) {
    
    oofloat rad = degToRad(degrees);

    ooint currentX, currentY;
    ooint rotatedX, rotatedY;
    for(oouint r=0; r<height; r++) {
        currentY = y + r;
        for(oouint c=0; c<width; c++) {
            currentX = x + c;
            
            rotatedX = currentX*cos(rad) - currentY*sin(rad) + rotationCenterX;
            rotatedY = currentX*sin(rad) + currentY*cos(rad) + rotationCenterY;
            
            if( rotatedX>=0 && rotatedX<game->output->graphic.width &&
               rotatedY>=0 && rotatedY<game->output->graphic.height ) {
                game->output->graphic.buffer[rotatedY*game->output->graphic.width+rotatedX] = colour;
            }
        }
    }
}

void drawShape(OOGame* game, ooint x, ooint y, oouint width, oouint height, oouint colour, ooint *mask) {
    
    ooint currentX, currentY;
    for(oouint r=0; r<height; r++) {
        currentY = y + r;
        for(oouint c=0; c<width; c++) {
            currentX = x + c;
            if( currentX>=0 && currentX<game->output->graphic.width &&
               currentY>=0 && currentY<game->output->graphic.height ) {
                game->output->graphic.buffer[currentY*game->output->graphic.width+currentX] = colour; //*mask[r*width + c];
                
            }
        }
    }
}

void drawTank(OOGame* game, OOTank *tank) {
    
    oouint colour = tank->colour;
    if ( tank->timeToLife >= 0 ) {
        colour *= (OOTimeToLife - tank->timeToLife)/OOTimeToLife;
    }
    
    drawRotatedRectangle(game, tank->centerSize/2*-1, tank->centerSize/2*-1, tank->centerSize, tank->centerSize, colour, tank->angle, tank->x, tank->y);
    
    drawRotatedRectangle(game, 0 - tank->wheelWidth/2, 0 - tank->centerSize/2 - tank->wheelHeight, tank->wheelWidth, tank->wheelHeight, colour, tank->angle, tank->x, tank->y);
    drawRotatedRectangle(game, 0 - tank->wheelWidth/2, tank->centerSize/2, tank->wheelWidth, tank->wheelHeight, colour, tank->angle, tank->x, tank->y);

    drawRotatedRectangle(game, tank->centerSize/2, 0 - tank->cannonHeight/2, tank->cannonWidth, tank->cannonHeight, colour, tank->angle, tank->x, tank->y);

    
    for ( ooint i=0; i < OONumBullets; i++ ) {
        if ( tank->bullets[i].visible ) {
            drawRectangle(game, tank->bullets[i].x - tank->bullets[i].size/2, tank->bullets[i].y - tank->bullets[i].size/2, tank->bullets[i].size, tank->bullets[i].size, 0xffffff00);
        }
    }
}

oobool forwardForPlayerIndex(ooint playerIndex, OOGameInput* input) {
    
    if ( playerIndex == 0 ) {
        if ( input->keyboard['w'] || input->keyboard['W'] ){
            return ootrue;
        }
    } else if ( playerIndex == 1 ) {
        if ( input->keyboard['u'] || input->keyboard['U'] ){
            return ootrue;
        }
    }
    return oofalse;
}

oobool rotateClockwise(ooint playerIndex, OOGameInput* input) {
    
    if ( playerIndex == 0 ) {
        if ( input->keyboard['d'] || input->keyboard['D'] ){
            input->keyboard['d'] = input->keyboard['D'] = oofalse;
            return ootrue;
        }
    } else if ( playerIndex == 1 ) {
        if ( input->keyboard['k'] || input->keyboard['K'] ){
            input->keyboard['k'] = input->keyboard['K'] = oofalse;
            return ootrue;
        }
    }
    return oofalse;
}

oobool rotateCounterClockwise(ooint playerIndex, OOGameInput* input) {
    
    if ( playerIndex == 0 ) {
        if ( input->keyboard['a'] || input->keyboard['A'] ){
            input->keyboard['a'] = input->keyboard['A'] = oofalse;
            return ootrue;
        }
    } else if ( playerIndex == 1 ) {
        if ( input->keyboard['h'] || input->keyboard['H'] ){
            input->keyboard['h'] = input->keyboard['H'] = oofalse;
            return ootrue;
        }
    }
    return oofalse;
}

oobool shootPlayerIndex(ooint playerIndex, OOGameInput* input) {
 
    if ( playerIndex == 0 ) {
        if ( input->keyboard['s'] || input->keyboard['S'] ){
            input->keyboard['s'] = input->keyboard['S'] = oofalse;
            return ootrue;
        }
    } else if ( playerIndex == 1 ) {
        if ( input->keyboard['j'] || input->keyboard['J'] ){
            input->keyboard['j'] = input->keyboard['J'] = oofalse;
            return ootrue;
        }
    }
   return oofalse;
}

oobool inBounds(oofloat x, oofloat y, oofloat boundX, oofloat boundY, oofloat boundWidth, oofloat boundHeight) {
    
    if ( x < boundX || x > boundX + boundWidth || y < boundY || y > boundY + boundHeight ) {
        return oofalse;
    }
    return ootrue;
}

oobool inBorder(oofloat x, oofloat y, OOGame* game) {
    
    return inBounds(x, y, game->border.x, game->border.y, game->border.width, game->border.height);
}

void updatePlayer(ooint playerIndex, OOGame * game, OOGameInput* input, OOGameOutput* output) {
    
    if ( game->players[playerIndex].timeToNextBullet >= 0 ) {
        game->players[playerIndex].timeToNextBullet -= game->input->dt;
    }
    if ( game->players[playerIndex].timeToLife >= 0 ) {
        game->players[playerIndex].timeToLife -= game->input->dt;
    }
    
    
    game->players[playerIndex].speed = 0;
    if ( forwardForPlayerIndex(playerIndex, input) ) {
        game->players[playerIndex].speed = output->graphic.height*0.5;
    }
    
    oofloat dAngle = 10;
    if ( game->players[playerIndex].timeToLife >= 0 ) {
        dAngle = 360 * (OOTimeToLife - game->players[playerIndex].timeToLife);
    }
    if ( rotateCounterClockwise(playerIndex, input) || game->players[playerIndex].timeToLife >= 0) {
        game->players[playerIndex].angle -= dAngle;
    } else if ( rotateClockwise(playerIndex, input) ){
        game->players[playerIndex].angle += dAngle;
    }
    if ( game->players[playerIndex].angle > 180 ) {
        game->players[playerIndex].angle = -180 + game->players[playerIndex].angle - 180;
    } else if ( game->players[playerIndex].angle < -180 ) {
        game->players[playerIndex].angle = 180 - (-game->players[playerIndex].angle - 180);
    }
    
    oouint dx = input->dt*game->players[playerIndex].speed*cos(degToRad(game->players[playerIndex].angle));
    oouint dy = input->dt*game->players[playerIndex].speed*sin(degToRad(game->players[playerIndex].angle));
    if ( inBorder(game->players[playerIndex].x + dx - game->players[playerIndex].size/2, game->players[playerIndex].y + dy - game->players[playerIndex].size/2, game)
        && inBorder(game->players[playerIndex].x + dx + game->players[playerIndex].size/2, game->players[playerIndex].y + dy + game->players[playerIndex].size/2, game)) {
        game->players[playerIndex].x += dx;
        game->players[playerIndex].y += dy;
    }
    
}



void updateBullets(ooint playerIndex, OOGame * game, OOGameInput* input, OOGameOutput* output) {

    if ( shootPlayerIndex(playerIndex, input) && game->players[playerIndex].timeToNextBullet < 0 ) {
        for ( ooint i = 0; i < OONumBullets; i++ ) {
            if ( !game->players[playerIndex].bullets[i].visible ) {
                game->players[playerIndex].bullets[i].angle = game->players[playerIndex].angle;
                game->players[playerIndex].bullets[i].x = game->players[playerIndex].x;
                game->players[playerIndex].bullets[i].y = game->players[playerIndex].y;
                game->players[playerIndex].bullets[i].visible = ootrue;
                game->players[playerIndex].timeToNextBullet = OOTimeToNextBullet;
                break;
            }
        }
    }
    
    oouint dx, dy;
    ooint opponentIndex = (playerIndex+1)%2;
    for ( ooint i = 0; i < OONumBullets; i++ ) {
        if ( game->players[playerIndex].bullets[i].visible ) {
            dx = input->dt*game->players[playerIndex].bullets[i].speed*cos(degToRad(game->players[playerIndex].angle));
            dy = input->dt*game->players[playerIndex].bullets[i].speed*sin(degToRad(game->players[playerIndex].angle));
            game->players[playerIndex].bullets[i].x += dx;
            game->players[playerIndex].bullets[i].y += dy;
            if ( !inBorder(game->players[playerIndex].bullets[i].x, game->players[playerIndex].bullets[i].y, game) ) {
                game->players[playerIndex].bullets[i].visible = oofalse;
            } else {
                if ( inBounds(game->players[playerIndex].bullets[i].x, game->players[playerIndex].bullets[i].y,
                              game->players[opponentIndex].x-game->players[opponentIndex].size/2, game->players[opponentIndex].y-game->players[opponentIndex].size/2, game->players[opponentIndex].size, game->players[opponentIndex].size) ) {
                    game->players[playerIndex].bullets[i].visible = oofalse;
                    game->players[playerIndex].points ++;
                    game->players[opponentIndex].timeToLife = OOTimeToLife;
                    if ( game->players[playerIndex].points >= OOMaxPoints ) {
                        game->winningPlayer = playerIndex;
                        game->timeToRestart = OOTimeToRestart;
                    }
                }
            }
        }
    }
}

void clearScreen(OOGame *game) {
	memset(game->output->graphic.buffer,0,game->output->graphic.width*game->output->graphic.height*sizeof(oouint));
}

void drawBg(OOGame *game, oouint colour) {
    drawRectangle(game, 0, 0, game->output->graphic.width,  game->output->graphic.height, colour);
}

void drawBorder(OOGame *game) {
    drawRectangle(game, game->border.x - game->border.thickness, game->border.y - game->border.thickness, game->border.width + game->border.thickness*2,  game->border.thickness, OOBorderColour);
    drawRectangle(game, game->border.x - game->border.thickness, game->border.y + game->border.height, game->border.width + game->border.thickness*2,  game->border.thickness, OOBorderColour);
    drawRectangle(game, game->border.x - game->border.thickness, game->border.y, game->border.thickness,  game->border.height, OOBorderColour);
    drawRectangle(game, game->border.x + game->border.width, game->border.y, game->border.thickness,  game->border.height, OOBorderColour);
}

void drawPoints(OOGame *game) {
    
    for ( int i=0; i<2; i++ ) {
        drawShape(game, game->points[i].x, game->points[i].y, game->points[i].width, game->points[i].height, OOBorderColour, num1);
    }
}

void advanceGame(void* gameMemory, OOGameInput* input, OOGameOutput* output) {
	OOGame * game = (OOGame*) gameMemory;

	game->input = input;
	game->output = output;

	if( !game->init) {
		initGame(game);
	}
    
    oouint bgColour = OOBgColour;
    if ( game->winningPlayer >= 0 && game->timeToRestart >= 0 ) {
        game->timeToRestart -= game->input->dt;
        bgColour = game->players[game->winningPlayer].colour;
    } else if ( game->winningPlayer >= 0 ) {
        initGame(game);
    }

	clearScreen(game);
    drawBg(game, bgColour);
    drawBorder(game);
    
    updatePlayer(0, game, input, output);
    updatePlayer(1, game, input, output);
    
    updateBullets(0, game, input, output);
    updateBullets(1, game, input, output);

    drawTank(game, &game->players[0]);
    drawTank(game, &game->players[1]);
    
//    drawPoints(game);
    
}
