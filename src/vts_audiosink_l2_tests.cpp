
/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2024 RDK Management
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

#include "com/rdk/hal/audiosink/IAudioSink.h"
#include "com/rdk/hal/audiosink/IAudioSinkManager.h"
#include "com/rdk/hal/audiosink/BnAudioSinkControllerListener.h"
#include "com/rdk/hal/audiosink/BnAudioSinkEventListener.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::audiosink;
using namespace com::rdk::hal::audiodecoder;

static std::vector<IAudioSink::Id> mSinkIds;
static std::vector<sp<audiosink::IAudioSink>> mAudioSinks;
static std::vector<sp<IAudioSinkController>> mControllers;
static android::sp<IAudioSinkManager> gAudioSinkManagerService = nullptr;

static constexpr int MAX_TRANSITORY_STATE_WAIT_MS = 10;
static constexpr int MAX_FINAL_STATE_WAIT_MS = 500;

/* Status::Exception */
const static ut_control_keyStringMapping_t statusException_mapTable[] =
{
    {"EX_NONE",                  (int32_t) Status::EX_NONE},
    {"EX_SECURITY",              (int32_t) Status::EX_SECURITY},
    {"EX_BAD_PARCELABLE",        (int32_t) Status::EX_BAD_PARCELABLE},
    {"EX_ILLEGAL_ARGUMENT",      (int32_t) Status::EX_ILLEGAL_ARGUMENT},
    {"EX_NULL_POINTER",          (int32_t) Status::EX_NULL_POINTER},
    {"EX_ILLEGAL_STATE",         (int32_t) Status::EX_ILLEGAL_STATE},
    {"EX_NETWORK_MAIN_THREAD",   (int32_t) Status::EX_NETWORK_MAIN_THREAD},
    {"EX_UNSUPPORTED_OPERATION", (int32_t) Status::EX_UNSUPPORTED_OPERATION},
    {"EX_SERVICE_SPECIFIC",      (int32_t) Status::EX_SERVICE_SPECIFIC},
    {"EX_PARCELABLE",            (int32_t) Status::EX_PARCELABLE},
    {"EX_HAS_REPLY_HEADER",      (int32_t) Status::EX_HAS_REPLY_HEADER},
    {"EX_TRANSACTION_FAILED",    (int32_t) Status::EX_TRANSACTION_FAILED},
    {NULL, -1}
};

/* bool */
const static ut_control_keyStringMapping_t bool_mapTable[] =
{
    {"false", (int32_t) false},
    {"true",  (int32_t) true},
    {NULL, -1}
};

int getService()
{
    gAudioSinkManagerService =
        android::interface_cast<::com::rdk::hal::audiosink::IAudioSinkManager>(
            android::defaultServiceManager()->getService(
                android::String16(::com::rdk::hal::audiosink::IAudioSinkManager::serviceName().c_str())
            )
        );
    if (gAudioSinkManagerService == nullptr)
    {
        printf("Service not available\n");
        return -1;
    }
    return 0;
};

class AudioSinkEventListenerImpl : public BnAudioSinkEventListener {
public:
    AudioSinkEventListenerImpl() {};
    virtual ~AudioSinkEventListenerImpl() = default;
    
    std::mutex mtx;
    std::condition_variable cv;
    std::deque<std::pair<State, State>> stateChanges;
    
    Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override {
        std::lock_guard<std::mutex> lock(mtx);
        stateChanges.emplace_back(oldState, newState);
        cv.notify_all();
        return Status::ok();
    }

    ::android::binder::Status onFirstFrameRendered(int64_t) override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onEndOfStream(int64_t) override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onAudioUnderflow() override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onAudioResumed(int64_t) override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onFlushComplete() override
    {
        return ::android::binder::Status::ok();
    }
	
	bool waitForState(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState, int timeoutMs = 500) {
        std::unique_lock<std::mutex> lock(mtx);
        return cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [&] {
            for (const auto& p : stateChanges) {
                if (p.first == oldState && p.second == newState) return true;
            }
            return false;
        });
    }
};

class AudioSinkControllerListenerImpl : public BnAudioSinkControllerListener {
public:

    AudioSinkControllerListenerImpl() {};
    virtual ~AudioSinkControllerListenerImpl() = default;

    ::android::binder::Status onStateChanged(::com::rdk::hal::State oldState,
                                             ::com::rdk::hal::State newState) override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onFirstFrameRendered(int64_t) override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onEndOfStream(int64_t) override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onAudioUnderflow() override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onAudioResumed(int64_t) override
    {
        return ::android::binder::Status::ok();
    }

    ::android::binder::Status onFlushComplete() override
    {
        return ::android::binder::Status::ok();
    }
};

class AudioSinkManagerBaseTest : public virtual UTCore
{
protected:
    void SetUp() override
    {
        UTCore::SetUp();
        UT_ASSERT_EQUAL(getService(), 0);
    }

    void TearDown() override 
    {
		UTCore::TearDown();
        gAudioSinkManagerService.clear();
    }
};

class AudioSinkBaseTest : public AudioSinkManagerBaseTest
{
public:
    std::vector<sp<AudioSinkEventListenerImpl>> mSinkEventListener;

    void SetUp() override
    {
        AudioSinkManagerBaseTest::SetUp();
        if (mSinkIds.empty())
        {
            std::vector<::com::rdk::hal::audiosink::IAudioSink::Id> ids;
            UT_LOG_INFO("Calling getAudioSinkIds(OUT:ids:[])");
            Status status = gAudioSinkManagerService->getAudioSinkIds(&ids);
            UT_LOG_INFO("Result getAudioSinkIds(OUT:ids:[size: %d]), status:[isOK:%s, exception:%s]",
                        ids.size(),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            mSinkIds = ids;
            UT_ASSERT_FALSE(mSinkIds.empty());
        }
        if (mAudioSinks.empty())
        {
            
            for (auto& sinkId : mSinkIds)
            {
                sp<AudioSinkEventListenerImpl> listener = new AudioSinkEventListenerImpl();
                android::sp<::com::rdk::hal::audiosink::IAudioSink> sink;
                UT_LOG_INFO("Calling getAudioSink(IN:sinkId:[%d], OUT:sink[])", sinkId.value);
                Status status = gAudioSinkManagerService->getAudioSink(sinkId, &sink);
                UT_LOG_INFO("Result getAudioSink(IN:sinkId:[%d], OUT:sink[0x%X]), status:[isOK:%s, exception:%s]",
                            sinkId.value, sink.get(),
                            UT_Control_GetMapString(bool_mapTable, status.isOk()),
                            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
                UT_ASSERT_TRUE(status.isOk());
                UT_ASSERT_NOT_EQUAL(sink, nullptr);
                bool registered = false;
                UT_LOG_INFO("Calling registerEventListener(IN:listener[0x%X], OUT:registered[])", listener.get());
                Status regStatus = sink->registerEventListener(listener, &registered);
                UT_LOG_INFO("Result registerEventListener(IN:listener[0x%X], OUT:registered[%s]), status:[isOK:%s, exception:%s]",
                            listener.get(),
                            UT_Control_GetMapString(bool_mapTable, registered),
                            UT_Control_GetMapString(bool_mapTable, regStatus.isOk()),
                            UT_Control_GetMapString(statusException_mapTable, regStatus.exceptionCode()));
                UT_ASSERT_TRUE(regStatus.isOk());
                UT_ASSERT_TRUE(registered);
                mAudioSinks.push_back(sink);
                mSinkEventListener.push_back(listener);
            }
        }
    }

    void TearDown() override
    {
        for (size_t i = 0; i < mAudioSinks.size(); ++i) {
            bool unregistered = false;
            UT_LOG_INFO("Calling unregisterEventListener(IN:listener[0x%X], OUT:unregistered[])", mSinkEventListener[i].get());
            Status status = mAudioSinks[i]->unregisterEventListener(mSinkEventListener[i], &unregistered);
            UT_LOG_INFO("Result unregisterEventListener(IN:listener[0x%X], OUT:unregistered[%s]), status:[isOK:%s, exception:%s]",
                        mSinkEventListener[i].get(),
                        UT_Control_GetMapString(bool_mapTable, unregistered),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(unregistered);
        }

        mAudioSinks.clear();
        mSinkIds.clear();
        mSinkEventListener.clear();
        AudioSinkManagerBaseTest::TearDown();
    }
};


class AudioSinkControllerBaseTest : public AudioSinkBaseTest
{
public:
    std::vector<sp<AudioSinkControllerListenerImpl>> mControllerListener;

    void SetUp() override
    {
        AudioSinkBaseTest::SetUp();
        if (mControllers.empty() && !mAudioSinks.empty())
        {
            sp<IAudioSinkController> controller;
            sp<AudioSinkControllerListenerImpl> controllerlistener = new AudioSinkControllerListenerImpl();

            for (auto& sink : mAudioSinks)
            {
                UT_LOG_INFO("Calling open(IN:controllerlistener[0x%X], OUT:controller[])", controllerlistener.get());
                auto status = sink->open(controllerlistener, &controller);
                UT_LOG_INFO("Result open(IN:controllerlistener[0x%X], OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controllerlistener.get(), controller.get(),
                            UT_Control_GetMapString(bool_mapTable, status.isOk()),
                            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
                UT_ASSERT_TRUE(status.isOk());
                UT_ASSERT_NOT_EQUAL(controller, nullptr);
                mControllers.push_back(controller);
                mControllerListener.push_back(controllerlistener);
            }
        }
    }

    void TearDown() override
    {
        UT_ASSERT_EQUAL(mControllers.size(), mAudioSinks.size());
        for (size_t i = 0; i < mAudioSinks.size(); ++i)
        {
            bool closed = false;
            UT_LOG_INFO("Calling close(IN:controller[0x%X], OUT:closed[])", mControllers[i].get());
            auto status = mAudioSinks[i]->close(mControllers[i], &closed);
            UT_LOG_INFO("Result close(IN:controller[0x%X], OUT:closed[%s]), status:[isOK:%s, exception:%s]",
                        mControllers[i].get(),
                        UT_Control_GetMapString(bool_mapTable, closed),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            EXPECT_TRUE(status.isOk());
            EXPECT_TRUE(closed);
        }
        mControllers.clear();
        mControllerListener.clear();
        AudioSinkBaseTest::TearDown();
    }
};

/**
* @brief Test the read and write operations for audio sink properties
*
* This test verifies the functionality of setting and getting properties for audio sinks. It ensures that the setProperty and getProperty methods work correctly and return expected results.
*
* **Test Group ID:** 02@n
* **Test Case ID:** 001@n
*
* **Test Procedure:**
* Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(AudioSinkBaseTest, TestReadWriteProperties) {
    for (size_t i = 0; i < mAudioSinks.size(); ++i) {
        sp<IAudioSink> sink = mAudioSinks[i];

        struct PropertyTestCase {
            ::com::rdk::hal::audiosink::Property property;
            int32_t testValue;
            const char* propertyName;
        };

        std::vector<PropertyTestCase> testCases = {
            {::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL, -20, "REFERENCE_LEVEL"},
            {::com::rdk::hal::audiosink::Property::MIXER_ID, 1, "MIXER_ID"},
            {::com::rdk::hal::audiosink::Property::AV_SOURCE, 2, "AV_SOURCE"},
            {::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK, 1, "DOLBY_ATMOS_LOCK"},
        };

        for (const auto& testCase : testCases) {
            PropertyValue setValue;
            setValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(testCase.testValue);

            bool setResult = false;
            UT_LOG_INFO("Calling setProperty(IN:property:[%s], IN:propertyValue:[%d])",
                        testCase.propertyName, testCase.testValue);
            Status setStatus = sink->setProperty(testCase.property, setValue, &setResult);
            UT_LOG_INFO("Result setProperty(IN:property:[%s], IN:propertyValue:[%d]), status:[isOK:%s, exception:%s]",
                        testCase.propertyName, testCase.testValue,
                        UT_Control_GetMapString(bool_mapTable, setStatus.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, setStatus.exceptionCode()));
            UT_ASSERT_TRUE(setStatus.isOk());
            UT_ASSERT_TRUE(setResult);

            ::std::optional<PropertyValue> getValue;
            UT_LOG_INFO("Calling getProperty(IN:property:[%s], OUT:propertyValue[])", testCase.propertyName);
            Status getStatus = sink->getProperty(testCase.property, &getValue);
            UT_LOG_INFO("Result getProperty(IN:property:[%s], OUT:propertyValue[%d]), status:[isOK:%s, exception:%s]",
                        testCase.propertyName,
                        static_cast<int32_t>(getValue->value->getTag()),
                        UT_Control_GetMapString(bool_mapTable, getStatus.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, getStatus.exceptionCode()));
            UT_ASSERT_TRUE(getStatus.isOk());
            UT_ASSERT_TRUE(getValue.has_value());
            UT_ASSERT_EQUAL(static_cast<int32_t>(getValue->value->getTag()), testCase.testValue);
        }
    }
}

/**
 * @brief Test the state transitions of the AudioSink component
 *
 * This test verifies the correct state transitions of the AudioSink component to ensure it behaves as expected under various conditions.
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 002@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(AudioSinkBaseTest, StateTransitionTest) {
    for (size_t i = 0; i < mAudioSinks.size(); ++i) {
        sp<IAudioSink> sink = mAudioSinks[i];
        sp<AudioSinkEventListenerImpl> listener = mSinkEventListener[i];
        sp<IAudioSinkController> controller;
        //sp<AudioSinkControllerListenerImpl> controllerlistener = new AudioSinkControllerListenerImpl();
        sp<IAudioSinkControllerListener> controllerListener = new AudioSinkControllerListenerImpl();

        // Open the audio sink
        UT_LOG_INFO("Calling open(IN:controllerListener[0x%X], OUT:controller[])", controllerListener.get());
        Status status = sink->open(controllerListener, &controller);
        UT_LOG_INFO("Result open(OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        // Verify state transition to OPENING
        UT_ASSERT_TRUE(listener->waitForState(State::CLOSED, State::OPENING, MAX_TRANSITORY_STATE_WAIT_MS));

        // Verify state transition to READY
        UT_ASSERT_TRUE(listener->waitForState(State::OPENING, State::READY, MAX_FINAL_STATE_WAIT_MS));

        ::com::rdk::hal::State currentState;
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        Status statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Start the audio sink
        UT_LOG_INFO("Calling start()");
        status = controller->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
       
        // Verify state transition to STARTING
        UT_ASSERT_TRUE(listener->waitForState(State::READY, State::STARTING, MAX_TRANSITORY_STATE_WAIT_MS));

        // Verify state transition to STARTED
        UT_ASSERT_TRUE(listener->waitForState(State::STARTING, State::STARTED, MAX_FINAL_STATE_WAIT_MS));

        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(statestatus.isOk());
        UT_ASSERT_EQUAL(currentState, State::STARTED);

        // Flush the audio sink
        UT_LOG_INFO("Calling flush()");
        status = controller->flush();
        UT_LOG_INFO("Result flush(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        
        // Verify state transition to FLUSHING
        UT_ASSERT_TRUE(listener->waitForState(State::STARTED, State::FLUSHING, MAX_TRANSITORY_STATE_WAIT_MS));

        // Verify state transition to STARTED
        UT_ASSERT_TRUE(listener->waitForState(State::FLUSHING, State::STARTED, MAX_FINAL_STATE_WAIT_MS));

        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(statestatus.isOk());
        UT_ASSERT_EQUAL(currentState, State::STARTED);

        // Stop the audio sink
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Verify state transition to STOPPING
        UT_ASSERT_TRUE(listener->waitForState(State::STARTED, State::STOPPING, MAX_TRANSITORY_STATE_WAIT_MS));

        // Verify state transition to READY
        UT_ASSERT_TRUE(listener->waitForState(State::STOPPING, State::READY, MAX_FINAL_STATE_WAIT_MS));

        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Close the audio sink
        bool closed = false;
        UT_LOG_INFO("Calling close(IN:controller:[0x%X], OUT:closed)", controller.get());
        status = sink->close(controller, &closed);
        UT_LOG_INFO("Result close(IN:controller:[0x%X], OUT:closed:[%d]), status:[isOK:%s, exception:%s]",
                    controller.get(), closed,UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);
        
        // Verify state transition to CLOSING
        UT_ASSERT_TRUE(listener->waitForState(State::READY, State::CLOSING, MAX_TRANSITORY_STATE_WAIT_MS));

        // Verify state transition to CLOSED
        UT_ASSERT_TRUE(listener->waitForState(State::CLOSING, State::CLOSED, MAX_FINAL_STATE_WAIT_MS));

        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::CLOSED);
    }
}

/**
* @brief Test the audio sink volume control functionality
*
* This test verifies the ability to set and get the volume on audio sinks. It ensures that the volume can be adjusted from 0.0 to 1.0 in increments of 0.1 and checks that the set volume matches the retrieved volume.
*
* **Test Group ID:** 02@n
* **Test Case ID:** 003@n
*
* **Test Procedure:**
* Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(AudioSinkBaseTest, TestAudioSinkVolumeControl) {
    for (size_t i = 0; i < mAudioSinks.size(); ++i) {
        sp<IAudioSink> sink = mAudioSinks[i];
        sp<IAudioSinkController> controller;
        sp<AudioSinkControllerListenerImpl> controllerlistener = new AudioSinkControllerListenerImpl();

        // Open the audio sink
        UT_LOG_INFO("Calling open(IN:controllerlistener[0x%X], OUT:controller[])", controllerlistener.get());
        Status status = sink->open(controllerlistener, &controller);
        UT_LOG_INFO("Result open(OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        ::com::rdk::hal::State currentState;
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        Status statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(statestatus.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Set and verify volume
        for (double volume = 0.0; volume <= 1.0; volume += 0.1) {
            Volume vol;
            vol.volume = volume;
            UT_LOG_INFO("Calling setVolume(IN:volume:[%f])", volume);
            bool volumeSet = false;
            status = controller->setVolume(vol, &volumeSet);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(volumeSet);
            UT_LOG_INFO("Result setVolume(IN:volume:[%f]), status:[isOK:%s, exception:%s]",
                        volume, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

            Volume currentVol;
            UT_LOG_INFO("Calling getVolume(OUT:volume)");
            status = controller->getVolume(&currentVol);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_EQUAL(currentVol.volume, volume);
            UT_LOG_INFO("Result getVolume(OUT:volume:[%f]), status:[isOK:%s, exception:%s]",
                        currentVol.volume, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        }

        // Close the audio sink
        bool closed = false;
        UT_LOG_INFO("Calling close(IN:controller:[0x%X], OUT:closed)", controller.get());
        status = sink->close(controller, &closed);
        UT_LOG_INFO("Result close(IN:controller:[0x%X], OUT:closed:[%d]), status:[isOK:%s, exception:%s]",
                    controller.get(), closed,UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);

        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(statestatus.isOk());
        UT_ASSERT_EQUAL(currentState, State::CLOSED);
    }
}

/**
* @brief Test the mute and unmute operations of the AudioSink component
*
* This test verifies the basic operations of the AudioSink component, including opening, setting volume, muting, unmuting, and closing the audio sink. It ensures that the state transitions and volume settings are correctly handled.
*
* **Test Group ID:** 02@n
* **Test Case ID:** 004@n
*
* **Test Procedure:**
* Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(AudioSinkBaseTest, TestAudioSinkOperations) {
    for (size_t i = 0; i < mAudioSinks.size(); ++i) {
        sp<IAudioSink> sink = mAudioSinks[i];
        sp<IAudioSinkController> controller;
        sp<AudioSinkControllerListenerImpl> controllerlistener = new AudioSinkControllerListenerImpl();

        // Open the audio sink
        UT_LOG_INFO("Calling open(IN:controllerlistener[0x%X], OUT:controller[])", controllerlistener.get());
        Status status = sink->open(controllerlistener, &controller);
        UT_LOG_INFO("Result open(OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        ::com::rdk::hal::State currentState;
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        Status statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(statestatus.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Set and verify volume
        for (double volume = 0.1; volume <= 1.0; volume += 0.1) {
            Volume vol;
            vol.volume = volume;
            vol.muted = true; 
            UT_LOG_INFO("Calling setVolume(IN:volume:[%f])", volume);
            bool volumeSet = false;
            status = controller->setVolume(vol, &volumeSet);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(volumeSet);
            UT_LOG_INFO("Result setVolume(IN:volume:[%f]), status:[isOK:%s, exception:%s]",
                        volume, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            
            Volume currentVol;
            UT_LOG_INFO("Calling getVolume(OUT:volume)");
            status = controller->getVolume(&currentVol);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(currentVol.muted);
            UT_LOG_INFO("Result getVolume(OUT:volume:[%f]), status:[isOK:%s, exception:%s]",
                        currentVol.volume, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

            vol.muted = false; 
            UT_LOG_INFO("Calling setVolume(IN:volume:[%f])", volume);
            volumeSet = false;
            status = controller->setVolume(vol, &volumeSet);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(volumeSet);
            UT_LOG_INFO("Result setVolume(IN:volume:[%f]), status:[isOK:%s, exception:%s]",
                        volume, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

            UT_LOG_INFO("Calling getVolume(OUT:volume)");
            status = controller->getVolume(&currentVol);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_FALSE(currentVol.muted);
            UT_ASSERT_EQUAL(currentVol.volume, volume);
            UT_LOG_INFO("Result getVolume(OUT:volume:[%f]), status:[isOK:%s, exception:%s]",
                        currentVol.volume, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        }

        // Close the audio sink
        bool closed = false;
        UT_LOG_INFO("Calling close(IN:controller:[0x%X], OUT:closed)", controller.get());
        status = sink->close(controller, &closed);
        UT_LOG_INFO("Result close(IN:controller:[0x%X], OUT:closed:[%d]), status:[isOK:%s, exception:%s]",
                    controller.get(), closed,UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);

        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        statestatus = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), statestatus:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, statestatus.isOk()), UT_Control_GetMapString(statusException_mapTable, statestatus.exceptionCode()));
        UT_ASSERT_TRUE(statestatus.isOk());
        UT_ASSERT_EQUAL(currentState, State::CLOSED);
    }
}

/**
 * @brief Test the Set VolumeRamp functionality of the AudioSink component
 *
 * This test verifies the complete lifecycle of the AudioSink component, including opening, starting, setting volumeRamp, stopping, and closing. It ensures that the AudioSink transitions through the expected states and that operations such as setting the volume ramp are successful.
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 005@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(AudioSinkBaseTest, TestAudioSinkVolumeRamp) {
    for (size_t i = 0; i < mAudioSinks.size(); ++i) {
        sp<IAudioSink> sink = mAudioSinks[i];
        sp<IAudioSinkController> controller;
        sp<AudioSinkControllerListenerImpl> controllerlistener = new AudioSinkControllerListenerImpl();

        // Open the audio sink
        UT_LOG_INFO("Calling open(IN:controllerlistener[0x%X], OUT:controller[])", controllerlistener.get());
        Status status = sink->open(controllerlistener, &controller);
        UT_LOG_INFO("Result open(OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        ::com::rdk::hal::State currentState;
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Start the audio sink
        UT_LOG_INFO("Calling start()");
        status = controller->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Verify the state is STARTED
        UT_LOG_INFO("Calling getState(OUT:currentState)");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:state:[%d]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::STARTED);

        // Set volume ramp
        double targetVolume = 0.5; // Example target volume
        int32_t overMs = 1000; // Example duration
        VolumeRamp volumeRamp = VolumeRamp::LINEAR;
        bool volumeRampStarted;
        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:volumeRampStarted)", targetVolume, overMs, volumeRamp);
        status = controller->setVolumeRamp(targetVolume, overMs, volumeRamp, &volumeRampStarted);
        UT_LOG_INFO("Result setVolumeRamp(OUT:volumeRampStarted:[%d]), status:[isOK:%s, exception:%s]", volumeRampStarted, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(volumeRampStarted);

        // Verify volume is set to target volume
        Volume volume;
        do {
            UT_LOG_INFO("Calling getVolume(OUT:volume)");
            status = controller->getVolume(&volume);
            UT_LOG_INFO("Result getVolume(OUT:volume:[%f]), status:[isOK:%s, exception:%s]", volume.volume, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
        } while (volume.volume != targetVolume);

        // Stop the audio sink
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Verify the state is READY
        UT_LOG_INFO("Calling getState(OUT:currentState)");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:state:[%d]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Close the audio sink
        bool closed = false;
        UT_LOG_INFO("Calling close(IN:controller:[0x%X], OUT:closed)", controller.get());
        status = sink->close(controller, &closed);
        UT_LOG_INFO("Result close(IN:controller:[0x%X], OUT:closed:[%d]), status:[isOK:%s, exception:%s]",
                    controller.get(), closed,UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);

        // Verify the state is CLOSED
        UT_LOG_INFO("Calling getState(OUT:currentState)");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:state:[%d]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::CLOSED);
    }
}

/**
* @brief Test the Set VolumeRamp functionality of the AudioSink component in Mute condition
*
* This test verifies the sequence of operations on the AudioSink component, including opening, starting, setting mute condition, setting volumeRamp, verifying volume, stopping, and closing. It ensures that the state transitions and volume settings are correctly handled.
*
* **Test Group ID:** 02@n
* **Test Case ID:** 006@n
*
* **Test Procedure:**
* Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(AudioSinkBaseTest, TestAudioSinkVolumeRampMute) {
    for (size_t i = 0; i < mAudioSinks.size(); ++i) {
        sp<IAudioSink> sink = mAudioSinks[i];
        sp<IAudioSinkController> controller;
        sp<AudioSinkControllerListenerImpl> controllerlistener = new AudioSinkControllerListenerImpl();

        // Open the audio sink
        UT_LOG_INFO("Calling open(IN:controllerlistener[0x%X], OUT:controller[])", controllerlistener.get());
        Status status = sink->open(controllerlistener, &controller);
        UT_LOG_INFO("Result open(OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        ::com::rdk::hal::State currentState;
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Start the audio sink
        UT_LOG_INFO("Calling start()");
        status = controller->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Verify the state is STARTED
        UT_LOG_INFO("Calling getState(OUT:state)");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:state[%d]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::STARTED);

        // Set volume to mute
        Volume volume;
        volume.volume = 0.0;
        volume.muted = true;
        bool volumeSet = false;
        UT_LOG_INFO("Calling setVolume(IN:volume:[%f])", volume);
        status = controller->setVolume(volume, &volumeSet);
        UT_LOG_INFO("Result setVolume(IN:volume:[%f]), status:[isOK:%s, exception:%s]",
            volume, UT_Control_GetMapString(bool_mapTable, status.isOk()),
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(volumeSet);

        // Set volume ramp
        double targetVolume = 1.0;
        int32_t overMs = 1000;
        VolumeRamp rampType = VolumeRamp::LINEAR;
        bool rampSet = false;
        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume[%f], IN:overMs[%d], IN:volumeRamp[%d])", targetVolume, overMs, static_cast<int>(rampType));
        status = controller->setVolumeRamp(targetVolume, overMs, rampType, &rampSet);
        UT_LOG_INFO("Result setVolumeRamp(IN:targetVolume[%f], IN:overMs[%d], IN:volumeRamp[%d]), status:[isOK:%s, exception:%s]", targetVolume, overMs, static_cast<int>(rampType), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(rampSet);

        // Verify volume is set to target volume
        Volume getvolume;
        do {
            UT_LOG_INFO("Calling getVolume(OUT:getvolume)");
            status = controller->getVolume(&getvolume);
            UT_LOG_INFO("Result getVolume(OUT:volume:[%f]), status:[isOK:%s, exception:%s]", getvolume.volume, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
        } while (getvolume.volume != targetVolume || getvolume.muted);
        UT_ASSERT_EQUAL(getvolume.volume, targetVolume);
        UT_ASSERT_FALSE(getvolume.muted);

        // Stop the audio sink
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Verify the state is READY
        UT_LOG_INFO("Calling getState(OUT:state)");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:state[%d]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_EQUAL(currentState, State::READY);
        UT_ASSERT_TRUE(status.isOk());

        // Close the audio sink
        bool closed = false;
        UT_LOG_INFO("Calling close(IN:controller:[0x%X], OUT:closed)", controller.get());
        status = sink->close(controller, &closed);
        UT_LOG_INFO("Result close(IN:controller:[0x%X], OUT:closed:[%d]), status:[isOK:%s, exception:%s]",
                    controller.get(), closed,UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);

        // Verify the state is CLOSED
        UT_LOG_INFO("Calling getState(OUT:state)");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:state[%d]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_EQUAL(currentState, State::CLOSED);
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the Set VolumeRamp functionality of the AudioSink component by invoking it twice
 *
 * This test verifies the complete lifecycle of an audio sink, including opening, starting, setting volume ramp, stopping, and closing. It ensures that each state transition is handled correctly and the expected state is achieved at each step.
 *
 * **Test Group ID:** 02@n
 * **Test Case ID:** 007@n
 *
 * **Test Procedure:**
 * Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
 */
UT_ADD_TEST(AudioSinkBaseTest, TestAudioSinkVolumeRampTwice) {
    for (size_t i = 0; i < mAudioSinks.size(); ++i) {
        sp<IAudioSink> sink = mAudioSinks[i];
        sp<IAudioSinkController> controller;
        sp<AudioSinkControllerListenerImpl> controllerlistener = new AudioSinkControllerListenerImpl();

        // Open the audio sink
        UT_LOG_INFO("Calling open(IN:controllerlistener[0x%X], OUT:controller[])", controllerlistener.get());
        Status status = sink->open(controllerlistener, &controller);
        UT_LOG_INFO("Result open(OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        ::com::rdk::hal::State currentState;
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Start the audio sink
        UT_LOG_INFO("Calling start()");
        status = controller->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Verify state is STARTED
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState[%s]), status:[isOK:%s, exception:%s]",
            currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::STARTED);

        // Set volume ramp
        double targetVolume = 0.5;
        int32_t overMs = 1000;
        VolumeRamp volumeRamp = VolumeRamp::LINEAR;
        bool rampStarted = false;
        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume[%f], IN:overMs[%d], IN:volumeRamp[%d], OUT:rampStarted)",
                    targetVolume, overMs, static_cast<int32_t>(volumeRamp));
        status = controller->setVolumeRamp(targetVolume, overMs, volumeRamp, &rampStarted);
        UT_LOG_INFO("Result setVolumeRamp(OUT:rampStarted[%d]), status:[isOK:%s, exception:%s]",
                    rampStarted, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(rampStarted);

        // Set volume ramp again
        targetVolume = 0.7;
        overMs = 1000;
        volumeRamp = VolumeRamp::LINEAR;
        rampStarted = false;
        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume[%f], IN:overMs[%d], IN:volumeRamp[%d], OUT:rampStarted)",
                    targetVolume, overMs, static_cast<int32_t>(volumeRamp));
        status = controller->setVolumeRamp(targetVolume, overMs, volumeRamp, &rampStarted);
        UT_LOG_INFO("Result setVolumeRamp(OUT:rampStarted[%d]), status:[isOK:%s, exception:%s]",
                    rampStarted, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(rampStarted);

        // Verify volume is set to target volume
        Volume volume;
        do {
            UT_LOG_INFO("Calling getVolume(OUT:volume)");
            status = controller->getVolume(&volume);
            UT_LOG_INFO("Result getVolume(OUT:volume:[%f]), status:[isOK:%s, exception:%s]", volume.volume, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
        } while (volume.volume != targetVolume);
        UT_ASSERT_EQUAL(volume.volume, targetVolume);

        // Stop the audio sink
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Verify state is READY
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState[%s]), status:[isOK:%s, exception:%s]",
            currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Close the audio sink
        bool closed = false;
        UT_LOG_INFO("Calling close(IN:controller:[0x%X], OUT:closed)", controller.get());
        status = sink->close(controller, &closed);
        UT_LOG_INFO("Result close(IN:controller:[0x%X], OUT:closed:[%d]), status:[isOK:%s, exception:%s]",
                    controller.get(), closed,UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);

        // Verify state is CLOSED
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState[%s]), status:[isOK:%s, exception:%s]",
            currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::CLOSED);
    }
}

/**
* @brief Tests the VolumeRamp functionality of AudioSink by verifying state transitions and volume adjustments.
*
* This test verifies the correct state transitions and operations of an audio sink. Opens the sink, starts playback, sets VolumeRamp, stops immediately, and verifies the target volume.  
* Ensures proper state transitions from READY to STARTED, back to READY, then CLOSED after closing.
*
* **Test Group ID:** 02@n
* **Test Case ID:** 008@n
*
* **Test Procedure:**
* Refer to UT specification documentation [audioSink_High-Level_TestSpecification.md](../../docs/pages/audioSink_High-Level_TestSpecification.md)
*/
UT_ADD_TEST(AudioSinkBaseTest, TestAudioSinkLifecycle) {
    for (size_t i = 0; i < mAudioSinks.size(); ++i) {
        sp<IAudioSink> sink = mAudioSinks[i];
        sp<IAudioSinkController> controller;
        sp<AudioSinkControllerListenerImpl> controllerlistener = new AudioSinkControllerListenerImpl();

        // Open the audio sink
        UT_LOG_INFO("Calling open(IN:controllerlistener[0x%X], OUT:controller[])", controllerlistener.get());
        Status status = sink->open(controllerlistener, &controller);
        UT_LOG_INFO("Result open(OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controller.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        ::com::rdk::hal::State currentState;
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState:[%s]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Start the audio sink
        UT_LOG_INFO("Calling start()");
        status = controller->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
 
        // Verify the state is STARTED
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:state:[%d]), status:[isOK:%s, exception:%s]", currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::STARTED);

        // Set volume ramp
        double targetVolume = 0.5;
        int32_t overMs = 1000;
        VolumeRamp volumeRamp = VolumeRamp::LINEAR;
        bool volumeRampStarted;
        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:volumeRampStarted)", targetVolume, overMs, static_cast<int32_t>(volumeRamp));
        status = controller->setVolumeRamp(targetVolume, overMs, volumeRamp, &volumeRampStarted);
        UT_LOG_INFO("Result setVolumeRamp(OUT:volumeRampStarted:[%d]), status:[isOK:%s, exception:%s]", volumeRampStarted, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(volumeRampStarted);

        // Stop the audio sink
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        
        // Verify state is READY
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState[%s]), status:[isOK:%s, exception:%s]",
            currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::READY);

        // Verify volume is set to target volume
        Volume volume;
        do {
            UT_LOG_INFO("Calling getVolume(OUT:volume)");
            status = controller->getVolume(&volume);
            UT_LOG_INFO("Result getVolume(OUT:volume:[%f]), status:[isOK:%s, exception:%s]", volume.volume, UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
        } while (volume.volume != targetVolume);
        UT_ASSERT_EQUAL(volume.volume, targetVolume);

        // Close the audio sink
        bool closed = false;
        UT_LOG_INFO("Calling close(IN:controller:[0x%X], OUT:closed)", controller.get());
        status = sink->close(controller, &closed);
        UT_LOG_INFO("Result close(IN:controller:[0x%X], OUT:closed:[%d]), status:[isOK:%s, exception:%s]",
                    controller.get(), closed,UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);

        // Verify state is CLOSED
        UT_LOG_INFO("Calling getState(OUT:currentState[])");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:currentState[%s]), status:[isOK:%s, exception:%s]",
            currentState, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(currentState, State::CLOSED);
    }
}

int main(int argc, char **argv)
{
    UT_status_t result;
    UT_init(argc, argv);
    result = UT_run_tests();
    return result;
}


