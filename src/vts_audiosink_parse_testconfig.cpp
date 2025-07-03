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
 * @addtogroup AUDIO_SINK Audio Sink Module
 * @{
 */

/**
 * @addtogroup AUDIO_SINK_HALTEST Audio Sink HAL Tests
 * @{
 */

/**
 * @defgroup AUDIO_SINK_HALTEST_PARSE_TESTCONFIG Audio Sink HAL Tests parse test configuration file
 * @{
 * @parblock
 *
 * ### Parse test configuration functions for Audio Sink :
 *
 * Parse configuration functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_audiosink_parse_testconfig.cpp
 *
 */

#include <filesystem>
#include "vts_audiosink_parse_testconfig.h"

using namespace std;

void* vts_kvp_create_instance(char *fileName)
{
    ut_kvp_instance_t* instance;
    ut_kvp_status_t result;

    UT_LOG("In %s", __FUNCTION__);

    instance = ut_kvp_createInstance();

    if(!instance)
    {
        UT_LOG_ERROR("Failed to create ut kvp instance");
        return nullptr;
    }

    result = ut_kvp_open(instance, fileName);

    if(result != UT_KVP_STATUS_SUCCESS)
    {
        UT_LOG_ERROR("Failed to open test configuration file");
        ut_kvp_destroyInstance(instance);
        return nullptr;
    }

    UT_LOG("Out  %s", __FUNCTION__);

    return (ut_kvp_instance_t*)instance;
}

void vts_kvp_destroy_instance(void *instance)
{
    ut_kvp_destroyInstance((ut_kvp_instance_t*)instance);
}

UT_status_t vts_audiosink_parse_testconfig(void* pTestInstance, const char *module, const char *testGroup,
                                           const char *testName, vector<testConfiguration_t>& testConfiguration)
{
    ut_kvp_status_t status;
    ut_kvp_instance_t* kvpTestInstance = (ut_kvp_instance_t*)pTestInstance;
    int32_t streamCount = 0;

    const string testList = string(module) + "." + string(testGroup) + "." + string(testName);
    const string controlList = testList + "." + "Controls";
    const string streamList = testList + "." + "Inputs" + "." + string("Streams");
    const string streamsDetails = string("AudioStreams");

    streamCount = ut_kvp_getListCount(kvpTestInstance, streamList.c_str());

    for(int32_t i = 0; i < streamCount; i++)
    {
        char resultKVP[UT_KVP_MAX_ELEMENT_SIZE] = {0};
        int32_t channelTypesCount = 0;
        int32_t ptsCount = 0;

        testConfiguration_t testDetails;

        string streamName = streamList + "." + to_string(i) + "Name";
        ut_kvp_getStringField(kvpTestInstance, streamName.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);

        string input       = streamsDetails + "." + string(resultKVP) + "." +"input";
        string codec       = streamsDetails + "." + string(resultKVP) + "." +"codec";
        string sampleRate  = streamsDetails + "." + string(resultKVP) + "." +"sample_rate";
        string channels    = streamsDetails + "." + string(resultKVP) + "." +"channels";
        string format      = streamsDetails + "." + string(resultKVP) + "." +"format";
        string numOfFrames = streamsDetails + "." + string(resultKVP) + "." +"num_of_frames";
        string frameSize   = streamsDetails + "." + string(resultKVP) + "." +"frame_size";
        string channelType = streamsDetails + "." + string(resultKVP) + "." +"channel_types";

        string framePTS  = streamList + "." + to_string(i) + "." +"PTS";
        string sinkDelay = controlList + "." + "delay_between_queue";
        string stopPts   = controlList + "." + "stop_start_pts";
        string flushPts  = controlList + "." + "flush_pts";
        string eosPts    = controlList + "." + "eos_pts";


        ut_kvp_getStringField(kvpTestInstance, input.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);
        testDetails.input = string(resultKVP);

        ut_kvp_getStringField(kvpTestInstance, codec.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);
        testDetails.codec = vts_audiosink_helper_StringTocodec(string(resultKVP));

        testDetails.sampleRate = ut_kvp_getUInt32Field(kvpTestInstance, sampleRate.c_str());
        testDetails.numChannels = ut_kvp_getUInt32Field(kvpTestInstance, channels.c_str());

        channelTypesCount = ut_kvp_getListCount(kvpTestInstance, channelType.c_str());
        for(int32_t j = 0; j < channelTypesCount; j++)
        {
            string channelTypeElement = channelType + "." + to_string(j);
            ut_kvp_getStringField(kvpTestInstance, channelTypeElement.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);
            testDetails.channelTypes.push_back(vts_audiosink_helper_StringToChannelType(string(resultKVP)));
        }

        ptsCount = ut_kvp_getListCount(kvpTestInstance, framePTS.c_str());
        for(int32_t j = 0; j < ptsCount; j++)
        {
            string ptsElement = framePTS + "." + to_string(j);
            int64_t ptsValue = ut_kvp_getUInt64Field(kvpTestInstance, ptsElement.c_str());
            testDetails.pts.push_back(ptsValue);
        }
        ut_kvp_getStringField(kvpTestInstance, format.c_str(), resultKVP, UT_KVP_MAX_ELEMENT_SIZE);
        testDetails.format = vts_audiosink_helper_StringToFormat(string(resultKVP));

        testDetails.numOfFrames = ut_kvp_getUInt32Field(kvpTestInstance, numOfFrames.c_str());
        testDetails.frameSize = ut_kvp_getUInt32Field(kvpTestInstance, frameSize.c_str());
        testDetails.frameSizeInBytes = testDetails.frameSize * vts_bytesPerSample(testDetails.format);
        testDetails.sinkDelay = ut_kvp_getUInt32Field(kvpTestInstance, sinkDelay.c_str());
        testDetails.stopPts   = ut_kvp_getUInt64Field(kvpTestInstance, stopPts.c_str());
        testDetails.flushPts  = ut_kvp_getUInt64Field(kvpTestInstance, flushPts.c_str());
        testDetails.eosPts    = ut_kvp_getUInt64Field(kvpTestInstance, eosPts.c_str());

        testConfiguration.push_back(testDetails);
    }

    return UT_STATUS_OK;
}

/** @} */ // End of AUDIO_SINK_HALTEST_PARSE_TESTCONFIG
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK
/** @} */ // End of HPK