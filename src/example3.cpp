//--------------------------------------------------------------------------------
// CREATED USING CHATGPT-5.6 TERRA ON MEDIUM
//--------------------------------------------------------------------------------

// Neon Skyrail — an animated 2D showcase for the current renderer.
// It deliberately uses only Position2D, Color, and Transform: the small API
// is enough to make a surprisingly deep layered scene.

#include <array>
#include <cmath>
#include <vector>

#include "external/GLFW/glfw3.h"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"

#include "entt/entt.hpp"

import Camera;
import Components;
import ECS;
import OpenGLRenderer;
import WindowManager;

namespace {

constexpr float pi = 3.14159265f;
using RGBA = std::array<float, 4>;

struct Painter {
  std::vector<GLfloat> positions;
  std::vector<GLfloat> colors;

  void vertex(float x, float y, const RGBA &c) {
    positions.insert(positions.end(), {x, y});
    colors.insert(colors.end(), c.begin(), c.end());
  }
  void tri(float ax, float ay, float bx, float by, float cx, float cy,
           const RGBA &c) {
    vertex(ax, ay, c);
    vertex(bx, by, c);
    vertex(cx, cy, c);
  }
  void quad(float l, float b, float r, float t, const RGBA &c) {
    tri(l, b, r, b, r, t, c);
    tri(l, b, r, t, l, t, c);
  }
  void gradient_quad(float l, float b, float r, float t, const RGBA &bottom,
                     const RGBA &top) {
    vertex(l, b, bottom);
    vertex(r, b, bottom);
    vertex(r, t, top);
    vertex(l, b, bottom);
    vertex(r, t, top);
    vertex(l, t, top);
  }
  void diamond(float x, float y, float radius, const RGBA &c) {
    tri(x, y + radius, x + radius, y, x, y - radius, c);
    tri(x, y + radius, x, y - radius, x - radius, y, c);
  }
  void disc(float x, float y, float radius, const RGBA &c, int sides = 16) {
    for (int i = 0; i < sides; ++i) {
      const float a = 2.0f * pi * i / sides;
      const float b = 2.0f * pi * (i + 1) / sides;
      tri(x, y, x + std::cos(a) * radius, y + std::sin(a) * radius,
          x + std::cos(b) * radius, y + std::sin(b) * radius, c);
    }
  }
};

glm::mat4 transform(float x, float y, float scale = 1.0f, float angle = 0.0f) {
  return glm::rotate(
      glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)),
                 glm::vec3(scale)),
      angle, glm::vec3(0.0f, 0.0f, 1.0f));
}

std::vector<GLfloat> craft_mesh() {
  Painter p;
  const RGBA hull{0.16f, 0.88f, 1.0f, 1.0f};
  const RGBA glass{0.55f, 0.15f, 0.95f, 1.0f};
  const RGBA light{1.0f, 0.85f, 0.25f, 1.0f};
  p.tri(-0.24f, 0.00f, 0.20f, 0.00f, 0.32f, 0.07f, hull);
  p.tri(-0.24f, 0.00f, 0.32f, 0.07f, -0.06f, 0.14f, hull);
  p.tri(-0.10f, 0.14f, 0.08f, 0.14f, 0.17f, 0.23f, glass);
  p.tri(-0.10f, 0.14f, 0.17f, 0.23f, -0.03f, 0.25f, glass);
  p.tri(-0.19f, 0.0f, -0.40f, -0.09f, -0.10f, 0.02f, light);
  p.tri(-0.19f, 0.0f, -0.10f, 0.02f, -0.30f, 0.08f, light);
  return p.positions;
}

std::vector<GLfloat> craft_colors() {
  Painter p;
  const RGBA hull{0.16f, 0.88f, 1.0f, 1.0f};
  const RGBA glass{0.55f, 0.15f, 0.95f, 1.0f};
  const RGBA light{1.0f, 0.85f, 0.25f, 1.0f};
  auto add = [&](const RGBA &c, int n) {
    for (int i = 0; i < n; ++i)
      p.vertex(0, 0, c);
  };
  add(hull, 6);
  add(glass, 6);
  add(light, 6);
  return p.colors;
}

} // namespace

int main() {
  WindowManager window_manager(true);
  const size_t window = window_manager.m_create_window(
      1280, 720, "Neon Skyrail — Example 3", nullptr, nullptr, true);
  window_manager.m_make_context_current(window);

  const float aspect = window_manager.m_get_aspect_ratio(window);
  Camera camera(
      CameraData{glm::ortho(-aspect, aspect, -1.0f, 1.0f), glm::mat4(1.0f)});
  ECS ecs;

  // One static mesh: sky gradients, a moon, distant mountains, buildings,
  // windows, rooftop gardens, and the elevated rail all render in one batch.
  Painter city;
  city.gradient_quad(-aspect, -1.0f, aspect, 1.0f,
                     {0.025f, 0.015f, 0.10f, 1.0f},
                     {0.04f, 0.18f, 0.34f, 1.0f});
  city.disc(0.90f, 0.55f, 0.28f, {0.95f, 0.40f, 0.72f, 1.0f}, 24);
  city.disc(0.90f, 0.55f, 0.22f, {1.0f, 0.72f, 0.50f, 1.0f}, 24);

  // Stars and a few larger sparkling constellations.
  for (int i = 0; i < 48; ++i) {
    const float x =
        -aspect + 0.08f + std::fmod(i * 0.347f, 2.0f * aspect - 0.16f);
    const float y = 0.10f + std::fmod(i * 0.193f, 0.82f);
    const float r = 0.006f + (i % 4) * 0.002f;
    city.diamond(x, y, r, {0.45f, 0.86f, 1.0f, 1.0f});
  }
  for (int i = 0; i < 8; ++i)
    city.diamond(-1.42f + i * 0.31f, 0.57f + (i % 3) * 0.11f, 0.018f,
                 {0.95f, 0.78f, 1.0f, 1.0f});

  // Distant geometric mountain silhouettes.
  for (int i = 0; i < 10; ++i) {
    const float x = -aspect - 0.15f + i * 0.40f;
    const float h = 0.28f + (i % 3) * 0.10f;
    city.tri(x, -0.14f, x + 0.42f, -0.14f, x + 0.21f, -0.14f + h,
             {0.08f, 0.13f, 0.28f, 1.0f});
    city.tri(x + 0.21f, -0.14f + h, x + 0.31f, -0.14f, x + 0.21f,
             -0.14f + h * 0.58f, {0.13f, 0.23f, 0.39f, 1.0f});
  }

  // Dense city blocks; every facade gets glowing window rows and antennae.
  const float widths[] = {0.24f, 0.32f, 0.19f, 0.29f, 0.23f, 0.34f,
                          0.21f, 0.27f, 0.18f, 0.31f, 0.25f};
  float x = -aspect;
  for (int b = 0; b < 11; ++b) {
    const float w = widths[b];
    const float h = 0.38f + (b % 5) * 0.10f;
    const float roof = -0.42f + h;
    const RGBA facade = (b % 2 == 0) ? RGBA{0.055f, 0.09f, 0.20f, 1.0f}
                                     : RGBA{0.10f, 0.055f, 0.22f, 1.0f};
    city.quad(x, -0.42f, x + w, roof, facade);
    city.tri(x - 0.025f, roof, x + w + 0.025f, roof, x + w * 0.5f, roof + 0.07f,
             {0.18f, 0.10f, 0.34f, 1.0f});
    for (int row = 0; row < 5; ++row)
      for (int col = 0; col < 3; ++col) {
        if ((row + col + b) % 4 == 0)
          continue;
        const float wx = x + 0.035f + col * (w - 0.07f) / 3.0f;
        const float wy = -0.34f + row * 0.085f;
        city.quad(wx, wy, wx + 0.030f, wy + 0.038f,
                  ((row + col + b) % 3 == 0) ? RGBA{1.0f, 0.36f, 0.66f, 1.0f}
                                             : RGBA{0.22f, 0.85f, 1.0f, 1.0f});
      }
    if (b % 3 == 0)
      city.quad(x + w * 0.48f, roof + 0.06f, x + w * 0.52f, roof + 0.18f,
                {0.85f, 0.35f, 0.9f, 1.0f});
    x += w - 0.015f;
  }

  // Skyrail viaduct, support pylons, and a lower foreground promenade.
  city.quad(-aspect, -0.12f, aspect, -0.06f, {0.10f, 0.72f, 0.88f, 1.0f});
  city.quad(-aspect, -0.17f, aspect, -0.12f, {0.035f, 0.08f, 0.16f, 1.0f});
  for (float p = -aspect + 0.12f; p < aspect; p += 0.40f) {
    city.quad(p, -0.52f, p + 0.045f, -0.17f, {0.08f, 0.16f, 0.28f, 1.0f});
    city.tri(p - 0.06f, -0.52f, p + 0.105f, -0.52f, p + 0.022f, -0.38f,
             {0.12f, 0.23f, 0.36f, 1.0f});
  }
  city.quad(-aspect, -1.0f, aspect, -0.69f, {0.018f, 0.026f, 0.08f, 1.0f});
  for (int i = 0; i < 15; ++i) {
    const float px = -aspect + i * 0.26f;
    city.tri(px, -0.70f, px + 0.13f, -0.70f, px + 0.065f, -0.60f,
             {0.05f, 0.20f, 0.21f, 1.0f});
  }

  // Reusable colored sprites, each with an independently animated transform.
  const auto ship = ecs.m_add_entity_with_component<Position2D>(craft_mesh());
  ecs.m_add_component_to_entity<Color>(ship, craft_colors());
  ecs.m_add_component_to_entity<Transform>(ship, transform(0, 0));

  Painter train_p;
  train_p.quad(-0.46f, -0.08f, 0.46f, 0.08f, {0.10f, 0.92f, 1.0f, 1.0f});
  train_p.tri(-0.46f, -0.08f, -0.58f, -0.01f, -0.46f, 0.08f,
              {0.55f, 0.12f, 0.95f, 1.0f});
  train_p.tri(0.46f, -0.08f, 0.58f, -0.01f, 0.46f, 0.08f,
              {0.55f, 0.12f, 0.95f, 1.0f});
  for (int i = 0; i < 5; ++i)
    train_p.quad(-0.31f + i * .14f, -0.005f, -0.23f + i * .14f, .055f,
                 {1.0f, .74f, .32f, 1.0f});
  const auto train =
      ecs.m_add_entity_with_component<Position2D>(train_p.positions);
  ecs.m_add_component_to_entity<Color>(train, train_p.colors);
  ecs.m_add_component_to_entity<Transform>(train, transform(0, 0));

  Painter drone_p;
  drone_p.diamond(0, 0, .055f, {1.0f, .25f, .65f, 1.0f});
  drone_p.quad(-.13f, -.008f, -.055f, .008f, {0.2f, .9f, 1.0f, 1.0f});
  drone_p.quad(.055f, -.008f, .13f, .008f, {0.2f, .9f, 1.0f, 1.0f});
  std::vector<entt::entity> drones;
  for (int i = 0; i < 7; ++i) {
    const auto d =
        ecs.m_add_entity_with_component<Position2D>(drone_p.positions);
    ecs.m_add_component_to_entity<Color>(d, drone_p.colors);
    ecs.m_add_component_to_entity<Transform>(d, transform(0, 0));
    drones.push_back(d);
  }

  Painter rain_p;
  rain_p.quad(-.008f, -.10f, .008f, .10f, {0.30f, .82f, 1.0f, 1.0f});
  std::vector<entt::entity> rain;
  for (int i = 0; i < 32; ++i) {
    const auto drop =
        ecs.m_add_entity_with_component<Position2D>(rain_p.positions);
    ecs.m_add_component_to_entity<Color>(drop, rain_p.colors);
    ecs.m_add_component_to_entity<Transform>(drop, transform(0, 0));
    rain.push_back(drop);
  }

  // A giant animated halo, a scanning beacon, flickering billboards, and
  // street traffic add obvious motion even while the skyline holds steady.
  Painter halo_p;
  for (int i = 0; i < 18; ++i) {
    const float a = 2.0f * pi * i / 18.0f;
    const float b = a + 0.18f;
    const float r0 = 0.28f, r1 = 0.315f;
    const RGBA c = (i % 2 == 0) ? RGBA{0.18f, 0.96f, 1.0f, 1.0f}
                                : RGBA{1.0f, 0.22f, 0.72f, 1.0f};
    halo_p.tri(std::cos(a) * r0, std::sin(a) * r0, std::cos(a) * r1,
               std::sin(a) * r1, std::cos(b) * r1, std::sin(b) * r1, c);
    halo_p.tri(std::cos(a) * r0, std::sin(a) * r0, std::cos(b) * r1,
               std::sin(b) * r1, std::cos(b) * r0, std::sin(b) * r0, c);
  }
  const auto halo =
      ecs.m_add_entity_with_component<Position2D>(halo_p.positions);
  ecs.m_add_component_to_entity<Color>(halo, halo_p.colors);
  ecs.m_add_component_to_entity<Transform>(halo, transform(0, 0));

  Painter beam_p;
  beam_p.tri(0, 0, 0.72f, -0.055f, 0.72f, 0.055f, {0.18f, 0.86f, 1.0f, 1.0f});
  beam_p.diamond(0.72f, 0.0f, 0.05f, {1.0f, 0.32f, 0.72f, 1.0f});
  const auto beacon =
      ecs.m_add_entity_with_component<Position2D>(beam_p.positions);
  ecs.m_add_component_to_entity<Color>(beacon, beam_p.colors);
  ecs.m_add_component_to_entity<Transform>(beacon, transform(0, 0));

  Painter sign_p;
  sign_p.quad(-.11f, -.045f, .11f, .045f, {1.0f, .18f, .67f, 1.0f});
  sign_p.quad(-.075f, -.018f, .075f, .018f, {0.25f, .95f, 1.0f, 1.0f});
  std::vector<entt::entity> signs;
  for (int i = 0; i < 8; ++i) {
    const auto sign =
        ecs.m_add_entity_with_component<Position2D>(sign_p.positions);
    ecs.m_add_component_to_entity<Color>(sign, sign_p.colors);
    ecs.m_add_component_to_entity<Transform>(sign, transform(0, 0));
    signs.push_back(sign);
  }

  Painter car_p;
  car_p.quad(-.075f, -.025f, .075f, .025f, {1.0f, .64f, .14f, 1.0f});
  car_p.tri(.075f, -.025f, .115f, 0, .075f, .025f, {1.0f, .25f, .55f, 1.0f});
  car_p.diamond(-.045f, 0, .018f, {0.75f, .98f, 1.0f, 1.0f});
  std::vector<entt::entity> traffic;
  for (int i = 0; i < 12; ++i) {
    const auto car =
        ecs.m_add_entity_with_component<Position2D>(car_p.positions);
    ecs.m_add_component_to_entity<Color>(car, car_p.colors);
    ecs.m_add_component_to_entity<Transform>(car, transform(0, 0));
    traffic.push_back(car);
  }

  // The renderer groups entities by component mask and draws groups in ECS
  // iteration order (newest first). Create this opaque, full-screen mesh last
  // so its no-Transform group is painted first, behind every animated sprite.
  const auto static_scene =
      ecs.m_add_entity_with_component<Position2D>(city.positions);
  ecs.m_add_component_to_entity<Color>(static_scene, city.colors);

  constexpr GLfloat clear_color[4] = {0.01f, 0.01f, 0.035f, 1.0f};
  OpenGLRenderer renderer(ecs.m_get_registry(), clear_color);

  while (!glfwWindowShouldClose(window_manager.m_get_window(window))) {
    const float t = static_cast<float>(glfwGetTime());
    const float ship_x = std::sin(t * .58f) * aspect * .68f;
    const float ship_y = .39f + std::sin(t * 1.9f) * .16f;
    ecs.m_update_entity_component<Transform>(
        ship, transform(ship_x, ship_y, .82f + .12f * std::sin(t * 3.8f),
                        std::cos(t * 1.9f) * .20f));
    ecs.m_update_entity_component<Transform>(
        train,
        transform(std::fmod(t * 1.15f, 2.0f * aspect + 1.4f) - aspect - .7f,
                  -.09f, .72f));

    ecs.m_update_entity_component<Transform>(
        halo,
        transform(.90f, .55f, 1.0f + .16f * std::sin(t * 2.8f), -t * .72f));
    ecs.m_update_entity_component<Transform>(
        beacon, transform(-.95f, .34f, .75f + .12f * std::sin(t * 4.0f),
                          std::sin(t * 1.15f) * .80f));

    for (size_t i = 0; i < signs.size(); ++i) {
      const float sx = -1.48f + static_cast<float>(i) * .40f;
      const float sy = -.02f + (i % 3) * .18f;
      const float pulse = .70f + .25f * std::sin(t * (3.0f + i * .17f) + i);
      ecs.m_update_entity_component<Transform>(
          signs[i], transform(sx, sy, pulse, std::sin(t * 1.7f + i) * .05f));
    }
    for (size_t i = 0; i < traffic.size(); ++i) {
      const float lane = -.77f + (i % 3) * .075f;
      const float speed = .55f + (i % 4) * .18f;
      const float tx =
          std::fmod(t * speed + i * .43f, 2.0f * aspect + .30f) - aspect - .15f;
      ecs.m_update_entity_component<Transform>(
          traffic[i], transform(tx, lane, .72f, (i % 2 ? .03f : -.03f)));
    }

    for (size_t i = 0; i < drones.size(); ++i) {
      const float phase = static_cast<float>(i) * .91f;
      const float dx = std::sin(t * (.55f + .06f * i) + phase) * aspect * .72f;
      const float dy =
          .18f + std::fmod(i * .17f, .46f) + std::sin(t * 2.3f + phase) * .05f;
      ecs.m_update_entity_component<Transform>(
          drones[i],
          transform(dx, dy, .65f, std::sin(t * 3.0f + phase) * .18f));
    }
    for (size_t i = 0; i < rain.size(); ++i) {
      const float phase = static_cast<float>(i);
      const float rx =
          -aspect + std::fmod(phase * .287f + t * .16f, 2.0f * aspect);
      const float ry =
          1.2f - std::fmod(phase * .113f + t * (.48f + (i % 5) * .08f), 2.25f);
      ecs.m_update_entity_component<Transform>(rain[i],
                                               transform(rx, ry, .42f, -.22f));
    }

    renderer.m_draw();
    glfwSwapBuffers(window_manager.m_get_window(window));
    glfwPollEvents();
  }

  camera.m_destroy();
  renderer.m_destroy();
  window_manager.m_destroy();
  return 0;
}
