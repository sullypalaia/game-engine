module;

#include <concepts>
#include <fstream>
#include <iostream>
#include <string>

#include "external/glad/glad.h"
#include "macros.h"

export module Shaders;

export GLuint generate_id(std::string *vertf, std::string *fragf = nullptr) {
  GLuint program = glCreateProgram();

  // compile vertex shader from string
  std::ifstream vert_stream(*vertf);
  if (!vert_stream) {
    std::cout << "failed to load vertex shader\n";
    return ERROR_ID;
  }
  const std::string vert_s{std::istreambuf_iterator<char>(vert_stream),
                           std::istreambuf_iterator<char>()};

  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  const GLchar *vert_cstr = vert_s.c_str();
  glShaderSource(vert_shader, 1, &vert_cstr, NULL);
  glCompileShader(vert_shader);

  GLint vert_comp_status;
  glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_comp_status);
  if (!vert_comp_status) {
    std::cerr << "failed to compile vertex shader" << *vertf
              << "\n>> getting shader log...\n";

    GLint vert_log_length;
    glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &vert_log_length);
    GLchar *vert_log =
        static_cast<GLchar *>(malloc(vert_log_length * sizeof(GLchar)));
    glGetShaderInfoLog(vert_shader, vert_log_length, NULL, vert_log);

    std::cerr << vert_log;
    return ERROR_ID;
  }

  glAttachShader(program, vert_shader);
  glDeleteShader(vert_shader);

  // compile fragment shader from string
  if (fragf) {
    std::ifstream frag_stream(*fragf);
    if (!frag_stream) {
      std::cout << "failed to load fragment shader\n";
      return ERROR_ID;
    }
    const std::string frag_s{std::istreambuf_iterator<char>(frag_stream),
                             std::istreambuf_iterator<char>()};

    GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
    const GLchar *frag_cstr = frag_s.c_str();
    glShaderSource(frag_shader, 1, &frag_cstr, NULL);
    glCompileShader(frag_shader);

    GLint frag_comp_status;
    glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_comp_status);
    if (!frag_comp_status) {
      std::cerr << "failed to compile fragment shader " << *fragf
                << "\n>> getting shader log...\n";

      GLint frag_log_length;
      glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &frag_log_length);
      GLchar *frag_log =
          static_cast<GLchar *>(malloc(frag_log_length * sizeof(GLchar)));
      glGetShaderInfoLog(frag_shader, frag_log_length, NULL, frag_log);

      std::cerr << frag_log;
      return ERROR_ID;
    }

    glAttachShader(program, frag_shader);
    glDeleteShader(frag_shader);
  }

  glLinkProgram(program);

  GLint link_status;
  glGetProgramiv(program, GL_LINK_STATUS, &link_status);
  if (!link_status) {
    std::cerr << "failed to link shader >> getting program log...\n";

    GLint program_log_length;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &program_log_length);
    GLchar *program_log = (GLchar *)malloc(program_log_length * sizeof(GLchar));
    glGetProgramInfoLog(program, program_log_length, NULL, program_log);

    std::cerr << program_log;
    return ERROR_ID;
  }

  return program;
}

// sets an attribute
export void set_attrib(GLuint vao, GLuint attrib_index, GLuint binding_index,
                       GLint size, GLenum type, GLboolean normalized,
                       GLuint relative_offset, GLbitfield flags) {
  glEnableVertexArrayAttrib(vao, attrib_index);
  glVertexArrayAttribFormat(vao, attrib_index, size, type, normalized,
                            relative_offset);
  glVertexArrayAttribBinding(vao, attrib_index, binding_index);
}
