module;

#include <bitset>
#include <functional>
#include <unordered_set>
#include <variant>
#include <vector>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

export module Buffers;

import Groups;
import Components;
import Shaders;

using uniforms_variant = std::variant<std::reference_wrapper<SolidColor>,
                                      std::reference_wrapper<Transform>>;
// at some point add variant support for the buffer data

export struct EntityInfo {
  std::vector<int> m_num_vertices;
  std::vector<int> m_base_vertex;
  std::vector<int> m_num_indices;
  std::vector<int> m_base_indices;
  std::vector<std::vector<uniforms_variant>> m_uniforms;

  EntityInfo() = default;
};

export class Buffers {
public:
  Buffers(entt::registry &registry);

  void m_delete_buffers();

  /**\brief creates the vbos and ebos for a group of entities and returns the
   * EntityInfo associated with each group
   * */
  std::vector<EntityInfo> m_create_buffers(const Groups &groups,
                                           const std::vector<GLuint> &vbo_ids);

private:
  entt::registry &m_registry;

  std::vector<std::vector<GLuint>> m_vbo_ids;
  std::vector<std::vector<GLsizei>> m_vbo_strides;
  std::vector<std::vector<std::vector<GLfloat>>> m_vbo_data;
  std::vector<std::vector<size_t>> m_vbo_data_sizes;
  std::vector<size_t> m_curr_pos_vbo_offsets;
  std::vector<std::vector<std::vector<size_t>>> m_curr_component_offsets;
  std::vector<std::vector<std::vector<size_t>>> m_num_rows;
  std::vector<std::vector<std::unordered_set<size_t>>> m_added_entities;
  std::vector<std::vector<std::vector<int>>> m_curr_entity_base_vertex;

  std::vector<GLuint> m_ebo_ids;
  std::vector<std::vector<size_t>> m_curr_entity_base_index;
  std::vector<size_t> m_ebo_data_sizes;
  std::vector<std::vector<GLuint>> m_ebo_data;

  // this will hold the number of vertices and the base vertex for each
  // entity
  std::vector<EntityInfo> m_entity_info;

  template <typename T>
  void m_add_component_to_buffer(const entt::entity entity,
                                 const Groups &groups,
                                 const size_t num_values_per_row,
                                 const size_t curr_entity);
};

Buffers::Buffers(entt::registry &registry) : m_registry(registry) {}

/**\brief create the VBOs and insert their data
 */
std::vector<EntityInfo>
Buffers::m_create_buffers(const Groups &groups,
                          const std::vector<GLuint> &vao_ids) {

  const size_t num_groups = groups.m_count;

  // resize the outer vectors
  m_vbo_ids.resize(num_groups);
  m_vbo_strides.resize(num_groups);
  m_vbo_data_sizes.resize(num_groups);
  m_vbo_data.resize(num_groups);
  m_curr_pos_vbo_offsets.resize(num_groups);
  m_curr_component_offsets.resize(num_groups);
  m_num_rows.resize(num_groups);
  m_added_entities.resize(num_groups);
  m_curr_entity_base_vertex.resize(num_groups);
  m_ebo_ids.resize(num_groups);
  m_curr_entity_base_index.resize(num_groups);
  m_ebo_data_sizes.resize(num_groups);
  m_ebo_data.resize(num_groups);

  // resize the inner vectors
  for (size_t i = 0; i < num_groups; ++i) {
    const size_t num_vbos = groups.m_groups[i].m_num_vbos;
    m_vbo_ids[i].resize(num_vbos);
    m_vbo_data[i].resize(num_vbos);
    m_vbo_data_sizes[i].resize(num_vbos);
    m_vbo_strides[i].resize(num_vbos);
    m_curr_component_offsets[i].resize(num_vbos);
    m_num_rows[i].resize(num_vbos);
    m_added_entities[i].resize(num_vbos);
    m_curr_entity_base_vertex[i].resize(num_vbos);

    // check if the group has indices and create an ebo if it does
    if (groups.m_groups[i].m_indexed) {
      m_ebo_ids.resize(num_groups);
      glCreateBuffers(1, &m_ebo_ids[i]);
    }
  }

  // reserve the entity vertex info vector
  m_entity_info.resize(num_groups);

  // create the vbos
  for (size_t i = 0; i < num_groups; ++i) {
    size_t num_vbos = groups.m_groups[i].m_num_vbos;
    m_vbo_ids[i].resize(num_vbos);
    glCreateBuffers(num_vbos, m_vbo_ids[i].data());
  }

  std::vector<size_t> curr_entities(num_groups);
  m_registry.view<entt::entity>().each([&](const entt::entity entity) {
    const size_t group_id = m_registry.get<GroupID>(entity).m_id;

    const size_t curr_entity = curr_entities[group_id];

    // check the component mask and add components accordingly
    const auto &bitmask = m_registry.get<ComponentMask>(entity).m_bits;

    // if this is indexed, we don't need to add the number of vertices and
    // base vertex to the entity info, since we will be using the ebo
    bool indexed = false;

    // test for indices and add them to the entity info if they exist
    if (bitmask.test(ComponentID_v<Indices>)) {
      indexed = true;

      m_curr_entity_base_index[group_id].resize(curr_entity + 2);

      const auto &indices = m_registry.get<Indices>(entity).m_data;
      const size_t base_index = m_curr_entity_base_index[group_id][curr_entity];
      const int num_indices = indices.size();

      m_entity_info[group_id].m_num_indices.push_back(num_indices);
      m_entity_info[group_id].m_base_indices.push_back(base_index);

      m_curr_entity_base_index[group_id][curr_entity + 1] =
          base_index + num_indices;

      // copy the indices into the ebo data for the group
      std::copy(indices.begin(), indices.end(),
                std::back_inserter(m_ebo_data[group_id]));

      // update the size of the ebo data for the gruop
      m_ebo_data_sizes[group_id] = m_ebo_data[group_id].size() * sizeof(GLuint);
    }

    if (bitmask.test(ComponentID_v<Position2D>)) {
      m_add_component_to_buffer<Position2D>(entity, groups, 2, curr_entity);

      if (!indexed) {
        const int num_vertices =
            static_cast<int>(m_registry.get<Position2D>(entity).m_data.size()) /
            2;

        m_entity_info[group_id].m_num_vertices.push_back(num_vertices);
        m_entity_info[group_id].m_base_vertex.push_back(
            m_curr_pos_vbo_offsets[group_id]);

        m_curr_pos_vbo_offsets[group_id] += num_vertices;
      }
    }

    if (bitmask.test(ComponentID_v<Position3D>)) {
      m_add_component_to_buffer<Position3D>(entity, groups, 3, curr_entity);

      if (!indexed) {
        const int num_vertices =
            static_cast<int>(m_registry.get<Position3D>(entity).m_data.size()) /
            3;

        m_entity_info[group_id].m_num_vertices.push_back(num_vertices);
        m_entity_info[group_id].m_base_vertex.push_back(
            m_curr_pos_vbo_offsets[group_id]);

        m_curr_pos_vbo_offsets[group_id] += num_vertices;
      }
    }

    if (bitmask.test(ComponentID_v<Color>)) {
      m_add_component_to_buffer<Color>(entity, groups, 4, curr_entity);
    }

    // add the uniforms for the entity
    std::vector<uniforms_variant> uniforms;

    if (bitmask.test(ComponentID_v<SolidColor>)) {
      uniforms.push_back(m_registry.get<SolidColor>(entity));
    }

    if (bitmask.test(ComponentID_v<Transform>)) {
      uniforms.push_back(m_registry.get<Transform>(entity));
    }

    m_entity_info[group_id].m_uniforms.push_back(uniforms);

    ++curr_entities[group_id];
  });

  // transfer the data to the gl buffers
  for (size_t i = 0; i < m_vbo_data.size(); ++i) {
    for (size_t j = 0; j < m_vbo_data[i].size(); ++j) {
      // add vbo storage
      glNamedBufferStorage(m_vbo_ids[i][j], m_vbo_data_sizes[i][j],
                           m_vbo_data[i][j].data(), 0);

      // set the vertex buffer for the vao
      glVertexArrayVertexBuffer(vao_ids[i], j, m_vbo_ids[i][j], 0,
                                groups.m_groups[i].m_strides[j]);
      ;

      // do the following only for indexed groups
      if (groups.m_groups[i].m_indexed) {
        // add ebo storage
        glNamedBufferStorage(m_ebo_ids[i], m_ebo_data_sizes[i],
                             m_ebo_data[i].data(), 0);

        // set the element buffer for the vao
        glVertexArrayElementBuffer(vao_ids[i], m_ebo_ids[i]);
      }
    }
  }

  // return the number of vertices for each group and each vbo
  return m_entity_info;
}

template <typename T>
void Buffers::m_add_component_to_buffer(const entt::entity entity,
                                        const Groups &groups,
                                        const size_t num_values_per_row,
                                        const size_t curr_entity) {
  // get the data of the component
  using data_type = typename decltype(T::m_data)::value_type;

  // get the group id, buffer binding, vbo stride, data, and destination
  // buffer
  const size_t group_id = m_registry.get<GroupID>(entity).m_id;
  constexpr size_t buffer_binding = BufferBinding_v<T>;
  const size_t vbo_stride =
      groups.m_groups[group_id].m_strides[buffer_binding] / sizeof(data_type);
  const auto &data = m_registry.get<T>(entity).m_data;
  auto &dest = m_vbo_data[group_id][buffer_binding];

  // this will run for entities that have not been seen yet
  if (m_added_entities[group_id][buffer_binding].find(curr_entity) ==
      m_added_entities[group_id][buffer_binding].end()) {

    // update the entity base vertex
    m_curr_entity_base_vertex[group_id][buffer_binding].resize(curr_entity + 2);
    if (curr_entity != 0)
      m_curr_entity_base_vertex[group_id][buffer_binding][curr_entity] =
          m_curr_entity_base_vertex[group_id][buffer_binding][curr_entity - 1] +
          m_num_rows[group_id][buffer_binding][curr_entity - 1] * vbo_stride;

    // add the number of rows for the current entity
    size_t entity_num_rows = data.size() / num_values_per_row;

    m_num_rows[group_id][buffer_binding].push_back(entity_num_rows);

    m_curr_component_offsets[group_id][buffer_binding].push_back(0);

    // insert the entity into the added entities set
    m_added_entities[group_id][buffer_binding].insert(curr_entity);

    size_t entity_size = entity_num_rows * vbo_stride;

    // resize the data vector
    if (m_vbo_data[group_id][buffer_binding].empty())
      m_vbo_data[group_id][buffer_binding].resize(entity_size);
    else
      m_vbo_data[group_id][buffer_binding].resize(
          m_vbo_data[group_id][buffer_binding].size() + entity_size);

    // update the base vertex of the next entity
    m_curr_entity_base_vertex[group_id][buffer_binding][curr_entity + 1] =
        m_curr_entity_base_vertex[group_id][buffer_binding][curr_entity] +
        entity_num_rows * vbo_stride;
  }

  size_t num_rows = m_num_rows[group_id][buffer_binding][curr_entity];

  // copy the data into the destination buffer at the correct offset for each
  // row
  for (size_t i = 0; i < num_rows; ++i) {
    auto src_offset = data.begin() + (num_values_per_row * i);

    std::copy(
        src_offset, src_offset + num_values_per_row,
        dest.begin() +
            m_curr_component_offsets[group_id][buffer_binding][curr_entity] +
            m_curr_entity_base_vertex[group_id][buffer_binding][curr_entity] +
            (vbo_stride * i));
  }

  // update the offset of the current component
  m_curr_component_offsets[group_id][buffer_binding][curr_entity] +=
      num_values_per_row;

  // update the size of the buffer
  m_vbo_data_sizes[group_id][BufferBinding_v<T>] =
      dest.size() * sizeof(data_type);
}

void Buffers::m_delete_buffers() {
  for (size_t i = 0; i < m_vbo_ids.size(); ++i) {
    glDeleteBuffers(m_vbo_ids[i].size(), m_vbo_ids[i].data());
  }
}
