module;

#include <cstdlib>
#include <format>
#include <iostream>

#include "external/glad/glad.h"

#include "external/GLFW/glfw3.h"

export module Window;

export class Window {
public:
  Window(int width, int height, const char *title, GLFWmonitor *monitor,
         GLFWwindow *share);

  constexpr GLFWwindow *m_get_window() const;
  constexpr float m_get_aspect_ratio() const;

  void m_destroy();

private:
  GLFWwindow *m_window = nullptr;
  float m_aspect_ratio;

  friend void resize_callback(GLFWwindow *window, int width, int height);
};

// debug callback
void debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                    GLsizei length, const GLchar *message,
                    const void *userParam) {
  if (severity != GL_DEBUG_SEVERITY_NOTIFICATION)
    std::cerr << "OPENGL ERROR TYPE: " << type << "\nSEVERITY: " << severity
              << "\nMESSAGE: " << message << '\n';
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

// resize callback
void resize_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
  Window *curr_window = static_cast<Window *>(glfwGetWindowUserPointer(window));
  curr_window->m_aspect_ratio =
      static_cast<float>(width) / static_cast<float>(height);
}

// create window and its context
Window::Window(int width, int height, const char *title, GLFWmonitor *monitor,
               GLFWwindow *share) {
  m_window = glfwCreateWindow(width, height, title, monitor, share);
  if (!m_window) {
    std::cerr << std::format("failed to create window \"{}\"\n", title);
    glfwTerminate();
    std::exit(-1);
  }

  glfwSetWindowUserPointer(m_window, this);

  glfwMakeContextCurrent(m_window);

  static bool glad_loaded = false;

  if (!glad_loaded) {
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glad_loaded = true;
  }

  glViewport(0, 0, width, height);

  glfwSetKeyCallback(m_window, key_callback);
  glfwSetFramebufferSizeCallback(m_window, resize_callback);

#ifndef NDEBUG
  glEnable(GL_DEBUG_OUTPUT);
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
  glDebugMessageCallback(debug_callback, nullptr);
#endif

  m_aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
}

constexpr GLFWwindow *Window::m_get_window() const { return m_window; }

constexpr float Window::m_get_aspect_ratio() const { return m_aspect_ratio; }

void Window::m_destroy() { glfwDestroyWindow(m_window); }
