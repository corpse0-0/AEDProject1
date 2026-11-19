/**
 * @file vector_scene.c
 * @brief Implementation skeleton of the VectorScene ADT (2D Vector Scene)
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

#include "vector_scene.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "primitive_internal.h"

// ============================================================================
// CONCRETE STRUCT LAYOUT (OPAQUE ADT)
// ============================================================================

/**
 * @brief Concrete structure definition for VectorScene.
 * Hidden from vector_scene.h to enforce encapsulation.
 */
struct _VectorScene {
  int width;              /**< Canvas width in pixels */
  int height;             /**< Canvas height in pixels */
  Color bg_color;         /**< Default background color */
  Primitive** primitives; /**< Dynamic array of pointers to Primitive */
  size_t count;           /**< Current number of primitives in scene */
  size_t capacity;        /**< Allocated capacity for primitives array */
  int next_auto_id;       /**< Auto-increment ID generator */
};

#define INITIAL_SCENE_CAPACITY 8

// ============================================================================
// HELPER FUNCTIONS (PROVIDED TO STUDENTS)
// ============================================================================

/**
 * @brief Helper to write an RGB pixel buffer to binary PPM file (P6 format).
 */
static bool save_ppm_image(const char* filename, int width, int height,
                           const uint8_t* pixel_buffer) {
  if (!filename || !pixel_buffer || width <= 0 || height <= 0) {
    return false;
  }

  FILE* file = fopen(filename, "wb");
  if (!file) {
    return false;
  }

  fprintf(file, "P6\n%d %d\n255\n", width, height);

  size_t total_bytes = (size_t)width * height * 3;
  size_t written = fwrite(pixel_buffer, 1, total_bytes, file);
  fclose(file);

  return written == total_bytes;
}

// ============================================================================
// ACCESSORS / GETTERS (PROVIDED TO STUDENTS)
// ============================================================================

int vector_scene_get_width(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->width;
}

int vector_scene_get_height(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->height;
}

Color vector_scene_get_background_color(const VectorScene* scene) {
  if (!scene) {
    Color black = {0, 0, 0};
    return black;
  }
  return scene->bg_color;
}

size_t vector_scene_get_count(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->count;
}

Primitive* vector_scene_get_primitive_by_index(const VectorScene* scene,
                                               size_t index) {
  if (!scene || index >= scene->count) {
    return NULL;
  }
  return scene->primitives[index];
}

Primitive* vector_scene_get_primitive_by_id(const VectorScene* scene, int id) {
  if (!scene || id < 0) {
    return NULL;
  }

  for (size_t i = 0; i < scene->count; i++) {
    if (scene->primitives[i] && scene->primitives[i]->id == id) {
      return scene->primitives[i];
    }
  }
  return NULL;
}

// ============================================================================
// SCENE LIFECYCLE (STUDENTS MUST COMPLETE)
// ============================================================================

/**
 * Cria uma cena vazia com as dimensões width x height e a cor de fundo
 * bg_color. Devolve NULL se width ou height forem <= 0, ou se a alocação
 * falhar.
 */
VectorScene* vector_scene_create(int width, int height, Color bg_color) {
  if (width <= 0 || height <= 0) {
    return NULL;
  }
  
  VectorScene *scene = malloc(sizeof(VectorScene));
  if (scene == NULL){
    return NULL;
  } 
  
  scene->primitives = malloc(INITIAL_SCENE_CAPACITY * sizeof(Primitive*));
  if (scene->primitives == NULL) {
    free(scene);/* Evita fuga de memória se o segundo malloc falhar */
    return NULL;
  }

  scene->width=width;
  scene->height=height;
  scene->bg_color=bg_color;
  scene->capacity=INITIAL_SCENE_CAPACITY;
  scene->count=0;
  scene->next_auto_id=0;

  return scene;
}

/**
 * Destrói a cena, libertando a sua memória e a de todas as primitivas que
 * contém (através de primitive_destroy). Depois da chamada, scene e as suas
 * primitivas deixam de ser válidos. Se scene for NULL, não faz nada.
 */
void vector_scene_destroy(VectorScene* scene) {
  if (!scene) {
    return;
  }

  for (size_t i = 0; i<scene->count; i++){
    primitive_destroy(scene->primitives[i]);
  }

  free(scene->primitives);
  free(scene);
}

// ============================================================================
// COLLECTION MANAGEMENT (STUDENTS MUST COMPLETE)
// ============================================================================

/**
 * Adiciona a primitiva p à cena. Se p->id < 0, é-lhe atribuído um id
 * automático. Devolve o id da primitiva, ou -1 se scene ou p forem NULL ou
 * se a alocação falhar (neste caso p não é libertada). Em caso de sucesso,
 * a cena passa a ser dona de p.
 */
int vector_scene_add_primitive(VectorScene* scene, Primitive* p){
  if (!scene || !p) {
    return -1;
  }

  /* Array cheio: duplica a capacidade (começa em 4) */
  if (scene->count >= scene->capacity) {
    size_t new_capacity = (scene->capacity == 0) ? 4 : scene->capacity * 2;
    Primitive** new_primitives = (Primitive**)realloc(scene->primitives, new_capacity * sizeof(Primitive*));
    if(new_primitives == NULL){return -1;}
    scene->primitives = new_primitives;
    scene->capacity = new_capacity;
  }

  if (p->id < 0) {
    p->id = scene->next_auto_id++;
  }

  scene->primitives[scene->count++] = p;
  /* Garante que a AABB está válida antes de a primitiva entrar na cena */
  primitive_update_aabb(p);

  return p->id;
}

/**
 * Remove da cena a primitiva com o id indicado e destrói-a. As restantes
 * primitivas mantêm a ordem relativa. Devolve true em caso de sucesso.
 * Devolve false se scene for NULL, se id < 0 ou se não existir nenhuma
 * primitiva com esse id.
 */
bool vector_scene_remove_primitive(VectorScene* scene, int id) {
  if (!scene || id < 0) {
    return false;
  }

  for (size_t i = 0; i<scene->count; i++){
    if(scene->primitives[i]->id == id){
      primitive_destroy(scene->primitives[i]);
      
      /* Desloca as seguintes para tapar o espaço, mantendo a ordem */
      for (size_t j = i; j + 1 < scene->count; j++) {
        scene->primitives[j] = scene->primitives[j + 1];
      }
      scene->count--;
      return true;
    }
  }
  return false;
}

// ============================================================================
// MANIPULATION AND TRANSFORMATIONS (STUDENTS MUST COMPLETE)
// ============================================================================

/**
 * Faz a translação pelo vetor (dx, dy) a primitiva da cena com o id indicado.
 * Devolve true em caso de sucesso. Devolve false se scene for NULL ou se não
 * existir nenhuma primitiva com esse id.
 */
bool vector_scene_translate_primitive(VectorScene* scene, int id, double dx,
                                      double dy) {
  if (!scene) {
    return false;
  }
  
  for (size_t i = 0; i<scene->count; i++){
    if(scene->primitives[i]->id == id){
      primitive_translate(scene->primitives[i], dx, dy);
      return true;
    }
  }
  return false;
}

/**
 * Altera o z_index da primitiva da cena com o id indicado. O z_index define a
 * ordem de desenho. Devolve true em caso de sucesso. Devolve false se scene
 * for NULL ou se não existir nenhuma primitiva com esse id.
 */
bool vector_scene_set_z_index(VectorScene* scene, int id, int new_z_index) {
  if (!scene) {
    return false;
  }

  for (size_t i = 0; i<scene->count; i++){
    if(scene->primitives[i]->id == id){
      scene->primitives[i]->z_index = new_z_index;
      return true;
    }
  }
  return false;
}

// ============================================================================
// SPATIAL SELECTION AND SORTING (STUDENTS MUST COMPLETE & ANALYZE)
// ============================================================================

/**
 * @brief Sorts an array of primitives by z_index using O(N^2) Insertion Sort.
 *
 * Iterative in-place sorting algorithm. Iterates through the primitive array
 * and inserts each element into its correct sorted position based on z_index.
 * Increments comparison and swap/move metrics in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param count Number of elements in the array.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */
static void insertion_sort_by_z(Primitive** arr, size_t count,
                                SortStats* stats) {
  if (arr == NULL || count < 2) {
    return;
  }
  
  for (size_t i = 1; i < count; i++){
    Primitive* key = arr[i];
    size_t j = i;
    /* Desloca para a direita os elementos com z_index maior que o de key */
    while(j > 0){
      if(stats) stats->comparisons++;
      if(arr[j-1]->z_index <= key->z_index){
        break;
      }
      arr[j] = arr[j-1];
      if(stats) stats->swaps++;
      j--;
    }
    arr[j] = key;
  }
}

/**
 * @brief Merges two sorted subarrays of primitive pointers by z_index.
 *
 * Helper function for MergeSort that combines [left..mid] and [mid+1..right]
 * using a temporary pointer array. Increments comparisons and swaps/moves
 * counters in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to merge.
 * @param temp_arr Auxiliary temporary array of Primitive pointers.
 * @param left Starting index of the first subarray.
 * @param mid Middle index separating the two subarrays.
 * @param right Ending index of the second subarray.
 * @param stats Pointer to SortStats structure to track metrics (can be NULL).
 */
static void merge_by_z(Primitive** arr, Primitive** temp_arr, size_t left,
                       size_t mid, size_t right, SortStats* stats) {
  size_t i = left;
  size_t j = mid + 1;
  size_t k = left;

  while (i <= mid && j <= right) {
    if (stats) stats->comparisons++;
    /* <= para ser estável */
    if (arr[i]->z_index <= arr[j]->z_index) {        
      temp_arr[k++] = arr[i++];
    } else {
      temp_arr[k++] = arr[j++];
    }
    /* Uma cópia para o temp */
    if (stats) stats->swaps++; 
  }

  /*Insere a metade que sobrou, que já está ordenada*/
  while (i<=mid){
    temp_arr[k++] = arr[i++];
    if(stats) stats->swaps++;
  }
  while (j<=right){
    temp_arr[k++] = arr[j++];
    if(stats) stats->swaps++;
  }


  /* Copia o resultado de volta para arr */
  for(size_t l = left; l <= right; l++){
    arr[l] = temp_arr[l];
    if(stats) stats->swaps++;
  }
}

/**
 * @brief Sorts a subarray of primitives by z_index using O(N log N) MergeSort.
 *
 * Stable recursive Divide-and-Conquer algorithm. Splits the array in half,
 * recursively sorts both halves, and invokes merge_by_z() to merge them.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param temp_arr Auxiliary temporary array allocated for merging.
 * @param left Starting index of the subarray to sort.
 * @param right Ending index of the subarray to sort.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */
 /*Ordena o array [left...right]*/
static void merge_sort_by_z(Primitive** arr, Primitive** temp_arr, size_t left,
                            size_t right, SortStats* stats) {
  if(left >= right){ return; }
  size_t mid = left + (right-left) / 2;

  merge_sort_by_z(arr, temp_arr, left, mid, stats);
  merge_sort_by_z(arr, temp_arr, mid+1, right, stats);

  merge_by_z(arr, temp_arr, left, mid, right, stats);
}

/**
 * Ordena as primitivas da cena por z_index, por ordem crescente. A ordenação
 * é estável: primitivas com o mesmo z_index mantêm a ordem relativa.
 * Se use_fast_sort for false usa Insertion Sort, O(N^2); caso contrário usa
 * MergeSort, O(N log N). Se stats não for NULL, as métricas são repostas a
 * zero e depois registadas durante a ordenação.
 * Se scene for NULL ou tiver menos de 2 primitivas, não faz nada.
 */
void vector_scene_sort_by_z(VectorScene* scene, bool use_fast_sort,
                            SortStats* stats) {
  if (scene == NULL || scene->count < 2) return;

  if(stats != NULL){
    stats->comparisons = 0;
    stats->swaps = 0;
  }

  if(!use_fast_sort){ 
    insertion_sort_by_z(scene->primitives, scene->count, stats);
  }else{
    Primitive** temp_arr = malloc(scene->count * sizeof(Primitive*)); 
    /* Sem memória para o array auxiliar: a cena não é ordenada */
    if(temp_arr == NULL){
      return;
    }
    merge_sort_by_z(scene->primitives, temp_arr, 0, scene->count -1, stats);
    free(temp_arr);
  }
}

/**
 * Devolve a primitiva mais acima da cena que contém o ponto pt, ou NULL se
 * nenhuma o contiver ou se scene for NULL. 
 * Assume que a cena está ordenada por z_index: percorre da última para a
 * primeira primitiva e devolve a primeira que contém pt.
 * Se is_opt for true, faz primeiro um teste rápido com a AABB e só depois o
 * teste exato. Se stats não for NULL, é reposto a zero e regista o número
 * de testes com AABB e de testes exatos.
 */
Primitive* vector_scene_pick_top_primitive(const VectorScene* scene, Point pt,
                                           bool is_opt, PickStats* stats) {
  if (!scene) {
    return NULL;
  }

  if (stats != NULL){
    stats->aabb_tests = 0;
    stats->exact_tests = 0;
  }

  for (size_t i = scene->count; i > 0; i--) {
    Primitive* prim = scene->primitives[i - 1];

    if(is_opt){
      BoundingBox box = primitive_get_aabb(prim);
      if(stats)stats->aabb_tests++;

      /* Fora da AABB (alargada pela tolerância do segmento): salta o teste
      exato, que não poderia aceitar o ponto */
      double tol = (prim->type == PRIM_SEGMENT) ? 1.0 : 0.0;
      if (pt.x < box.min.x - tol || pt.x > box.max.x + tol ||
          pt.y < box.min.y - tol || pt.y > box.max.y + tol) {
        continue;
      }
    }

    if (stats) stats->exact_tests++;
    if (primitive_contains_point(prim, pt)){
      return prim;
    }

  }

  return NULL;
}

// ============================================================================
// RENDERING / RASTERIZATION (STUDENTS MUST COMPLETE)
// ============================================================================

/**
 * Renderiza a cena para um ficheiro de imagem PPM.
 * Usa o algoritmo do pintor: as primitivas são desenhadas por ordem
 * crescente de z_index (as de cima sobrepõem-se às de baixo) e combinadas
 * com o fundo por alpha blending. A ordem da cena não é alterada.
 * Devolve true se o ficheiro foi gravado. Devolve false se scene ou
 * output_filename forem NULL, se as dimensões forem inválidas, se a
 * alocação falhar ou se a gravação falhar.
 */
bool vector_scene_render(const VectorScene* scene,
                         const char* output_filename) {
  if (!scene || !output_filename) {
    return false;
  }


  int width = scene->width;
  int height = scene->height;
  if (width <= 0 || height <= 0) {
    return false;
  }

  uint8_t* pixel_buffer = malloc((size_t)width * height * 3);
  if (!pixel_buffer) {
    return false;
  }
  /* Preenche o buffer com a cor de fundo */
  for (size_t i = 0; i < (size_t)width * height; i++) {
    pixel_buffer[i * 3] = scene->bg_color.r;
    pixel_buffer[i * 3 + 1] = scene->bg_color.g;
    pixel_buffer[i * 3 + 2] = scene->bg_color.b;
  }

  /* Ordena uma cópia do array, para não alterar a ordem da cena.
     Se a cena estiver vazia, sorted fica NULL e o ciclo abaixo não corre */
  Primitive** sorted = NULL;
  if (scene->count > 0) {
    sorted = malloc(scene->count * sizeof(Primitive*));
    if (!sorted) {
      free(pixel_buffer);
      return false;
    }

    memcpy(sorted, scene->primitives, scene->count * sizeof(Primitive*));
    insertion_sort_by_z(sorted, scene->count, NULL);
  }

  for (size_t i = 0; i < scene->count; i++) {
    Primitive* p = sorted[i];
    if (!p) {
      continue;
    }

    BoundingBox box = primitive_get_aabb(p);
    /* O pixel (x, y) tem o centro em (x + 0.5, y + 0.5). Só interessam os
    pixéis cujo centro cai dentro da AABB */
    double x_min_d = ceil(box.min.x - 0.5);
    double x_max_d = floor(box.max.x - 0.5);
    double y_min_d = ceil(box.min.y - 0.5);
    double y_max_d = floor(box.max.y - 0.5);

    /* Fora da imagem: nada a desenhar */
    if (x_max_d < 0.0 || y_max_d < 0.0 ||
        x_min_d > (double)(width - 1) ||
        y_min_d > (double)(height - 1)) {
      continue;
    }

    /* Clipping aos limites da imagem */
    if (x_min_d < 0.0) x_min_d = 0.0;
    if (y_min_d < 0.0) y_min_d = 0.0;
    if (x_max_d > (double)(width - 1)) x_max_d = (double)(width - 1);
    if (y_max_d > (double)(height - 1)) y_max_d = (double)(height - 1);

    if (x_min_d > x_max_d || y_min_d > y_max_d) {
      continue;
    }

    int x_min = (int)x_min_d;
    int x_max = (int)x_max_d;
    int y_min = (int)y_min_d;
    int y_max = (int)y_max_d;

    double alpha = p->alpha;
    if (alpha < 0.0) alpha = 0.0;
    if (alpha > 1.0) alpha = 1.0;

    for (int y = y_min; y <= y_max; y++) {
      for (int x = x_min; x <= x_max; x++) {
        Point pixel_center = {(double)x + 0.5, (double)y + 0.5};
        if (!primitive_contains_point(p, pixel_center)) {
          continue;
        }

        /* Alpha blending: cor * alpha + fundo * (1 - alpha) */
        size_t offset = ((size_t)y * width + (size_t)x) * 3;
        pixel_buffer[offset] = (uint8_t)lround(
            p->color.r * alpha + pixel_buffer[offset] * (1.0 - alpha));
        pixel_buffer[offset + 1] = (uint8_t)lround(
            p->color.g * alpha + pixel_buffer[offset + 1] * (1.0 - alpha));
        pixel_buffer[offset + 2] = (uint8_t)lround(
            p->color.b * alpha + pixel_buffer[offset + 2] * (1.0 - alpha));
      }
    }
  }

  bool saved = save_ppm_image(output_filename, scene->width, scene->height,
                              pixel_buffer);
  free(sorted);
  free(pixel_buffer);
  return saved;
}

// ============================================================================
// FILE INPUT / OUTPUT (PROVIDED TO STUDENTS)
// ============================================================================

bool vector_scene_save_text(const VectorScene* scene, const char* filename) {
  if (!scene || !filename) {
    return false;
  }

  FILE* file = fopen(filename, "w");
  if (!file) {
    return false;
  }

  Color bg = scene->bg_color;
  fprintf(file, "SCENE %d %d %d %d %d\n", scene->width, scene->height, bg.r,
          bg.g, bg.b);

  for (size_t i = 0; i < scene->count; i++) {
    Primitive* p = scene->primitives[i];
    if (!p) {
      continue;
    }

    Color c = p->color;

    switch (p->type) {
      case PRIM_CIRCLE:
        fprintf(file, "CIRC %d %d %d %d %d %.3f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha, p->geom.circle.center.x,
                p->geom.circle.center.y, p->geom.circle.radius);
        break;

      case PRIM_RECTANGLE:
        fprintf(file, "RECT %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha,
                p->geom.rectangle.top_left.x, p->geom.rectangle.top_left.y,
                p->geom.rectangle.width, p->geom.rectangle.height);
        break;

      case PRIM_SEGMENT:
        fprintf(file, "SEG %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha, p->geom.segment.p1.x,
                p->geom.segment.p1.y, p->geom.segment.p2.x,
                p->geom.segment.p2.y);
        break;

      case PRIM_POLYGON:
        fprintf(file, "POLY %d %d %d %d %d %.3f %zu", p->id, p->z_index, c.r,
                c.g, c.b, p->alpha, p->geom.polygon.num_vertices);

        for (size_t v = 0; v < p->geom.polygon.num_vertices; v++) {
          fprintf(file, " %.2f %.2f", p->geom.polygon.vertices[v].x,
                  p->geom.polygon.vertices[v].y);
        }
        fprintf(file, "\n");
        break;
    }
  }

  fclose(file);
  return true;
}

VectorScene* vector_scene_load_text(const char* filename) {
  if (!filename) {
    return NULL;
  }

  FILE* file = fopen(filename, "r");
  if (!file) {
    return NULL;
  }

  char line[1024];
  VectorScene* scene = NULL;

  while (fgets(line, sizeof(line), file)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
      continue;
    }

    char tag[16];
    if (sscanf(line, "%15s", tag) != 1) {
      continue;
    }

    if (strcmp(tag, "SCENE") == 0) {
      int w, h, r, g, b;
      if (sscanf(line, "SCENE %d %d %d %d %d", &w, &h, &r, &g, &b) == 5) {
        Color bg = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
        scene = vector_scene_create(w, h, bg);
      }
    } else if (scene != NULL) {
      int id, z_index, r, g, b;
      double alpha;

      if (strcmp(tag, "CIRC") == 0) {
        double cx, cy, radius;
        if (sscanf(line, "CIRC %d %d %d %d %d %lf %lf %lf %lf", &id, &z_index,
                   &r, &g, &b, &alpha, &cx, &cy, &radius) == 9) {
          Point center = {cx, cy};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_circle(id, center, radius, color,
                                                 alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "RECT") == 0) {
        double x, y, w, h;
        if (sscanf(line, "RECT %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x, &y, &w, &h) == 10) {
          Point top_left = {x, y};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_rectangle(id, top_left, w, h, color,
                                                    alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "SEG") == 0) {
        double x1, y1, x2, y2;
        if (sscanf(line, "SEG %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x1, &y1, &x2, &y2) == 10) {
          Point p1 = {x1, y1};
          Point p2 = {x2, y2};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p =
              primitive_create_segment(id, p1, p2, color, alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "POLY") == 0) {
        size_t num_verts;
        int read_header = sscanf(line, "POLY %d %d %d %d %d %lf %zu", &id,
                                 &z_index, &r, &g, &b, &alpha, &num_verts);
        if (read_header == 7) {
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_polygon(id, color, alpha, z_index);
          if (p) {
            char* ptr = line;
            for (int k = 0; k < 8; k++) {
              while (*ptr && *ptr != ' ' && *ptr != '\t') {
                ptr++;
              }
              while (*ptr && (*ptr == ' ' || *ptr == '\t')) {
                ptr++;
              }
            }

            for (size_t v = 0; v < num_verts; v++) {
              double vx, vy;
              if (sscanf(ptr, "%lf %lf", &vx, &vy) == 2) {
                Point pt = {vx, vy};
                polygon_add_vertex(p, pt);

                for (int k = 0; k < 2; k++) {
                  while (*ptr && *ptr != ' ' && *ptr != '\t' && *ptr != '\n' &&
                         *ptr != '\r') {
                    ptr++;
                  }
                  while (*ptr && (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' ||
                                  *ptr == '\r')) {
                    ptr++;
                  }
                }
              }
            }
            vector_scene_add_primitive(scene, p);
          }
        }
      }
    }
  }

  fclose(file);
  return scene;
}
