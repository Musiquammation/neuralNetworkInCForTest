#ifndef GAMES_ORB_COORDS_H_
#define GAMES_ORB_COORDS_H_

#include <tools/tools.h>
structdef(Vector);
structdef(Vector_int);
structdef(Vector_short);

struct Vector {
    float x;
    float y;
};

struct Vector_int {
    int x;
    int y;
};

struct Vector_short {
    short x;
    short y;
};


float Q_rsqrt(float x);
float coords_getSqDist(float dx, float dy);

Vector coords_getStartSpeed(
    float dx,
    float dy,
    float gravity,
    float maxSpeed
);


#endif