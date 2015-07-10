# CerealBox

CeralBox is minimal game engine written in C.
It's designed to provide C programmers with the bare minimum of what's needed to create a game.

Simply provide an implementation for the following functions and you are good to go:

oouint64 gameMemorySize();

void advanceGame(void * gameMemory, OOGameInput * input, OOGameOutput * output);

The advanceGame function provides and input (mouse, keyboard and delta time since the last update)
and output (a graphic buffer and an audio buffer).

You can open the project in Visual Studio 2013, from the CerealBox\win32\vsproject directory.
There is also a reference implementation in CerealBox\games\samplegame.

Start from there and have fun!

Any questions please contact me at ali@motisi.com.

If you are interested in this project also consider following me on twitter https://twitter.com/alimotisi

@alimotisi
