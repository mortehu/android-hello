package com.mortehu.helloworld;

import android.app.Activity;
import android.os.Bundle;

public class HelloWorld extends Activity {
  OpenGLView mView;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    mView = new OpenGLView(getApplication());
    setContentView(mView);
  }

  @Override
  protected void onPause() {
    super.onPause();
    mView.onPause();
  }

  @Override
  protected void onResume() {
    super.onResume();
    mView.onResume();
  }
}
