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
 * @defgroup AUDIO_SINK_HALTEST_PARSE_HFP Audio Sink HAL Tests parse Hardware feature profile
 * @{
 * @parblock
 *
 * ### Parse HFP functions for Audio Sink :
 *
 * Parse HFP functions required for the module across all vendors.
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_audiosink_parse_hfp.cpp
 *
 */

#include "vts_audiosink_helper_functions.h"
#include "vts_audiosink_parse_hfp.h"

using namespace std;

vector<sinkmanager> gAudiosinks_hfp;

UT_status_t vts_audiosink_parseAudioSinkHFP()
{
    const string audiosinkRoot = "audiosink";
    string audiosinkpath = audiosinkRoot + ".IAudioSink";
    string platformCapabilitesPath = audiosinkRoot + ".platformCapabilities";
    int32_t number_of_audiosinks, available_sinks_count;
    sinkmanager temporary;
    platformcap caps;
    char mixer_pcmformat_buffer[UT_KVP_MAX_ELEMENT_SIZE];

    gAudiosinks_hfp.clear();

    number_of_audiosinks = UT_KVP_PROFILE_GET_LIST_COUNT(audiosinkpath.c_str());
    if (number_of_audiosinks <= 0)
    {
        UT_LOG_ERROR("No audiosinks found in the HFP");
        return UT_STATUS_FAILURE;
    }

    for (int32_t i = 0; i < number_of_audiosinks; ++i)
    {
        string audiosinkidpath = audiosinkpath + "." + std::to_string(i);
        int32_t sink_id = UT_KVP_PROFILE_GET_UINT32(audiosinkidpath.c_str());
        temporary.ids.push_back(sink_id);
    }

    temporary.supportsSecure = UT_KVP_PROFILE_GET_BOOL((audiosinkRoot + ".supportsSecure").c_str());

    string availableIDPath = audiosinkRoot + ".availableSinks";
    available_sinks_count = UT_KVP_PROFILE_GET_LIST_COUNT(availableIDPath.c_str());
    for (int32_t i = 0; i < available_sinks_count; ++i)
    {
        string availableIDPathId = availableIDPath + "." + to_string(i);
        int32_t sink_id = UT_KVP_PROFILE_GET_UINT32(availableIDPathId.c_str());
        temporary.availableIds.push_back(sink_id);
    }

    string availableDecoderIDPath = audiosinkRoot + ".availableDecoders";
    int32_t available_decoders_count = UT_KVP_PROFILE_GET_LIST_COUNT(availableDecoderIDPath.c_str());
    for (int32_t i = 0; i < available_decoders_count; ++i)
    {
        string availableDecoderIDPathId = availableDecoderIDPath + "." + to_string(i);
        int32_t decoder_id = UT_KVP_PROFILE_GET_UINT32(availableDecoderIDPathId.c_str());
        temporary.availableDecoderIds.push_back(decoder_id);
    }

    const bool supportsLowLatency = UT_KVP_PROFILE_GET_BOOL((platformCapabilitesPath + ".supportsLowLatency").c_str());
    const uint32_t systemMixerSampleRateHz = UT_KVP_PROFILE_GET_UINT32((platformCapabilitesPath + ".systemMixerSampleRateHz").c_str());
    UT_KVP_PROFILE_GET_STRING((platformCapabilitesPath + ".systemMixerPCMFormat").c_str(), mixer_pcmformat_buffer);
    const bool supportsPlanarFormat = UT_KVP_PROFILE_GET_BOOL((platformCapabilitesPath + ".supportsPlanarFormat").c_str());

    caps.lowlatency = supportsLowLatency ? "true" : "false";
    caps.samplerate = std::to_string(systemMixerSampleRateHz);
    caps.mixer_pcmformat = std::string(mixer_pcmformat_buffer);
    caps.supportsPlanarFormat = supportsPlanarFormat ? "true" : "false";

    temporary.supportedcaps.push_back(caps);
    gAudiosinks_hfp.push_back(temporary);
    temporary = sinkmanager{};
    return UT_STATUS_OK;
}
/** @} */ // End of AUDIO_SINK_HALTEST_PARSE_HFP
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK
/** @} */ // End of HPK
