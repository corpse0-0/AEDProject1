/**
 * @file vector_scene.h
 * @brief Definition of the VectorScene ADT (2D Vector Scene)
 *
 * Module managing collections of vector objects, depth sorting, spatial
 * selection, file I/O, and raster rendering.
 */

#ifndef VECTOR_SCENE_H
#define VECTOR_SCENE_H

#include <stdbool.h>
#include <stddef.h>

#include "primitive.h"

// ============================================================================
// OPAQUE STRUCTURE HANDLE
// ============================================================================

/**
 * @brief Opaque handle to a VectorScene instance.
 *
 * Concrete struct layout is hidden in vector_scene.c to enforce encapsulation.
 */
typedef struct _VectorScene VectorScene;

// ============================================================================
// PERFORMANCE METRICS STRUCTURES
// ============================================================================

/**
 * @brief Statistics counter for depth sorting algorithms.
 */
typedef struct {
  unsigned long comparisons; /**< Number of element comparisons made */
  unsigned long swaps;       /**< Number of element swaps/moves made */
} SortStats;

/**
 * @brief Statistics counter for spatial picking operations.
 */
typedef struct {
  unsigned long aabb_tests;  /**< Number of Broad-phase AABB checks */
  unsigned long exact_tests; /**< Number of Narrow-phase exact checks */
} PickStats;

// ============================================================================
// SCENE LIFECYCLE
// ============================================================================

/**
 * @brief Allocates and initializes a new vector scene.
 *
 * @param width Canvas width in pixels (must be > 0).
 * @param height Canvas height in pixels (must be > 0).
 * @param bg_color Default background RGB color.
 * @return Pointer to allocated VectorScene handle, or NULL on failure.
 * @pre width > 0 && height > 0
 */
VectorScene* vector_scene_create(int width, int height, Color bg_color);

/**
 * @brief Frees all memory associated with scene and all owned primitives.
 *
 * @param scene Pointer to VectorScene handle.
 * @note Safe to call with NULL. Calls primitive_destroy on every primitive.
 */
void vector_scene_destroy(VectorScene* scene);

// ============================================================================
// ACCESSORS (GETTERS FOR OPAQUE ADT)
// ============================================================================

/**
 * @brief Gets canvas width of the scene.
 *
 * @param scene Pointer to VectorScene handle.
 * @return Canvas width in pixels, or 0 if scene is NULL.
 */
int vector_scene_get_width(const VectorScene* scene);

/**
 * @brief Gets canvas height of the scene.
 *
 * @param scene Pointer to VectorScene handle.
 * @return Canvas height in pixels, or 0 if scene is NULL.
 */
int vector_scene_get_height(const VectorScene* scene);

/**
 * @brief Gets background color of the scene.
 *
 * @param scene Pointer to VectorScene handle.
 * @return RGB Color structure of background, or black (0,0,0) if NULL.
 */
Color vector_scene_get_background_color(const VectorScene* scene);

/**
 * @brief Gets current number of primitives in the scene.
 *
 * @param scene Pointer to VectorScene handle.
 * @return Primitive count, or 0 if scene is NULL.
 */
size_t vector_scene_get_count(const VectorScene* scene);

/**
 * @brief Retrieves pointer to primitive by its array index.
 *
 * @param scene Pointer to VectorScene handle.
 * @param index 0-based index in primitive array.
 * @return Pointer to Primitive, or NULL if out of bounds or scene is NULL.
 * @pre index < vector_scene_get_count(scene)
 */
Primitive* vector_scene_get_primitive_by_index(const VectorScene* scene,
                                               size_t index);

/**
 * @brief Searches for a primitive in the scene by its unique ID.
 *
 * @param scene Pointer to VectorScene handle.
 * @param id Unique primitive identifier.
 * @return Pointer to Primitive if found, or NULL if not found or scene is NULL.
 */
Primitive* vector_scene_get_primitive_by_id(const VectorScene* scene, int id);

// ============================================================================
// COLLECTION MANAGEMENT
// ============================================================================

/**
 * @brief Adds a primitive to the scene.
 *
 * @param scene Pointer to VectorScene handle.
 * @param p Pointer to Primitive to add.
 * @return Assigned primitive ID (>= 0), or -1 on error.
 * @note Scene TAKES OWNERSHIP of pointer p. Caller must NOT free p.
 */
int vector_scene_add_primitive(VectorScene* scene, Primitive* p);

/**
 * @brief Removes a primitive from scene by ID and frees its memory.
 *
 * @param scene Pointer to VectorScene handle.
 * @param id Unique identifier of primitive to remove.
 * @return true if primitive was removed, false if ID not found or scene NULL.
 * @note Calls primitive_destroy on removed primitive and shifts array.
 */
bool vector_scene_remove_primitive(VectorScene* scene, int id);

// ============================================================================
// MANIPULATION AND TRANSFORMATIONS
// ============================================================================

/**
 * @brief Translates primitive with specified ID by (dx, dy).
 *
 * @param scene Pointer to VectorScene handle.
 * @param id Unique primitive identifier.
 * @param dx Horizontal displacement.
 * @param dy Vertical displacement.
 * @return true if primitive was found and translated, false otherwise.
 */
bool vector_scene_translate_primitive(VectorScene* scene, int id, double dx,
                                      double dy);

/**
 * @brief Modifies depth layer (z_index) of a primitive.
 *
 * @param scene Pointer to VectorScene handle.
 * @param id Unique primitive identifier.
 * @param new_z_index New depth layer index.
 * @return true if primitive was found and updated, false otherwise.
 */
bool vector_scene_set_z_index(VectorScene* scene, int id, int new_z_index);

// ============================================================================
// SPATIAL SELECTION AND SORTING (WITH PERFORMANCE INSTRUMENTATION)
// ============================================================================

/**
 * @brief Sorts primitives in scene by ascending z_index and updates stats.
 *
 * @param scene Pointer to VectorScene handle.
 * @param use_fast_sort If true uses O(N log N) sort; if false O(N^2) sort.
 * @param stats Pointer to SortStats structure to populate.
 * @note Operates on Primitive* pointer array. Resets stats before sorting.
 */
void vector_scene_sort_by_z(VectorScene* scene, bool use_fast_sort,
                            SortStats* stats);

/**
 * @brief Selects topmost visible primitive containing point using 2 phases.
 *
 * @param scene Pointer to VectorScene handle.
 * @param pt 2D Point coordinates to test.
 * @param is_opt Flag indicating if the optimized (two-step) approach is used.
 * @param stats Pointer to PickStats structure to populate.
 * @return Pointer to topmost Primitive containing pt, or NULL if none.
 * @note Phase 1: AABB test (stats->aabb_tests), Phase 2: Exact
 * (stats->exact_tests).
 */
Primitive* vector_scene_pick_top_primitive(const VectorScene* scene, Point pt,
                                           bool is_opt, PickStats* stats);

// ============================================================================
// RENDERING / RASTERIZATION
// ============================================================================

/**
 * @brief Generates final raster image file applying Painter's Algorithm.
 *
 * @param scene Pointer to VectorScene handle.
 * @param output_filename Output image file path (e.g., "output.ppm").
 * @return true if image rendered and written successfully, false on error.
 */
bool vector_scene_render(const VectorScene* scene, const char* output_filename);

// ============================================================================
// FILE INPUT / OUTPUT
// ============================================================================

/**
 * @brief Saves vector scene configuration and primitives to text file (.vsc).
 *
 * @param scene Pointer to VectorScene handle.
 * @param filename Output text file path.
 * @return true if saved successfully, false on I/O error.
 */
bool vector_scene_save_text(const VectorScene* scene, const char* filename);

/**
 * @brief Loads and constructs a new VectorScene from text file (.vsc).
 *
 * @param filename Input text file path.
 * @return Pointer to new VectorScene handle, or NULL on error.
 */
VectorScene* vector_scene_load_text(const char* filename);

#endif  // VECTOR_SCENE_H
