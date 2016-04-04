#ifndef PB_GENERATION_H
#define PB_GENERATION_H

#include <pb/pb_exports.h>
#include <pb/pb_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Generates a single-floor building, using up to num_rooms rooms, with a floor of size w by h.
 *
 * @param room_types Descriptions of each room
 */
DECLSPEC pb_building* PBCALL pb_gen_single_floor(pb_room* room_types, size_t num_rooms, size_t w, size_t h);

/**
 *
 */
DECLSPEC pb_building* PBCALL pb_gen_multi_floor(pb_room* room_types, size_t num_rooms, size_t w, size_t h);


#ifdef __cplusplus
}
#endif
#endif /* PB_GENERATION_H */