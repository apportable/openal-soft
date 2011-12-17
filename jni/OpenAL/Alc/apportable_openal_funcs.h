typedef struct {
  void (*alc_android_suspend)();
  void (*alc_android_resume)();
  void (*alc_android_set_java_vm)(JavaVM*);
#ifdef HAVE_OPENSLES
  SLEngineItf (*alc_opensles_get_native_audio_engine_engine)();
  SLEngineItf (*alc_opensles_get_native_audio_output_mix)();
  SLresult (*alc_opensles_create_native_audio_engine)();
#endif
} ApportableOpenALFuncs;
ApportableOpenALFuncs apportableOpenALFuncs;
