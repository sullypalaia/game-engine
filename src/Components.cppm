module;

#include "external/glad/glad.h"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"

#include "entt/entt.hpp"

export module Components;

import Shaders;

//--------begin opengl-compatible components--------

export struct VertexData {
  const GLsizeiptr *m_sizes;
  const void **m_data;
};

export struct ElementBuffer {
  const GLuint *m_data;
};

export struct RenderInfo {
  const GLenum m_primitive_type;
  const GLint m_num_vertices;
  const GLint m_num_indices;
};

export struct Attrib {
  const GLuint m_loc;
  const GLint m_size;
  const GLint m_attrib_group;
  const GLenum m_type;
};

export struct Attribs {
  const Attrib *m_attribs;
  const size_t m_num_attribs;
  const size_t m_num_attrib_groups;
};

export struct Transform {
  glm::mat4 m_transform;
  GLint m_transform_loc;
};

export struct ShaderProgram {
  GLuint m_id;
};

//--------end opengl-compatible components--------
