/*
 * Copyright (C) 2011 The Android Open Source Project
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

/* This is an OpenAL backend for Android using the native audio APIs based on OpenSL ES 1.0.1.
 * It is based on source code for the native-audio sample app bundled with NDK.
 */

#include <stdlib.h>
#include <time.h>
#include "alMain.h"
#include "AL/al.h"
#include "AL/alc.h"

#include "verde/verde_helpers.h"

#define LOG_NDEBUG 0
#define LOG_TAG "OpenAL"

// for __android_log_print(ANDROID_LOG_INFO, "YourApp", "formatted message");
#if 1
#define LOGV(...) VERDE_DEBUG(__VA_ARGS__)
#endif

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "apportable_openal_funcs.h"

// engine interfaces
static SLObjectItf engineObject = NULL;
static SLEngineItf engineEngine;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

// this callback handler is called every time a buffer finishes playing
static void opensles_callback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    // LOGV("opensles_callback");
#define bufferSize (1024*4)
    static char buffer0[bufferSize], buffer1[bufferSize];
    static char * const buffers[2] = {buffer0, buffer1};
    static unsigned bufferIndex = 0;
#if 0
    static unsigned nextCount = ~0;
    static char nextBuffer[1024] = "Hello, world!";
    static unsigned nextSize = 1024;
#endif

    assert(bq == bqPlayerBufferQueue);
    assert(NULL != context);
    ALCdevice *pDevice = (ALCdevice *) context;
    ALint frameSize = FrameSizeFromDevFmt(pDevice->FmtChans, pDevice->FmtType);
    void *buffer = buffers[bufferIndex];
    // LOGV("bq=%p, pDevice=%p, frameSize=%u, buffer=%p, bufferIndex=%u, bufferSize=%u", bq, pDevice, frameSize, buffer, bufferIndex, bufferSize);
    bufferIndex ^= 1;
    // LOGV("aluMixData");
    
    aluMixData(pDevice, buffer, bufferSize/frameSize);
    
    // LOGV("Enqueue2");
#if 0
    // for streaming playback, replace this test by logic to find and fill the next buffer
    if (--nextCount > 0 && NULL != nextBuffer && 0 != nextSize) {
        SLresult result;
        // enqueue another buffer
        result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, nextBuffer, nextSize);
        // the most likely other result is SL_RESULT_BUFFER_INSUFFICIENT,
        // which for this code example would indicate a programming error
        assert(SL_RESULT_SUCCESS == result);
    }
#else
    SLresult result;
    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, buffer, bufferSize);
    assert(SL_RESULT_SUCCESS == result);
#endif
}


static const ALCchar opensles_device[] = "OpenSL ES";

// Apportable extensions
SLEngineItf alc_opensles_get_native_audio_engine_engine()
{
    return engineEngine;
}

SLEngineItf alc_opensles_get_native_audio_output_mix()
{
    return outputMixObject;
}

SLresult alc_opensles_create_native_audio_engine()
{
    if (engineObject)
        return SL_RESULT_SUCCESS;

    SLresult result;

    // create engine
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the engine
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    assert(SL_RESULT_SUCCESS == result);

    // create output mix
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == result);

    // realize the output mix
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    return result;
}

// Backend functions, in same order as type BackendFuncs
static ALCboolean opensles_open_playback(ALCdevice *pDevice, const ALCchar *deviceName)
{
    LOGV("opensles_open_playback pDevice=%p, deviceName=%s", pDevice, deviceName);

    // create the engine and output mix objects
    SLresult result = alc_opensles_create_native_audio_engine();

    // create buffer queue audio player

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};
    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 2, SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT|SL_SPEAKER_FRONT_RIGHT, SL_BYTEORDER_LITTLEENDIAN};
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // create audio player
    LOGV("create audio player");
    const SLInterfaceID ids[1] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE};
    const SLboolean req[1] = {SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
        1, ids, req);
    assert(SL_RESULT_SUCCESS == result);

    // realize the player
    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == result);

    // get the play interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(SL_RESULT_SUCCESS == result);

    // get the buffer queue interface
    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);

    // register callback on the buffer queue
    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, opensles_callback, (void *) pDevice);
    assert(SL_RESULT_SUCCESS == result);

    // set the player's state to playing
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);

    // enqueue the first buffer to kick off the callbacks
    LOGV("enqueue");
    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, "\0", 1);
    assert(SL_RESULT_SUCCESS == result);

    return ALC_TRUE;
}


static void opensles_close_playback(ALCdevice *pDevice)
{
    LOGV("opensles_close_playback pDevice=%p", pDevice);

    // shut down the native audio system

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL) {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
    }

    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL) {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }

    // destroy engine object, and invalidate all associated interfaces
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }

}

static ALCboolean opensles_reset_playback(ALCdevice *pDevice)
{
    LOGV("opensles_reset_playback pDevice=%p", pDevice);
    unsigned bits = BytesFromDevFmt(pDevice->FmtType) * 8;
    unsigned channels = ChannelsFromDevFmt(pDevice->FmtChans);
    unsigned samples = pDevice->UpdateSize;
    unsigned size = samples * channels * bits / 8;
    LOGV("bits=%u, channels=%u, samples=%u, size=%u, freq=%u", bits, channels, samples, size, pDevice->Frequency);
    SetDefaultWFXChannelOrder(pDevice);

#if 0
    if (pSDL_WasInit(SDL_INIT_AUDIO))
    {
        pSDL_PauseAudio(1);
        pSDL_CloseAudio();
        pSDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    switch (aluBytesFromFormat(device->Format))
    {
        case 1:
            desired.format = AUDIO_U8;
            break;
        case 4:
            switch (ChannelsFromDevFormat(device->DevFmt))
            {
                case 1: device->Format = AL_FORMAT_MONO16; break;
                case 2: device->Format = AL_FORMAT_STEREO16; break;
                case 4: device->Format = AL_FORMAT_QUAD16; break;
                case 6: device->Format = AL_FORMAT_51CHN16; break;
                case 7: device->Format = AL_FORMAT_61CHN16; break;
                case 8: device->Format = AL_FORMAT_71CHN16; break;
            }
            /* fall-through */
        case 2:
            desired.format = AUDIO_S16;
            break;
        default:
            AL_PRINT("Unknown format: 0x%x\n", device->Format);
            return ALC_FALSE;
    }

	desired.freq = device->Frequency;
	desired.channels = aluChannelsFromFormat(device->Format);
	desired.samples = device->UpdateSize * desired.channels;
	desired.callback = opensles_callback;
	desired.userdata = device;

    device->NumUpdates = 2;

    if (pSDL_InitSubSystem(SDL_INIT_AUDIO) == -1)
    {
        AL_PRINT("SDL_InitSubSystem(SDL_INIT_AUDIO) failed: %s\n", pSDL_GetError());
        return ALC_FALSE;
    }

    if (pSDL_OpenAudio(&desired, NULL) == -1)
    {
        AL_PRINT("SDL_OpenAudio failed: %s\n", pSDL_GetError());
        pSDL_QuitSubSystem(SDL_INIT_AUDIO);
        return ALC_FALSE;
    }

    pSDL_PauseAudio(0);
#endif

    return ALC_TRUE;
}


static void opensles_stop_playback(ALCdevice *pDevice)
{
    LOGV("opensles_stop_playback device=%p", pDevice);
}

static ALCboolean opensles_open_capture(ALCdevice *pDevice, const ALCchar *deviceName)
{
    LOGV("opensles_open_capture  device=%p, deviceName=%s", pDevice, deviceName);
    return ALC_FALSE;
}

static void opensles_close_capture(ALCdevice *pDevice)
{
    LOGV("opensles_closed_capture device=%p", pDevice);
}

static void opensles_start_capture(ALCdevice *pDevice)
{
    LOGV("opensles_start_capture device=%p", pDevice);
}

static void opensles_stop_capture(ALCdevice *pDevice)
{
    LOGV("opensles_stop_capture device=%p", pDevice);
}

static void opensles_capture_samples(ALCdevice *pDevice, ALCvoid *pBuffer, ALCuint lSamples)
{
    LOGV("opensles_capture_samples device=%p, pBuffer=%p, lSamples=%u", pDevice, pBuffer, lSamples);
}

static ALCuint opensles_available_samples(ALCdevice *pDevice)
{
    LOGV("opensles_available_samples device=%p", pDevice);
    return 0;
}

// table of backend function pointers

BackendFuncs opensles_funcs = {
    opensles_open_playback,
    opensles_close_playback,
    opensles_reset_playback,
    opensles_stop_playback,
    opensles_open_capture,
    opensles_close_capture,
    opensles_start_capture,
    opensles_stop_capture,
    opensles_capture_samples,
    opensles_available_samples
};

// global entry points called from XYZZY


void alc_opensles_suspend()
{
  SLresult result;

  if (bqPlayerPlay) {
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PAUSED);
    assert(SL_RESULT_SUCCESS == result);
    result = (*bqPlayerBufferQueue)->Clear(bqPlayerBufferQueue);
    assert(SL_RESULT_SUCCESS == result);
  }
}

void alc_opensles_resume()
{
  SLresult result;

  if (bqPlayerPlay) {
    result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    // Pump some blank data into the buffer to stimulate the callback
    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, "\0", 1);
    assert(SL_RESULT_SUCCESS == result);
  }
}

void alc_opensles_init(BackendFuncs *func_list)
{
    LOGV("alc_opensles_init");
    *func_list = opensles_funcs;
    apportableOpenALFuncs.alc_android_suspend = alc_opensles_suspend;
    apportableOpenALFuncs.alc_android_resume = alc_opensles_resume;
    apportableOpenALFuncs.alc_opensles_get_native_audio_engine_engine = alc_opensles_get_native_audio_engine_engine;
    apportableOpenALFuncs.alc_opensles_get_native_audio_output_mix = alc_opensles_get_native_audio_output_mix;
    apportableOpenALFuncs.alc_opensles_create_native_audio_engine = alc_opensles_create_native_audio_engine;
}

void alc_opensles_deinit(void)
{
    LOGV("alc_opensles_deinit");
}

void alc_opensles_probe(int type)
{
    switch (type) {
    case DEVICE_PROBE:
        LOGV("alc_opensles_probe DEVICE_PROBE");
        AppendDeviceList(opensles_device);
        break;
    case ALL_DEVICE_PROBE:
        LOGV("alc_opensles_probe ALL_DEVICE_PROBE");
        AppendAllDeviceList(opensles_device);
        break;
    default:
        LOGV("alc_opensles_probe type=%d", type);
        break;
    }
}
