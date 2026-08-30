module;

#include <vector>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

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
  std::vector<GLuint> m_programs;
};

/**\brief finds groups, creates vaos, uses Buffers to create buffers, uses
 * ShaderGen to generate shaders
 */
OpenGLRenderer::OpenGLRenderer(entt::registry &registry)
    : m_registry(registry), m_buffers(registry) {
  const Groups groups = create_groups(registry);

  m_vaos.resize(groups.m_count);
  glCreateVertexArrays(groups.m_count, m_vaos.data());

  Buffers buffers(m_registry);
  buffers.m_create_buffers(groups);

  ShaderGen shader_gen(registry, groups.m_count, m_vaos);
  std::vector<GLuint> program_ids = shader_gen.m_gen_shaders();
}

void OpenGLRenderer::m_draw() const {
  for (const auto &id : m_vaos) {
    glBindVertexArray(id);
    glDrawArrays(GL_TRIANGLES,
  }
}

OpenGLRenderer::~OpenGLRenderer() { m_buffers.m_delete_buffers(); }
