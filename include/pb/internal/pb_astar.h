#ifndef PB_ASTAR_H
#define PB_ASTAR_H

#include <pb/util/pb_vector.h>
#include <pb/util/pb_graph.h>

/* Function prototype for an A* heuristic function */
typedef float(*pb_astar_heuristic)(pb_vertex const* vertex, pb_vertex const* goal);

/**
 * An implementation of A* pathfinding for the pb_sq_house algorithm.
 * @param graph     The graph to search.
 * @param start     The start vertex.
 * @param goal      The goal vertex.
 * @param heuristic The heuristic function used to estimate a vertex's distance from the goal.
 *
 * @return The list of vertices (in order from start to goal) comprising the shortest path.
 */
pb_vector* pb_astar(pb_graph const* graph, pb_vertex const* start, pb_vertex const* goal);

#endif /* PB_ASTAR_H */