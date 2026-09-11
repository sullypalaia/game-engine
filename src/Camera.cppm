//--------------------------------------------------
// CURRENTLY THIS CAMERA SYSTEM ONLY SUPPORTS OPENGL
//---------------------------------------------------

module;

#include "external/glad/glad.h"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"

export module Camera;

/**
 * \brief projection and view matrices that will be inside a uniform buffer,
 * accessible by any glsl shader
 */
export struct CameraData {
  glm::mat4 proj;
  glm::mat4 view;
};

export class Camera {
public:
  Camera(const CameraData &cam_data);

  void m_update(const CameraData &cam_data);

  void m_destroy();

private:
  GLuint m_cam_buffer;
};

Camera::Camera(const CameraData &cam_data) { m_update(cam_data); }

/**
 * currently a placeholder
 * the camera types will be components in the future to respect data-oriented
 * design principles
 */
void Camera::m_update(const CameraData &cam_data) {
  glCreateBuffers(1, &m_cam_buffer);
  glNamedBufferStorage(m_cam_buffer, sizeof(CameraData), &cam_data,
                       GL_DYNAMIC_STORAGE_BIT);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_cam_buffer);
}

void Camera::m_destroy() { glDeleteBuffers(1, &m_cam_buffer); }
