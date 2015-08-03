#include "../../../../common/src/oogame.h"

#define oomax(a,b) ((a) > (b) ? (a) : (b))
#define oomin(a,b) ((a) < (b) ? (a) : (b))

#define CG_BG_GRID_WIDTH 48
#define CG_BG_GRID_HEIGHT 27

#define CG_TANK_SHAPE_WIDTH 9
#define CG_TANK_SHAPE_HEIGHT 7

#define CG_LCD_WIDTH 3
#define CG_LCD_HEIGHT 5

#define CG_LCD_NUMBERS 10

typedef struct {
	oobyte lcd[CG_LCD_WIDTH*CG_LCD_HEIGHT];
} oolcddigit;

typedef struct {
	oofloat x;
	oofloat y;
	oofloat angle;
	oobool visible;
} oobullet;

typedef struct {
	oofloat x;
	oofloat y;
	oofloat angle; // in radiants
	oobyte numberOfHits;

	oouint colour;
	oobullet bullet;

	oobyte keyLeft;
	oobyte keyRight;
	oobyte keyForward;
	oobyte keyFire;
	oobyte gamePadIndex;

} ootank;

typedef struct {
	oobool init;
	OOGameInput* input;
	OOGameOutput* output;
	oofloat time;

	ooint fatPixelWidth;
	ooint fatPixelHeight;
	oobyte bgGrid[CG_BG_GRID_WIDTH*CG_BG_GRID_HEIGHT];
	ooint smallPixelWidth;
	ooint smallPixelHeight;

	ootank tank1;
	ootank tank2;

	oobyte tankShape[CG_TANK_SHAPE_WIDTH*CG_TANK_SHAPE_HEIGHT];
	oolcddigit numbers[CG_LCD_NUMBERS];

} oogame;


oouint64 gameMemorySize() {
	return sizeof(oogame);
}

ooinline getGridFatPixelValue(oogame* g, ooint x, ooint y) {
	if( x<0 || y<0 || x>=CG_BG_GRID_WIDTH || y>=CG_BG_GRID_HEIGHT ) {
		return 0;
	}
	return g->bgGrid[y*CG_BG_GRID_WIDTH+x];
}

ooinline void setGridFatPixel(oogame*g, ooint x, ooint y, oobyte value) {
	g->bgGrid[y*CG_BG_GRID_WIDTH+x] = value;
}

void initBGGrid(oogame* g) {
	for(ooint i=0; i<(CG_BG_GRID_WIDTH*CG_BG_GRID_HEIGHT); i++) {
		g->bgGrid[i] = 0;
	}

	oobyte shape[] = {
		0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 
		0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0, 
																					     
																					     
																					     
		1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,     1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,     1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,     1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   1, 1, 1, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 1,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 1, 1,   1, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 1,   1, 1, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 1, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 1, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 1,   1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,   1, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 1, 
																					     
		1, 0, 0, 0, 0, 0,   1, 0, 0, 0, 1, 1,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   1, 1, 0, 0, 0, 1,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   1, 0, 0, 0, 1, 1,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   1, 1, 0, 0, 0, 1,   0, 0, 0, 0, 0, 1, 
		
		1, 0, 0, 0, 0, 0,   1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 1,   1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,   1, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 1, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 1, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 1, 1,   1, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 1,   1, 1, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   1, 1, 1, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 1, 1, 1,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,     0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,     1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1,     1, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 0,   0, 0, 0, 0, 0, 1, 
		1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,     1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1,   1, 1, 1, 1, 1, 1, 
	};
	for(ooint i=0; i<(CG_BG_GRID_WIDTH*CG_BG_GRID_HEIGHT); i++) {
		//g->tankShape[i] = shape[i];
		g->bgGrid[i] = shape[i];
	}


	/*
	ooint topPadding = 4;
	for(ooint x=0; x<CG_BG_GRID_WIDTH; x++) {
		setGridFatPixel(g, x, topPadding, 1);
		setGridFatPixel(g, x, CG_BG_GRID_HEIGHT-1, 1);
	}
	for(ooint y=topPadding; y<CG_BG_GRID_HEIGHT; y++) {
		setGridFatPixel(g, 0, y, 1);
		setGridFatPixel(g, CG_BG_GRID_WIDTH-1, y, 1);
	}
	*/
}

void initTankShape(oogame* g) {
	oobyte shape[] = {
		0, 1, 1, 1, 1, 1, 1, 0, 0,
		0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 0, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 1, 1, 1, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 0, 0, 0,
		0, 1, 1, 1, 1, 1, 1, 0, 0,
	};
	for(ooint i=0; i<(CG_TANK_SHAPE_WIDTH*CG_TANK_SHAPE_HEIGHT); i++) {
		g->tankShape[i] = shape[i];
	}

}

void setTank1InitialPositions(oogame* g) {
	g->tank1.x = g->output->graphic.width*0.05f;
	g->tank1.y = 15.f*g->fatPixelHeight;
	g->tank1.angle = 0.f;
	g->tank1.bullet.visible = oofalse;
}

void setTank2InitialPositions(oogame* g) {
	g->tank2.x = g->output->graphic.width*0.95f;
	g->tank2.y = 15.f*g->fatPixelHeight;
	g->tank2.bullet.visible = oofalse;
	g->tank2.angle = -M_PIf;
}

void setTankInitialPositions(oogame* g, ootank* t) {
	if(t==&g->tank1) {
		setTank1InitialPositions(g);
	}
	else {
		setTank2InitialPositions(g);
	}
}


void setInitialPositions(oogame* g) {
	setTank1InitialPositions(g);
	setTank2InitialPositions(g);
}


void restartGame(oogame* g) {
	setInitialPositions(g);
	g->tank1.numberOfHits = 0;
	g->tank2.numberOfHits = 0;
}

void setLCDNumber(oolcddigit* digit, oobyte* shape) {
	ooint size = CG_LCD_WIDTH*CG_LCD_HEIGHT;
	for(ooint i=0; i<size; i++) {
		digit->lcd[i] = shape[i];
	}
}

void initNumbers(oogame* g) {
	oobyte shape0[] = {
		1, 1, 1,
		1, 0, 1,
		1, 0, 1,
		1, 0, 1,
		1, 1, 1,
	};
	setLCDNumber(&g->numbers[0], shape0);

	oobyte shape1[] = {
		0, 1, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
	};
	setLCDNumber(&g->numbers[1], shape1);

	oobyte shape2[] = {
		1, 1, 1,
		0, 0, 1,
		1, 1, 1,
		1, 0, 0,
		1, 1, 1,
	};
	setLCDNumber(&g->numbers[2], shape2);

	oobyte shape3[] = {
		1, 1, 1,
		0, 0, 1,
		1, 1, 1,
		0, 0, 1,
		1, 1, 1,
	};
	setLCDNumber(&g->numbers[3], shape3);

	oobyte shape4[] = {
		1, 0, 1,
		1, 0, 1,
		1, 1, 1,
		0, 0, 1,
		0, 0, 1,
	};
	setLCDNumber(&g->numbers[4], shape4);

	oobyte shape5[] = {
		1, 1, 1,
		1, 0, 0,
		1, 1, 1,
		0, 0, 1,
		1, 1, 1,
	};
	setLCDNumber(&g->numbers[5], shape5);
	
}

void initGame(oogame* g) {
	g->fatPixelWidth = (ooint)((g->output->graphic.width)/CG_BG_GRID_WIDTH);
	if(g->fatPixelWidth<0) {
		g->fatPixelWidth = 1;
	}
	g->fatPixelHeight = (ooint)(g->output->graphic.height/CG_BG_GRID_HEIGHT);
	if(g->fatPixelHeight<0) {
		g->fatPixelHeight = 1;
	}

	g->smallPixelWidth = g->fatPixelWidth/4;
	if(g->smallPixelWidth <0) {
		g->smallPixelWidth = 1;
	}
	g->smallPixelHeight = g->fatPixelHeight/4;
	if(g->smallPixelHeight<0) {
		g->smallPixelHeight = 1;
	}

	initBGGrid(g);
	initTankShape(g);
	initNumbers(g);


	g->tank1.keyLeft = 'A';
	g->tank1.keyRight = 'D';
	g->tank1.keyForward = 'W';
	g->tank1.keyFire = 'S';
	g->tank1.colour = 0xff0000ff;
	g->tank1.gamePadIndex = 0;

	g->tank2.keyLeft = 'J';
	g->tank2.keyRight = 'L';
	g->tank2.keyForward = 'I';
	g->tank2.keyFire = 'K';	
	g->tank2.colour = 0xff00ff00;
	g->tank2.gamePadIndex = 1;

	setInitialPositions(g);

	g->init = ootrue;
}

void drawRectangle(oogame* g, ooint x, ooint y, oouint width, oouint height, oouint colour) {
	ooint x2 = x+width-1;
	ooint y2 = y+height-1;

	if( x2<0 || y2<0 || x>=g->output->graphic.width || y>=g->output->graphic.height ) {
		// clip
		return;
	}

	ooint minX = oomax(0, x);
	ooint maxX = oomin(g->output->graphic.width-1, x2);
	
	ooint minY = oomax(0, y);
	ooint maxY = oomin(g->output->graphic.height-1, y2);


	oobool useMemCopy = oofalse;
	for(y=minY; y<=maxY; y++) {
		if( useMemCopy ) {
			memcpy(&g->output->graphic.buffer[y*g->output->graphic.width+minX],
					&g->output->graphic.buffer[minY*g->output->graphic.width+minX],
					(maxX-minX+1)*4);
		}
		else {
			for(x=minX; x<=maxX; x++) {
				g->output->graphic.buffer[y*g->output->graphic.width+x] = colour;
			}
			useMemCopy = ootrue;
		}

	}
}

void drawRectangleFromCentre(oogame* g, ooint xCenter, ooint yCenter, oouint width, oouint height, oouint colour) {
	ooint x = xCenter-(width>>2);
	ooint y = yCenter-(height>>2);
	drawRectangle(g, x, y, width, height, colour);
}

void drawSquareFromCentre(oogame* g, ooint xCenter, ooint yCenter, oouint size, oouint colour) {
	drawRectangleFromCentre(g, xCenter, yCenter, size, size, colour);
}

void clearScreen(oogame* g) {
	memset(g->output->graphic.buffer,0,g->output->graphic.width*g->output->graphic.height*sizeof(oouint));
}







void drawBGGrid(oogame* g) {
	for(ooint y=0; y<CG_BG_GRID_HEIGHT; y++) {
		for(ooint x=0; x<CG_BG_GRID_WIDTH; x++) {
			if( getGridFatPixelValue(g, x, y)==1 ) {
				drawRectangle(g, x*g->fatPixelWidth, y*g->fatPixelHeight, g->fatPixelWidth, g->fatPixelHeight, 0xffeec151);
			}			
		}
	}
}

void drawTankBullet(oogame* g, ootank* tank) {
	oobullet* bullet = &tank->bullet;
	if( bullet->visible ) {
		drawRectangleFromCentre(g, (ooint)bullet->x, (ooint)bullet->y, g->smallPixelWidth, g->smallPixelHeight, tank->colour);
	}
	//drawRectangleFromCentre(g, (ooint)bullet->x, (ooint)bullet->y, g->smallPixelWidth, g->smallPixelHeight, 0xffff0000);
}

void drawTank(oogame* g, ootank* tank) {
	oofloat fx, fy, relX, relY;
	for(ooint y=0; y<CG_TANK_SHAPE_HEIGHT; y++) {
		for(ooint x=0; x<CG_TANK_SHAPE_WIDTH; x++) {
			if( g->tankShape[y*CG_TANK_SHAPE_WIDTH+x]==1 ) {
				relX = x*g->smallPixelWidth - ((g->smallPixelWidth*CG_TANK_SHAPE_WIDTH)*0.5f)+g->smallPixelWidth*0.5f;
				relY = y*g->smallPixelHeight - ((g->smallPixelWidth*CG_TANK_SHAPE_HEIGHT)*0.5f)+g->smallPixelHeight*0.5f;
				fx = tank->x + relX*cosf(tank->angle) - relY*sinf(tank->angle);
				fy = tank->y + relX*sinf(tank->angle) + relY*cosf(tank->angle);
				drawRectangleFromCentre(g, (ooint)fx, (ooint)fy, g->smallPixelWidth, g->smallPixelHeight, tank->colour);
			}
		}
	}

	drawTankBullet(g, tank);
}


void drawTanks(oogame* g) {
	drawTank(g, &g->tank1);
	drawTank(g, &g->tank2);
}

oobool keyPressed(oogame* g, oobyte key) {
	return g->input->keyboard[key];
}

void shootTankBullet(oogame* g, ootank* tank) {
	oobullet* bullet = &tank->bullet;
	if( !bullet->visible ) {
		bullet->x = tank->x + CG_TANK_SHAPE_WIDTH*g->smallPixelWidth * cosf(tank->angle) * 0.6f;
		bullet->y = tank->y + CG_TANK_SHAPE_HEIGHT*g->smallPixelHeight * sinf(tank->angle) * 0.6f;
		bullet->angle = tank->angle;
		bullet->visible = ootrue;
	}
}

oofloat distanceBetweenTanks(oogame* g) {
	oofloat dx = g->tank1.x - g->tank2.x;
	oofloat dy = g->tank1.y - g->tank2.y;
	return sqrtf(dx*dx+dy*dy);
}

oobool tankIsOnInvalidPosition(oogame* g, ootank* tank) {
	if( distanceBetweenTanks(g)<g->smallPixelHeight*CG_TANK_SHAPE_HEIGHT ) {
		return ootrue;
	}
	// collision with bg
	oofloat maxDistance = g->fatPixelWidth*0.5f + (g->smallPixelHeight*CG_TANK_SHAPE_HEIGHT)*0.6f;
	oofloat cellX, cellY, dx, dy, distance;
	for(ooint y=0; y<CG_BG_GRID_HEIGHT; y++) {
		for(ooint x=0; x<CG_BG_GRID_WIDTH; x++) {
			if( getGridFatPixelValue(g, x, y)==0 ) {
				continue;
			}
			cellX = x*g->fatPixelWidth + g->fatPixelWidth*0.5f;
			cellY = y*g->fatPixelHeight + g->fatPixelHeight*0.5f;
			dx = cellX - tank->x;
			dy = cellY - tank->y;
			distance = sqrtf(dx*dx+dy*dy);
			if( distance<maxDistance ) {
				return ootrue;
			}
		}
	}
	return oofalse;
}

void processTankInput(oogame* g, ootank* tank) {
	oofloat speed = g->input->dt*M_PIf;
	if( keyPressed(g, tank->keyLeft) ) {
		tank->angle -= speed;
	}
	if( keyPressed(g, tank->keyRight) ) {
		tank->angle += speed;
	}
	if( keyPressed(g, tank->keyFire) ) {
		shootTankBullet(g, tank);
	}
	if( keyPressed(g, tank->keyForward ) ) {
		oofloat forwardSpeed = g->input->dt * g->output->graphic.height * 0.2f;
		oofloat dx = forwardSpeed * cosf(tank->angle);
		oofloat dy = forwardSpeed * sinf(tank->angle);
		tank->x += dx;
		tank->y += dy;
		if( tankIsOnInvalidPosition(g, tank) ) {
			tank->x -= dx;
			tank->y -= dy;
			tank->angle -= speed;
		}
	}
}


void processInput(oogame* g) {
	processTankInput(g, &g->tank1);
	processTankInput(g, &g->tank2);
}

ootank* opponentTank(oogame* g, ootank* tank) {
	return ((tank==&g->tank1) ? &g->tank2 : &g->tank1);
}

oobool bulletCollidesWithTank(oogame* g, oobullet* b, ootank* tank) {
	oofloat dx = b->x-tank->x;
	oofloat dy = b->y-tank->y;
	oofloat distance = sqrtf(dx*dx+dy*dy);
	return distance<(g->smallPixelHeight*CG_TANK_SHAPE_HEIGHT);
}

oobool bulletCollidesWithWall(oogame* g, oobullet* b) {
	ooint x = (ooint)(b->x / g->fatPixelWidth);
	ooint y = (ooint)(b->y / g->fatPixelHeight);
	oobyte value = getGridFatPixelValue(g, x, y);
	
	if( value==1 ) {
		return ootrue;
	}
	return oofalse;
}

void updateTank(oogame* g, ootank* tank) {
	oobullet* bullet = &tank->bullet;
	if( bullet->visible ) {
		oofloat speed = g->output->graphic.height*0.8f * g->input->dt;
		bullet->x += speed * cosf(bullet->angle);
		bullet->y += speed * sinf(bullet->angle);

		ootank* opponent = opponentTank(g, tank);
		if( bulletCollidesWithWall(g, bullet) ) {
			bullet->visible = oofalse;
		} else {
			if( bulletCollidesWithTank(g, bullet, opponent) ) {
				opponent->numberOfHits++;
				if( opponent->numberOfHits>=5 ) {
					restartGame(g);
				}
				else {
					setInitialPositions(g);
					//setTankInitialPositions(g, opponent);
				}
				//opponent->x = 0.f;
				//opponent->y = 0.f;
			}
		}

		if( bullet->x>g->output->graphic.width || bullet->y>g->output->graphic.height ||
			bullet->x<0.f || bullet->y<0.f ) {
			bullet->visible = oofalse;
		}
	}
}

void updateTanks(oogame* g) {
	updateTank(g, &g->tank1);
	updateTank(g, &g->tank2);
}

void drawNumber(oogame* g, oobyte num, oouint colour, oofloat xCenter, oofloat yCenter) {
	num = num%10;
	ooint fx, fy;
	for(ooint y=0; y<CG_LCD_HEIGHT; y++) {
		for(ooint x=0; x<CG_LCD_WIDTH; x++) {
			if( g->numbers[num].lcd[y*CG_LCD_WIDTH+x]!=0 ) {
				fx = (ooint)(x*g->smallPixelWidth - (CG_LCD_WIDTH*g->smallPixelWidth)*0.5f + xCenter);
				fy = (ooint)(y*g->smallPixelHeight - (CG_LCD_HEIGHT*g->smallPixelHeight)*0.5f + yCenter);
				drawRectangleFromCentre(g, fx, fy, g->smallPixelWidth, g->smallPixelHeight, colour);
			}			
		}
	}
}

void drawLCD(oogame* g) {
	drawNumber(g, g->tank2.numberOfHits, g->tank1.colour, g->output->graphic.width*0.4f, g->fatPixelHeight*1.5f);
	drawNumber(g, g->tank1.numberOfHits, g->tank2.colour, g->output->graphic.width*0.6f, g->fatPixelHeight*1.5f);
}

void advanceGame(void* gameMemory, OOGameInput* input, OOGameOutput* output) {
	oogame* g = (oogame*) gameMemory;

	g->input = input;
	g->output = output;
	if( !g->init) {
		initGame(g);
	}

	//clearScreen(game);
	processInput(g);

	updateTanks(g);

	drawRectangle(g, 0, 0, (ooint)output->graphic.width, (ooint)output->graphic.height, 0xffbb4f0e);

	//setInitialPositions(g); // TODO remove
	//initBGGrid(g); // TODO remove

	drawBGGrid(g);

	drawTanks(g);

	drawLCD(g);

	//drawSquareFromCentre(game,(ooint)input->mouse.x,input->mouse.y,100,0xffff0000);

	g->time += input->dt;
}
