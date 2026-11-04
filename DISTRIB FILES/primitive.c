/**
 * @file primitive.c
 * @brief Implementation skeleton of the Primitive ADT (2D Vector Objects)
 *
 * Algoritmos e Estruturas de Dados 2026/2027
 * Departamento de Electrónica, Telecomunicações e Informática - UA
 */

/**
 * STUDENT NAME:
 * STUDENT NUMBER:
 * STUDENT NAME:
 * STUDENT NUMBER:
 */

#include "primitive.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "primitive_internal.h"

// ============================================================================
// HELPER GEOMETRIC MATH FUNCTIONS (PROVIDED)
// ============================================================================

/**
 * @brief Tests exact containment of a point inside a circle.
 */
static bool math_circle_contains_point(Point center, double radius, Point pt) {
  double dx = pt.x - center.x;
  double dy = pt.y - center.y;
  return (dx * dx + dy * dy) <= (radius * radius);
}

/**
 * @brief Tests exact containment of a point inside an axis-aligned rectangle.
 */
static bool math_rect_contains_point(Point top_left, double width,
                                     double height, Point pt) {
  return (pt.x >= top_left.x && pt.x <= (top_left.x + width) &&
          pt.y >= top_left.y && pt.y <= (top_left.y + height));
}

/**
 * @brief Tests exact containment of a point near line segment (with tolerance).
 */
static bool math_segment_contains_point(Point p1, Point p2, double tolerance,
                                        Point pt) {
  double l2 = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
  if (l2 == 0.0) {
    double dx = pt.x - p1.x;
    double dy = pt.y - p1.y;
    return sqrt(dx * dx + dy * dy) <= tolerance;
  }
  double t =
      ((pt.x - p1.x) * (p2.x - p1.x) + (pt.y - p1.y) * (p2.y - p1.y)) / l2;
  if (t < 0.0) {
    t = 0.0;
  } else if (t > 1.0) {
    t = 1.0;
  }

  double proj_x = p1.x + t * (p2.x - p1.x);
  double proj_y = p1.y + t * (p2.y - p1.y);
  double dx = pt.x - proj_x;
  double dy = pt.y - proj_y;

  return sqrt(dx * dx + dy * dy) <= tolerance;
}

/**
 * @brief Tests containment of point inside polygon using Ray-Casting.
 */
static bool math_polygon_contains_point(const Point* vertices,
                                        size_t num_vertices, Point pt) {
  if (num_vertices < 3 || vertices == NULL) {
    return false;
  }

  bool inside = false;
  for (size_t i = 0, j = num_vertices - 1; i < num_vertices; j = i++) {
    double xi = vertices[i].x, yi = vertices[i].y;
    double xj = vertices[j].x, yj = vertices[j].y;

    bool intersect = ((yi > pt.y) != (yj > pt.y)) &&
                     (pt.x < (xj - xi) * (pt.y - yi) / (yj - yi) + xi);
    if (intersect) {
      inside = !inside;
    }
  }
  return inside;
}

// ============================================================================
// INTERNAL AUXILIARY FUNCTIONS (PROVIDED)
// ============================================================================

/**
 * @brief Allocates base Primitive structure and initializes common fields.
 */
static Primitive* primitive_allocate_common(int id, PrimitiveType type,
                                            Color color, double alpha,
                                            int z_index) {
  Primitive* p = calloc(1, sizeof(Primitive));
  if (p == NULL) {
    return NULL;
  }

  p->id = id;
  p->type = type;
  p->color = color;
  p->alpha = alpha;
  p->z_index = z_index;
  p->is_aabb_dirty = true;
  p->aabb.min = (Point){0.0, 0.0};
  p->aabb.max = (Point){0.0, 0.0};

  return p;
}

// ============================================================================
// INSPECTION / GETTER FUNCTIONS (PROVIDED TO STUDENTS)
// ============================================================================

int primitive_get_id(const Primitive* p) {
  if (p == NULL) return -1;
  return p->id;
}

PrimitiveType primitive_get_type(const Primitive* p) {
  assert(p != NULL && "Primitive pointer is NULL");

  return p->type;
}

int primitive_get_z_index(const Primitive* p) {
  if (p == NULL) return 0;
  return p->z_index;
}

Color primitive_get_color(const Primitive* p) {
  if (p == NULL) {
    Color black = {0, 0, 0};
    return black;
  }
  return p->color;
}

double primitive_get_alpha(const Primitive* p) {
  if (p == NULL) return 0.0;
  return p->alpha;
}

BoundingBox primitive_get_aabb(Primitive* p) {
  assert(p != NULL && "Primitive pointer is NULL");

  // Force AABB update
  if (p->is_aabb_dirty) {
    primitive_update_aabb(p);
  }
  return p->aabb;
}

size_t polygon_get_num_vertices(const Primitive* p) {
  if (p == NULL || p->type != PRIM_POLYGON) return 0;
  return p->geom.polygon.num_vertices;
}

// ============================================================================
// CREATION AND DESTRUCTION FUNCTIONS (STUDENTS MUST COMPLETE)
// ============================================================================

/**
 * [PROVIDED AS REFERENCE MODEL]
 * Creates and dynamically allocates a Circle primitive.
 */
Primitive* primitive_create_circle(int id, Point center, double radius,
                                   Color color, double alpha, int z_index) {
  if (radius <= 0.0 || alpha < 0.0 || alpha > 1.0) {
    return NULL;
  }

  Primitive* p =
      primitive_allocate_common(id, PRIM_CIRCLE, color, alpha, z_index);
  if (p == NULL) {
    return NULL;
  }

  p->geom.circle.center = center;
  p->geom.circle.radius = radius;

  // Automatically calculate initial AABB
  primitive_update_aabb(p);

  return p;
}

Primitive* primitive_create_rectangle(int id, Point top_left, double width,
                                      double height, Color color, double alpha,
                                      int z_index) {
  /* TODO: Student Implementation
   */
  (void)id;
  (void)top_left;
  (void)width;
  (void)height;
  (void)color;
  (void)alpha;
  (void)z_index;
  return NULL;
}

Primitive* primitive_create_segment(int id, Point p1, Point p2, Color color,
                                    double alpha, int z_index) {
  /* TODO: Student Implementation
   */
  (void)id;
  (void)p1;
  (void)p2;
  (void)color;
  (void)alpha;
  (void)z_index;
  return NULL;
}

Primitive* primitive_create_polygon(int id, Color color, double alpha,
                                    int z_index) {
  /* TODO: Student Implementation
   */
  (void)id;
  (void)color;
  (void)alpha;
  (void)z_index;
  return NULL;
}

void primitive_destroy(Primitive* p) {
  /* TODO: Student Implementation
   */
  (void)p;
}

// ============================================================================
// DYNAMIC ARRAY MANIPULATION (POLYGON) (STUDENTS MUST COMPLETE)
// ============================================================================

bool polygon_add_vertex(Primitive* p, Point pt) {
  /* TODO: Student Implementation
   */
  (void)p;
  (void)pt;
  return false;
}

bool polygon_remove_vertex(Primitive* p, size_t index) {
  /* TODO: Student Implementation
   */
  (void)p;
  (void)index;
  return false;
}

// ============================================================================
// GEOMETRIC OPERATIONS AND TRANSFORMATIONS (STUDENTS MUST COMPLETE)
// ============================================================================

void primitive_update_aabb(Primitive* p) {
  if (p == NULL || !p->is_aabb_dirty) {
    return;
  }

  switch (p->type) {
    case PRIM_CIRCLE:
      p->aabb.min.x = p->geom.circle.center.x - p->geom.circle.radius;
      p->aabb.min.y = p->geom.circle.center.y - p->geom.circle.radius;
      p->aabb.max.x = p->geom.circle.center.x + p->geom.circle.radius;
      p->aabb.max.y = p->geom.circle.center.y + p->geom.circle.radius;
      break;

    case PRIM_RECTANGLE:
      /* TODO: Compute AABB for Rectangle
       */
      break;

    case PRIM_SEGMENT:
      /* TODO: Compute AABB for Segment
       */
      break;

    case PRIM_POLYGON:
      /* TODO: Compute AABB for Polygon
       * Iterate vertices array to find min/max coordinates
       */
      break;
  }

  p->is_aabb_dirty = false;
}

void primitive_translate(Primitive* p, double dx, double dy) {
  if (p == NULL) {
    return;
  }

  /* TODO: Student Implementation
   * Translate geometry based on p->type:
   * - PRIM_CIRCLE: translate the center
   * - PRIM_RECTANGLE: translate top-left vertex
   * - PRIM_SEGMENT: translate both endpoints
   * - PRIM_POLYGON: translate each vertex
   * Set p->is_aabb_dirty = true
   */
  (void)dx;
  (void)dy;
}

void primitive_scale(Primitive* p, double factor) {
  if (p == NULL || factor <= 0.0) {
    return;
  }

  /* TODO: Student Implementation
   * Scale geometry based on p->type:
   * - PRIM_CIRCLE: radius *= factor
   * - PRIM_RECTANGLE: width *= factor, height *= factor
   * - PRIM_SEGMENT: scale endpoints relative to midpoint
   * - PRIM_POLYGON: scale vertices relative to polygon bounding box center
   * Set p->is_aabb_dirty = true
   */
  (void)factor;
}

bool primitive_contains_point(const Primitive* p, Point pt) {
  if (p == NULL) {
    return false;
  }

  // Narrow-phase exact geometric test dispatch
  switch (p->type) {
    case PRIM_CIRCLE:
      return math_circle_contains_point(p->geom.circle.center,
                                        p->geom.circle.radius, pt);

    case PRIM_RECTANGLE:
      /* TODO: Does rectangle contain point?
       */
      return false;

    case PRIM_SEGMENT:
      /* TODO: Does segment contain point?
       */
      return false;

    case PRIM_POLYGON:
      /* TODO: Does polygon contain point?
       */
      return false;
  }

  return false;
}
