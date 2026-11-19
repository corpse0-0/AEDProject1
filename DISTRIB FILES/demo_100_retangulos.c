/**
 * @file demo_app.c
 * @brief Cria uma cena 2D composta exclusivamente por 100 retangulos.
 *
 * Algoritmos e Estruturas de Dados (2026/2027)
 * Universidade de Aveiro
 */

#include <stdio.h>
#include <stdlib.h>

#include "primitive.h"
#include "primitive_internal.h"
#include "vector_scene.h"
#include "elapsed_time.h"

int main(void) {
  const int canvas_width = 800;
  const int canvas_height = 600;
  const int columns = 100;
  const int rows = 100;
  const float rect_width = 8.0f;
  const double rect_height = 6.0f;

  // Fundo escuro para destacar os 100 retangulos.
  Color background = {18, 18, 28};
  VectorScene* scene =
      vector_scene_create(canvas_width, canvas_height, background);

  if (scene == NULL) {
    fprintf(stderr, "Erro: nao foi possivel criar a cena.\n");
    return EXIT_FAILURE;
  }

  // Cada celula da grelha recebe exatamente um retangulo.
  for (int row = 0; row < rows; row++) {
    for (int col = 0; col < columns; col++) {
      // Criar um mosaico colorido, variando a cor por linha e coluna.
      Color color = {
	  (unsigned char)((col * 37) % 256),
	  (unsigned char)((row * 53) % 256),
	  (unsigned char)(((row + col) * 29) % 256)
      };

      float alpha = 1.0f;
      // Z-Index crescente por linha: cada linha fica numa camada.
      int z_index = (row * columns + col) % 17;

      Point top_left = {
          col * rect_width,
          row * rect_height
      };

      Primitive* rectangle = primitive_create_rectangle(
          -1, top_left, rect_width, rect_height, color, alpha, z_index);

      if (rectangle == NULL) {
        fprintf(stderr, "Erro ao criar o retangulo na linha %d, coluna %d.\n",
                row, col);
        vector_scene_destroy(scene);
        return EXIT_FAILURE;
      }

      if (vector_scene_add_primitive(scene, rectangle) < 0) {
        fprintf(stderr, "Erro ao adicionar o retangulo na cena.\n");
        primitive_destroy(rectangle);
        vector_scene_destroy(scene);
        return EXIT_FAILURE;
      }
    }
  }

  printf("Cena criada com %zu primitivas.\n",
         vector_scene_get_count(scene));

  // Guardar a descricao da cena e gerar a imagem.
  if (!vector_scene_save_text(scene, "100_retangulos.vsc")) {
    fprintf(stderr, "Erro ao guardar 100_retangulos.vsc.\n");
    vector_scene_destroy(scene);
    return EXIT_FAILURE;
  }

  if (!vector_scene_render(scene, "100_retangulos.ppm")) {
    fprintf(stderr, "Erro ao renderizar 100_retangulos.ppm.\n");
    vector_scene_destroy(scene);
    return EXIT_FAILURE;
  }

  printf("Ficheiro da cena: 100_retangulos.vsc\n");
  printf("Imagem gerada: 100_retangulos.ppm\n");
  printf("\n");
  double start, end, sort_time;
  SortStats sort_stats;
  start = cpu_time();

  // Demonstrar a ordenacao das 100 primitivas por Z-Index.
  vector_scene_sort_by_z(scene, false, &sort_stats);

  end = cpu_time();
  sort_time = end - start;

  printf("Sorting Time: %.9f segundos\n" , sort_time);

  printf("Ordenacao por Z-Index:\n");
  printf("Comparacoes: %lu | Movimentos: %lu\n",
         sort_stats.comparisons, sort_stats.swaps);
  printf("\n");
  Point point = {400.0f, 300.0f};
  double start1, end1, pick_time1;
  PickStats pick_stats;
  start1 = cpu_time();

  //Com otimização
  vector_scene_pick_top_primitive(scene, point, true, &pick_stats);
  end1 = cpu_time();
  pick_time1 = end1 - start1;
  printf("Com otimização\n");
  printf("Testes AABB: %zu\n", pick_stats.aabb_tests);
  printf("Testes geométricos: %zu\n", pick_stats.exact_tests);
  printf("Tempo: %.9f segundos\n", pick_time1);
  printf("\n");
  double start2, end2, pick_time2;
  start2 = cpu_time();

  //Sem otimização
  vector_scene_pick_top_primitive(scene, point, false, &pick_stats);
  end2 = cpu_time();
  pick_time2 = end2 - start2;
  printf("Sem otimização\n");
  printf("Testes AABB: %zu\n", pick_stats.aabb_tests);
  printf("Testes geométricos: %zu\n", pick_stats.exact_tests);
  printf("Tempo: %.9f segundos\n", pick_time2);

  vector_scene_destroy(scene);
  return EXIT_SUCCESS;
}
