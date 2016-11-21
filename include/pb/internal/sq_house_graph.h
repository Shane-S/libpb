#ifndef PB_SQ_HOUSE_GRAPH_H
#define PB_SQ_HOUSE_GRAPH_H

#include <stdlib.h>
#include <pb/sq_house.h>
#include <pb/util/graph/graph.h>
#include <pb/util/hashmap/hashmap.h>
#include <pb/util/vector/vector.h>

/**
 * Defines a connection to a neighbouring room.
 */
typedef struct _pb_sq_house_room_conn {
    pb_room* room;
    pb_room* neighbour;
    pb_point2D overlap_start;
    pb_point2D overlap_end;
    side wall;
    int can_connect; /* Whether there could be a door between these two rooms */
} pb_sq_house_room_conn;

/**
 * Determines which wall (if any) is shared by room1 and room2.
 *
 * @param room1 The first room to check for a shared wall.
 * @param room2 The second room to check for a shared wall.
 *
 * @return The appropriate value from the side enum if there is a shared wall,
 *         -1 if there are no shared walls between the rooms.
 */
int pb_sq_house_get_shared_wall(pb_room* room1, pb_room* room2);

/**
 * Gets the overlap between the shared walls in room1 and room2.
 *
 * Note that end-start in the dimension of overlap will always be increasing.
 * E.g. if the top wall intersected, end.x - start.x will always be >= 0.
 *
 * @param room1 The first room (must be the same as room1 in pb_sq_house_shared_wall).
 * @param room2 The second room (must be the same as room2 in pb_sq_house_shared_wall).
 * @param wall  The wall shared between the rooms (as per pb_sq_house_shared_wall).
 *              Must be [1, 4]; if it's outside this range, the function returns without modifying
 *              its arguments.
 * @param start Holds the start point of overlap.
 * @param end   Holds the end point of overlap.
 */
void pb_sq_house_get_wall_overlap(pb_room const* room1, pb_room const* room2, int wall, pb_point2D* start, pb_point2D* end);

/**
 * Generates a connectivity graph between the rooms of a given floor.
 *
 * The edges in the graph are of type pb_sq_house_room_conn and indicate whether these rooms
 * could be connected by a door (based on their room_specs).
 *
 * Vertices are identified by the room they store; call pb_graph_get_vertex with a pointer to
 * a room as the vert_id to get its corresponding vertex. That vertex's data member will also
 * be set to the room pointer (vertex->data == vert_id).
 *
 * @param room_specs The map containing room specifications for this house.
 * @param floor      The floor for which the connectivity graph will be generated.
 * @return A graph containing the rooms' connections.
 */
pb_graph* pb_sq_house_generate_floor_graph(pb_sq_house_house_spec* house_spec, pb_hashmap* room_specs, pb_floor* floor);

/**
 * Given a floor graph generated by pb_sq_house_generate_floor_graph, finds any rooms that
 * don't have any adjacent rooms to which they can connect.
 *
 * Note that if a room says that it can connect to a neighbour, but the neighbour can't
 * connect to that room, both rooms ARE considered connected.
 *
 * @param floor_graph The floor connectivity graph.
 * @param floor       The floor being processed.
 * @return A pb_hashmap containing the list of disconnected rooms. The vert_id for the room is the key,
 *         and the room pointer (which is also the vert_id, but whatever) is the value. Note that the
 *         first room on the floor (the room connecting to outside on the first floor and the first set
 *         of stairs on every other floor) will never be in this list. Returns NULL on failure.
 */
pb_hashmap* pb_sq_house_find_disconnected_rooms(pb_graph* floor_graph, pb_floor* floor);

#endif /* PB_SQ_HOUSE_GRAPH_H */
