/**
 * @file demo_app.c
 * @brief Application Program Demonstrating VectorScene and Primitive ADTs
 *
 * Algoritmos e Estruturas de Dados (2026/2027)
 * Departamento de Electrónica, Telecomunicações e Informática
 * Universidade de Aveiro
 *
 * This program illustrates the core functionalities of the 2D Vector Engine:
 * 1. Programmatic creation of a 2D vector scene with diverse shapes.
 * 2. Saving the vector scene to a text file (.vsc).
 * 3. Raster rendering of the scene to a PPM image file with alpha blending.
 * 4. Loading a scene from a text file (.vsc).
 * 5. Spatial selection (picking), transformations, z-index updates, and
 * sorting.
 * 6. Saving and rendering the modified scene to new output files.
 */

#include <stdio.h>
#include <stdlib.h>

#include "primitive.h"
#include "primitive_internal.h"
#include "vector_scene.h"

// ============================================================================
// HELPER PRINT FUNCTIONS
// ============================================================================

static void print_banner(const char* title) {
  printf(
      "\n===================================================================="
      "\n");
  printf(" %s\n", title);
  printf(
      "===================================================================="
      "\n");
}

static void print_primitive_info(const Primitive* p) {
  if (p == NULL) {
    printf("  [Primitive] NULL\n");
    return;
  }

  const char* type_names[] = {"Circle", "Rectangle", "Segment", "Polygon"};
  Color rgb = primitive_get_color(p);
  printf(
      "  [Primitive ID %d] Type: %s | Z-Index: %d | Color: (%d, %d, %d) | "
      "Alpha: %.2f\n",
      primitive_get_id(p), type_names[primitive_get_type(p)],
      primitive_get_z_index(p), rgb.r, rgb.g, rgb.b, primitive_get_alpha(p));
}

// ============================================================================
// MAIN APPLICATION
// ============================================================================

int main(void) {

  print_banner("1. LOADING SCENE FROM FILE AND MANIPULATING OBJECTS");

  // Load scene from file
  VectorScene* loaded_scene = vector_scene_load_text("garfield.vsc");
  if (loaded_scene == NULL) {
    fprintf(stderr, "Error: Failed to load scene from file '%s'.\n",
            "garfield.vsc");
    return EXIT_FAILURE;
  }

  printf("Loaded scene from '%s': Canvas %dx%d, Total Primitives: %zu\n",
         "garfield.vsc", vector_scene_get_width(loaded_scene),
         vector_scene_get_height(loaded_scene),
         vector_scene_get_count(loaded_scene));

  print_banner("2. SAVING AND RENDERING MODIFIED SCENE");

  const char* mod_text_file = "modified_scene.vsc";
  const char* mod_image_file = "modified_scene.ppm";

  if (vector_scene_save_text(loaded_scene, mod_text_file)) {
    printf(
        "Successfully saved modified scene configuration to text file: '%s'\n",
        mod_text_file);
  }

  if (vector_scene_render(loaded_scene, mod_image_file)) {
    printf("Successfully rendered modified raster image file: '%s'\n",
           mod_image_file);
  }

  // Clean up loaded scene
  vector_scene_destroy(loaded_scene);
  printf("Freed loaded scene memory.\n");

  print_banner("DEMO COMPLETED SUCCESSFULLY!");
  return EXIT_SUCCESS;
}
