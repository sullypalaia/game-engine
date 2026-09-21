//--------------------------------------------------
// CURRENTLY THIS CAMERA SYSTEM ONLY SUPPORTS OPENGL AND GLFW
//---------------------------------------------------

module;

#include <algorithm>

#include "external/glad/glad.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GLFW/glfw3.h>

#include "utils/definitions.h"

export module engine.scene.camera_manager;

import engine.platform.window_manager;

/**\brief this class manages cameras, which can be attached to windows
 */
export class CameraManager {
public:
  CameraManager(WindowManager &window_manager);

  const size_t m_create_static_camera_perspective_proj(
      const glm::vec3 initial_pos, const glm::mat4 initial_rot_mat,
      const float fovy, const float znear, const float zfar,
      const size_t window);

  const size_t m_create_static_camera_ortho_proj(
      const glm::vec3 initial_pos, const glm::mat4 initial_rot_mat,
      const float left, const float right, const float bottom, const float top,
      const float znear, const float zfar, const size_t window);

  const size_t m_create_first_person_camera(const glm::vec3 initial_pos,
                                            const glm::mat4 initial_rot_mat,
                                            const float camera_speed,
                                            const float fovy, const float znear,
                                            const float zfar,
                                            const size_t window);

  void m_update();

  void m_destroy();

  std::vector<unsigned int> m_cam_buffers;

  // window info
  inline static WindowManager *m_window_manager = nullptr;
  std::vector<size_t> m_windows;

  // camera info
  std::vector<glm::vec3> m_pose;
  std::vector<glm::vec3> m_ups;
  std::vector<glm::vec3> m_fronts;
  std::vector<float> m_speeds;
  std::vector<float> m_fovys;
  std::vector<float> m_znears;
  std::vector<float> m_zfars;
  std::vector<CameraData> m_cam_datas;

  float m_last_frame_time = 0.0f;

  /**\brief this will check for inputs to change the position of the camera
   */
  void m_process_input();

  static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);

  static inline size_t m_camera_id = 0;
};

CameraManager::CameraManager(WindowManager &window_manager) {
  if (!m_window_manager) {
    m_window_manager = &window_manager;
  }
}

const size_t CameraManager::m_create_first_person_camera(
    const glm::vec3 initial_pos, const glm::mat4 initial_rot_mat,
    const float camera_speed, const float fovy, const float znear,
    const float zfar, const size_t window) {
  m_pose.push_back(initial_pos);
  m_ups.push_back(glm::vec3(0.0f, 1.0f, 0.0f));
  m_fronts.push_back(glm::vec3(0.0f, 0.0f, -1.0f));
  m_speeds.push_back(camera_speed);
  m_fovys.push_back(fovy);
  m_znears.push_back(znear);
  m_zfars.push_back(zfar);

  // create the camera data
  m_cam_datas.push_back(
      {glm::lookAt(initial_pos, initial_pos + glm::vec3(0.0f, 0.0f, -1.0f),
                   glm::vec3(0.0f, 1.0f, 0.0f)),
       glm::perspective(glm::radians(fovy),
                        m_window_manager->m_get_aspect_ratio(window), znear,
                        zfar)});

  // create the storage for the buffer
  glCreateBuffers(1, &m_cam_buffers.emplace_back());
  glNamedBufferStorage(m_cam_buffers.back(), sizeof(CameraData),
                       &m_cam_datas.back(), GL_DYNAMIC_STORAGE_BIT);

  // attach the camera to the window manager
  m_window_manager->m_attach_camera(window, m_camera_id);

  // glfw setup
  glfwSetInputMode(m_window_manager->m_get_window(window), GLFW_CURSOR,
                   GLFW_CURSOR_DISABLED);
  glfwSetCursorPosCallback(m_window_manager->m_get_window(window),
                           CameraManager::cursor_pos_callback);

  // add the window to the list of windows if it is not already there
  if (std::find(m_windows.begin(), m_windows.end(), window) == m_windows.end())
    m_windows.push_back(window);

  return m_camera_id++;
}

void CameraManager::cursor_pos_callback(GLFWwindow *window, double xpos,
                                        double ypos) {
  const size_t window_id =
      *(static_cast<size_t *>(glfwGetWindowUserPointer(window)));

  const std::vector<size_t> camera_ids =
      m_window_manager->m_get_cameras(window_id);

  for (const size_t camera_id : camera_ids) {
  }
}

void CameraManager::m_update() { m_process_input(); }

void CameraManager::m_process_input() {
  float curr_frame_time = glfwGetTime();
  float delta_time = curr_frame_time - m_last_frame_time;
  m_last_frame_time = curr_frame_time;

  for (const size_t window : m_windows) {
    m_window_manager->m_make_context_current(window);

    const std::vector<size_t> camera_ids =
        m_window_manager->m_get_cameras(window);

    for (const size_t camera_id : camera_ids) {
      glm::vec3 &pos = m_pose[camera_id];
      glm::vec3 &up = m_ups[camera_id];
      glm::vec3 &front = m_fronts[camera_id];
      float &speed = m_speeds[camera_id];
      float &fovy = m_fovys[camera_id];
      float &znear = m_znears[camera_id];
      float &zfar = m_zfars[camera_id];

      // update the position
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_W))
        pos += delta_time * speed * front;
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_A))
        pos -= delta_time * speed * glm::normalize(glm::cross(front, up));
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_D))
        pos += delta_time * speed * glm::normalize(glm::cross(front, up));
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_S))
        pos -= delta_time * speed * front;
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_SPACE))
        pos += delta_time * speed * up;
      if (glfwGetKey(m_window_manager->m_get_window(window),
                     GLFW_KEY_LEFT_SHIFT))
        pos -= delta_time * speed * up;

      m_cam_datas[camera_id] = {
          glm::lookAt(pos, pos + front, up),
          glm::perspective(glm::radians(fovy),
                           m_window_manager->m_get_aspect_ratio(window), znear,
                           zfar)};

      glNamedBufferSubData(m_cam_buffers[camera_id], 0, sizeof(CameraData),
                           &m_cam_datas[camera_id]);
      glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_cam_buffers[camera_id]);
    }
  }
}

void CameraManager::m_destroy() {
  for (const size_t window : m_windows) {
    const std::vector<size_t> camera_ids =
        m_window_manager->m_get_cameras(window);

    for (const size_t camera_id : camera_ids)
      glDeleteBuffers(1, &m_cam_buffers[camera_id]);
  }
}
