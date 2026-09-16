module;

#include <bitset>
#include <variant>
#include <vector>

#include <glm/gtc/matrix_transform.hpp>

export module engine.ecs.components;

// forward declarations

// static attributes
export struct Position2D;
export struct Position3D;
export struct Color;

// uniforms
export struct Transform;
export struct SolidColor;

// indices
export struct Indices;

// uniforms variant type for storing the uniforms of an entity in a group
export using uniforms_variant = std::variant<std::reference_wrapper<SolidColor>,
                                             std::reference_wrapper<Transform>>;

export enum class ComponentTypes {
  // per-vertex attributes
  POSITION_2D,
  POSITION_3D,
  Color,
  // ^buff cat 1 - static

  // the rest of these should not be per-vertex attributes:
  TRANSFORM, // note that TRANSFORM can also be used as the size of the
             // per-vertex attribute list
  SOLID_COLOR,

  // entities need to provide indices for indexed drawing
  INDICES,

  BIT_COUNT
};

// for buffers that update at different frequencies
export int buffer_cat_lens[]{3};

/**\brief template specializations of this take components and return their
 * value in the ComponentTypes enum
 */
export template <typename T> struct ComponentID;

// template specializations of ComponentID
template <> struct ComponentID<Position2D> {
  static constexpr int value = static_cast<int>(ComponentTypes::POSITION_2D);
};

template <> struct ComponentID<Position3D> {
  static constexpr int value = static_cast<int>(ComponentTypes::POSITION_3D);
};

template <> struct ComponentID<Color> {
  static constexpr int value = static_cast<int>(ComponentTypes::Color);
};

template <> struct ComponentID<Transform> {
  static constexpr int value = static_cast<int>(ComponentTypes::TRANSFORM);
};

template <> struct ComponentID<SolidColor> {
  static constexpr int value = static_cast<int>(ComponentTypes::SOLID_COLOR);
};

template <> struct ComponentID<Indices> {
  static constexpr int value = static_cast<int>(ComponentTypes::INDICES);
};

export template <typename T>
constexpr int ComponentID_v = ComponentID<T>::value;

/**\brief utility to return the size (in bytes) of a component
 */
export template <typename T> struct ComponentSize;

template <> struct ComponentSize<Position2D> {
  static constexpr int value = 2 * sizeof(float);
};

template <> struct ComponentSize<Position3D> {
  static constexpr int value = 3 * sizeof(float);
};

template <> struct ComponentSize<Color> {
  static constexpr int value = 4 * sizeof(float);
};

export template <typename T>
constexpr int ComponentSize_v = ComponentSize<T>::value;

export enum class BufferBindings { BUFFER0, BUFFER1 };

/**\brief returns the buffer binding that the per-vertex attribute should be
 * attached to
 *
 * This ensures that attributes are only updated when they need to be updated.
 * Static positions may need to be set once, whereas changing colors may need to
 * be updated for every draw call
 */
export template <typename T> struct BufferBinding;

template <> struct BufferBinding<Position2D> {
  static constexpr size_t value = static_cast<size_t>(BufferBindings::BUFFER0);
};

template <> struct BufferBinding<Position3D> {
  static constexpr size_t value = static_cast<size_t>(BufferBindings::BUFFER0);
};

template <> struct BufferBinding<Color> {
  static constexpr size_t value = static_cast<size_t>(BufferBindings::BUFFER0);
};

export template <typename T>
constexpr int BufferBinding_v = BufferBinding<T>::value;

/**\brief with a component mask, we can easily create groups by comparing
 * the masks of different components
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
  std::vector<float> m_data;
};

export struct Position3D {
  std::vector<float> m_data;
};

export struct Color {
  std::vector<float> m_data;
};

export struct Transform {
  glm::mat4 m_data;
};

export struct SolidColor {
  std::vector<float> m_data;
};

export struct Indices {
  std::vector<unsigned int> m_data;
};
