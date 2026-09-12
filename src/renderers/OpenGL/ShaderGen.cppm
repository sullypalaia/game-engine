module;

#include <bitset>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

#include "external/glad/glad.h"

#include "entt/entt.hpp"

#include "macros.h"

export module ShaderGen;

import Components;
import Shaders;
import Concepts;
import Groups;

/**\brief class encapsulating all shader generation functionality
 */
export class ShaderGen {
public:
  ShaderGen(entt::registry &registry, const Groups &groups,
            std::vector<GLuint> &vaos);

  std::vector<GLuint> m_gen_shaders();

private:
  entt::registry &m_registry;
  const Groups &m_groups;
  std::vector<GLuint> &m_vaos;

  std::vector<std::string> m_vert_shaders;
  std::vector<std::string> m_frag_shaders;
};

ShaderGen::ShaderGen(entt::registry &registry, const Groups &groups,
                     std::vector<GLuint> &vaos)
    : m_registry(registry), m_groups(groups), m_vaos(vaos),
      m_vert_shaders(m_groups.m_count), m_frag_shaders(m_groups.m_count) {}

std::vector<GLuint> ShaderGen::m_gen_shaders() {
  // iterate through the groups and generate the shader code for each group
  for (size_t i = 0; i < m_groups.m_count; ++i) {
    GLuint curr_attrib_loc = 0;
    GLuint curr_uniform_loc = 0;
    std::string vert_shader = GLSL_VERSION_STR;
    std::string frag_shader = GLSL_VERSION_STR;
    std::string pos_str = "\tgl_Position = ";
    std::string pos_ts;
    std::string color_str;
    GLuint relative_offset = 0;

    const Group &group = m_groups.m_groups[i];
    const std::bitset<static_cast<size_t>(ComponentTypes::BIT_COUNT)> &bitmask =
        group.m_components;

    // add the view and proj matrices to vertex shader
    pos_ts += "proj * view * ";

    //-------------------uniforms-------------------

    if (bitmask.test(ComponentID_v<SolidColor>)) {
      frag_shader += std::format("layout (location = {}) uniform vec4 color;\n",
                                 curr_uniform_loc);
      color_str = "color;";
      ++curr_uniform_loc;
    }

    if (bitmask.test(ComponentID_v<Transform>)) {
      vert_shader += std::format("layout (location = {}) uniform mat4 model;\n",
                                 curr_uniform_loc);
      pos_ts += "model * ";
      ++curr_uniform_loc;
    }

    //-------------------vertex attributes-------------------

    if (bitmask.test(ComponentID_v<Position2D>)) {
      vert_shader +=
          std::format("layout (location = {}) in vec2 pos;\n", curr_attrib_loc);
      pos_ts += "vec4(pos, 0.0f, 1.0f);\n";
      set_attrib(m_vaos[group.m_id], curr_attrib_loc,
                 BufferBinding_v<Position2D>, 2, GL_FLOAT, GL_FALSE,
                 relative_offset, 0);
      ++curr_attrib_loc;
      relative_offset += sizeof(float) * 2;
    }

    if (bitmask.test(ComponentID_v<Position3D>)) {
      vert_shader +=
          std::format("layout (location = {}) in vec3 pos;\n", curr_attrib_loc);
      pos_ts += "vec4(pos, 1.0f);\n";
      set_attrib(m_vaos[group.m_id], curr_attrib_loc,
                 BufferBinding_v<Position3D>, 3, GL_FLOAT, GL_FALSE,
                 relative_offset, 0);
      ++curr_attrib_loc;
      relative_offset += sizeof(float) * 3;
    }

    if (bitmask.test(ComponentID_v<Color>)) {
      vert_shader += std::format("layout (location = {}) in vec4 color;\n",
                                 curr_attrib_loc);
      vert_shader += std::format("out vec4 color_out;\n");

      frag_shader += std::format("in vec4 color_out;\n", curr_attrib_loc);
      color_str = "color_out;";
      set_attrib(m_vaos[group.m_id], curr_attrib_loc, BufferBinding_v<Color>, 4,
                 GL_FLOAT, GL_FALSE, relative_offset, 0);
      ++curr_attrib_loc;
      relative_offset += sizeof(float) * 4;
    }

    // add the mats uniform block to the vertex shader
    vert_shader += "layout (std140, binding = 0) uniform Mats {\n\tmat4 "
                   "view;\n\tmat4 proj;\n};\n\n";

    vert_shader += "void main() {\n";

    // set the color in the vertex shader if it is a vertex attribute
    if (bitmask.test(ComponentID_v<Color>)) {
      vert_shader += "\tcolor_out = color;\n";
    }

    // add the position line and end
    pos_str += pos_ts;
    vert_shader += pos_str;
    vert_shader += "}\n";

    // set the color in the vertex shader
    frag_shader +=
        "\nout vec4 frag_color;\n\nvoid main() {\n\tfrag_color = " + color_str +
        "\n}\n";

    m_vert_shaders[i] = vert_shader;
    m_frag_shaders[i] = frag_shader;
  }

  // create the shaders directory if it doesn't exist
  std::filesystem::create_directory("shaders");

  // create the files and upload the shader code to them
  for (size_t i = 0; i < m_groups.m_count; ++i) {
    std::string vert_file = std::format("shaders/{}.vert", i);

    std::ofstream vert_fs(vert_file);
    if (!vert_fs)
      std::cerr << std::format("failed to open file {}\n", vert_file);

    vert_fs << m_vert_shaders[i];

    vert_fs.close();

    std::string frag_file = std::format("shaders/{}.frag", i);

    std::ofstream frag_fs(frag_file);
    if (!frag_fs)
      std::cerr << std::format("failed to open file {}\n", frag_file);

    frag_fs << m_frag_shaders[i];

    frag_fs.close();
  }

  return generate_ids(m_vert_shaders, m_frag_shaders, m_groups.m_count);
}
