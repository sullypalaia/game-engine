module;

export module Concepts;

/**
 * \brief checks if the given component's data field is a vector
*/
export template <typename T>
concept component_with_vector_data = requires(T x) { x.m_data.data(); };
