module;

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <type_traits>
#include <unordered_set>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

#include "macros.h"

export module ShaderGen;

import Components;
import Shaders;
import Concepts;

/**\brief class encapsulating all shader generation functionality
*/
export class ShaderGen {
public:
  ShaderGen(entt::registry &registry, const size_t num_groups,
            std::vector<GLuint> &vaos);

  std::vector<GLuint> m_gen_shaders();

private:
  template <typename T>
  /**\brief adds a vertex attribute with the specified type, name, size
   *
   * This function also avoids creating duplicates of the same attribute declaration in case of multiple entities in the same group
  */
  void m_add_attr(const char *type, const GLenum gl_type,
                  std::string_view desired_name, const GLint size,
                  const char *pos_ts = nullptr);

  /**\brief increases the relative offset of the given attribute in a group, given the attribute's data is a vector
   */
  template <typename T>
    requires component_with_vector_data<T>
  void m_incr_relative_offset(const size_t group_id, const GLint size);

  /**\brief increases the relative offset of the given attribute in a group, given the attribute's data is NOT a vector
  */
  template <typename T>
  void m_incr_relative_offset(const size_t group_id, const GLint size);

  /**\brief adds a uniform variable to a vertex shader, ensuring that no duplicate declarations are made
  */
  template <typename T>
  void m_add_vert_uniform(const char *type, std::string_view desired_name);

  /**\brief adds a uniform variable to a fragment shader, ensuring that no duplicate declarations are made
  */ 
  template <typename T>
  void m_add_frag_uniform(const char *type, std::string_view desired_name);

  entt::registry &m_registry;

  size_t m_num_groups;

  std::vector<GLuint> m_curr_uniform_locs;
  std::vector<GLuint> m_curr_attrib_locs;
  std::vector<std::string> m_vert_shaders;
  std::vector<std::string> m_frag_shaders;
  std::vector<std::string> m_pos_strs;
  std::vector<std::string> m_pos_ts;
  std::vector<std::string> m_color_strs;
  std::vector<GLuint> m_relative_offsets;
  std::unordered_set<size_t> m_group_ids;

  std::vector<GLuint> &m_vaos;
};

ShaderGen::ShaderGen(entt::registry &registry, const size_t num_groups,
                     std::vector<GLuint> &vaos)
    : m_registry(registry), m_num_groups(num_groups),
      m_curr_uniform_locs(num_groups), m_vaos(vaos),
      m_curr_attrib_locs(m_num_groups),
      m_vert_shaders(m_num_groups, GLSL_VERSION_STR),
      m_frag_shaders(m_num_groups, GLSL_VERSION_STR),
      m_pos_strs(m_num_groups, "\tgl_Position = "), m_pos_ts(m_num_groups),
      m_color_strs(m_num_groups), m_relative_offsets(m_num_groups, 0) {}

std::vector<GLuint> ShaderGen::m_gen_shaders() {
  // in variables

  m_add_attr<Position2D>("vec2", GL_FLOAT, "pos", 2,
                         "vec4(pos, 0.0f, 1.0f);\n");

  m_add_attr<Position3D>("vec3", GL_FLOAT, "pos", 3, "vec4(pos, 1.0f);\n");

  for (std::string &vert_shader : m_vert_shaders)
    vert_shader += '\n';

  // uniform variables

  m_add_vert_uniform<Transform>("mat4", "model");

  std::filesystem::create_directory("shaders");

  for (size_t i = 0; i < m_num_groups; ++i) {
    // mats
    m_vert_shaders[i] += "layout (std140, binding = 0) uniform Mats {\n\tmat4 "
                         "view;\n\tmat4 proj;\n};\n\n";

    m_vert_shaders[i] += "void main() {\n";

    // main body

    // add the position line and end
    m_pos_strs[i] += m_pos_ts[i];
    m_vert_shaders[i] += m_pos_strs[i];
    m_vert_shaders[i] += "}\n";

    std::string file = std::format("shaders/{}.vert", i);

    std::ofstream fs(file);
    if (!fs)
      std::cerr << std::format("failed to open file {}\n", file);

    fs << m_vert_shaders[i];

    fs.close();
  }

  for (std::string &frag_shader : m_frag_shaders) {
    frag_shader += "out vec4 frag_color;\n\n";
  }

  // uniform variables

  m_add_frag_uniform<SolidColor>("vec4", "color");

  for (size_t i = 0; i < m_num_groups; ++i) {
    m_frag_shaders[i] +=
        "\nvoid main() {\n\tfrag_color = " + m_color_strs[i] + "\n}\n";

    std::filesystem::create_directory("shaders");
    std::string file = std::format("shaders/{}.frag", i);

    std::ofstream fs(file);
    if (!fs)
      std::cerr << std::format("failed to open file {}\n", file);

    fs << m_frag_shaders[i];

    fs.close();
  }

  return generate_ids(m_vert_shaders, m_frag_shaders, m_num_groups);
}

template <typename T>
void ShaderGen::m_add_attr(const char *type, const GLenum gl_type,
                           std::string_view desired_name, const GLint size,
                           const char *pos_ts) {
  auto view = m_registry.view<T>();
  for (auto entity : view) {
    const size_t group_id = m_registry.get<GroupID>(entity).m_id;

    if (m_group_ids.find(group_id) != m_group_ids.end())
      continue;

    m_group_ids.insert(group_id);

    m_vert_shaders[group_id] +=
        std::format("layout (location = {}) in {} {};\n",
                    m_curr_attrib_locs[group_id], type, desired_name);

    if (pos_ts)
      m_pos_ts[group_id] = pos_ts; //"vec4(pos, 0.0f, 1.0f);\n";

    // binding index 0 - vertex attributes change every frame
    set_attrib(m_vaos[group_id], m_curr_attrib_locs[group_id],
               BufferBinding_v<T>, size, gl_type, GL_FALSE,
               m_relative_offsets[group_id], 0);
    m_incr_relative_offset<T>(group_id, size);

    ++m_curr_attrib_locs[group_id];
  }

  m_group_ids.clear();
}

template <typename T>
void ShaderGen::m_add_vert_uniform(const char *type,
                                   std::string_view desired_name) {
  auto view = m_registry.view<T>();
  for (auto entity : view) {
    const size_t group_id = m_registry.get<GroupID>(entity).m_id;

    if (m_group_ids.find(group_id) != m_group_ids.end())
      continue;

    m_group_ids.insert(group_id);

    m_vert_shaders[group_id] +=
        std::format("layout (location = {}) uniform {} {};\n",
                    m_curr_uniform_locs[group_id], type, desired_name);

    if constexpr (std::is_same_v<T, Transform>)
      m_pos_strs[group_id] += std::format("{} * ", desired_name);

    ++m_curr_uniform_locs[group_id];
  }

  m_group_ids.clear();
}

template <typename T>
void ShaderGen::m_add_frag_uniform(const char *type,
                                   std::string_view desired_name) {
  auto view = m_registry.view<T>();
  for (auto entity : view) {
    const size_t group_id = m_registry.get<GroupID>(entity).m_id;

    if (m_group_ids.find(group_id) != m_group_ids.end())
      continue;

    m_group_ids.insert(group_id);

    m_frag_shaders[group_id] +=
        std::format("layout (location = {}) uniform {} {};\n",
                    m_curr_uniform_locs[group_id], type, desired_name);

    if constexpr (std::is_same_v<T, SolidColor>)
      m_color_strs[group_id] += std::format("{};", desired_name);

    ++m_curr_uniform_locs[group_id];
  }

  m_group_ids.clear();
}

template <typename T>
  requires component_with_vector_data<T>
void ShaderGen::m_incr_relative_offset(const size_t group_id,
                                       const GLint size) {
  m_relative_offsets[group_id] +=
      size * sizeof(typename decltype(T::m_data)::value_type);
}

template <typename T>
void ShaderGen::m_incr_relative_offset(const size_t group_id,
                                       const GLint size) {
  m_relative_offsets[group_id] += size * sizeof(decltype(T::m_data));
}
