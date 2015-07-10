# CerealBox

CeralBox is minimal game engine written in C.
It's designed to provide C programmers with the bare minimum of what's needed to create a game.

Simply provide an implementation for the following functions and you are good to go:

oouint64 gameMemorySize();
void advanceGame(void * gameMemory, OOGameInput * input, OOGameOutput * output);
