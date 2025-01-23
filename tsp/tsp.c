#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

// compute the distance between two points
float distance(float x0, float y0, float x1, float y1) 
{
    float diff[2] = {
        x1 - x0, 
        y1 - y0
    };

    return sqrtf(diff[0] * diff[0] + diff[1] * diff[1]);
}

int main(int argc, char **argv)
{
    return 0;
}