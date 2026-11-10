/**
 * @file primitive.c
 * @brief Implementation skeleton of the Primitive ADT (2D Vector Objects)
 *
 * Algoritmos e Estruturas de Dados 2026/2027
 * Departamento de Electrónica, Telecomunicações e Informática - UA
 */

/**
 * STUDENT NAME: Beatriz de Jesus Lopes
 * STUDENT NUMBER: 132384
 * STUDENT NAME: Matilde Freitas Moreira
 * STUDENT NUMBER: 132357
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

  if  (width <= 0.0 ||  height <=0.0 || alpha > 1.0 || alpha < 0.0 ){
    return NULL;
  }

  Primitive* p =
      primitive_allocate_common( id, PRIM_RECTANGLE, color, alpha, z_index);
  if (p == NULL) {
    return NULL;
  }

  p->geom.rectangle.width = width;
  p->geom.rectangle.height = height;
  p->geom.rectangle.top_left = top_left;

  primitive_update_aabb(p);

  return p;
}

Primitive* primitive_create_segment(int id, Point p1, Point p2, Color color,
                                    double alpha, int z_index) {
  
  if (alpha > 1.0 || alpha < 0.0){
    return NULL;
  } 
  
  Primitive* p =
      primitive_allocate_common(id, PRIM_SEGMENT, color, alpha, z_index);
  if (p == NULL){
    return NULL;
  }

  p->geom.segment.p1 = p1;
  p->geom.segment.p2 = p2;

  primitive_update_aabb(p);

  return p;
}

Primitive* primitive_create_polygon(int id, Color color, double alpha,
                                    int z_index) {
  if (alpha > 1.0 || alpha < 0.0){
    return NULL;
  }  
  Primitive* p =
      primitive_allocate_common(id, PRIM_POLYGON, color, alpha, z_index);
  if (p == NULL){
    return NULL;
  }

  primitive_update_aabb(p);

  return p;
}

void primitive_destroy(Primitive* p) {
  if (p == NULL){
    return;
  }
  if(p->type == PRIM_POLYGON){
    free(p->geom.polygon.vertices);
  }
  
  free(p);
}

// ============================================================================
// DYNAMIC ARRAY MANIPULATION (POLYGON) (STUDENTS MUST COMPLETE)
// ============================================================================

bool polygon_add_vertex(Primitive* p, Point pt) {
  if(p == NULL || p->type != PRIM_POLYGON){
    return false;
  }

  PolygonGeom* poly = &(p->geom.polygon);
  
  if (poly->num_vertices >= poly->capacity) {
      size_t new_capacity = (poly->capacity == 0) ? 4 : poly->capacity * 2;
      Point* new_vertices = (Point*)realloc(poly->vertices, new_capacity * sizeof(Point));
        
      if (new_vertices == NULL) {
          return false; // Allocation failed
      }
        
      poly->vertices = new_vertices;
      poly->capacity = new_capacity;
  }
  // Append the new vertex and increment count
  poly->vertices[poly->num_vertices++] = pt;

  // Mark the bounding box as dirty per your documentation
  p->is_aabb_dirty = true;

  return true;
}

bool polygon_remove_vertex(Primitive* p, size_t index) {
  if(p == NULL || p->type != PRIM_POLYGON || index >= p->geom.polygon.num_vertices){
    return false;
  }

  PolygonGeom* poly = &(p->geom.polygon);
  
  if (index >= poly->num_vertices) {
    return false;
  }

  for (size_t i = index; i < poly->num_vertices - 1; i++) {
      poly->vertices[i] = poly->vertices[i + 1];
  }
  
  poly->num_vertices--;

  p->is_aabb_dirty = true;

  return true;
}

// ============================================================================
// GEOMETRIC OPERATIONS AND TRANSFORMATIONS (STUDENTS MUST COMPLETE)
// ============================================================================

void primitive_update_aabb(Primitive* p) {
  if (p == NULL || !p->is_aabb_dirty) {
    return;
  }
  
  PolygonGeom* poly = &(p->geom.polygon);
  
  switch (p->type) {
    case PRIM_CIRCLE:
      p->aabb.min.x = p->geom.circle.center.x - p->geom.circle.radius;
      p->aabb.min.y = p->geom.circle.center.y - p->geom.circle.radius;
      p->aabb.max.x = p->geom.circle.center.x + p->geom.circle.radius;
      p->aabb.max.y = p->geom.circle.center.y + p->geom.circle.radius;
      break;

    case PRIM_RECTANGLE:
      p->aabb.min.x = p->geom.rectangle.top_left.x;
      p->aabb.min.y = p->geom.rectangle.top_left.y - p->geom.rectangle.height;
      p->aabb.max.x = p->geom.rectangle.top_left.x + p->geom.rectangle.width;
      p->aabb.max.y = p->geom.rectangle.top_left.y;
      break;

    case PRIM_SEGMENT:
      p->aabb.min.x = fmin(p->geom.segment.p1.x, p->geom.segment.p2.x);
      p->aabb.min.y = fmin(p->geom.segment.p1.y, p->geom.segment.p2.y);
      p->aabb.max.x = fmax(p->geom.segment.p1.x, p->geom.segment.p2.x);
      p->aabb.max.y = fmax(p->geom.segment.p1.y, p->geom.segment.p2.y);
      break;

    case PRIM_POLYGON:

      
      if (poly->num_vertices == 0) {
        //empty polygon
        p->aabb.min.x = 0;
        p->aabb.min.y = 0;
        p->aabb.max.x = 0;
        p->aabb.max.y = 0;
        break;
      }

      p->aabb.min.x = poly->vertices[0].x;//vertex 0
      p->aabb.min.y = poly->vertices[0].y;
      p->aabb.max.x = poly->vertices[0].x;
      p->aabb.max.y = poly->vertices[0].y;

      // cicle through the rest to find the lowest and highest x and y
      for (size_t i = 1; i < poly->num_vertices; i++) {
        if (poly->vertices[i].x < p->aabb.min.x) {
          p->aabb.min.x = poly->vertices[i].x;
        }
        if (poly->vertices[i].y < p->aabb.min.y) {
          p->aabb.min.y = poly->vertices[i].y;
        }
        if (poly->vertices[i].x > p->aabb.max.x) {
          p->aabb.max.x = poly->vertices[i].x;
        }
        if (poly->vertices[i].y > p->aabb.max.y) {
          p->aabb.max.y = poly->vertices[i].y;
        }
      }
      break;
  }

  p->is_aabb_dirty = false;
}

void primitive_translate(Primitive* p, double dx, double dy) {
  if (p == NULL) {
    return;
  }

  PolygonGeom* poly = &(p->geom.polygon);

  switch (p->type) {
    case PRIM_CIRCLE:
      p->geom.circle.center.x+=dx;
      p->geom.circle.center.y+=dy;
    break;

    case PRIM_RECTANGLE:
      p->geom.rectangle.top_left.x+=dx;
      p->geom.rectangle.top_left.y+=dy;
    break;

    case PRIM_SEGMENT:
      p->geom.segment.p1.x+=dx;
      p->geom.segment.p1.y+=dy;
      p->geom.segment.p2.x+=dx;
      p->geom.segment.p2.y+=dy;
    break;

    case PRIM_POLYGON:
      for (size_t i = 0; i < poly->num_vertices; i++) {
        poly->vertices[i].x += dx;
        poly->vertices[i].y += dy;
      }

    break;
  }

  p->is_aabb_dirty=true;
}

void primitive_scale(Primitive* p, double factor) {
  switch (p->type) {
    case PRIM_CIRCLE:
      /* o centro mantém-se, só o raio muda */
      p->geom.circle.radius *= factor;
      break;

    case PRIM_RECTANGLE: {
      /* escala em torno do centro do retângulo */
      RectGeom* r = &(p->geom.rectangle);
      double cx = r->top_left.x + r->width / 2.0;
      double cy = r->top_left.y + r->height / 2.0;
      r->width *= factor;
      r->height *= factor;
      r->top_left.x = cx - r->width / 2.0;
      r->top_left.y = cy - r->height / 2.0;
      break;
    }

    case PRIM_SEGMENT: {
      /* escala em torno do ponto médio */
      SegmentGeom* s = &(p->geom.segment);
      double cx = (s->p1.x + s->p2.x) / 2.0;
      double cy = (s->p1.y + s->p2.y) / 2.0;
      s->p1.x = cx + (s->p1.x - cx) * factor;
      s->p1.y = cy + (s->p1.y - cy) * factor;
      s->p2.x = cx + (s->p2.x - cx) * factor;
      s->p2.y = cy + (s->p2.y - cy) * factor;
      break;
    }

    case PRIM_POLYGON: {
      PolygonGeom* poly = &(p->geom.polygon);
      if (poly->num_vertices == 0) {
        break;
      }
      /* centro da AABB atual (atualizada primeiro, pode estar desatualizada) */
      BoundingBox box = primitive_get_aabb(p);
      double cx = (box.min.x + box.max.x) / 2.0;
      double cy = (box.min.y + box.max.y) / 2.0;
      for (size_t i = 0; i < poly->num_vertices; i++) {
        poly->vertices[i].x = cx + (poly->vertices[i].x - cx) * factor;
        poly->vertices[i].y = cy + (poly->vertices[i].y - cy) * factor;
      }
      break;
    }
  }

  p->is_aabb_dirty = true;
}

bool primitive_contains_point(const Primitive* p, Point pt) {
  if (p == NULL) {
    return false;
  }

  const PolygonGeom* poly = &(p->geom.polygon);

  // Narrow-phase exact geometric test dispatch
  switch (p->type) {
    case PRIM_CIRCLE:
      return math_circle_contains_point(p->geom.circle.center,
                                        p->geom.circle.radius, pt);

    case PRIM_RECTANGLE:
      return math_rect_contains_point(p->geom.rectangle.top_left, p->geom.rectangle.width,
                                       p->geom.rectangle.height, pt);

    case PRIM_SEGMENT:
      return math_segment_contains_point(p->geom.segment.p1, p->geom.segment.p2, 1.0
                                         , pt);

    case PRIM_POLYGON:
      return math_polygon_contains_point(poly->vertices, poly->num_vertices, pt);
  }

  return false;
}