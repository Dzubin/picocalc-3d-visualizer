#include "shapes.h"
#include "constants.h"
#include "mesh_definitions.h"

// The closed set of approved object types a scene_shapes[] entry below can
// use. OBJECT_STAR is a single point, not a mesh - it has no entry in
// mesh_definitions.h and mesh_for_object_type() is never called for one
// (shape_edge_count()/shape_face_count() return 0 for it instead, so
// nothing ever asks for its geometry). Adding a genuinely new solid TYPE
// means adding another mesh_t constant to mesh_definitions.h plus a case
// in mesh_for_object_type() - nothing in renderer.c or hidden_line.c needs
// to change either way.
typedef enum {
    OBJECT_CUBE,
    OBJECT_SQUARE_PYRAMID,
    OBJECT_TETRAHEDRON,
    OBJECT_OCTAGONAL_PRISM,
    OBJECT_HOUSE,
    OBJECT_LINE,
    OBJECT_SPHERE,
    OBJECT_STAR,
} object_type_t;

typedef struct {
    object_type_t type;
    vec3_t center;
    float scale;          // size multiplier (1.0 = this project's default size); ignored for OBJECT_STAR, write 0
    unsigned char color;  // 0-9, see FACE_COLOR_PALETTE in constants.h
} shape_instance_t;

// Author: Thomas Dzubin
static const mesh_t *mesh_for_object_type(object_type_t type)
{
    switch (type) {
    case OBJECT_SQUARE_PYRAMID:   return &square_pyramid_mesh;
    case OBJECT_TETRAHEDRON:      return &tetrahedron_mesh;
    case OBJECT_OCTAGONAL_PRISM:  return &octagonal_prism_mesh;
    case OBJECT_HOUSE:            return &house_mesh;
    case OBJECT_LINE:             return &line_mesh;
    case OBJECT_SPHERE:           return &sphere_mesh;
    case OBJECT_CUBE:
    default:                      return &cube_mesh;
    }
}

// ---------------------------------------------------------------------
// THE SCENE - edit this table to add, remove, or move objects. This is the
// one place a newcomer needs to look to change what's in the scene.
// ---------------------------------------------------------------------
// Each entry is { type, {x, y, z}, scale, color }:
//   - type: OBJECT_CUBE, OBJECT_SQUARE_PYRAMID, OBJECT_TETRAHEDRON,
//     OBJECT_OCTAGONAL_PRISM, OBJECT_HOUSE, OBJECT_LINE, OBJECT_SPHERE, or
//     OBJECT_STAR (a single point, drawn regardless of hidden-line removal
//     - scale is meaningless for one, write 0 by convention). OBJECT_LINE
//     also ignores its color (its edge is always drawn in COLOR_SHAPE,
//     like every other object's edges) - write 0 for that too.
//   - {x, y, z}: world-space position, in the same world units as
//     everything else in this project (see constants.h's comment on the
//     100x world-unit scale). (0, 0, 0) is the origin the X/Y/Z axes cross.
//   - scale: size multiplier for a solid object - 1.0 is this project's
//     default size (see SHAPE_HALF_SIZE in constants.h), 2.0 is double,
//     0.5 is half. Every object here can have its own. Note the house and
//     the line are already elongated in their local geometry (4x and 2x
//     SHAPE_HALF_SIZE respectively) before scale is even applied.
//   - color: 0-9, see FACE_COLOR_PALETTE in constants.h (0=black, 1=white,
//     2-9=muted colours). For a solid object this only shows up on its
//     faces while hidden-line removal is on (see ENABLE_FACE_COLOR); for a
//     star it's just that star's colour, always visible.
//
// There's no rotation support in this project - every object appears in
// whatever orientation mesh_definitions.h authored it in, just moved,
// resized, and recoloured.
//
// Keep a solid object's center a few hundred units clear of the origin (or
// it crowds the axes) and clear of ORBIT_RADIUS_MIN, the camera's closest
// zoom-in distance (or the camera can fly through it). The values below
// place one object per octant around the origin, with the star sharing the
// line's octant and the sphere taking the octant that frees up - change
// any of them freely, or delete/add entries - shape_count() and everything
// downstream just reads however many rows are in this table.
static const shape_instance_t scene_shapes[] = {
    { OBJECT_CUBE,           { 375.0f,  300.0f,  375.0f}, 1.0f, 2 },
    { OBJECT_SQUARE_PYRAMID, { 375.0f,  300.0f, -375.0f}, 1.0f, 3 },
    { OBJECT_TETRAHEDRON,    { 375.0f, -300.0f,  375.0f}, 1.0f, 4 },
    { OBJECT_LINE,           { 375.0f, -300.0f, -375.0f}, 1.0f, 0 },
    { OBJECT_STAR,           { 450.0f, -300.0f, -300.0f}, 0.0f, 1 },
    { OBJECT_OCTAGONAL_PRISM, {-375.0f,  300.0f,  375.0f}, 1.0f, 6 },
    { OBJECT_HOUSE,          {-375.0f,  300.0f, -375.0f}, 1.0f, 8 },
    { OBJECT_SPHERE,         {-375.0f, -300.0f,  375.0f}, 1.0f, 7 },
};

#define SCENE_TABLE_COUNT ((int)(sizeof(scene_shapes) / sizeof(scene_shapes[0])))

int shape_count(void)
{
    return SCENE_TABLE_COUNT;
}

int shape_is_star(int shape_index)
{
    return scene_shapes[shape_index].type == OBJECT_STAR;
}

int shape_edge_count(int shape_index)
{
    const shape_instance_t *instance = &scene_shapes[shape_index];
    if (instance->type == OBJECT_STAR) return 0;
    return mesh_for_object_type(instance->type)->edge_count;
}

int shape_face_count(int shape_index)
{
    const shape_instance_t *instance = &scene_shapes[shape_index];
    if (instance->type == OBJECT_STAR) return 0;
    return mesh_for_object_type(instance->type)->face_count;
}

void shape_edge(int shape_index, int edge_index, vec3_t *out_v0, vec3_t *out_v1)
{
    const shape_instance_t *instance = &scene_shapes[shape_index];
    const mesh_t *mesh = mesh_for_object_type(instance->type);
    int i0 = mesh->edge_vertices[edge_index][0];
    int i1 = mesh->edge_vertices[edge_index][1];

    *out_v0 = vec3_add(instance->center, vec3_scale(mesh->vertices[i0], instance->scale));
    *out_v1 = vec3_add(instance->center, vec3_scale(mesh->vertices[i1], instance->scale));
}

// Author: Thomas Dzubin
void shape_face(int shape_index, int face_index,
                 vec3_t out_vertices[SHAPE_MAX_FACE_VERTICES], int *out_vertex_count,
                 vec3_t *out_normal)
{
    const shape_instance_t *instance = &scene_shapes[shape_index];
    const mesh_t *mesh = mesh_for_object_type(instance->type);
    int count = mesh->face_vertex_count[face_index];
    int i;

    for (i = 0; i < count; i++) {
        vec3_t local = mesh->vertices[mesh->face_vertices[face_index][i]];
        out_vertices[i] = vec3_add(instance->center, vec3_scale(local, instance->scale));
    }
    *out_vertex_count = count;

    /* The first three vertices of any convex planar face already
       determine its plane, so this works for any vertex count - no
       per-mesh-type normal needs to be authored by hand. Scaling by a
       positive factor (see shape_instance_t.scale) never flips a face's
       winding, so this stays correct regardless of an object's size. */
    *out_normal = vec3_normalize(vec3_cross(vec3_sub(out_vertices[1], out_vertices[0]),
                                             vec3_sub(out_vertices[2], out_vertices[0])));
}

void shape_edge_faces(int shape_index, int edge_index, int *out_face0, int *out_face1)
{
    const mesh_t *mesh = mesh_for_object_type(scene_shapes[shape_index].type);
    *out_face0 = mesh->edge_faces[edge_index][0];
    *out_face1 = mesh->edge_faces[edge_index][1];
}

vec3_t shape_center(int shape_index)
{
    return scene_shapes[shape_index].center;
}

unsigned short shape_color(int shape_index)
{
    return FACE_COLOR_PALETTE[scene_shapes[shape_index].color];
}
