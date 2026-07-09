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

# BUILD_DIR is the top-level repo build/ directory. When invoked as
# `make -C vts_hal_test_components/audiosink/mockservice`, CURDIR is that
# mockservice/ directory, so ../../../build resolves to <repo-root>/build.
BUILD_DIR ?= $(CURDIR)/../../../build

# Headers/libraries come from the AIDL codegen output produced by
# ./build.sh (rdk-halif-aidl/out/...), not from any audiosink_gen/
# staging directory.
RDK_HAL_DIR ?= $(BUILD_DIR)/../rdk-halif-aidl
HALIF_LIB_DIR ?= $(RDK_HAL_DIR)/out/target/lib/halif

# Header directories are versioned (e.g. .../audiosink/0.2.0.0/include)
# with no "current" alias, so pick whichever version directory build.sh
# produced.
AUDIOSINK_HEADERS ?= $(firstword $(wildcard $(RDK_HAL_DIR)/out/build/include/audiosink/*/include))
AUDIODECODER_HEADERS ?= $(firstword $(wildcard $(RDK_HAL_DIR)/out/build/include/audiodecoder/*/include))
AVCLOCK_HEADERS ?= $(firstword $(wildcard $(RDK_HAL_DIR)/out/build/include/avclock/*/include))
COMMON_HEADERS ?= $(firstword $(wildcard $(RDK_HAL_DIR)/out/build/include/common/*/include))

MOCK_SERVICE_DIR ?= $(CURDIR)/src

MOCK_SERVICE_BIN_SRC := $(MOCK_SERVICE_DIR)/AudioSinkService.cpp
MOCK_SERVICE_BIN_SRC += $(MOCK_SERVICE_DIR)/AudioSink.cpp

MOCK_SERVICE_INC_FLAGS := -I$(AUDIOSINK_HEADERS)
MOCK_SERVICE_INC_FLAGS += -I$(AUDIODECODER_HEADERS)
MOCK_SERVICE_INC_FLAGS += -I$(AVCLOCK_HEADERS)
MOCK_SERVICE_INC_FLAGS += -I$(COMMON_HEADERS)
MOCK_SERVICE_INC_FLAGS += -I$(BUILD_DIR)/usr/include

MOCK_SERVICE_LD_LIBS := -lbinder -lutils

MOCK_SERVICE_LDFLAGS := -L$(HALIF_LIB_DIR)
MOCK_SERVICE_LDFLAGS += -Wl,-rpath-link=$(HALIF_LIB_DIR)
MOCK_SERVICE_LDFLAGS += -L$(BUILD_DIR)/usr/lib
MOCK_SERVICE_LDFLAGS += -Wl,-rpath-link=$(BUILD_DIR)/usr/lib
MOCK_SERVICE_LDFLAGS += -Wl,--copy-dt-needed-entries
# -rpath-link only helps the linker resolve symbols at build time; it is not
# embedded in the binary. Embed a $ORIGIN-relative RUNPATH so the binary can
# find its .so dependencies at runtime as long as they are copied alongside it.
MOCK_SERVICE_LDFLAGS += -Wl,-rpath,'$$ORIGIN' -Wl,--disable-new-dtags

MOCK_SERVICE_CFLAGS := -Wall -Wextra -std=c++17 -ggdb

MOCK_SERVICE_BIN := $(BUILD_DIR)/bin/$(MOCK_SERVICE_BIN)

all: $(MOCK_SERVICE_BIN)

check-libs:
	@echo "Checking required libraries..."
	@test -n "$(AUDIOSINK_HEADERS)" || { echo "ERROR: missing audiosink headers in $(RDK_HAL_DIR)/out/build/include/audiosink/*/include - run ./build.sh all first"; exit 1; }
	@test -n "$(AUDIODECODER_HEADERS)" || { echo "ERROR: missing audiodecoder headers in $(RDK_HAL_DIR)/out/build/include/audiodecoder/*/include - run ./build.sh all first"; exit 1; }
	@test -n "$(AVCLOCK_HEADERS)" || { echo "ERROR: missing avclock headers in $(RDK_HAL_DIR)/out/build/include/avclock/*/include - run ./build.sh all first"; exit 1; }
	@test -n "$(COMMON_HEADERS)" || { echo "ERROR: missing common headers in $(RDK_HAL_DIR)/out/build/include/common/*/include - run ./build.sh all first"; exit 1; }
	@test -e "$(HALIF_LIB_DIR)/libaudiosink-v0.2.0.0-cpp.so" || { echo "ERROR: missing $(HALIF_LIB_DIR)/libaudiosink-v0.2.0.0-cpp.so - run ./build.sh all first"; exit 1; }
	@test -e "$(HALIF_LIB_DIR)/libaudiodecoder-v0.2.0.0-cpp.so" || { echo "ERROR: missing $(HALIF_LIB_DIR)/libaudiodecoder-v0.2.0.0-cpp.so - run ./build.sh all first"; exit 1; }
	@test -e "$(HALIF_LIB_DIR)/libavclock-v0.2.0.0-cpp.so" || { echo "ERROR: missing $(HALIF_LIB_DIR)/libavclock-v0.2.0.0-cpp.so - run ./build.sh all first"; exit 1; }
	@test -e "$(HALIF_LIB_DIR)/libcommon-v0.2.0.0-cpp.so" || { echo "ERROR: missing $(HALIF_LIB_DIR)/libcommon-v0.2.0.0-cpp.so - run ./build.sh all first"; exit 1; }
	@test -e "$(BUILD_DIR)/usr/lib/libbinder.so" || { echo "ERROR: missing $(BUILD_DIR)/usr/lib/libbinder.so - run ./build.sh all first"; exit 1; }
	@test -e "$(BUILD_DIR)/usr/lib/libutils.so" || { echo "ERROR: missing $(BUILD_DIR)/usr/lib/libutils.so - run ./build.sh all first"; exit 1; }
	@echo "Library check passed"

$(MOCK_SERVICE_BIN): check-libs $(MOCK_SERVICE_BIN_SRC:%.cpp=%.o)
	@mkdir -p $(dir $@)
	g++ $(MOCK_SERVICE_LDFLAGS) \
		$(MOCK_SERVICE_BIN_SRC:%.cpp=%.o) -o $@ \
		$(MOCK_SERVICE_LD_LIBS) \
		-lpthread \
		-laudiosink-v0.2.0.0-cpp \
		-laudiodecoder-v0.2.0.0-cpp \
		-lavclock-v0.2.0.0-cpp \
		-lcommon-v0.2.0.0-cpp \
		-lavbufferhelper

%.o: %.cpp
	g++ $(MOCK_SERVICE_CFLAGS) $(MOCK_SERVICE_INC_FLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(MOCK_SERVICE_BIN_SRC:%.cpp=%.o)
	rm -f $(MOCK_SERVICE_BIN)

