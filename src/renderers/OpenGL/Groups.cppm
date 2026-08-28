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
};

export struct Groups {
  size_t m_count;
  std::vector<size_t> m_num_vbos;
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
  size_t curr_id = 0;

  bool found_group;
  registry.view<entt::entity>().each([&](const entt::entity entity) {
    found_group = false;
    for (auto &group : groups) {
      if (group.m_components == registry.get<ComponentMask>(entity).m_bits) {
        registry.emplace<GroupID>(entity, group.m_id);
        found_group = true;
        break;
      }
    }
    if (!found_group) {
      // update the number of vbos based on the categories of the per-veretx
      // components
      size_t num_vbos = 0;

      ComponentMask &mask = registry.get<ComponentMask>(entity);
      for (int i : buffer_cat_lens) {
        for (int j = 0; j < i; ++j) {
          if ((1 << j) & mask.m_bits != 0) {
            ++num_vbos;
            break;
          }
        }
      }

      num_vbos_v.push_back(num_vbos);

      groups.push_back({curr_id, num_vbos, mask.m_bits});
      registry.emplace<GroupID>(entity, curr_id);

      ++curr_id;
    }
  });

  return Groups{curr_id, num_vbos_v};
}
