/* -*- Mode: Java; c-basic-offset: 4; tab-width: 20; indent-tabs-mode: nil; -*-
 * ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Mozilla Android code.
 *
 * The Initial Developer of the Original Code is Mozilla Foundation.
 * Portions created by the Initial Developer are Copyright (C) 2011-2012
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Patrick Walton <pcwalton@mozilla.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#include <jni.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <cassert>
#include <cstdlib>
#include <EGL/egl.h>
#include <pthread.h>

#define NS_ASSERTION(cond, msg) assert((cond) && msg)

namespace {

JavaVM *sJVM = NULL;

template<typename T>
class AndroidEGLObject {
public:
    AndroidEGLObject(JNIEnv* aJEnv, jobject aJObj)
    : mPtr(reinterpret_cast<typename T::NativeType>(aJEnv->GetIntField(aJObj, jPointerField))) {}

    static void Init(JNIEnv* aJEnv) {
        jclass jClass = reinterpret_cast<jclass>
            (aJEnv->NewGlobalRef(aJEnv->FindClass(sClassName)));
        jPointerField = aJEnv->GetFieldID(jClass, sPointerFieldName, "I");
    }

    typename T::NativeType const& operator*() const {
        return mPtr;
    }

private:
    static jfieldID jPointerField;
    static const char* sClassName;
    static const char* sPointerFieldName;

    const typename T::NativeType mPtr;
};

class AndroidEGLDisplayInfo {
public:
    typedef EGLDisplay NativeType;
private:
    AndroidEGLDisplayInfo() {}
};

class AndroidEGLConfigInfo {
public:
    typedef EGLConfig NativeType;
private:
    AndroidEGLConfigInfo() {}
};

class AndroidEGLContextInfo {
public:
    typedef EGLContext NativeType;
private:
    AndroidEGLContextInfo() {}
};

class AndroidEGLSurfaceInfo {
public:
    typedef EGLSurface NativeType;
private:
    AndroidEGLSurfaceInfo() {}
};

typedef AndroidEGLObject<AndroidEGLDisplayInfo> AndroidEGLDisplay;
typedef AndroidEGLObject<AndroidEGLConfigInfo> AndroidEGLConfig;
typedef AndroidEGLObject<AndroidEGLContextInfo> AndroidEGLContext;
typedef AndroidEGLObject<AndroidEGLSurfaceInfo> AndroidEGLSurface;

template<>
const char *AndroidEGLDisplay::sClassName = "com/google/android/gles_jni/EGLDisplayImpl";
template<>
const char *AndroidEGLDisplay::sPointerFieldName = "mEGLDisplay";
template<>
jfieldID AndroidEGLDisplay::jPointerField = 0;
template<>
const char *AndroidEGLConfig::sClassName = "com/google/android/gles_jni/EGLConfigImpl";
template<>
const char *AndroidEGLConfig::sPointerFieldName = "mEGLConfig";
template<>
jfieldID AndroidEGLConfig::jPointerField = 0;
template<>
const char *AndroidEGLContext::sClassName = "com/google/android/gles_jni/EGLContextImpl";
template<>
const char *AndroidEGLContext::sPointerFieldName = "mEGLContext";
template<>
jfieldID AndroidEGLContext::jPointerField = 0;
template<>
const char *AndroidEGLSurface::sClassName = "com/google/android/gles_jni/EGLSurfaceImpl";
template<>
const char *AndroidEGLSurface::sPointerFieldName = "mEGLSurface";
template<>
jfieldID AndroidEGLSurface::jPointerField = 0;

class AndroidGLController {
public:
    static void Init(JNIEnv* aJEnv);

    void Acquire(JNIEnv *aJEnv, jobject aJObj);
    void Acquire(JNIEnv *aJEnv);
    void Release();

    void SetGLVersion(int aVersion);
    void InitGLContext();
    void DisposeGLContext();
    EGLDisplay GetEGLDisplay();
    EGLConfig GetEGLConfig();
    EGLContext GetEGLContext();
    EGLSurface GetEGLSurface();
    bool HasSurface();
    bool SwapBuffers();
    bool CheckForLostContext();
    void WaitForValidSurface();
    int GetWidth();
    int GetHeight();

private:
    static jmethodID jSetGLVersionMethod;
    static jmethodID jInitGLContextMethod;
    static jmethodID jDisposeGLContextMethod;
    static jmethodID jGetEGLDisplayMethod;
    static jmethodID jGetEGLConfigMethod;
    static jmethodID jGetEGLContextMethod;
    static jmethodID jGetEGLSurfaceMethod;
    static jmethodID jHasSurfaceMethod;
    static jmethodID jSwapBuffersMethod;
    static jmethodID jCheckForLostContextMethod;
    static jmethodID jWaitForValidSurfaceMethod;
    static jmethodID jGetWidthMethod;
    static jmethodID jGetHeightMethod;

    JNIEnv *mJEnv;
    jobject mJObj;
};

static AndroidGLController sController;

jmethodID AndroidGLController::jSetGLVersionMethod = 0;
jmethodID AndroidGLController::jInitGLContextMethod = 0;
jmethodID AndroidGLController::jDisposeGLContextMethod = 0;
jmethodID AndroidGLController::jGetEGLDisplayMethod = 0;
jmethodID AndroidGLController::jGetEGLConfigMethod = 0;
jmethodID AndroidGLController::jGetEGLContextMethod = 0;
jmethodID AndroidGLController::jGetEGLSurfaceMethod = 0;
jmethodID AndroidGLController::jHasSurfaceMethod = 0;
jmethodID AndroidGLController::jSwapBuffersMethod = 0;
jmethodID AndroidGLController::jCheckForLostContextMethod = 0;
jmethodID AndroidGLController::jWaitForValidSurfaceMethod = 0;
jmethodID AndroidGLController::jGetWidthMethod = 0;
jmethodID AndroidGLController::jGetHeightMethod = 0;

void
AndroidGLController::Init(JNIEnv *aJEnv)
{
    const char *className = "org/mozilla/testnewglsurfaceview/GLController";
    jclass jClass = reinterpret_cast<jclass>(aJEnv->NewGlobalRef(aJEnv->FindClass(className)));

    jSetGLVersionMethod = aJEnv->GetMethodID(jClass, "setGLVersion", "(I)V");
    jInitGLContextMethod = aJEnv->GetMethodID(jClass, "initGLContext", "()V");
    jDisposeGLContextMethod = aJEnv->GetMethodID(jClass, "disposeGLContext", "()V");
    jGetEGLDisplayMethod = aJEnv->GetMethodID(jClass, "getEGLDisplay",
                                              "()Ljavax/microedition/khronos/egl/EGLDisplay;");
    jGetEGLConfigMethod = aJEnv->GetMethodID(jClass, "getEGLConfig",
                                             "()Ljavax/microedition/khronos/egl/EGLConfig;");
    jGetEGLContextMethod = aJEnv->GetMethodID(jClass, "getEGLContext",
                                              "()Ljavax/microedition/khronos/egl/EGLContext;");
    jGetEGLSurfaceMethod = aJEnv->GetMethodID(jClass, "getEGLSurface",
                                              "()Ljavax/microedition/khronos/egl/EGLSurface;");
    jHasSurfaceMethod = aJEnv->GetMethodID(jClass, "hasSurface", "()Z");
    jSwapBuffersMethod = aJEnv->GetMethodID(jClass, "swapBuffers", "()Z");
    jCheckForLostContextMethod = aJEnv->GetMethodID(jClass, "checkForLostContext", "()Z");
    jWaitForValidSurfaceMethod = aJEnv->GetMethodID(jClass, "waitForValidSurface", "()V");
    jGetWidthMethod = aJEnv->GetMethodID(jClass, "getWidth", "()I");
    jGetHeightMethod = aJEnv->GetMethodID(jClass, "getHeight", "()I");
}

void
AndroidGLController::Acquire(JNIEnv* aJEnv, jobject aJObj)
{
    mJEnv = aJEnv;
    mJObj = aJEnv->NewGlobalRef(aJObj);
}

void
AndroidGLController::Acquire(JNIEnv* aJEnv)
{
    mJEnv = aJEnv;
}

void
AndroidGLController::Release()
{
    if (mJObj) {
        mJEnv->DeleteGlobalRef(mJObj);
        mJObj = NULL;
    }

    mJEnv = NULL;
}

void
AndroidGLController::SetGLVersion(int aVersion)
{
    mJEnv->CallVoidMethod(mJObj, jSetGLVersionMethod, aVersion);
}

void
AndroidGLController::InitGLContext()
{
    mJEnv->CallVoidMethod(mJObj, jInitGLContextMethod);
}

void
AndroidGLController::DisposeGLContext()
{
    mJEnv->CallVoidMethod(mJObj, jDisposeGLContextMethod);
}

EGLDisplay
AndroidGLController::GetEGLDisplay()
{
    AndroidEGLDisplay jEGLDisplay(mJEnv, mJEnv->CallObjectMethod(mJObj, jGetEGLDisplayMethod));
    return *jEGLDisplay;
}

EGLConfig
AndroidGLController::GetEGLConfig()
{
    AndroidEGLConfig jEGLConfig(mJEnv, mJEnv->CallObjectMethod(mJObj, jGetEGLConfigMethod));
    return *jEGLConfig;
}

EGLContext
AndroidGLController::GetEGLContext()
{
    AndroidEGLContext jEGLContext(mJEnv, mJEnv->CallObjectMethod(mJObj, jGetEGLContextMethod));
    return *jEGLContext;
}

EGLSurface
AndroidGLController::GetEGLSurface()
{
    AndroidEGLSurface jEGLSurface(mJEnv, mJEnv->CallObjectMethod(mJObj, jGetEGLSurfaceMethod));
    return *jEGLSurface;
}

bool
AndroidGLController::HasSurface()
{
    return mJEnv->CallBooleanMethod(mJObj, jHasSurfaceMethod);
}

bool
AndroidGLController::SwapBuffers()
{
    return mJEnv->CallBooleanMethod(mJObj, jSwapBuffersMethod);
}

bool
AndroidGLController::CheckForLostContext()
{
    return mJEnv->CallBooleanMethod(mJObj, jCheckForLostContextMethod);
}

void
AndroidGLController::WaitForValidSurface()
{
    mJEnv->CallVoidMethod(mJObj, jWaitForValidSurfaceMethod);
}

int
AndroidGLController::GetWidth()
{
    return mJEnv->CallIntMethod(mJObj, jGetWidthMethod);
}

int
AndroidGLController::GetHeight()
{
    return mJEnv->CallIntMethod(mJObj, jGetHeightMethod);
}

void*
start(void* userdata)
{
    JNIEnv *jEnv;
    sJVM->AttachCurrentThread(&jEnv, NULL);
    sController.Acquire(jEnv);

	float vertices[] = { 0.0f, 1.0f, 0.0f, 	//Top
					  	-1.0f, -1.0f, 0.0f, //Bottom Left
						 1.0f, -1.0f, 0.0f 	//Bottom Right
    };
    const int numVertices = 9;
	
	float colors[] = {
        1.0f, 0.0f, 0.0f, 1.0f, //Set The Color To Red, last value 100% luminance
		0.0f, 1.0f, 0.0f, 1.0f, //Set The Color To Green, last value 100% luminance
		0.0f, 0.0f, 1.0f, 1.0f 	//Set The Color To Blue, last value 100% luminance
	};
    const float numColors = 12;

    sController.WaitForValidSurface();
    sController.SetGLVersion(1);
    sController.InitGLContext();

    if (jEnv->ExceptionCheck()) {
        jEnv->ExceptionDescribe();
        jEnv->ExceptionClear();
    }

    float currentScale = 1.0f, scaleDelta = -0.01f;

    while (true) {
        sController.WaitForValidSurface();

        glViewport(0, 0, sController.GetWidth(), sController.GetHeight());
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        currentScale += scaleDelta;
        if (scaleDelta < 0.0f && currentScale < -1.0f) {
            currentScale = -1.0f;
            scaleDelta = -scaleDelta;
        } else if (scaleDelta > 0.0f && currentScale > 1.0f) {
            currentScale = 1.0f;
            scaleDelta = -scaleDelta;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Set the face rotation
		glFrontFace(GL_CW);
		
		//Point to our buffers
		glVertexPointer(3, GL_FLOAT, 0, vertices);
		glColorPointer(4, GL_FLOAT, 0, colors);
		
		//Enable the vertex and color state
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);

        glLoadIdentity();
        glScalef(currentScale, 1.0f, 1.0f);
		
		//Draw the vertices as triangles
		glDrawArrays(GL_TRIANGLES, 0, numVertices / 3);
		
		//Disable the client state before leaving
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

        sController.SwapBuffers();
    }

    return NULL;
}

}   // end anonymous namespace

#define TestNewGLSurfaceView_start  \
    Java_org_mozilla_testnewglsurfaceview_TestNewGLSurfaceView_start

extern "C" {

JNIEXPORT void JNICALL
TestNewGLSurfaceView_start(JNIEnv* aJEnv, jobject aSelf, jobject aController)
{
    aJEnv->GetJavaVM(&sJVM);

    AndroidEGLDisplay::Init(aJEnv);
    AndroidEGLConfig::Init(aJEnv);
    AndroidEGLContext::Init(aJEnv);
    AndroidEGLSurface::Init(aJEnv);
    AndroidGLController::Init(aJEnv);

    sController.Acquire(aJEnv, aController);

    pthread_t thread;
    pthread_create(&thread, NULL, start, aJEnv);
}

}   // end extern "C"

