#ifdef ANDROID
#include <jni.h>
#include "alMain.h"
#include "apportable_openal_funcs.h"

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
	BackendFuncs func_list;
	alc_audiotrack_init(&func_list);
	apportableOpenALFuncs.alc_android_set_java_vm(vm);
	return JNI_VERSION_1_4;
}

ALC_API void ALC_APIENTRY alcSuspend(void) {
	if (apportableOpenALFuncs.alc_android_suspend) {
		apportableOpenALFuncs.alc_android_suspend();
	}
}

ALC_API void ALC_APIENTRY alcResume(void) {
	if (apportableOpenALFuncs.alc_android_resume) {
		apportableOpenALFuncs.alc_android_resume();
	}
}

#endif