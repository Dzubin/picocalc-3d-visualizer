//
//  shapes.h - everything placeable in the scene: solid objects (cube,
//  square pyramid, tetrahedron, octagonal prism, house, line, sphere) and
//  stars, all defined by a plain, hand-edited table at the top of
//  shapes.c (see its comment) - each entry picks one of the approved
//  object types, a world-space (x, y, z) position, a size, and a colour.
//  No platform dependencies.
//

#ifndef SHAPES_H
#define SHAPES_H

#include "vec3.h"

// SHAPE_MAX_EDGES/FACES are sized for the icosahedron (sphere_mesh, see
// mesh_definitions.h) - the biggest mesh here by a wide margin. Every mesh
// type reserves arrays at these sizes regardless of how many of its own
// vertices/edges/faces it actually uses, so growing these for one detailed
// shape costs a little unused space in every simpler one too - fine at
// this scale (a few hundred bytes per mesh_t), but keep in mind before
// adding something even more detailed.
#define SHAPE_MAX_VERTICES 16
#define SHAPE_MAX_EDGES 30
#define SHAPE_MAX_FACES 20
#define SHAPE_MAX_FACE_VERTICES 8

// Sentinel for mesh_t.edge_faces: "no face here". Only OBJECT_LINE's mesh
// uses this today (a line has an edge but no faces at all to be adjacent
// to it) - see shape_edge_faces()'s comment for how renderer.c's
// self-occlusion test treats it.
#define MESH_NO_FACE 0xFF

// A solid object TYPE's local-space geometry (centered on its own origin,
// before the scene entry's own scale and position are applied): a vertex
// list, an edge list (each edge is a vertex index pair plus the indices of
// the 2 faces that border it, or MESH_NO_FACE if a side has none - see
// above), and a face list (each face is a vertex count, 3 to
// SHAPE_MAX_FACE_VERTICES, plus that many vertex indices, wound so
// cross(v1-v0, v2-v0) points outward). No face normal is stored -
// shapes.c computes it from that winding at query time, so a new mesh
// only needs its topology, never a hand-derived normal. Defined in
// mesh_definitions.h, not here - see that file before touching one.
typedef struct {
    int vertex_count;
    vec3_t vertices[SHAPE_MAX_VERTICES];

    int edge_count;
    unsigned char edge_vertices[SHAPE_MAX_EDGES][2];
    unsigned char edge_faces[SHAPE_MAX_EDGES][2];

    int face_count;
    unsigned char face_vertex_count[SHAPE_MAX_FACES];
    unsigned char face_vertices[SHAPE_MAX_FACES][SHAPE_MAX_FACE_VERTICES];
} mesh_t;

int shape_count(void);

// True if this scene entry is a star (a single point with no edges or
// faces) rather than a solid object. shape_edge_count()/shape_face_count()
// are always 0 for one, so renderer.c draws it as a point instead of
// iterating edges/faces.
int shape_is_star(int shape_index);

int shape_edge_count(int shape_index);
int shape_face_count(int shape_index);

// This entry's edge, as world-space endpoints (its local geometry scaled
// and moved to its scene position). Never called for a star - see
// shape_is_star().
void shape_edge(int shape_index, int edge_index, vec3_t *out_v0, vec3_t *out_v1);

// This entry's face: world-space vertices (out_vertices[0..
// *out_vertex_count-1], up to SHAPE_MAX_FACE_VERTICES of them) and its
// outward unit normal. Never called for a star.
void shape_face(int shape_index, int face_index,
                 vec3_t out_vertices[SHAPE_MAX_FACE_VERTICES], int *out_vertex_count,
                 vec3_t *out_normal);

// The two face indices (see shape_face()) adjacent to a given edge (see
// shape_edge()) - either may be MESH_NO_FACE (see above) if that edge has
// no face on that side (only OBJECT_LINE today, on both sides). An edge is
// hidden by its own shape (self-occlusion) only when both sides resolve to
// a real, back-facing face - a MESH_NO_FACE side can never occlude, so it
// always counts as "not blocking" on its own. Never called for a star.
void shape_edge_faces(int shape_index, int edge_index, int *out_face0, int *out_face1);

// This entry's world-space centre - a solid object's placement point, or a
// star's entire position (a star has no other geometry).
vec3_t shape_center(int shape_index);

// This entry's flat colour, resolved from its 0-9 palette index (see
// FACE_COLOR_PALETTE in constants.h) to the actual RGB565 value.
unsigned short shape_color(int shape_index);

#endif
