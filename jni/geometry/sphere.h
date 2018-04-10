#pragma once

#include <algorithm>
#include <vector>

#include "geometry/vector.h"

// Generates a sphere by repeatedly subdividing an octahedron, at each pass
// moving vertices out to the unit sphere.
template <typename IndexType>
void sphere(const size_t quality, std::vector<vec3>* vertices,
            std::vector<IndexType>* indices) {
  // Generate octahedron.
  vertices->emplace_back(0, 0, 1);
  vertices->emplace_back(0, 1, 0);
  vertices->emplace_back(-1, 0, 0);
  vertices->emplace_back(0, -1, 0);
  vertices->emplace_back(1, 0, 0);
  vertices->emplace_back(0, 0, -1);

  indices->emplace_back(0);
  indices->emplace_back(1);
  indices->emplace_back(2);
  indices->emplace_back(0);
  indices->emplace_back(2);
  indices->emplace_back(3);
  indices->emplace_back(0);
  indices->emplace_back(3);
  indices->emplace_back(4);
  indices->emplace_back(0);
  indices->emplace_back(4);
  indices->emplace_back(1);
  indices->emplace_back(1);
  indices->emplace_back(5);
  indices->emplace_back(2);
  indices->emplace_back(2);
  indices->emplace_back(5);
  indices->emplace_back(3);
  indices->emplace_back(3);
  indices->emplace_back(5);
  indices->emplace_back(4);
  indices->emplace_back(4);
  indices->emplace_back(5);
  indices->emplace_back(1);

  // Divide all faces into 4 new ones `quality` times.
  for (size_t q = 0; q < quality; ++q) {
    std::vector<IndexType> new_indices;

    while (!indices->empty()) {
      size_t points[3];

      points[2] = indices->back();
      indices->pop_back();
      points[1] = indices->back();
      indices->pop_back();
      points[0] = indices->back();
      indices->pop_back();

      vec3 midpoints[3];

      midpoints[0] =
          (((*vertices)[points[0]] + (*vertices)[points[1]]) / 2).normalize();
      midpoints[1] =
          (((*vertices)[points[0]] + (*vertices)[points[2]]) / 2).normalize();
      midpoints[2] =
          (((*vertices)[points[1]] + (*vertices)[points[2]]) / 2).normalize();

      size_t midpoint_indices[3];

      for (size_t i = 0; i < 3; i++) {
        size_t j;

        for (j = 0; j < vertices->size(); j++) {
          if ((*vertices)[j] == midpoints[i]) {
            midpoint_indices[i] = j;
            break;
          }
        }

        if (j == vertices->size()) {
          midpoint_indices[i] = vertices->size();
          vertices->emplace_back(midpoints[i]);
        }
      }

      new_indices.emplace_back(points[0]);
      new_indices.emplace_back(midpoint_indices[0]);
      new_indices.emplace_back(midpoint_indices[1]);

      new_indices.emplace_back(midpoint_indices[0]);
      new_indices.emplace_back(midpoint_indices[2]);
      new_indices.emplace_back(midpoint_indices[1]);

      new_indices.emplace_back(midpoint_indices[0]);
      new_indices.emplace_back(points[1]);
      new_indices.emplace_back(midpoint_indices[2]);

      new_indices.emplace_back(midpoint_indices[1]);
      new_indices.emplace_back(midpoint_indices[2]);
      new_indices.emplace_back(points[2]);
    }

    *indices = std::move(new_indices);
  }
}
