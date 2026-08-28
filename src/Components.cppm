module;

#include <bitset>
#include <optional>
#include <vector>

#include "external/glad/glad.h"
#include "external/glm/glm.hpp"

#include "entt/entt.hpp"

export module Components;

import Shaders;

// forward declarations
export struct Position2D;
export struct Position3D;
export struct Transform;
export struct SolidColor;

export enum class ComponentTypes {
  //per-vertex attributes
  POSITION_2D,
  POSITION_3D, // buff cat 1

  //the rest of these should not be per-vertex attributes
  TRANSFORM,
  SOLID_COLOR,
  BIT_COUNT
};

// for buffers that update at different frequencies
export int buffer_cat_lens[]{2};

/**\brief template specializations of this take components and return their value in the ComponentTypes enum
*/
export template <typename T> struct ComponentID;

// template specializations of ComponentID
template <> struct ComponentID<Position2D> {
  static constexpr int value = static_cast<int>(ComponentTypes::POSITION_2D);
};

template <> struct ComponentID<Position3D> {
  static constexpr int value = static_cast<int>(ComponentTypes::POSITION_3D);
};

template <> struct ComponentID<Transform> {
  static constexpr int value = static_cast<int>(ComponentTypes::TRANSFORM);
};

template <> struct ComponentID<SolidColor> {
  static constexpr int value = static_cast<int>(ComponentTypes::SOLID_COLOR);
};

export template <typename T>
constexpr int ComponentID_v = ComponentID<T>::value;

export enum class BufferBindings { BUFFER0, BUFFER1 };

/**\brief returns the buffer binding that the per-vertex attribute should be attached to
 *
 * This ensures that attributes are only updated when they need to be updated.
 * Static positions may need to be set once, whereas changing colors may need to be updated for every draw call
*/
export template <typename T> struct BufferBinding;

template <> struct BufferBinding<Position2D> {
  static constexpr size_t value = static_cast<int>(BufferBindings::BUFFER0);
};

template <> struct BufferBinding<Position3D> {
  static constexpr size_t value = static_cast<int>(BufferBindings::BUFFER0);
};

export template <typename T>
constexpr int BufferBinding_v = BufferBinding<T>::value;

/**\brief with a component mask, we can easily create groups by comparing the masks of different components
*/
export struct ComponentMask {
  std::bitset<static_cast<size_t>(ComponentTypes::BIT_COUNT)> m_bits;
};

/**\brief each entity belongs to a group with the same vao and shader program
*/
export struct GroupID {
  size_t m_id;
};

//-------------user components---------------

export struct Position2D {
  std::vector<GLfloat> m_data;
};

export struct Position3D {
  std::vector<GLfloat> m_data;
};

export struct Transform {
  const glm::mat4 m_data;
};

export struct SolidColor {
  GLfloat m_data[4];
};

//----------------opengl interface--------------

namespace OpenGL {
export struct Mesh {
  GLuint m_vao;
  GLuint m_program;
  std::vector<GLuint> m_vbos;
  std::optional<GLuint> m_ebo;
};

} // namespace OpenGL
