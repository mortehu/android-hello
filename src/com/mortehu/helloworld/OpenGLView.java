package com.mortehu.helloworld;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

class OpenGLView extends GLSurfaceView {
  static {
    System.loadLibrary("hello-world");
  }

  // Constants missing in EGL10.
  private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;
  private static int EGL_OPENGL_ES2_BIT = 4;

  public static native void surfaceCreated();
  public static native void surfaceChanged(int width, int height);
  public static native void drawFrame();
  public static native void touchEvent(float x, float y, int state);

  private static class ContextFactory implements GLSurfaceView.EGLContextFactory {
    public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig eglConfig) {
      int[] attrib_list = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE};
      EGLContext context =
          egl.eglCreateContext(display, eglConfig, EGL10.EGL_NO_CONTEXT, attrib_list);
      logEglErrors(egl);
      return context;
    }

    public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
      egl.eglDestroyContext(display, context);
    }
  }

  private static class ConfigChooser implements GLSurfaceView.EGLConfigChooser {
    private static final int[] ATTRIB_LIST = {
      EGL10.EGL_RED_SIZE, 4,
      EGL10.EGL_GREEN_SIZE, 4,
      EGL10.EGL_BLUE_SIZE, 4,
      EGL10.EGL_DEPTH_SIZE, 16,
      EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
      EGL10.EGL_NONE};

    public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
      int[] num_config = new int[1];
      EGLConfig[] configs = new EGLConfig[1];
      egl.eglChooseConfig(display, ConfigChooser.ATTRIB_LIST, configs, 1, num_config);

      return configs[0];
    }
  }

  private static class Renderer implements GLSurfaceView.Renderer {
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
      OpenGLView.surfaceCreated();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
      OpenGLView.surfaceChanged(width, height);
    }

    public void onDrawFrame(GL10 gl) {
      OpenGLView.drawFrame();
    }
  }


  public OpenGLView(Context context) {
    super(context);

    setEGLContextFactory(new ContextFactory());
    setEGLConfigChooser(new ConfigChooser());
    setRenderer(new Renderer());
  }

  @Override
  public boolean onTouchEvent(MotionEvent e) {
    float x = e.getX();
    float y = e.getY();

    switch (e.getAction()) {
      case MotionEvent.ACTION_DOWN:
        touchEvent(x, y, 0);
        break;

      case MotionEvent.ACTION_MOVE:
        touchEvent(x, y, 1);
        break;

      case MotionEvent.ACTION_UP:
        touchEvent(x, y, 2);
        break;
    }

    return true;
  }

  private static void logEglErrors(EGL10 egl) {
    int error;
    while (EGL10.EGL_SUCCESS != (error = egl.eglGetError()))
      Log.e("hello-world", String.format("EGL error: %d", error));
  }
}
