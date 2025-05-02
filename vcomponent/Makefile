.PHONY: all


AUDIOSINK_AIDL_LIB_NAME := audiosinkaidl
AUDIODECODER_AIDL_LIB_NAME := audiodecoderaidl

EMULATE_SERVICE_BIN := EmulateAudioSinkService
AUDIOSINK_AIDL_LIB := lib$(AUDIOSINK_AIDL_LIB_NAME).so
AUDIODECODER_AIDL_LIB := lib$(AUDIODECODER_AIDL_LIB_NAME).so

EMULATE_SERVICE_DIR ?= $(CURDIR)/src

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


EMULATE_SERVICE_BIN_SRC := $(EMULATE_SERVICE_DIR)/AudioSink.cpp
EMULATE_SERVICE_BIN_SRC += $(EMULATE_SERVICE_DIR)/AudioSinkService.cpp


# FIXME: to be compile out of source tree

EMULATE_SERVICE_INC_FLAGS := -I$(AIDL_GEN_DIR)/h
EMULATE_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/usr/include
EMULATE_SERVICE_INC_FLAGS += -I$(EMULATE_SERVICE_DIR)

EMULATE_SERVICE_LD_LIBS := -lbinder -lutils

EMULATE_SERVICE_LDFLAGS := -L$(BUILD_DIR)/usr/lib
EMULATE_SERVICE_LDFLAGS += -Wl,-rpath-link=$(BUILD_DIR)/usr/lib



EMULATE_SERVICE_CFLAGS := -Wall -Wextra -std=c++17 -ggdb


AUDIOSINK_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AUDIOSINK_AIDL_LIB)
AUDIODECODER_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AUDIODECODER_AIDL_LIB)
EMULATE_SERVICE_BIN := $(BUILD_DIR)/usr/lib/$(EMULATE_SERVICE_BIN)



all: $(EMULATE_SERVICE_BIN) $(AUDIOSINK_AIDL_LIB) $(AUDIODECODER_AIDL_LIB)


$(AUDIODECODER_AIDL_LIB): $(AUDIODECODER_GEN_SRC:%.cpp=%.o) $(COMMAN_GEN_SRC:%.cpp=%.o)
	g++ -shared $(EMULATE_SERVICE_LDFLAGS) $^ -o $@ $(EMULATE_SERVICE_LD_LIBS)

$(AUDIOSINK_AIDL_LIB): $(AUDIODECODER_AIDL_LIB) 
$(AUDIOSINK_AIDL_LIB): $(AUDIOSINK_GEN_SRC:%.cpp=%.o) $(COMMAN_GEN_SRC:%.cpp=%.o) 
	g++ -shared $(EMULATE_SERVICE_LDFLAGS) $(filter %.o, $^) -o $@ $(EMULATE_SERVICE_LD_LIBS) \
	       	-l$(AUDIODECODER_AIDL_LIB_NAME)
	


$(EMULATE_SERVICE_BIN): $(AUDIODECODER_AIDL_LIB) $(AUDIOSINK_AIDL_LIB) 
$(EMULATE_SERVICE_BIN): $(EMULATE_SERVICE_BIN_SRC:%.cpp=%.o)
	g++ $(EMULATE_SERVICE_LDFLAGS) \
		$(EMULATE_SERVICE_BIN_SRC:%.cpp=%.o) -o $@ \
		$(EMULATE_SERVICE_LD_LIBS) -l$(AUDIODECODER_AIDL_LIB_NAME) -l$(AUDIOSINK_AIDL_LIB_NAME)

# do not clash with `ut-core` by chaning the object file name to .o.1
%.o: %.cpp
	g++ $(EMULATE_SERVICE_CFLAGS) $(EMULATE_SERVICE_INC_FLAGS) -fPIC -c $< -o $@

