module;

#include "entt/entt.hpp"
#include "external/glad/glad.h"

#include "external/glm/glm.hpp"
#include "external/glm/gtc/type_ptr.hpp"

export module ECS;

import Meshes;
import Materials;
import Components;
import Concepts;
import Shaders;

export class ECS {
public:
  ECS() = default;

  ECS(ECS &&other) = delete;
  ECS &operator=(ECS &&other) = delete;

  ~ECS();

  // add entity with components
  template <typename T, typename... Args>
  const entt::entity m_add_entity_with_component(Args... args);

  // add component to existing entity
  template <typename T, typename... Args>
  void m_add_component_to_entity(entt::entity entity, Args... args);

  // update component of existing entity
  template <typename T, typename... Args>
  void m_update_entity_component(entt::entity entity, Args... args);

  // update entities
  void m_update();

  entt::registry &m_get_registry();

private:
  entt::registry m_reg;

  // update materials
  void m_update_materials(entt::entity entity);

  // update transforms
  void m_update_transforms(entt::entity entity);

  // initialization
  void m_init();

  // draw
  void m_draw();
};

void ECS::m_update() {}

// add entity with component
template <typename T, typename... Args>
const entt::entity ECS::m_add_entity_with_component(Args... args) {
  const entt::entity entity = m_reg.create();
  m_reg.emplace<T>(entity, args...);
  return entity;
}

// add component to existing entity
template <typename T, typename... Args>
void ECS::m_add_component_to_entity(entt::entity entity, Args... args) {
  m_reg.emplace<T>(entity, args...);
}

// update component of existing entity
template <typename T, typename... Args>
void ECS::m_update_entity_component(entt::entity entity, Args... args) {
  m_reg.replace<T>(entity, args...);
}

entt::registry &ECS::m_get_registry() { return m_reg; }

// destroy all entities
ECS::~ECS() { m_reg.clear(); }
