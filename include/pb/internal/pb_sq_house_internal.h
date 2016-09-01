#ifndef PB_SQ_HOUSE_INTERNAL_H
#define PB_SQ_HOUSE_INTERNAL_H

#include <pb/pb_sq_house.h>
#include <pb/internal/pb_squarify.h>

char** pb_sq_house_choose_rooms(pb_hash* room_specs, pb_sq_house_house_spec* house_spec);

#endif /* PB_SQ_HOUSE_INTERNAL */