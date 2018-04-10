#pragma once

#include <sstream>
#include <stdexcept>

#include <android/log.h>

template <typename... Args>
static void info(Args... args) {
  __android_log_print(ANDROID_LOG_INFO, "hello-world", args...);
}

template <typename... Args>
static void error(Args... args) {
  __android_log_print(ANDROID_LOG_ERROR, "hello-world", args...);
}

#define UTILS_REQUIRE(cond, ...)                           \
  if (!(cond)) do {                                        \
      std::ostringstream msg;                              \
      msg << __FILE__ << ":" << __LINE__ << ": " << #cond; \
      throw std::runtime_error(msg.str());                 \
  } while (0)

#define UTILS_GL_CHECK(glcode, ...)                                       \
  do {                                                                    \
    glcode;                                                               \
    auto error = glGetError();                                            \
    if (error) {                                                          \
      std::ostringstream msg;                                             \
      msg << __FILE__ << ":" << __LINE__ << ": GL call failed: " << #glcode \
          << " (" << error << ")";                                        \
      throw std::runtime_error(msg.str());                                \
    }                                                                     \
  } while (0)
