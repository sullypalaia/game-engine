module;

#include <functional>
#include <variant>
#include <vector>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

#include "external/glm/glm.hpp"
#include "external/glm/gtc/type_ptr.hpp"

#include "macros.h"

export module OpenGLRenderer;

import ECS;
import Shaders;
import Components;
import ShaderGen;
import Groups;
import Buffers;

/**\brief the rendering class for the OpenGL 4.5 API
 */
export class OpenGLRenderer {
public:
  OpenGLRenderer(entt::registry &registry);

  ~OpenGLRenderer();

  void m_draw() const;

private:
  entt::registry &m_registry;

  Buffers m_buffers;

  std::vector<GLuint> m_vaos;
  std::vector<EntityInfo> m_entity_info;
  std::vector<GLuint> m_program_ids;

  Groups m_groups;
};

/**\uses Groups.cppm to create groups, creates vaos, uses Buffers.cppm to create
 * buffers, uses ShaderGen.cppm to generate shaders
 */
OpenGLRenderer::OpenGLRenderer(entt::registry &registry)
    : m_registry(registry), m_buffers(registry) {
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
  for (size_t i = 0; i < m_vaos.size(); ++i) {
    glBindVertexArray(m_vaos[i]);
    glUseProgram(m_program_ids[i]);

    const EntityInfo &entity_info = m_entity_info[i];

    // we can multi-draw if there are not any uniforms in the group
    if (entity_info.m_uniforms[0].empty()) {
      glMultiDrawArrays(GL_TRIANGLES, entity_info.m_base_vertex.data(),
                        entity_info.m_num_vertices.data(),
                        entity_info.m_num_vertices.size());
    } else {
      for (size_t j = 0; j < entity_info.m_num_vertices.size(); ++j) {

        // set the uniforms based on their type at runtime
        for (const auto &uniform : entity_info.m_uniforms[j]) {
          std::visit(
              [&](const auto &u) {
                using T = std::decay_t<decltype(u)>;

                const auto &data = u.get().m_data;

                if constexpr (std::is_same_v<
                                  T, std::reference_wrapper<SolidColor>>) {
                  GLint color_loc =
                      glGetUniformLocation(m_program_ids[i], "color");
                  glUniform4f(color_loc, data[0], data[1], data[2], data[3]);
                }

                if constexpr (std::is_same_v<
                                  T, std::reference_wrapper<Transform>>) {
                  GLint model_loc =
                      glGetUniformLocation(m_program_ids[i], "model");
                  glUniformMatrix4fv(model_loc, 1, GL_FALSE,
                                     glm::value_ptr(data));
                }
              },
              uniform);
        }

        // draw the entity
        glDrawArrays(GL_TRIANGLES, entity_info.m_base_vertex[j],
                     entity_info.m_num_vertices[j]);
      }
    }
  }
}

OpenGLRenderer::~OpenGLRenderer() { m_buffers.m_delete_buffers(); }
