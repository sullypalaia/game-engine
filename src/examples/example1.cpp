//----------------------------------------------------------------------------------
// THIS EXAMPLE WAS CREATED BY AI TO DEMONSTRATE THE BASIC USE OF THE ENGINE FOR
// CREATING 2D SHAPES, COLORS, and TRANSFORMS
//----------------------------------------------------------------------------------

#include <array>

#include "external/GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "entt/entt.hpp"

#include "macros.h"

import Meshes;
import ECS;
import Shaders;
import Camera;
import WindowManager;
import Components;
import OpenGLRenderer;

int main() {
  // create the window manager and window
  WindowManager window_manager(true);
  const size_t window = window_manager.m_create_window(
      800, 600, "Rendering Example 1", nullptr, nullptr, true);

  // attach the window to the current context
  window_manager.m_make_context_current(window);

  // create the ecs
  ECS ecs;

  // camera setup
  const float window_aspect_ratio = window_manager.m_get_aspect_ratio(window);
  Camera camera(CameraData{
      glm::ortho(-1.0f * window_aspect_ratio, window_aspect_ratio, -1.0f, 1.0f),
      glm::mat4(1.0f)});

  // Filled pear body, built as a triangle fan around the center.
  const std::vector<GLfloat> pear_ring = {
      0.00f,  0.38f,  0.10f,  0.30f, 0.23f,  0.12f,  0.25f,
      -0.08f, 0.16f,  -0.30f, 0.00f, -0.40f, -0.16f, -0.30f,
      -0.25f, -0.08f, -0.23f, 0.12f, -0.10f, 0.30f};
  const std::array<GLfloat, 2> pear_center{0.0f, -0.05f};
  const std::array<GLfloat, 4> pear_center_color{0.55f, 1.0f, 0.15f, 1.0f};
  const std::array<GLfloat, 4> pear_edge_color{0.05f, 0.55f, 0.08f, 1.0f};

  std::vector<GLfloat> pear_pos;
  std::vector<GLfloat> pear_colors;
  for (size_t i = 0; i < pear_ring.size() / 2; ++i) {
    const size_t next = (i + 1) % (pear_ring.size() / 2);
    pear_pos.insert(pear_pos.end(),
                    {pear_center[0], pear_center[1], pear_ring[2 * i],
                     pear_ring[2 * i + 1], pear_ring[2 * next],
                     pear_ring[2 * next + 1]});
    pear_colors.insert(pear_colors.end(), pear_center_color.begin(),
                       pear_center_color.end());
    pear_colors.insert(pear_colors.end(), pear_edge_color.begin(),
                       pear_edge_color.end());
    pear_colors.insert(pear_colors.end(), pear_edge_color.begin(),
                       pear_edge_color.end());
  }

  // A separate leaf angled away from the pear's stem.
  const std::vector<GLfloat> leaf_pos = {0.00f, 0.38f, 0.03f, 0.48f,
                                         0.20f, 0.52f, 0.00f, 0.38f,
                                         0.20f, 0.52f, 0.12f, 0.35f};
  const std::vector<GLfloat> leaf_colors = {
      0.10f, 0.65f, 0.08f, 1.0f, 0.25f, 0.90f, 0.12f, 1.0f,
      0.05f, 0.45f, 0.04f, 1.0f, 0.10f, 0.65f, 0.08f, 1.0f,
      0.25f, 0.90f, 0.12f, 1.0f, 0.05f, 0.45f, 0.04f, 1.0f};

  const std::vector<GLfloat> apple_ring = {
      0.00f,  0.34f,  0.16f,  0.28f, 0.27f,  0.12f,  0.24f,
      -0.12f, 0.12f,  -0.30f, 0.00f, -0.36f, -0.12f, -0.30f,
      -0.24f, -0.12f, -0.27f, 0.12f, -0.16f, 0.28f};
  std::vector<GLfloat> apple_pos;
  for (size_t i = 0; i < apple_ring.size() / 2; ++i) {
    const size_t next = (i + 1) % (apple_ring.size() / 2);
    apple_pos.insert(apple_pos.end(),
                     {0.0f, 0.0f, apple_ring[2 * i], apple_ring[2 * i + 1],
                      apple_ring[2 * next], apple_ring[2 * next + 1]});
  }
  const std::vector<GLfloat> apple_leaf_pos = {0.00f, 0.32f, 0.03f, 0.43f,
                                               0.20f, 0.47f, 0.00f, 0.32f,
                                               0.20f, 0.47f, 0.11f, 0.29f};

  const glm::mat4 pear_transform =
      glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.45f, 0.0f, 0.0f)),
                 glm::vec3(0.75f));
  const glm::mat4 apple_transform =
      glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.45f, 0.0f, 0.0f)),
                 glm::vec3(0.75f));

  const entt::entity pear =
      ecs.m_add_entity_with_component<Position2D>(pear_pos);
  ecs.m_add_component_to_entity<Color>(pear, pear_colors);
  ecs.m_add_component_to_entity<Transform>(pear, pear_transform);

  const entt::entity pear_leaf =
      ecs.m_add_entity_with_component<Position2D>(leaf_pos);
  ecs.m_add_component_to_entity<Color>(pear_leaf, leaf_colors);
  ecs.m_add_component_to_entity<Transform>(pear_leaf, pear_transform);

  const entt::entity apple =
      ecs.m_add_entity_with_component<Position2D>(apple_pos);
  ecs.m_add_component_to_entity<SolidColor>(apple, 0.9f, 0.05f, 0.05f, 1.0f);
  ecs.m_add_component_to_entity<Transform>(apple, apple_transform);

  const entt::entity apple_leaf =
      ecs.m_add_entity_with_component<Position2D>(apple_leaf_pos);
  ecs.m_add_component_to_entity<SolidColor>(apple_leaf, 0.05f, 0.45f, 0.04f,
                                            1.0f);
  ecs.m_add_component_to_entity<Transform>(apple_leaf, apple_transform);

  // creates the renderer
  constexpr GLfloat clear_color[4] = {0.8f, 0.8f, 1.0f, 1.0f};
  OpenGLRenderer renderer(ecs.m_get_registry(), clear_color);

  // window loop
  while (!glfwWindowShouldClose(window_manager.m_get_window(window))) {
    ecs.m_update_entity_component<Transform>(
        pear, glm::rotate(pear_transform, static_cast<float>(glfwGetTime()),
                          glm::vec3(0.0f, 0.0f, 1.0f)));
    ecs.m_update_entity_component<Transform>(
        pear_leaf,
        glm::rotate(pear_transform, static_cast<float>(glfwGetTime()),
                    glm::vec3(0.0f, 0.0f, 1.0f)));
    ecs.m_update_entity_component<Transform>(
        apple,
        glm::rotate(apple_transform, static_cast<float>(glfwGetTime() * 1.5f),
                    glm::vec3(0.0f, 0.0f, 1.0f)));
    ecs.m_update_entity_component<Transform>(
        apple_leaf,
        glm::rotate(apple_transform, static_cast<float>(glfwGetTime() * 1.5f),
                    glm::vec3(0.0f, 0.0f, 1.0f)));

    renderer.m_draw();

    glfwSwapBuffers(window_manager.m_get_window(window));
    glfwPollEvents();
  }

  renderer.m_destroy();
  window_manager.m_destroy();

  //-----------------end engine example----------------

  return 0;
}
