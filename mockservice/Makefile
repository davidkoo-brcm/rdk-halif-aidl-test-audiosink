#** *****************************************************************************
# *
# * If not stated otherwise in this file or this component's LICENSE file the
# * following copyright and licenses apply:
# *
# * Copyright 2025 RDK Management
# *
# * Licensed under the Apache License, Version 2.0 (the "License");
# * you may not use this file except in compliance with the License.
# * You may obtain a copy of the License at
# *
# *
# * http://www.apache.org/licenses/LICENSE-2.0
# *
# * Unless required by applicable law or agreed to in writing, software
# * distributed under the License is distributed on an "AS IS" BASIS,
# * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# * See the License for the specific language governing permissions and
# * limitations under the License.
# *
#** ******************************************************************************

.PHONY: all clean check-libs

MOCK_SERVICE_BIN := MockAudioSinkService

BUILD_DIR ?= $(CURDIR)/../../../build
RDK_HAL_DIR ?= $(BUILD_DIR)/../rdk-halif-aidl

MOCK_SERVICE_DIR ?= $(CURDIR)/src
MOCK_SERVICE_OBJ_DIR := $(CURDIR)/.obj

# Build HALIF sources locally for host architecture so we do not link ARM-only
# libs from out/target or build/usr/lib.
AUDIO_SINK_CPP_SRC_DIR := $(RDK_HAL_DIR)/audiosink/0.2.0.0/src/com/rdk/hal/audiosink
AUDIO_DECODER_CPP_SRC_DIR := $(RDK_HAL_DIR)/audiodecoder/0.2.0.0/src/com/rdk/hal/audiodecoder
AVCLOCK_CPP_SRC_DIR := $(RDK_HAL_DIR)/avclock/0.2.0.0/src/com/rdk/hal/avclock
COMMON_CPP_SRC_DIR := $(RDK_HAL_DIR)/common/0.2.0.0/src/com/rdk/hal

MOCK_SERVICE_BIN_SRC := $(MOCK_SERVICE_DIR)/AudioSinkService.cpp
MOCK_SERVICE_BIN_SRC += $(MOCK_SERVICE_DIR)/AudioSink.cpp
MOCK_SERVICE_BIN_SRC += $(wildcard $(AUDIO_SINK_CPP_SRC_DIR)/*.cpp)
MOCK_SERVICE_BIN_SRC += $(wildcard $(AUDIO_DECODER_CPP_SRC_DIR)/*.cpp)
MOCK_SERVICE_BIN_SRC += $(wildcard $(AVCLOCK_CPP_SRC_DIR)/*.cpp)
MOCK_SERVICE_BIN_SRC += $(wildcard $(COMMON_CPP_SRC_DIR)/*.cpp)

MOCK_SERVICE_LOCAL_SRC := $(filter $(MOCK_SERVICE_DIR)/%.cpp,$(MOCK_SERVICE_BIN_SRC))
MOCK_SERVICE_RDK_HAL_SRC := $(filter $(RDK_HAL_DIR)/%.cpp,$(MOCK_SERVICE_BIN_SRC))

MOCK_SERVICE_INC_FLAGS := -I$(BUILD_DIR)/audiodecoder_gen/current/h
MOCK_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/audiosink_gen/current/h
MOCK_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/videodecoder_gen/current/h
MOCK_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/avclock_gen/current/h
MOCK_SERVICE_INC_FLAGS += -I$(RDK_HAL_DIR)/audiodecoder/0.2.0.0/include
MOCK_SERVICE_INC_FLAGS += -I$(RDK_HAL_DIR)/audiosink/0.2.0.0/include
MOCK_SERVICE_INC_FLAGS += -I$(RDK_HAL_DIR)/avclock/0.2.0.0/include
MOCK_SERVICE_INC_FLAGS += -I$(RDK_HAL_DIR)/common/0.2.0.0/include
MOCK_SERVICE_INC_FLAGS += -I$(RDK_HAL_DIR)/out/target/include
MOCK_SERVICE_INC_FLAGS += -I$(RDK_HAL_DIR)/out/build/include/binder_sdk
MOCK_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/usr/include
MOCK_SERVICE_INC_FLAGS += -I$(MOCK_SERVICE_DIR)

MOCK_SERVICE_LDFLAGS := -L$(RDK_HAL_DIR)/build/binder
MOCK_SERVICE_LDFLAGS += -Wl,-rpath-link=$(RDK_HAL_DIR)/build/binder

MOCK_SERVICE_LD_LIBS := -lbinder -lutils -lbase -lcutils -llog -lpthread

MOCK_SERVICE_CFLAGS := -Wall -Wextra -std=c++17 -ggdb

MOCK_SERVICE_OBJ := $(patsubst $(MOCK_SERVICE_DIR)/%.cpp,$(MOCK_SERVICE_OBJ_DIR)/src/%.o,$(MOCK_SERVICE_LOCAL_SRC))
MOCK_SERVICE_OBJ += $(patsubst $(RDK_HAL_DIR)/%.cpp,$(MOCK_SERVICE_OBJ_DIR)/rdk-halif-aidl/%.o,$(MOCK_SERVICE_RDK_HAL_SRC))
MOCK_SERVICE_BIN := $(BUILD_DIR)/bin/$(MOCK_SERVICE_BIN)

all: $(MOCK_SERVICE_BIN)

check-libs:
	@echo "Checking required libraries..."
	@test -d "$(RDK_HAL_DIR)" || { echo "ERROR: missing $(RDK_HAL_DIR)"; exit 1; }
	@test -d "$(AUDIO_SINK_CPP_SRC_DIR)" || { echo "ERROR: missing $(AUDIO_SINK_CPP_SRC_DIR)"; exit 1; }
	@test -d "$(AUDIO_DECODER_CPP_SRC_DIR)" || { echo "ERROR: missing $(AUDIO_DECODER_CPP_SRC_DIR)"; exit 1; }
	@test -d "$(AVCLOCK_CPP_SRC_DIR)" || { echo "ERROR: missing $(AVCLOCK_CPP_SRC_DIR)"; exit 1; }
	@test -d "$(COMMON_CPP_SRC_DIR)" || { echo "ERROR: missing $(COMMON_CPP_SRC_DIR)"; exit 1; }
	@test -e "$(RDK_HAL_DIR)/build/binder/libbinder.so" || { echo "ERROR: missing host binder lib $(RDK_HAL_DIR)/build/binder/libbinder.so - run rdk-halif-aidl/build_binder.sh"; exit 1; }
	@test -e "$(RDK_HAL_DIR)/build/binder/libutils.so" || { echo "ERROR: missing host binder lib $(RDK_HAL_DIR)/build/binder/libutils.so - run rdk-halif-aidl/build_binder.sh"; exit 1; }
	@echo "Library check passed"

$(MOCK_SERVICE_BIN): check-libs $(MOCK_SERVICE_OBJ)
	@mkdir -p $(dir $@)
	g++ $(MOCK_SERVICE_LDFLAGS) \
		$(MOCK_SERVICE_OBJ) -o $@ \
		$(MOCK_SERVICE_LD_LIBS)

$(MOCK_SERVICE_OBJ_DIR)/src/%.o: $(MOCK_SERVICE_DIR)/%.cpp
	@mkdir -p $(dir $@)
	g++ $(MOCK_SERVICE_CFLAGS) $(MOCK_SERVICE_INC_FLAGS) -fPIC -c $< -o $@

$(MOCK_SERVICE_OBJ_DIR)/rdk-halif-aidl/%.o: $(RDK_HAL_DIR)/%.cpp
	@mkdir -p $(dir $@)
	g++ $(MOCK_SERVICE_CFLAGS) $(MOCK_SERVICE_INC_FLAGS) -fPIC -c $< -o $@

clean:
	rm -rf $(MOCK_SERVICE_OBJ_DIR)
	rm -f $(MOCK_SERVICE_BIN)

