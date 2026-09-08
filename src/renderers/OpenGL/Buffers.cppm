module;

#include <variant>
#include <vector>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

export module Buffers;

import Groups;
import Components;
import Shaders;

using uniforms_variant = std::variant<SolidColor>;

export struct EntityInfo {
  std::vector<size_t> m_num_vertices;
  std::vector<size_t> m_base_vertex;
  std::vector<std::vector<uniforms_variant>> m_uniforms;

  EntityInfo(size_t num_vertices, size_t base_vertex)
      : m_num_vertices({num_vertices}), m_base_vertex({base_vertex}) {}
};

export class Buffers {
public:
  Buffers(entt::registry &registry);

  std::vector<EntityInfo> m_create_buffers(const Groups &groups,
                                           const std::vector<GLuint> &vao_ids);

  void m_delete_buffers();

private:
  entt::registry &m_registry;

  std::vector<std::vector<GLuint>> m_vbo_ids;
  std::vector<std::vector<GLsizei>> m_vbo_strides;
  std::vector<std::vector<std::vector<GLfloat>>> m_data;
  std::vector<std::vector<size_t>> m_data_sizes;
  std::vector<size_t> m_num_vertices;
  std::vector<size_t> m_curr_pos_vbo_offsets;

  // this will hold the number of vertices and the base vertex for each entity
  std::vector<EntityInfo> m_entity_vertex_info;

  std::vector<EntityInfo> m_create_vbos(const Groups &groups,
                                        const std::vector<GLuint> &vbo_ids);

  void m_delete_vbos();
  void m_create_ebo();

  template <typename T>
  void m_add_component_to_buffer(const entt::entity entity);
};

Buffers::Buffers(entt::registry &registry) : m_registry(registry) {}

std::vector<EntityInfo>
Buffers::m_create_buffers(const Groups &groups,
                          const std::vector<GLuint> &vao_ids) {
  return m_create_vbos(groups, vao_ids);
}

/**\brief create the VBOs and insert their data
 */
std::vector<EntityInfo>
Buffers::m_create_vbos(const Groups &groups,
                       const std::vector<GLuint> &vao_ids) {
  // resize the outer vectors
  m_vbo_ids.resize(groups.m_count);
  m_vbo_strides.resize(groups.m_count);
  m_data_sizes.resize(groups.m_count);
  m_num_vertices.resize(groups.m_count);
  m_data.resize(groups.m_count);
  m_curr_pos_vbo_offsets.resize(groups.m_count);

  // resize the inner vectors
  for (size_t i = 0; i < groups.m_count; ++i) {
    const size_t num_vbos = groups.m_groups[i].m_num_vbos;
    m_vbo_ids[i].resize(num_vbos);
    m_data[i].resize(num_vbos);
    m_data_sizes[i].resize(num_vbos);
    m_vbo_strides[i].resize(num_vbos);
  }

  // reserve the entity vertex info vector
  m_entity_vertex_info.reserve(groups.m_count);
  for (size_t i = 0; i < groups.m_count; ++i) {
    m_entity_vertex_info.emplace_back(0, 0);
  }

  // create the buffers
  for (size_t i = 0; i < groups.m_count; ++i) {
    size_t num_vbos = groups.m_groups[i].m_num_vbos;
    m_vbo_ids[i].resize(num_vbos);
    glCreateBuffers(num_vbos, m_vbo_ids[i].data());
  }

  m_registry.view<entt::entity>().each([&](const entt::entity entity) {
    const size_t group_id = m_registry.get<GroupID>(entity).m_id;

    // check the component mask and add components accordingly
    const auto &bitmask = m_registry.get<ComponentMask>(entity).m_bits;
    if (bitmask.test(ComponentID_v<Position2D>)) {
      m_add_component_to_buffer<Position2D>(entity);

      size_t num_vertices =
          m_registry.get<Position2D>(entity).m_data.size() / 2.0;

      m_entity_vertex_info[group_id] = {num_vertices,
                                        m_curr_pos_vbo_offsets[group_id]};

      m_curr_pos_vbo_offsets[group_id] += num_vertices;
    }

    if (bitmask.test(ComponentID_v<Position3D>)) {
      m_add_component_to_buffer<Position3D>(entity);

      m_num_vertices[group_id] +=
          m_registry.get<Position3D>(entity).m_data.size() / 3.0;

      size_t num_vertices =
          m_registry.get<Position3D>(entity).m_data.size() / 3.0;

      m_entity_vertex_info[group_id] = {num_vertices,
                                        m_curr_pos_vbo_offsets[group_id]};

      m_curr_pos_vbo_offsets[group_id] += num_vertices;
    }

    // add the uniforms for the entity
    std::vector<uniforms_variant> uniforms;

    if (bitmask.test(ComponentID_v<SolidColor>)) {
      uniforms.push_back(m_registry.get<SolidColor>(entity));
    }

    m_entity_vertex_info[group_id].m_uniforms.push_back(uniforms);
  });

  // transfer the data to the gl buffers
  for (size_t i = 0; i < m_data.size(); ++i) {
    for (size_t j = 0; j < m_data[i].size(); ++j) {
      glNamedBufferStorage(m_vbo_ids[i][j], m_data_sizes[i][j],
                           m_data[i][j].data(), 0);

      glVertexArrayVertexBuffer(vao_ids[i], j, m_vbo_ids[i][j], 0,
                                m_vbo_strides[i][j]);
    }
  }

  // return the number of vertices for each group and each vbo
  return m_entity_vertex_info;
}

void Buffers::m_create_ebo() {}

template <typename T>
void Buffers::m_add_component_to_buffer(const entt::entity entity) {
  // get the data of the component
  using data_type = typename decltype(T::m_data)::value_type;
  std::vector<data_type> vbo_data = m_registry.get<T>(entity).m_data;

  // append the data to the end of the corresponding buffer
  auto &target_buffer =
      m_data[m_registry.get<GroupID>(entity).m_id][BufferBinding_v<T>];
  target_buffer.insert(target_buffer.end(), vbo_data.begin(), vbo_data.end());

  // update the size of the buffer
  m_data_sizes[m_registry.get<GroupID>(entity).m_id][BufferBinding_v<T>] +=
      vbo_data.size() * sizeof(data_type);

  // set the stride of the buffer
  m_vbo_strides[m_registry.get<GroupID>(entity).m_id][BufferBinding_v<T>] +=
      ComponentSize_v<T>;
}

void Buffers::m_delete_buffers() { m_delete_vbos(); }

void Buffers::m_delete_vbos() {
  for (size_t i = 0; i < m_vbo_ids.size(); ++i) {
    glDeleteBuffers(m_vbo_ids[i].size(), m_vbo_ids[i].data());
  }
}
