// add proper error handling to this

module;

#include <iostream>
#include <string>
#include <vector>

#include "external/glad/glad.h"

export module Shaders;

/**
 * \brief takes shader strings as input, creates shaders, compiles shaders, checks for errors, returns their ids
*/
export std::vector<GLuint> generate_ids(std::vector<std::string> &vert_shaders,
                                        std::vector<std::string> &frag_shaders,
                                        const size_t num_groups) {

  std::vector<GLuint> ids;

  for (size_t i = 0; i < num_groups; ++i) {
    GLuint program = glCreateProgram();

    GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *vert_cstr = vert_shaders[i].c_str();
    glShaderSource(vert_shader, 1, &vert_cstr, NULL);
    glCompileShader(vert_shader);

    GLint vert_comp_status;
    glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_comp_status);
    if (!vert_comp_status) {
      std::cerr << "vertex shader " << i << " compilation failed\n";

      GLint vert_log_length;
      glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &vert_log_length);
      GLchar *vert_log =
          static_cast<GLchar *>(malloc(vert_log_length * sizeof(GLchar)));
      glGetShaderInfoLog(vert_shader, vert_log_length, NULL, vert_log);

      std::cerr << vert_log;
    }

    glAttachShader(program, vert_shader);
    glDeleteShader(vert_shader);

    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *frag_cstr = frag_shaders[i].c_str();
    glShaderSource(frag_shader, 1, &frag_cstr, NULL);
    glCompileShader(frag_shader);

    GLint frag_comp_status;
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_comp_status);
    if (!frag_comp_status) {
      std::cerr << "fragment shader " << i << " compilation failed\n";

      GLint frag_log_length;
      glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &frag_log_length);
      GLchar *frag_log =
          static_cast<GLchar *>(malloc(frag_log_length * sizeof(GLchar)));
      glGetShaderInfoLog(frag_shader, frag_log_length, NULL, frag_log);

      std::cerr << frag_log;
    }

    glAttachShader(program, frag_shader);
    glDeleteShader(frag_shader);

    glLinkProgram(program);

    GLint link_status;
    glGetProgramiv(program, GL_LINK_STATUS, &link_status);
    if (!link_status) {
      std::cerr << "failed to link shader >> getting program log...\n";

      GLint program_log_length;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &program_log_length);
      GLchar *program_log =
          (GLchar *)malloc(program_log_length * sizeof(GLchar));
      glGetProgramInfoLog(program, program_log_length, NULL, program_log);

      std::cerr << program_log;
    }

    ids.push_back(program);
  }

  return ids;
}

/**
 * \brief describes per-vertex attribute for a vao and binds it to specified binding index
*/
export void set_attrib(GLuint vao, GLuint attrib_index, GLuint binding_index,
                       GLint size, GLenum type, GLboolean normalized,
                       GLuint relative_offset, GLbitfield flags) {
  glEnableVertexArrayAttrib(vao, attrib_index);
  glVertexArrayAttribFormat(vao, attrib_index, size, type, normalized,
                            relative_offset);
  glVertexArrayAttribBinding(vao, attrib_index, binding_index);
}
