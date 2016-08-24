#pragma once
#include <stddef.h>

typedef struct _pb_point {
    float x;
    float y;
} pb_point;

typedef struct _pb_rect {
    pb_point top_left;
    float w;
    float h;
} pb_rect;

typedef struct _pb_shape {
    pb_point *points;
    size_t num_points;

    // When num_points > 2, determines whether the last vertex connects to the first
    int connected;
} pb_shape;

/**
 * A vertex in 3D geometry.
 */
typedef struct _pb_geom_vertex {
    // The vertex's position in model space
    float x;
    float y;
    float z;

    // The normal vector
    float nx;
    float ny;
    float nz;

    // The u and v coordinates
    float u;
    float v;
} pb_geom_vertex;