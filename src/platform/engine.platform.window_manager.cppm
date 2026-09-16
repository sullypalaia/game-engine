//--------------------------------------------------
// CURRENTLY THIS WINDOW MANAGER ONLY SUPPORTS OPENGL
//---------------------------------------------------

module;

#include <cassert>
#include <cstdlib>
#include <format>
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
                               GLFWmonitor *monitor, GLFWwindow *share,
                               const bool x11 = false);

  // get the window if needed
  GLFWwindow *m_get_window(const size_t window) const;

  // get the aspect ratio of a window
  float m_get_aspect_ratio(const size_t window) const;

  // attach the window to the current context
  void m_make_context_current(const size_t window);

  // destroy all the managed windows
  void m_destroy();

private:
  std::vector<GLFWwindow *> m_windows;

  /**
   * \brief resizes the opengl viewport when the window size changes
   */
  friend void resize_callback(GLFWwindow *window, int width, int height);

  inline static size_t current_id = 0;

  inline static bool glad_initialized = false;
};

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// resize callback
void resize_callback(GLFWwindow *window, int width, int height) {
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
                                            GLFWwindow *share, const bool x11) {
  // create the window with the specified parameters
  GLFWwindow *window = glfwCreateWindow(width, height, title, monitor, share);
  if (!window) {
    std::cerr << std::format("failed to create window \"{}\"\n", title);
    glfwTerminate();
    std::exit(-1);
  }

  // set the user pointer to the windo
  glfwSetWindowUserPointer(window, m_windows.data());

  // set the callbacks for the window
  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, resize_callback);

  // add the new window to the vector, return the current id, and increment the
  // current id for the next window
  m_windows.push_back(window);
  return current_id++;
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
  glfwGetWindowSize(curr_window, &width, &height);
  glViewport(0, 0, width, height);
}

GLFWwindow *WindowManager::m_get_window(const size_t window) const {
  return m_windows[window];
}

float WindowManager::m_get_aspect_ratio(const size_t window) const {
  int width, height;
  glfwGetWindowSize(m_windows[window], &width, &height);

  return static_cast<float>(width) / static_cast<float>(height);
}

void WindowManager::m_destroy() {
  for (const auto &window : m_windows)
    glfwDestroyWindow(window);
  glfwTerminate();
}
