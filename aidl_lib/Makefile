#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2025 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

.PHONY: all

AVBUFFER_AIDL_LIB_NAME := avbufferaidl
VIDEODECODER_AIDL_LIB_NAME := videodecoderaidl
AUDIODECODER_AIDL_LIB_NAME := audiodecoderaidl
AVBUFFER_HELPER_LIB_NAME := avbufferhelper
AUDIOSINK_AIDL_LIB_NAME := audiosinkaidl

AVBUFFER_AIDL_LIB := lib$(AVBUFFER_AIDL_LIB_NAME).so
VIDEODECODER_AIDL_LIB := lib$(VIDEODECODER_AIDL_LIB_NAME).so
AUDIODECODER_AIDL_LIB := lib$(AUDIODECODER_AIDL_LIB_NAME).so
AVBUFFERHELPER_LIB := lib$(AVBUFFER_HELPER_LIB_NAME).so
AUDIOSINK_AIDL_LIB := lib$(AUDIOSINK_AIDL_LIB_NAME).so

BUILD_DIR ?= $(CURDIR)/../build

AUDIODECODER_AIDL_GEN_DIR ?= $(BUILD_DIR)/audiodecoder_gen/current
AVBUFFER_AIDL_GEN_DIR ?= $(BUILD_DIR)/avbuffer_gen/current
VIDEODECODER_AIDL_GEN_DIR ?= $(BUILD_DIR)/videodecoder_gen/current
AUDIOSINK_GEN_DIR ?= $(BUILD_DIR)/audiosink_gen/current

AVBUFFER_AIDL_SRC += ${AVBUFFER_AIDL_GEN_DIR}/cpp/com/rdk/hal/avbuffer/HeapMetrics.cpp
AVBUFFER_AIDL_SRC += ${AVBUFFER_AIDL_GEN_DIR}/cpp/com/rdk/hal/avbuffer/IAVBuffer.cpp
AVBUFFER_AIDL_SRC += ${AVBUFFER_AIDL_GEN_DIR}/cpp/com/rdk/hal/avbuffer/IAVBufferSpaceListener.cpp
AVBUFFER_AIDL_SRC += ${AVBUFFER_AIDL_GEN_DIR}/cpp/com/rdk/hal/avbuffer/Pool.cpp
AVBUFFER_AIDL_SRC += ${AVBUFFER_AIDL_GEN_DIR}/cpp/com/rdk/hal/avbuffer/PoolMetrics.cpp

AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/Capabilities.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/ErrorCode.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSinkController.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSinkControllerListener.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSink.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSinkEventListener.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/IAudioSinkManager.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/PlatformCapabilities.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/Property.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/Volume.cpp
AUDIOSINK_GEN_SRC += ${AUDIOSINK_GEN_DIR}/cpp/com/rdk/hal/audiosink/VolumeRamp.cpp

AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/AVSource.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/HALError.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/PropertyValue.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/State.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/Capabilities.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/ChannelType.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/Codec.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/CSDAudioFormat.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/ErrorCode.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/FrameMetadata.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoderController.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoderControllerListener.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoder.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoderEventListener.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/IAudioDecoderManager.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/PCMFormat.cpp
AUDIODECODER_AIDL_SRC += ${AUDIODECODER_AIDL_GEN_DIR}/cpp/com/rdk/hal/audiodecoder/Property.cpp

VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/AVSource.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/PropertyValue.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/State.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/Capabilities.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/CodecCapabilities.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/Codec.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/CSDVideoFormat.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/DynamicRange.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/ErrorCode.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/FrameMetadata.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoderController.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoderControllerListener.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoder.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoderEventListener.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/IVideoDecoderManager.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/OperationalMode.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/PixelFormat.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/Property.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/PropertyKVPair.cpp
VIDEODECODER_AIDL_SRC += $(VIDEODECODER_AIDL_GEN_DIR)/cpp/com/rdk/hal/videodecoder/ScanType.cpp

AIDL_LIB_INC_FLAGS := -I$(AUDIODECODER_AIDL_GEN_DIR)/h
AIDL_LIB_INC_FLAGS += -I$(AVBUFFER_AIDL_GEN_DIR)/h
AIDL_LIB_INC_FLAGS += -I$(AUDIOSINK_GEN_DIR)/h
AIDL_LIB_INC_FLAGS += -I$(VIDEODECODER_AIDL_GEN_DIR)/h
AIDL_LIB_INC_FLAGS += -I$(BUILD_DIR)/usr/include

AIDL_LIB_LD_LIBS := -lbinder -lutils

AIDL_LIB_LDFLAGS := -L$(BUILD_DIR)/usr/lib
AIDL_LIB_LDFLAGS += -Wl,-rpath-link=$(BUILD_DIR)/usr/lib

AIDL_LIB_CFLAGS := -Wall -Wextra -std=c++17 -ggdb

AUDIOSINK_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AUDIOSINK_AIDL_LIB)
AVBUFFER_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AVBUFFER_AIDL_LIB)
AUDIODECODER_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(AUDIODECODER_AIDL_LIB)
VIDEODECODER_AIDL_LIB := $(BUILD_DIR)/usr/lib/$(VIDEODECODER_AIDL_LIB)

all: $(AVBUFFER_AIDL_LIB) $(VIDEODECODER_AIDL_LIB) $(AUDIODECODER_AIDL_LIB) $(AUDIOSINK_AIDL_LIB)

$(AUDIODECODER_AIDL_LIB): $(AUDIODECODER_AIDL_SRC:%.cpp=%.o)
	g++ -shared $(AIDL_LIB_LDFLAGS) $^ -o $@ $(AIDL_LIB_LD_LIBS)

$(VIDEODECODER_AIDL_LIB): $(VIDEODECODER_AIDL_SRC:%.cpp=%.o)
	g++ -shared $(AIDL_LIB_LDFLAGS) $^ -o $@ $(AIDL_LIB_LD_LIBS)

$(AVBUFFER_AIDL_LIB): $(AVBUFFER_AIDL_SRC:%.cpp=%.o)
	g++ -shared $(AIDL_LIB_LDFLAGS) $^ -o $@ $(AIDL_LIB_LD_LIBS)

$(AUDIOSINK_AIDL_LIB): $(AUDIOSINK_GEN_SRC:%.cpp=%.o)
	g++ -shared $(AIDL_LIB_LDFLAGS) $^ -o $@ $(AIDL_LIB_LD_LIBS)

%.o: %.cpp
	g++ $(AIDL_LIB_CFLAGS) $(AIDL_LIB_INC_FLAGS) -fPIC -c $< -o $@
