#include <string.h>
#include <GLFW/glfw3.h>
#include "yard/core.h"
#include "yard/window.h"

struct window {
  GLFWwindow *handle;
  uint32_t width;
  uint32_t height;
  bool keys[KEY_AMOUNT];
  bool pkeys[KEY_AMOUNT];
};

static struct window window;

static void
key_callback(GLFWwindow* _window, int key, int _scancode, int action, int _mods) {
  (void)_window; (void)_scancode; (void) _mods;
  if (action != GLFW_PRESS && action != GLFW_RELEASE) return;
  switch (key) {
    case GLFW_KEY_ESCAPE: window.keys[K_EXIT]  = action == GLFW_PRESS; break;
    case GLFW_KEY_RIGHT:  window.keys[K_RIGHT] = action == GLFW_PRESS; break;
    case GLFW_KEY_LEFT:   window.keys[K_LEFT]  = action == GLFW_PRESS; break;
    case GLFW_KEY_UP:     window.keys[K_UP]    = action == GLFW_PRESS; break;
    case GLFW_KEY_DOWN:   window.keys[K_DOWN]  = action == GLFW_PRESS; break;
    case GLFW_KEY_Z:      window.keys[K_A]     = action == GLFW_PRESS; break;
    case GLFW_KEY_X:      window.keys[K_B]     = action == GLFW_PRESS; break;
  }
}

bool
window_make(uint32_t width, uint32_t height) {
  log_info("making window...");
  if (!glfwInit()) {
    log_error("couldn't initialize glfw");
    return false;
  }
  log_info("glfw initialized");
  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  window.handle = glfwCreateWindow(width, height, "Yet Another Roguelike-ish Dungeon", 0, 0);
  if (!window.handle) {
    log_error("couldn't create window handle");
    return false;
  }
  glfwMakeContextCurrent(window.handle);
  log_infof("window created with %d %d dimensions", width, height);
  const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  if (vidmode) {
    glfwSetWindowPos(window.handle, vidmode->width / 2 - width / 2, vidmode->height / 2 - height / 2);
    log_info("window centered");
  } else {
    log_warn("couldn't center window");
  }
  (void)memset(window.keys, false, sizeof (bool) * KEY_AMOUNT);
  (void)glfwSetKeyCallback(window.handle, key_callback);
  log_info("window input setup");
  log_info("window creation complete!");
  return true;
}

void
window_close(void) {
  (void)glfwSetWindowShouldClose(window.handle, true);
}

bool
window_is_running(void) {
  memcpy(window.keys, window.pkeys, sizeof (bool) * KEY_AMOUNT);
  glfwPollEvents();
  glfwSwapBuffers(window.handle);
  return !glfwWindowShouldClose(window.handle);
}

bool
window_is_key_press(enum key key) {
#if DEV
  if (key < KEY_FIRST || key >= KEY_AMOUNT) {
    log_errorf("%s: passing invalid key", __func__);
    return false;
  }
#endif
  return !window.pkeys[key] && window.keys[key];
}

bool
window_is_key_down(enum key key) {
#if DEV
  if (key < KEY_FIRST || key >= KEY_AMOUNT) {
    log_errorf("%s: passing invalid key", __func__);
    return false;
  }
#endif
  return window.keys[key];
}

bool
window_is_key_up(enum key key) {
#if DEV
  if (key < KEY_FIRST || key >= KEY_AMOUNT) {
    log_errorf("%s: passing invalid key", __func__);
    return false;
  }
#endif
  return !window.keys[key];
}
