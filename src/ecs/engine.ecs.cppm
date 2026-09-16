module;

#include <bitset>

#include "entt/entt.hpp"

export module engine.ecs;

export import engine.ecs.components;

/**
 * \brief ECS that provides functions to create entities with a component or
 * add a component to an existing entity
 *
 * Bitmasks are added or updated for every component added to make jobs,
 * like rendering, more efficient. Any other system of the game engine is
 * kept separate from the ECS to allow for different renderers,physics
 * engines, etc...
 */
export class ECS {
public:
  ECS() = default;

  // ECS should not be moved
  ECS(ECS &&other) = delete;
  ECS &operator=(ECS &&other) = delete;

  ~ECS();

  // add entity with components
  template <typename T, typename... Args>
  const entt::entity m_add_entity_with_component(Args &&...args);

  // add component to existing entity
  template <typename T, typename... Args>
  void m_add_component_to_entity(entt::entity entity, Args &&...args);

  // update component of existing entity
  template <typename T, typename... Args>
  void m_update_entity_component(entt::entity entity, Args &&...args);

  // update entities
  void m_update();

  entt::registry &m_get_registry();

private:
  entt::registry m_reg;
};

void ECS::m_update() {}

// add entity with component
template <typename T, typename... Args>
const entt::entity ECS::m_add_entity_with_component(Args &&...args) {
  const entt::entity entity = m_reg.create();
  m_reg.emplace<T>(entity, std::forward<Args>(args)...);

  std::bitset<static_cast<size_t>(ComponentTypes::BIT_COUNT)> new_bitset{};
  new_bitset.set(ComponentID_v<T>);
  m_reg.emplace<ComponentMask>(entity, new_bitset);

  return entity;
}

// add component to existing entity
template <typename T, typename... Args>
void ECS::m_add_component_to_entity(entt::entity entity, Args &&...args) {
  m_reg.emplace<T>(entity, std::forward<Args>(args)...);

  ComponentMask &bitmask = m_reg.get<ComponentMask>(entity);
  bitmask.m_bits.set(ComponentID_v<T>);
}

// update component of existing entity
template <typename T, typename... Args>
void ECS::m_update_entity_component(entt::entity entity, Args &&...args) {
  m_reg.replace<T>(entity, std::forward<Args>(args)...);

  ComponentMask &bitmask = m_reg.get<ComponentMask>(entity);
  bitmask.m_bits.set(ComponentID_v<T>);
}

entt::registry &ECS::m_get_registry() { return m_reg; }

// destroy all entities
ECS::~ECS() { m_reg.clear(); }
