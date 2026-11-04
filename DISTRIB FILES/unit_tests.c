/**
 * @file unit_tests.c
 * @brief Unit test suite for Primitive and VectorScene ADTs (Phases 1-5)
 *
 * Algoritmos e Estruturas de Dados 2026/2027
 * Departamento de Electrónica, Telecomunicações e Informática - UA
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "primitive_internal.h"
#include "vector_scene.h"

// ============================================================================
// LIGHTWEIGHT UNIT TEST FRAMEWORK
// ============================================================================

static int g_tests_run = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg)                                        \
  do {                                                                \
    g_tests_run++;                                                    \
    if (cond) {                                                       \
      g_tests_passed++;                                               \
    } else {                                                          \
      g_tests_failed++;                                               \
      printf("  [FAIL] %s (Line %d): %s\n", __func__, __LINE__, msg); \
    }                                                                 \
  } while (0)

#define TEST_ASSERT_NOT_NULL(ptr, msg) TEST_ASSERT((ptr) != NULL, msg)
#define TEST_ASSERT_NULL(ptr, msg) TEST_ASSERT((ptr) == NULL, msg)

#define TEST_ASSERT_INT_EQ(expected, actual, msg)                         \
  do {                                                                    \
    int _exp = (expected);                                                \
    int _act = (actual);                                                  \
    char _buf[256];                                                       \
    snprintf(_buf, sizeof(_buf), "%s (Expected: %d, Got: %d)", msg, _exp, \
             _act);                                                       \
    TEST_ASSERT(_exp == _act, _buf);                                      \
  } while (0)

#define TEST_ASSERT_SIZE_EQ(expected, actual, msg)                          \
  do {                                                                      \
    size_t _exp = (expected);                                               \
    size_t _act = (actual);                                                 \
    char _buf[256];                                                         \
    snprintf(_buf, sizeof(_buf), "%s (Expected: %zu, Got: %zu)", msg, _exp, \
             _act);                                                         \
    TEST_ASSERT(_exp == _act, _buf);                                        \
  } while (0)

#define TEST_ASSERT_DOUBLE_NEAR(expected, actual, tol, msg)                   \
  do {                                                                        \
    double _exp = (expected);                                                 \
    double _act = (actual);                                                   \
    double _diff = fabs(_exp - _act);                                         \
    char _buf[256];                                                           \
    snprintf(_buf, sizeof(_buf), "%s (Expected: %.3f, Got: %.3f)", msg, _exp, \
             _act);                                                           \
    TEST_ASSERT(_diff <= (tol), _buf);                                        \
  } while (0)

// ============================================================================
// PHASE 1 TESTS: CONSTRUCTORS & DESTRUCTION (PRIMITIVE)
// ============================================================================

static void test_phase1_primitive_constructors(void) {
  printf("\n--- Running Phase 1 Tests: Constructors & Destruction ---\n");

  Color red = {255, 0, 0};
  Point center = {10.0, 20.0};
  Primitive* c = primitive_create_circle(1, center, 5.0, red, 1.0, 10);
  TEST_ASSERT_NOT_NULL(c, "Circle allocation failed");
  if (c) {
    TEST_ASSERT_INT_EQ(1, c->id, "Circle ID match");
    TEST_ASSERT_INT_EQ(PRIM_CIRCLE, c->type, "Circle type tag match");
    TEST_ASSERT_DOUBLE_NEAR(10.0, c->geom.circle.center.x, 0.001,
                            "Circle center x");
    TEST_ASSERT_DOUBLE_NEAR(5.0, c->geom.circle.radius, 0.001, "Circle radius");
    primitive_destroy(c);
  }

  Point top_left = {5.0, 5.0};
  Primitive* r =
      primitive_create_rectangle(2, top_left, 20.0, 10.0, red, 0.8, 5);
  TEST_ASSERT_NOT_NULL(r, "Rectangle allocation failed");
  if (r) {
    TEST_ASSERT_INT_EQ(2, r->id, "Rectangle ID match");
    TEST_ASSERT_INT_EQ(PRIM_RECTANGLE, r->type, "Rectangle type tag match");
    TEST_ASSERT_DOUBLE_NEAR(20.0, r->geom.rectangle.width, 0.001,
                            "Rectangle width");
    primitive_destroy(r);
  }

  Point p1 = {0.0, 0.0}, p2 = {10.0, 10.0};
  Primitive* s = primitive_create_segment(3, p1, p2, red, 1.0, 2);
  TEST_ASSERT_NOT_NULL(s, "Segment allocation failed");
  if (s) {
    TEST_ASSERT_INT_EQ(3, s->id, "Segment ID match");
    TEST_ASSERT_INT_EQ(PRIM_SEGMENT, s->type, "Segment type tag match");
    primitive_destroy(s);
  }

  Primitive* poly = primitive_create_polygon(4, red, 0.5, 1);
  TEST_ASSERT_NOT_NULL(poly, "Polygon allocation failed");
  if (poly) {
    TEST_ASSERT_INT_EQ(4, poly->id, "Polygon ID match");
    TEST_ASSERT_INT_EQ(PRIM_POLYGON, poly->type, "Polygon type tag match");
    TEST_ASSERT_SIZE_EQ(0, poly->geom.polygon.num_vertices,
                        "Initial vertices count == 0");
    primitive_destroy(poly);
  }

  // Invalid parameters check
  Primitive* invalid = primitive_create_circle(5, center, -2.0, red, 1.0, 1);
  TEST_ASSERT_NULL(invalid, "Circle rejects negative radius");

  primitive_destroy(NULL);  // Safe destroy check
}

// ============================================================================
// PHASE 2 TESTS: DYNAMIC VERTEX ARRAY MANIPULATION (POLYGON)
// ============================================================================

static void test_phase2_polygon_manipulation(void) {
  printf("\n--- Running Phase 2 Tests: Polygon Dynamic Array ---\n");

  Color blue = {0, 0, 255};
  Primitive* poly = primitive_create_polygon(10, blue, 1.0, 1);
  TEST_ASSERT_NOT_NULL(poly, "Polygon creation");

  Point v1 = {0.0, 0.0}, v2 = {10.0, 0.0}, v3 = {5.0, 10.0}, v4 = {0.0, 5.0};
  TEST_ASSERT(polygon_add_vertex(poly, v1), "Add vertex 1");
  TEST_ASSERT(polygon_add_vertex(poly, v2), "Add vertex 2");
  TEST_ASSERT(polygon_add_vertex(poly, v3), "Add vertex 3");
  TEST_ASSERT(polygon_add_vertex(poly, v4), "Add vertex 4");

  TEST_ASSERT_SIZE_EQ(4, poly->geom.polygon.num_vertices, "4 vertices added");
  TEST_ASSERT(poly->is_aabb_dirty, "Adding vertex sets is_aabb_dirty");

  // Remove vertex at index 1 (v2)
  TEST_ASSERT(polygon_remove_vertex(poly, 1), "Remove vertex index 1");
  TEST_ASSERT_SIZE_EQ(3, poly->geom.polygon.num_vertices, "3 vertices left");
  TEST_ASSERT_DOUBLE_NEAR(5.0, poly->geom.polygon.vertices[1].x, 0.001,
                          "Shift verification");

  // Out of bounds removal
  TEST_ASSERT(!polygon_remove_vertex(poly, 10), "Out of bounds removal fails");

  primitive_destroy(poly);
}

// ============================================================================
// PHASE 3 TESTS: GEOMETRY, AABB, TRANSFORMATIONS & CONTAINMENT
// ============================================================================

static void test_phase3_geometry_and_aabb(void) {
  printf("\n--- Running Phase 3 Tests: Geometry & Containment ---\n");

  Color green = {0, 255, 0};
  Point center = {50.0, 50.0};
  Primitive* c = primitive_create_circle(1, center, 10.0, green, 1.0, 1);

  TEST_ASSERT_DOUBLE_NEAR(40.0, c->aabb.min.x, 0.001, "Circle AABB min.x");
  TEST_ASSERT_DOUBLE_NEAR(60.0, c->aabb.max.x, 0.001, "Circle AABB max.x");

  Point inside = {52.0, 52.0};
  Point outside = {100.0, 100.0};
  TEST_ASSERT(primitive_contains_point(c, inside), "Circle contains point");
  TEST_ASSERT(!primitive_contains_point(c, outside), "Circle outside point");

  // Translation
  primitive_translate(c, 10.0, -5.0);
  TEST_ASSERT(c->is_aabb_dirty, "Translate sets is_aabb_dirty");
  primitive_update_aabb(c);
  TEST_ASSERT_DOUBLE_NEAR(60.0, c->geom.circle.center.x, 0.001,
                          "Translated center x");
  TEST_ASSERT_DOUBLE_NEAR(50.0, c->aabb.min.x, 0.001,
                          "Updated AABB after translate");

  primitive_destroy(c);
}

// ============================================================================
// PHASE 4 TESTS: VECTOR SCENE COLLECTION MANAGEMENT
// ============================================================================

static void test_phase4_scene_collection(void) {
  printf("\n--- Running Phase 4 Tests: VectorScene Collection ---\n");

  Color bg = {255, 255, 255};
  VectorScene* scene = vector_scene_create(800, 600, bg);
  TEST_ASSERT_NOT_NULL(scene, "Scene creation");
  TEST_ASSERT_INT_EQ(800, vector_scene_get_width(scene), "Scene width");

  Color red = {255, 0, 0};
  Point center = {100.0, 100.0};
  Primitive* c1 = primitive_create_circle(-1, center, 20.0, red, 1.0, 5);

  int id1 = vector_scene_add_primitive(scene, c1);
  TEST_ASSERT(id1 >= 0, "Scene Add Primitive auto-assigns valid ID");
  TEST_ASSERT_SIZE_EQ(1, vector_scene_get_count(scene), "Scene count == 1");

  Primitive* found = vector_scene_get_primitive_by_id(scene, id1);
  TEST_ASSERT_NOT_NULL(found, "Find primitive by ID");

  TEST_ASSERT(vector_scene_remove_primitive(scene, id1),
              "Remove primitive by ID");
  TEST_ASSERT_SIZE_EQ(0, vector_scene_get_count(scene), "Scene count == 0");

  vector_scene_destroy(scene);
}

// ============================================================================
// PHASE 5 TESTS: SORTING, PICKING & RENDER
// ============================================================================

static void test_phase5_sorting_picking_render(void) {
  printf("\n--- Running Phase 5 Tests: Sorting, Picking & Render ---\n");

  Color bg = {255, 255, 255};
  VectorScene* scene = vector_scene_create(200, 200, bg);

  Color c1_color = {255, 0, 0};
  Color c2_color = {0, 255, 0};
  Point center = {100.0, 100.0};

  Primitive* p1 = primitive_create_circle(1, center, 40.0, c1_color, 1.0, 10);
  Primitive* p2 = primitive_create_circle(2, center, 20.0, c2_color, 1.0, 20);

  vector_scene_add_primitive(scene, p1);
  vector_scene_add_primitive(scene, p2);

  // Pick top primitive
  PickStats pick_stats = {0, 0};
  Point test_pt = {100.0, 100.0};
  Primitive* top =
      vector_scene_pick_top_primitive(scene, test_pt, true, &pick_stats);
  TEST_ASSERT_NOT_NULL(top, "Top primitive found");
  if (top) {
    TEST_ASSERT_INT_EQ(2, top->id, "Top primitive ID match (z_index 20)");
  }
  TEST_ASSERT(pick_stats.aabb_tests > 0, "AABB tests counted");

  // Sort Test
  SortStats sort_stats = {0, 0};
  vector_scene_sort_by_z(scene, true, &sort_stats);
  TEST_ASSERT(sort_stats.comparisons > 0, "Sort comparisons counted");

  // Render Test
  TEST_ASSERT(vector_scene_render(scene, "test_render.ppm"),
              "Render output file");

  vector_scene_destroy(scene);
}

// ============================================================================
// MAIN TEST RUNNER
// ============================================================================

int main(int argc, char** argv) {
  int phase = 0;
  if (argc > 1) {
    phase = atoi(argv[1]);
  }

  printf("=========================================================\n");
  printf("  AED 2026/2027 — UNIT TEST SUITE (C STYLE GUIDE COMPLIANT)\n");
  printf("=========================================================\n");

  if (phase == 0 || phase == 1) {
    test_phase1_primitive_constructors();
  }
  if (phase == 0 || phase == 2) {
    test_phase2_polygon_manipulation();
  }
  if (phase == 0 || phase == 3) {
    test_phase3_geometry_and_aabb();
  }
  if (phase == 0 || phase == 4) {
    test_phase4_scene_collection();
  }
  if (phase == 0 || phase == 5) {
    test_phase5_sorting_picking_render();
  }

  printf("\n=========================================================\n");
  printf("  TEST RESULTS: %d/%d Passed (%d Failed)\n", g_tests_passed,
         g_tests_run, g_tests_failed);
  printf("=========================================================\n");

  return (g_tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
