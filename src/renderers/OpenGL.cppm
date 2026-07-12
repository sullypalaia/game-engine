module;

#include <vector>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"

#include "macros.h"

export module OpenGLRenderer;

import ECS;
import Materials;
import Shaders;
import Components;

// non-indexed mesh
struct Mesh {
  const GLuint m_vao;
  const GLint m_num_vertices;
};

// indexed mesh
struct IndexedMesh {
  const GLuint m_vao;
  const GLint m_num_indices;
};

export class OpenGLRenderer {
public:
  OpenGLRenderer(entt::registry &registry);

  ~OpenGLRenderer();

  void m_init();

  void m_update();

private:
  entt::registry &m_registry;

  std::vector<GLuint> m_vaos;
  std::vector<GLuint> m_ebos;
  std::vector<std::vector<GLuint>> m_vbos;

  void m_init_vertex_buffers(const entt::entity entity,
                             std::vector<GLuint> &vaos, const size_t num_vbos);
  void m_init_element_buffer(GLuint &vao, const entt::entity entity);
  void m_init_entities();

  void m_update_materials(entt::entity entity);
  void m_update_transforms(entt::entity entity);

  void m_draw();
};

OpenGLRenderer::OpenGLRenderer(entt::registry &registry)
    : m_registry(registry) {}

void OpenGLRenderer::m_init_vertex_buffers(const entt::entity entity,
                                           std::vector<GLuint> &vbos,
                                           const size_t num_vbos) {
  glCreateBuffers(num_vbos, vbos.data());

  const VertexData &data = m_registry.get<VertexData>(entity);

  for (size_t i = 0; i < num_vbos; ++i)
    glNamedBufferStorage(vbos[i], data.m_sizes[i], data.m_data[i], 0);
}

void OpenGLRenderer::m_init_element_buffer(GLuint &vao,

                                           const entt::entity entity) {
  const ElementBuffer &buffer = m_registry.get<ElementBuffer>(entity);

  GLuint ebo;
  glCreateBuffers(1, &ebo);
  m_ebos.push_back(ebo);
  glNamedBufferStorage(ebo, 1, buffer.m_data, 0);

  glVertexArrayElementBuffer(vao, ebo);
}

void OpenGLRenderer::m_init_entities() {
  m_registry.view<Attribs>().each([&](entt::entity entity, Attribs &attribs) {
    GLuint vao;
    glCreateVertexArrays(1, &vao);
    m_vaos.push_back(vao);

    size_t num_attribs = attribs.m_num_attribs;
    size_t num_attrib_groups = attribs.m_num_attrib_groups;

    assert(num_attribs > 0 && num_attrib_groups > 0);

    std::vector<GLuint> vbos;
    vbos.resize(num_attrib_groups);

    m_init_vertex_buffers(entity, vbos, num_attrib_groups);

    std::vector<GLuint> offsets(num_attrib_groups, 0);

    for (size_t i = 0; i < num_attribs; ++i) {
      const Attrib &attrib = attribs.m_attribs[i];
      const GLuint attrib_loc = attrib.m_loc;
      const GLint attrib_size = attrib.m_size;
      const GLenum attrib_type = attrib.m_type;
      const GLint attrib_group = attrib.m_attrib_group;

      size_t type_size;
      switch (attrib_type) {
      case GL_FLOAT:
        type_size = sizeof(GLfloat);
        break;
      default:
        type_size = BAD_TYPE;
        break;
      }

      set_attrib(vao, attrib_loc, attrib_group, attrib_size, attrib_type,
                 GL_FALSE, offsets[attrib_group], 0);

      offsets[attrib_group] += attrib_size * type_size;
    }

    for (size_t i = 0; i < num_attrib_groups; ++i)
      glVertexArrayVertexBuffer(vao, i, vbos[i], 0, offsets[i]);

    m_vbos.push_back(std::move(vbos));

    if (m_registry.all_of<ElementBuffer>(entity)) {
      m_init_element_buffer(vao, entity);
      m_registry.emplace<IndexedMesh>(
          entity, vao, m_registry.get<RenderInfo>(entity).m_num_indices);
    } else {
      m_registry.emplace<Mesh>(
          entity, vao, m_registry.get<RenderInfo>(entity).m_num_vertices);
    }
  });
}

void OpenGLRenderer::m_update_materials(entt::entity entity) {
  if (m_registry.all_of<SolidColor>(entity)) {
    glUseProgram(m_registry.get<ShaderProgram>(entity).m_id);

    const SolidColor &color = m_registry.get<SolidColor>(entity);
    glUniform4f(color.m_attrib_location, color.m_color[0], color.m_color[1],
                color.m_color[2], color.m_color[3]);
  }
}

void OpenGLRenderer::m_update_transforms(entt::entity entity) {
  glUseProgram(m_registry.get<ShaderProgram>(entity).m_id);

  const Transform &transform = m_registry.get<Transform>(entity);
  glUniformMatrix4fv(transform.m_transform_loc, 1, GL_FALSE,
                     glm::value_ptr(transform.m_transform));
}

void OpenGLRenderer::m_update() {
  m_registry.view<entt::entity>().each([this](const entt::entity entity) {
    if (m_registry.all_of<Transform>(entity))
      m_update_transforms(entity);

    if (m_registry.any_of<SolidColor>(entity))
      m_update_materials(entity);
  });

  m_draw();
}

void OpenGLRenderer::m_init() { m_init_entities(); }

void OpenGLRenderer::m_draw() {
  m_registry.view<Mesh>().each(
      [this](const entt::entity entity, const Mesh &mesh) {
        glBindVertexArray(mesh.m_vao);
        glUseProgram(m_registry.get<ShaderProgram>(entity).m_id);
        glDrawArrays(m_registry.get<RenderInfo>(entity).m_primitive_type, 0,
                     mesh.m_num_vertices);
      });
  m_registry.view<IndexedMesh>().each(
      [this](const entt::entity entity, const IndexedMesh &mesh) {
        glBindVertexArray(mesh.m_vao);
        glUseProgram(m_registry.get<ShaderProgram>(entity).m_id);
        glDrawElements(m_registry.get<RenderInfo>(entity).m_primitive_type,
                       mesh.m_num_indices, GL_UNSIGNED_INT, 0);
      });
}

OpenGLRenderer::~OpenGLRenderer() {
  glDeleteVertexArrays(m_vaos.size(), m_vaos.data());
  for (const auto &buff_group : m_vbos)
    glDeleteBuffers(buff_group.size(), buff_group.data());
  glDeleteBuffers(m_ebos.size(), m_ebos.data());
}
