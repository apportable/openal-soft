#ifndef __ANDROID_OPENAL_FUNCS_H__
#define __ANDROID_OPENAL_FUNCS_H__
#ifdef ANDROID
#include <jni.h>

typedef struct {
  void (*alc_android_suspend)();
  void (*alc_android_resume)();
  void (*alc_android_set_java_vm)(JavaVM*);
} AndroidOpenALFuncs;
AndroidOpenALFuncs androidOpenALFuncs;

#endif
#endif
