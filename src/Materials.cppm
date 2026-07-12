module;

#include "external/glad/glad.h"

export module Materials;

export struct SolidColor {
  GLint m_attrib_location;
  GLfloat *m_color;
};
