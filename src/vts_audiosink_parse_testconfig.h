/**
 *  If not stated otherwise in this file or this component's LICENSE
 *  file the following copyright and licenses apply:
 *
 *  Copyright 2024 RDK Management
 *
 *  Licensed under the Apache License, Version 2.0 (the License);
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an AS IS BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 * @par The Hardware Porting Kit
 * HPK is the next evolution of the well-defined Hardware Abstraction Layer
 * (HAL), but augmented with more comprehensive documentation and test suites
 * that OEM or SOC vendors can use to self-certify their ports before taking
 * them to RDKM for validation or to an operator for final integration and
 * deployment. The Hardware Porting Kit effectively enables an OEM and/or SOC
 * vendor to self-certify their own Video Accelerator devices, with minimal RDKM
 * assistance.
 *
 */

/**
 * @addtogroup AUDIO_DECODER Audio Decoder Module
 * @{
 */

/**
 * @addtogroup AUDIO_DECODER_HALTEST Audio Decoder HAL Tests
 * @{
 */

/**
 * @defgroup AUDIO_DECODER_HALTEST_PARSE_TESTCONFIG_HEADER Audio Decoder HAL Tests parse test configuration Header File
 * @{
 * @parblock
 *
 * ### Parse test configuration functions for Audio Decoder :
 *
 * Parse test configuration functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_audiodecoder_parse_testconfig.h
 *
 */

#ifndef __VTS_AUDIODECODER_PARSE_TESTCONFIG_H__
#define __VTS_AUDIODECODER_PARSE_TESTCONFIG_H__

#include <ut.h>
#include <ut_log.h>
#include <vector>
#include "ut_kvp_profile.h"
#include "vts_audiosink_helper_functions.h"
#include "vts_audiosink_parse_hfp.h"

using namespace std;
using namespace com::rdk::hal;
using namespace com::rdk::hal::audiodecoder;

#define VTS_TEST_CONFIG_PATH "./test_configuration/vts_l3_test_config.yaml"

typedef struct testConfiguration
{
    std::string input;
    Codec codec;
    int32_t sampleRate;
    int32_t numChannels;
    PCMFormat format;
    int32_t numOfFrames;
    int32_t frameSize;
    int32_t frameSizeInBytes;
    int32_t sinkDelay;
    int64_t stopPts;
    int64_t flushPts;
    int64_t eosPts;
    vector<audiodecoder::ChannelType> channelTypes;
    vector<int64_t> pts;
}testConfiguration_t;

void* vts_kvp_create_instance(char *fileName);
UT_status_t vts_audiosink_parse_testconfig(void* pTestInstance, const char *module, const char *testGroup,
                                           const char *testName, std::vector<testConfiguration_t>& streamDetails);
void vts_kvp_destroy_instance(void *instance);

#endif //__VTS_AUDIODECODER_PARSE_TESTCONFIG_H__

/** @} */ // End of AUDIO_DECODER_HALTEST_PARSE_TESTCONFIG_HEADER
/** @} */ // End of AUDIO_DECODER_HALTEST
/** @} */ // End of AUDIO_DECODER
/** @} */ // End of HPK