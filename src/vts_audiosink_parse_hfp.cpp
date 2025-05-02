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

std::vector<sinkmanager> gAudiosinks_hfp;

UT_status_t vts_audiosink_parseAudioSinkHFP()
{
    std::string audiosinkpath = AUDIOSINKMGR_IAUDIOSINK;
    std::string platformcapabilitespath = AUDIOSINKMGR_PLATFORMCAPS;
    int32_t number_of_audiosinks;
    int32_t platformcapabilites_count;
    sinkmanager temporary;
    platformcap caps;
    char lowlatency_buffer[UT_KVP_MAX_ELEMENT_SIZE];
    char samplerate_buffer[UT_KVP_MAX_ELEMENT_SIZE];
    char mixer_pcmformat_buffer[UT_KVP_MAX_ELEMENT_SIZE];
    char supportsPlanarFormat_buffer[UT_KVP_MAX_ELEMENT_SIZE];

    number_of_audiosinks = UT_KVP_PROFILE_GET_LIST_COUNT(audiosinkpath.c_str());
    if(number_of_audiosinks <= 0)
    {
        UT_LOG_ERROR("No audiosinks found in the HFP");
        return UT_STATUS_FAILURE;
    }

    for(int32_t i = 0; i < number_of_audiosinks; ++i)
    {
        std::string audiosinkidpath = audiosinkpath + std::to_string(i) + ".id";
        int32_t sink_id = UT_KVP_PROFILE_GET_UINT32(audiosinkidpath.c_str());
        temporary.ids.push_back(sink_id);
    }
    
    // Read platform capabilites attributes
    std::string lowlatency_path = platformcapabilitespath + ".0.supportsLowLatency";
    std::string samplerate_path = platformcapabilitespath + ".1.systemMixerSampleRateHz";
    std::string mixer_pcmformat_path = platformcapabilitespath + ".2.systemMixerPCMFormat";
    std::string supportsPlanarFormat_path = platformcapabilitespath + ".3.supportsPlanarFormat";
    
    UT_KVP_PROFILE_GET_STRING(lowlatency_path.c_str(), lowlatency_buffer);
    UT_KVP_PROFILE_GET_STRING(samplerate_path.c_str(), samplerate_buffer);
    UT_KVP_PROFILE_GET_STRING(mixer_pcmformat_path.c_str(), mixer_pcmformat_buffer);
    UT_KVP_PROFILE_GET_STRING(supportsPlanarFormat_path.c_str(), supportsPlanarFormat_buffer);

    caps.lowlatency = std::string(lowlatency_buffer);
    caps.samplerate = std::string(samplerate_buffer);
    caps.mixer_pcmformat = std::string(mixer_pcmformat_buffer);
    caps.supportsPlanarFormat = std::string(supportsPlanarFormat_buffer);

    UT_LOG_INFO("lowlatency %s: samplerate=%s, mixer_pcmformat=%s, supportsplanarformat=%s",
                        caps.lowlatency.c_str() , caps.samplerate.c_str() , caps.mixer_pcmformat.c_str(), caps.supportsPlanarFormat.c_str());
    
    temporary.supportedcaps.push_back(caps);
    gAudiosinks_hfp.push_back(temporary);
    temporary.ids.clear();
    temporary.supportedcaps.clear();
    return UT_STATUS_OK;
}

/** @} */ // End of AUDIO_SINK_HALTEST_PARSE_HFP
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK
/** @} */ // End of HPK