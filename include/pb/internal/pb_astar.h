#ifndef PB_ASTAR_H
#define PB_ASTAR_H

#include <pb/util/pb_vector.h>
#include <pb/util/pb_graph.h>

/* Function prototype for an A* heuristic function */
typedef float(*pb_astar_heuristic)(pb_vertex const* vertex, pb_vertex const* goal);

/**
 * An implementation of A* pathfinding for the pb_sq_house algorithm.
 * @param start     The start vertex.
 * @param goal      The goal vertex.
 * @param heuristic The heuristic function used to estimate a vertex's distance from the goal.
 * @param path      If a path was found, this will hold a pointer to a vector containing the vertices making up the path.
 *
 * @return 0 if a path was found, -1 if not. Note that -1 will be also be returned if the function runs out of memory.
 */
int pb_astar(pb_vertex const* start, pb_vertex const* goal, pb_astar_heuristic heuristic, pb_vector** path);

#endif /* PB_ASTAR_H */