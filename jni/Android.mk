LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

OPENAL_DIR := OpenAL

LOCAL_LDLIBS    := -llog
LOCAL_MODULE    := openal
LOCAL_ARM_MODE  := arm
LOCAL_CFLAGS    +=  -I$(OPENAL_DIR) \
                    -I$(OPENAL_DIR)/include \
                    -I$(OPENAL_DIR)/OpenAL32/Include \
                    -DAL_ALEXT_PROTOTYPES \
                    -DANDROID \
                     -shared -Wl,-Bsymbolic  \
				    -fpic \
				    -ffunction-sections \
				    -funwind-tables \
				    -fstack-protector \
				    -fno-short-enums \
                    -DHAVE_GCC_VISIBILITY \
					-g \

					# -fpic \
					# 				    -ffunction-sections \
					# 				    -funwind-tables \
					# 				    -fstack-protector \
					# 				    -fno-short-enums \
					# 				    -D__ARM_ARCH_5__ \
					# 				    -D__ANDROID__  \
					# 				    -march=armv5 \
					# 				    -msoft-float \


#    -DVERDE_USE_REAL_FILE_IO \

# FIXME
#LOCAL_CFLAGS    +=  -I/Developer/AndroidNDK/platforms/android-8/arch-arm/usr/include

# Default to Fixed-point math
LOCAL_CFLAGS    +=  -DOPENAL_FIXED_POINT -DOPENAL_FIXED_POINT_SHIFT=16

LOCAL_SRC_FILES :=  \
                    $(OPENAL_DIR)/Alc/android.c              \
                    $(OPENAL_DIR)/OpenAL32/alAuxEffectSlot.c \
                    $(OPENAL_DIR)/OpenAL32/alBuffer.c        \
                    $(OPENAL_DIR)/OpenAL32/alDatabuffer.c    \
                    $(OPENAL_DIR)/OpenAL32/alEffect.c        \
                    $(OPENAL_DIR)/OpenAL32/alError.c         \
                    $(OPENAL_DIR)/OpenAL32/alExtension.c     \
                    $(OPENAL_DIR)/OpenAL32/alFilter.c        \
                    $(OPENAL_DIR)/OpenAL32/alListener.c      \
                    $(OPENAL_DIR)/OpenAL32/alSource.c        \
                    $(OPENAL_DIR)/OpenAL32/alState.c         \
                    $(OPENAL_DIR)/OpenAL32/alThunk.c         \
                    $(OPENAL_DIR)/Alc/ALc.c                  \
                    $(OPENAL_DIR)/Alc/alcConfig.c            \
                    $(OPENAL_DIR)/Alc/alcEcho.c              \
                    $(OPENAL_DIR)/Alc/alcModulator.c         \
                    $(OPENAL_DIR)/Alc/alcReverb.c            \
                    $(OPENAL_DIR)/Alc/alcRing.c              \
                    $(OPENAL_DIR)/Alc/alcThread.c            \
                    $(OPENAL_DIR)/Alc/ALu.c                  \
                    $(OPENAL_DIR)/Alc/bs2b.c                 \
                    $(OPENAL_DIR)/Alc/null.c                 \
                    $(OPENAL_DIR)/Alc/panning.c              \
                    $(OPENAL_DIR)/Alc/mixer.c                \
                    $(OPENAL_DIR)/Alc/audiotrack.c           \
                    

# If building for versions after FROYO
#LOCAL_CFLAGS    += -DPOST_FROYO
#LOCAL_SRC_FILES += $(OPENAL_DIR)/Alc/opensles.o

include $(BUILD_SHARED_LIBRARY)


