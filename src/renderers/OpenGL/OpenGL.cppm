module;

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

    for (size_t j = 0; j < entity_info.m_num_vertices.size(); ++j) {
      // set the uniforms based on their type at runtime
      for (const auto &uniform : entity_info.m_uniforms[j]) {
        std::visit(
            [&](const auto &u) {
              using T = std::decay_t<decltype(u)>;
              if constexpr (std::is_same_v<T, SolidColor>) {
                GLint color_loc =
                    glGetUniformLocation(m_program_ids[i], "color");
                glUniform4f(color_loc, u.m_data[0], u.m_data[1], u.m_data[2],
                            u.m_data[3]);
              }

              if constexpr (std::is_same_v<T, Transform>) {
                GLint model_loc =
                    glGetUniformLocation(m_program_ids[i], "model");
                glUniformMatrix4fv(model_loc, 1, GL_FALSE,
                                   glm::value_ptr(u.m_data));
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

OpenGLRenderer::~OpenGLRenderer() { m_buffers.m_delete_buffers(); }
