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
  Buffers(const Groups &groups, entt::registry &registry);

  void m_create_vbos();

private:
  const Groups &m_groups;
  entt::registry &m_registry;

  std::vector<std::vector<GLuint>> m_vbo_ids;
  std::vector<std::vector<std::vector<std::variant<GLfloat>>>> m_data;
  std::vector<std::vector<size_t>> m_data_sizes;

  template <typename T>
  void m_add_component_to_buffer(const entt::entity entity);
};

Buffers::Buffers(const Groups &groups, entt::registry &registry)
    : m_groups(groups), m_vbo_ids(groups.m_count), m_registry(registry),
      m_data_sizes(groups.m_count), m_data(groups.m_count) {}

/**\brief create the VBOs and insert their data
 */
void Buffers::m_create_vbos() {
  // create the buffers
  for (size_t i = 0; i < m_groups.m_count; ++i) {
    size_t num_vbos = m_groups.m_num_vbos[i];
    m_vbo_ids[i].resize(num_vbos);
    glCreateBuffers(num_vbos, m_vbo_ids[i].data());
  }

  // set the sizes of the buffers to 0 (if they are currently empty) and add the
  // data to its group and vbo binding
  m_registry.view<entt::entity>().each([&](const entt::entity entity) {
    const size_t group_id = m_registry.get<GroupID>(entity).m_id;
    if (m_data_sizes[group_id].empty()) {
      m_data_sizes[group_id].resize(m_groups.m_num_vbos[group_id]);
    }

    // check the component mask and add components accordingly
    const auto &bitmask = m_registry.get<ComponentMask>(entity).m_bits;
    if (bitmask.test(ComponentID_v<Position2D>))
      m_add_component_to_buffer<Position2D>(entity);

    if (bitmask.test(ComponentID_v<Position3D>))
      m_add_component_to_buffer<Position3D>(entity);
  });
}

template <typename T>
void Buffers::m_add_component_to_buffer(const entt::entity entity) {
  std::vector<typename decltype(T::m_data)::value_type> vbo_data =
      m_registry.get<T>(entity).m_data;

  m_data[m_registry.get<GroupID>(entity)][BufferBinding_v<T>].push_back(
      vbo_data);
}
