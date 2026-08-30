module;

#include <variant>
#include <vector>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

export module Buffers;

import Groups;
import Components;

export class Buffers {
public:
  Buffers(entt::registry &registry);

  void m_create_buffers(const Groups &groups);

  void m_delete_buffers();

private:
  entt::registry &m_registry;

  std::vector<std::vector<GLuint>> m_vbo_ids;
  std::vector<std::vector<std::vector<std::variant<GLfloat>>>> m_data;
  std::vector<std::vector<size_t>> m_data_sizes;

  void m_create_vbos(const Groups &groups);

  void m_delete_vbos();

  template <typename T>
  void m_add_component_to_buffer(const entt::entity entity);
};

Buffers::Buffers(entt::registry &registry) : m_registry(registry) {}

void Buffers::m_create_buffers(const Groups &groups) { m_create_vbos(groups); }

/**\brief create the VBOs and insert their data
 */
void Buffers::m_create_vbos(const Groups &groups) {
  m_vbo_ids.resize(groups.m_count);
  m_data_sizes.resize(groups.m_count);
  m_data.resize(groups.m_count);

  // create the buffers
  for (size_t i = 0; i < groups.m_count; ++i) {
    size_t num_vbos = groups.m_num_vbos[i];
    m_vbo_ids[i].resize(num_vbos);
    glCreateBuffers(num_vbos, m_vbo_ids[i].data());
  }

  m_registry.view<entt::entity>().each([&](const entt::entity entity) {
    const size_t group_id = m_registry.get<GroupID>(entity).m_id;
    // set all the sizes to 0
    if (m_data_sizes[group_id].empty()) {
      m_data_sizes[group_id].resize(groups.m_num_vbos[group_id]);
    }

    // check the component mask and add components accordingly
    const auto &bitmask = m_registry.get<ComponentMask>(entity).m_bits;
    if (bitmask.test(ComponentID_v<Position2D>))
      m_add_component_to_buffer<Position2D>(entity);

    if (bitmask.test(ComponentID_v<Position3D>))
      m_add_component_to_buffer<Position3D>(entity);
  });

  // transfer the data to the gl buffers
  for (size_t i = 0; i < m_data.size(); ++i) {
    for (size_t j = 0; j < m_data[i].size(); ++j) {
      glNamedBufferStorage(m_vbo_ids[i][j], m_data_sizes[i][j],
                           m_data[i][j].data(), 0);
    }
  }
}

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
}

void Buffers::m_delete_buffers() { m_delete_vbos(); }

void Buffers::m_delete_vbos() {
  for (size_t i = 0; i < m_vbo_ids.size(); ++i) {
    glDeleteBuffers(m_vbo_ids[i].size(), m_vbo_ids[i].data());
  }
}
