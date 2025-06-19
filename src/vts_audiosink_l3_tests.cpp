/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @addtogroup HPK Hardware Porting Kit
 * @{
 *
 */

/**
 * @addtogroup AUDIO_SINK Audio Decoder Module
 * @{
 *
 */

/**
 * @defgroup AUDIO_SINK_HALTEST Audio Decoder HAL Tests
 * @{
 *
 */

/**
 * @defgroup AUDIO_SINK_HALTEST_L3 Audio Decoder HAL Tests L3 File
 * @{
 * @parblock
 *
 * ### L3 Test Cases for Audio Sink HAL :
 *
 * This module includes Level 3 functional tests (success and failure scenarios).
 * This is to ensure that the API meets the operational requirements of the module across all vendors.
 *
 * **Pre-Conditions:** @n
 * **Dependencies:** None@n
 *
 * @endparblock
 *
 */

/**
 * @file vts_audiosink_l3_tests.c
 *
 */

#include <ut_kvp_profile.h>
#include <ut.h>
#include <ut_log.h>
#include <ut_control_plane.h>
#include <deque>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <vector>
#include <optional>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <unordered_set>
#include <tuple>
#include <map>

#include "vts_audiosink_helper_functions.h"
#include "vts_audiosink_parse_hfp.h"
#include "vts_audiosink.h"

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_AudioSink, UT_TESTS_L3);

extern sp<IAudioSinkManager> gAudioSinkManager;

class VTS_L2_AudioSink : public UTCore
{
public:
VTS_L2_AudioSink() : UTCore() {}

    ~VTS_L2_AudioSink() override = default;

    public:
        std::vector<std::tuple<sp<IAudioSink>, sp<AudioSinkControllerListener>, sp<AudioSinkEventListener>>> mAudioSinkList;

        void SetUp() override
        {
            std::vector<IAudioSink::Id> sinkIds;
            /* Get the Audio Sink IDs */
            AUDIOSINK_MANAGER_GETAUDIOSINKIDS(sinkIds);

            android::ProcessState::self()->startThreadPool();

            for(const auto& _sinkId: sinkIds)
            {
                sp<IAudioSink> audioSink;

                sp<AudioSinkEventListener> eventListener = new AudioSinkEventListener();

                sp<AudioSinkControllerListener> controlListener = new AudioSinkControllerListener();

                /* Get Audio decoder handle */
                AUDIOSINK_MANAGER_GETAUDIOSINK(_sinkId, audioSink);

                /* Register events */
                AUDIOSINK_REGISTER_EVENT_SUCCESS(audioSink, eventListener);

                /* Get Capabilities of Audio Sink */
                //AUDIOSINK_GETCAPABILITIES(audioDecoder, capabilities);

                mAudioSinkList.push_back({audioSink, controlListener, eventListener});
            }
        }

    void checkCompleteStateTransition(sp<IAudioSink> sink, sp<AudioSinkEventListener> eventListener,
                                      sp<AudioSinkControllerListener> controlListener, State oldState,
                                      State intermediateState, State newState)
    {
        ::com::rdk::hal::State currentState;

        UT_ASSERT_TRUE(eventListener->checkStateTransistion(oldState, intermediateState));
        UT_ASSERT_TRUE(controlListener->checkStateTransistion(oldState, intermediateState));

        eventListener->waitForState(newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));
        controlListener->waitForState(newState, std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS));

        AUDIOSINK_GET_STATE(sink, currentState);

        UT_ASSERT_EQUAL(currentState, newState);

        UT_ASSERT_TRUE(eventListener->checkStateTransistion(intermediateState, newState));
        UT_ASSERT_TRUE(controlListener->checkStateTransistion(intermediateState, newState));

        eventListener->resetStateChanges();
        controlListener->resetStateChanges();
    }
    void TearDown() override
    {
        for (const auto& [sink, controlListener, eventListener] : mAudioSinkList)
        {
            /* Un-register the events */
            AUDIOSINK_UNREGISTER_EVENT_SUCCESS(sink, eventListener);
        }
        mAudioSinkList.clear();
        //TODO: check right place to call joinThreadPool
        //android::IPCThreadState::self()->joinThreadPool();
    }
};

/**
* @brief Test the audio sink underflow events
*
* This test verifies the functionality of underflow events.
*
* **Test Group ID:** 03@n
* **Test Case ID:** 001@n
*
* **Test Procedure:**
* Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(VTS_L2_AudioSink, Sink_Sequential_NonTunnel_NonSecure_Underflow)
{
}

/** @} */ // End of AUDIO_SINK_HALTEST L3 File
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK Module
/** @} */ // End of HPK