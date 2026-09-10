module;

#include <bitset>
#include <vector>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

export module Groups;

import Components;

export struct Group {
  size_t m_id;
  size_t m_num_vbos;
  std::bitset<static_cast<size_t>(ComponentTypes::BIT_COUNT)> m_components;
  size_t m_num_entities;
  std::vector<size_t> m_strides;
  bool m_indexed;

  // constructor because its not an aggregate anymore
  Group(size_t id, size_t num_vbos,
        std::bitset<static_cast<size_t>(ComponentTypes::BIT_COUNT)> components,
        std::vector<size_t> strides, bool indexed)
      : m_id(id), m_num_vbos(num_vbos), m_components(components),
        m_num_entities(1), m_strides(strides), m_indexed(indexed) {}

  Group(const Group &other) = default;
  Group &operator=(const Group &other) = default;

  // need to make these noexcept so that they can be moved with vectors
  Group(Group &&other) noexcept = default;
  Group &operator=(Group &&other) noexcept = default;

  ~Group() = default;
};

export struct Groups {
  size_t m_count;
  std::vector<Group> m_groups;
};

/**\brief matches every entity to a corresponding group based on its component
 * mask and updates its group id component
 *
 * After matching entities to a group, this returns a struct containing the
 * number of groups and the number of vbos in each group
 */
export Groups create_groups(entt::registry &registry) {
  // create the groups
  std::vector<Group> groups;
  std::vector<size_t> num_vbos_v;
  std::vector<std::bitset<static_cast<size_t>(ComponentTypes::BIT_COUNT)>>
      components;
  size_t curr_id = 0;
  size_t curr_vertex_offset = 0;

  bool found_group;
  registry.view<entt::entity>().each([&](const entt::entity entity) {
    found_group = false;
    // check if the entity's component mask matches any existing group
    for (auto &group : groups) {
      if (group.m_components == registry.get<ComponentMask>(entity).m_bits) {
        // add the entity to the group
        registry.emplace<GroupID>(entity, group.m_id);
        found_group = true;
        break;
      }
    }
    // create a new group if the entity's component mask doesn't match any
    // existing group
    if (!found_group) {
      size_t num_vbos = 0;

      // update the number of vbos for this group
      ComponentMask &mask = registry.get<ComponentMask>(entity);
      for (int i : buffer_cat_lens) {
        for (int j = 0; j < i; ++j) {
          if ((1 << j) & mask.m_bits != 0) {
            ++num_vbos;
            break;
          }
        }
      }

      // calculate the strides for the buffers in terms of the number of
      // elements of the component type
      std::vector<size_t> strides(num_vbos);
      for (size_t i = 0; i < static_cast<size_t>(ComponentTypes::TRANSFORM);
           ++i) {
        if (mask.m_bits.test(i)) {
          switch (i) {
          case ComponentID_v<Position2D>:
            strides[BufferBinding_v<Position2D>] += ComponentSize_v<Position2D>;
            break;
          case ComponentID_v<Position3D>:
            strides[BufferBinding_v<Position3D>] += ComponentSize_v<Position3D>;
            break;
          case ComponentID_v<Color>:
            strides[BufferBinding_v<Color>] += ComponentSize_v<Color>;
            break;
          }
        }
      }

      num_vbos_v.push_back(num_vbos);
      components.push_back(mask.m_bits);

      // update the groups vector
      groups.push_back({curr_id, num_vbos, mask.m_bits, std::move(strides),
                        registry.all_of<Indices>(entity)});
      registry.emplace<GroupID>(entity, curr_id);

      ++curr_id;
    }
  });

  return Groups{curr_id, std::move(groups)};
}
