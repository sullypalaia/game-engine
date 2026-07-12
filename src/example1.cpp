//

#include <iostream>

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
import Materials;
import Components;
import OpenGLRenderer;

int main() {
  // glfw setup
  glfwInit();
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#ifndef NDEBUG
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

  // create window
  Window window(1920, 1080, "engine", NULL, NULL);

  // camera setup
  /*
  Camera camera(CameraData{glm::ortho(-1.0f * window.m_get_aspect_ratio(),
                                      window.m_get_aspect_ratio(), -1.0f, 1.0f),
                           glm::mat4(1.0f)});
  */

  Camera camera(CameraData{glm::mat4(1.0f), glm::mat4(1.0f)});

  //---------------------testing----------------------

  std::string vert_shader = "shaders/example1.vert";
  std::string frag_shader = "shaders/example1.frag";
  GLuint program = generate_id(&vert_shader, &frag_shader);
  if (program == ERROR_ID) {
    std::cerr << "bad ID generated\n";
    return -1;
  }

  // vertex data
  constexpr GLfloat pos[]{
      -0.5f, -0.5f, // bottom left
      0.5f,  -0.5f, // bottom right
      0.0f,  0.5f,  // top
  };
  const void *pos_ptr = pos;
  constexpr GLsizeiptr pos_size = sizeof(pos);

  const VertexData vertex_data{&pos_size, &pos_ptr};
  const RenderInfo render_info{GL_TRIANGLES, 3, 0};
  const Attrib pos_attrib{0, 2, 0, GL_FLOAT};

  ECS ecs;

  const entt::entity entity =
      ecs.m_add_entity_with_component<VertexData>(vertex_data);
  ecs.m_add_component_to_entity<RenderInfo>(entity, render_info);
  ecs.m_add_component_to_entity<Attribs>(
      entity, &pos_attrib, static_cast<size_t>(1), static_cast<size_t>(1));
  ecs.m_add_component_to_entity<ShaderProgram>(entity, program);

  OpenGLRenderer renderer(ecs.m_get_registry());
  renderer.m_init();

  //---------------------end testing-----------------------

  constexpr GLfloat clear_color[4]{1.0f, 1.0f, 1.0f, 0.0f};

  while (!glfwWindowShouldClose(window.m_get_window())) {
    glClearBufferfv(GL_COLOR, 0, clear_color);

    renderer.m_update();

    ecs.m_update();

    glfwSwapBuffers(window.m_get_window());
    glfwPollEvents();
  }

  window.m_destroy();

  return 0;
}
