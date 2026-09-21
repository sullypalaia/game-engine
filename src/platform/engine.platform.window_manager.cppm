//--------------------------------------------------
// CURRENTLY THIS WINDOW MANAGER ONLY SUPPORTS OPENGL
//---------------------------------------------------

module;

#include <cassert>
#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <vector>

#include "external/glad/glad.h"

#include "GLFW/glfw3.h"

export module engine.platform.window_manager;

/**
 * \brief creates a window manager for managing windows
 */
export class WindowManager {
public:
  WindowManager(const bool x11 = false);

  /**\brief creates a window, adds it to the windows vector, and returns the
   * handler
   */
  const size_t m_create_window(int width, int height, const char *title,
                               GLFWmonitor *monitor, GLFWwindow *share);

  // get the window if needed
  GLFWwindow *m_get_window(const size_t window) const;

  // get the aspect ratio of a window
  float m_get_aspect_ratio(const size_t window) const;

  // attach the window to the current context
  void m_make_context_current(const size_t window);

  // attach a camera to the window
  void m_attach_camera(const size_t window, const size_t camera);

  // get a camera of a window
  const std::vector<size_t> m_get_cameras(const size_t window) const;

  void m_loop(std::function<void()> update_func);

  // destroy all the managed windows
  void m_destroy();

private:
  // each window has these properties:
  std::vector<size_t> m_window_ids;
  std::vector<GLFWwindow *> m_windows;
  std::vector<std::vector<size_t>> m_window_cameras;

  /**
   * \brief resizes the opengl viewport when the window size changes
   */
  inline static void resize_callback(GLFWwindow *window, int width, int height);

  size_t m_current_id = 0;

  inline static bool glad_initialized = false;
};

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// resize callback
void WindowManager::resize_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

WindowManager::WindowManager(const bool x11) {
  if (x11)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);

  if (!glfwInit()) {
    std::cerr << "failed to initialize glfw\n";
    std::exit(-1);
  }

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
}

/**
 * \brief creates GLFW window and its associated opengl context
 *, sets callbacks
 */
const size_t WindowManager::m_create_window(int width, int height,
                                            const char *title,
                                            GLFWmonitor *monitor,
                                            GLFWwindow *share) {
  // create the window with the specified parameters
  GLFWwindow *window = glfwCreateWindow(width, height, title, monitor, share);
  if (!window) {
    std::cerr << std::format("failed to create window \"{}\"\n", title);
    glfwTerminate();
    std::exit(-1);
  }

  // set the user pointer to the window
  m_window_ids.push_back(m_current_id);
  glfwSetWindowUserPointer(window, &m_window_ids[m_current_id]);

  // set the callbacks for the window
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, WindowManager::resize_callback);

  m_windows.push_back(window);
  m_window_cameras.push_back(std::vector<size_t>());

  return m_current_id++;
}

/**\brief makes the opengl context of the specified window current, and sets the
 * viewport to the size of the window
 */
void WindowManager::m_make_context_current(const size_t window) {
  GLFWwindow *curr_window = m_windows[window];

  // attach the window to the current context
  glfwMakeContextCurrent(curr_window);

  // load opengl functions
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  // set the viewport
  int width, height;
  glfwGetFramebufferSize(curr_window, &width, &height);
  glViewport(0, 0, width, height);
}

GLFWwindow *WindowManager::m_get_window(const size_t window) const {
  return m_windows[window];
}

float WindowManager::m_get_aspect_ratio(const size_t window) const {
  int width, height;
  glfwGetFramebufferSize(m_windows[window], &width, &height);

  return static_cast<float>(width) / static_cast<float>(height);
}

void WindowManager::m_attach_camera(const size_t window, const size_t camera) {
  m_window_cameras[window].push_back(camera);
}

const std::vector<size_t>
WindowManager::m_get_cameras(const size_t window) const {
  return m_window_cameras[window];
}

void WindowManager::m_loop(std::function<void()> update_func) {
  if (!update_func)
    return;

  while (!glfwWindowShouldClose(m_windows[0])) {
    glfwPollEvents();
    update_func();
    glfwSwapBuffers(m_windows[0]);
  }
}

void WindowManager::m_destroy() {
  for (const auto &window : m_windows)
    glfwDestroyWindow(window);
  glfwTerminate();
}
