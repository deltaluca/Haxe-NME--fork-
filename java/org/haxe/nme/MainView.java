/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package org.haxe.nme;

import android.content.Context;
import android.graphics.PixelFormat;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.app.Activity;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.opengles.GL10;

/**
 * A simple GLSurfaceView sub-class that demonstrate how to perform
 * OpenGL ES 2.0 rendering into a GL Surface. Note the following important
 * details:
 *
 * - The class must use a custom context factory to enable 2.0 rendering.
 *   See ContextFactory class definition below.
 *
 * - The class must use a custom EGLConfigChooser to be able to select
 *   an EGLConfig that supports 2.0. This is done by providing a config
 *   specification to eglChooseConfig() that has the attribute
 *   EGL10.ELG_RENDERABLE_TYPE containing the EGL_OPENGL_ES2_BIT flag
 *   set. See ConfigChooser class definition below.
 *
 * - The class must select the surface's format, then choose an EGLConfig
 *   that matches it exactly (with regards to red/green/blue/alpha channels
 *   bit depths). Failure to do so would result in an EGL_BAD_MATCH error.
 */
class MainView extends GLSurfaceView {

    Activity mActivity;
    public MainView(Context context,Activity inActivity) {
        super(context);
        mActivity = inActivity;
        setFocusable(true);
        setFocusableInTouchMode(true);
        setRenderer(new Renderer(this));
    }

   static final int etTouchBegin = 15;
   static final int etTouchMove  = 16;
   static final int etTouchEnd   = 17;
   static final int etTouchTap   = 18;

   static final int resTerminate = -1;

   public void HandleResult(int inCode) {
       if (inCode==resTerminate)
       {
          //Log.v("VIEW","Terminate Request.");
          mActivity.finish();
          return;
       }
   }

   @Override
   public boolean onTouchEvent(final MotionEvent ev) {
       final MainView me = this;
	   queueEvent(new Runnable(){
            public void run() {

               final int action = ev.getAction();

               int type = -1;

               switch (action & MotionEvent.ACTION_MASK) {
                  case MotionEvent.ACTION_DOWN: type = etTouchBegin; break;
                  case MotionEvent.ACTION_MOVE: type = etTouchMove; break;
                  case MotionEvent.ACTION_UP: type = etTouchEnd; break;
                  case MotionEvent.ACTION_CANCEL: type = etTouchEnd; break;
               }

               for (int i = 0; i < ev.getPointerCount(); i++) {
                  int id = ev.getPointerId(i);
                  float x = ev.getX(i);
                  float y = ev.getY(i);
                  me.HandleResult( NME.onTouch(type,x,y,id) );
               }

      }});
      return true;
    }


    @Override
    public boolean onTrackballEvent(final MotionEvent ev) {
       final MainView me = this;
       queueEvent(new Runnable(){
          public void run() {
          float x = ev.getX();
          float y = ev.getY();
          me.HandleResult( NME.onTrackball(x,y) );
       }});
       return false;
    }

    public int translateKey(int inCode) {
       switch(inCode)
       {
          case KeyEvent.KEYCODE_DPAD_CENTER: return 13; /* Fake ENTER */
          case KeyEvent.KEYCODE_DPAD_LEFT: return 37;
          case KeyEvent.KEYCODE_DPAD_RIGHT: return 39;
          case KeyEvent.KEYCODE_DPAD_UP: return 38;
          case KeyEvent.KEYCODE_DPAD_DOWN: return 40;
          case KeyEvent.KEYCODE_BACK: return 27; /* Fake Escape */
       }

       return 0;
    }

    @Override
    public boolean onKeyDown(final int inKeyCode, KeyEvent event) {
         //Log.v("VIEW","onKeyDown " + inKeyCode);
         final MainView me = this;
         final int keyCode = translateKey(inKeyCode);
         if (keyCode!=0) {
             queueEvent(new Runnable() {
                 // This method will be called on the rendering thread:
                 public void run() {
                     me.HandleResult(NME.onKeyChange(keyCode,true));
                 }});
             return true;
         }
         return super.onKeyDown(inKeyCode, event);
     }


    @Override
    public boolean onKeyUp(final int inKeyCode, KeyEvent event) {
         //Log.v("VIEW","onKeyUp " + inKeyCode);
         final MainView me = this;
         final int keyCode = translateKey(inKeyCode);
         if (keyCode!=0) {
             queueEvent(new Runnable() {
                 // This method will be called on the rendering thread:
                 public void run() {
                     me.HandleResult(NME.onKeyChange(keyCode,false));
                 }});
             return true;
         }
         return super.onKeyDown(inKeyCode, event);
     }


    private static class Renderer implements GLSurfaceView.Renderer {
        MainView mMainView;

        public Renderer(MainView inView) { mMainView = inView; }

        public void onDrawFrame(GL10 gl) {
            //Log.v("VIEW","onDrawFrame !");
            mMainView.HandleResult( NME.onRender() );
            //Log.v("VIEW","onDrawFrame DONE!");
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            //Log.v("VIEW","onSurfaceChanged " + width +"," + height);
            mMainView.HandleResult( NME.onResize(width,height) );
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            // Do nothing.
        }
    }
}



