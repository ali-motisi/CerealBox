#ifndef oogame_h

#include "oocommon.h"

#define OOPointerevents_Max 10
#define OOGameInput_Keys_Max 512

typedef struct {
	oofloat x;
	oofloat y;
	oobool leftButtonDown;
	oobool rightButtonDown;
} OOMouseInput;

typedef struct {
	oofloat dt;
	oobool keyboard[OOGameInput_Keys_Max];
	OOMouseInput mouse;
} OOGameInput;

typedef struct {
	ooint width;
	ooint height;
	oouint* buffer;
} OOGraphic;


typedef struct {
	oouint framesToWrite;
	ooshort* buffer;
} OOAudio;

typedef struct {
	OOGraphic graphic;
	OOAudio audio;
} OOGameOutput;




// all you need to create your game is to provide and implementation for the following methods

oouint64 gameMemorySize();

void advanceGame(void * gameMemory, OOGameInput * input, OOGameOutput * output);

#endif