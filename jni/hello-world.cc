#include <android/log.h>
#include <jni.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

namespace {

template <typename... Args>
static void info(Args... args) {
  __android_log_print(ANDROID_LOG_INFO, "hello-world", args...);
}

template <typename... Args>
static void error(Args... args) {
  __android_log_print(ANDROID_LOG_ERROR, "hello-world", args...);
}

static void checkGlError(const char* op) {
  for (GLint error = glGetError(); error; error = glGetError()) {
    info("after %s() glError (0x%x)\n", op, error);
  }
}

static const char gVertexShader[] =
    "attribute vec4 vPosition;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "}\n";

static const char gFragmentShader[] =
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(0.0, 0.25, 0.5, 1.0);\n"
    "}\n";

GLuint loadShader(GLenum shaderType, const char* pSource) {
  GLuint shader = glCreateShader(shaderType);
  if (shader) {
    glShaderSource(shader, 1, &pSource, NULL);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
      GLint infoLen = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
      if (infoLen) {
        char* buf = (char*)malloc(infoLen);
        if (buf) {
          glGetShaderInfoLog(shader, infoLen, NULL, buf);
          error("Could not compile shader %d:\n%s\n", shaderType, buf);
          free(buf);
        }
        glDeleteShader(shader);
        shader = 0;
      }
    }
  }
  return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
  GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
  if (!vertexShader) {
    return 0;
  }

  GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
  if (!pixelShader) {
    return 0;
  }

  GLuint program = glCreateProgram();
  if (program) {
    glAttachShader(program, vertexShader);
    checkGlError("glAttachShader");
    glAttachShader(program, pixelShader);
    checkGlError("glAttachShader");
    glLinkProgram(program);
    GLint linkStatus = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
      GLint bufLength = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
      if (bufLength) {
        char* buf = (char*)malloc(bufLength);
        if (buf) {
          glGetProgramInfoLog(program, bufLength, NULL, buf);
          error("Could not link program:\n%s\n", buf);
          free(buf);
        }
      }
      glDeleteProgram(program);
      program = 0;
    }
  }
  return program;
}

GLuint gProgram;
GLuint gvPositionHandle;

bool surfaceChanged(int w, int h) {
  gProgram = createProgram(gVertexShader, gFragmentShader);
  if (!gProgram) {
    error("Could not create program.");
    return false;
  }
  gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
  checkGlError("glGetAttribLocation");
  info("glGetAttribLocation(\"vPosition\") = %d\n", gvPositionHandle);

  glViewport(0, 0, w, h);
  checkGlError("glViewport");
  return true;
}

const GLfloat gTriangleVertices[] = {0.0f, 0.5f, -0.5f, -0.5f, 0.5f, -0.5f};

void drawFrame() {
  static float grey;
  grey += 0.01f;
  if (grey > 1.0f) {
    grey = 0.0f;
  }
  glClearColor(grey * 0.5, grey, grey, 1.0f);
  checkGlError("glClearColor");
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
  checkGlError("glClear");

  glUseProgram(gProgram);
  checkGlError("glUseProgram");

  glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0,
                        gTriangleVertices);
  checkGlError("glVertexAttribPointer");
  glEnableVertexAttribArray(gvPositionHandle);
  checkGlError("glEnableVertexAttribArray");
  glDrawArrays(GL_TRIANGLES, 0, 3);
  checkGlError("glDrawArrays");
}

}  // namespace

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_surfaceCreated(JNIEnv* env, jobject obj) {
}

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_surfaceChanged(JNIEnv* env, jobject obj,
                                                      jint width, jint height) {
  surfaceChanged(width, height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_mortehu_helloworld_OpenGLView_drawFrame(JNIEnv* env, jobject obj) {
  drawFrame();
}
