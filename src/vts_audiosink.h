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
 * @defgroup AUDIO_SINK_HALTEST_HEADER Audio Sink HAL Tests helper functions Header File
 * @{
 * @parblock
 *
 * ### Common functions/macros required for the audio module :
 *
 * Common functions/macros required for the audio module .
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_audiosink.h
 *
 */

#ifndef __VTS_AUDIOSINK_H__
#define __VTS_AUDIOSINK_H__

#include <thread>
#include <chrono>

#include "com/rdk/hal/audiosink/IAudioSinkManager.h"
#include "com/rdk/hal/audiosink/BnAudioSinkControllerListener.h"
#include "com/rdk/hal/audiosink/BnAudioSinkEventListener.h"
#include "com/rdk/hal/audiodecoder/FrameMetadata.h"

using android::sp;
using android::String16;
using android::defaultServiceManager;
using android::binder::Status;

using namespace com::rdk::hal;
using namespace com::rdk::hal::audiosink;

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

const static std::multimap<Property, int32_t> validIntPropertyReady =
{
    {Property::REFERENCE_LEVEL, -31},
    {Property::MIXER_ID , 0},
    {Property::AV_SOURCE , int32_t(static_cast<int32_t>(AVSource::AUTO))},
    {Property::AV_SOURCE, int32_t(static_cast<int32_t>(AVSource::UNKNOWN))},
    {Property::AV_SOURCE, int32_t(static_cast<int32_t>(AVSource::IP))},
    {Property::AV_SOURCE, int32_t(static_cast<int32_t>(AVSource::TUNER))},
    {Property::AV_SOURCE, int32_t(static_cast<int32_t>(AVSource::SYSTEM))},
    {Property::AV_SOURCE, int32_t(static_cast<int32_t>(AVSource::HDMI_1))},
    {Property::DOLBY_ATMOS_LOCK, 0},
    {Property::DOLBY_ATMOS_LOCK, 1},
};

const static std::multimap<Property, int32_t> invalidIntPropertyReady =
{
    {Property::REFERENCE_LEVEL, -100},
    {Property::MIXER_ID, -2},
    {Property::AV_SOURCE , int32_t(static_cast<int32_t>(AVSource::AUTO))-1},
    {Property::AV_SOURCE , int32_t(static_cast<int32_t>(AVSource::COMPOSITE_5 ))+1},
    {Property::DOLBY_ATMOS_LOCK, -1},
};

const static std::multimap<Property, int32_t> setIntPropertyReadonly =
{
    {Property::RESOURCE_ID, 1},
    {Property::RENDER_LATENCY_NS , 2},
    {Property::QUEUE_DEPTH_NS , 3},
};

#define MAX_TRANSITORY_STATE_WAIT_MS  5
#define MAX_FINAL_STATE_WAIT_MS  500

#define AUDIOSINK_MANAGER_GETAUDIOSINKIDS(audioSinkIds) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getAudioSinkIds(OUT:sinkIds:[])"); \
        status = (gAudioSinkManager)->getAudioSinkIds(&audioSinkIds); \
        UT_LOG_INFO("Result getAudioSinksIds(status:[isOK:%s, exception:%s]", \
                                UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AUDIOSINK_GETPLATFORMCAPABILITIES(platformCapabilities) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("getPlatformCapabilities(OUT:platformCapabilities[])"); \
        status = (gAudioSinkManager)->getPlatformCapabilities(&platformCapabilities); \
        UT_LOG_INFO("Result getCapabilities(OUT:capabilities), status:[isOK:%s, exception:%s]", \
                                UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AUDIOSINK_MANAGER_GETAUDIOSINK(_sinkId, audioSink) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getAudioSink(IN:audioSinkId:[%d], OUT:audioSink[])", _sinkId.value); \
        status = (gAudioSinkManager)->getAudioSink(_sinkId, &audioSink); \
        UT_LOG_INFO("Result getAudioSink(IN:_sinkId:[%d], OUT:audioSink[0x%X]), status:[isOK:%s, exception:%s]", \
                                _sinkId.value, audioSink.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(audioSink.get() != nullptr); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AUDIOSINK_MANAGER_GETAUDIOSINK_FAILURE(_sinkId, audioSink, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling getAudioSink(IN:_sinkId:[%d], OUT:audioSink[])", _sinkId.value); \
        status = (gAudioSinkManager)->getAudioSink(_sinkId, &audioSink); \
        UT_LOG_INFO("Result getAudioSink(IN:_sinkId:[%d], OUT:audioSink[0x%X]), status:[isOK:%s, exception:%s]", \
                                _sinkId.value, audioSink.get(), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_TRUE(audioSink.get() == nullptr); \
    } while (0)

#define AUDIODSINK_GETCAPABILITIES(audioSink, capabilities) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getCapabilities(OUT:capabilities)"); \
        status = (audioSink)->getCapabilities(&capabilities); \
        UT_LOG_INFO("Result getCapabilities(OUT:capabilities), status:[isOK:%s, exception:%s]", \
                                UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_NOT_EQUAL(&capabilities, nullptr); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AUDIOSINK_GET_PROPERTY_SUCCESS(audioSink, property, propertyValue) \
    do { \
        ::android::binder::Status status; \
        ::com::rdk::hal::State state; \
        UT_LOG_INFO("Calling getProperty(IN:property[%s], OUT:propertyValue[]", audiosink::toString(property).c_str()); \
        status = (audioSink)->getProperty(property, &propertyValue); \
        switch (propertyValue->value.value().getTag()) \
        { \
            case PropertyValue::Value::Tag::intValue: \
                UT_LOG_INFO("Result getProperty(IN:property[%s], OUT:propertyValue[%d]), status:[isOK:%s, exception:%s]", audiosink::toString(property).c_str(), \
                propertyValue->value->get<PropertyValue::Value::Tag::intValue>(), \
                UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
                break; \
            case PropertyValue::Value::Tag::stringValue: \
                UT_LOG_INFO("Result getProperty(IN:property[%s], OUT:propertyValue[%s]), status:[isOK:%s, exception:%s]", audiosink::toString(property).c_str(), \
                propertyValue->value->get<PropertyValue::Value::Tag::stringValue>().string(), \
                UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
                break; \
        } \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AUDIOSINK_GET_PROPERTY_FAILURE(audioSink, property, propertyValue, exception) \
    do { \
        ::android::binder::Status status; \
        ::com::rdk::hal::State state; \
        UT_LOG_INFO("Calling getProperty(IN:property[%s], OUT:propertyValue[]", audiosink::toString(property).c_str()); \
        status = (audioSink)->getProperty(property, &propertyValue); \
        UT_LOG_INFO("Result getProperty(IN:property[%s], OUT:propertyValue[]), status:[isOK:%s, exception:%s]", audiosink::toString(property).c_str(), \
                UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
    } while (0)

#define AUDIOSINK_GET_STATE(audioSink, state) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getState(OUT:state[]"); \
        status = (audioSink)->getState(&state); \
        UT_LOG_INFO("Result getState(OUT:state[%s]), status:[isOK:%s, exception:%s]", com::rdk::hal::toString(state).c_str(), \
                UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AUDIOSINK_WAIT_FOR_STATE(audioSink, desiredState, lastState) \
    do { \
        int32_t sleep_time = 0; \
        do \
        { \
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_TRANSITORY_STATE_WAIT_MS)); \
            sleep_time += MAX_TRANSITORY_STATE_WAIT_MS; \
            /* Check the state of the sink */ \
            AUDIOSINK_GET_STATE((audioSink), lastState); \
        } while (sleep_time < MAX_FINAL_STATE_WAIT_MS && lastState != desiredState); \
    } while (0)

#define AUDIOSINK_OPEN_SUCCESS(audioSink, controllerListener, audioSinkController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling open(IN:controllerListener[0x%p], OUT:audioSinkController[])", (controllerListener).get()); \
        status = (audioSink)->open((controllerListener), &(audioSinkController)); \
        UT_LOG_INFO("Result open(IN:controllerListener[0x%p], OUT:audioSinkController[0x%X]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        \
        if (stateCheck) \
        { \
            ::com::rdk::hal::State lastState; \
            AUDIOSINK_WAIT_FOR_STATE(audioSink, State::READY, lastState); \
            UT_ASSERT_EQUAL(lastState, State::READY); \
        } \
    } while (0)

#define AUDIOSINK_OPEN_FAILURE(audioSink, controllerListener, audioSinkController, exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling open failure(IN:controllerListener[0x%p], OUT:audioDecoderController[])", \
                     controllerListener); \
        status = (audioSink)->open((controllerListener), &(audioSinkController)); \
        UT_LOG_INFO("Result open failure (IN:controllerListener[0x%p], OUT:audioSinkController[0x%X]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_EQUAL(audioSinkController.get(), nullptr); \
    } while (0)

#define AUDIOSINK_CLOSE_SUCCESS(audioSink, audioSinkController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling close(IN:audioSinkController[0x%p], OUT:_aidl_return[])", audioSinkController.get()); \
        status = audioSink->close(audioSinkController, &_aidl_return); \
        UT_LOG_INFO("Result close(IN:audioSinkController[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            audioSinkController.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        if (stateCheck) \
        { \
            ::com::rdk::hal::State lastState; \
            AUDIOSINK_WAIT_FOR_STATE(audioSink, State::CLOSED, lastState); \
            UT_ASSERT_EQUAL(lastState, State::CLOSED); \
        } \
    } while (0)

#define AUDIOSINK_CLOSE_FAILURE(audioSink, audioSinkController, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling close(OUT:_aidl_return[])"); \
        status = (audioSink)->close(audioSinkController, &_aidl_return); \
        UT_LOG_INFO("Result close(OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AUDIOSINK_REGISTER_EVENT_SUCCESS(audioSink, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = (audioSink)->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(_aidl_return); \
    } while (0)

#define AUDIOSINK_REGISTER_EVENT_FAILURE(audioSink, eventListener, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = (audioSink)->registerEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result registerEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        if (exception == Status::EX_NONE) \
            UT_ASSERT_TRUE(status.isOk()); \
        else \
            UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AUDIOSINK_UNREGISTER_EVENT_SUCCESS(audioSink, eventListener) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = (audioSink)->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AUDIOSINK_UNREGISTER_EVENT_FAILURE(audioSink, eventListener, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[])", eventListener.get()); \
        status = (audioSink)->unregisterEventListener(eventListener, &_aidl_return); \
        UT_LOG_INFO("Result unregisterEventListener(IN:eventListener[0x%p], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", \
            eventListener.get(), UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AUDIOSINK_START_SUCCESS(audioSink, audioSinkController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling start()"); \
        status = (audioSinkController)->start(); \
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        if (stateCheck) \
        { \
            ::com::rdk::hal::State lastState; \
            AUDIOSINK_WAIT_FOR_STATE(audioSink, State::STARTED, lastState); \
            UT_ASSERT_EQUAL(lastState, State::STARTED); \
        } \
    } while (0)

#define AUDIOSINK_START_FAILURE(audioSink, audioSinkController, exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling start()"); \
        status = (audioSinkController)->start(); \
        UT_LOG_INFO("Result start(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AUDIOSINK_STOP_SUCCESS(audioSink, audioSinkController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling stop()"); \
        status = (audioSinkController)->stop(); \
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        if (stateCheck) \
        { \
            ::com::rdk::hal::State lastState; \
            AUDIOSINK_WAIT_FOR_STATE(audioSink, State::READY, lastState); \
            UT_ASSERT_EQUAL(lastState, State::READY); \
        } \
    } while (0)

#define AUDIOSINK_STOP_FAILURE(audioSinkController, exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling stop()"); \
        status = (audioSinkController)->stop(); \
        UT_LOG_INFO("Result stop(), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AUDIOSINK_SETAUDIODECODER_SUCCESS(decoderId, audioSinkController) \
    do { \
        ::android::binder::Status status; \
        ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId; \
        audioDecoderId.value = decoderId; \
        bool aidl_return; \
        UT_LOG_INFO("Calling setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return[])", audioDecoderId.value); \
        status = (audioSinkController)->setAudioDecoder(audioDecoderId, &aidl_return); \
        UT_LOG_INFO("Result setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    audioDecoderId.value, aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(aidl_return); \
        UT_ASSERT_TRUE(status.isOk()); \
    } while (0)

#define AUDIOSINK_SETAUDIODECODER_FAILURE(decoderId, audioSinkController, exception) \
    do { \
        ::android::binder::Status status; \
        ::com::rdk::hal::audiodecoder::IAudioDecoder::Id audioDecoderId; \
        audioDecoderId.value = decoderId; \
        bool aidl_return = false; \
        UT_LOG_INFO("Calling setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return[])", audioDecoderId.value); \
        status = (audioSinkController)->setAudioDecoder(audioDecoderId, &aidl_return); \
        UT_LOG_INFO("Result setAudioDecoder(IN:audioDecoderId:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    audioDecoderId.value, aidl_return ? "true" : "false", \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AUDIOSINK_GETAUDIODECODER_SUCCESS(audioDecoderId, audioSinkController) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling setAudioDecoder(IN:audioDecoderId:[], OUT:_aidl_return[])"); \
        status = (audioSinkController)->getAudioDecoder(&audioDecoderId); \
        UT_LOG_INFO("Result setAudioDecoder(IN:audioDecoderId:[%d], status:[isOK:%s, exception:%s]", \
                    audioDecoderId.value, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
    } while (0)

#define AUDIOSINK_GETAUDIODECODER_FAILURE(audioDecoderId, audioSinkController, exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling setAudioDecoder(OUT:audioDecoderId:[]"); \
        status = (audioSinkController)->getAudioDecoder(&audiodecoderId);\
        UT_LOG_INFO("Result setAudioDecoder(OUT:audioDecoderId:[%d]), status:[isOK:%s, exception:%s]", \
                    audioDecoderId.value, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AUDIOSINK_QUEUEAUDIOFRAME_FAILURE(audioSinkcontroller, nsPresentationTime, bufferHandle, metadata, exception) \
    do { \
        ::android::binder::Status status; \
        bool aidl_return; \
        UT_LOG_INFO("Calling queueAudioFrame(IN:nsPresentationTime:[%lld], IN:bufferHandle:[%lld], IN:metadata:[sourceCodec:%d, isDolbyAtmos:%d, numChannels:%d, sampleRate:%d, format:%d, source:%d])", \
                    nsPresentationTime, bufferHandle, static_cast<int>(metadata.sourceCodec), metadata.isDolbyAtmos, metadata.numChannels, metadata.sampleRate, static_cast<int>(metadata.format), static_cast<int>(metadata.source)); \
        status = (audioSinkController)->queueAudioFrame(nsPresentationTime, bufferHandle, metadata, &aidl_return); \
        UT_LOG_INFO("Result queueAudioFrame(IN:nsPresentationTime:[%lld], IN:bufferHandle:[%lld], IN:metadata:[sourceCodec:%d, isDolbyAtmos:%d, numChannels:%d, sampleRate:%d, format:%d, source:%d]), status:[isOK:%s, exception:%s]", \
                    nsPresentationTime, bufferHandle, static_cast<int>(metadata.sourceCodec), metadata.isDolbyAtmos, metadata.numChannels, metadata.sampleRate, static_cast<int>(metadata.format), static_cast<int>(metadata.source), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(status.isOk()); \
        } while(0)

#define AUDIOSINK_FLUSH_SUCCESS(audioSink, audioSinkController, stateCheck) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling flush"); \
        status = (audioSinkController)->flush(); \
        UT_LOG_INFO("Result flush(status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        if (stateCheck) \
        { \
            std::this_thread::sleep_for(std::chrono::milliseconds(MAX_FINAL_STATE_WAIT_MS)); \
            ::com::rdk::hal::State lastState; \
            AUDIOSINK_WAIT_FOR_STATE(audioSink, State::STARTED, lastState); \
            UT_ASSERT_EQUAL(lastState, State::STARTED); \
        } \
    } while (0)

#define AUDIOSINK_FLUSH_FAILURE(audioDecoderController, exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling flush"); \
        status = (audioSinkController)->flush(); \
        UT_LOG_INFO("Result flush(status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AUDIOSINK_SETPROPERTY_SUCCESS(audioSink, property, propertyValue) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setProperty(IN:property[%s], IN:propertyValue[0x%X], OUT:_aidl_return[])", com::rdk::hal::audiosink::toString(property).c_str(), \
                      propertyValue.value); \
        status = (audioSink)->setProperty(property, propertyValue, &_aidl_return); \
        UT_LOG_INFO("Result setProperty(IN:property[%s], IN:propertyValue[0x%X], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", com::rdk::hal::audiosink::toString(property).c_str(), propertyValue.value, \
            UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AUDIOSINK_SETPROPERTY_FAILURE(audioSink, property, propertyValue, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling setProperty(IN:property[%s], IN:propertyValue[0x%X], OUT:_aidl_return[])", com::rdk::hal::audiosink::toString(property).c_str(), propertyValue.value); \
        status = (audioSink)->setProperty(property, propertyValue, &_aidl_return); \
        UT_LOG_INFO("Result setProperty(IN:property[%s], IN:propertyValue[0x%X], OUT:_aidl_return[%s]), status:[isOK:%s, exception:%s]", com::rdk::hal::audiosink::toString(property).c_str(), propertyValue.value, \
            UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AUDIOSINK_SETVOLUME_SUCCESS(audioSinkController, volumeSet) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setVolume(IN:volume:[%f], IN:muted:[%s])", volumeSet.volume, UT_Control_GetMapString(bool_mapTable, volumeSet.muted)); \
        status = (audioSinkController)->setVolume(volumeSet, &(_aidl_return)); \
        UT_LOG_INFO("Result setVolume(IN:volume:[%f], IN:muted:[%s], OUT:volumeSetResult:[%s]), status:[isOK:%s, exception:%s]", volumeSet.volume, \
            UT_Control_GetMapString(bool_mapTable, volumeSet.muted), \
            UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_TRUE(status.isOk()); \
    } while (0)

#define AUDIOSINK_SETVOLUME_FAILURE(audioSinkController, volumeSet, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setVolume(IN:volume:[%f], IN:muted:[%s])", volumeSet.volume, UT_Control_GetMapString(bool_mapTable, volumeSet.muted)); \
        status = (audioSinkController)->setVolume(volumeSet, &(_aidl_return)); \
        UT_LOG_INFO("Result setVolume(IN:volume:[%f], IN:muted:[%s], OUT:volumeSetResult:[%s]), status:[isOK:%s, exception:%s]", volumeSet.volume, \
            UT_Control_GetMapString(bool_mapTable, volumeSet.muted), \
            UT_Control_GetMapString(bool_mapTable, _aidl_return), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return);  \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AUDIOSINK_GETVOLUME_SUCCESS(audioSinkController, volumeGet) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling getVolume(OUT:volumeGet[])"); \
        status = (audioSinkController)->getVolume(&volumeGet); \
        UT_LOG_INFO("Result getVolume(OUT:volumeGet[volume:%f, muted:%s]), status:[isOK:%s, exception:%s]", \
            volumeGet.volume,UT_Control_GetMapString(bool_mapTable, volumeGet.muted), \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(status.isOk()); \
    } while (0)

#define AUDIOSINK_GETVOLUME_FAILURE(audioSinkController, volumeGet, exception) \
    do { \
        ::android::binder::Status status; \
        UT_LOG_INFO("Calling getVolume(OUT:volumeGet[])"); \
        status = (audioSinkController)->getVolume(&volumeGet); \
        UT_LOG_INFO("Result getVolume(OUT:status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode()));\
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(status.isOk()); \
    } while (0)

#define AUDIOSINK_SETVOLUMERAMP_SUCCESS(audioSinkController, targetVolume, overMs, volumeRamp) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:aidlReturn)", targetVolume, \
            overMs, static_cast<int32_t>(volumeRamp)); \
        status = (audioSinkController)->setVolumeRamp(targetVolume, overMs, volumeRamp, &_aidl_return); \
        UT_LOG_INFO("Result setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:aidlReturn:[%s]), status:[isOK:%s, exception:%s]", targetVolume, \
            overMs, static_cast<int32_t>(volumeRamp), _aidl_return ? "true" : "false", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
    }while (0)

#define AUDIOSINK_SETVOLUMERAMP_FAILURE(audioSinkController, targetVolume, overMs, volumeRamp, exception) \
    do { \
        ::android::binder::Status status; \
        bool _aidl_return; \
        UT_LOG_INFO("Calling setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:aidlReturn)", targetVolume, \
            overMs, static_cast<int32_t>(volumeRamp)); \
        status = (audioSinkController)->setVolumeRamp(targetVolume, overMs, volumeRamp, &_aidl_return); \
        UT_LOG_INFO("Result setVolumeRamp(IN:targetVolume:[%f], IN:overMs:[%d], IN:volumeRamp:[%d], OUT:aidlReturn:[%s]), status:[isOK:%s, exception:%s]", targetVolume, \
            overMs, static_cast<int32_t>(volumeRamp), _aidl_return ? "true" : "false", \
            UT_Control_GetMapString(bool_mapTable, status.isOk()), \
            UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return);  \
        UT_ASSERT_FALSE(status.isOk()); \
    }while (0)

#define AUDIOSINK_SETPROPERTY_LIST_SUCCESS(audioSink, properyList, type) \
    do { \
        for (const auto& [property, value] : properyList) \
        { \
            PropertyValue propertyValue; \
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::type>(value); \
            AUDIOSINK_SETPROPERTY_SUCCESS(audioSink, property, propertyValue); \
        } \
    } while (0)

#define AUDIOSINK_SETPROPERTY_LIST_FAILURE(audioSink, properyList, type, exception) \
    do { \
        for (const auto& [property, value] : properyList) \
        { \
            PropertyValue propertyValue; \
            propertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::type>(value); \
            AUDIOSINK_SETPROPERTY_FAILURE(audioSink, property, propertyValue, exception); \
        } \
    } while (0)

class AudioSinkControllerListener: public BnAudioSinkControllerListener
{
    public:
        AudioSinkControllerListener() = default;
        virtual ~AudioSinkControllerListener() = default;

        Status onStateChanged(State oldState,State newState) override
        {
            UT_LOG("\n=======State Change received: %d, %d=============\n", oldState, newState);
            {
                std::lock_guard<std::mutex> lock(mMutex);
                mStateChanges.emplace_back(oldState, newState);
                mCurrentState = newState;
            }
            mCV.notify_all(); // Unblock any waiting test code
            return Status::ok();
        }

        Status onFirstFrameRendered(int64_t nsPresentationTime) override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            firstframerendered.push_back(nsPresentationTime);
            return Status::ok();
        }

        Status onEndOfStream(int64_t nsPresentationTime) override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            endofstream.push_back(nsPresentationTime);
            return Status::ok();
        }

        Status onAudioUnderflow() override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return Status::ok();
        }

        Status onAudioResumed(int64_t nsPresentationTime) override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            audioresumed.push_back(nsPresentationTime);
            return Status::ok();
        }

        Status onFlushComplete() override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return Status::ok();
        }
    private:
        mutable std::mutex mMutex;
        std::condition_variable mCV;

        State mCurrentState;
        std::vector<std::pair<State, State>> mStateChanges;
        std::vector<int64_t> firstframerendered;
        std::vector<int64_t> endofstream;
        std::vector<int64_t> audioresumed;
};

class AudioSinkEventListener: public BnAudioSinkEventListener
{
    public:
        AudioSinkEventListener(): mCurrentState(State::CLOSED) {}
        virtual ~AudioSinkEventListener() = default;

        Status onStateChanged(State oldState,State newState) override
        {
            UT_LOG("\n=======State Change received: %d, %d=============\n", oldState, newState);
            {
                std::lock_guard<std::mutex> lock(mMutex);
                mStateChanges.emplace_back(oldState, newState);
                mCurrentState = newState;
            }
            mCV.notify_all(); // Unblock any waiting test code
            return Status::ok();
        }

        Status onFirstFrameRendered(int64_t nsPresentationTime) override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            firstframerendered.push_back(nsPresentationTime);
            return Status::ok();
        }

        Status onEndOfStream(int64_t nsPresentationTime) override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            endofstream.push_back(nsPresentationTime);
            return Status::ok();
        }

        Status onAudioUnderflow() override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return Status::ok();
        }

        Status onAudioResumed(int64_t nsPresentationTime) override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            audioresumed.push_back(nsPresentationTime);
            return Status::ok();
        }

        Status onFlushComplete() override
        {
            std::lock_guard<std::mutex> lock(mMutex);
            return Status::ok();
        }
    private:
        mutable std::mutex mMutex;
        std::condition_variable mCV;

        State mCurrentState;
        std::vector<std::pair<State, State>> mStateChanges;
        std::vector<int64_t> firstframerendered;
        std::vector<int64_t> endofstream;
        std::vector<int64_t> audioresumed;
};
#endif //__VTS_AUDIOSINK_H__

/** @} */ // End of AUDIO_SINK_HALTEST_HEADER
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK
/** @} */ // End of HPK
