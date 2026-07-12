module;

#include <cmath>

#include "macros.h"

#include "external/glad/glad.h"

export module Meshes;

import Shaders;

//-------square---------
// abs(edge) for all edges is set to 1 by default
export constexpr GLfloat square_vertices[]{
    -1.0, -1.0, // bottom left
    -1.0, 1.0,  // top left
    1.0,  -1.0, // bottom right
    1.0,  1.0   // top right
};

// CCW
export constexpr GLuint square_indices[]{
    0, 3, 1, // left
    0, 2, 3  // right
};

//------triangle--------
export constexpr GLfloat triangle_vertices[]{
    -1.0, -1.0, // bottom left
    1.0,  -1.0, // bottom right
    0.0,  1.0   // top middle
};
