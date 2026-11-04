/**
 * @file primitive_internal.h
 * @brief Internal primitive details, not to be seen by clients
 */

#ifndef PRIMITIVE_INTERNAL_H
#define PRIMITIVE_INTERNAL_H

#include "primitive.h"  // Inclui Point, Color, BoundingBox, PrimitiveType

// ============================================================================
// PRIMITIVE TYPES AND TAGGED UNION
// ============================================================================

/**
 * @brief Specific geometry data for a Circle.
 */
typedef struct {
  Point center;  /**< Center point of the circle */
  double radius; /**< Radius of the circle (must be > 0) */
} CircleGeom;

/**
 * @brief Specific geometry data for a Rectangle.
 */
typedef struct {
  Point top_left; /**< Top-left corner coordinates */
  double width;   /**< Width of rectangle (must be > 0) */
  double height;  /**< Height of rectangle (must be > 0) */
} RectGeom;

/**
 * @brief Specific geometry data for a Line Segment.
 */
typedef struct {
  Point p1; /**< Start endpoint coordinates */
  Point p2; /**< End endpoint coordinates */
} SegmentGeom;

/**
 * @brief Specific geometry data for a Polygon (Dynamic Array of Points).
 */
typedef struct {
  Point* vertices;     /**< Dynamic array of vertex coordinates */
  size_t num_vertices; /**< Current number of vertices in array */
  size_t capacity;     /**< Allocated capacity for vertices array */
} PolygonGeom;

/**
 * @brief Unified Vector Primitive Structure (Tagged Union).
 */
struct _Primitive {
  int id;             /**< Unique primitive identifier */
  PrimitiveType type; /**< Type tag indicating active union member */
  Color color;        /**< Base color (RGB) */
  double alpha;       /**< Opacity degree [0.0 = transparent, 1.0 = opaque] */
  int z_index;        /**< Depth layer index (higher = rendered on top) */
  BoundingBox aabb;   /**< Bounding box for fast spatial tests */
  bool is_aabb_dirty; /**< Invalidation flag: true if AABB needs update */

  /** Union containing geometry specific to active 'type' */
  union {
    CircleGeom circle;   /**< Active when type == PRIM_CIRCLE */
    RectGeom rectangle;  /**< Active when type == PRIM_RECTANGLE */
    SegmentGeom segment; /**< Active when type == PRIM_SEGMENT */
    PolygonGeom polygon; /**< Active when type == PRIM_POLYGON */
  } geom;
};

#endif  // PRIMITIVE_INTERNAL_H
