package org.mozilla.testnewglsurfaceview;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class TestNewGLSurfaceView extends Activity implements GLSurfaceView.Renderer
{
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        FlexibleGLSurfaceView glSurfaceView = new FlexibleGLSurfaceView(this);
        glSurfaceView.setRenderer(this);
        setContentView(glSurfaceView);
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        gl.glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    }

    public void onDrawFrame(GL10 gl) {
        gl.glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        gl.glViewport(0, 0, width, height);
        gl.glMatrixMode(GL10.GL_PROJECTION);
        gl.glLoadIdentity();
    }
}
