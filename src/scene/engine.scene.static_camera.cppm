//--------------------------------------------------
// CURRENTLY THIS CAMERA SYSTEM ONLY SUPPORTS OPENGL
//---------------------------------------------------

module;

#include "../utils/macros.h"
#include "external/glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/definitions.h"

export module engine.scene.static_camera;

/**\brief this is a camera with a fixed position
 */
export class StaticCamera {
public:
  StaticCamera(const glm::vec3 initial_pos, const glm::mat4 initial_rot_mat,
               const int proj_type);

  void m_destroy();

private:
  GLuint m_cam_buffer;
};

StaticCamera::StaticCamera(const glm::vec3 initial_pos,
                           const glm::mat4 initial_rot_mat,
                           const int proj_type) {
  // create the camera data

  glm::mat4 transform = glm::translate(initial_rot_mat, initial_pos);

  glm::mat4 projection;
  if (proj_type == ENGINE_USE_ORTHO_PROJ) {
    projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
  } else if (proj_type == ENGINE_USE_PERSPECTIVE_PROJ) {
    projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
  }

  CameraData cam_data{transform, projection};

  glCreateBuffers(1, &m_cam_buffer);
  glNamedBufferStorage(m_cam_buffer, sizeof(CameraData), &cam_data, 0);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_cam_buffer);
}

void StaticCamera::m_destroy() { glDeleteBuffers(1, &m_cam_buffer); }
