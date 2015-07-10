/*
 * CerealBox - Copyright 2015 Ali Motisi. All rights reserved.
*/

#ifndef win32audio_h

#include <windows.h>
#include <dsound.h>
#include "../../common/src/oocommon.h"

void win32AudioInit(HWND window);
void win32AudioDestroy();
void win32AudioOutput(ooshort * data, oouint numOfFrames);
oouint win32AudioBufferSize();
oouint win32AudioFramesPerSecond();

#endif