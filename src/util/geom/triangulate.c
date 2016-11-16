#include <pb/util/geom/triangulate.h>
#include <pb/util/vector/vector.h>
#include <stdlib.h>

static int const CONVEX = 0;
static int const REFLEX = 1;
static int const EAR = 2;

typedef struct {
    int pt;
    size_t point_idx;
} pb_earclip_point;


size_t pb_shape2D_get_num_tris(pb_shape2D const* shape) {
    return shape->points.size - 2;
}

/**
 * Given a vertex and the previous and following vertices (CCW order),
 * determines wheter the given vertex is convex (the angle formed with
 * the previous and next vertices is < 180 degrees).
 *
 * @param vert The vertex to check for convexness.
 * @param prev The vertex directly before this vertex in the polygon.
 * @param next The vertex directly after this vertex in the polygon.
 * @return 1 if the angle formed between this vertex and its preceding
 *         and following vertices is < 180, false otherwise.
 */
int pb_earclip_is_convex(pb_point2D const* point, pb_point2D const* prev, pb_point2D const* next) {
    /* The x and y components of the vector perpendicular to the vector from next->prev */    
    float px, py;
    
    /* The x and y components of the vector from point->next */ 
    float vx, vy;

    float dot;

    /* Find vector perpendicular to prev->next; this faces toward the interior of the polygon */
    px = -(next->y - prev->y);
    py = next->x - prev->x;

    /* Get vector vert->next */
    vx = next->x - point->x;
    vy = next->y - point->y;

    /* Vector (px, py) cuts angle between lines (prev, vert) and (vert, next) in half
     * if dot product is positive, half angle < 90, so angle < 180 and vert is convex */ 
    dot = (px * vx) + (py * vy);

    return dot > 0;
}

int PB_UTIL_DECLSPEC pb_tri_contains_point PB_UTIL_CALL (pb_point2D* const t0, pb_point2D* const t1,
                                                         pb_point2D* const t2, pb_point2D* const p) {
    float area = 0.5f * (-t1->y * t2->x +
                         t0->y * (-t1->x + t2->x) +
                         t0->x * (t1->y - t2->y) +
                         t1->x * t2->y);

    float s = 1 / (2 * area) * (t0->y * t2->x - t0->x * t2->y + (t2->y - t0->y) * p->x + (t0->x - t2->x) * p->y);
    float t = 1 / (2 * area) * (t0->x * t1->y - t0->y * t1->x + (t0->y - t1->y) * p->x + (t1->x - t0->x) * p->y);
    float u = 1 - s - t;

    return s >= 0.f && t >= 0.f && u >= 0.f;
}

/**
 * Checks whether the given point in the earclip point list is an ear. 
 *
 * @param list     The list of pb_earclip_point holding the remaining points.
 * @param list_idx The index of the list item to check.
 * @param shape    The shape containing the points.
 * @return Non-zero if the point is an ear, 0 otherwsise.
 */
int pb_earclip_is_ear(pb_vector const* list, size_t list_idx, pb_shape2D const* shape) {
    pb_earclip_point* ecpoints = (pb_earclip_point*)list->items;
    pb_point2D* points = (pb_point2D*)shape->points.items;

    size_t t0_idx = list_idx == 0 ? ecpoints[list->size - 1].point_idx : ecpoints[list_idx - 1].point_idx;
    size_t t1_idx = list_idx;
    size_t t2_idx = (list_idx + 1) % list->size; /* We probably won't overflow here */

    pb_point2D* t0 = points + ecpoints[t0_idx].point_idx;
    pb_point2D* t1 = points + ecpoints[t1_idx].point_idx;
    pb_point2D* t2 = points + ecpoints[t2_idx].point_idx;

    size_t i;
    for(i = 0; i < list->size; ++i) {
        /* Don't test against points that are part of the triangle */
        if(i == t0_idx || i == t1_idx || i == t2_idx)
            continue;

        if(ecpoints[i].pt == REFLEX) {
            pb_point2D* p = points + ecpoints[i].point_idx;
            if(pb_tri_contains_point(t0, t1, t2, p)) {
                break;
            }
        }
    }

    return i == list->size;
}

/* TODO: This could definitely use some optimisation.
 * Linked lists (could be array-backed) instead of vectors for better removal cost
 * Detect when polygon becomes convex and do linear after that point
 * Switch for an algorithm that's n log n or better to start with */
PB_UTIL_DECLSPEC size_t* PB_UTIL_CALL pb_triangulate(pb_shape2D const* shape) {
    pb_point2D const* points = (pb_point2D*)shape->points.items;

    pb_vector point_list;

    size_t* tris = NULL;
    size_t cur_tri = 0;

    unsigned i, j;
    unsigned tri_idx = 0;
    unsigned num_convex = 0;

    point_list.items = NULL;

    tris = malloc(sizeof(size_t) * pb_shape2D_get_num_tris(shape) * 3);
    if(!tris) {
        goto err_return;
    }

    /* Don't bother doing all the other stuff if we're just processing a triangle */
    if(shape->points.size == 3) {
        tris[0] = 0;
        tris[1] = 1;
        tris[2] = 2;
        return tris;
    }

    if(pb_vector_init(&point_list, sizeof(pb_earclip_point), shape->points.size) == -1) {
        goto err_return;
    }

    /* Create vector of points */
    for(i = 0; i < shape->points.size; ++i) {
        pb_vector_push_back(&point_list, &i);
    }

    /* Determine whether each point is convex or reflex */
    for(i = 0; i < shape->points.size; ++i) {
        pb_earclip_point* ecpoints = (pb_earclip_point*)point_list.items;
        ecpoints[i].point_idx = i;

        if(pb_earclip_is_convex(points + i,
                                points + (i == 0 ? shape->points.size - 1 : i - 1),
                                points + (i == shape->points.size - 1 ? 0 : i + 1))) {
            ecpoints[i].pt = CONVEX;
            num_convex++;
        } else {
            ecpoints[i].pt = REFLEX;
        }
    }

    if(num_convex == point_list.size) {
        /* Convex polygon; triangulate in linear time by creating triangles from one vertex to
         * all other vertices */
        for(i = 1; i < point_list.size - 1; ++i) {
            size_t* tri = tris + ((i - 1) * 3);
            tri[0] = 0;
            tri[1] = i;
            tri[2] = i + 1;
        }
    } else {
         pb_earclip_point* ecpoints = (pb_earclip_point*)point_list.items;       
         size_t ear_idx;

        /* Find all ears */
        for(i = 0; i < shape->points.size; ++i) {
            if(ecpoints[i].pt == CONVEX && pb_earclip_is_ear(&point_list, i, shape)) {
                ecpoints[i].pt = EAR;
                ear_idx = i;
            }
        }

        /* Remove ears one by one until we're left with a triangle */
        while(1) {
            size_t ear_prev_idx = ear_idx == 0 ? point_list.size - 1 : ear_idx - 1;
            size_t ear_next_idx = (ear_idx + 1) % point_list.size;
            int updated_ear_idx = 0;

            tris[tri_idx] = ecpoints[ear_prev_idx].point_idx;
            tris[tri_idx + 1] = ecpoints[ear_idx].point_idx;
            tris[tri_idx + 2] = ecpoints[ear_next_idx].point_idx;

            tri_idx += 3;

            pb_vector_remove_at(&point_list, ear_idx);
            if(point_list.size == 3) {
                break;
            }

            /* Adjust next and prev indices after resize and check whether they became ears */
            if(ear_idx == 0 || ear_idx == point_list.size) {
                ear_prev_idx = point_list.size - 1;
                ear_next_idx = 0;
            } else {
                ear_next_idx = ear_idx;
            }

            /* Check whether points became convex to save us some time in checking earness later */
            if(ecpoints[ear_prev_idx].pt == REFLEX) {
                size_t ear_prev_prev_idx = ear_prev_idx == 0 ? point_list.size - 1 : ear_prev_idx - 1;
                size_t ear_prev_next_idx = (ear_prev_idx + 1) % point_list.size;

                if(pb_earclip_is_convex(points + ecpoints[ear_prev_idx].point_idx,
                                        points + ecpoints[ear_prev_prev_idx].point_idx,
                                        points + ecpoints[ear_prev_next_idx].point_idx)) {
                    ecpoints[ear_prev_idx].pt = CONVEX;
                }
            }

            if(ecpoints[ear_next_idx].pt == REFLEX) {
                size_t ear_next_prev_idx = ear_next_idx == 0 ? point_list.size - 1 : ear_next_idx - 1;
                size_t ear_next_next_idx = (ear_next_idx + 1) % point_list.size;
                if(pb_earclip_is_convex(points + ecpoints[ear_next_idx].point_idx,
                                        points + ecpoints[ear_next_prev_idx].point_idx,
                                        points + ecpoints[ear_next_next_idx].point_idx)) {
                    ecpoints[ear_next_idx].pt = CONVEX;
                }
            }

            /* Check whether points adjacent to the ear became convex and/or ears, then
             * find the next ear index. According to the paper, an ear might not still be an ear after removal of
             * another ear, so we need to check that too. I might revisit this later since I'm not convinced an ear
             * can stop being an ear though. */
            if(ecpoints[ear_prev_idx].pt == EAR || ecpoints[ear_prev_idx].pt == CONVEX) {
                if(pb_earclip_is_ear(&point_list, ear_prev_idx, shape)) {
                    ecpoints[ear_prev_idx].pt = EAR;
                    ear_idx = ear_prev_idx;
                    updated_ear_idx = 1;
                } else {
                    ecpoints[ear_prev_idx].pt = CONVEX;
                }
            }

            if(ecpoints[ear_next_idx].pt == EAR || ecpoints[ear_next_idx].pt == CONVEX) {
                if(pb_earclip_is_ear(&point_list, ear_next_idx, shape)) {
                    ecpoints[ear_next_idx].pt = EAR;
                    ear_idx = ear_next_idx;
                    updated_ear_idx = 1;
                } else {
                    ecpoints[ear_next_idx].pt = CONVEX;
                }
            }

            /* If neither point was or became an ear, find the next ear */
            if(!updated_ear_idx) {
                for(i = 0; i < point_list.size; ++i) {
                    if(ecpoints[i].pt == EAR) {
                        ear_idx = i;
                        break;
                    }
                }
            }
        }

        /* There are only three points left, so we don't need to worry about ear_prev, next, etc. */
        tris[tri_idx] = ecpoints[0].point_idx;
        tris[tri_idx + 1] = ecpoints[1].point_idx;
        tris[tri_idx + 2] = ecpoints[2].point_idx;
    }

    pb_vector_free(&point_list);
    return tris;

err_return:
    pb_vector_free(&point_list);
    free(tris);
    return NULL;
}
