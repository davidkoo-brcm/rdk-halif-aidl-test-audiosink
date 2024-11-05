
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

 /**
* @file AudioSinkClient.cpp
* @page AudioSink hal Level 1 Tests
*
* ## Module's Role
* This module includes Level 1 functional tests (success and failure scenarios).
* This is to ensure that the AudioSink hal APIs meet the requirements across all vendors.
*
*/

#include <gtest/gtest.h>
#include <binder/IServiceManager.h>
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <binder/Status.h>
#include <utils/String16.h>
#include <stdio.h>
#include <vector>
#include <optional>
#include <algorithm>
#include <unordered_set>
#include "yaml_utils.h"

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


sp<IAudioSinkManager> service;
sp<IAudioSink> _audioSink;
IAudioSink::Id g_audiosinkId;

std::vector<int> _audiosink_ids_from_profile;
std::vector<int> _audiodecoder_ids_from_profile;
bool supportsLowLatency_from_profile;
int systemMixerSampleRateHz_from_profile;
com::rdk::hal::audiodecoder::PCMFormat systemMixerPCMFormat_from_profile;
bool supportsPlanarFormat_from_profile;


class AudioSinkControllerListener : public BnAudioSinkControllerListener {
public:

    AudioSinkControllerListener() {};
    virtual ~AudioSinkControllerListener() = default;

    ::android::binder::Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onFirstFrameRendered(int64_t nsPresentationTime) override 
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onEndOfStream(int64_t nsPresentationTime) override 
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onAudioUnderflow() override 
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onAudioResumed(int64_t nsPresentationTime) override 
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onFlushComplete() override 
    {
        return ::android::binder::Status::ok();
    }
};

class AudioSinkEventListener : public BnAudioSinkEventListener {
public:

    AudioSinkEventListener() {};
    virtual ~AudioSinkEventListener() = default;

    ::android::binder::Status onStateChanged(::com::rdk::hal::State oldState, ::com::rdk::hal::State newState) override
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onFirstFrameRendered(int64_t nsPresentationTime) override 
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onEndOfStream(int64_t nsPresentationTime) override 
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onAudioUnderflow() override 
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onAudioResumed(int64_t nsPresentationTime) override 
    {
        return ::android::binder::Status::ok();
    }
    ::android::binder::Status onFlushComplete() override 
    {
        return ::android::binder::Status::ok();
    }
};

class L1AudioSinkSetPropertyTest : public ::testing::Test
{
protected:
    std::map<::com::rdk::hal::audiosink::Property, ::com::rdk::hal::PropertyValue> propertyMap;
    void SetUp() override
    {
        for (auto property :
                {
                        ::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL,
                        ::com::rdk::hal::audiosink::Property::AV_SOURCE,
                        ::com::rdk::hal::audiosink::Property::MIXER_ID,
                        ::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK
                }) {
            std::optional<::com::rdk::hal::PropertyValue> propertyValue;
            Status status = _audioSink->getProperty(property, &propertyValue);
            ASSERT_TRUE(status.isOk()) << "getProperty failed for property: " << static_cast<int32_t>(property);
            if (status.isOk() && propertyValue.has_value())
            {
                propertyMap[property] = propertyValue.value();
            }
            else
            {
                printf("Failed to get property\n");
            }
        }
    }
    void TearDown() override
    {
        for (const auto& [property, originalValue] : propertyMap)
        {
            bool _aidl_return;
            android::binder::Status status = _audioSink->setProperty(property, originalValue, &_aidl_return);
            ASSERT_TRUE(status.isOk()) << "Failed to revert property " << static_cast<int32_t>(property)
                                       << " to its original value. Status: " << status.toString8() << std::endl;
        }
    }
};

class L1AudioSinkOpenTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        printf("Set Up : L1AudioSinkOpenTest\n");
        //prerequisite - checking state
        ::com::rdk::hal::State state;
        ::android::binder::Status status = _audioSink->getState(&state);
        EXPECT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::CLOSED);

        sp<IAudioSinkControllerListener> listener = new AudioSinkControllerListener();
        status = _audioSink->open(listener, &audioSinkController);
        ASSERT_TRUE(status.isOk());
        ASSERT_NE(audioSinkController, nullptr) << "audioSinkController is null after open()\n";

        status = _audioSink->getState(&state);
        EXPECT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::READY);

    }
    void TearDown() override
    {
        printf("Teardown : L1AudioSinkOpenTest\n");
        ::com::rdk::hal::State state;
        Status status = _audioSink->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::READY);//change
        bool _aidl_return;
        status = _audioSink->close(audioSinkController, &_aidl_return);
        ASSERT_TRUE(status.isOk());
    }
    android::sp<IAudioSinkController> audioSinkController;
};

class L1AudioSinkCloseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        printf("Set Up : L1AudioSinkCloseTest\n");
        //prerequisite - checking state
        ::com::rdk::hal::State state;
        ::android::binder::Status status = _audioSink->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::CLOSED);

        sp<IAudioSinkControllerListener> listener = new AudioSinkControllerListener();
        status = _audioSink->open(listener, &audioSinkController);
        ASSERT_TRUE(status.isOk());
        ASSERT_NE(audioSinkController, nullptr) << "audioSinkController is null after open()\n";

        status = _audioSink->getState(&state);
        EXPECT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::READY);
    }
    android::sp<::com::rdk::hal::audiosink::IAudioSinkController> audioSinkController;
};

class L1AudioSinkEventRegisterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        audioSinkEventListener = new AudioSinkEventListener();
        bool _aidl_return;
        ::android::binder::Status status = _audioSink->registerEventListener(audioSinkEventListener, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);
    }
    android::sp<::com::rdk::hal::audiosink::IAudioSinkEventListener> audioSinkEventListener;
};

class L1AudioSinkStartTest : public L1AudioSinkOpenTest
{
protected:
    void SetUp() override
    {
        L1AudioSinkOpenTest::SetUp();
        printf("Set Up : L1AudioSinkStartTest\n");
        Status status = audioSinkController->start();
        ASSERT_TRUE(status.isOk());

        ::com::rdk::hal::State state;
        status = _audioSink->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::STARTED);

    }
    void TearDown() override
    {
        printf("Teardown : L1AudioSinkStartTest\n");
        ::com::rdk::hal::State state;
        Status status = _audioSink->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::STARTED);

        status = audioSinkController->stop();
        ASSERT_TRUE(status.isOk());
        L1AudioSinkOpenTest::TearDown();
    }
};

class L1AudioSinkStopTest : public L1AudioSinkOpenTest
{
protected:
    void SetUp() override
    {
        L1AudioSinkOpenTest::SetUp();
        printf("Set Up : L1AudioSinkStopTest\n");
        Status status = audioSinkController->start();
        ASSERT_TRUE(status.isOk());

        ::com::rdk::hal::State state;
        status = _audioSink->getState(&state);
        ASSERT_TRUE(status.isOk());
        ASSERT_EQ(state, ::com::rdk::hal::State::STARTED);
    }
    void TearDown() override
    {
        printf("Teardown : L1AudioSinkStopTest\n");
        L1AudioSinkOpenTest::TearDown();
    }
};

int getService() 
{
    service = android::interface_cast<IAudioSinkManager>(
            defaultServiceManager()->getService(String16(IAudioSinkManager::serviceName().c_str())));
    if (service == nullptr) 
    {
        printf("Service not available\n");
        return -1;
    }
    return 0;
}

/*currently HAL FEATURE PROFILE contain string value for systemMixerPCMFormat instead of enum. so the helper required to convert*/
com::rdk::hal::audiodecoder::PCMFormat StringToPCMformat(const std::string& format) 
{
    static const std::unordered_map<std::string, com::rdk::hal::audiodecoder::PCMFormat> formatMap = {
    {"F64LE", com::rdk::hal::audiodecoder::PCMFormat::F64LE},
    {"F64BE", com::rdk::hal::audiodecoder::PCMFormat::F64BE},
    {"F32LE", com::rdk::hal::audiodecoder::PCMFormat::F32LE},
    {"F32BE", com::rdk::hal::audiodecoder::PCMFormat::F32BE},
    {"S32LE", com::rdk::hal::audiodecoder::PCMFormat::S32LE},
    {"S32BE", com::rdk::hal::audiodecoder::PCMFormat::S32BE},
    {"U32LE", com::rdk::hal::audiodecoder::PCMFormat::U32LE},
    {"U32BE", com::rdk::hal::audiodecoder::PCMFormat::U32BE},
    {"S24_32LE", com::rdk::hal::audiodecoder::PCMFormat::S24_32LE},
    {"S24_32BE", com::rdk::hal::audiodecoder::PCMFormat::S24_32BE},
    {"U24_32LE", com::rdk::hal::audiodecoder::PCMFormat::U24_32LE},
    {"U24_32BE", com::rdk::hal::audiodecoder::PCMFormat::U24_32BE},
    {"S24LE", com::rdk::hal::audiodecoder::PCMFormat::S24LE},
    {"S24BE", com::rdk::hal::audiodecoder::PCMFormat::S24BE},
    {"U24LE", com::rdk::hal::audiodecoder::PCMFormat::U24LE},
    {"U24BE", com::rdk::hal::audiodecoder::PCMFormat::U24BE},
    {"S20LE", com::rdk::hal::audiodecoder::PCMFormat::S20LE},
    {"S20BE", com::rdk::hal::audiodecoder::PCMFormat::S20BE},
    {"U20LE", com::rdk::hal::audiodecoder::PCMFormat::U20LE},
    {"U20BE", com::rdk::hal::audiodecoder::PCMFormat::U20BE},
    {"S18LE", com::rdk::hal::audiodecoder::PCMFormat::S18LE},
    {"S18BE", com::rdk::hal::audiodecoder::PCMFormat::S18BE},
    {"U18LE", com::rdk::hal::audiodecoder::PCMFormat::U18LE},
    {"U18BE", com::rdk::hal::audiodecoder::PCMFormat::U18BE},
    {"S16LE", com::rdk::hal::audiodecoder::PCMFormat::S16LE},
    {"S16BE", com::rdk::hal::audiodecoder::PCMFormat::S16BE},
    {"U16LE", com::rdk::hal::audiodecoder::PCMFormat::U16LE},
    {"U16BE", com::rdk::hal::audiodecoder::PCMFormat::U16BE},
    {"S8", com::rdk::hal::audiodecoder::PCMFormat::S8},
    {"U8", com::rdk::hal::audiodecoder::PCMFormat::U8}
    };

    auto it = formatMap.find(format);
    if (it != formatMap.end()) 
    {
        return it->second;
    }
    return static_cast<com::rdk::hal::audiodecoder::PCMFormat>(-1);
}

// Helper function to validate platform capabilities
void validatePlatformCapabilities(const ::com::rdk::hal::audiosink::PlatformCapabilities& capabilities)
{
    EXPECT_EQ(capabilities.supportsLowLatency,supportsLowLatency_from_profile) <<"supportsLowLatency not matching with profile";
    EXPECT_EQ(capabilities.systemMixerSampleRateHz,systemMixerSampleRateHz_from_profile) <<"systemMixerSampleRateHz not matching with profile";
    EXPECT_EQ(capabilities.systemMixerPCMFormat,systemMixerPCMFormat_from_profile) <<"systemMixerPCMFormat not matching with profile";
    EXPECT_EQ(capabilities.supportsPlanarFormat,supportsPlanarFormat_from_profile) <<"supportsPlanarFormat not matching with profile"; 
}

bool helper_validatePropertyValue( const std::optional<::com::rdk::hal::PropertyValue>& propertyValue, const ::com::rdk::hal::audiosink::Property property)
{
    if (!propertyValue.has_value() || !propertyValue->value.has_value())
    {
        return false;
    }

    std::variant<int32_t, std::string> val;

    const auto& propVal = propertyValue->value.value();
    switch (propVal.getTag())
    {
        case PropertyValue::Value::Tag::intValue:
            val = propVal.get<PropertyValue::Value::Tag::intValue>();
            break;
        case PropertyValue::Value::Tag::stringValue:
        {
            android::String16 string16Val = propVal.get<PropertyValue::Value::Tag::stringValue>();
            android::String8 string8Val(string16Val);  // Convert String16 to String8
            val = std::string(string8Val.string());    // Convert String8 to std::string
        }
            break;
        case PropertyValue::Value::Tag::booleanValue:
            val = propVal.get<PropertyValue::Value::Tag::booleanValue>() ? 1 : 0;  // Store as int32_t (1 for true, 0 for false)
            break;

        default:
            std::cerr << "Unsupported type in PropertyValue::Value" << std::endl;
            break;
    }
    

    switch (property)
    {
        case ::com::rdk::hal::audiosink::Property::RESOURCE_ID:
            if(std::holds_alternative<int32_t>(val) && std::get<int32_t>(val) == g_audiosinkId.value)
             {
                return true;
             }
             else
             {
                std::cout<<"invalid RESOURCE_ID "<<std::endl;
                return false;
             }

        case ::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL:
            return std::holds_alternative<int32_t>(val) && std::get<int32_t>(val) == -31;

        case ::com::rdk::hal::audiosink::Property::AV_SOURCE:
            return std::holds_alternative<int32_t>(val) &&
                   (std::get<int32_t>(val) == static_cast<int32_t>(::com::rdk::hal::AVSource::AUTO) ||
                    std::get<int32_t>(val) == static_cast<int32_t>(::com::rdk::hal::AVSource::UNKNOWN) ||
                    (std::get<int32_t>(val) >= static_cast<int32_t>(::com::rdk::hal::AVSource::IP) &&
                     std::get<int32_t>(val) <= static_cast<int32_t>(::com::rdk::hal::AVSource::SYSTEM)) ||
                    (std::get<int32_t>(val) >= static_cast<int32_t>(::com::rdk::hal::AVSource::HDMI_1) &&
                     std::get<int32_t>(val) <= static_cast<int32_t>(::com::rdk::hal::AVSource::HDMI_5)) ||
                    (std::get<int32_t>(val) >= static_cast<int32_t>(::com::rdk::hal::AVSource::COMPOSITE_1) &&
                     std::get<int32_t>(val) <= static_cast<int32_t>(::com::rdk::hal::AVSource::COMPOSITE_5)));

        case ::com::rdk::hal::audiosink::Property::MIXER_ID:
            return std::holds_alternative<int32_t>(val) && (std::get<int32_t>(val) >= 0);

        case ::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK:
            return std::holds_alternative<int32_t>(val) &&
                   (std::get<int32_t>(val) == 0 || std::get<int32_t>(val) == 1);

        case ::com::rdk::hal::audiosink::Property::RENDER_LATENCY_NS:
            return std::holds_alternative<int>(val);

        case ::com::rdk::hal::audiosink::Property::QUEUE_DEPTH_NS:
            return std::holds_alternative<int>(val);       

        default:
            return true;
    }
}

void helper_populateFrameMetadata(::com::rdk::hal::audiodecoder::FrameMetadata& metadata)
{
    metadata.sourceCodec = ::com::rdk::hal::audiodecoder::Codec::PCM;
    metadata.isDolbyAtmos = false;
    metadata.numChannels = 2;
    metadata.channelTypes = {::com::rdk::hal::audiodecoder::ChannelType::FRONT_LEFT, ::com::rdk::hal::audiodecoder::ChannelType::FRONT_RIGHT};
    metadata.sampleRate = 48000;
    metadata.format = ::com::rdk::hal::audiodecoder::PCMFormat::S16LE;
    metadata.planarFormat = false;
    metadata.trimStartNs = 0;
    metadata.trimEndNs = 0;
    metadata.lowLatency = false;
    metadata.endOfStream = false;
    metadata.discontinuity = false;
    metadata.source = ::com::rdk::hal::AVSource::HDMI_1;
}

void helper_validateVolume(const Volume& _volume)
{
    ASSERT_TRUE(_volume.volume >= 0.0);
    ASSERT_TRUE(_volume.volume <= 1.0);
    ASSERT_TRUE(_volume.muted == true || _volume.muted == false);
}

void helper_setVolumeParcelable(Volume& volume, double volValue, bool mute)
{
    volume.volume = volValue;
    volume.muted = mute;
}

bool helper_areElementsUnique(const std::vector<IAudioSink::Id>& sinkIds) {
    std::unordered_set<int> uniqueElements;
    
    for (const auto& _sinkId : sinkIds) 
    {
        if(_sinkId.value < 0)
        {
            std::cout <<"find Invalid sink ID : " <<_sinkId.value <<std::endl;
            return false;
        } 
        if (uniqueElements.find(_sinkId.value) != uniqueElements.end()) 
        {
            std::cout<<"sink ID is not unique : " <<_sinkId.value <<std::endl;
            return false;
        }
        uniqueElements.insert(_sinkId.value);
    }
    return true;
}

/**
* @brief Tests the retrieval of audio sink IDs and ensures their validity and uniqueness
*
* This test case verifies that the method getAudioSinkIds() returns a valid and non-empty list of audio sink IDs. 
* It also checks for the uniqueness of these IDs and their presence in a predefined list of audio sink IDs from a HAL feature profile.@n
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 001 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize vector to store audio sink IDs | audioSinkIds | Should be empty initially | Should be successful |
* | 02 | Call getAudioSinkIds() to retrieve the audio sink IDs | audioSinkIds = empty vector | status.isOk() should be true | Should Pass |
* | 03 | Check if retrieved audio sink IDs list is not empty | audioSinkIds | Should not be empty | Should Pass |
* | 04 | Verify that all retrieved audio sink IDs are unique using helper function | audioSinkIds | All IDs should be unique | Should be successful |
* | 05 | Loop through each retrieved audio sink ID and verify it is listed in the HAL feature profile | audioSinkIds, _audiosink_ids_from_profile | Each ID should be found in the profile | Should Pass |
*/
TEST(L1AudioSinkManagerTest, test_l1_audiosink_positive_1_getAudioSinkIds)
{
    ::std::vector<IAudioSink::Id> audioSinkIds;

    Status status = service->getAudioSinkIds(&audioSinkIds);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(audioSinkIds.empty());

    ASSERT_TRUE(helper_areElementsUnique(audioSinkIds));

    /* checking whether retrieved audio sink ids from the api getAudioSinkIds() is listed in the HAL FEATURE PROFILE */
    for(const auto& _audioSinkId: audioSinkIds)
    {   
        auto result = std::find(_audiosink_ids_from_profile.begin(), _audiosink_ids_from_profile.end(), _audioSinkId.value);
        EXPECT_NE(result, _audiosink_ids_from_profile.end());
    }
}

/**
* @brief Test to validate the retrieval of platform capabilities in L1 audio sink
*
* This test ensures that the `getPlatformCapabilities` method of the L1 audio sink manager service is functioning correctly by verifying 
* that the method returns a successful status and valid platform capabilities. 
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
* | 01 | Define the PlatformCapabilities object | platformCapabilities | Object should be defined successfully | Should be successful |
* | 02 | Invoke getPlatformCapabilities method on service | &platformCapabilities | status.isOk() should be true | Should Pass |
* | 03 | Validate the platform capabilities | platformCapabilities | Platform capabilities should be valid | Should be successful |
*/
TEST(L1AudioSinkManagerTest, test_l1_audiosink_positive_1_getPlatformCapabilities)
{
    PlatformCapabilities platformCapabilities;
    Status status = service->getPlatformCapabilities(&platformCapabilities);
    EXPECT_TRUE(status.isOk());
    validatePlatformCapabilities(platformCapabilities);
}

/**
* @brief This test aims to validate the retrieval of an audio sink by its ID.
*
* This test will loop through a predefined list of audio sink IDs,
* retrieve the corresponding audio sink using the getAudioSink method from the service,
* and verify that the operation is successful.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set the audio sink ID to the  value from the profile | audioSinkId.value = value from profile | Should be successful  | Should be successful |
* | 02 | Call getAudioSink with the set audio sink ID and retrieve the audio sink | audioSinkId, audioSink | status.isOk() should be true, audioSink != nullptr | Should Pass |
* | 03 | Repeat steps 01 to 02 for each audio sink ID in the list | _audiosink_ids_from_profile |  | Should be successful |
*/
TEST(L1AudioSinkManagerTest, test_l1_audiosink_positive_1_getAudioSink)
{
    IAudioSink::Id audioSinkId;
    ::android::sp<::com::rdk::hal::audiosink::IAudioSink> audioSink;

    for(const auto& _audioSinkId: _audiosink_ids_from_profile)
    {
        audioSinkId.value = _audioSinkId;
        android::binder::Status status = service->getAudioSink(audioSinkId, &audioSink);
        EXPECT_TRUE(status.isOk());
        EXPECT_TRUE(audioSink != nullptr);
    }
}

/**
* @brief This test verifies the audiosink interface instance consistency during multiple invocations
*
* This test checks that multiple calls to getAudioSink() return the same IAudioSink instance by comparing properties. It ensures proper functionality and resource handling in the system.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 004@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Initialize audioSinkId, audioSink_1, audioSink_2, property, resource_id_1, resource_id_2 with initial values | audioSinkId = initial, audioSink_1 = nullptr, audioSink_2 = nullptr, property = RESOURCE_ID, resource_id_1 = std::nullopt, resource_id_2 = std::nullopt | Variables should be initialized successfully | Should be successful |
* | 02| Loop through _audiosink_ids_from_profile to get each _audioSinkId | _audioSinkId = [_value_from_profile] | Each _audioSinkId should be processed | Should be successful |
* | 03| Assign value to audioSinkId.value from _audioSinkId | audioSinkId.value = _audioSinkId | audioSinkId should be set | Should be successful |
* | 04| Call service->getAudioSink to get audioSink_1 | audioSinkId = _audioSinkId, audioSink_1 | service->getAudioSink should pass and audioSink_1 should be a valid pointer | Should Pass |
* | 05| Check if service->getAudioSink status is OK and audioSink_1 is not null | status.isOK() = true, audioSink_1 != nullptr | Verification should be successful | Should be successful |
* | 06| Call audioSink_1->getProperty to get resource_id_1 | audioSink_1, property = RESOURCE_ID, resource_id_1 | audioSink_1->getProperty should pass and resource_id_1 should be set | Should Pass |
* | 07| Call service->getAudioSink to get audioSink_2 | audioSinkId, audioSink_2 | service->getAudioSink should pass and audioSink_2 should be a valid pointer | Should Pass |
* | 08| Check if service->getAudioSink status is OK and audioSink_2 is not null | status.isOK() = true, audioSink_2 != nullptr | Verification should be successful | Should be successful |
* | 09| Call audioSink_2->getProperty to get resource_id_2 | audioSink_2, property = RESOURCE_ID, resource_id_2 | audioSink_2->getProperty should pass and resource_id_2 should be set | Should Pass |
* | 10| Get intValue from resource_id_1 | resource_id_1->value->get<int32_t> | int32_t _resource_id_1 should hold the int value extracted | Should be successful |
* | 11| Get intValue from resource_id_2 | resource_id_2->value->get<int32_t> | int32_t _resource_id_2 should hold the int value extracted | Should be successful |
* | 12| Validate whether multiple invocations give the same audiosink instance by comparing _resource_id_1 and _resource_id_2 | _resource_id_1, _resource_id_2 | The values of _resource_id_1 and _resource_id_2 should be equal | Should Pass |
*/
TEST(L1AudioSinkManagerTest, test_l1_audiosink_positive_2_getAudioSink)
{
    IAudioSink::Id audioSinkId;
    ::android::sp<::com::rdk::hal::audiosink::IAudioSink> audioSink_1;
    ::android::sp<::com::rdk::hal::audiosink::IAudioSink> audioSink_2;
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::RESOURCE_ID;
    std::optional<::com::rdk::hal::PropertyValue> resource_id_1;
    std::optional<::com::rdk::hal::PropertyValue> resource_id_2;

    for(const auto& _audioSinkId: _audiosink_ids_from_profile)
    {
        audioSinkId.value = _audioSinkId;
        android::binder::Status status = service->getAudioSink(audioSinkId, &audioSink_1);
        EXPECT_TRUE(status.isOk());
        EXPECT_TRUE(audioSink_1 != nullptr);

        status = audioSink_1->getProperty(property, &resource_id_1);
        EXPECT_TRUE(status.isOk());

        status = service->getAudioSink(audioSinkId, &audioSink_2);
        EXPECT_TRUE(status.isOk());
        EXPECT_TRUE(audioSink_2 != nullptr);

        status = audioSink_2->getProperty(property, &resource_id_2);
        EXPECT_TRUE(status.isOk());

        int32_t _resource_id_1 = resource_id_1->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();
        int32_t _resource_id_2 = resource_id_2->value->get<::com::rdk::hal::PropertyValue::Value::Tag::intValue>();

        /* validating whether multiple invocation gives same audiosink interface instance by checking RESOURCE_ID */
        EXPECT_EQ(_resource_id_1, _resource_id_2)<<"GOT DIFFERENT INSTANCE OF AUDIOSINK DURING MULTIPLE INVOCATION. ID for first invocation : "<<_resource_id_1 <<" ID for second invocation : "<<_resource_id_2 <<std::endl;

    }
}

/**
* @brief Test for verifying the positive scenario of getting an audio sink
*
* This test validates whether the service can correctly retrieve an audio sink when provided with a valid but undefined AudioSink ID. This helps ensure the function can handle edge cases gracefully.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 005 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Initialize IAudioSink::Id with UNDEFINED value | audioSinkId = UNDEFINED | Initialized audioSinkId should be undefined | Should be successful |
* | 02| Initialize audio sink object | audioSink = null | Initialized audioSink should be null | Should be successful |
* | 03| Call getAudioSink API | audioSinkId = UNDEFINED, audioSink = null | Status from API should be ok, audioSink should not be null | Should Pass |
*/
TEST(L1AudioSinkManagerTest, test_l1_audiosink_positive_3_getAudioSink)
{
    IAudioSink::Id audioSinkId;
    audioSinkId.value = IAudioSink::Id::UNDEFINED;
    ::android::sp<::com::rdk::hal::audiosink::IAudioSink> audioSink;

    android::binder::Status status = service->getAudioSink(audioSinkId, &audioSink);
    EXPECT_TRUE(status.isOk());
    EXPECT_TRUE(audioSink != nullptr);
}

/**
* @brief This test function is designed to test the negative scenario where an invalid AudioSink ID is provided.
*
* This test checks that the system correctly handles invalid input by providing a negative AudioSink ID and verifies that the operation fails as expected.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 006 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Set invalid AudioSink ID value | id.value = -2 | id.value = -2 | Should be successful |
* | 02| Call getAudioSink function with invalid ID | id.value = -2 | status.isOk() = false and audioSink = nullptr | Should Fail |
*/
TEST(L1AudioSinkManagerTest, test_l1_audiosink_negative_1_getAudioSink)
{
    ::com::rdk::hal::audiosink::IAudioSink::Id id;
    id.value = -2; // Invalid ID value
    ::android::sp<::com::rdk::hal::audiosink::IAudioSink> audioSink;
    ::android::binder::Status status = service->getAudioSink(id, &audioSink);
    ASSERT_FALSE(status.isOk());
    ASSERT_EQ(audioSink, nullptr);
}

/**
* @brief Validate the retrieval of audio sink capabilities
*
* This test aims to verify that the AudioSink component can correctly return its capabilities when requested. This
* is crucial for ensuring that the component is compliant with expected behaviors and can be integrated
* seamlessly with other parts of the audio system.
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
* | 01 | Instantiate a Capabilities object to hold the response | None | Should be successful | Should be successful |
* | 02 | Call getCapabilities on the AudioSink object and store the result in the Capabilities object | _aidl_return = Capabilities object | status.isOk() should be true | Should Pass |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_1_getCapabilities)
{
    ::com::rdk::hal::audiosink::Capabilities _aidl_return;
    ::android::binder::Status status = _audioSink->getCapabilities(&_aidl_return);
    ASSERT_TRUE(status.isOk());
}

/**
 * @brief This test case verifies that the getProperty function from the L1AudioSink module returns valid property values for a set of predefined properties.
 *
 * This test iterates through a list of predefined properties and verifies that the getProperty function returns valid and expected values for each property. This ensures that the getProperty function is working correctly and that the properties are being retrieved correctly.
 *
 * **Test Group ID:** Basic: 01 @n
 * **Test Case ID:** 008 @n
 * **Priority:** High @n
 * @n
 * **Pre-Conditions:** None @n
 * **Dependencies:** None @n
 * **User Interaction:** None @n
 *  @n
 * **Test Procedure:** @n
 * | Variation / Step | Description | Test Data |Expected Result |Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
 * | 01| Invoke getProperty for ::com::rdk::hal::audiosink::Property::RESOURCE_ID | prop = RESOURCE_ID, propertyValue = &propertyValue | status.isOk() should be true, propertyValue should match expectation| Should Pass |
 * | 02| Validate property value with helper_validatePropertyValue | prop = RESOURCE_ID, propertyValue = &propertyValue | Return true if the value is correct according to current audio sink ID | Should Pass |
 * | 03| Invoke getProperty for ::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL | prop = REFERENCE_LEVEL, propertyValue = &propertyValue | status.isOk() should be true, propertyValue should be -31 | Should Pass |
 * | 04| Validate property value with helper_validatePropertyValue | prop = REFERENCE_LEVEL, propertyValue = &propertyValue | Return true if the value is -31 | Should Pass |
 * | 05| Invoke getProperty for ::com::rdk::hal::audiosink::Property::RENDER_LATENCY_NS | prop = RENDER_LATENCY_NS, propertyValue = &propertyValue | status.isOk() should be true, propertyValue should be an integer | Should Pass |
 * | 06| Validate property value with helper_validatePropertyValue | prop = RENDER_LATENCY_NS, propertyValue = &propertyValue | Return true if the value is an integer | Should Pass |
 * | 07| Invoke getProperty for ::com::rdk::hal::audiosink::Property::QUEUE_DEPTH_NS | prop = QUEUE_DEPTH_NS, propertyValue = &propertyValue | status.isOk() should be true, propertyValue should be an integer | Should Pass |
 * | 08| Validate property value with helper_validatePropertyValue | prop = QUEUE_DEPTH_NS, propertyValue = &propertyValue | Return true if the value is an integer | Should Pass |
 * | 09| Invoke getProperty for ::com::rdk::hal::audiosink::Property::MIXER_ID | prop = MIXER_ID, propertyValue = &propertyValue | status.isOk() should be true, propertyValue should be 0 | Should Pass |
 * | 10| Validate property value with helper_validatePropertyValue | prop = MIXER_ID, propertyValue = &propertyValue | Return true if the value is 0 | Should Pass |
 * | 11| Invoke getProperty for ::com::rdk::hal::audiosink::Property::AV_SOURCE | prop = AV_SOURCE, propertyValue = &propertyValue | status.isOk() should be true, propertyValue should match any valid AVSource values| Should Pass |
 * | 12| Validate property value with helper_validatePropertyValue | prop = AV_SOURCE, propertyValue = &propertyValue | Return true if the value matches any valid AVSource value | Should Pass |
 * | 13| Invoke getProperty for ::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK | prop = DOLBY_ATMOS_LOCK, propertyValue = &propertyValue | status.isOk() should be true, propertyValue should be 0 or 1 | Should Pass |
 * | 14| Validate property value with helper_validatePropertyValue | prop = DOLBY_ATMOS_LOCK, propertyValue = &propertyValue | Return true if the value is 0 or 1 | Should Pass |
 */
TEST(L1AudioSinkTest, test_l1_audiosink_positive_1_getProperty)
{
    std::vector<::com::rdk::hal::audiosink::Property> properties = {
            ::com::rdk::hal::audiosink::Property::RESOURCE_ID,
            ::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL,
            ::com::rdk::hal::audiosink::Property::RENDER_LATENCY_NS,
            ::com::rdk::hal::audiosink::Property::QUEUE_DEPTH_NS,
            ::com::rdk::hal::audiosink::Property::MIXER_ID,
            ::com::rdk::hal::audiosink::Property::AV_SOURCE,
            ::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK
    };

    for (::com::rdk::hal::audiosink::Property prop : properties)
    {
        std::optional<PropertyValue> propertyValue;
        ::android::binder::Status status = _audioSink->getProperty(prop, &propertyValue);
        EXPECT_TRUE(status.isOk());
        EXPECT_TRUE(helper_validatePropertyValue(propertyValue, prop));
    }
}

/**
* @brief Test the invalid property scenario for the getProperty method in L1AudioSinkTest.
*
* This test validates the handling of invalid property IDs in the getProperty method of the L1AudioSinkTest class. It ensures that the API returns a failure status when an invalid property ID is requested.
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
* | 01| Initialize optional property value container | std::optional<PropertyValue> propertyValue; | Should be successful | Should be successful |
* | 02| Cast an invalid property ID to Property enum | ::com::rdk::hal::audiosink::Property invalidProperty = static_cast<::com::rdk::hal::audiosink::Property>(-1); | Should be successful |Should be successful |
* | 03| Invoke getProperty with invalid property | invalidProperty = -1, propertyValue = nullptr | status.isOk() should be false | Should Pass | 
*/
TEST(L1AudioSinkTest, test_l1_audiosink_negative_1_getProperty)
{
    std::optional<PropertyValue> propertyValue;
    ::com::rdk::hal::audiosink::Property invalidProperty = static_cast<::com::rdk::hal::audiosink::Property>(-1);
    ::android::binder::Status status = _audioSink->getProperty(invalidProperty, &propertyValue);
    EXPECT_FALSE(status.isOk());
}

/**
* @brief This test checks the functionality of setting a property in the L1AudioSink.
*
* This test specifically tests setting the REFERENCE_LEVEL property of the L1AudioSink.
* It ensures that a property can be successfully set and reverted back to its initial value
* without any issues.
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
* | Variation / Step | Description                        | Test Data                                 | Expected Result                                            | Notes |
* | :----:           | ---------                          | ----------                               | --------------                                             | ----- |
* | 01               | Retrieve initial value of property | property = REFERENCE_LEVEL               | status.isOk() and initialvalue should be set         | Should be successful |
* | 02               | Set the property value             | property = REFERENCE_LEVEL, value = -31  | status.isOk() and _aidl_return should be true        | Should Pass |
* | 03               | Revert to initial value            | property = REFERENCE_LEVEL, value = initial | status.isOk() and _aidl_return should be true        | Should Pass |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_1_setProperty)
{
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::REFERENCE_LEVEL;
    std::optional<PropertyValue> initialvalue;
    PropertyValue propertyValue;
    bool _aidl_return;

    /* storing initial value */
    Status status = _audioSink->getProperty(property, &initialvalue);
    ASSERT_TRUE(status.isOk()) << "getProperty failed for property: REFERENCE_LEVEL";

    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-31);
    status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);

    /*clean-up setting back initial value*/
    status = _audioSink->setProperty(property, initialvalue.value(), &_aidl_return);
    ASSERT_TRUE(status.isOk()) << "Failed to revert value for property : REFERENCE_LEVEL";
}

/**
* @brief Testing the negative scenario for setting property in L1AudioSink
*
* This test aims to validate the behavior of the `_audioSink->setProperty` function when faced with an invalid or failing situation. The test ensures that the proper status and return values are returned when attempting to set the audio sink property to RESOURCE_ID, with an explicit integer value.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize property to RESOURCE_ID | property = ::com::rdk::hal::audiosink::Property::RESOURCE_ID | Should be successful | Should be successful |
* | 02 | Allocate and set integer value in PropertyValue | propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1); | Should be successful |Should be successful |
* | 03 | Call _audioSink->setProperty with initialized property and propertyValue | input: property, propertyValue, output: &_aidl_return | status.isOk() = true; _aidl_return = false | Should Fail |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_negative_1_setProperty)
{
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::RESOURCE_ID;
    PropertyValue propertyValue;
    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1);
    bool _aidl_return;
    android::binder::Status status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief This test verifies that the audio sink property setting functionality works as expected.
*
* This test will check the ability to set and subsequently revert an audio sink property, specifically the MIXER_ID, and ensure that the API behaves correctly by setting a new value and reverting it back to its initial state.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Retrieve initial value of MIXER_ID property | property = MIXER_ID | status.isOk() should be true, initialvalue should have a valid value | Should be successful |
* | 02| Set MIXER_ID property to 0 | property = MIXER_ID, propertyValue = 0, _aidl_return to capture result | status.isOk() should be true, _aidl_return should be true | Should Pass |
* | 03| Revert MIXER_ID property to its initial value | property = MIXER_ID, initialvalue to set back, _aidl_return to capture result | status.isOk() should be true, _aidl_return should be true | Should Pass |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_2_setProperty)
{
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::MIXER_ID;
    PropertyValue propertyValue;
    bool _aidl_return;
    std::optional<PropertyValue> initialvalue;

    /* storing initial value */
    Status status = _audioSink->getProperty(property, &initialvalue);
    ASSERT_TRUE(status.isOk()) << "getProperty failed for property: MIXER_ID";

    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(0);
    status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);

    /*clean-up setting back initial value*/
    status = _audioSink->setProperty(property, initialvalue.value(), &_aidl_return);
    ASSERT_TRUE(status.isOk()) << "Failed to revert value for property : MIXER_ID";
}

/**
* @brief This test verifies the behavior of the setProperty function when an invalid property value is passed.
*
* The objective of this unit test is to ensure that invoking the setProperty function with an invalid property value fails as expected.
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
* | 01 | Initialize the property with MIXER_ID and set propertyValue to an invalid value | property = MIXER_ID, propertyValue.value = -1 | Property should be set | Should be successful |
* | 02 | Call the setProperty with invalid property value | property = MIXER_ID, propertyValue = -1, _aidl_return = false | status.isOk() should return true and _aidl_return should be false | Should fail |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_negative_3_setProperty) {
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::MIXER_ID;
    PropertyValue propertyValue;
    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-1);
    bool _aidl_return;
    android::binder::Status status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Test the setting of various AVSource properties in the audio sink.
*
* This test evaluates the setProperty function with different AVSource values, ensuring proper interaction with the audio sink. The initial value is preserved and restored after testing.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 014 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Store the initial value of AV_SOURCE property using GetProperty method | property = AV_SOURCE, initialvalue = None | Status should be OK, initialvalue should store the current AV_SOURCE | Should be successful |
* | 02 | Set AV_SOURCE property to AUTO and verify | property = AV_SOURCE, propertyValue = AUTO, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 03 | Set AV_SOURCE property to UNKNOWN and verify | property = AV_SOURCE, propertyValue = UNKNOWN, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 04 | Set AV_SOURCE property to IP and verify | property = AV_SOURCE, propertyValue = IP, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 05 | Set AV_SOURCE property to TUNER and verify | property = AV_SOURCE, propertyValue = TUNER, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 06 | Set AV_SOURCE property to SYSTEM and verify | property = AV_SOURCE, propertyValue = SYSTEM, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 07 | Set AV_SOURCE property to HDMI_1 and verify | property = AV_SOURCE, propertyValue = HDMI_1, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 08 | Set AV_SOURCE property to HDMI_2 and verify | property = AV_SOURCE, propertyValue = HDMI_2, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 09 | Set AV_SOURCE property to HDMI_3 and verify | property = AV_SOURCE, propertyValue = HDMI_3, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 10 | Set AV_SOURCE property to HDMI_4 and verify | property = AV_SOURCE, propertyValue = HDMI_4, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 11 | Set AV_SOURCE property to HDMI_5 and verify | property = AV_SOURCE, propertyValue = HDMI_5, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 12 | Set AV_SOURCE property to COMPOSITE_1 and verify | property = AV_SOURCE, propertyValue = COMPOSITE_1, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 13 | Set AV_SOURCE property to COMPOSITE_2 and verify | property = AV_SOURCE, propertyValue = COMPOSITE_2, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 14 | Set AV_SOURCE property to COMPOSITE_3 and verify | property = AV_SOURCE, propertyValue = COMPOSITE_3, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 15 | Set AV_SOURCE property to COMPOSITE_4 and verify | property = AV_SOURCE, propertyValue = COMPOSITE_4, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 16 | Set AV_SOURCE property to COMPOSITE_5 and verify | property = AV_SOURCE, propertyValue = COMPOSITE_5, _aidl_return = None | Status should be OK, _aidl_return should be true | Should Pass |
* | 17 | Restore the initial value of AV_SOURCE property using SetProperty method | property = AV_SOURCE, initialvalue = stored value, _aidl_return = None | Status should be OK, _aidl_return should be true | Done by Pre-requisite TearDown function |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_3_setProperty)
{
    bool _aidl_return;
    std::optional<PropertyValue> initialvalue;
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::AV_SOURCE;

    /* storing initial value */
    Status status = _audioSink->getProperty(property, &initialvalue);
    ASSERT_TRUE(status.isOk()) << "getProperty failed for property: AV_SOURCE";

    for (auto avSource :
    {
        ::com::rdk::hal::AVSource::AUTO,
        ::com::rdk::hal::AVSource::UNKNOWN,
        ::com::rdk::hal::AVSource::IP,
        ::com::rdk::hal::AVSource::TUNER,
        ::com::rdk::hal::AVSource::SYSTEM,
        ::com::rdk::hal::AVSource::HDMI_1,
        ::com::rdk::hal::AVSource::HDMI_2,
        ::com::rdk::hal::AVSource::HDMI_3,
        ::com::rdk::hal::AVSource::HDMI_4,
        ::com::rdk::hal::AVSource::HDMI_5,
        ::com::rdk::hal::AVSource::COMPOSITE_1,
        ::com::rdk::hal::AVSource::COMPOSITE_2,
        ::com::rdk::hal::AVSource::COMPOSITE_3,
        ::com::rdk::hal::AVSource::COMPOSITE_4,
        ::com::rdk::hal::AVSource::COMPOSITE_5
    }) {
            PropertyValue propertyValue;
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(int32_t(static_cast<int32_t>(avSource)));
            status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
            ASSERT_TRUE(status.isOk());
            ASSERT_TRUE(_aidl_return);
    }

    /*clean-up setting back initial value*/
    status = _audioSink->setProperty(property, initialvalue.value(), &_aidl_return);
    ASSERT_TRUE(status.isOk()) << "Failed to revert value for property : AV_SOURCE";
}

/**
* @brief Verifies the behavior of setProperty with invalid property value
*
* This test ensures that the setProperty function behaves correctly when called with an invalid property value of -2.@n
* @n
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
* | 01 | Set the property to AV_SOURCE | property = ::com::rdk::hal::audiosink::Property::AV_SOURCE | Should be successful | Should be successful |
* | 02 | Assign invalid integer value -2 to propertyValue | propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-2) | Should be successful | Should be successful |
* | 03 | Call setProperty with invalid propertyValue | property = ::com::rdk::hal::audiosink::Property::AV_SOURCE, propertyValue = -2, _aidl_return = false | status.isOk() should be true and _aidl_return should be false | Should Fail |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_negative_4_setProperty)
{
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::AV_SOURCE;
    PropertyValue propertyValue;
    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-2);
    bool _aidl_return;
    android::binder::Status status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Test for setting and reverting the DOLBY_ATMOS_LOCK property on an audio sink.
*
* This test verifies the proper behavior of setting a property on an audio sink and then reverting it back to its original value. It ensures that the DOLBY_ATMOS_LOCK property can be set successfully and that the expected results are returned.
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
* | :--------------: | ----------- | --------- | --------------- | ----- |
* | 01 | Store initial value of DOLBY_ATMOS_LOCK property | property = DOLBY_ATMOS_LOCK, initialvalue = null | status.isOk() = true | Should be successful |
* | 02 | Set property value to 1 | propertyValue.value = 1 | status.isOk() = true, _aidl_return = true | Should Pass |
* | 03 | Revert property to initial value | property = DOLBY_ATMOS_LOCK, initialvalue = stored value | status.isOk() = true | Should be successful |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_4_setProperty)
{
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK;
    PropertyValue propertyValue;
    std::optional<PropertyValue> initialvalue;
    bool _aidl_return;

    /* storing initial value */
    Status status = _audioSink->getProperty(property, &initialvalue);
    ASSERT_TRUE(status.isOk()) << "getProperty failed for property: DOLBY_ATMOS_LOCK";

    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(1);
    status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);

    /*clean-up setting back initial value*/
    status = _audioSink->setProperty(property, initialvalue.value(), &_aidl_return);
    ASSERT_TRUE(status.isOk()) << "Failed to revert value for property : DOLBY_ATMOS_LOCK";
}

/**
* @brief Tests the setting of an audio property in the AudioSink module.
*
* This test verifies the correct behavior of setting a property in the AudioSink module, specifically the DOLBY_ATMOS_LOCK property. It checks both the setting and reverting of this property to ensure proper handling.
* @n
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 017@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* |Variation / Step | Description | Test Data | Expected  Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Initialize Property and initialvalue variables, and define the property to DOLBY_ATMOS_LOCK | property = DOLBY_ATMOS_LOCK | Initialization should be successful | Should be successful |
* | 02 | Call getProperty to store the initial value in initialvalue variable | property = DOLBY_ATMOS_LOCK | status.isOk() should be true | Should be successful |
* | 03 | Set the property value to 0 | propertyValue.value = 0 | The value should be set successfully | Should be successful |
* | 04 | Call setProperty to set the property to the new value | property = DOLBY_ATMOS_LOCK, propertyValue = 0 | status.isOk() should be true and the aidl_return should be true | Should Pass |
* | 05 | Call setProperty to revert the property to its initial value | property = DOLBY_ATMOS_LOCK, initialvalue | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_5_setProperty)
{
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK;
    PropertyValue propertyValue;
    std::optional<PropertyValue> initialvalue;
    bool _aidl_return;

    /* storing initial value */
    Status status = _audioSink->getProperty(property, &initialvalue);
    ASSERT_TRUE(status.isOk()) << "getProperty failed for property: DOLBY_ATMOS_LOCK";


    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(0);
    status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);

    /*clean-up setting back initial value*/
    status = _audioSink->setProperty(property, initialvalue.value(), &_aidl_return);
    ASSERT_TRUE(status.isOk()) << "Failed to revert value for property : DOLBY_ATMOS_LOCK";
}

/**
* @brief Test the setProperty function with negative property value
*
* This test checks the behavior of the setProperty function when it is provided with an invalid property value.@n
* This is to ensure that the function properly handles negative scenarios by returning appropriate error statuses.@n
* @n
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 018@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                         | Test Data                                              | Expected Result                      | Notes                        |
* | :--------------: | ----------------------------------- | ------------------------------------------------------ | ----------------------------------- | ----------------------------- |
* | 01               | Prepare the property variable.      | property = DOLBY_ATMOS_LOCK                             | Should be successful                | Should be successful         |
* | 02               | Prepare the propertyValue variable. | propertyValue.value = -1                                | Should be successful                | Should be successful         |
* | 03               | Call setProperty API.               | property=DOLBY_ATMOS_LOCK, propertyValue.value=-1,      | status.isOk()=true, _aidl_return=false | Should Fail              |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_negative_5_setProperty)
{
    ::com::rdk::hal::audiosink::Property property = ::com::rdk::hal::audiosink::Property::DOLBY_ATMOS_LOCK;
    PropertyValue propertyValue;
    bool _aidl_return;
    propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(-1);
    ::android::binder::Status status = _audioSink->setProperty(property, propertyValue, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Test to verify the getState function of AudioSink.
*
* This test checks if the getState function of AudioSink returns a valid state and ensures the status is OK.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Setup the test environment | _audioSink initialized | Environment setup for the test | should be successful |
* | 02| Call getState function on the audioSink object | state | status.isOk() should be true, and state to be a valid enum value | Should Pass |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_1_getState)
{
    ::com::rdk::hal::State state;
    ::android::binder::Status status = _audioSink->getState(&state);
    ASSERT_TRUE(status.isOk());
    bool validEnumValue = (
            state == ::com::rdk::hal::State::UNKNOWN ||
            state == ::com::rdk::hal::State::CLOSED ||
            state == ::com::rdk::hal::State::OPENING ||
            state == ::com::rdk::hal::State::READY ||
            state == ::com::rdk::hal::State::STARTING ||
            state == ::com::rdk::hal::State::STARTED ||
            state == ::com::rdk::hal::State::FLUSHING ||
            state == ::com::rdk::hal::State::STOPPING ||
            state == ::com::rdk::hal::State::CLOSING
    );
    ASSERT_TRUE(validEnumValue);
}

/**
* @brief Test to validate the positive scenario of opening an audio sink
*
* This test verifies if the audio sink can be opened successfully when it is in the CLOSED state@n
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
* | 01 | Retrieve the current state of the audio sink | input: None, output: state | state should be CLOSED | Should be successful |
* | 02 | Create listener object for AudioSinkController | input: None, output: listener | listener should not be null | Should be successful |
* | 03 | Open the audio sink with the created listener | input: listener, output: audioSinkController | status.isOk() should be true, audioSinkController should not be null | Should Pass |
* | 04 | Close the audio sink to cleanup | input: audioSinkController, output: _aidl_return | status should be Ok, _aidl_return should be true | Done by Pre-requisite TearDown function |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_1_open)
{
    //prerequisite - checking state
    ::com::rdk::hal::State state;
    ::android::binder::Status status = _audioSink->getState(&state);
    ASSERT_TRUE(status.isOk());
    ASSERT_EQ(state, ::com::rdk::hal::State::CLOSED);

    android::sp<IAudioSinkController> audioSinkController;
    sp<IAudioSinkControllerListener> listener = new AudioSinkControllerListener();
    status = _audioSink->open(listener, &audioSinkController);
    ASSERT_TRUE(status.isOk());
    ASSERT_NE(audioSinkController, nullptr);
    //cleaning up
    bool _aidl_return;
    status = _audioSink->close(audioSinkController, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test the behavior of `open` function when called in a non-allowed state.
*
* This test verifies the behavior of the `open` function when it is called in the `READY` state, 
* which is expected to fail with an `EX_ILLEGAL_STATE` status due to it being an invalid state for the operation.
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
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Call the `open` function from the `READY` state (should fail) | listener=AudioSinkControllerListener() | status.exceptionCode() = EX_ILLEGAL_STATE | Should Fail |
* | 04 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_negative_1_open)
{
    android::sp<IAudioSinkControllerListener> audioSinkControllerListener = new AudioSinkControllerListener();
    android::sp<IAudioSinkController> aidl_return;
    //calling open() from READY state (EX_ILLEGAL_STATE)
    android::binder::Status status = _audioSink->open(audioSinkControllerListener, &aidl_return);
    ASSERT_EQ(status.exceptionCode(), android::binder::Status::EX_ILLEGAL_STATE);
}

/**
 * @brief Test the successful closure of an audio sink.
 *
 * This test verifies whether the audio sink can be successfully closed after being opened. 
 * It ensures that the audio sink state transitions are correctly handled.
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
 * | Variation / Step| Description  | Test Data | Expected Result| Notes|
 * | :-------------: | -------------| ----------| -------------- | -----|
 * | 01| Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
 * | 02| Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
 * | 03| Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
 * | 04| Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
 * | 05| ASSERT that _audioSink object is not null.| _audioSink | _audioSink != null | Should Pass|
 * | 06| ASSERT that audioSinkController is not null.| audioSinkController |audioSinkController != null| Should Pass|
 * | 07| Closing the audio sink using the audio sink controller and getting return value and status.  | audioSinkController, aidl_return| status.isOk() should be true and aidl_return is true| Should Pass|
 */
TEST_F(L1AudioSinkCloseTest, test_l1_audiosink_positive_1_close)
{
    ASSERT_NE(_audioSink, nullptr) << "audioDecoder is null, cannot proceed with test.";
    ASSERT_NE(audioSinkController, nullptr) << "audioDecoderController is null, cannot proceed with test.";

    bool aidl_return;
    Status status = _audioSink->close(audioSinkController, &aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(aidl_return);
}

/**
* @brief Test the negative scenario for closing the AudioSink when it is in the STARTED state
*
* This test checks the behavior of the `close` function when attempting to close an AudioSink that is in the STARTED state, 
* which should raise an IllegalStateException
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 023 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n 
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Start the audio sink controller | None | status.isOk() = true | Should be successful |
* | 06 | Attempt to close the audio sink while in STARTED state | audioSinkController, &aidl_return | status.isok() = true, aidl_return = false | Should Fail |
* | 07 | Stop the audio sink controller | None | status.isOk() = true | Should be successful |
* | 08 | Close the audio sink | audioSinkController, &aidl_return | status.isOk() = true, aidl_return = true | Should Pass |
*/
TEST_F(L1AudioSinkCloseTest, test_l1_audiosink_negative_1_close)
{
    bool aidl_return;
    //changing state to STARTED
    Status status = audioSinkController->start();
    ASSERT_TRUE(status.isOk());
    //calling close() from STARTED state
    status = _audioSink->close(audioSinkController, &aidl_return);
    ASSERT_TRUE(status.isOk());    
    ASSERT_FALSE(aidl_return);
    //clean up
    status = audioSinkController->stop();
    ASSERT_TRUE(status.isOk());
    status = _audioSink->close(audioSinkController, &aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(aidl_return);
}

/**
* @brief Test for registering event listener to the audio sink
*
* This test case validates the functionality of registering an event listener to the audio sink and ensures that the operation succeeds. It tests the interaction between the audio sink and the event listener to check if the registration and deregistration are handled correctly.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 024 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01| Create a new instance of AudioSinkEventListener. |None | Should be successful | Should be successful |
* | 02| Register the event listener to the audio sink using registerEventListener. | audioSinkEventListener | status.isOk() should be true and '_aidl_return' should be true. | Should Pass |
* | 03| Unregister the event listener to clean up using unregisterEventListener. | audioSinkEventListener | status.isOk() should be true and '_aidl_return' should be true. | Should Pass |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_positive_1_registerEventListener)
{
    android::sp<IAudioSinkEventListener> audioSinkEventListener = new AudioSinkEventListener();
    bool _aidl_return;
    ::android::binder::Status status = _audioSink->registerEventListener(audioSinkEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
    //clean up
    status = _audioSink->unregisterEventListener(audioSinkEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Verifies that registering an already registered audio sink event listener fails.
*
* This test ensures that attempting to register an AudioSinkEventListener that is already
* registered results in an appropriate failure response, thus preventing duplicate registrations.
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
* | 01 | SetUp() function is invoked to initialize the audio sink event listener and register it for the first time | audioSinkEventListener, _aidl_return | status.isOk() should be true and _aidl_return is true | Done by Pre-requisite SetUp function |
* | 02 | Attempt to register the audioSinkEventListener again to simulate registration failure | audioSinkEventListener | status.isOk() should be true and _aidl_return is false | Should Fail |
* | 03 | Unregister the audioSinkEventListener to clean up | audioSinkEventListener | status.isOk() should be true and _aidl_return is true | Should be successful |
*/
TEST_F(L1AudioSinkEventRegisterTest, test_l1_audiosink_negative_1_registerEventListener)
{
    bool _aidl_return;
    // Register the listener for the second time (first time from the setup() function)
    Status status = _audioSink->registerEventListener(audioSinkEventListener, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
    //clean up
    status = _audioSink->unregisterEventListener(audioSinkEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test for successful unregistration of an audio sink event listener.
*
* This test verifies that the `unregisterEventListener` function works correctly by unregistering a previously registered audio sink event listener and checking for successful status and return value.
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
* | 01 | SetUp() function is invoked to initialize the audio sink event listener and register it for the first time | audioSinkEventListener, _aidl_return | status.isOk() should be true and _aidl_return is true | Done by Pre-requisite SetUp function |
* | 02 | Register the event listener using the `registerEventListener` method | audioSinkEventListener | Status and _aidl_return should be true | Done by Pre-requisite SetUp function |
* | 03 | Unregister the audioSinkEventListener | audioSinkEventListener | status.isOk() should be true and _aidl_return is true | Should be successful |
*/
TEST_F(L1AudioSinkEventRegisterTest, test_l1_audiosink_positive_1_unregisterEventListener)
{
    bool _aidl_return;
    android::binder::Status status = _audioSink->unregisterEventListener(audioSinkEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    EXPECT_TRUE(_aidl_return);
}

/**
* @brief Test to verify the negative scenario for unregistering an event listener from IAudioSink
*
* This test checks the behavior of the `unregisterEventListener` method when called with an invalid or unregistered listener instance.
* The test is necessary to ensure that the function handles error conditions properly and does not falsely unregister a listener.
*@n
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 027@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
*
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Create a new instance of `AudioSinkEventListener` | listener = new AudioSinkEventListener() | Instance should be created successfully | Should be successful |
* | 02 | Call `unregisterEventListener` with the created listener | listener, _aidl_return (uninitialized) | status.isOk() should be true, _aidl_return false | Should Fail |
*/
TEST(L1AudioSinkTest, test_l1_audiosink_negative_1_unregisterEventListener)
{
    sp<IAudioSinkEventListener> listener = new AudioSinkEventListener();
    bool _aidl_return;
    android::binder::Status status = _audioSink->unregisterEventListener(listener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Testing the unregisterEventListener functionality with a negative scenario
*
* This test case verifies the behavior of the system when attempting to unregister an already unregistered event listener for an audio sink.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 028 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description      | Test Data                               | Expected Result                | Notes                     |
* | :--------------: | ---------------- | -------------------------------------- | ------------------------------ | ------------------------- |
* | 01 | SetUp() function is invoked to initialize the audio sink event listener and register it for the first time | audioSinkEventListener, _aidl_return | status.isOk() should be true and _aidl_return is true | Done by Pre-requisite SetUp function |
* | 02 | Unregister the event listener | audioSinkEventListener, _aidl_return     | status.isOk() and _aidl_return should be true  | Should Pass|
* | 03 | Attempt to unregister the already unregistered event listener | audioSinkEventListener, _aidl_return |  status.isOk() = true and _aidl_return should be false | Should Fail |
*/
TEST_F(L1AudioSinkEventRegisterTest, test_l1_audiosink_negative_2_unregisterEventListener)
{
    // Unregister the listener
    bool _aidl_return;
    Status status = _audioSink->unregisterEventListener(audioSinkEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);

    // Try to unregister again
    status = _audioSink->unregisterEventListener(audioSinkEventListener, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief This test verifies the functionality of setting the audio decoder with specified IDs in the audio sink controller.
*
* This test ensures that the audio decoder can be correctly set in the audio sink controller by validating the status and return value for each audio decoder ID retrieved from the profile.
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
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Retrieve each audio decoder ID from the profile and set the audio decoder in the audio sink controller |  audioDecoderId.value = id from profile | Should Pass |  |
* | 04 | Validate that the status is OK for setting the audio decoder | status.isOk() | status.isOk() should be true | Should be successful |
* | 05 | Validate that the return value is true for setting the audio decoder | _aidl_return | _aidl_return should be true  | Should be successful |
* | 06 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_positive_1_setAudioDecoder)
{
    ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId;
    bool _aidl_return;

    for(const auto& _id: _audiodecoder_ids_from_profile)
    {
        audioDecoderId.value = _id;
        ::android::binder::Status status = audioSinkController->setAudioDecoder(audioDecoderId, &_aidl_return);
        ASSERT_TRUE(status.isOk());
        ASSERT_TRUE(_aidl_return);
    }
}

/**
* @brief This test verifies the functionality of setting an audio decoder in the audio sink controller.
*
* This test ensures that the audio sink controller can successfully set an audio decoder using a valid decoder ID.
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
* | 01 | Invoke SetUp function | None | Must set the state to READY, audioSinkController should not be null | Done by Pre-requisite SetUp function |
* | 02 | Check if _audioSink is not null | None | Test should continue only if _audioSink is not null | Should be successful |
* | 03 | Check if audioSinkController is not null | None | Test should continue only if audioSinkController is not null | Should be successful |
* | 04 | Set audioDecoderId to UNDEFINED | audioDecoderId.value = UNDEFINED | Should set a valid ID for audio decoder | Should be successful | 
* | 05 | Call setAudioDecoder on audioSinkController | audioDecoderId.value = UNDEFINED | status.isOk() should be true and _aidl_return should be true | Should Pass |
* | 06 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_positive_2_setAudioDecoder)
{
    ASSERT_NE(_audioSink, nullptr) << "audioDecoder is null, cannot proceed with test.";
    ASSERT_NE(audioSinkController, nullptr) << "audioDecoderController is null, cannot proceed with test.";
    ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId;
    bool _aidl_return;
    audioDecoderId.value = ::com::rdk::hal::audiodecoder::IAudioDecoder::Id::UNDEFINED;
//    audioDecoderId.value = -1;
    ::android::binder::Status status = audioSinkController->setAudioDecoder(audioDecoderId, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test for setting an invalid audio decoder ID to ensure proper error handling.
*
* This test is designed to verify that the `setAudioDecoder` API correctly handles an invalid audio decoder ID by returning an error status and ensuring the decoder is not set.
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
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Set an invalid audio decoder ID (-2) using the `setAudioDecoder` API. | audioDecoderId.value = -2, _aidl_return | status.isOk() should be true, _aidl_return should be false. | Should Fail |
* | 04 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_negative_1_setAudioDecoder)
{
    ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId;
    bool _aidl_return;
    audioDecoderId.value = -2;
    ::android::binder::Status status = audioSinkController->setAudioDecoder(audioDecoderId, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Tests the negative case for setting Audio Decoder in STARTED state
*
* This test checks that the setAudioDecoder function cannot be called when the AudioSink is in the STARTED state, enforcing the correct state transitions.
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
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Attempt to set audio decoder while in STARTED state. | audioDecoderId = UNDEFINED, shouldFail | status.exceptionCode() is EX_ILLEGAL_STATE | Should Fail |
* | 08 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 09 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 10 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_negative_2_setAudioDecoder)
{
    ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId;
    bool _aidl_return;
    // calling setAudioDecoder() from state STARTED
    audioDecoderId.value = ::com::rdk::hal::audiodecoder::IAudioDecoder::Id::UNDEFINED;
    ::android::binder::Status status = audioSinkController->setAudioDecoder(audioDecoderId, &_aidl_return);
    EXPECT_EQ(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_STATE);
}

/**
* @brief Test to verify the functionality of getAudioDecoder API from IAudioSinkController
*
* This test verifies that the getAudioDecoder function of the IAudioSinkController can be invoked successfully and returns the proper audio decoder ID when the audio sink is in the READY state.
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
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Call getAudioDecoder function from IAudioSinkController | N/A | status.isOk() should be true, audioDecoderId is valid | Should Pass |
* | 04 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/ 
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_positive_1_getAudioDecoder)
{
    ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId;
    ::android::binder::Status status = audioSinkController->getAudioDecoder(&audioDecoderId);
    ASSERT_TRUE(status.isOk());
}

/**
* @brief Test the retrieval of audio decoder ID from the audio sink controller
*
* This test checks if the audio sink controller can successfully retrieve the audio decoder ID 
* when the audio sink is in the started state. This is important to ensure the functional integrity 
* of audio stream management in the started state.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 034@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Call getAudioDecoder() | &audioDecoderId | Should retrieve correct audio decoder id | should pass |
* | 08 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 09 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 10 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_positive_2_getAudioDecoder)
{
    ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId;
    ::android::binder::Status status = audioSinkController->getAudioDecoder(&audioDecoderId);
    ASSERT_TRUE(status.isOk());
}

/**
* @brief Test for starting the audio sink successfully.
*
* This test validates the ability to start the audio sink, ensure it operates as expected and then stops it succesfully.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 035 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Start the audio sink | None | status.isOk() should be true | Should Pass |
* | 04 | Stop the audio sink to clean up | None | status.isOk() should be true | Should be successful |
* | 05 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_positive_1_start)
{
    Status status = audioSinkController->start();
    ASSERT_TRUE(status.isOk());
    //cleaning up
    status = audioSinkController->stop();
    ASSERT_TRUE(status.isOk());
}

/**
* @brief Test to verify if calling start() from state STARTED returns the correct exception code.
*
* This test checks whether the audio sink controller returns an illegal state exception when attempting to start the audio sink after it has already been started. This is important to ensure the integrity and expected flow of states in the audio sink controller.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 036 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                                             | Test Data                                | Expected Result                                                        | Notes                 |
* | :--------------: | ------------------------------------------------------- | --------------------------------------- | ---------------------------------------------------------------------- | --------------------- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Actual test - calling start() from state STARTED | None | Status.exceptionCode() should be EX_ILLEGAL_STATE | Should Fail|
* | 08 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 09 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 10 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_negative_1_start)
{
    //calling start() from state STARTED
    Status status = audioSinkController->start();
    ASSERT_EQ(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_STATE);
}

/**
* @brief Test the stopping mechanism of L1AudioSink when it is in a started state.
*
* This test case validates the ability to stop the audio sink once it is in a STARTED state.
* It ensures that the stop function works correctly without errors.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 037@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Stop the audio sink using the audioSinkController | None | status.isOk() should be true | Should Pass |
* | 08 | Check the state after stopping the audio sink | state | status.isOk() should be true, state is READY | Done by Pre-requisite TearDown function |
* | 09 | Close the audio sink in teardown | audioSinkController | status.isOk() should be true, _aidl_return is true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStopTest, test_l1_audiosink_positive_1_stop)
{
    Status status = audioSinkController->stop();
    ASSERT_TRUE(status.isOk());
}

/**
* @brief Test the negative scenario of stopping the audio sink when it is not in an appropriate state.
*
* This test aims to check the behavior of the audio sink when the stop method is called while the audio sink is not in a state where stopping is allowed. This ensures proper error handling and stability of the audio sink API.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 038 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None
* **Dependencies:** None
* **User Interaction:** None
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Attempt to stop the audio sink when it is not in the correct state to be stopped | N/A | exceptionCode() == ::android::binder::Status::EX_ILLEGAL_STATE | Should Fail |
* | 04 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_negative_1_stop)
{
    Status status = audioSinkController->stop();
    ASSERT_EQ(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_STATE);
}

/**
* @brief This test checks the functionality of queueing an audio frame with invalid arguments.
*
* This test validates that the `queueAudioFrame` method of the `audioSinkController` correctly handles illegal arguments and returns the appropriate exception code.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 039 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Populate FrameMetadata using helper function. | metadata with all pre-defined values | metadata should be populated with appropriate test values. | Should be successful |
* | 08 | Call queueAudioFrame with invalid bufferHandle | nsPresentationTime = 1000, bufferHandle = -2000, metadata | queueAudioFrame should handle illegal arguments and return EX_ILLEGAL_ARGUMENT. | Should Fail |
* | 09 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 10 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 11 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_negative_1_queueAudioFrame) 
{
    ::com::rdk::hal::audiodecoder::FrameMetadata metadata;
    helper_populateFrameMetadata(metadata);
    int64_t nsPresentationTime = 1000;
    int64_t bufferHandle = -2000;
    bool _aidl_return;
    Status status = audioSinkController->queueAudioFrame(nsPresentationTime, bufferHandle, metadata, &_aidl_return);
    ASSERT_EQ(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_ARGUMENT);
}

/**
* @brief This test case is intended to validate the flushing of the audio sink.
*
* The objective is to test the flush operation of the audio sink controller in a positive scenario.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 040 @n
* **Priority:** High @n
*  @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
*  @n
* **Test Procedure:**
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :--------------: | ----------- | --------- | --------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Flush the audio sink and verify if the operation is successful. | None | status.isOk() should be true | Should Pass |
* | 08 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 09 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 10 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_positive_1_flush)
{
    Status status = audioSinkController->flush();
    ASSERT_TRUE(status.isOk());
}

/**
* @brief Test L1 AudioSink functionality by calling flush() in an invalid state
*
* This test checks the behavior of the AudioSink when the flush()
* function is called in the READY state, which is expected to throw
* an illegal state exception.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 041@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Call flush() method on AudioSinkController in READY state |None | An exception with the code EX_ILLEGAL_STATE should be thrown | Should fail |
* | 04 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_negative_1_flush)
{
    //calling flush() from READY state (EX_ILLEGAL_STATE)
    Status status = audioSinkController->flush();
    ASSERT_EQ(status.exceptionCode(), Status::EX_ILLEGAL_STATE);
}

/**
* @brief Tests the audio sink controller's getVolume functionality when the audio sink is in the READY state.
*
* This test checks whether the getVolume method of the audio sink controller correctly retrieves the volume information when the state is READY. It ensures that the function returns an OK status and that the volume structure holds valid values.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 042@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Calling getVolume in state READY | None | status.isOk() should be true, volume: valid range | Should Pass |
* | 04 | Validating the volume structure returned by getVolume | volume structure | Volume within range and muted flag valid | Should be successful |
* | 05 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_positive_1_getVolume)
{
    Volume volume;
    //calling getVolume() in state READY
    Status status = audioSinkController->getVolume(&volume);
    ASSERT_TRUE(status.isOk());
    helper_validateVolume(volume);
}

/**
* @brief This test verifies the `getVolume` function's behavior when the audio sink is in the STARTED state.
*
* This test checks whether the `getVolume` function retrieves the volume correctly when called in the STARTED state. This ensures that the audio sink controller is correctly managing the audio sink's state transitions and volume settings.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 043@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Call `audioSinkController->getVolume(&volume)` to get the volume | volume = output variable | status.isOk() should be true | Should Pass |
* | 08 | Validate the retrieved volume using `helper_validateVolume(volume)` | volume = output from getVolume | Assert volume is within valid range and muted flag is boolean | Should Pass |
* | 09 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 10 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 11 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_positive_2_getVolume)
{
    Volume volume;
    //calling getVolume() in state STARTED
    Status status = audioSinkController->getVolume(&volume);
    ASSERT_TRUE(status.isOk());
    helper_validateVolume(volume);
}

/**
* @brief Test for setting the volume on the audio sink controller.
*
* This test ensures that the volume can be set correctly on the audio sink controller using the setVolume function. 
* It validates this by checking the return status and confirming that the volume is set successfully.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 044@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description                                                  | Test Data                                        | Expected Result                                  | Notes                      |
* | :--------------: | ------------------------------------------------------------ | ------------------------------------------------ | ----------------------------------------------- | -------------------------- |
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Prepare volume object using helper_setVolumeParcelable. | volume, volValue = 0.0, mute = false | | Should be successful|
* | 04 | Call setVolume on the audioSinkController.| volume = 0.0, mute = false, _aidl_return | status.isOk() and _aidl_return should be True | Should Pass |
* | 05 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_positive_1_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, 0.0, false);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
 * @brief Test the `setVolume` function of the audio sink controller for successful operation with valid volume.
 *
 * This test checks the `setVolume` API of the audio sink controller. It verifies that the volume setting 
 * operation can be performed successfully with a given valid input volume (1.0) and mute (false) values.
 *
 * **Test Group ID:** Basic: 01 @n
 * **Test Case ID:** 045 @n
 * **Priority:** High @n
 * @n
 * **Pre-Conditions:** None @n
 * **Dependencies:** None @n
 * **User Interaction:** None @n
 * @n
 * **Test Procedure:**@n
 * | Variation / Step | Description | Test Data | Expected Result | Notes |
 * | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Initialize Volume structure using `helper_setVolumeParcelable` | volume = {}, volValue = 1.0, mute = false | Volume structure should be initialized with the specified values | Should be successful |
* | 04 | Call `setVolume` on `audioSinkController` and check status | volume = { volume = 1.0, muted = false }, _aidl_return = true | status.isOk() should be true and _aidl_return should be true | Should Pass |
* | 04 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
 */
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_positive_2_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, 1.0, false);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief This test case checks if the volume can be set properly using the setVolume function.
*
* The primary objective is to validate if the setVolume function of the audioSinkController correctly sets the given volume value.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 046@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- | -------------- | ----- |
* | 01 | Check the initial state of audio sink and open it | state = CLOSED, Initial state | Status OK, audioSinkController not null, state = READY | Done by Pre-requisite SetUp function |
* | 02 | Prepare the volume parcelable using helper function | volValue = 0.3, mute = true | Correctly set the volume and mute status in Parcelable | Should be successful |
* | 03 | Set the volume using the setVolume function | volume = { volume=0.3, muted=true }, return = true | status.isOk() should be true, _aidl_return = true | Should Pass |
* | 04 | Close the audio sink | state = READY | Status OK, state = READY | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_positive_3_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, 0.3, true);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test case to set volume on the L1 audio sink.
*
* This test verifies the ability to set the volume level on the L1 audio sink and checks whether 
* the state changes appropriately after the volume is set. This is essential to ensure the volume 
* control functionality works as expected.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 047@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Helper function to set volume: set volume value and mute state | volValue = 0.3, mute = true | Volume object should be set with provided values | Should be successful |
* | 08 | Call `setVolume` on audioSinkController with the volume object | volume = { volume: 0.3, muted: true } | status.isOk() should be true , _aidl_return = true | Should Pass |
* | 09 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 10 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 11 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_positive_4_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, 0.3, true);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Test setVolume functionality for audio sink
*
* This test validates the setVolume functionality of the audio sink controller to ensure that the volume is set correctly and the sink is not muted.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 048@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Initialize Volume object using helper | Call helper_setVolumeParcelable to set volume and mute | volume=0.3, mute=false | Should be successful |
* | 08 | Set volume using audioSinkController | volume=0.3, mute=false | status.isOk() should be true and  _aidl_return=true | Should Pass |
* | 09 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 10 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 11 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_positive_5_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, 0.3, false);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_TRUE(_aidl_return);
}

/**
* @brief Negative test case for setting volume
*
* This test is intended to validate the error handling of the setVolume API when
* an invalid volume value is provided. Specifically, it tests the behavior when 
* a negative volume value is used.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 049@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 | SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 | Initialize Volume struct with invalid volume value and mute status | volume = Volume, volValue = -0.1, mute = false | Should be successful | |
* | 04 | Call setVolume API with initialized Volume struct | volume = Volume with volume = -0.1, muted = false | status.isOk() = true, _aidl_return = false | Should Fail |
* | 05 | TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_negative_1_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, -0.1, false);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Test setting the volume to an invalid value using the audio sink controller.
*
* This test verifies if the audio sink controller correctly handles an invalid volume value 
* and responds with an appropriate error status. It helps in ensuring the robustness of the
* volume setting functionality.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 050 @n
* **Priority:** High @n
* @n
* **Pre-Conditions:** None @n
* **Dependencies:** None @n
* **User Interaction:** None @n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 |SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 |SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 |SetVolume with invalid value using helper function | volValue = 1.1, mute = false | Should be successful | |
* | 04 |Try to set volume and verify return status | volume = {volume = 1.1, muted = false}, _aidl_return = false | status.isOk() should be true, _aidl_return should be false | Should Fail |
* | 05 |TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_negative_2_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, 1.1, false);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Testing the negative scenario for setting volume on the audio sink controller
*
* This test verifies that setting an invalid volume value (-0.2) on the audio sink controller fails as expected. 
* The invalid volume value should trigger an error, confirming robust error handling on the audio sink.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 051@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | helper_setVolumeParcelable: Set volume | volume: -0.2, mute: false | Volume object should have volume set to -0.2, muted set to false | Should be successful |
* | 08 | Test: Invoke setVolume with negative volume | volume: -0.2, mute: false, _aidl_return: false | status.isOk() should be true, _aidl_return should be false | Should Fail |
* | 09 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 10 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 11 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_negative_3_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, -0.2, false);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Test the negative case for setting the volume on the audio sink.
*
* This test attempts to set an invalid volume value (greater than the maximum allowable) 
* on the audio sink and verifies that the operation fails as expected.
*
* **Test Group ID:** Basic: 01 @n
* **Test Case ID:** 052@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Set invalid volume using helper function. | volume = {volume = 1.3, muted = true} | Should be successful | Should be successful |
* | 08 | Invoke setVolume API with invalid volume. | volume = {volume = 1.3, muted = true}, _aidl_return expected false | status.isOk() == true, _aidl_return == false | Should Fail |
* | 09 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 10 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 11 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_negative_4_setVolume)
{
    Volume volume;
    helper_setVolumeParcelable(volume, 1.3, true);
    bool _aidl_return;
    Status status = audioSinkController->setVolume(volume, &_aidl_return);
    ASSERT_TRUE(status.isOk());
    ASSERT_FALSE(_aidl_return);
}

/**
* @brief Test setting volume ramp on audio sink
*
* This test verifies that the audio sink properly sets the volume ramp with different types of ramp profiles. The test ensures that the setVolumeRamp function operates correctly for each type of volume ramp by verifying that the operation is successful and the function returns the expected boolean result.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 053@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Setting volume ramp to LINEAR and verifying result | targetVolume = 0.5, overMs = 1000, volumeRamp = VolumeRamp::LINEAR, &_aidl_return | status.isOk(), _aidl_return == true | Should Pass |
* | 08 | Setting volume ramp to IN_CUBIC and verifying result | targetVolume = 0.5, overMs = 1000, volumeRamp = VolumeRamp::IN_CUBIC, &_aidl_return | status.isOk(), _aidl_return == true | Should Pass |
* | 09 | Setting volume ramp to OUT_CUBIC and verifying result | targetVolume = 0.5, overMs = 1000, volumeRamp = VolumeRamp::OUT_CUBIC, &_aidl_return | status.isOk(), _aidl_return == true | Should Pass |
* | 10 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 11 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 12 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_positive_1_setVolumeRamp) {
    double targetVolume = 0.5;
    int32_t overMs = 1000;
    bool _aidl_return;
    for (auto volumeRampVal :
    {
        ::com::rdk::hal::audiosink::VolumeRamp::LINEAR,
        ::com::rdk::hal::audiosink::VolumeRamp::IN_CUBIC,
        ::com::rdk::hal::audiosink::VolumeRamp::OUT_CUBIC
    }) {
            VolumeRamp volumeRamp = volumeRampVal;
            ::android::binder::Status status = audioSinkController->setVolumeRamp(targetVolume, overMs, volumeRamp, &_aidl_return);
            EXPECT_TRUE(status.isOk());
            EXPECT_TRUE(_aidl_return);
    }
}

/**
* @brief Test for setting an invalid negative volume ramp value.
*
* This test verifies that setting a negative target volume throws an error and does not alter the current volume state.@n
* 
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 054@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Attempt to set a negative target volume (-0.1) for the volume ramp over 1000ms with LINEAR ramp type. | targetVolume = -0.1, overMs = 1000, volumeRamp = LINEAR, _aidl_return = false | status.isOk()  = true and _aidl_return should be false | Should Fail |
* | 08 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 09 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 10 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_negative_1_setVolumeRamp)
{
    double targetVolume = -0.1; //invalid value
    int32_t overMs = 1000;
    VolumeRamp volumeRamp = VolumeRamp::LINEAR;
    bool _aidl_return;
    ::android::binder::Status status = audioSinkController->setVolumeRamp(targetVolume, overMs, volumeRamp, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
}

/**
* @brief This test case checks the behavior of the setVolumeRamp function with an invalid overMs value.
*
* The function setVolumeRamp is tested with a negative value for the duration (overMs) to ensure the function handles such invalid input correctly.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 055@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
*
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Call setVolumeRamp with targetVolume=0.5, overMs=-1000, and volumeRamp=VolumeRamp::IN_CUBIC. | targetVolume=0.5, overMs=-1000, volumeRamp=VolumeRamp::IN_CUBIC, _aidl_return=false |status.isOk() should be true and _aidl_return should be false. | Should Fail |
* | 08 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 09 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 10 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_negative_2_setVolumeRamp)
{
    double targetVolume = 0.5;
    int32_t overMs = -1000; //invalid value
    VolumeRamp volumeRamp = VolumeRamp::IN_CUBIC;
    bool _aidl_return;
    ::android::binder::Status status = audioSinkController->setVolumeRamp(targetVolume, overMs, volumeRamp, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
}

/**
* @brief This test checks the behavior of the `setVolumeRamp` function when given an invalid volume ramp value.
*
* This test ensures that the `setVolumeRamp` function properly handles invalid input for the volume ramp value by passing a value outside the enum range. It verifies that the function call fails and does not mistakenly process the invalid input.
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 056@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data |Expected Result |Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 | Set up L1AudioSinkOpenTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 02 | Retrieve current state and verify it's CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite SetUp function |
* | 03 | Create listener and open AudioSink. | listener = new AudioSinkControllerListener() | status.isOk() is true, audioSinkController is initialized | Done by Pre-requisite SetUp function |
* | 04 | Retrieve current state and verify it's READY. | state | status.isOk() is true, state is READY | Done by Pre-requisite SetUp function |
* | 05 | Set up L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite SetUp function |
* | 06 | Start audio sink and check if the state changes to STARTED. | state | status.isOk() is true, state is STARTED | Done by Pre-requisite SetUp function |
* | 07 | Call setVolumeRamp with invalid volume ramp value | targetVolume = 0.5, overMs = 1000, volumeRamp = 5, _aidl_return = false | status.isOk() should be true, _aidl_return = false | Should Fail |
* | 08 | Tear down L1AudioSinkStartTest environment. | None | Should be successful | Done by Pre-requisite TearDown function |
* | 09 | Stop audio sink and check if the state changes back from STARTED. | state | status.isOk() is true, state is READY | Done by Pre-requisite TearDown function |
* | 10 | Close AudioSink and verify state transition to CLOSED. | state | status.isOk() is true, state is CLOSED | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkStartTest, test_l1_audiosink_negative_3_setVolumeRamp)
{
    double targetVolume = 0.5;
    int32_t overMs = 1000;
    VolumeRamp volumeRamp = static_cast<VolumeRamp>(5); // out of enum range
    bool _aidl_return;
    ::android::binder::Status status = audioSinkController->setVolumeRamp(targetVolume, overMs, volumeRamp, &_aidl_return);
    EXPECT_TRUE(status.isOk());
    EXPECT_FALSE(_aidl_return);
}

/**
* @brief Test for setting volume ramp in an invalid state
*n
* This test verifies that the setVolumeRamp function fails when called in an invalid state, specifically when the audio sink is not in the correct state to set volume ramp.@n
*
* **Test Group ID:** Basic: 01@n
* **Test Case ID:** 057@n
* **Priority:** High@n
* @n
* **Pre-Conditions:** None@n
* **Dependencies:** None@n
* **User Interaction:** None@n
* @n
* **Test Procedure:**@n
* | Variation / Step | Description | Test Data | Expected Result | Notes |
* | :----: | --------- | ---------- |-------------- | ----- |
* | 01 |SetUp L1AudioSinkOpenTest: Check initial state | None | Initial state should be CLOSED | Done by Pre-requisite Setup function |
* | 02 |SetUp L1AudioSinkOpenTest: Open audio sink | listener: non-null, audioSinkController: non-null | State should change to READY, audioSinkController should not be null |  Done by Pre-requisite Setup function |
* | 03 |Set volume ramp with targetVolume: 0.0, overMs: 1000, volumeRamp: LINEAR, and check return status and aidl_return | targetVolume: 0.0, overMs: 1000, volumeRamp: VolumeRamp::LINEAR, _aidl_return: false | Status is EX_ILLEGAL_STATE, _aidl_return is false | Should Fail |
* | 04 |TearDown L1AudioSinkOpenTest: Close audio sink | audioSinkController: non-null | status.isOk() should be true | Done by Pre-requisite TearDown function |
*/
TEST_F(L1AudioSinkOpenTest, test_l1_audiosink_negative_4_setVolumeRamp)
{
    double targetVolume = 0.0;
    int32_t overMs = 1000;
    VolumeRamp volumeRamp = VolumeRamp::LINEAR;
    bool _aidl_return;
    ::android::binder::Status status = audioSinkController->setVolumeRamp(targetVolume, overMs, volumeRamp, &_aidl_return);
    EXPECT_EQ(status.exceptionCode(), ::android::binder::Status::EX_ILLEGAL_STATE);
    EXPECT_FALSE(_aidl_return);
}

int main(int argc, char **argv) 
{
    std::string yaml_file;
    int opt;
    int result1;
    int result2;
    int number_of_audiosink;
    int number_of_audiodecoders;

    std::string audiosinkpath = "components.IAudioSinkManager.IAudioSink";
    std::string audiodecoderpath = "components.IAudioDecoderManager.IAudioDecoder";
    std::string supportsLowLatencypath = "components.IAudioSinkManager.platformCapabilities.0.supportsLowLatency";
    std::string systemMixerSampleRateHzpath = "components.IAudioSinkManager.platformCapabilities.0.systemMixerSampleRateHz";
    std::string systemMixerPCMFormatpath = "components.IAudioSinkManager.platformCapabilities.0.systemMixerPCMFormat";
    std::string supportsPlanarFormatpath = "components.IAudioSinkManager.platformCapabilities.0.supportsPlanarFormat";            

    ::testing::InitGoogleTest(&argc, argv);
    if(getService() == -1)
    {
        return -1;
    }

    while ((opt = getopt(argc, argv, "p:")) != -1) 
    {
        switch (opt) 
        {
            case 'p':
                yaml_file = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -p <yaml_file>" << std::endl;
                return 1;
        }
    }

    if (!yaml_file.empty()) 
    {
        try 
        {
            YAML::Node config = YAML::LoadFile(yaml_file.c_str());

            number_of_audiodecoders = get_list_Count_from_profile(config, audiodecoderpath);
            if(number_of_audiodecoders <= 0)
            {
                std::cout<< " No audiodecoders find in the profile" <<std::endl;
                return -1;
            }
            std::cout <<"audiodecoders : " <<number_of_audiodecoders <<std::endl;

            for(int i=0; i<number_of_audiodecoders; ++i)
            {
                std::string audiodecoderidpath = audiodecoderpath+"."+std::to_string(i) + ".id";
                int decoder_id = get_integer_value_from_profile(config,audiodecoderidpath);
                _audiodecoder_ids_from_profile.push_back(decoder_id);
            }

            number_of_audiosink = get_list_Count_from_profile(config, audiosinkpath);
            if(number_of_audiosink <= 0)
            {
                std::cout<< " No audiosink find in the profile" <<std::endl;
                return -1;
            }
            std::cout <<"audiosink count : " <<number_of_audiosink <<std::endl;

            for(int i=0; i<number_of_audiosink; ++i)
            {
                std::string audiosinkidpath = audiosinkpath+"."+std::to_string(i) + ".id";
                int id = get_integer_value_from_profile(config,audiosinkidpath);
                _audiosink_ids_from_profile.push_back(id);
            }

            supportsLowLatency_from_profile = get_boolean_value_from_profile(config,supportsLowLatencypath);
            systemMixerSampleRateHz_from_profile = get_integer_value_from_profile(config,systemMixerSampleRateHzpath);
            supportsPlanarFormat_from_profile = get_boolean_value_from_profile(config,supportsPlanarFormatpath);
            std::string systemMixerPCMFormat_string_value = get_string_value_from_profile(config,systemMixerPCMFormatpath);
            systemMixerPCMFormat_from_profile = StringToPCMformat(systemMixerPCMFormat_string_value);

            for(const auto& id: _audiosink_ids_from_profile)
            {
                g_audiosinkId.value = id;
                Status status = service->getAudioSink(g_audiosinkId, &_audioSink);
                ::testing::GTEST_FLAG(filter) = "L1AudioSinkTest.*:L1AudioSinkOpenTest.*:L1AudioSinkCloseTest.*:L1AudioSinkEventRegisterTest.*:L1AudioSinkStartTest.*:L1AudioSinkStopTest.*";
                result1 = RUN_ALL_TESTS();
            }

            ::testing::GTEST_FLAG(filter) = "L1AudioSinkManagerTest.*";
            result2 = RUN_ALL_TESTS();
            return (result1 || result2);
        }
        catch (const YAML::BadFile& e) 
        {
            std::cerr << "Error: Could not open YAML file. " << e.what() << std::endl;
            return -1;
        } 
        catch (const YAML::BadConversion& e) 
        {
            std::cerr << "Error: Bad type conversion in YAML. " << e.what() << std::endl;
            return -1;
        }
        catch (const std::runtime_error& e) 
        {
            std::cerr << "Runtime Error: " << e.what() << std::endl;
            return -1;
        } 
        catch (const std::exception& e) 
        {
            std::cerr << "Error: " << e.what() << std::endl;
            return -1;
        }
    }
    else
    {
        std::cout << "USAGE : ./AudioSinkClient -p <config_file>" <<std::endl;
    }

}

