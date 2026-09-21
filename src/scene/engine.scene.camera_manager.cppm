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

  const size_t
  m_create__camera_perspective_proj(const glm::vec3 initial_pos,
                                    const glm::mat4 initial_rot_mat,
                                    const float fovy, const float znear,
                                    const float zfar, const size_t window);

  const size_t m_create__camera_ortho_proj(const glm::vec3 initial_pos,
                                           const glm::mat4 initial_rot_mat,
                                           const float left, const float right,
                                           const float bottom, const float top,
                                           const float znear, const float zfar,
                                           const size_t window);

  const size_t m_create_first_person_camera(
      const glm::vec3 initial_pos, const float yaw, const float roll,
      const float pitch, const float sensitivity, const float camera_speed,
      const float fovy, const float znear, const float zfar,
      const size_t window);

  void m_update();

  void m_destroy();

  std::vector<unsigned int> m_cam_buffers;

  // window info
  inline static WindowManager *m_window_manager = nullptr;
  std::vector<size_t> m_windows;

  // camera info
  inline static std::vector<glm::vec3> m_pose;
  inline static std::vector<glm::vec3> m_ups;
  inline static std::vector<glm::vec3> m_fronts;
  inline static std::vector<float> m_speeds;
  inline static std::vector<float> m_fovys;
  inline static std::vector<float> m_znears;
  inline static std::vector<float> m_zfars;
  inline static std::vector<CameraData> m_cam_datas;

  inline static std::vector<float> m_yaws;
  inline static std::vector<float> m_pitches;
  inline static std::vector<float> m_rolls;
  inline static std::vector<float> m_last_xs;
  inline static std::vector<float> m_last_ys;
  inline static std::vector<float> m_sensitivities;

  float m_last_frame_time = 0.0f;

  /**\brief this will check for inputs to change the position of the camera
   */
  void m_process_input();

  inline static void cursor_pos_callback(GLFWwindow *window, double xpos,
                                         double ypos);

  size_t m_curr_cam_id = 0;
};

CameraManager::CameraManager(WindowManager &window_manager) {
  if (!m_window_manager) {
    m_window_manager = &window_manager;
  }
}

const size_t CameraManager::m_create_first_person_camera(
    const glm::vec3 initial_pos, const float yaw, const float roll,
    const float pitch, const float sensitivity, const float camera_speed,
    const float fovy, const float znear, const float zfar,
    const size_t window) {
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

  // set the rpys and last_xs/last_ys to 0
  m_yaws.push_back(yaw);
  m_pitches.push_back(pitch);
  m_rolls.push_back(roll);
  m_last_xs.push_back(0.0f);
  m_last_ys.push_back(0.0f);
  m_sensitivities.push_back(sensitivity);

  // create the storage for the buffer
  glCreateBuffers(1, &m_cam_buffers.emplace_back());
  glNamedBufferStorage(m_cam_buffers.back(), sizeof(CameraData),
                       &m_cam_datas.back(), GL_DYNAMIC_STORAGE_BIT);

  // attach the camera to the window manager
  m_window_manager->m_attach_camera(window, m_curr_cam_id);

  // glfw setup
  glfwSetInputMode(m_window_manager->m_get_window(window), GLFW_CURSOR,
                   GLFW_CURSOR_DISABLED);

  glfwSetCursorPosCallback(m_window_manager->m_get_window(window),
                           CameraManager::cursor_pos_callback);

  // add the window to the list of windows if it is not already there
  if (std::find(m_windows.begin(), m_windows.end(), window) == m_windows.end())
    m_windows.push_back(window);

  return m_curr_cam_id++;
}

void CameraManager::cursor_pos_callback(GLFWwindow *window, double xpos,
                                        double ypos) {
  const size_t window_id =
      *(static_cast<size_t *>(glfwGetWindowUserPointer(window)));

  const std::vector<size_t> camera_ids =
      m_window_manager->m_get_cameras(window_id);

  for (const size_t camera_id : camera_ids) {
    float &yaw = m_yaws[camera_id];
    float &pitch = m_pitches[camera_id];
    float &roll = m_rolls[camera_id];

    float &last_x = m_last_xs[camera_id];
    float &last_y = m_last_ys[camera_id];

    // if the last x and y are 0, set them to the current x and y
    if (last_x == 0.0f && last_y == 0.0f) {
      last_x = xpos;
      last_y = ypos;
    }

    const float sensitivity = m_sensitivities[camera_id];

    // find the offset
    float xoffset = (xpos - last_x) * sensitivity;
    float yoffset = (last_y - ypos) * sensitivity;

    // update last_x and last_y
    last_x = xpos;
    last_y = ypos;

    // update yaw and pitch
    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
      pitch = 89.0f;
    if (pitch < -89.0f)
      pitch = -89.0f;

    // get the current direction
    glm::vec3 direction;
    direction.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    direction.y = std::sin(glm::radians(pitch));
    direction.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

    // update the fronts
    m_fronts[camera_id] = glm::normalize(direction);

    m_cam_datas[camera_id] = {
        glm::lookAt(m_pose[camera_id], m_pose[camera_id] + m_fronts[camera_id],
                    m_ups[camera_id]),
        glm::perspective(glm::radians(m_fovys[camera_id]),
                         m_window_manager->m_get_aspect_ratio(window_id),
                         m_znears[camera_id], m_zfars[camera_id])};
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
      glm::vec3 right = glm::normalize(glm::cross(front, up));
      float speed = m_speeds[camera_id];

      // update the position
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_W))
        pos -= delta_time * speed * glm::normalize(glm::cross(right, up));
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_A))
        pos -= delta_time * speed * right;
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_D))
        pos += delta_time * speed * right;
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_S))
        pos += delta_time * speed * glm::normalize(glm::cross(right, up));
      if (glfwGetKey(m_window_manager->m_get_window(window), GLFW_KEY_SPACE))
        pos += delta_time * speed * up;
      if (glfwGetKey(m_window_manager->m_get_window(window),
                     GLFW_KEY_LEFT_SHIFT))
        pos -= delta_time * speed * up;

      m_cam_datas[camera_id] = {
          glm::lookAt(pos, pos + front, up),
          glm::perspective(glm::radians(m_fovys[camera_id]),
                           m_window_manager->m_get_aspect_ratio(window),
                           m_znears[camera_id], m_zfars[camera_id])};

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
