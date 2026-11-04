/**
 * @file primitive.h
 * @brief Definition of the Primitive ADT (2D Vector Objects)
 *
 * Module defining geometric primitives, attributes, and operations on 2D
 * vector shapes (Circle, Rectangle, Line Segment, and Polygon).
 */

#ifndef PRIMITIVE_H
#define PRIMITIVE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// ============================================================================
// OPAQUE STRUCTURE HANDLE
// ============================================================================

/**
 * @brief Opaque handle to a Primitive instance.
 *
 * Concrete struct layout is hidden in primitive.c to enforce encapsulation.
 */
typedef struct _Primitive Primitive;

// ============================================================================
// AUXILIARY SIMPLE STRUCTURES
// ============================================================================

/**
 * @brief Color in RGB format (0-255 per channel).
 */
typedef struct {
  uint8_t r; /**< Red channel value [0..255] */
  uint8_t g; /**< Green channel value [0..255] */
  uint8_t b; /**< Blue channel value [0..255] */
} Color;

/**
 * @brief Point in 2D Cartesian plane.
 */
typedef struct {
  double x; /**< Horizontal coordinate */
  double y; /**< Vertical coordinate */
} Point;

/**
 * @brief Axis-Aligned Bounding Box (AABB).
 */
typedef struct {
  Point min; /**< Bottom-left corner (x_min, y_min) */
  Point max; /**< Top-right corner (x_max, y_max) */
} BoundingBox;

// ============================================================================
// PRIMITIVE TYPES
// ============================================================================

/**
 * @brief Primitive type identifier.
 */
typedef enum {
  PRIM_CIRCLE,    /**< Circle shape defined by center and radius */
  PRIM_RECTANGLE, /**< Rectangle defined by top-left corner, width, height */
  PRIM_SEGMENT,   /**< Line segment defined by two endpoints */
  PRIM_POLYGON    /**< Closed polygon defined by dynamic vertex array */
} PrimitiveType;

// ============================================================================
// CREATION AND DESTRUCTION
// ============================================================================

/**
 * @brief Creates and dynamically allocates a Circle primitive.
 *
 * @param id Unique identifier for the primitive.
 * @param center Center point coordinates.
 * @param radius Circle radius (must be > 0.0).
 * @param color Base RGB color.
 * @param alpha Opacity degree [0.0 = transparent, 1.0 = opaque].
 * @param z_index Depth layer index.
 * @return Pointer to allocated Primitive, or NULL on allocation failure.
 * @pre radius > 0.0 && alpha >= 0.0 && alpha <= 1.0
 */
Primitive* primitive_create_circle(int id, Point center, double radius,
                                   Color color, double alpha, int z_index);

/**
 * @brief Creates and dynamically allocates a Rectangle primitive.
 *
 * @param id Unique identifier for the primitive.
 * @param top_left Top-left corner coordinates.
 * @param width Rectangle width (must be > 0.0).
 * @param height Rectangle height (must be > 0.0).
 * @param color Base RGB color.
 * @param alpha Opacity degree [0.0 = transparent, 1.0 = opaque].
 * @param z_index Depth layer index.
 * @return Pointer to allocated Primitive, or NULL on allocation failure.
 * @pre width > 0.0 && height > 0.0 && alpha >= 0.0 && alpha <= 1.0
 */
Primitive* primitive_create_rectangle(int id, Point top_left, double width,
                                      double height, Color color, double alpha,
                                      int z_index);

/**
 * @brief Creates and dynamically allocates a Segment primitive.
 *
 * @param id Unique identifier for the primitive.
 * @param p1 Start endpoint coordinates.
 * @param p2 End endpoint coordinates.
 * @param color Base RGB color.
 * @param alpha Opacity degree [0.0 = transparent, 1.0 = opaque].
 * @param z_index Depth layer index.
 * @return Pointer to allocated Primitive, or NULL on allocation failure.
 * @pre alpha >= 0.0 && alpha <= 1.0
 */
Primitive* primitive_create_segment(int id, Point p1, Point p2, Color color,
                                    double alpha, int z_index);

/**
 * @brief Creates and dynamically allocates an empty Polygon primitive.
 *
 * @param id Unique identifier for the primitive.
 * @param color Base RGB color.
 * @param alpha Opacity degree [0.0 = transparent, 1.0 = opaque].
 * @param z_index Depth layer index.
 * @return Pointer to allocated Primitive, or NULL on allocation failure.
 * @pre alpha >= 0.0 && alpha <= 1.0
 * @note Initializes vertices array to NULL with capacity 0 and num_vertices 0.
 */
Primitive* primitive_create_polygon(int id, Color color, double alpha,
                                    int z_index);

/**
 * @brief Frees all memory associated with a primitive structure.
 *
 * @param p Pointer to Primitive structure to be destroyed.
 * @note Safe to call with NULL. Frees internal polygon vertices array.
 */
void primitive_destroy(Primitive* p);

// ============================================================================
// DYNAMIC ARRAY MANIPULATION (POLYGON)
// ============================================================================

/**
 * @brief Appends a new vertex to the polygon primitive.
 *
 * Reallocates internal vertices array if capacity is reached and sets
 * is_aabb_dirty to true.
 *
 * @param p Pointer to Primitive structure (must be of type PRIM_POLYGON).
 * @param pt 2D Point coordinates to append.
 * @return true if vertex was added, false on failure or invalid primitive.
 * @pre p != NULL && p->type == PRIM_POLYGON
 * @note Sets p->is_aabb_dirty = true.
 */
bool polygon_add_vertex(Primitive* p, Point pt);

/**
 * @brief Removes a vertex from polygon array by index and shifts elements.
 *
 * @param p Pointer to Primitive structure (must be of type PRIM_POLYGON).
 * @param index 0-based index of vertex to remove.
 * @return true if vertex was removed, false if index is out of bounds.
 * @pre p != NULL && p->type == PRIM_POLYGON && index <
 * p->geom.polygon.num_vertices
 * @note Shifts remaining vertices left and sets p->is_aabb_dirty = true.
 */
bool polygon_remove_vertex(Primitive* p, size_t index);

// ============================================================================
// INSPECTION / GETTER FUNCTIONS
// ============================================================================

/**
 * @brief Get unique primitive id.
 *
 * @param p Pointer to Primitive structure.
 * @return Unique primitive id (id >= 0), or -1 if p is NULL
 */
int primitive_get_id(const Primitive* p);

/**
 * @brief Get the geometric type of the primitive.
 *
 * @param p Pointer to Primitive structure.
 * @return Primitive type (PRIM_CIRCLE, PRIM_RECTANGLE, etc.).
 * @pre p != NULL
 */
PrimitiveType primitive_get_type(const Primitive* p);

/**
 * @brief Get the primitive depth index (z_index).
 *
 * @param p Pointer to Primitive structure.
 * @return z_index value, or 0 if p is NULL.
 */
int primitive_get_z_index(const Primitive* p);

/**
 * @brief Get the primitive background RGB color.
 *
 * @param p Pointer to Primitive structure.
 * @return Color RGB struct, or black (0,0,0) if p is NULL.
 */
Color primitive_get_color(const Primitive* p);

/**
 * @brief Get the primitive opacity value (alpha).
 *
 * @param p Pointer to Primitive structure.
 * @return Alpha value in [0.0..1.0], or 0.0 if p is NULL.
 */
double primitive_get_alpha(const Primitive* p);

/**
 * @brief Get the updated bounding box (AABB) for the primitive.
 *
 * If AABB is marked as "dirty", forces its update before
 * returning the value.
 *
 * @param p Pointer to Primitive structure.
 * @return BoundingBox struct with min/max coordinates.
 * @pre p != NULL
 */
BoundingBox primitive_get_aabb(Primitive* p);

/**
 * @brief Get the current number of vertices of a Polygon type primitive.
 *
 * @param p Pointer to Primitive structure.
 * @return Vertex count, or 0 if p is NULL or not a Polygon.
 */
size_t polygon_get_num_vertices(const Primitive* p);

// ============================================================================
// GEOMETRIC OPERATIONS AND TRANSFORMATIONS
// ============================================================================

/**
 * @brief Recalculates primitive AABB if is_aabb_dirty is true.
 *
 * @param p Pointer to Primitive structure.
 * @note Sets p->is_aabb_dirty = false upon completion.
 */
void primitive_update_aabb(Primitive* p);

/**
 * @brief Translates primitive geometry by displacement vector (dx, dy).
 *
 * @param p Pointer to Primitive structure.
 * @param dx Horizontal displacement.
 * @param dy Vertical displacement.
 * @note Sets p->is_aabb_dirty = true.
 */
void primitive_translate(Primitive* p, double dx, double dy);

/**
 * @brief Scales primitive geometry relative to its origin/center.
 *
 * @param p Pointer to Primitive structure.
 * @param factor Scaling factor (must be > 0.0).
 * @pre p != NULL && factor > 0.0
 * @note Sets p->is_aabb_dirty = true.
 */
void primitive_scale(Primitive* p, double factor);

/**
 * @brief Checks if a 2D point is contained within the primitive.
 *
 * @param p Pointer to Primitive structure.
 * @param pt 2D Point coordinates to test.
 * @return true if point is inside or on boundary, false otherwise.
 * @note Performs narrow-phase exact geometric test.
 */
bool primitive_contains_point(const Primitive* p, Point pt);

#endif  // PRIMITIVE_H
