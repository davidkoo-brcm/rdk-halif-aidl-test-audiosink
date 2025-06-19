.PHONY: all

AUDIOSINK_AIDL_LIB_NAME := audiosinkaidl
AUDIODECODER_AIDL_LIB_NAME := audiodecoderaidl

AUDIOSINK_AIDL_LIB := lib$(AUDIOSINK_AIDL_LIB_NAME).so
AUDIODECODER_AIDL_LIB := lib$(AUDIODECODER_AIDL_LIB_NAME).so

BUILD_DIR ?= $(CURDIR)/../build

AIDL_GEN_DIR ?= $(BUILD_DIR)/audiosink_gen/current

COMMAN_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/AVSource.cpp
COMMAN_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/HALError.cpp
COMMAN_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/PropertyValue.cpp
COMMAN_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/State.cpp

AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/Capabilities.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/ErrorCode.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSinkController.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSinkControllerListener.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSink.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSinkEventListener.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSinkManager.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/PlatformCapabilities.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/Property.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/Volume.cpp
AUDIOSINK_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiosink/VolumeRamp.cpp

AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/Capabilities.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/ChannelType.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/Codec.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/CSDAudioFormat.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/ErrorCode.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/FrameMetadata.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoderController.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoderControllerListener.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoder.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoderEventListener.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoderManager.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/PCMFormat.cpp
AUDIODECODER_GEN_SRC += ${AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/Property.cpp

AUDIOSINK_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AUDIOSINK_AIDL_LIB)
AUDIODECODER_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AUDIODECODER_AIDL_LIB)

AIDL_LIB_INC_FLAGS := -I$(AIDL_GEN_DIR)/h
AIDL_LIB_INC_FLAGS += -I$(AV_BUFFER_AIDL_GEN_DIR)/h
AIDL_LIB_INC_FLAGS += -I$(VIDEO_DECODER_AIDL_GEN_DIR)/h
AIDL_LIB_INC_FLAGS += -I$(BUILD_DIR)/usr/include

AIDL_LIB_CFLAGS := -Wall -Wextra -std=c++17 -ggdb

AIDL_LIB_LDFLAGS := -L$(BUILD_DIR)/usr/lib
AIDL_LIB_LDFLAGS += -Wl,-rpath-link=$(BUILD_DIR)/usr/lib

AIDL_LIB_LD_LIBS := -lbinder -lutils

all: $(AUDIOSINK_AIDL_LIB) $(AUDIODECODER_AIDL_LIB)

$(AUDIODECODER_AIDL_LIB): $(AUDIODECODER_GEN_SRC:%.cpp=%.o) $(COMMAN_GEN_SRC:%.cpp=%.o)
	g++ -shared $(AIDL_LIB_LDFLAGS) $^ -o $@ $(AIDL_LIB_LD_LIBS)

$(AUDIOSINK_AIDL_LIB): $(AUDIODECODER_AIDL_LIB)
$(AUDIOSINK_AIDL_LIB): $(AUDIOSINK_GEN_SRC:%.cpp=%.o) $(COMMAN_GEN_SRC:%.cpp=%.o)
	g++ -shared $(AIDL_LIB_LDFLAGS) $(filter %.o, $^) -o $@ $(AIDL_LIB_LD_LIBS) \
	       	-l$(AUDIODECODER_AIDL_LIB_NAME)

# do not clash with `ut-core` by chaning the object file name to .o.1
%.o: %.cpp
	g++ $(AIDL_LIB_CFLAGS) $(AIDL_LIB_INC_FLAGS) -fPIC -c $< -o $@

