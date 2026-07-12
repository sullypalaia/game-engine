module;

#include "external/glad/glad.h"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"

export module Camera;

export struct CameraData {
  glm::mat4 proj;
  glm::mat4 view;
};

export class Camera {
public:
  Camera(const CameraData &cam_data);

  void m_update(const CameraData &cam_data);

  ~Camera();

private:
  GLuint m_cam_buffer;
};

Camera::Camera(const CameraData &cam_data) { m_update(cam_data); }

void Camera::m_update(const CameraData &cam_data) {
  glCreateBuffers(1, &m_cam_buffer);
  glNamedBufferStorage(m_cam_buffer, sizeof(CameraData), &cam_data,
                       GL_DYNAMIC_STORAGE_BIT);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_cam_buffer);
}

Camera::~Camera() { glDeleteBuffers(1, &m_cam_buffer); }
