#ifndef PB_EXTRUSION_H
#define PB_EXTRUSION_H

#include <pb/util/geom/types.h>
#include "floor_plan.h"

/**
 * A room corresponding to a pb_room specified in a floor plan.
 *
 * walls:   The walls encapsulating the room. Each wall corresponds to a pair of points in floor.shape, e.g. wall[0]
 *          contains the shapes for the wall from floor.shape[0] to floor.shape[1], etc. Note that if the shape indicated
 *          that a given wall wasn't connected, then the corresponding wall be set to NULL.
 * windows: The set of windows in the room, corresponding to the windows in a pb_room.
 * doors:   The set of doors in the room, corresponding to the doors in a pb_room.
 * ground:  The ground. If the corresponding pb_room has has_ground set to 0, then this will be NULL.
 * ceiling: The room's ceiling. If the corresponding pb_room has has_ceiling set to 0, then this will be NULL.
 */
typedef struct {
    pb_shape3D** walls;
    size_t* wall_counts;
    size_t num_walls;

    pb_shape3D* windows;
    size_t num_windows;

    pb_shape3D* doors;
    size_t num_doors;

    pb_shape3D* ground;
    pb_shape3D* ceiling;
} pb_extruded_room;

/**
 * A floor corresponding to a pb_floor specified in a floor plan.
 *
 * rooms:   The rooms extruded by the extrusion algorithm. The number of rooms will be specified in the corresponding
 *          floor in the floor plan.
 * walls:   The walls encapsulating the floor. Each wall corresponds to a pair of points in floor.shape, e.g. wall[0]
 *          contains the shapes for the wall from floor.shape[0] to floor.shape[1], etc.
 * windows: The set of windows on the floor, corresponding to the windows in a pb_floor.
 * doors:   The set of doors on the floor, corresponding to the doors in a pb_floor.
 */
typedef struct {
    pb_extruded_room** rooms;
    size_t num_rooms;

    pb_shape3D** walls;
    size_t* wall_counts;


    pb_shape3D* windows;
    size_t num_windows;

    pb_shape3D* doors;
    size_t num_doors;
} pb_extruded_floor;

/**
 * Determines how many new shapes will result from calling the extrusion function.
 *
 * This function allows the extrusion function some flexibility, but note that it MUST be deterministic.
 * It should hopefully be fairly quick since it will be called quite a few times.
 *
 * @param wall           The line representing this wall.
 * @param wall_structure The line representing the wall structure.
 * @param normal         This wall's (2D) normal vector.
 * @param floor_height   The height to which each floor will be extruded. The produced shapes must occupy this height.
 * @param struct_height  The requested height for the window/door. The function choose not to respect this.
 * @param start_height   The height at which each extruded shape must start.
 * @param param          The supplied parameter, if any.
 * @param num_walls      The number of walls that will be allocated by the function.
 * @param num_structures The number of wall structure shapes that will be allocated by the function. Allows things like
 *                       the creation of double doors if the wall is big enough.
 */
typedef void (*pb_wall_structure_count_func)(pb_line2D const* wall, pb_line2D const* wall_structure, pb_point2D const* normal,
                                             pb_point2D const* bottom_floor_centre, float floor_height,
                                             float struct_height, float start_height,
                                             void* param, size_t* num_walls, size_t* num_structures);
/**
 * Extrudes the given wall structure (door/window) to a 3D shape.
 *
 * @param wall           The line representing this wall.
 * @param wall_structure The line representing the wall structure.
 * @param normal         This wall's (2D) normal vector.
 * @param floor_height   The height to which each floor will be extruded. The produced shapes must occupy this height.
 * @param struct_height  The requested height for the window/door. The function choose not to respect this.
 * @param start_height   The height at which each extruded shape must start.
 * @param param          The supplied parameter, if any.
 * @param walls_out      On success, holds a pointer to the list of wall shapes.
 * @param structures_out On success, holds a pointer to the list of wall structure shapes.
 *
 * @return 0 on success, -1 on failure.
 */
typedef int (*pb_wall_structure_extrusion_func)(pb_line2D const* wall, pb_line2D const* wall_structure, pb_point2D const* normal,
                                                pb_point2D const* bottom_floor_centre, float floor_height,
                                                float struct_height, float start_height,
                                                void* param, pb_shape3D** walls_out, pb_shape3D** structures_out);

typedef struct {
    pb_wall_structure_count_func count;
    pb_wall_structure_extrusion_func extrude;
} pb_wall_structure_extruder;

/**
 * Extrudes a wall with the given doors and windows.
 *
 * @param wall                  The wall to extrude.
 * @param doors                 The doors in this wall as a list of lines. Note that this will be sorted by this function.
 * @param num_doors             The number od doors in the doors list.
 * @param windows               The windows in this wall as a list of lines. Note that this will be sorted by this function.
 * @param num_windows           The number of windows contained in the windows list.
 * @param bottom_floor_centre   The centre point of the bottom floor.
 * @param normal                This parent wall's normal vector.
 * @param start_height          The starting height for this wall.
 * @param floor_height          The height of a floor (i.e. the wall's height).
 * @param door_height           The desired height for doors. May be entirely disregarded by the extrusion function.
 * @param window_height         The desired height for windows. May be entirely disregarded by the extrusion function.
 * @param door_extruder         The function to extrude doors and the function to count how many doors and walls it will produce.
 * @param window_extruder       The function to extrude windows and the function to count how many windows and walls it will produce.
 * @param door_extruder_param   An optional parameter to pass to the door extruder.
 * @param window_extruder_param An optional parameter to pass to the window extruder.
 * @param walls_out             Holds the list of walls produced by extrusion.
 * @param num_walls_out         Holds the number of walls in walls_out.
 * @param doors_out             Holds the list of doors produced by extrusion.
 * @param num_doors_out         Holds the number of door shapes produced by extrusion.
 * @param windows_out           Holds the list of window shapes produced by extrusion.
 * @param num_windows_out       Holds the number of windows produced by extrusion.
 *
 * @return 0 on success, -1 on failure (out of memory). Note that the out parameters will not be valid on failure. The
 *         input lists will be in a valid but unspecified state.
 */
PB_DECLSPEC int PB_CALL pb_extrude_wall(pb_line2D const* wall,
                                        pb_wall_structure const* doors, size_t num_doors,
                                        pb_wall_structure const* windows, size_t num_windows,
                                        pb_point2D const* bottom_floor_centre,
                                        pb_point2D const* normal,
                                        float start_height,
                                        float floor_height,
                                        float door_height,
                                        float window_height,
                                        pb_wall_structure_extruder const* door_extruder,
                                        pb_wall_structure_extruder const* window_extruder,
                                        void* extruder_param,
                                        void* window_extruder_param,
                                        pb_shape3D** walls_out, size_t* num_walls_out,
                                        pb_shape3D** doors_out, size_t* num_doors_out,
                                        pb_shape3D** windows_out, size_t* num_windows_out);

/**
 * Extrudes a room to a set of 3D shapes.
 *
 * @param room                  The room to extrude.
 * @param bottom_floor_centre   The centre point of the bottom floor.
 * @param start_height          The starting height for this room.
 * @param floor_height          The height of the current floor.
 * @param door_height           The desired height for doors in this room.
 * @param window_height         The desired window height for windows in this room.
 * @param door_extruder         The pair of functions to extrude doors.
 * @param window_extruder       The pair of functions to exrude windows.
 * @param door_extruder_param   An optional parameter to pass to the door extruder.
 * @param window_extruder_param An optional parameter to pass to the window extruder.
 *
 * @return A pointer to an allocated pb_exruded_room on success, NULL on failure.
 */
PB_DECLSPEC pb_extruded_room* PB_CALL pb_extrude_room(pb_room const* room,
                                                      pb_point2D const* bottom_floor_centre,
                                                      float start_height,
                                                      float floor_height,
                                                      float door_height,
                                                      float window_height,
                                                      pb_wall_structure_extruder const* door_extruder,
                                                      pb_wall_structure_extruder const* window_extruder,
                                                      void* door_extruder_param,
                                                      void* window_extruder_param);

/**
 *
 * @param f
 * @param bottom_floor_centre  The centre point of the bottom floor.
 * @param start_height         This floor's start height.
 * @param floor_height         This floor's height.
 * @param door_height          The desired height for doors.
 * @param window_height        The desired height for windows.
 * @param door_extruder        A pair of functions to extrude doors and determine how many doors and walls will be produced.
 * @param window_extruder      A pair of functions to extrude windows and determine how many windows and walls will be produced.
 * @param door_extruder_param   An optional parameter to pass to the door extruder.
 * @param window_extruder_param An optional parameter to pass to the window extruder.
 *
 * @return A pointer to an allocated pb_extruded_floor on success, NULL on failure.
 */
PB_DECLSPEC pb_extruded_floor* PB_CALL pb_extrude_floor(pb_floor const* f,
                                                        pb_point2D const* bottom_floor_centre,
                                                        float start_height,
                                                        float floor_height,
                                                        float door_height,
                                                        float window_height,
                                                        pb_wall_structure_extruder const* door_extruder,
                                                        pb_wall_structure_extruder const* window_extruder,
                                                        void* door_extruder_param,
                                                        void* window_extruder_param);

/**
 * Extrudes a building to a series of 3D shapes.
 * @param building              The building to extrude.
 * @param floor_height          The height for each floor.
 * @param door_height           The height for doors. Must be < floor height.
 * @param window_height         The height for windows. Must be < window height.
 * @param door_extruder         The function to extrude doors.
 * @param window_extruder       The function to extrude windows.
 * @param door_extruder_param   An optional parameter to pass to the door extruder.
 * @param window_extruder_param An optional parameter to pass to the window extruder.
 *
 * @return On success, a list of pb_extruded_floor, with the number of floors == building->num_floors.
 *         Each floor contains a number of rooms (matching building->floors[i].num_rooms). Returns NULL on failure.
 */
PB_DECLSPEC pb_extruded_floor** PB_CALL pb_extrude_building(pb_building* building,
                                                            float floor_height,
                                                            float door_height,
                                                            float window_height,
                                                            pb_wall_structure_extruder const* door_extruder,
                                                            pb_wall_structure_extruder const* window_extruder,
                                                            void* door_extruder_param,
                                                            void* window_extruder_param);


PB_DECLSPEC void PB_CALL pb_extruded_room_free(pb_extruded_room* f);
PB_DECLSPEC void PB_CALL pb_extruded_floor_free(pb_extruded_floor* f);

#ifdef __cplusplus
}
#endif

#endif /* PB_EXTRUSION_H */
