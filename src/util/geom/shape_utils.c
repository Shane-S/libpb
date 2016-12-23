#include <pb/util/geom/shape_utils.h>
#include <stdlib.h>
#include <pb/util/geom/types.h>

PB_UTIL_DECLSPEC int pb_shape2D_init(pb_shape2D* shape, unsigned int num_points) {

    /* Initialise the vectors */
    if (pb_vector_init(&shape->points, sizeof(pb_point2D), num_points) == -1) {
        return -1;
    }

    return 0;
}

PB_UTIL_DECLSPEC pb_shape2D* pb_shape2D_create(unsigned int num_points) {
    pb_shape2D* result = NULL;
    pb_shape2D* points = NULL;

    result = malloc(sizeof(pb_shape2D));
    if (!result) return NULL;

    if (pb_shape2D_init(result, num_points) == -1) {
        free(result);
        return NULL;
    }

    return result;
}

PB_UTIL_DECLSPEC void PB_UTIL_CALL pb_shape2D_free(pb_shape2D* shape) {
    pb_vector_free(&shape->points);
}

PB_UTIL_DECLSPEC int PB_UTIL_CALL pb_shape3D_init(pb_shape3D* shape, unsigned int num_tris) {

    /* Initialise the vectors */
    shape->tris = malloc(sizeof(pb_vert3D) * num_tris * 3);
    if (shape->tris == NULL) {
        return NULL;
    }
    shape->num_tris = num_tris;

    return 0;
}

PB_UTIL_DECLSPEC pb_shape3D* pb_shape3D_create(unsigned int num_tris) {
    pb_shape3D* result = NULL;

    result = malloc(sizeof(pb_shape3D));
    if (!result) return NULL;

    if (pb_shape3D_init(result, num_tris) == -1) {
        free(result);
        return NULL;
    }

    return result;
}

PB_UTIL_DECLSPEC void pb_shape3D_free(pb_shape3D* shape) {
    free(shape->tris);
}
