LOCAL_PATH := $(call my-dir)

ANALYZE      ?= no
ANALYZE_OUTPUT?=/dev/null

include $(CLEAR_VARS)
TARGET_ARCH_ABI  ?=armeabi-v7a
LOCAL_LDLIBS     := -llog
LOCAL_MODULE     := openal
LOCAL_ARM_MODE   := arm
CLANG_VERSION    ?= 3.1
ROOTDIR          ?= $(LOCAL_PATH)
OPENAL_DIR       := OpenAL
MODULE           := openal
MODULE_DST       := obj/local/$(TARGET_ARCH_ABI)/objs/openal
ifeq ("$(BINDIR)","")
    BINDIR       := $(abspath $(ROOTDIR)/../obj/local/$(TARGET_ARCH_ABI)/objs/ )
else
    BINDIR       := $(abspath $(BINDIR) )
endif

ANDROID_NDK_ROOT=/Developer/DestinyCloudFist/android-ndk-r8b

LOCAL_CFLAGS    +=  -I$(ROOTDIR)/$(OPENAL_DIR) \
                    -I$(ROOTDIR)/$(OPENAL_DIR)/include \
                    -I$(ROOTDIR)/$(OPENAL_DIR)/OpenAL32/Include \
                    -DAL_ALEXT_PROTOTYPES \
                    -DANDROID \
                    -fpic \
                    -ffunction-sections \
                    -funwind-tables \
                    -fstack-protector \
                    -fno-short-enums \
                    -DHAVE_GCC_VISIBILITY \
                    -O3 \
                    -g \

ifeq ($(TARGET_ARCH_ABI),x86)
LOCAL_CFLAGS    +=  \
                    -DBUILD_FOUNDATION_LIB \
                    -DTARGET_OS_android \
                    -D__POSIX_SOURCE \
                    -DURI_NO_UNICODE \
                    -DURI_ENABLE_ANSI \
                    -nostdinc \
                    -I/$(ANDROID_NDK_ROOT)/toolchains/x86-4.4.3/prebuilt/$(HOST_OS)-$(HOST_ARCH)/lib/gcc/i686-android-linux/4.4.3/include/ \
                    -I$(ANDROID_NDK_ROOT)/platforms/android-8/arch-x86/usr/include/ \

else
LOCAL_CFLAGS    +=  \
                    -DBUILD_FOUNDATION_LIB \
                    -DTARGET_OS_android \
                    -D__POSIX_SOURCE \
                    -DURI_NO_UNICODE \
                    -DURI_ENABLE_ANSI \
                    -nostdinc \
                    -I/$(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/$(HOST_OS)-$(HOST_ARCH)/lib/gcc/arm-linux-androideabi/4.4.3/include/ \
                    -I$(ANDROID_NDK_ROOT)/platforms/android-8/arch-arm/usr/include/ \

endif

LOCAL_LDLIBS    += --build-id -Bsymbolic -shared

# Default to Fixed-point math
ifeq ($(TARGET_ARCH_ABI),armeabi)
  # ARMv5, used fixed point math
  LOCAL_CFLAGS += -marm -DOPENAL_FIXED_POINT -DOPENAL_FIXED_POINT_SHIFT=16
endif

MAX_SOURCES_LOW ?= 4
MAX_SOURCES_START ?= 8
MAX_SOURCES_HIGH ?= 64
LOCAL_CFLAGS += -DMAX_SOURCES_LOW=$(MAX_SOURCES_LOW) -DMAX_SOURCES_START=$(MAX_SOURCES_START) -DMAX_SOURCES_HIGH=$(MAX_SOURCES_HIGH)

LOCAL_SRC_FILES :=  \
                    Alc/android.o              \
                    OpenAL32/alAuxEffectSlot.o \
                    OpenAL32/alBuffer.o        \
                    OpenAL32/alDatabuffer.o    \
                    OpenAL32/alEffect.o        \
                    OpenAL32/alError.o         \
                    OpenAL32/alExtension.o     \
                    OpenAL32/alFilter.o        \
                    OpenAL32/alListener.o      \
                    OpenAL32/alSource.o        \
                    OpenAL32/alState.o         \
                    OpenAL32/alThunk.o         \
                    Alc/ALc.o                  \
                    Alc/alcConfig.o            \
                    Alc/alcEcho.o              \
                    Alc/alcModulator.o         \
                    Alc/alcReverb.o            \
                    Alc/alcRing.o              \
                    Alc/alcThread.o            \
                    Alc/ALu.o                  \
                    Alc/bs2b.o                 \
                    Alc/null.o                 \
                    Alc/panning.o              \
                    Alc/mixer.o                \
                    Alc/audiotrack.o           \

OBJECTS:=$(LOCAL_SRC_FILES)

# If building for versions after FROYO
ifeq ($(POST_FROYO), yes)
  LOCAL_CFLAGS +=   -DPOST_FROYO -I$(ANDROID_NDK_ROOT)/platforms/android-9/arch-arm/usr/include/
  LOCAL_LDLIBS += -ldl -L$(ANDROID_NDK_ROOT)/platforms/android-9/arch-arm/usr/lib/
  LOCAL_SRC_FILES += Alc/opensles.o
endif

ifeq ($(TARGET_ARCH_ABI),x86)
  CC= /Developer/DestinyCloudFist/clang-$(CLANG_VERSION)/bin/clang --sysroot=$(ANDROID_NDK_ROOT)/platforms/android-8/arch-x86 $(CXX_SYSTEM) -ccc-host-triple i686-android-linux -march=i386 -D__compiler_offsetof=__builtin_offsetof
  CCAS=$(ANDROID_NDK_ROOT)/toolchains/x86-4.4.3/prebuilt/$(HOST_OS)-$(HOST_ARCH)/bin/i686-android-linux-gcc
else
ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
  CC= /Developer/DestinyCloudFist/clang-$(CLANG_VERSION)/bin/clang --sysroot=$(ANDROID_NDK_ROOT)/platforms/android-8/arch-arm $(CXX_SYSTEM) -ccc-host-triple arm-android-eabi-v7a  -march=armv7a -mfloat-abi=softfp -mfpu=vfp -D__compiler_offsetof=__builtin_offsetof
  CCAS=$(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.6/prebuilt/$(HOST_OS)-$(HOST_ARCH)/bin/arm-linux-androideabi-gcc
else
  CC= /Developer/DestinyCloudFist/clang-$(CLANG_VERSION)/bin/clang --sysroot=$(ANDROID_NDK_ROOT)/platforms/android-8/arch-arm $(CXX_SYSTEM) -ccc-host-triple arm-android-eabi -march=armv5 -D__compiler_offsetof=__builtin_offsetof
  CCAS=$(ANDROID_NDK_ROOT)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/$(HOST_OS)-$(HOST_ARCH)/bin/arm-linux-androideabi-gcc
endif
endif

OBJDIR = $(BINDIR)/$(MODULE_DST)

MODULE_CFLAGS := $(COMMON_CFLAGS) $(CFLAGS) $(LOCAL_CFLAGS)

DEBUG_LOGGING_FLAGS ?= -DDEBUG_LOG\(...\)=do\{\}while\(0\)\; -DDEBUG_BREAK\(\)=do\{\}while\(0\)\; -DRELEASE_LOG\(...\)=do\{\}while\(0\);

ifneq ("$(ANALYZE)", "yes")
# Start Compile Rules


$(OBJDIR)/%.out.s: $(ROOTDIR)/$(MODULE)/%.c
	@echo Compiling .c $<
	@mkdir -p $(dir $@)
	@$(CC)  $(MODULE_CFLAGS) $(MODULE_CCFLAGS) $(DEBUG_LOGGING_FLAGS) -S $< -o $@

$(OBJDIR)/%.fixed.s: $(OBJDIR)/%.out.s
	@echo fixing $<
	@perl fixup_assembly.pl < $< > $@

$(OBJDIR)/%.o: $(OBJDIR)/%.fixed.s
	@echo assembling $<
	@$(CCAS) $(MODULE_ASFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(ROOTDIR)/$(MODULE)/%.s
	@echo Assembling $<
	@mkdir -p $(dir $@)
	@$(CC) $(MODULE_ASFLAGS) -c $< -o $@

# End Compile Rules
else
# Start Analyze Rules

$(OBJDIR)/%.o: $(ROOTDIR)/$(MODULE)/%.c
	@echo Analyzing $<
	@mkdir -p `echo $@ | sed s/[^/]*[.]o$$//`
	@$(CC) $(MODULE_CFLAGS) $(MODULE_CCFLAGS) $(DEBUG_LOGGING_FLAGS) -S --analyze $< -o /dev/null 2>> $(ANALYZE_OUTPUT)

# End Analyze Rules
endif

include $(BUILD_SHARED_LIBRARY)

