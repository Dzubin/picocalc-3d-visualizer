//
//  mesh_definitions.h - the raw vertex/edge/face topology for every solid
//  object type shapes.c can place (cube, square pyramid, tetrahedron,
//  octagonal prism, house, line, sphere). Kept in its own file, away from
//  shapes.c's easily-edited scene table, specifically so it isn't sitting
//  right next to the part of the code a newcomer IS meant to edit.
//
//  ============================================================================
//  DON'T CHANGE THESE UNLESS YOU REALLY KNOW WHAT YOU ARE DOING!
//  ============================================================================
//  Want a shape at a different position, size, or colour? Edit shapes.c's
//  scene_shapes[] table instead - nothing here needs to change for that.
//  Everything below is topology, not appearance: every edge_vertices index
//  points into vertices[], every edge_faces index points into the face
//  list (or is MESH_NO_FACE - see shapes.h), and every face must be wound
//  so cross(v1-v0, v2-v0) points OUTWARD (see shapes.h). Getting a single
//  face's winding backwards makes that face's hidden-line self-occlusion
//  test come out inverted - whole sides of the shape vanish or reappear
//  depending on viewing angle, with no obvious connection to what you'd
//  expect changed. This actually happened once during development - it's a
//  real, easy-to-make, hard-to-spot mistake, not a hypothetical one.
//

#ifndef MESH_DEFINITIONS_H
#define MESH_DEFINITIONS_H

#include "shapes.h"
#include "constants.h"

#define S SHAPE_HALF_SIZE

static const mesh_t cube_mesh = {
    8,
    {
        {-S, -S, -S}, { S, -S, -S}, { S, -S,  S}, {-S, -S,  S},
        {-S,  S, -S}, { S,  S, -S}, { S,  S,  S}, {-S,  S,  S},
    },
    12,
    {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7},
    },
    {
        {0, 4}, {0, 3}, {0, 5}, {0, 2},
        {1, 4}, {1, 3}, {1, 5}, {1, 2},
        {4, 2}, {4, 3}, {5, 3}, {5, 2},
    },
    6,
    {4, 4, 4, 4, 4, 4},
    {
        {0, 1, 2, 3},
        {7, 6, 5, 4},
        {0, 3, 7, 4},
        {5, 6, 2, 1},
        {4, 5, 1, 0},
        {3, 2, 6, 7},
    },
};

/* Base square at y=-S, apex at y=+S directly above the base's center -
   same overall height (2*S) and footprint (2*S square) as the cube, for a
   comparable on-screen size. */
static const mesh_t square_pyramid_mesh = {
    5,
    {
        {-S, -S, -S}, { S, -S, -S}, { S, -S,  S}, {-S, -S,  S},
        { 0,  S,  0},
    },
    8,
    {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {0, 4}, {1, 4}, {2, 4}, {3, 4},
    },
    {
        {0, 1}, {0, 2}, {0, 3}, {0, 4},
        {4, 1}, {1, 2}, {2, 3}, {3, 4},
    },
    5,
    {4, 3, 3, 3, 3},
    {
        {0, 1, 2, 3},
        {1, 0, 4, 0},
        {2, 1, 4, 0},
        {3, 2, 4, 0},
        {0, 3, 4, 0},
    },
};

/* A regular tetrahedron: 4 alternating corners of a cube (the same trick
   that makes two interlocking tetrahedra out of a cube's 8 corners), which
   are equidistant from each other and from the origin by construction -
   no separate size tuning needed to keep it "regular". */
static const mesh_t tetrahedron_mesh = {
    4,
    {
        { S,  S,  S}, { S, -S, -S}, {-S,  S, -S}, {-S, -S,  S},
    },
    6,
    {
        {0, 1}, {0, 2}, {0, 3}, {1, 2}, {1, 3}, {2, 3},
    },
    {
        {2, 3}, {1, 3}, {1, 2}, {0, 3}, {0, 2}, {0, 1},
    },
    4,
    {3, 3, 3, 3},
    {
        {1, 3, 2, 0},
        {0, 2, 3, 0},
        {0, 3, 1, 0},
        {0, 1, 2, 0},
    },
};

/* A single segment from -S to +S along local X, with no faces at all - see
   MESH_NO_FACE in shapes.h for how its one edge's (nonexistent) adjacent
   faces are recorded, and shape_edge_faces()'s comment for how that keeps
   self-occlusion correct (a line can still be hidden behind a DIFFERENT
   object via the normal per-pixel depth test - it just can't hide behind
   itself, which is obviously correct for something with no volume). Always
   drawn in COLOR_SHAPE like any other object's edges (see constants.h's
   "edges stay white" rule) - its own scene-table colour is unused, same
   convention as a star's unused scale (write 0). There's no rotation
   support in this project, so a placed line can only ever run along local
   X. */
#define N MESH_NO_FACE
static const mesh_t line_mesh = {
    2,
    {
        {-S, 0, 0}, { S, 0, 0},
    },
    1,
    {
        {0, 1},
    },
    {
        {N, N},
    },
    0,
    { 0 },
    { { 0 } },
};
#undef N

/* An 8-sided prism approximating a cylinder: a regular octagon at y=-S
   (vertices 0-7) and another at y=+S (vertices 8-15, directly above,
   same angular order), connected by 8 rectangular side faces. C is
   cos(45 degrees) = sin(45 degrees), used for the 4 octagon corners that
   don't land on a pure axis. Winding for every face below was derived and
   cross-checked by hand (not copied from another mesh - an octagon's
   winding doesn't reduce to the cube/pyramid cases) - see this file's
   top warning before changing any of it. */
#define C 0.70710678f
static const mesh_t octagonal_prism_mesh = {
    16,
    {
        { S,       -S,  0.0f   }, { S*C,     -S,  S*C   }, { 0.0f,    -S,  S      }, {-S*C,     -S,  S*C   },
        {-S,       -S,  0.0f   }, {-S*C,     -S, -S*C   }, { 0.0f,    -S, -S      }, { S*C,     -S, -S*C   },
        { S,        S,  0.0f   }, { S*C,      S,  S*C   }, { 0.0f,     S,  S      }, {-S*C,      S,  S*C   },
        {-S,        S,  0.0f   }, {-S*C,      S, -S*C   }, { 0.0f,     S, -S      }, { S*C,      S, -S*C   },
    },
    24,
    {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 0},
        {8, 9}, {9, 10}, {10, 11}, {11, 12}, {12, 13}, {13, 14}, {14, 15}, {15, 8},
        {0, 8}, {1, 9}, {2, 10}, {3, 11}, {4, 12}, {5, 13}, {6, 14}, {7, 15},
    },
    {
        {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9},
        {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9},
        {2, 9}, {2, 3}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {7, 8}, {8, 9},
    },
    10,
    {8, 8, 4, 4, 4, 4, 4, 4, 4, 4},
    {
        {0, 1, 2, 3, 4, 5, 6, 7},
        {8, 15, 14, 13, 12, 11, 10, 9},
        {8, 9, 1, 0},
        {9, 10, 2, 1},
        {10, 11, 3, 2},
        {11, 12, 4, 3},
        {12, 13, 5, 4},
        {13, 14, 6, 5},
        {14, 15, 7, 6},
        {15, 8, 0, 7},
    },
};
#undef C

// House dimensions thanks to Mark Baumann, USask Computer Science Class of 1983 CMPT460
/* A "house": a pentagonal cross-section (a square topped by a triangular
   gable) extruded along local X from -2S to +2S, i.e. a length of 4S -
   the combined length of two S-sized cubes placed end to end - with a
   ridge line running the full length at the apex, exactly like a real
   gabled roof over an elongated box. Authored as one convex solid (a
   pentagonal prism) rather than as separate cube+cube+roof pieces, since
   that's what "two cubes with a roof on top, sharing their footprint"
   actually IS geometrically once the roof sits flush on the box - and a
   single convex mesh gets correct self-occlusion for free from the same
   analytic test every other solid here uses, instead of relying on
   cross-shape depth testing between 3 separate objects. */
static const mesh_t house_mesh = {
    10,
    {
        {-2*S, -S, -S}, {-2*S, -S,  S}, {-2*S,  S,  S}, {-2*S, 2*S, 0.0f}, {-2*S,  S, -S},
        { 2*S, -S, -S}, { 2*S, -S,  S}, { 2*S,  S,  S}, { 2*S, 2*S, 0.0f}, { 2*S,  S, -S},
    },
    15,
    {
        {0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0},
        {5, 6}, {6, 7}, {7, 8}, {8, 9}, {9, 5},
        {0, 5}, {1, 6}, {2, 7}, {3, 8}, {4, 9},
    },
    {
        {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6},
        {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6},
        {2, 6}, {2, 3}, {3, 4}, {4, 5}, {5, 6},
    },
    7,
    {5, 5, 4, 4, 4, 4, 4},
    {
        {0, 1, 2, 3, 4},
        {5, 9, 8, 7, 6},
        {0, 5, 6, 1},
        {1, 6, 7, 2},
        {2, 7, 8, 3},
        {3, 8, 9, 4},
        {4, 9, 5, 0},
    },
};

/* A low-facet sphere approximation: a regular icosahedron (12 vertices, 30
   edges, 20 triangular faces - the most sphere-like of the 5 Platonic
   solids, and the standard starting point for a low-poly sphere). Every
   vertex sits at exactly the same distance (S) from the center, unlike
   every other mesh here, whose "radius" varies by direction - so this is
   the one mesh where SHAPE_HALF_SIZE reads literally as a radius.

   Vertices are the 12 cyclic permutations of (0, +-1, +-phi), phi the
   golden ratio, normalized so every vertex has length 1 before scaling by
   S - A = 1/sqrt(1+phi^2) is the normalized "1" component, B = A*phi is
   the normalized "phi" component (A^2 + B^2 = 1 by construction). This
   construction is a standard, well-documented one; the specific vertex
   numbering, full edge list, and every face's winding below were still
   independently re-derived and cross-checked by hand (via explicit
   cross-product-vs-centroid-direction checks, the same method used to
   verify every other mesh in this file) rather than copied from a
   reference, given how easy a single backwards face is to get wrong and
   how hard it is to spot after the fact (see this file's top warning). */
#define A (0.52573111f * S)
#define B (0.85065081f * S)
static const mesh_t sphere_mesh = {
    12,
    {
        {0.0f,  A,  B}, {0.0f,  A, -B}, {0.0f, -A,  B}, {0.0f, -A, -B},
        { A,  B, 0.0f}, { A, -B, 0.0f}, {-A,  B, 0.0f}, {-A, -B, 0.0f},
        { B, 0.0f,  A}, { B, 0.0f, -A}, {-B, 0.0f,  A}, {-B, 0.0f, -A},
    },
    30,
    {
        {0, 2}, {0, 4}, {0, 6}, {0, 8}, {0, 10},
        {1, 3}, {1, 4}, {1, 6}, {1, 9}, {1, 11},
        {2, 5}, {2, 7}, {2, 8}, {2, 10},
        {3, 5}, {3, 7}, {3, 9}, {3, 11},
        {4, 6}, {4, 8}, {4, 9},
        {5, 7}, {5, 8}, {5, 9},
        {6, 10}, {6, 11},
        {7, 10}, {7, 11},
        {8, 9},
        {10, 11},
    },
    {
        {0, 4}, {1, 2}, {2, 3}, {0, 1}, {3, 4},
        {5, 9}, {6, 7}, {7, 8}, {5, 6}, {8, 9},
        {12, 16}, {15, 16}, {0, 12}, {4, 15},
        {17, 19}, {18, 19}, {5, 17}, {9, 18},
        {2, 7}, {1, 10}, {6, 10},
        {16, 19}, {11, 12}, {11, 17},
        {3, 13}, {8, 13},
        {14, 15}, {14, 18},
        {10, 11},
        {13, 14},
    },
    20,
    {3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3},
    {
        {0, 2, 8}, {0, 8, 4}, {0, 4, 6}, {0, 6, 10}, {0, 10, 2},
        {1, 9, 3}, {1, 4, 9}, {1, 6, 4}, {1, 11, 6}, {1, 3, 11},
        {4, 8, 9}, {8, 5, 9}, {2, 5, 8}, {10, 6, 11}, {10, 11, 7},
        {10, 7, 2}, {2, 7, 5}, {3, 9, 5}, {3, 7, 11}, {3, 5, 7},
    },
};
#undef A
#undef B

#undef S

#endif
