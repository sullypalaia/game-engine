#include <vector>

#include "GLFW/glfw3.h"
#include "entt/entt.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

import engine.ecs;
import engine.graphics.apis.opengl;
import engine.platform.window_manager;
import engine.assets.importer;
import engine.scene.camera;

int main() {
  WindowManager window_manager(true);
  const size_t window = window_manager.m_create_window(
      1920, 1080, "pumpkin pie", nullptr, nullptr, true);

  window_manager.m_make_context_current(window);

  ECS ecs;

  float clear_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

  Camera camera(
      CameraData{glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f), glm::mat4(1.0f)});

  Importer importer;
  const std::vector<entt::entity> entities =
      importer.m_import({"example_assets/scene.gltf"}, ecs);

  for (const auto &entity : entities) {
    ecs.m_add_component_to_entity<SolidColor>(
        entity, std::move(std::vector<float>{0.0f, 0.0f, 0.0f, 1.0f}));
  }

  OpenGLRenderer renderer(ecs.m_get_registry(), clear_color);

  while (!glfwWindowShouldClose(window_manager.m_get_window(window))) {
    renderer.m_draw();

    glfwSwapBuffers(window_manager.m_get_window(window));
    glfwPollEvents();
  }

  renderer.m_destroy();
  window_manager.m_destroy();

  return 0;
}
