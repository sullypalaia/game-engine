//----------------------------------------------------------------------------------
// THIS EXAMPLE WAS CREATED BY AI TO DEMONSTRATE THE BASIC USE OF THE ENGINE FOR
// CREATING 2D SHAPES, COLORS, and TRANSFORMS
//----------------------------------------------------------------------------------

#include <array>
#include <cmath>
#include <vector>

#include "external/GLFW/glfw3.h"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"

import Camera;
import Components;
import ECS;
import OpenGLRenderer;
import WindowManager;

namespace {

std::vector<GLfloat> triangle() {
  return {-0.08f, -0.06f, 0.08f, -0.06f, 0.0f, 0.10f};
}

std::vector<GLfloat> diamond() {
  return {0.0f, 0.14f, 0.14f, 0.0f, 0.0f, -0.14f, -0.14f, 0.0f};
}

std::vector<GLfloat> vertex_color(const std::array<GLfloat, 4> &color,
                                  const size_t vertex_count) {
  std::vector<GLfloat> colors;
  colors.reserve(vertex_count * color.size());
  for (size_t i = 0; i < vertex_count; ++i)
    colors.insert(colors.end(), color.begin(), color.end());
  return colors;
}

glm::mat4 make_transform(float x, float y, float scale, float rotation) {
  return glm::rotate(
      glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)),
                 glm::vec3(scale)),
      rotation, glm::vec3(0.0f, 0.0f, 1.0f));
}

} // namespace

int main() {
  WindowManager window_manager(true);
  const size_t window = window_manager.m_create_window(
      1100, 700, "Rendering Example 2", nullptr, nullptr, true);
  window_manager.m_make_context_current(window);

  const float aspect = window_manager.m_get_aspect_ratio(window);
  Camera camera(
      CameraData{glm::ortho(-aspect, aspect, -1.0f, 1.0f), glm::mat4(1.0f)});

  ECS ecs;

  const std::vector<GLfloat> particle = triangle();
  const auto particle_color =
      vertex_color({0.15f, 0.55f, 1.0f, 1.0f}, particle.size() / 2);
  const auto particle_a = ecs.m_add_entity_with_component<Position2D>(particle);
  ecs.m_add_component_to_entity<Color>(particle_a, particle_color);
  const auto particle_b = ecs.m_add_entity_with_component<Position2D>(particle);
  ecs.m_add_component_to_entity<Color>(
      particle_b, vertex_color({0.2f, 0.9f, 0.85f, 1.0f}, 3));
  const auto particle_c = ecs.m_add_entity_with_component<Position2D>(particle);
  ecs.m_add_component_to_entity<Color>(
      particle_c, vertex_color({0.95f, 0.35f, 0.3f, 1.0f}, 3));
  const auto particle_d = ecs.m_add_entity_with_component<Position2D>(particle);
  ecs.m_add_component_to_entity<Color>(
      particle_d, vertex_color({0.95f, 0.8f, 0.2f, 1.0f}, 3));

  const auto indexed_shape =
      ecs.m_add_entity_with_component<Position2D>(diamond());
  ecs.m_add_component_to_entity<Color>(
      indexed_shape, vertex_color({0.45f, 0.2f, 1.0f, 1.0f}, 4));
  ecs.m_add_component_to_entity<Indices>(
      indexed_shape, std::vector<unsigned int>{0, 1, 2, 0, 2, 3});

  const auto red = ecs.m_add_entity_with_component<Position2D>(triangle());
  ecs.m_add_component_to_entity<SolidColor>(red, 1.0f, 0.15f, 0.12f, 1.0f);
  ecs.m_add_component_to_entity<Transform>(
      red, make_transform(0.0f, 0.0f, 1.0f, 0.0f));

  const auto gold = ecs.m_add_entity_with_component<Position2D>(triangle());
  ecs.m_add_component_to_entity<SolidColor>(gold, 1.0f, 0.65f, 0.08f, 1.0f);
  ecs.m_add_component_to_entity<Transform>(
      gold, make_transform(0.0f, 0.0f, 1.0f, 0.0f));

  const auto blue = ecs.m_add_entity_with_component<Position2D>(triangle());
  ecs.m_add_component_to_entity<SolidColor>(blue, 0.15f, 0.65f, 1.0f, 1.0f);
  ecs.m_add_component_to_entity<Transform>(
      blue, make_transform(0.0f, 0.0f, 1.0f, 0.0f));

  const auto violet = ecs.m_add_entity_with_component<Position2D>(triangle());
  ecs.m_add_component_to_entity<SolidColor>(violet, 0.7f, 0.25f, 1.0f, 1.0f);
  ecs.m_add_component_to_entity<Transform>(
      violet, make_transform(0.0f, 0.0f, 1.0f, 0.0f));

  constexpr GLfloat clear_color[4] = {0.015f, 0.02f, 0.07f, 1.0f};
  OpenGLRenderer renderer(ecs.m_get_registry(), clear_color);

  while (!glfwWindowShouldClose(window_manager.m_get_window(window))) {
    const float time = static_cast<float>(glfwGetTime());

    ecs.m_update_entity_component<Transform>(
        red, make_transform(std::cos(time) * aspect * 0.55f,
                            std::sin(time * 1.7f) * 0.55f, 0.75f, time * 2.0f));
    ecs.m_update_entity_component<Transform>(
        gold,
        make_transform(std::cos(time * 1.3f) * aspect * 0.42f,
                       std::sin(time * 2.1f) * 0.42f, 0.55f, -time * 2.7f));
    ecs.m_update_entity_component<Transform>(
        blue,
        make_transform(std::cos(time * 0.7f) * aspect * 0.72f,
                       std::sin(time * 0.9f) * 0.72f, 0.45f, time * 3.4f));
    ecs.m_update_entity_component<Transform>(
        violet,
        make_transform(std::cos(time * 1.9f) * aspect * 0.3f,
                       std::sin(time * 1.2f) * 0.3f, 0.9f, -time * 1.5f));

    renderer.m_draw();

    glfwSwapBuffers(window_manager.m_get_window(window));
    glfwPollEvents();
  }

  renderer.m_destroy();
  window_manager.m_destroy();
  return 0;
}
