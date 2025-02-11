#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

typedef struct 
{
    float x;
    float y;
} City;

// compute the distance between two points
float distance(float x0, float y0, float x1, float y1) 
{
    float diff[2] = 
    {
        x1 - x0, 
        y1 - y0
    };

    return sqrtf(diff[0] * diff[0] + diff[1] * diff[1]);
}

void permute(int *path, int pos, int n, City *cities, float *minDist, int *bestPath, bool *visited) 
{
    if (pos == n) 
    {
        float total = 0;
        for (int i = 0; i < n-1; i++)
            total += distance(cities[path[i]].x, cities[path[i]].y,
                            cities[path[i+1]].x, cities[path[i+1]].y);
        total += distance(cities[path[n-1]].x, cities[path[n-1]].y,
                        cities[path[0]].x, cities[path[0]].y);
        
        if (total < *minDist) 
        {
            *minDist = total;
            memcpy(bestPath, path, n * sizeof(int));
        }
        return;
    }

    for (int i = 0; i < n; i++) 
    {
        if (!visited[i]) 
        {
            visited[i] = true;
            path[pos] = i;
            permute(path, pos + 1, n, cities, minDist, bestPath, visited);
            visited[i] = false;
        }
    }
}

int main(int argc, char **argv) 
{
    City cities[11];
    int n = 0;
    float x, y;
    
    while (n < 11 && fscanf(stdin, "%f, %f", &x, &y) == 2) 
    {
        cities[n].x = x;
        cities[n].y = y;
        n++;
    }
    
    if (n == 0) return 1;

    int path[n];
    int bestPath[n];
    bool visited[n];
    memset(visited, 0, sizeof(visited));
    float minDist = FLT_MAX;
    visited[0] = true;
    path[0] = 0;
    permute(path, 1, n, cities, &minDist, bestPath, visited);
    fprintf(stdout, "%.2f\n", minDist);
    return 0;
}