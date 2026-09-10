// this renders different primitives separately, each with different attributes
// - no multi-draw

#include "external/glad/glad.h"

#include "external/GLFW/glfw3.h"

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"

#include "entt/entt.hpp"

#include "macros.h"

import Meshes;
import ECS;
import Shaders;
import Camera;
import Window;
import Components;
import OpenGLRenderer;

int main() {
  //----------------window setup--------------
  // glfw setup
  //

  // use x11 for renderdoc support
  glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

  glfwInit();
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);

#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

  //-------------end window setup--------------

  //-------------begin engine example-----------

  // create window
  Window window(1920, 1080, "engine", NULL, NULL);

  // camera setup
  Camera camera(CameraData{glm::ortho(-1.0f * window.m_get_aspect_ratio(),
                                      window.m_get_aspect_ratio(), -1.0f, 1.0f),
                           glm::mat4(1.0f)});

  // triangle vertices
  std::vector<GLfloat> triangle_pos = {
      -0.5f, -0.5f, // bottom left
      0.5f,  -0.5f, // bottom right
      0.0f,  0.5f   // top
  };

  std::vector<GLfloat> triangle_pos1 = {
      -0.2f, 0.0f, // bottom left
      0.2f,  0.0f, // bottom right
      0.0f,  0.4f  // top
  };

  std::vector<GLfloat> triangle_pos2 = {
      -0.2f, -0.4f, // bottom left
      0.2f,  -0.4f, // bottom right
      0.0f,  0.0f   // top
  };

  std::vector<GLfloat> triangle_colors = {
      1.0f, 0.0f, 0.0f, 1.0f, // bottom left
      0.0f, 1.0f, 0.0f, 1.0f, // bottom right
      0.0f, 0.0f, 1.0f, 1.0f  // top
  };

  // rectangle vertices
  constexpr GLfloat rect_pos[]{
      -0.5f, -0.5f, // bottom left - 0
      0.5f,  -0.5f, // bottom right - 1
      -0.5f, 0.5f,  // top left - 2
      0.5f,  0.5f   // top right - 3
  };

  // rectangle indices
  constexpr GLuint rect_indices[]{
      2, 0, 3, // top
      0, 1, 3  // bottom
  };

  // rectangle colors are separate because they are updated every frame
  GLfloat rect_colors[]{
      0.0f, 1.0f, 0.0f, 1.0f, // bottom left
      0.0f, 1.0f, 0.0f, 1.0f, // bottom right
      0.0f, 0.0f, 1.0f, 1.0f, // top left
      0.0f, 0.0f, 1.0f, 1.0f  // top right
  };

  ECS ecs;

  const glm::mat4 entity1transform =
      glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)),
                 glm::vec3(0.2f));

  const glm::mat4 entity2transform =
      glm::scale(glm::mat4(1.0f), glm::vec3(0.2f));

  const glm::mat4 entity3transform =
      glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f)),
                 glm::vec3(0.2f));

  const entt::entity entity1 =
      ecs.m_add_entity_with_component<Position2D>(triangle_pos);
  ecs.m_add_component_to_entity<SolidColor>(entity1, 0.0f, 0.0f, 1.0f, 1.0f);
  ecs.m_add_component_to_entity<Transform>(entity1, entity1transform);

  const entt::entity entity2 =
      ecs.m_add_entity_with_component<Position2D>(triangle_pos);
  ecs.m_add_component_to_entity<Color>(entity2, triangle_colors);
  ecs.m_add_component_to_entity<Transform>(entity2, entity2transform);

  const entt::entity entity3 =
      ecs.m_add_entity_with_component<Position2D>(triangle_pos);
  ecs.m_add_component_to_entity<SolidColor>(entity3, 1.0f, 0.0f, 0.0f, 1.0f);
  ecs.m_add_component_to_entity<Transform>(entity3, entity3transform);

  const entt::entity entity4 =
      ecs.m_add_entity_with_component<Position2D>(triangle_pos1);
  ecs.m_add_component_to_entity<Color>(entity4, triangle_colors);

  const entt::entity entity5 =
      ecs.m_add_entity_with_component<Position2D>(triangle_pos2);
  ecs.m_add_component_to_entity<Color>(entity5, triangle_colors);

  {
    // make sure the destructor is called before the window is destroyed
    OpenGLRenderer renderer(ecs.m_get_registry());

    constexpr GLfloat clear_color[4]{1.0f, 1.0f, 1.0f, 0.0f};

    while (!glfwWindowShouldClose(window.m_get_window())) {
      glClearBufferfv(GL_COLOR, 0, clear_color);

      // yes, I know I know this is a bad way to do this, but I don't care for
      // this example right now

      ecs.m_update_entity_component<Transform>(
          entity1,
          glm::rotate(entity1transform, static_cast<float>(glfwGetTime()),
                      glm::vec3(0.0f, 0.0f, 1.0f)));

      ecs.m_update_entity_component<Transform>(
          entity2,
          glm::rotate(entity2transform, static_cast<float>(glfwGetTime()),
                      glm::vec3(0.0f, 0.0f, -1.0f)));

      ecs.m_update_entity_component<Transform>(
          entity3, glm::rotate(entity3transform,
                               static_cast<float>(glfwGetTime() * 2.0f),
                               glm::vec3(0.0f, 0.0f, 1.0f)));

      renderer.m_draw();

      glfwSwapBuffers(window.m_get_window());
      glfwPollEvents();
    }
  }

  window.m_destroy();

  //-----------------end engine example----------------

  return 0;
}
