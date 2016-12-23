#pragma once
#include <stddef.h>
#include <pb/util/util_exports.h>
#include <pb/util/vector/vector.h>

typedef struct {
    float x;
    float y;
} pb_point2D;

typedef struct {
    pb_point2D start;
    pb_point2D end;
} pb_line2D;

typedef struct {
    /* Vector of pb_point2D with the shape's points */
    pb_vector points;
} pb_shape2D;

typedef struct {
    pb_point2D bottom_left;
    float w;
    float h;
} pb_rect;

typedef struct {
    float x;
    float y;
    float z;
} pb_point3D;

/**
 * A vertex in 3D geometry.
 * On most compilers, this should have no padding, so the buffer in pb_shape3D
 * will be tightly packed.
 */
typedef struct {
    /* The vertex's position in model space */
    float x;
    float y;
    float z;

    /* The normal vector */
    float nx;
    float ny;
    float nz;

    /* The u and v coordinates */
    float u;
    float v;
} pb_vert3D;

typedef struct {
    /* A buffer of triangles. Winding order is CCW. */
    pb_vert3D* tris;

    /* The number of triangles in this shape. As you'd expect, multiply by 3 to get the number of vertices. */
    size_t num_tris;

    /* This shape's position relative to some origin. */
    pb_point3D pos;
} pb_shape3D;