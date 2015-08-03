/*
* CerealBox - Copyright 2015 Ali Motisi. All rights reserved.
*/

#ifndef oocommon_h
#define oocommon_h

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>


//#define OO_FULLSCREEN

#define M_PIf 3.14159265359f
#define ooinline __inline

// types are prefixed with oo as two Os in OutOfTheBit
// I could have used cb as for Cereal Box but oo is faster to type
// plus the code can be shared more easily across projects

typedef char oochar;
typedef uint8_t oobyte;
typedef uint8_t oobool;

typedef int32_t ooint;
typedef int16_t ooshort;
typedef int64_t ooint64;
typedef float oofloat;

typedef uint32_t oouint;
typedef uint16_t ooushort;
typedef uint64_t oouint64;

#define ootrue 1
#define oofalse 0

#endif