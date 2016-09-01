#ifndef PB_SQ_HOUSE_H
#define PB_SQ_HOUSE_H

#include <pb/pb_exports.h>
#include <pb/pb_geom.h>
#include <pb/pb_extrusion.h>
#include <pb/util/pb_hash.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _pb_sq_house_room_spec {
    char const* name;
    char const** adjacent;
    float area;
    int max_instances;
    int priority;
} pb_sq_house_room_spec;

typedef struct _pb_sq_house_house_spec {
    float height;
    float width;
    int num_rooms;
} pb_sq_house_house_spec;

PB_DECLSPEC pb_building* PB_CALL pb_sq_house(pb_sq_house_house_spec* house_spec, pb_hash* room_specs);

#ifdef __cplusplus
}
#endif
#endif /* PB_SQ_HOUSE_H */