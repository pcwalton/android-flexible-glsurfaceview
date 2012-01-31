package org.mozilla.testnewglsurfaceview;

import android.app.Activity;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class TestNewGLSurfaceView extends Activity implements GLSurfaceView.Renderer
{
    private FlexibleGLSurfaceView mGLSurfaceView;

	/** The buffer holding the vertices */
	private FloatBuffer vertexBuffer;
	/** The buffer holding the colors */
	private FloatBuffer colorBuffer;
	
	/** The initial vertex definition */
	private float vertices[] = { 
								0.0f, 1.0f, 0.0f, 	//Top
								-1.0f, -1.0f, 0.0f, //Bottom Left
								1.0f, -1.0f, 0.0f 	//Bottom Right
												};
	
	/** The initial color definition */
	private float colors[] = {
		    					1.0f, 0.0f, 0.0f, 1.0f, //Set The Color To Red, last value 100% luminance
		    					0.0f, 1.0f, 0.0f, 1.0f, //Set The Color To Green, last value 100% luminance
		    					0.0f, 0.0f, 1.0f, 1.0f 	//Set The Color To Blue, last value 100% luminance
					    							};

    private float mCurrentScale;
    private float mScaleDelta;

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

		ByteBuffer byteBuf = ByteBuffer.allocateDirect(vertices.length * 4);
		byteBuf.order(ByteOrder.nativeOrder());
		vertexBuffer = byteBuf.asFloatBuffer();
		vertexBuffer.put(vertices);
		vertexBuffer.position(0);
		
		byteBuf = ByteBuffer.allocateDirect(colors.length * 4);
		byteBuf.order(ByteOrder.nativeOrder());
		colorBuffer = byteBuf.asFloatBuffer();
		colorBuffer.put(colors);
		colorBuffer.position(0);

        mCurrentScale = 1.0f;
        mScaleDelta = -0.01f;

        mGLSurfaceView = new FlexibleGLSurfaceView(this);
        mGLSurfaceView.setRenderer(this);
        mGLSurfaceView.createGLThread();
        setContentView(mGLSurfaceView);
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        gl.glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    }

    public void onDrawFrame(GL10 gl) {
        mCurrentScale += mScaleDelta;
        if (mScaleDelta < 0.0f && mCurrentScale < -1.0f) {
            mCurrentScale = -1.0f;
            mScaleDelta = -mScaleDelta;
        } else if (mScaleDelta > 0.0f && mCurrentScale > 1.0f) {
            mCurrentScale = 1.0f;
            mScaleDelta = -mScaleDelta;
        }

        gl.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);

		//Set the face rotation
		gl.glFrontFace(GL10.GL_CW);
		
		//Point to our buffers
		gl.glVertexPointer(3, GL10.GL_FLOAT, 0, vertexBuffer);
		gl.glColorPointer(4, GL10.GL_FLOAT, 0, colorBuffer);
		
		//Enable the vertex and color state
		gl.glEnableClientState(GL10.GL_VERTEX_ARRAY);
		gl.glEnableClientState(GL10.GL_COLOR_ARRAY);

        gl.glLoadIdentity();
        gl.glScalef(mCurrentScale, 1.0f, 1.0f);
		
		//Draw the vertices as triangles
		gl.glDrawArrays(GL10.GL_TRIANGLES, 0, vertices.length / 3);
		
		//Disable the client state before leaving
		gl.glDisableClientState(GL10.GL_VERTEX_ARRAY);
		gl.glDisableClientState(GL10.GL_COLOR_ARRAY);

        mGLSurfaceView.requestRender();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        gl.glViewport(0, 0, width, height);
        gl.glMatrixMode(GL10.GL_PROJECTION);
        gl.glLoadIdentity();

        mGLSurfaceView.requestRender();
    }
}
