module;

#include <format>
#include <functional>
#include <iostream>
#include <variant>
#include <vector>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

#include "external/GLFW/glfw3.h"

#include "external/glm/gtc/type_ptr.hpp"

#include "macros.h"

export module OpenGLRenderer;

import ECS;
import Shaders;
import Components;
import ShaderGen;
import Groups;
import Buffers;

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar *message,
                    const void *userParam);

/**\brief the rendering class for the OpenGL 4.5 API
 */
export class OpenGLRenderer {
public:
  OpenGLRenderer(entt::registry &registry,
                 const GLfloat clear_color[4] = m_default_clear_color);

  void m_destroy();

  void m_draw() const;

private:
  entt::registry &m_registry;

  Buffers m_buffers;

  std::vector<GLuint> m_vaos;
  std::vector<EntityInfo> m_entity_info;
  std::vector<GLuint> m_program_ids;

  Groups m_groups;

  const GLfloat *m_clear_color;

  constexpr static GLfloat m_default_clear_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};

  void m_set_uniforms(const size_t group, const size_t entity) const;
};

/**\uses Groups.cppm to create groups, creates vaos, uses Buffers.cppm to create
 * buffers, uses ShaderGen.cppm to generate shaders
 */
OpenGLRenderer::OpenGLRenderer(entt::registry &registry,
                               const GLfloat clear_color[4])
    : m_registry(registry), m_buffers(registry), m_clear_color(clear_color) {

  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

// set up debugging if it is enabled
#ifndef NDEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(debug_callback, nullptr);
#endif

  // create the groups
  m_groups = create_groups(registry);

  // create the vaos
  m_vaos.resize(m_groups.m_count);
  glCreateVertexArrays(m_groups.m_count, m_vaos.data());

  // create the buffers and return the number of vertices for each draw call
  m_entity_info = m_buffers.m_create_buffers(m_groups, m_vaos);

  // generate the shaders
  ShaderGen shader_gen(registry, m_groups, m_vaos);
  m_program_ids = shader_gen.m_gen_shaders();
}

void OpenGLRenderer::m_draw() const {
  // clear the color buffer
  glClearBufferfv(GL_COLOR, 0, m_clear_color);

  for (size_t i = 0; i < m_vaos.size(); ++i) {
    glBindVertexArray(m_vaos[i]);
    glUseProgram(m_program_ids[i]);

    const EntityInfo &entity_info = m_entity_info[i];

    bool indexed = false;
    std::vector<const void *> index_offsets;
    if (m_groups.m_groups[i].m_indexed) {
      indexed = true;

      // these all need to be casted to const void* before we can draw them
      for (size_t offset : entity_info.m_base_indices) {
        index_offsets.push_back(
            reinterpret_cast<const void *>(offset * sizeof(GLuint)));
      }
    }

    // we can multi-draw if there are not any uniforms in the group
    if (entity_info.m_uniforms[0].empty()) {
      if (indexed) {
        glMultiDrawElementsBaseVertex(
            GL_TRIANGLES, entity_info.m_num_indices.data(), GL_UNSIGNED_INT,
            index_offsets.data(), entity_info.m_num_indices.size(),
            entity_info.m_base_vertex.data());
      } else {
        glMultiDrawArrays(GL_TRIANGLES, entity_info.m_base_vertex.data(),
                          entity_info.m_num_vertices.data(),
                          entity_info.m_num_vertices.size());
      }
    } else {

      bool batchable = true;
      const auto &first_uniform = entity_info.m_uniforms[0];
      for (size_t j = 1; j < entity_info.m_uniforms.size(); ++j) {
        const auto &curr_uniform = entity_info.m_uniforms[j];

        // check batchability of the uniforms by comparing the data of each
        // uniform in the group
        for (size_t k = 0; k < first_uniform.size(); ++k) {
          std::visit(
              // for glm data
              [&](const auto &prev_uniform, const auto &curr_uniform) {
                if constexpr (std::is_same_v<
                                  std::decay_t<decltype(prev_uniform)>,
                                  std::reference_wrapper<Transform>> &&
                              std::is_same_v<
                                  std::decay_t<decltype(curr_uniform)>,
                                  std::reference_wrapper<Transform>>) {
                  if (prev_uniform.get().m_data != curr_uniform.get().m_data) {
                    batchable = false;
                  }

                  // for std::vector data
                } else if constexpr (
                    !(std::is_same_v<std::decay_t<decltype(prev_uniform)>,
                                     std::reference_wrapper<Transform>> ||
                      std::is_same_v<std::decay_t<decltype(curr_uniform)>,
                                     std::reference_wrapper<Transform>>)) {
                  if (prev_uniform.get().m_data != curr_uniform.get().m_data) {
                    batchable = false;
                  }
                }
              },
              first_uniform[k], curr_uniform[k]);

          if (!batchable)
            break;
        }

        if (!batchable)
          break;
      }

      if (batchable) {
        // sets the uniforms based on their type at runtime
        m_set_uniforms(i, 0);

        // draw the entity
        if (indexed) {
          glMultiDrawElementsBaseVertex(
              GL_TRIANGLES, entity_info.m_num_indices.data(), GL_UNSIGNED_INT,
              index_offsets.data(), entity_info.m_num_indices.size(),
              entity_info.m_base_vertex.data());
        } else {
          glMultiDrawArrays(GL_TRIANGLES, entity_info.m_base_vertex.data(),
                            entity_info.m_num_vertices.data(),
                            entity_info.m_num_vertices.size());
        }

      } else {
        for (size_t j = 0; j < entity_info.m_uniforms.size(); ++j) {
          // sets the uniforms based on their type at runtime
          m_set_uniforms(i, j);

          // draw the entity
          if (indexed) {
            const void *indices_ptr = reinterpret_cast<const void *>(
                entity_info.m_base_indices[j] * sizeof(GLuint));
            glDrawElementsBaseVertex(GL_TRIANGLES, entity_info.m_num_indices[j],
                                     GL_UNSIGNED_INT, index_offsets[j],
                                     entity_info.m_base_vertex[j]);
          } else {
            glDrawArrays(GL_TRIANGLES, entity_info.m_base_vertex[j],
                         entity_info.m_num_vertices[j]);
          }
        }
      }
    }
  }
}

void OpenGLRenderer::m_set_uniforms(const size_t group,
                                    const size_t entity) const {
  const auto &uniforms = m_entity_info[group].m_uniforms[entity];

  for (const auto &uniform : uniforms) {
    if (std::holds_alternative<std::reference_wrapper<SolidColor>>(uniform)) {
      const auto &color =
          std::get<std::reference_wrapper<SolidColor>>(uniform).get().m_data;
      GLint color_loc = glGetUniformLocation(m_program_ids[group], "color");
      glUniform4f(color_loc, color[0], color[1], color[2], color[3]);
    } else if (std::holds_alternative<std::reference_wrapper<Transform>>(
                   uniform)) {
      const auto &transform =
          std::get<std::reference_wrapper<Transform>>(uniform).get().m_data;
      GLint model_loc = glGetUniformLocation(m_program_ids[group], "model");
      glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(transform));
    }
  }
}

void OpenGLRenderer::m_destroy() {
  m_buffers.m_delete_buffers();
  glDeleteVertexArrays(m_vaos.size(), m_vaos.data());
  for (GLuint program_id : m_program_ids)
    glDeleteProgram(program_id);
}

void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar *message,
                    const void *userParam) {
  std::cerr << std::format("OpenGL Debug Message: {}\n", message);
}
