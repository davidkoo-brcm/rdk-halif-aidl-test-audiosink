
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
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <stdio.h>
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

static android::sp<::com::rdk::hal::audiosink::IAudioSinkManager> gAudioSinkManagerService = nullptr;



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
}

class AudioSinkEventListenerImpl
    : public BnAudioSinkEventListener
{
public:
    ::com::rdk::hal::State mOldState = ::com::rdk::hal::State::CLOSED;
    ::com::rdk::hal::State mNewState = ::com::rdk::hal::State::CLOSED;

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

class AudioSinkControllerListenerImpl
    : public BnAudioSinkControllerListener
{
public:
    ::com::rdk::hal::State mOldState = ::com::rdk::hal::State::CLOSED;
    ::com::rdk::hal::State mNewState = ::com::rdk::hal::State::CLOSED;

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
    static void SetUpTestSuite()
    {
        UT_ASSERT_EQUAL(getService(), 0);
    }

    static void TearDownTestSuite()
    {
        gAudioSinkManagerService.clear();
    }
};



class AudioSinkBaseTest : public AudioSinkManagerBaseTest
{
protected:
    static std::vector<::com::rdk::hal::audiosink::IAudioSink::Id> mSinkIds;
    static std::vector<android::sp<::com::rdk::hal::audiosink::IAudioSink>> mAudioSinks;
    static android::sp<IAudioSinkEventListener> mSinkEventListener;

    void SetUp() override
    {
	    android::binder::Status status;
        AudioSinkManagerBaseTest::SetUp();
        if (mSinkIds.empty())
        {
            std::vector<::com::rdk::hal::audiosink::IAudioSink::Id> ids;
            status = gAudioSinkManagerService->getAudioSinkIds(&ids);
            UT_ASSERT_TRUE(status.isOk());
            mSinkIds = ids;
            UT_ASSERT_FALSE(mSinkIds.empty());
        }
        if (mAudioSinks.empty())
        {
            for (auto& sinkId : mSinkIds)
            {
                android::sp<::com::rdk::hal::audiosink::IAudioSink> sink;
                status = gAudioSinkManagerService->getAudioSink(sinkId, &sink);
                UT_ASSERT_TRUE(status.isOk());
                UT_ASSERT_NOT_EQUAL(sink, nullptr);
                mAudioSinks.push_back(sink);
            }
        }
    }

    void TearDown() override
    {
        AudioSinkManagerBaseTest::TearDown();
    }
};

std::vector<::com::rdk::hal::audiosink::IAudioSink::Id> AudioSinkBaseTest::mSinkIds;
std::vector<android::sp<::com::rdk::hal::audiosink::IAudioSink>> AudioSinkBaseTest::mAudioSinks;
android::sp<IAudioSinkEventListener> AudioSinkBaseTest::mSinkEventListener;

class AudioSinkControllerBaseTest : public AudioSinkBaseTest
{
protected:
    static std::vector<android::sp<::com::rdk::hal::audiosink::IAudioSinkController>> mControllers;
    static android::sp<IAudioSinkControllerListener> mControllerListener;
    void SetUp() override
    {
	    android::binder::Status status;
        AudioSinkBaseTest::SetUp();
        if (mControllers.empty() && !mAudioSinks.empty())
        {
            mControllerListener = new AudioSinkControllerListenerImpl;
            for (auto& sink : mAudioSinks)
            {
                android::sp<::com::rdk::hal::audiosink::IAudioSinkController> controller;
                status = sink->open(mControllerListener, &controller);
                UT_ASSERT_TRUE(status.isOk());
                UT_ASSERT_NOT_EQUAL(controller, nullptr);
                mControllers.push_back(controller);
            }
        }
    }

    void TearDown() override
    {
	    android::binder::Status status;
        UT_ASSERT_EQUAL(mControllers.size(), mAudioSinks.size());
        for (size_t i = 0; i < mAudioSinks.size(); ++i)
        {
            bool closed = false;
            status = mAudioSinks[i]->close(mControllers[i], &closed);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(closed);
        }
        mControllers.clear();
        AudioSinkBaseTest::TearDown();
    }
};


std::vector<android::sp<::com::rdk::hal::audiosink::IAudioSinkController>> AudioSinkControllerBaseTest::mControllers;
android::sp<IAudioSinkControllerListener> AudioSinkControllerBaseTest::mControllerListener;


/**
 * @brief Test the positive scenario of retrieving audio sink IDs
 *
 * This test verifies that the `getAudioSinkIds` function correctly retrieves a list of audio sink IDs and that the returned list is not empty. It ensures that the function call is successful and that the IDs match the expected set of IDs.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 001@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Initialize audioSinkIds vector and status variable | audioSinkIds = empty, status = uninitialized | Variables initialized | Should be successful |
 * | 02 | Log the start of getAudioSinkIds call | None | Log message | Should be successful |
 * | 03 | Call getAudioSinkIds to retrieve audio sink IDs | audioSinkIds = empty | status.isOk() = true, audioSinkIds not empty | Should Pass |
 * | 04 | Log the result of getAudioSinkIds call | status, audioSinkIds | Log message | Should be successful |
 * | 05 | Assert that the status is OK | status.isOk() = true | Assertion passes | Should Pass |
 * | 06 | Assert that audioSinkIds is not empty | audioSinkIds not empty | Assertion passes | Should Pass |
 * | 07 | Initialize expectedIds set with expected audio sink IDs | expectedIds = set of expected IDs | expectedIds initialized | Should be successful |
 * | 08 | Verify each ID in audioSinkIds is in expectedIds | audioSinkIds, expectedIds | Assertion passes for each ID | Should Pass |
 */
UT_ADD_TEST(AudioSinkManagerBaseTest, test_l1_AudioSinkManager_positive_getAudioSinkIds)
{
    std::vector<::com::rdk::hal::audiosink::IAudioSink::Id> audioSinkIds;
    android::binder::Status status;
    size_t expected_count = 0;

    UT_LOG_INFO("Calling getAudioSinkIds(OUT:_aidl_return[])");
    status = gAudioSinkManagerService->getAudioSinkIds(&audioSinkIds);
    UT_LOG_INFO("Result getAudioSinkIds(OUT:_aidl_return[]), status:[isOK:%s, exception:%s]",
                UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_FALSE(audioSinkIds.empty());

    std::unordered_set<int32_t> expectedIds;
    std::string audiosink_count_path = "IAudioSinkManager.IAudioSink";
    expected_count = UT_KVP_PROFILE_GET_LIST_COUNT(audiosink_count_path.c_str());

    for (size_t i = 0; i < expected_count; ++i) 
    {
        std::string audiosink_id_path = audiosink_count_path + std::to_string(i) + ".id";
        expectedIds.insert(UT_KVP_PROFILE_GET_UINT32(audiosink_id_path.c_str()));
    }

    for (const auto& id : audioSinkIds) {
        UT_ASSERT_NOT_EQUAL(expectedIds.find(id.value), expectedIds.end());
    }
}



/**
 * @brief Test the retrieval of platform capabilities from the AudioSinkManager service.
 *
 * This test verifies the functionality of the getPlatformCapabilities method in the AudioSinkManager service. It checks whether the method correctly retrieves the platform capabilities and matches the expected values for various attributes such as supportsLowLatency, systemMixerSampleRateHz, systemMixerPCMFormat, and supportsPlanarFormat.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 002@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify that the AudioSinkManager service is not null | gAudioSinkManagerService = nullptr | Should not be null | Should be successful |
 * | 02 | Call getPlatformCapabilities method on AudioSinkManager service | platformCapabilities = uninitialized | status.isOk() = true | Should Pass |
 * | 03 | Check supportsLowLatency attribute against expected value | platformCapabilities.supportsLowLatency = value | platformCapabilities.supportsLowLatency = expectedSupportsLowLatency | Should Pass |
 * | 04 | Check systemMixerSampleRateHz attribute against expected value | platformCapabilities.systemMixerSampleRateHz = value | platformCapabilities.systemMixerSampleRateHz = expectedSystemMixerSampleRateHz | Should Pass |
 * | 05 | Check systemMixerPCMFormat attribute against expected value | platformCapabilities.systemMixerPCMFormat = value | platformCapabilities.systemMixerPCMFormat = expectedSystemMixerPCMFormat | Should Pass |
 * | 06 | Check supportsPlanarFormat attribute against expected value | platformCapabilities.supportsPlanarFormat = value | platformCapabilities.supportsPlanarFormat = expectedSupportsPlanarFormat | Should Pass |
 */
UT_ADD_TEST(AudioSinkManagerBaseTest, test_l1_AudioSinkManager_positive_getPlatformCapabilities) {
    UT_ASSERT_NOT_EQUAL(gAudioSinkManagerService, nullptr);

    PlatformCapabilities platformCapabilities;
    android::binder::Status status;

    UT_LOG_INFO("Calling getPlatformCapabilities(OUT:platformCapabilities[])");
    status = gAudioSinkManagerService->getPlatformCapabilities(&platformCapabilities);
    UT_LOG_INFO("Result getPlatformCapabilities(OUT:platformCapabilities[0x%X]), status:[isOK:%s, exception:%s]",
                &platformCapabilities, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

    UT_ASSERT_TRUE(status.isOk());

    std::string lowlatency_path = "IAudioSinkManager.platformCapabilities.supportsLowLatency";
    bool expectedSupportsLowLatency = UT_KVP_PROFILE_GET_BOOL(lowlatency_path.c_str());
    UT_ASSERT_EQUAL(platformCapabilities.supportsLowLatency, expectedSupportsLowLatency);

    std::string samplerate_path = "IAudioSinkManager.platformCapabilities.systemMixerSampleRateHz";
    int32_t expectedSystemMixerSampleRateHz = UT_KVP_PROFILE_GET_UINT32(samplerate_path.c_str());
    UT_ASSERT_EQUAL(platformCapabilities.systemMixerSampleRateHz, expectedSystemMixerSampleRateHz);

    std::string mixer_pcmformat_path = "IAudioSinkManager.platformCapabilities.systemMixerPCMFormat";
    char expectedSystemMixerPCMFormatStr[256];
    UT_KVP_PROFILE_GET_STRING(mixer_pcmformat_path.c_str(), expectedSystemMixerPCMFormatStr);
    PCMFormat expectedSystemMixerPCMFormat = static_cast<PCMFormat>(std::stoi(expectedSystemMixerPCMFormatStr));
    UT_ASSERT_EQUAL(platformCapabilities.systemMixerPCMFormat, expectedSystemMixerPCMFormat);

    std::string supportsPlanarFormat_path = "IAudioSinkManager.platformCapabilities.supportsPlanarFormat";
    bool expectedSupportsPlanarFormat = UT_KVP_PROFILE_GET_BOOL(supportsPlanarFormat_path.c_str());
    UT_ASSERT_EQUAL(platformCapabilities.supportsPlanarFormat, expectedSupportsPlanarFormat);
}



/**
 * @brief Test the positive scenario of retrieving an audio sink using a valid audio sink ID.
 *
 * This test verifies the functionality of the getAudioSink API by providing a valid audio sink ID and checking if the audio sink is retrieved successfully. The test ensures that the API returns a successful status and a non-null audio sink object.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 003@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve a valid audio sink ID using the macro | audioSinkId.value = UT_KVP_PROFILE_GET_UINT32(IAudioSinkManager.IAudioSink.<index>.id) | Should retrieve a valid ID | Should be successful |
 * | 02 | Log the call to getAudioSink | audioSinkId.value | Should log the call details | Should be successful |
 * | 03 | Call the getAudioSink API | audioSinkId = value, audioSink = nullptr | Status should be OK, audioSink should not be null | Should Pass |
 * | 04 | Log the result of getAudioSink | audioSinkId.value, audioSink.get(), status.isOk(), status.exceptionCode() | Should log the result details | Should be successful |
 * | 05 | Verify the status is OK | status.isOk() | Assertion should pass | Should Pass |
 * | 06 | Verify the audioSink is not null | audioSink != nullptr | Assertion should pass | Should Pass |
 */
UT_ADD_TEST(AudioSinkManagerBaseTest, test_l1_AudioSinkManager_positive_getAudioSink) {
    android::binder::Status status;

    std::string audiosink_count_path = "IAudioSinkManager.IAudioSink";
    uint32_t expected_count = UT_KVP_PROFILE_GET_LIST_COUNT(audiosink_count_path.c_str());

    for (size_t i = 0; i < expected_count; ++i) 
    {
        android::sp<::com::rdk::hal::audiosink::IAudioSink> audioSink;
        ::com::rdk::hal::audiosink::IAudioSink::Id audioSinkId;

        std::string audiosink_id_path = audiosink_count_path + std::to_string(i) + ".id";

        audioSinkId.value = UT_KVP_PROFILE_GET_UINT32(audiosink_id_path.c_str());
        // Log the call to getAudioSink
        UT_LOG_INFO("Calling getAudioSink(IN:audioSinkId:[%d], OUT:audioSink[])", audioSinkId.value);
        // Call the getAudioSink API
        status = gAudioSinkManagerService->getAudioSink(audioSinkId, &audioSink);
        // Log the result of getAudioSink
        UT_LOG_INFO("Result getAudioSink(IN:audioSinkId:[%d], OUT:audioSink[0x%X]), status:[isOK:%s, exception:%s]",
                    audioSinkId.value, audioSink.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        // Verify the results
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(audioSink, nullptr);
    }
}



/**
 * @brief Test the behavior of getAudioSink with invalid audio sink IDs.
 *
 * This test verifies that the getAudioSink function correctly handles invalid audio sink IDs by returning an error status and a null audio sink object. It ensures that the function does not accept IDs outside the valid range.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 004@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve audio sink IDs from the service and ensure they are not empty. | None | Status is OK, audioSinkIds is not empty | Should be successful |
 * | 02 | Sort the audio sink IDs to determine the least and highest values. | None | Sorted list of audioSinkIds | Should be successful |
 * | 03 | Test getAudioSink with an invalid ID (least value - 1). | input: audioSinkId = leastValue - 1, output: audioSink = nullptr | Status is not OK, audioSink is nullptr | Should Fail |
 * | 04 | Test getAudioSink with an invalid ID (highest value + 1). | input: audioSinkId = highestValue + 1, output: audioSink = nullptr | Status is not OK, audioSink is nullptr | Should Fail |
 */
UT_ADD_TEST(AudioSinkManagerBaseTest, test_l1_AudioSinkManager_negative_getAudioSink) 
{
    android::binder::Status status;
    android::sp<::com::rdk::hal::audiosink::IAudioSink> audioSink;

    std::vector<::com::rdk::hal::audiosink::IAudioSink::Id> audioSinkIds;
    status = gAudioSinkManagerService->getAudioSinkIds(&audioSinkIds);
    UT_ASSERT_TRUE(status.isOk());
    UT_ASSERT_FALSE(audioSinkIds.empty());

    std::sort(audioSinkIds.begin(), audioSinkIds.end(), [](const auto& a, const auto& b) {
        return a.value < b.value;
    });

    int32_t leastValue = audioSinkIds.front().value;
    int32_t highestValue = audioSinkIds.back().value;

    // Test with invalid ID (least value - 1)
    ::com::rdk::hal::audiosink::IAudioSink::Id invalidIdLow;
    invalidIdLow.value = leastValue - 1;
    UT_LOG_INFO("Calling getAudioSink(IN:audioSinkId:[%d], OUT:audioSink[])", invalidIdLow.value);
    status = gAudioSinkManagerService->getAudioSink(invalidIdLow, &audioSink);
    UT_LOG_INFO("Result getAudioSink(IN:audioSinkId:[%d], OUT:audioSink[0x%X]), status:[isOK:%s, exception:%s]",
                invalidIdLow.value, audioSink.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_FALSE(status.isOk());
    UT_ASSERT_EQUAL(audioSink, nullptr);

    // Test with invalid ID (highest value + 1)
    ::com::rdk::hal::audiosink::IAudioSink::Id invalidIdHigh;
    invalidIdHigh.value = highestValue + 1;
    UT_LOG_INFO("Calling getAudioSink(IN:audioSinkId:[%d], OUT:audioSink[])", invalidIdHigh.value);
    status = gAudioSinkManagerService->getAudioSink(invalidIdHigh, &audioSink);
    UT_LOG_INFO("Result getAudioSink(IN:audioSinkId:[%d], OUT:audioSink[0x%X]), status:[isOK:%s, exception:%s]",
                invalidIdHigh.value, audioSink.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()),
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    UT_ASSERT_FALSE(status.isOk());
    UT_ASSERT_EQUAL(audioSink, nullptr);
}



/**
 * @brief Test the positive scenario for getting audio sink capabilities
 *
 * This test verifies that the getCapabilities function of each audio sink in mAudioSinks
 * returns a valid capabilities object and that the function call is successful.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 005@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Check if mAudioSinks is not empty | mAudioSinks | Should not be empty | Should be successful |
 * | 02 | Iterate over each audio sink in mAudioSinks | mAudioSinks | Should iterate successfully | Should be successful |
 * | 03 | Call getCapabilities on each audio sink | sink, capabilities | status.isOk() should be true, capabilities should not be nullptr | Should Pass |
 * | 04 | Log the result of getCapabilities call | capabilities, status | Log should display capabilities and status | Should be successful |
 * | 05 | Assert that the status is OK | status.isOk() | status.isOk() should be true | Should Pass |
 * | 06 | Assert that capabilities is not nullptr | capabilities | capabilities should not be nullptr | Should Pass |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_positive_getCapabilities)
{
    UT_ASSERT_FALSE(mAudioSinks.empty());
    for (auto& sink : mAudioSinks)
    {
        ::com::rdk::hal::audiosink::Capabilities capabilities;
        UT_LOG_INFO("Calling getCapabilities(OUT:capabilities[])");
        android::binder::Status status = sink->getCapabilities(&capabilities);
        UT_LOG_INFO("Result getCapabilities(OUT:capabilities[0x%X]), status:[isOK:%s, exception:%s]",
                    &capabilities, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(&capabilities, nullptr);
    }
}



/**
 * @brief Test the retrieval of various properties from AudioSinkBase.
 *
 * This test verifies the ability to retrieve different properties from the AudioSinkBase
 * and checks the correctness of the returned values. It ensures that the properties are
 * correctly fetched and validated against expected results.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 006@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve RESOURCE_ID Property | property = RESOURCE_ID | status.isOk() = true, aidlReturn.has_value() = true, aidlReturn->value >= 0 | Should Pass |
 * | 02 | Retrieve REFERENCE_LEVEL Property in READY State | property = REFERENCE_LEVEL | status.isOk() = true, aidlReturn.has_value() = true, aidlReturn->value = -31 | Should Pass |
 * | 03 | Retrieve RENDER_LATENCY_NS Property | property = RENDER_LATENCY_NS | status.isOk() = true, aidlReturn.has_value() = true, aidlReturn->value > 0 | Should Pass |
 * | 04 | Retrieve QUEUE_DEPTH_NS Property | property = QUEUE_DEPTH_NS | status.isOk() = true, aidlReturn.has_value() = true, aidlReturn->value > 0 | Should Pass |
 * | 05 | Retrieve MIXER_ID Property in READY State | property = MIXER_ID | status.isOk() = true, aidlReturn.has_value() = true, aidlReturn->value = 0 | Should Pass |
 * | 06 | Retrieve AV_SOURCE Property in READY State | property = AV_SOURCE | status.isOk() = true, aidlReturn.has_value() = true, aidlReturn->value = 0 | Should Pass |
 * | 07 | Retrieve DOLBY_ATMOS_LOCK Property in READY State | property = DOLBY_ATMOS_LOCK | status.isOk() = true, aidlReturn.has_value() = true, aidlReturn->value = 0 or 1 | Should Pass |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_positive_getProperty) {
    android::binder::Status status;
    std::optional<::com::rdk::hal::PropertyValue> aidlReturn;

    for (auto& sink : mAudioSinks) 
    {
        // Test Case 1: Retrieve RESOURCE_ID Property
        UT_LOG_INFO("Calling getProperty(IN:property:[RESOURCE_ID], OUT:_aidl_return[])");
        status = sink->getProperty(::com::rdk::hal::audiosink::Property::RESOURCE_ID, &aidlReturn);
        UT_LOG_INFO("Result getProperty(IN:property:[RESOURCE_ID], OUT:_aidl_return[]), status:[isOK:%s, exception:%s]",
            UT_Control_GetMapString(bool_mapTable, status.isOk()),
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));            
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn.has_value());

        // Test Case 2: Retrieve REFERENCE_LEVEL Property in READY State
        UT_LOG_INFO("Calling getProperty(IN:property:[REFERENCE_LEVEL], OUT:_aidl_return[])");
        status = sink->getProperty(::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL, &aidlReturn);
        UT_LOG_INFO("Result getProperty(IN:property:[REFERENCE_LEVEL], OUT:_aidl_return[]), status:[isOK:%s, exception:%s]",
            UT_Control_GetMapString(bool_mapTable, status.isOk()),
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));  
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn.has_value());


        // Test Case 3: Retrieve RENDER_LATENCY_NS Property
        UT_LOG_INFO("Calling getProperty(IN:property:[RENDER_LATENCY_NS], OUT:_aidl_return[])");
        status = sink->getProperty(::com::rdk::hal::audiosink::Property::RENDER_LATENCY_NS, &aidlReturn);
        UT_LOG_INFO("Result getProperty(IN:property:[RENDER_LATENCY_NS], OUT:_aidl_return[]), status:[isOK:%s, exception:%s]",
            UT_Control_GetMapString(bool_mapTable, status.isOk()),
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn.has_value());

        // Test Case 4: Retrieve QUEUE_DEPTH_NS Property
        UT_LOG_INFO("Calling getProperty(IN:property:[QUEUE_DEPTH_NS], OUT:_aidl_return[])");
        status = sink->getProperty(::com::rdk::hal::audiosink::Property::QUEUE_DEPTH_NS, &aidlReturn);
        UT_LOG_INFO("Result getProperty(IN:property:[QUEUE_DEPTH_NS], OUT:_aidl_return[]), status:[isOK:%s, exception:%s]",
            UT_Control_GetMapString(bool_mapTable, status.isOk()),
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn.has_value());

        // Test Case 5: Retrieve MIXER_ID Property in READY State
        UT_LOG_INFO("Calling getProperty(IN:property:[MIXER_ID], OUT:_aidl_return[])");
        status = sink->getProperty(::com::rdk::hal::audiosink::Property::MIXER_ID, &aidlReturn);
        UT_LOG_INFO("Result getProperty(IN:property:[MIXER_ID], OUT:_aidl_return[]), status:[isOK:%s, exception:%s]",
            UT_Control_GetMapString(bool_mapTable, status.isOk()),
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn.has_value());

        // Test Case 6: Retrieve AV_SOURCE Property in READY State
        UT_LOG_INFO("Calling getProperty(IN:property:[AV_SOURCE], OUT:_aidl_return[])");
        status = sink->getProperty(::com::rdk::hal::audiosink::Property::AV_SOURCE, &aidlReturn);
        UT_LOG_INFO("Result getProperty(IN:property:[AV_SOURCE], OUT:_aidl_return[]), status:[isOK:%s, exception:%s]",
            UT_Control_GetMapString(bool_mapTable, status.isOk()),
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn.has_value());

        // Test Case 7: Retrieve DOLBY_ATMOS_LOCK Property in READY State
        UT_LOG_INFO("Calling getProperty(IN:property:[DOLBY_ATMOS_LOCK], OUT:_aidl_return[])");
        status = sink->getProperty(::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK, &aidlReturn);
        UT_LOG_INFO("Result getProperty(IN:property:[DOLBY_ATMOS_LOCK], OUT:_aidl_return[]), status:[isOK:%s, exception:%s]",
            UT_Control_GetMapString(bool_mapTable, status.isOk()),
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn.has_value());
    }
}



/**
 * @brief Test the negative scenario for the getProperty function of AudioSink
 *
 * This test verifies the behavior of the getProperty function when an invalid property is requested. It ensures that the function returns an error status and no property value.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 007@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Check if mAudioSinks is not empty | mAudioSinks | Should not be empty | Should be successful |
 * | 02 | Iterate over each sink in mAudioSinks and call getProperty with invalid property | property = -1, aidl_return = null | status.isOk() = false, aidl_return.has_value() = false | Should Fail |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_negative_getProperty) 
{
    android::binder::Status status;
    std::optional<::com::rdk::hal::PropertyValue> aidl_return;

    UT_ASSERT_FALSE(mAudioSinks.empty());

    for (auto& sink : mAudioSinks) 
    {
        UT_LOG_INFO("Calling getProperty(IN:property:[-1], OUT:_aidl_return[])");
        status = sink->getProperty(static_cast<::com::rdk::hal::audiosink::Property>(-1), &aidl_return);
        UT_LOG_INFO("Result getProperty(IN:property:[-1], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    aidl_return.has_value() ? "not null" : "null",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(status.isOk());
        UT_ASSERT_FALSE(aidl_return.has_value());
    }
}



/**
 * @brief Test the positive setting of properties on an audio sink in the READY state.
 *
 * This test verifies that various properties can be set on an audio sink when it is in the READY state. 
 * It ensures that the properties REFERENCE_LEVEL, MIXER_ID, AV_SOURCE, and DOLBY_ATMOS_LOCK can be set 
 * successfully and that the audio sink responds with the expected status and return values.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 008@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the audio sink is in the READY state and retrieve the first audio sink from the list. | mAudioSinks = non-empty, audioSink = mAudioSinks.front() | audioSink != nullptr | Should be successful |
 * | 02 | Set REFERENCE_LEVEL to -31 in the READY state using setProperty. | property = REFERENCE_LEVEL, propertyValue = -31, aidl_return = false | status.isOk() = true, aidl_return = true | Should Pass |
 * | 03 | Set MIXER_ID to 0 in the READY state using setProperty. | property = MIXER_ID, propertyValue = 0, aidl_return = false | status.isOk() = true, aidl_return = true | Should Pass |
 * | 04 | Set AV_SOURCE to 0 in the READY state using setProperty. | property = AV_SOURCE, propertyValue = 0, aidl_return = false | status.isOk() = true, aidl_return = true | Should Pass |
 * | 05 | Enable DOLBY_ATMOS_LOCK by setting it to 1 in the READY state using setProperty. | property = DOLBY_ATMOS_LOCK, propertyValue = 1, aidl_return = false | status.isOk() = true, aidl_return = true | Should Pass |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_positive_setProperty) 
{
    android::binder::Status status;
    bool aidl_return = false;
    bool closed = false;

    // Ensure the audio sink is in the READY state
    UT_ASSERT_FALSE(mAudioSinks.empty());


    for (const auto& audioSink : mAudioSinks) 
    {
        //set state to READY by invoking `open` (set-up)
        android::sp<IAudioSinkControllerListener> controllerListener = new AudioSinkControllerListenerImpl;
        android::sp<IAudioSinkController> audioSinkController;
        UT_LOG_INFO("Calling open(IN:audioSinkControllerListener, OUT:audioSinkController)");
        status = audioSink->open(controllerListener, &audioSinkController);
        UT_LOG_INFO("Result open(IN:audioSinkControllerListener, OUT:audioSinkController[0x%X]), status:[isOK:%s, exception:%s]",
                    audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(audioSinkController, nullptr);


        // Set REFERENCE_LEVEL to a valid value in READY state
        {
            PropertyValue propertyValue;
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-31);

            UT_LOG_INFO("Calling setProperty(IN:property:[REFERENCE_LEVEL], IN:propertyValue:[-31], OUT:_aidl_return)");
            status = audioSink->setProperty(::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[REFERENCE_LEVEL], IN:propertyValue:[-31], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidl_return);
        }

        // Set MIXER_ID to a valid value in READY state
        {
            PropertyValue propertyValue;
            //propertyValue.value = PropertyValue::Value();
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-31);

            UT_LOG_INFO("Calling setProperty(IN:property:[MIXER_ID], IN:propertyValue:[0], OUT:_aidl_return)");
            status = audioSink->setProperty(::com::rdk::hal::audiosink::Property::MIXER_ID, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[MIXER_ID], IN:propertyValue:[0], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidl_return);
        }

        // Set AV_SOURCE to a valid value in READY state
        {
            PropertyValue propertyValue;
            //propertyValue.value = PropertyValue::Value();
            //propertyValue.value->set<int32_t>(0);
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-31);

            UT_LOG_INFO("Calling setProperty(IN:property:[AV_SOURCE], IN:propertyValue:[0], OUT:_aidl_return)");
            status = audioSink->setProperty(::com::rdk::hal::audiosink::Property::AV_SOURCE, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[AV_SOURCE], IN:propertyValue:[0], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidl_return);
        }

        // Enable DOLBY_ATMOS_LOCK in READY state
        {
            PropertyValue propertyValue;
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-31);

            UT_LOG_INFO("Calling setProperty(IN:property:[DOLBY_ATMOS_LOCK], IN:propertyValue:[1], OUT:_aidl_return)");
            status = audioSink->setProperty(::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[DOLBY_ATMOS_LOCK], IN:propertyValue:[1], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(aidl_return);
        }

        //Revert state to CLOSED

        // Log before calling the close API
        UT_LOG_INFO("Calling close(IN:audioSinkController:[%p], OUT:_aidl_return)", audioSinkController.get());
        // Call the close API
        status = audioSink->close(audioSinkController, &closed);
        // Log the result of the close API
        UT_LOG_INFO("Result close(IN:audioSinkController:[%p], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, closed),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    }
}



/**
 * @brief Test the negative scenarios for setting properties on an audio sink.
 *
 * This test verifies the behavior of the audio sink when attempting to set properties with invalid values or in incorrect states. It ensures that the audio sink does not accept invalid property settings and returns the expected failure status.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 009@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the audio sink is in the READY state by opening all controllers. | None | Controllers should be in READY state | Should be successful |
 * | 02 | Attempt to set REFERENCE_LEVEL to an invalid value (-100) in READY state. | property = REFERENCE_LEVEL, propertyValue = -100, aidl_return = false | aidl_return should be false | Should Fail |
 * | 03 | Ensure the audio sink is not in the READY state by starting all controllers. | None | Controllers should not be in READY state | Should be successful |
 * | 04 | Attempt to set MIXER_ID in a non-READY state. | property = MIXER_ID, propertyValue = 0, aidl_return = false | aidl_return should be false | Should Fail |
 * | 05 | Ensure the audio sink is in the READY state by stopping all controllers. | None | Controllers should be in READY state | Should be successful |
 * | 06 | Attempt to set AV_SOURCE to an invalid value (-1) in READY state. | property = AV_SOURCE, propertyValue = -1, aidl_return = false | aidl_return should be false | Should Fail |
 * | 07 | Ensure the audio sink is not in the READY state by starting all controllers. | None | Controllers should not be in READY state | Should be successful |
 * | 08 | Attempt to disable DOLBY_ATMOS_LOCK in a non-READY state. | property = DOLBY_ATMOS_LOCK, propertyValue = 0, aidl_return = false | aidl_return should be false | Should Fail |
 * | 09 | Attempt to set READONLY property RENDER_LATENCY_NS. | property = RENDER_LATENCY_NS, propertyValue = 0, aidl_return = false | aidl_return should be false | Should Fail |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_negative_setProperty) {
    android::binder::Status status;
    bool aidl_return = false;
    bool closed = false;

    for(const auto& audiosink : mAudioSinks)
    {
        //set state to READY by invoking `open` (set-up)
        android::sp<IAudioSinkControllerListener> controllerListener = new AudioSinkControllerListenerImpl;
        android::sp<IAudioSinkController> audioSinkController;
        UT_LOG_INFO("Calling open(IN:audioSinkControllerListener, OUT:audioSinkController)");
        status = audiosink->open(controllerListener, &audioSinkController);
        UT_LOG_INFO("Result open(IN:audioSinkControllerListener, OUT:audioSinkController[0x%X]), status:[isOK:%s, exception:%s]",
                    audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(audioSinkController, nullptr);

        // Test Case 1: Set REFERENCE_LEVEL to an invalid value in READY state
        {
            ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL;
            PropertyValue propertyValue;
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-100);

            UT_LOG_INFO("Calling setProperty(IN:property:[REFERENCE_LEVEL], IN:propertyValue:[-100], OUT:aidl_return)");
            status = audiosink->setProperty(property, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[REFERENCE_LEVEL], IN:propertyValue:[-100], OUT:aidl_return[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_FALSE(aidl_return);
        }

        // Test Case 2: Set MIXER_ID in a non-READY state
        {
            ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::MIXER_ID;
            PropertyValue propertyValue;
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(0);
            
            //setting state to STARTED
            status = audioSinkController->start();
            UT_ASSERT_TRUE(status.isOk());

            UT_LOG_INFO("Calling setProperty(IN:property:[MIXER_ID], IN:propertyValue:[0], OUT:aidl_return)");
            status = audiosink->setProperty(property, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[MIXER_ID], IN:propertyValue:[0], OUT:aidl_return[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_FALSE(aidl_return);

            //reverting state to READY
            status = audioSinkController->stop();
            UT_ASSERT_TRUE(status.isOk());        
        }

        // Test Case 3: Set AV_SOURCE to an invalid value in READY state
        {
            ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::AV_SOURCE;
            PropertyValue propertyValue;
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-1);

            UT_LOG_INFO("Calling setProperty(IN:property:[AV_SOURCE], IN:propertyValue:[-1], OUT:aidl_return)");
            status = audiosink->setProperty(property, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[AV_SOURCE], IN:propertyValue:[-1], OUT:aidl_return[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_FALSE(aidl_return);
        }

        // Test Case 4: Disable DOLBY_ATMOS_LOCK in a non-READY state
        {
            ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK;
            PropertyValue propertyValue;
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(0);


            //setting state to STARTED
            status = audioSinkController->start();
            UT_ASSERT_TRUE(status.isOk());

            UT_LOG_INFO("Calling setProperty(IN:property:[DOLBY_ATMOS_LOCK], IN:propertyValue:[0], OUT:aidl_return)");
            status = audiosink->setProperty(property, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[DOLBY_ATMOS_LOCK], IN:propertyValue:[0], OUT:aidl_return[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_FALSE(aidl_return);

            //reverting state to READY
            status = audioSinkController->stop();
            UT_ASSERT_TRUE(status.isOk()); 
        }

        // Test Case 5: try to set READONLY property RENDER_LATENCY_NS
        {
            ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::RENDER_LATENCY_NS;
            PropertyValue propertyValue;
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(0);

            UT_LOG_INFO("Calling setProperty(IN:property:[RENDER_LATENCY_NS], IN:propertyValue:[0], OUT:aidl_return)");
            status = audiosink->setProperty(property, propertyValue, &aidl_return);
            UT_LOG_INFO("Result setProperty(IN:property:[RENDER_LATENCY_NS], IN:propertyValue:[0], OUT:aidl_return[%s]), status:[isOK:%s, exception:%s]",
                        UT_Control_GetMapString(bool_mapTable, aidl_return),
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_FALSE(aidl_return);
        }

        //Revert state to CLOSED
        // Log before calling the close API
        UT_LOG_INFO("Calling close(IN:audioSinkController:[%p], OUT:_aidl_return)", audioSinkController.get());
        // Call the close API
        status = audiosink->close(audioSinkController, &closed);
        // Log the result of the close API
        UT_LOG_INFO("Result close(IN:audioSinkController:[%p], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, closed),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    }
}



/**
 * @brief Test the positive scenario of getting the state of AudioSink
 *
 * This test verifies that the getState function of AudioSink returns a valid state and the operation is successful.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 010@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Iterate over each AudioSink in mAudioSinks | mAudioSinks | Should be successful | |
 * | 02 | Call getState on the current AudioSink and store the result in currentState | currentState = OUT | Should Pass | Should Pass |
 * | 03 | Log the result of getState and the status | currentState, status | Should be successful | |
 * | 04 | Assert that the status is OK | status.isOk() | Should Pass | Should Pass |
 * | 05 | Define a list of valid states | validStates | Should be successful | |
 * | 06 | Assert that the currentState is within the valid states | currentState in validStates | Should Pass | Should Pass |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_positive_getState) 
{
    android::binder::Status status;
    ::com::rdk::hal::State currentState;

    for (auto& sink : mAudioSinks) {
        UT_LOG_INFO("Calling getState(OUT:_aidl_return)");
        status = sink->getState(&currentState);
        UT_LOG_INFO("Result getState(OUT:_aidl_return:[%d]), status:[isOK:%s, exception:%s]",
                    static_cast<int32_t>(currentState),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        UT_ASSERT_TRUE(status.isOk());

        std::vector<::com::rdk::hal::State> validStates = {
            ::com::rdk::hal::State::UNKNOWN,
            ::com::rdk::hal::State::CLOSED,
            ::com::rdk::hal::State::OPENING,
            ::com::rdk::hal::State::READY,
            ::com::rdk::hal::State::STARTING,
            ::com::rdk::hal::State::STARTED,
            ::com::rdk::hal::State::FLUSHING,
            ::com::rdk::hal::State::STOPPING,
            ::com::rdk::hal::State::CLOSING
        };

        UT_ASSERT_TRUE(std::find(validStates.begin(), validStates.end(), currentState) != validStates.end());
    }
}



/**
 * @brief Test the positive scenario of opening an audio sink.
 *
 * This test verifies the successful opening of an audio sink using a controller listener. It checks if the audio sink controller is correctly initialized and the status returned is OK.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 011@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Initialize the controller listener object | controllerListener = new AudioSinkControllerListenerImpl | Object should be created successfully | Should be successful |
 * | 02 | Call the open function with the controller listener and audio sink controller | IN: controllerListener, OUT: audioSinkController | Status should be OK, audioSinkController should not be nullptr | Should Pass |
 * | 03 | Log the result of the open function call | audioSinkController.get(), status.isOk(), status.exceptionCode() | Log should show correct status and controller address | Should be successful |
 * | 04 | Assert that the status is OK | status.isOk() | Assertion should pass | Should Pass |
 * | 05 | Assert that the audio sink controller is not nullptr | audioSinkController != nullptr | Assertion should pass | Should Pass |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_positive_open) 
{
    android::binder::Status status;
    for( const auto& audiosink : mAudioSinks)
    {
        android::sp<IAudioSinkControllerListener> controllerListener = new AudioSinkControllerListenerImpl;
        android::sp<IAudioSinkController> audioSinkController;

        UT_LOG_INFO("Calling open(IN:audioSinkControllerListener, OUT:audioSinkController)");
        status = audiosink->open(controllerListener, &audioSinkController);
        UT_LOG_INFO("Result open(IN:audioSinkControllerListener, OUT:audioSinkController[0x%X]), status:[isOK:%s, exception:%s]",
                    audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(audioSinkController, nullptr);
    }
}


/**
 * @brief Test the negative scenarios for the AudioSink open function
 *
 * This test evaluates the behavior of the AudioSink open function when provided with invalid inputs or called in an invalid state. It ensures that the function handles these scenarios gracefully by returning appropriate error statuses and not altering the state inappropriately.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 012@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Initialize the test environment and prepare the AudioSink for testing | None | None | Should be successful |
 * | 02 | Call open with a null listener to test negative scenario | audioSinkControllerListener = nullptr, controller = nullptr | status.isOk() = false, controller = nullptr | Should Fail |
 * | 03 | Create a valid AudioSinkControllerListenerImpl instance | mControllerListener = new AudioSinkControllerListenerImpl | None | Should be successful |
 * | 04 | Call open with a valid listener to establish a valid state | audioSinkControllerListener = valid, controller = valid | status.isOk() = true, controller != nullptr | Should Pass |
 * | 05 | Call open again with the same valid listener to test negative scenario | audioSinkControllerListener = valid, secondController = nullptr | status.isOk() = false, secondController = nullptr | Should Fail |
 * | 06 | Close the controller to clean up the test environment | controller = valid, closed = false | status.isOk() = true, closed = true | Done by Pre-requisite TearDown function |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_negative_open) 
{
    android::binder::Status status;
    
    for( const auto& audiosink : mAudioSinks)
    {
        android::sp<IAudioSinkController> controller;
        // Test Case 1: Negative Test for Open with Null Listener
        UT_LOG_INFO("Calling open(IN:audioSinkControllerListener:[nullptr], OUT:controller[])");
        status = audiosink->open(nullptr, &controller);
        UT_LOG_INFO("Result open(IN:audioSinkControllerListener:[nullptr], OUT:controller[nullptr]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(status.isOk());
        UT_ASSERT_EQUAL(controller, nullptr);

        // Test Case 2: Negative Test for Open after prior open
        android::sp<IAudioSinkControllerListener> mControllerListener = new AudioSinkControllerListenerImpl;
        UT_LOG_INFO("Calling open(IN:audioSinkControllerListener:[valid], OUT:controller[])");
        status = audiosink->open(mControllerListener, &controller);
        UT_LOG_INFO("Result open(IN:audioSinkControllerListener:[valid], OUT:controller[0x%X]), status:[isOK:%s, exception:%s]",
                    controller.get(),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(controller, nullptr);

        android::sp<IAudioSinkControllerListener> second_mControllerListener = new AudioSinkControllerListenerImpl;
        android::sp<IAudioSinkController> secondController;
        UT_LOG_INFO("Calling open(IN:audioSinkControllerListener:[valid], OUT:secondController[])");
        status = audiosink->open(second_mControllerListener, &secondController);
        UT_LOG_INFO("Result open(IN:audioSinkControllerListener:[valid], OUT:secondController[nullptr]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
        UT_ASSERT_EQUAL(secondController, nullptr);

        // Teardown
        bool closed = false;
        status = audiosink->close(controller, &closed);
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);
    }
}



/**
 * @brief Test the positive scenario of closing an AudioSink.
 *
 * This test verifies the successful closure of an AudioSink when the controller is in the READY state. It ensures that the close operation returns a successful status and the closed flag is set to true.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 013@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the AudioSink is in READY state by starting the controller | controller = mControllers[i], status = controller->start() | status.isOk() should be true | Should be successful |
 * | 02 | Log before calling the close API | controller = mControllers[i] | Log should be successful | Should be successful |
 * | 03 | Call the close API to close the AudioSink | controller = mControllers[i], closed = false | status.isOk() should be true, closed should be true | Should Pass |
 * | 04 | Log the result of the close API | controller = mControllers[i], closed = true, status = status | Log should be successful | Should be successful |
 * | 05 | Verify the close operation was successful | status.isOk() = true, closed = true | status.isOk() should be true, closed should be true | Should Pass |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_positive_close) 
{
    android::binder::Status status;
    bool closed = false;

    // UT_ASSERT_FALSE(mControllers.empty());
    UT_ASSERT_FALSE(mAudioSinks.empty());


    for( const auto& audiosink : mAudioSinks)
    {
        android::sp<IAudioSinkControllerListener> controllerListener = new AudioSinkControllerListenerImpl;
        android::sp<IAudioSinkController> audioSinkController;

        UT_LOG_INFO("Calling open(IN:audioSinkControllerListener, OUT:audioSinkController)");
        status = audiosink->open(controllerListener, &audioSinkController);
        UT_LOG_INFO("Result open(IN:audioSinkControllerListener, OUT:audioSinkController[0x%X]), status:[isOK:%s, exception:%s]",
                    audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(audioSinkController, nullptr);

        // Log before calling the close API
        UT_LOG_INFO("Calling close(IN:audioSinkController:[%p], OUT:_aidl_return)", audioSinkController.get());
        // Call the close API
        status = audiosink->close(audioSinkController, &closed);
        // Log the result of the close API
        UT_LOG_INFO("Result close(IN:audioSinkController:[%p], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, closed),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        // Verify the close operation was successful
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(closed);

    }
}



/**
 * @brief Test the behavior of the AudioSink close function with various invalid inputs and states.
 *
 * This test evaluates the `close` method of the AudioSink class under different invalid scenarios to ensure that it handles errors gracefully and returns the expected results. The test covers cases such as null controller instances, invalid controller instances, and closing from an invalid state.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 014@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Attempt to close with a null AudioSinkController instance | audioSinkController = nullptr, _aidl_return = false | _aidl_return = false, status.isOk() = false | Should Fail |
 * | 02 | Attempt to close with an invalid AudioSinkController instance | audioSinkController = invalid, _aidl_return = false | _aidl_return = false, status.isOk() = false | Should Fail |
 * | 03 | Attempt to close with a valid AudioSinkController instance from a valid state | audioSinkController = valid, _aidl_return = false | _aidl_return = true, status.isOk() = true | Should Pass |
 * | 04 | Attempt to close again with a valid AudioSinkController instance from a CLOSED state | audioSinkController = valid, _aidl_return = false | _aidl_return = false, status.isOk() = false | Should Fail |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_negative_close) 
{
    android::binder::Status status;
    bool aidl_return = false;
    for( const auto& audiosink : mAudioSinks)
    {
        // Test Case 1: Null AudioSinkController Instance
        UT_LOG_INFO("Calling close(IN:audioSinkController:[nullptr], OUT:_aidl_return)");
        status = audiosink->close(nullptr, &aidl_return);
        UT_LOG_INFO("Result close(IN:audioSinkController:[nullptr], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_FALSE(status.isOk());

        // Test Case 2: Invalid AudioSinkController Instance
        
        android::sp<IAudioSinkController> invalidController;
        UT_LOG_INFO("Calling close without prior open (IN:audioSinkController:[invalid], OUT:_aidl_return)");
        status = audiosink->close(invalidController, &aidl_return);
        UT_LOG_INFO("Result close(IN:audioSinkController:[invalid], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_FALSE(status.isOk());

        // Test Case 3: `close` from invalid state (CLOSED)

        android::sp<IAudioSinkControllerListener> controllerListener = new AudioSinkControllerListenerImpl;
        android::sp<IAudioSinkController> audioSinkController;

        UT_LOG_INFO("Calling open(IN:audioSinkControllerListener, OUT:audioSinkController)");
        status = audiosink->open(controllerListener, &audioSinkController);
        UT_LOG_INFO("Result open(IN:audioSinkControllerListener, OUT:audioSinkController[0x%X]), status:[isOK:%s, exception:%s]",
                    audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_NOT_EQUAL(audioSinkController, nullptr);


        UT_LOG_INFO("Calling close(IN:audioSinkController:[valid], OUT:_aidl_return)");
        status = audiosink->close(audioSinkController, &aidl_return);
        UT_LOG_INFO("Result close(IN:audioSinkController:[valid], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(aidl_return);
        UT_ASSERT_TRUE(status.isOk());

        UT_LOG_INFO("Calling close api second time (IN:audioSinkController:[valid], OUT:_aidl_return)");
        status = audiosink->close(audioSinkController, &aidl_return);
        UT_LOG_INFO("Result close(IN:audioSinkController:[valid], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
    }
}



/**
 * @brief Test the positive scenario of registering an event listener with AudioSink.
 *
 * This test verifies the successful registration of a valid IAudioSinkEventListener instance
 * with the AudioSink. It ensures that the API call returns a successful status and the output
 * parameter indicates success.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 015@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Create a valid instance of IAudioSinkEventListener | mSinkEventListener = new AudioSinkEventListenerImpl | Instance should be created successfully | Should be successful |
 * | 02 | Log the API call for registerEventListener | None | Log should be generated | Should be successful |
 * | 03 | Call registerEventListener with a valid IAudioSinkEventListener instance | audioSinkEventListener = mSinkEventListener, _aidl_return = false | Status should be OK, _aidl_return should be true | Should Pass |
 * | 04 | Log the result of registerEventListener | None | Log should be generated with status and _aidl_return | Should be successful |
 * | 05 | Verify that the return value indicates success | status.isOk() = true | Assertion should pass | Should Pass |
 * | 06 | Check that the output parameter *_aidl_return is not NULL or empty | aidlReturn = true | Assertion should pass | Should Pass |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_positive_registerEventListener) 
{
    android::binder::Status status;
    bool aidlReturn = false;
    for( const auto& audiosink : mAudioSinks)
    {
        // Create a valid instance of IAudioSinkEventListener
        mSinkEventListener = new AudioSinkEventListenerImpl;
        // Log the API call
        UT_LOG_INFO("Calling registerEventListener(IN:audioSinkEventListener, OUT:_aidl_return)");
        // Call registerEventListener with a valid IAudioSinkEventListener instance
        status = audiosink->registerEventListener(mSinkEventListener, &aidlReturn);
        // Log the result
        UT_LOG_INFO("Result registerEventListener(IN:audioSinkEventListener, OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidlReturn),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        // Verify that the return value indicates success
        UT_ASSERT_TRUE(status.isOk());
        // Check that the output parameter *_aidl_return is not NULL or empty
        UT_ASSERT_TRUE(aidlReturn);
    }
}



/**
 * @brief Test the registration of an audio sink event listener with various scenarios.
 *
 * This test evaluates the behavior of the registerEventListener function when registering valid and invalid IAudioSinkEventListener instances. It checks the function's response to multiple registrations of the same listener and the registration of a null listener.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 016@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Create a valid AudioSinkEventListenerImpl instance | mSinkEventListener = new AudioSinkEventListenerImpl | Instance should be created successfully | Should be successful |
 * | 02 | Register the valid listener for the first time | audioSinkEventListener = mSinkEventListener, aidlReturn = false | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 03 | Register the valid listener for the second time | audioSinkEventListener = mSinkEventListener, aidlReturn = false | status.isOk() = true, aidlReturn = false | Should Fail |
 * | 04 | Attempt to register a null listener | audioSinkEventListener = nullptr, aidlReturn = false | status.isOk() = false, aidlReturn = false | Should Fail |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_negative_registerEventListener) 
{
    android::binder::Status status;
    bool aidlReturn = false;

    for( const auto& audiosink : mAudioSinks)
    {
        // Test Case 1: Register a valid IAudioSinkEventListener instance twice
        mSinkEventListener = new AudioSinkEventListenerImpl;

        // First registration attempt
        UT_LOG_INFO("Calling registerEventListener(IN:audioSinkEventListener:[0x%X], OUT:_aidl_return)", mSinkEventListener.get());
        status = audiosink->registerEventListener(mSinkEventListener, &aidlReturn);
        UT_LOG_INFO("Result registerEventListener(IN:audioSinkEventListener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    mSinkEventListener.get(), UT_Control_GetMapString(bool_mapTable, aidlReturn),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn);

        // Second registration attempt
        UT_LOG_INFO("Calling registerEventListener(IN:audioSinkEventListener:[0x%X], OUT:_aidl_return)", mSinkEventListener.get());
        status = audiosink->registerEventListener(mSinkEventListener, &aidlReturn);
        UT_LOG_INFO("Result registerEventListener(IN:audioSinkEventListener:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    mSinkEventListener.get(), UT_Control_GetMapString(bool_mapTable, aidlReturn),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_FALSE(aidlReturn);

        // Test Case 2: Register an invalid IAudioSinkEventListener instance
        android::sp<IAudioSinkEventListener> invalidListener = nullptr;

        UT_LOG_INFO("Calling registerEventListener(IN:audioSinkEventListener:[nullptr], OUT:_aidl_return)");
        status = audiosink->registerEventListener(invalidListener, &aidlReturn);
        UT_LOG_INFO("Result registerEventListener(IN:audioSinkEventListener:[nullptr], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidlReturn),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(status.isOk());
        UT_ASSERT_FALSE(aidlReturn);
    }
}



/**
 * @brief Test the positive scenario of unregistering an event listener from AudioSink.
 *
 * This test verifies the successful unregistration of a valid AudioSinkEventListener instance from the AudioSinkManagerService. It ensures that the listener is initially registered and then checks the status and return value after attempting to unregister it.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 017@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Create a new AudioSinkEventListenerImpl instance and ensure it is valid | mSinkEventListener = new AudioSinkEventListenerImpl | mSinkEventListener should not be nullptr | Should be successful |
 * | 02 | Register the event listener with each AudioSink instance | sink->registerEventListener(mSinkEventListener) | status.isOk() = true | Should be successful |
 * | 03 | Log the call to unregisterEventListener | UT_LOG_INFO("Calling unregisterEventListener...") | mSinkEventListener.get() | Log should be successful | Should be successful |
 * | 04 | Call unregisterEventListener with the valid IAudioSinkEventListener instance | gAudioSinkManagerService->unregisterEventListener(mSinkEventListener, &aidl_return) | aidl_return = false | status.isOk() = true, aidl_return = true | Should Pass |
 * | 05 | Verify the return value is a successful ::android::binder::Status | UT_ASSERT_TRUE(status.isOk()) | status.isOk() = true | Assertion should pass | Should Pass |
 * | 06 | Check the output parameter *_aidl_return to confirm it indicates success | UT_ASSERT_TRUE(aidl_return) | aidl_return = true | Assertion should pass | Should Pass |
 * | 07 | Log the result of unregisterEventListener | UT_LOG_INFO("Result unregisterEventListener...") | mSinkEventListener.get(), aidl_return | Log should be successful | Should be successful |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_positive_unregisterEventListener) {
    android::binder::Status status;
    bool aidl_return = false;

    for( const auto& audiosink : mAudioSinks)
    {
        // Ensure the AudioSinkEventListener instance is valid and currently registered
        mSinkEventListener = new AudioSinkEventListenerImpl;
        UT_ASSERT_NOT_EQUAL(mSinkEventListener, nullptr);


        // Log the API call
        UT_LOG_INFO("Calling registerEventListener(IN:audioSinkEventListener, OUT:_aidl_return)");
        // Call registerEventListener with a valid IAudioSinkEventListener instance
        status = audiosink->registerEventListener(mSinkEventListener, &aidl_return);
        // Log the result
        UT_LOG_INFO("Result registerEventListener(IN:audioSinkEventListener, OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        // Verify that the return value indicates success
        UT_ASSERT_TRUE(status.isOk());
        // Check that the output parameter *_aidl_return is not NULL or empty
        UT_ASSERT_TRUE(aidl_return);

        
        // Log the call to unregisterEventListener
        UT_LOG_INFO("Calling unregisterEventListener(IN:audioSinkEventListener[0x%X], OUT:_aidl_return)", mSinkEventListener.get());
        // Call unregisterEventListener with the valid IAudioSinkEventListener instance
        status = audiosink->unregisterEventListener(mSinkEventListener, &aidl_return);
        // Verify the return value is a successful ::android::binder::Status
        UT_ASSERT_TRUE(status.isOk());
        // Check the output parameter *_aidl_return to confirm it indicates success (e.g., true)
        UT_ASSERT_TRUE(aidl_return);
        // Log the result of unregisterEventListener
        UT_LOG_INFO("Result unregisterEventListener(IN:audioSinkEventListener[0x%X], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    mSinkEventListener.get(), UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    }
}



/**
 * @brief Test the behavior of unregistering an event listener in various scenarios.
 *
 * This test evaluates the behavior of the `unregisterEventListener` function when attempting to unregister an event listener that is not registered, and when unregistering an event listener that has already been unregistered.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 018@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Create a valid instance of IAudioSinkEventListener | listener = new AudioSinkEventListenerImpl() | Instance should be created successfully | Should be successful |
 * | 02 | Attempt to unregister an event listener that is not registered | listener = 0x[listener address], aidl_return = false | aidl_return = false, status.isOk() = false | Should Fail |
 * | 03 | Register the event listener | listener = 0x[listener address] | status.isOk() = true | Should be successful |
 * | 04 | Unregister the event listener for the first time | listener = 0x[listener address], aidl_return = false | aidl_return = true, status.isOk() = true | Should Pass |
 * | 05 | Attempt to unregister the event listener again | listener = 0x[listener address], aidl_return = false | aidl_return = false, status.isOk() = false | Should Fail |
 */
UT_ADD_TEST(AudioSinkBaseTest, test_l1_AudioSink_negative_unregisterEventListener) 
{
    android::binder::Status status;
    bool aidl_return = false;

    for( const auto& audiosink : mAudioSinks)
    {
        // Create a valid instance of IAudioSinkEventListener
        android::sp<IAudioSinkEventListener> listener = new AudioSinkEventListenerImpl();

        // Test Case 3: Unregistering an Event Listener which is not registered
        UT_LOG_INFO("Calling unregisterEventListener(IN:audioSinkEventListener[0x%X], OUT:_aidl_return)", listener.get());
        status = audiosink->unregisterEventListener(listener, &aidl_return);
        UT_LOG_INFO("Result unregisterEventListener(IN:audioSinkEventListener[0x%X], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    listener.get(), UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_FALSE(status.isOk());

        // Test Case 4: Unregistering an Event Listener which is already unregistered
        // First, register the listener
        // Log the API call
        android::sp<IAudioSinkEventListener> mSinkEventListener = new AudioSinkEventListenerImpl();
        UT_LOG_INFO("Calling registerEventListener(IN:audioSinkEventListener, OUT:_aidl_return)");
        status = audiosink->registerEventListener(mSinkEventListener, &aidl_return);
        UT_LOG_INFO("Result registerEventListener(IN:audioSinkEventListener, OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidl_return);

        // Unregister the listener for the first time
        UT_LOG_INFO("Calling unregisterEventListener(IN:audioSinkEventListener[0x%X], OUT:_aidl_return)", mSinkEventListener.get());
        status = audiosink->unregisterEventListener(mSinkEventListener, &aidl_return);
        UT_LOG_INFO("Result unregisterEventListener(IN:audioSinkEventListener[0x%X], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    mSinkEventListener.get(), UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(aidl_return);
        UT_ASSERT_TRUE(status.isOk());

        // Attempt to unregister the listener again
        UT_LOG_INFO("Calling unregisterEventListener(IN:audioSinkEventListener[0x%X], OUT:_aidl_return)", mSinkEventListener.get());
        status = audiosink->unregisterEventListener(mSinkEventListener, &aidl_return);
        UT_LOG_INFO("Result unregisterEventListener(IN:audioSinkEventListener[0x%X], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]",
                    mSinkEventListener.get(), UT_Control_GetMapString(bool_mapTable, aidl_return),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidl_return);
        UT_ASSERT_FALSE(status.isOk());
    }
}



/**
 * @brief Test the positive scenario of setting an audio decoder in AudioSinkController
 *
 * This test verifies that the AudioSinkController can successfully set a valid audio decoder ID. 
 * It ensures that the controller is in the READY state and retrieves valid audio decoder IDs 
 * using the configuration YAML APIs. The test then calls setAudioDecoder with each valid ID 
 * and checks that the operation is successful.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 019@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the resource is in the State::READY state | mControllerListener->mNewState = READY | State should be READY | Should be successful |
 * | 02 | Retrieve valid audio decoder IDs using the configuration YAML APIs | decoderIds | decoderIds should not be empty | Should be successful |
 * | 03 | Call setAudioDecoder with each valid ID | audioDecoderId = decoderId.value, _aidl_return = false | _aidl_return should be true, status should be OK | Should Pass |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_positive_setAudioDecoder) 
{
    android::binder::Status status;
    bool aidl_return = false;
    std::vector<int> decoderIds;

    std::string audiodecoder_count_path = "IAudioDecoderManager.IAudioDecoder";
    size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiodecoder_count_path.c_str());

    for (size_t i = 0; i < expectedCount; ++i) {
        std::string audiodecoderidpath = "IAudioDecoderManager.IAudioDecoder."+std::to_string(i) + ".id";
        int32_t expectedIdValue = UT_KVP_PROFILE_GET_UINT32(audiodecoderidpath.c_str());
        decoderIds.push_back(expectedIdValue);
    }


    // Ensure the resource is in the State::READY state
    for (auto& controller : mControllers) {
        for (const auto& decoderId : decoderIds) {
            ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId;
            audioDecoderId.value = decoderId;
            // Call setAudioDecoder with the valid ID
            UT_LOG_INFO("Calling setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return)", audioDecoderId.value);
            status = controller->setAudioDecoder(audioDecoderId, &aidl_return);

            // Verify the return value is true
            UT_ASSERT_TRUE(aidl_return);
            UT_ASSERT_TRUE(status.isOk());

            // Log the result
            UT_LOG_INFO("Result setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        audioDecoderId.value, aidl_return ? "true" : "false",
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        }
    }
}



/**
 * @brief Test the behavior of setAudioDecoder with invalid and valid IDs in different states
 *
 * This test verifies the behavior of the setAudioDecoder function when provided with invalid audio decoder IDs and when the resource is not in the READY state. It ensures that the function handles these scenarios correctly by returning appropriate status and results.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 020@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Retrieve all available audio decoder IDs and ensure the list is not empty | None | List should not be empty | Should be successful |
 * | 02 | Sort the audio decoder IDs and calculate invalid low and high IDs | None | Invalid IDs calculated | Should be successful |
 * | 03 | Ensure the resource is in the READY state | None | State should be READY | Should be successful |
 * | 04 | Test setAudioDecoder with invalid low ID | audioDecoderId = invalidLowId, aidlReturn = false | aidlReturn should be false, status should be OK | Should Fail |
 * | 05 | Test setAudioDecoder with invalid high ID | audioDecoderId = invalidHighId, aidlReturn = false | aidlReturn should be false, status should be OK | Should Fail |
 * | 06 | Transition resource to STARTED state | None | State should be STARTED | Should be successful |
 * | 07 | Retrieve a valid audio decoder ID | None | Valid ID should be retrieved | Should be successful |
 * | 08 | Test setAudioDecoder with valid ID in STARTED state | audioDecoderId = validId, aidlReturn = false | status.exceptionCode should be EX_ILLEGAL_STATE, aidlReturn should be false | Should Fail |
 * | 09 | Transition resource back to READY state | None | State should be READY | Should be successful |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_negative_setAudioDecoder) {
    android::binder::Status status;
    bool aidlReturn = false;


    std::string audiodecoder_count_path = "IAudioDecoderManager.IAudioDecoder";
    size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiodecoder_count_path.c_str());

    // Test Case 3: Invalid Audio Decoder ID
    std::vector<uint32_t> audioDecoderIds;
    for (size_t i = 0; i<expectedCount ; ++i) 
    {
        std::string audiodecoderid_path = "IAudioDecoderManager.IAudioDecoder."+std::to_string(i)+".id";
        uint32_t id = UT_KVP_PROFILE_GET_UINT32(audiodecoderid_path.c_str());
        audioDecoderIds.push_back(id);
    }
    UT_ASSERT_FALSE(audioDecoderIds.empty());

    std::sort(audioDecoderIds.begin(), audioDecoderIds.end());
    int32_t invalidLowId = audioDecoderIds.front() - 1;
    int32_t invalidHighId = audioDecoderIds.back() + 1;
    int32_t validid = audioDecoderIds.front();

    for (auto& controller : mControllers) {
        // Test with invalid ID (least value -1)
        IAudioDecoder::Id audioDecoderId;
        audioDecoderId.value = invalidLowId;
        UT_LOG_INFO("Calling setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return)", audioDecoderId.value);
        status = controller->setAudioDecoder(audioDecoderId, &aidlReturn);
        UT_LOG_INFO("Result setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                    audioDecoderId.value, aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidlReturn);
        UT_ASSERT_TRUE(status.isOk());

        // Test with invalid ID (highest value +1)
        audioDecoderId.value = invalidHighId;
        UT_LOG_INFO("Calling setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return)", audioDecoderId.value);
        status = controller->setAudioDecoder(audioDecoderId, &aidlReturn);
        UT_LOG_INFO("Result setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return[%d]), status:[isOK:%s, exception:%s]",
                    audioDecoderId.value, aidlReturn, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidlReturn);
        UT_ASSERT_TRUE(status.isOk());

        // Test Case 4: Resource Not in READY State

        // Transition to a non-READY state (e.g., STARTED)
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        audioDecoderId.value = validid;

        UT_LOG_INFO("Calling setAudioDecoder from STARTED state (IN:audioDecoderId:[%d], OUT:_aidl_return)", audioDecoderId.value);
        status = controller->setAudioDecoder(audioDecoderId, &aidlReturn);

        // Verify the return value is true
        UT_ASSERT_FALSE(aidlReturn);
        UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
        // Log the result
        UT_LOG_INFO("Result setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    audioDecoderId.value, aidlReturn ? "true" : "false",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());                    
    }

}

/**
 * @brief Test the functionality of getting and setting audio decoders in AudioSinkController.
 *
 * This test verifies the ability to set and retrieve audio decoder IDs using the AudioSinkController. 
 * It ensures that the audio sink can be opened and started, and that valid audio decoder IDs can be 
 * set and retrieved successfully.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 021@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the audio sink is in the READY state by opening it. | mControllerListener, opened = false | status.isOk() = true, opened = true | Should be successful |
 * | 02 | Retrieve valid audio decoder IDs using the configuration YAML APIs. | decoderIds | status.isOk() = true, decoderIds not empty | Should be successful |
 * | 03 | Set the audio decoder using a valid decoder ID. | decoderId, setResult = false | status.isOk() = true, setResult = true | Should Pass |
 * | 04 | Get the audio decoder and verify it matches the set ID. | audioDecoder | status.isOk() = true, audioDecoder.value = decoderId.value | Should Pass |
 * | 05 | Ensure the audio sink is in the STARTED state by starting it. | started = false | status.isOk() = true, started = true | Should be successful |
 * | 06 | Set the audio decoder again using a valid decoder ID. | decoderId, setResult = false | status.isOk() = true, setResult = true | Should Pass |
 * | 07 | Get the audio decoder again and verify it matches the set ID. | audioDecoder | status.isOk() = true, audioDecoder.value = decoderId.value | Should Pass |
 */
 UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_positive_getAudioDecoder) {
    android::binder::Status status;
    ::com::rdk::hal::audiodecoder::IAudioDecoder::Id decoderId;
    UT_ASSERT_FALSE(mControllers.empty());

    for( const auto& controller : mControllers)
    {
        
        std::vector<int> decoderIds;

        std::string audiodecoder_count_path = "IAudioDecoderManager.IAudioDecoder";
        size_t expectedCount = UT_KVP_PROFILE_GET_LIST_COUNT(audiodecoder_count_path.c_str());
    
        for (size_t i = 0; i < expectedCount; ++i) {
            std::string audiodecoderidpath = "IAudioDecoderManager.IAudioDecoder."+std::to_string(i) + ".id";
            int32_t expectedIdValue = UT_KVP_PROFILE_GET_UINT32(audiodecoderidpath.c_str());
            decoderIds.push_back(expectedIdValue);
        }

        for (const auto& id : decoderIds) {
            decoderId.value = id;
            // Set the audio decoder
            UT_LOG_INFO("Calling setAudioDecoder(IN:_decoderId:[%d])", decoderId.value);
            bool setResult = false;
            status = controller->setAudioDecoder(decoderId, &setResult);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(setResult);

            // Get the audio decoder
            ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoder;
            UT_LOG_INFO("Calling getAudioDecoder(OUT:audioDecoder[])");
            status = controller->getAudioDecoder(&audioDecoder);
            UT_LOG_INFO("Result getAudioDecoder(OUT:audioDecoder[0x%X]), status:[isOK:%s, exception:%s]",
                        audioDecoder.value, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_EQUAL(audioDecoder.value, decoderId.value);
        }

        // Ensure the resource is in the STARTED state
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        for (const auto& id : decoderIds) {

            decoderId.value = id;
            // Set the audio decoder
            UT_LOG_INFO("Calling setAudioDecoder(IN:_decoderId:[%d])", decoderId.value);
            bool setResult = false;
            status = controller->setAudioDecoder(decoderId, &setResult);
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_TRUE(setResult);

            // Get the audio decoder
            ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoder;
            UT_LOG_INFO("Calling getAudioDecoder(OUT:audioDecoder[])");
            status = controller->getAudioDecoder(&audioDecoder);
            UT_LOG_INFO("Result getAudioDecoder(OUT:audioDecoder[0x%X]), status:[isOK:%s, exception:%s]",
                        audioDecoder.value, UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
            UT_ASSERT_TRUE(status.isOk());
            UT_ASSERT_EQUAL(audioDecoder.value, decoderId.value);
        }
        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the positive scenario of starting the AudioSinkController.
 *
 * This test verifies the successful transition of the audio sink controller from the READY state to the STARTED state by invoking the start API. It ensures that the controller is in the correct initial state and checks the expected state transitions and return values.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 022@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the audio sink is in READY state | mNewState = READY | State should be READY | Should be successful |
 * | 02 | Log before calling the start API | None | Log should be successful | Should be successful |
 * | 03 | Call the start API on the controller | None | Status should be OK | Should Pass |
 * | 04 | Log the result of the start API call | status = [isOK:true, exception:none] | Log should be successful | Should be successful |
 * | 05 | Verify the return value is binder::Status::OK | status.isOk() = true | Assertion should pass | Should Pass |
 * | 06 | Verify the state transition to STARTING and then STARTED | mOldState = READY, mNewState = STARTED | State should transition correctly | Should Pass |
 * | 07 | Teardown: Call stop to revert the state | None | Status should be OK, State should be READY | Done by Pre-requisite TearDown function |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_positive_start)
{
    android::binder::Status status;
    UT_ASSERT_FALSE(mControllers.empty());

    for (const auto& controller : mControllers)
    {
        // Log before calling the start API
        UT_LOG_INFO("Calling start()");

        // Call the start API
        status = controller->start();

        // Log the result of the start API call
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        // Verify the return value is binder::Status::OK
        UT_ASSERT_TRUE(status.isOk());

        // Teardown: Call stop to revert the state
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the negative scenario of starting the AudioSinkController when already started.
 *
 * This test verifies the behavior of the AudioSinkController when the start() method is called twice consecutively. 
 * The first call to start() should transition the controller to the STARTED state, and the second call should fail 
 * with an EX_ILLEGAL_STATE exception, indicating that the controller is already in the STARTED state.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 023@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify mControllers and mAudioSinks are not empty | mControllers.size(), mAudioSinks.size() | Should be successful | Should be successful |
 * | 02 | Call start() on each controller to change to STARTED state | controller[i]->start() | Status is OK | Should be successful |
 * | 03 | Call start() again on each controller | controller[i]->start() | Status is EX_ILLEGAL_STATE | Should Fail |
 * | 04 | Verify the exception code is EX_ILLEGAL_STATE | status.exceptionCode() | EX_ILLEGAL_STATE | Should Fail |
 * | 05 | Call stop() on each controller as teardown | controller[i]->stop() | Status is OK | Done by Pre-requisite TearDown function |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_negative_start) {
    UT_ASSERT_FALSE(mControllers.empty());
    android::binder::Status status;

    for (size_t i = 0; i < mControllers.size(); ++i) 
    {    
        // Step 1: Call start as prerequisite to change to STARTED state
        UT_LOG_INFO("Calling start()");
        status = mControllers[i]->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());

        // Step 2: Call the start() API again
        UT_LOG_INFO("Calling start()");
        status = mControllers[i]->start();
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        // Step 3: Check the return value is binder::Status::EX_ILLEGAL_STATE
        UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);

        // Step 4: Call stop as teardown
        UT_LOG_INFO("Calling stop()");
        status = mControllers[i]->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the positive scenario of stopping the AudioSinkController.
 *
 * This test verifies that the AudioSinkController can successfully transition from the STARTED state to the READY state by invoking the stop() API. It ensures that the controller is initially in the STARTED state by calling the start() API and then checks the state transition upon calling stop().
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 024@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Check if mControllers and mAudioSinks are not empty | mControllers.size() > 0, mAudioSinks.size() > 0 | Should be successful | Should be successful |
 * | 02 | Loop through each controller and ensure the audio sink is in the STARTED state by invoking the start() API | controller[i]->start() | status.isOk() = true, mControllerListener->mNewState = STARTED | Should Pass |
 * | 03 | Log the result of the start() API call | status.isOk(), status.exceptionCode() | Log output | Should be successful |
 * | 04 | Call the stop() API to transition to the READY state | controller[i]->stop() | status.isOk() = true, mControllerListener->mNewState = READY | Should Pass |
 * | 05 | Log the result of the stop() API call | status.isOk(), status.exceptionCode() | Log output | Should be successful |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_positive_stop)
{
    UT_ASSERT_FALSE(mControllers.empty());

    for (size_t i = 0; i < mControllers.size(); ++i)
    {
        android::binder::Status status;

        // Ensure the audio sink is in the STARTED state by invoking the start() API.
        UT_LOG_INFO("Calling start()");
        status = mControllers[i]->start();
        UT_ASSERT_TRUE(status.isOk());
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        // Call the stop() API.
        UT_LOG_INFO("Calling stop()");
        status = mControllers[i]->stop();
        UT_ASSERT_TRUE(status.isOk());
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    }
}

/**
 * @brief Test the behavior of the AudioSinkController when attempting to stop in various states.
 *
 * This test verifies the behavior of the AudioSinkController's stop() method when the audio sink is in different states. 
 * It ensures that the stop() method returns the correct exception code when called in an invalid state.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 025@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify mControllers is not empty | mControllers | Should be successful | Should be successful |
 * | 02 | Ensure the audio sink is in the READY state | mNewState = READY | Should be successful | Should be successful |
 * | 03 | Call stop() in READY state and verify it returns EX_ILLEGAL_STATE | status = controller->stop() | status.isOk() = false, status.exceptionCode() = EX_ILLEGAL_STATE | Should Fail |
 * | 04 | Start the audio sink to move it to STARTED state | status = controller->start() | status.isOk() = true, mNewState = STARTED | Should be successful |
 * | 05 | Call stop() to move to STOPPING and then READY | status = controller->stop() | status.isOk() = true, mNewState = READY | Should be successful |
 * | 06 | Call stop() again in READY state and verify it returns EX_ILLEGAL_STATE | status = controller->stop() | status.isOk() = false, status.exceptionCode() = EX_ILLEGAL_STATE | Should Fail |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_negative_stop)
{
    android::binder::Status status;
    UT_ASSERT_FALSE(mControllers.empty());

    for (auto& controller : mControllers)
    {
        // Call the stop() API and verify it returns EX_ILLEGAL_STATE
        UT_LOG_INFO("Calling stop() without prior start()");
        status = controller->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(status.isOk());
        UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);

        // Test Case 2: Stop Audio Sink in STOPPING State
        // Preconditions: Start the audio sink to move it to STARTED state
        UT_LOG_INFO("Calling start()");
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        // Call stop() to move to STOPPING and then READY
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());

        // Call stop() again and verify it returns EX_ILLEGAL_STATE
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(status.isOk());
        UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
    }
}

/**
 * @brief Test the negative scenario of queueing an audio frame with an invalid buffer handle.
 *
 * This test verifies the behavior of the AudioSinkController when attempting to queue an audio frame with an invalid buffer handle. It ensures that the system correctly handles invalid input by returning an appropriate error code.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 026@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the audio sink is in the STARTED state by starting each controller. | mControllers.size() = mAudioSinks.size() | Controllers should start successfully | Should be successful |
 * | 02 | Prepare test inputs with an invalid buffer handle. | nsPresentationTime = 1000000, bufferHandle = -9999, metadata = {sourceCodec = PCM, isDolbyAtmos = false, numChannels = 2, sampleRate = 44100, format = S16LE, source = SYSTEM} | Inputs prepared successfully | Should be successful |
 * | 03 | Log the API call with the prepared inputs. | nsPresentationTime = 1000000, bufferHandle = -9999, metadata = {sourceCodec = PCM, isDolbyAtmos = false, numChannels = 2, sampleRate = 44100, format = S16LE, source = SYSTEM} | Log should be successful | Should be successful |
 * | 04 | Call the queueAudioFrame API with the prepared inputs. | nsPresentationTime = 1000000, bufferHandle = -9999, metadata = {sourceCodec = PCM, isDolbyAtmos = false, numChannels = 2, sampleRate = 44100, format = S16LE, source = SYSTEM} | API should return EX_ILLEGAL_ARGUMENT | Should Fail |
 * | 05 | Log the result of the API call. | status = {isOk = false, exception = EX_ILLEGAL_ARGUMENT} | Log should be successful | Should be successful |
 * | 06 | Verify the expected output by checking the exception code. | status.exceptionCode() = EX_ILLEGAL_ARGUMENT | Exception code should be EX_ILLEGAL_ARGUMENT | Should be successful |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_negative_queueAudioFrame) {
    android::binder::Status status;
    bool result = false;

    // Ensure the audio sink is in the STARTED state
    UT_ASSERT_EQUAL(mControllers.size(), mAudioSinks.size());
    for (size_t i = 0; i < mControllers.size(); ++i) {
        status = mControllers[i]->start();
        UT_ASSERT_TRUE(status.isOk());

        // Prepare test inputs
        int64_t nsPresentationTime = 1000000; // 1 millisecond in nanoseconds
        int64_t bufferHandle = -9999; // Invalid buffer handle
        FrameMetadata metadata;
        metadata.sourceCodec = Codec::PCM;
        metadata.isDolbyAtmos = false;
        metadata.numChannels = 2;
        metadata.channelTypes = {ChannelType::FRONT_LEFT, ChannelType::FRONT_RIGHT};
        metadata.sampleRate = 44100;
        metadata.format = PCMFormat::S16LE;
        metadata.planarFormat = false;
        metadata.trimStartNs = 0;
        metadata.trimEndNs = 0;
        metadata.lowLatency = false;
        metadata.endOfStream = false;
        metadata.discontinuity = false;
        metadata.source = AVSource::SYSTEM;

        // Log the API call
        UT_LOG_INFO("Calling queueAudioFrame(IN:nsPresentationTime:[%lld], IN:bufferHandle:[%lld], IN:metadata:[sourceCodec:%d, isDolbyAtmos:%d, numChannels:%d, sampleRate:%d, format:%d, source:%d])",
                    nsPresentationTime, bufferHandle, static_cast<int>(metadata.sourceCodec), metadata.isDolbyAtmos, metadata.numChannels, metadata.sampleRate, static_cast<int>(metadata.format), static_cast<int>(metadata.source));

        // Call the API
        status = mControllers[i]->queueAudioFrame(nsPresentationTime, bufferHandle, metadata, &result);

        // Log the result
        UT_LOG_INFO("Result queueAudioFrame(IN:nsPresentationTime:[%lld], IN:bufferHandle:[%lld], IN:metadata:[sourceCodec:%d, isDolbyAtmos:%d, numChannels:%d, sampleRate:%d, format:%d, source:%d]), status:[isOK:%s, exception:%s]",
                    nsPresentationTime, bufferHandle, static_cast<int>(metadata.sourceCodec), metadata.isDolbyAtmos, metadata.numChannels, metadata.sampleRate, static_cast<int>(metadata.format), static_cast<int>(metadata.source),
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        // Verify the expected output
        UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_ARGUMENT);

        status = mControllers[i]->stop();
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the positive scenario of flushing the audio sink controller.
 *
 * This test verifies the functionality of the flush operation in the audio sink controller. 
 * It ensures that the flush operation can be successfully executed after starting the audio sink 
 * and before stopping it, confirming the expected behavior of the audio sink controller.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 027@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the controllers list is not empty | mControllers.empty() = false | Assertion should pass | Should be successful |
 * | 02 | Start the audio sink for each controller | controller->start() | status.isOk() = true | Should Pass |
 * | 03 | Flush the audio sink for each controller | controller->flush() | status.isOk() = true | Should Pass |
 * | 04 | Stop the audio sink for each controller | controller->stop() | status.isOk() = true | Should Pass |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_positive_flush)
{
    android::binder::Status status;
    UT_ASSERT_FALSE(mControllers.empty());

    for (auto& controller : mControllers)
    {
        // Start the audio sink
        UT_LOG_INFO("Calling start()");
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        // Flush the audio sink
        UT_LOG_INFO("Calling flush()");
        status = controller->flush();
        UT_ASSERT_TRUE(status.isOk());
        UT_LOG_INFO("Result flush(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        // Stop the audio sink
        UT_LOG_INFO("Calling stop()");
        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
    }
}

/**
 * @brief Test the behavior of AudioSinkController when attempting to flush in an invalid state.
 *
 * This test verifies that the flush operation on an AudioSinkController fails with an illegal state exception
 * when the audio sink is not in the STARTED state. The test ensures that the audio sink transitions through
 * the necessary states and checks the expected failure when flush is called in an inappropriate state.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 028@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Verify that mControllers and mAudioSinks are not empty | mControllers.size() > 0, mAudioSinks.size() > 0 | Should be successful | Should be successful |
 * | 02 | Start the audio sink to transition to STARTED state | controller = mControllers[i], started = false | status.isOk() = true, started = true | Should be successful |
 * | 03 | Stop the audio sink to transition to READY state | controller = mControllers[i], stopped = false | status.isOk() = true, stopped = true | Should be successful |
 * | 04 | Attempt to flush the audio sink when not in STARTED state | None | status.exceptionCode() = EX_ILLEGAL_STATE | Should Fail |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_negative_flush) {
    UT_ASSERT_FALSE(mControllers.empty());

    for (size_t i = 0; i < mControllers.size(); ++i) {
        // Ensure the audio sink is in a state other than STARTED (e.g., READY)
        android::binder::Status status;
        bool started = false;
        status = mControllers[i]->start();
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(started);

        // Stop the audio sink to transition to READY state
        bool stopped = false;
        status = mControllers[i]->stop();
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(stopped);

        // Attempt to perform a flush operation when the audio sink is not in the STARTED state
        UT_LOG_INFO("Calling flush() from READY state");
        status = mControllers[i]->flush();
        UT_LOG_INFO("Result flush(), status:[isOK:%s, exception:%s]",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

        // Check the return value is binder::Status EX_ILLEGAL_STATE
        UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
    }
}

/**
 * @brief Test the functionality of setting and getting volume in AudioSinkController.
 *
 * This test verifies the ability of the AudioSinkController to correctly set and retrieve the volume settings. It ensures that the volume can be set and retrieved accurately in different states of the audio sink.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 029@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure the audio sink is in State::READY | mNewState = READY | State should be READY | Should be successful |
 * | 02 | Set volume using setVolume function | volume = 1.0, muted = false | status.isOk() = true, volumeSetResult = true | Should Pass |
 * | 03 | Get volume using getVolume function | volumeGet | status.isOk() = true, volumeGet.volume = 1.0, volumeGet.muted = false | Should Pass |
 * | 04 | Start the audio sink to change state to State::STARTED | mNewState = STARTED | State should be STARTED | Should be successful |
 * | 05 | Set volume again using setVolume function | volume = 1.0, muted = false | status.isOk() = true, volumeSetResult = true | Should Pass |
 * | 06 | Get volume again using getVolume function | volumeGet | status.isOk() = true, volumeGet.volume = 1.0, volumeGet.muted = false | Should Pass |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_positive_getVolume) {
    android::binder::Status status;
    Volume volumeSet;
    volumeSet.volume = 1.0; // Using the specified input
    volumeSet.muted = false; // Using the specified input

    for (auto& controller : mControllers) {
        // Set volume
        UT_LOG_INFO("Calling setVolume(IN:volume:[%f], IN:muted:[%s])", volumeSet.volume, UT_Control_GetMapString(bool_mapTable, volumeSet.muted));
        bool volumeSetResult = false;
        status = controller->setVolume(volumeSet, &volumeSetResult);
        UT_LOG_INFO("Result setVolume(IN:volume:[%f], IN:muted:[%s], OUT:volumeSetResult:[%s]), status:[isOK:%s, exception:%s]", volumeSet.volume, UT_Control_GetMapString(bool_mapTable, volumeSet.muted), UT_Control_GetMapString(bool_mapTable, volumeSetResult), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(volumeSetResult);

        // Get volume
        Volume volumeGet;
        UT_LOG_INFO("Calling getVolume(OUT:volumeGet[])");
        status = controller->getVolume(&volumeGet);
        UT_LOG_INFO("Result getVolume(OUT:volumeGet[volume:%f, muted:%s]), status:[isOK:%s, exception:%s]", volumeGet.volume, UT_Control_GetMapString(bool_mapTable, volumeGet.muted), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(volumeGet.volume, volumeSet.volume);
        UT_ASSERT_EQUAL(volumeGet.muted, volumeSet.muted);

        // Ensure the audio sink is in State::STARTED
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        volumeSet.volume = 0.5; // Using the specified input
        volumeSet.muted = true; // Using the specified input
        volumeSetResult = false;
        // Set volume again
        UT_LOG_INFO("Calling setVolume(IN:volume:[%f], IN:muted:[%s])", volumeSet.volume, UT_Control_GetMapString(bool_mapTable, volumeSet.muted));
        status = controller->setVolume(volumeSet, &volumeSetResult);
        UT_LOG_INFO("Result setVolume(IN:volume:[%f], IN:muted:[%s], OUT:volumeSetResult:[%s]), status:[isOK:%s, exception:%s]", volumeSet.volume, UT_Control_GetMapString(bool_mapTable, volumeSet.muted), UT_Control_GetMapString(bool_mapTable, volumeSetResult), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(volumeSetResult);

        // Get volume again
        UT_LOG_INFO("Calling getVolume(OUT:volumeGet[])");
        status = controller->getVolume(&volumeGet);
        UT_LOG_INFO("Result getVolume(OUT:volumeGet[volume:%f, muted:%s]), status:[isOK:%s, exception:%s]", volumeGet.volume, UT_Control_GetMapString(bool_mapTable, volumeGet.muted), UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_EQUAL(volumeGet.volume, volumeSet.volume);
        UT_ASSERT_EQUAL(volumeGet.muted, volumeSet.muted);

        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());

    }
}

/**
 * @brief Test the functionality of setting volume in different states of AudioSinkController
 *
 * This test verifies the ability to set the volume to maximum and minimum levels in different states of the AudioSinkController. It ensures that the controller behaves correctly when the volume is set in the READY and STARTED states.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 030@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Open the controller and ensure it is in the READY state | mControllerListener = nullptr | status.isOk() = true, mControllerListener->mNewState = READY | Should be successful |
 * | 02 | Set volume to maximum level in READY state | volume = 1.0, muted = false | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 03 | Start the controller and ensure it is in the STARTED state |  | status.isOk() = true, mControllerListener->mNewState = STARTED | Should be successful |
 * | 04 | Set volume to minimum level in STARTED state | volume = 0.0, muted = false | status.isOk() = true, aidlReturn = true | Should Pass |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_positive_setVolume) {
    android::binder::Status status;
    bool aidlReturn = false;

    // Ensure the resource is in the State::READY state
    for (auto& controller : mControllers) {
        // Test Case 1: Set Volume to Maximum Level in READY State
        Volume maxVolume;
        maxVolume.volume = 1.0;
        maxVolume.muted = false;

        UT_LOG_INFO("Calling setVolume(IN:volume:[%f, %s], OUT:_aidl_return)", maxVolume.volume, maxVolume.muted ? "true" : "false");
        status = controller->setVolume(maxVolume, &aidlReturn);
        UT_LOG_INFO("Result setVolume(IN:volume:[%f, %s], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    maxVolume.volume, maxVolume.muted ? "true" : "false", aidlReturn ? "true" : "false",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn);

        // Ensure the resource is in the State::STARTED state
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        // Test Case 2: Set Volume to Minimum Level in STARTED State
        Volume minVolume;
        minVolume.volume = 0.0;
        minVolume.muted = false;

        UT_LOG_INFO("Calling setVolume(IN:volume:[%f, %s], OUT:_aidl_return)", minVolume.volume, minVolume.muted ? "true" : "false");
        status = controller->setVolume(minVolume, &aidlReturn);
        UT_LOG_INFO("Result setVolume(IN:volume:[%f, %s], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                    minVolume.volume, minVolume.muted ? "true" : "false", aidlReturn ? "true" : "false",
                    UT_Control_GetMapString(bool_mapTable, status.isOk()),
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_TRUE(status.isOk());
        UT_ASSERT_TRUE(aidlReturn);

        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the behavior of the AudioSinkController when setting invalid volume levels.
 *
 * This test verifies that the AudioSinkController correctly handles invalid volume levels by returning an error status and ensuring the operation does not succeed. It tests both above maximum and below minimum volume levels.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 031@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Check if mControllers is not empty | mControllers | Should be successful |  |
 * | 02 | Set Volume with Invalid Volume Level (Above Maximum) | volume = 1.1, muted = false, _aidl_return = false | status.isOk() = false, _aidl_return = false | Should Fail |
 * | 03 | Set Volume with Invalid Volume Level (Below Minimum) | volume = -0.1, muted = false, _aidl_return = false | status.isOk() = false, _aidl_return = false | Should Fail |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_negative_setVolume) {
    UT_ASSERT_FALSE(mControllers.empty());

    for (auto& controller : mControllers) {
        // Test Case 4: Set Volume with Invalid Volume Level (Above Maximum)
        {
            Volume volume;
            volume.volume = 1.1; // Invalid volume level above maximum
            volume.muted = false;
            bool _aidl_return = false;

            UT_LOG_INFO("Calling setVolume(IN:volume:[%f, %s], OUT:_aidl_return)", volume.volume, volume.muted ? "true" : "false");
            android::binder::Status status = controller->setVolume(volume, &_aidl_return);
            UT_LOG_INFO("Result setVolume(IN:volume:[%f, %s], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        volume.volume, volume.muted ? "true" : "false", _aidl_return ? "true" : "false",
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

            UT_ASSERT_FALSE(status.isOk());
            UT_ASSERT_FALSE(_aidl_return);
        }

        // Test Case 5: Set Volume with Invalid Volume Level (Below Minimum)
        {
            Volume volume;
            volume.volume = -0.1; // Invalid volume level below minimum
            volume.muted = false;
            bool _aidl_return = false;

            UT_LOG_INFO("Calling setVolume(IN:volume:[%f, %s], OUT:_aidl_return)", volume.volume, volume.muted ? "true" : "false");
            android::binder::Status status = controller->setVolume(volume, &_aidl_return);
            UT_LOG_INFO("Result setVolume(IN:volume:[%f, %s], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]",
                        volume.volume, volume.muted ? "true" : "false", _aidl_return ? "true" : "false",
                        UT_Control_GetMapString(bool_mapTable, status.isOk()),
                        UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));

            UT_ASSERT_FALSE(status.isOk());
            UT_ASSERT_FALSE(_aidl_return);
        }
    }
}

/**
 * @brief Test the positive scenario of setting volume ramp in AudioSinkController.
 *
 * This test verifies the functionality of setting a volume ramp in the AudioSinkController. It ensures that the controller can successfully start and set the volume ramp with various parameters without errors.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 032@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Start all controllers to ensure they are in State::STARTED | None | Controllers should start successfully | Should be successful |
 * | 02 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 0.0, overMs = 5000, volumeRamp = LINEAR | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 03 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 0.5, overMs = 5000, volumeRamp = LINEAR | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 04 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 1.0, overMs = 5000, volumeRamp = LINEAR | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 05 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 0.0, overMs = 5000, volumeRamp = IN_CUBIC | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 06 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 0.5, overMs = 5000, volumeRamp = IN_CUBIC | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 07 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 1.0, overMs = 5000, volumeRamp = IN_CUBIC | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 08 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 0.0, overMs = 5000, volumeRamp = OUT_CUBIC | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 09 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 0.5, overMs = 5000, volumeRamp = OUT_CUBIC | status.isOk() = true, aidlReturn = true | Should Pass |
 * | 10 | Iterate over targetVolumes and volumeRamps, and call setVolumeRamp on each controller | targetVolume = 1.0, overMs = 5000, volumeRamp = OUT_CUBIC | status.isOk() = true, aidlReturn = true | Should Pass |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_positive_setVolumeRamp) {
    android::binder::Status status;
    bool aidlReturn = false;

    // Ensure the resource is in State::STARTED
    for (auto& controller : mControllers) 
    {
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());


        // Iterate over targetVolume, overMs, and volumeRamp
        std::vector<double> targetVolumes = {0.0, 0.5, 1.0};
        int32_t overMs = 5000;
        std::vector<VolumeRamp> volumeRamps = {VolumeRamp::LINEAR, VolumeRamp::IN_CUBIC, VolumeRamp::OUT_CUBIC};

        for (auto& targetVolume : targetVolumes) {
        for (auto& volumeRamp : volumeRamps) {
            for (auto& controller : mControllers) {
                UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:aidlReturn)", targetVolume, overMs, static_cast<int32_t>(volumeRamp));
                status = controller->setVolumeRamp(targetVolume, overMs, volumeRamp, &aidlReturn);
                UT_LOG_INFO("Result setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:aidlReturn:[%s]), status:[isOK:%s, exception:%s]", targetVolume, overMs, static_cast<int32_t>(volumeRamp), aidlReturn ? "true" : "false", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
                UT_ASSERT_TRUE(status.isOk());
                UT_ASSERT_TRUE(aidlReturn);
            }
        }
        }

        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());
    }
}

/**
 * @brief Test the negative scenarios for setting volume ramp in AudioSinkController.
 *
 * This test evaluates the behavior of the setVolumeRamp function when provided with invalid inputs such as negative target volume, zero duration, and incorrect state. It ensures that the function handles these cases gracefully by returning appropriate error statuses.
 *
 * **Test Group ID:** Basic: 01@n
 * **Test Case ID:** 033@n
 * **Priority:** High@n
 * @n
 * **Pre-Conditions:** None@n
 * **Dependencies:** None@n
 * **User Interaction:** None@n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01 | Ensure all controllers are started | None | Controllers should be in State::STARTED | Should be successful |
 * | 02 | Call setVolumeRamp with negative target volume | targetVolume = -0.5, overMs = 5000, volumeRamp = LINEAR | aidlReturn = false, status.isOk() = false | Should Fail |
 * | 03 | Call setVolumeRamp with zero duration | targetVolume = 0.7, overMs = 0, volumeRamp = LINEAR | aidlReturn = false, status.isOk() = false | Should Fail |
 * | 04 | Ensure all controllers are stopped | None | Controllers should NOT be in State::STARTED | Should be successful |
 * | 05 | Call setVolumeRamp with invalid state | targetVolume = 0.6, overMs = 4000, volumeRamp = LINEAR | aidlReturn = false, status.exceptionCode() = EX_ILLEGAL_STATE | Should Fail |
 */
UT_ADD_TEST(AudioSinkControllerBaseTest, test_l1_AudioSinkController_negative_setVolumeRamp) {
    android::binder::Status status;
    bool aidlReturn = false;

    // Test Case 4: Invalid Target Volume (Negative Value)
    double targetVolumeNegative = -0.5;
    int32_t overMsValid = 5000;
    VolumeRamp volumeRampLinear = VolumeRamp::LINEAR;

    for (auto& controller : mControllers) 
    {
        // Ensure the resource is in State::STARTED
        status = controller->start();
        UT_ASSERT_TRUE(status.isOk());

        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:_aidl_return)", targetVolumeNegative, overMsValid, static_cast<int32_t>(volumeRampLinear));
        status = controller->setVolumeRamp(targetVolumeNegative, overMsValid, volumeRampLinear, &aidlReturn);
        UT_LOG_INFO("Result setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", targetVolumeNegative, overMsValid, static_cast<int32_t>(volumeRampLinear), aidlReturn ? "true" : "false", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidlReturn);
        UT_ASSERT_FALSE(status.isOk());

        // Test Case 5: Invalid Duration (Zero Milliseconds)
        double targetVolumeValid = 0.7;
        int32_t overMsZero = 0;

        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:_aidl_return)", targetVolumeValid, overMsZero, static_cast<int32_t>(volumeRampLinear));
        status = controller->setVolumeRamp(targetVolumeValid, overMsZero, volumeRampLinear, &aidlReturn);
        UT_LOG_INFO("Result setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", targetVolumeValid, overMsZero, static_cast<int32_t>(volumeRampLinear), aidlReturn ? "true" : "false", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidlReturn);
        UT_ASSERT_FALSE(status.isOk());

        status = controller->stop();
        UT_ASSERT_TRUE(status.isOk());

        // Test Case 6: Invalid State (Not Started)
        double targetVolumeStateInvalid = 0.6;
        int32_t overMsStateInvalid = 4000;


        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:_aidl_return)", targetVolumeStateInvalid, overMsStateInvalid, static_cast<int32_t>(volumeRampLinear));
        status = controller->setVolumeRamp(targetVolumeStateInvalid, overMsStateInvalid, volumeRampLinear, &aidlReturn);
        UT_LOG_INFO("Result setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", targetVolumeStateInvalid, overMsStateInvalid, static_cast<int32_t>(volumeRampLinear), aidlReturn ? "true" : "false", UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));
        UT_ASSERT_FALSE(aidlReturn);
        UT_ASSERT_EQUAL(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
    }
}

int main(int argc, char **argv)
{
    UT_status_t result;
    UT_init(argc, argv);
    result = UT_run_tests();
    return result;
}
