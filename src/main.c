#include <stdio.h>
#include <assert.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define log_info(msg) fprintf(stderr, "\x1b[1;32minfo: " msg "\x1b[0m\n")
#define log_warn(msg) fprintf(stderr, "\x1b[1;33mwarn: " msg "\x1b[0m\n")
#define log_error(msg) fprintf(stderr, "\x1b[1;31merror: " msg "\x1b[0m\n")
#define log_infof(fmt, ...) fprintf(stderr, "\x1b[1;32minfo: " fmt "\x1b[0m\n", ## __VA_ARGS__)
#define log_warnf(fmt, ...) fprintf(stderr, "\x1b[1;33mwarn: " fmt "\x1b[0m\n", ## __VA_ARGS__)
#define log_errorf(fmt, ...) fprintf(stderr, "\x1b[1;31merror: " fmt "\x1b[0m\n", ## __VA_ARGS__)

#define WINDOW_W 640
#define WINDOW_H 480

int
main(void) {
  assert(glfwInit());
  log_info("glfw initialized");
  glfwWindowHint(GLFW_RESIZABLE, false);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  GLFWwindow *window = glfwCreateWindow(WINDOW_W, WINDOW_H, "jarpg", 0, 0);
  assert(window);
  glfwMakeContextCurrent(window);
  log_infof("window created with %d %d dimensions", WINDOW_W, WINDOW_H);
  const GLFWvidmode *vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  if (vidmode) {
    glfwSetWindowPos(window, vidmode->width / 2 - WINDOW_W / 2, vidmode->height / 2 - WINDOW_H / 2);
    log_info("window centered");
  } else {
    log_warn("couldn't center window");
  }
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
    glfwSwapBuffers(window);
  }
  glfwTerminate();
  return 0;
}
