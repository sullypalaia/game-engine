#include <functional>
#include <vector>

#include "../src/scene/utils/definitions.h"
#include "GLFW/glfw3.h"
#include "entt/entt.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "../src/utils/macros.h"

import engine.ecs;
import engine.graphics.apis.opengl;
import engine.platform.window_manager;
import engine.assets.importer;
import engine.scene.camera_manager;

int main() {
  WindowManager window_manager(false);
  const size_t window = window_manager.m_create_window(
      1920, 1080, "pumpkin pie", nullptr, nullptr);

  window_manager.m_make_context_current(window);

  ECS ecs;

  const float clear_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};

  CameraManager camera_manager(window_manager);
  const size_t camera = camera_manager.m_create_first_person_camera(
      glm::vec3(0.0f, 0.0f, 1.0f), -90.0f, 0.0f, 0.0f, 0.1f, 1.0f, 45.0f, 0.1f,
      100.0f, window);

  Importer importer;
  const std::vector<entt::entity> entities =
      importer.m_import({"example_assets/scene.gltf"}, ecs);

  int count = 0;
  for (const auto &entity : entities) {
    if (count % 2 == 0) {
      ecs.m_add_component_to_entity<SolidColor>(
          entity, std::move(std::vector<float>{1.0f, 0.0f, 0.0f, 1.0f}));
    } else {
      ecs.m_add_component_to_entity<SolidColor>(
          entity, std::move(std::vector<float>{0.0f, 1.0f, 0.0f, 1.0f}));
    }

    ++count;
  }

  OpenGLRenderer renderer(ecs.m_get_registry(), clear_color);

  std::function<void()> update_func = [&]() {
    camera_manager.m_update();
    renderer.m_draw();
  };

  window_manager.m_loop(update_func);

  camera_manager.m_destroy();
  renderer.m_destroy();
  window_manager.m_destroy();

  return 0;
}
