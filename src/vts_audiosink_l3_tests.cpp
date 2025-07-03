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
 * @addtogroup AUDIO_SINK Audio Sink Module
 * @{
 *
 */

/**
 * @defgroup AUDIO_SINK_HALTEST Audio Sink HAL Tests
 * @{
 *
 */

/**
 * @defgroup AUDIO_SINK_HALTEST_L3 Audio Sink HAL Tests L3 File
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

#include "com/rdk/hal/avbuffer/IAVBuffer.h"
#include "com/rdk/hal/avbuffer/BnAVBufferSpaceListener.h"

#include "vts_audiosink_helper_functions.h"
#include "vts_audiosink_parse_hfp.h"
#include "vts_audiosink.h"
#include "vts_avbuffer.h"
#include "vts_audiosink_parse_testconfig.h"

#include "IAVBufferHelper.h"

static const char *gModuleName = "IAudioSinkManager";
static const char *gTestGroup = "L3";
static IAVBufferHelper* gAVbufferHelper;

sp<IAVBuffer> vts_avbuffer_getService()
{
    return
        android::interface_cast<IAVBuffer>(
            android::defaultServiceManager()->getService(
                android::String16(IAVBuffer::serviceName().c_str())
            )
        );
}

#define AVBUFFER_CREATE_AUDIOPOOL_SUCCESS(avBufferManager, secureHeap, audioSinkId, listener, audioPool) \
    do { \
        UT_LOG_INFO("Calling createAudioPool(IN:secureHeap:[%d], IN:audioSinkId:[%d], IN:listener:[%p], OUT:audioPool)", \
                    secureHeap, audioSinkId.value, listener.get()); \
        Status status = avBufferManager->createAudioPool(secureHeap, audioSinkId, listener, &audioPool); \
        UT_LOG_INFO("Result createAudioPool(IN:secureHeap:[%d], IN:audioSinkId:[%d], IN:listener:[%p], OUT:audioPool.handle:[%d]), status:[isOK:%s, exception:%s]", \
                    secureHeap, audioSinkId.value, listener.get(), audioPool.handle, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_NOT_EQUAL(audioPool.handle, Pool::INVALID); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_DESTROY_POOL_SUCCESS(avBufferManager, pool) \
    do { \
        bool _aidl_return = false; \
        Status status; \
        UT_LOG_INFO("Calling destroyPool(IN:poolHandle:[%d], OUT:_aidl_return:[])", pool); \
        status = avBufferManager->destroyPool(pool, &_aidl_return); \
        UT_LOG_INFO("Result destroyPool with (IN:poolHandle:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    pool.handle, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_ALLOC_SUCCESS(avBufferManager, pool, size, bufferHandle) \
    do { \
        UT_LOG_INFO("Calling alloc(IN:poolHandle:[%d], IN:size:[%d], OUT:bufferHandle)", pool.handle, size); \
        Status status = avBufferManager->alloc(pool, size, &bufferHandle); \
        UT_LOG_INFO("Result alloc(IN:poolHandle:[%d], IN:size:[%d], OUT:bufferHandle:[0x%X]), status:[isOK:%s, exception:%s]", \
                    pool.handle, size, bufferHandle, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_NOT_EQUAL(bufferHandle, IAVBuffer::INVALID_HANDLE); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_FREE_SUCCESS(avBufferManager, bufferHandle) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_INFO("Calling free(IN:bufferHandle:[0x%X], OUT:_aidl_return)", bufferHandle); \
        Status status = avBufferManager->free(bufferHandle, &_aidl_return); \
        UT_LOG_INFO("Result free(IN:bufferHandle:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    bufferHandle, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(_aidl_return); \
    } while (0)

#define AVBUFFER_GETMETRICS(avBufferManager, secureHeap, heapMetrics) \
    do { \
        UT_LOG_INFO("Calling getHeapMetrics(IN:secureHeap:[%s], OUT:heapMetrics[])", \
                    UT_Control_GetMapString(bool_mapTable, secureHeap)); \
        Status status = avBufferManager->getHeapMetrics(secureHeap, &heapMetrics); \
        UT_LOG_INFO("Result getHeapMetrics(IN:secureHeap:[%s], OUT:heapMetrics[secure:%s, bytesUsed:%d, bytesTotal:%d]), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, secureHeap), \
                    UT_Control_GetMapString(bool_mapTable, heapMetrics.secure), \
                    heapMetrics.bytesUsed, \
                    heapMetrics.bytesTotal, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

/* Test class registration with UT_Core */
UT_ADD_TEST_TO_GROUP(VTS_L3_AudioSink, UT_TESTS_L3);

extern sp<IAudioSinkManager> gAudioSinkManager;

class AVBufferSpaceListener : public BnAVBufferSpaceListener
{
public:

    AVBufferSpaceListener() {};
    virtual ~AVBufferSpaceListener() = default;

    Status onSpaceAvailable() override
    {
        return Status::ok();
    }
};

class SinkTest
{
public:
    SinkTest(IAudioSink::Id sinkId, sp<IAudioSink> sinkHandle,
             sp<AudioSinkControllerListener> controlListener,
             sp<AudioSinkEventListener> eventListener,
             sp<IAVBuffer> AVBufferManager, IAudioDecoder::Id decoderId,
             bool secureDecode, bool isTunnel, testConfiguration_t config)
    {
        mSinkId = sinkId;
        mDecoderId = decoderId;
        mSinkHandle = sinkHandle;
        mControlListener = controlListener;
        mSecureBuffer = secureDecode;
        mEventListener = eventListener;
        mTestConfiguration = config;
        mAVBufferManager = AVBufferManager;
        mIsTunnel = isTunnel;
    }

    void run()
    {
        if(mIsTunnel)
        {
            audioSinkTunnelTest();
        }
        else
        {
            audioSinkNonTunnelTest();
        }
    }

private:
    IAudioSink::Id mSinkId;
    IAudioDecoder::Id mDecoderId;
    sp<IAudioSink> mSinkHandle;
    sp<AudioSinkControllerListener> mControlListener;
    sp<AudioSinkEventListener> mEventListener;
    sp<IAVBuffer> mAVBufferManager;
    bool mSecureBuffer;
    bool mIsTunnel;
    testConfiguration_t mTestConfiguration;

    void audioSinkNonTunnelTest()
    {
        try {
            sp<IAudioSinkController> audioSinkController;
            int64_t audioBufferHandle;
            Pool audioPool;
            PropertyValue avSourcePropertyValue;
            sp<IAVBufferSpaceListener> bufferListener = new AVBufferSpaceListener();
            audiodecoder::FrameMetadata metadata;

            avSourcePropertyValue.value = PropertyValue::Value::make<PropertyValue::Value::Tag::intValue>(int32_t(static_cast<int32_t>(AVSource::IP)));

            UT_LOG_INFO("[SinkID: %d] Start of Testing Stream: %s", mSinkId.value, mTestConfiguration.input.c_str());

            metadata.sourceCodec = mTestConfiguration.codec;
            metadata.isDolbyAtmos = false;
            metadata.numChannels = mTestConfiguration.numChannels;
            metadata.channelTypes = mTestConfiguration.channelTypes;
            metadata.sampleRate = mTestConfiguration.sampleRate;
            metadata.format = mTestConfiguration.format;
            metadata.planarFormat = false;
            metadata.trimStartNs = 0;
            metadata.trimEndNs = 0;
            metadata.lowLatency = false;
            metadata.endOfStream = false;
            metadata.discontinuity = false;
            metadata.source = AVSource::SYSTEM;

            /* Create Audio Pool */
            AVBUFFER_CREATE_AUDIOPOOL_SUCCESS(mAVBufferManager, mSecureBuffer, mDecoderId, bufferListener, audioPool);

            /* Open the Decoder */
            AUDIOSINK_OPEN_SUCCESS(mSinkHandle, mControlListener, audioSinkController, true);

            AUDIOSINK_SETAUDIODECODER_SUCCESS(mDecoderId.value, audioSinkController);

            /* Start the decoder */
            AUDIOSINK_START_SUCCESS(mSinkHandle, audioSinkController, true);

            UT_LOG_INFO("[BHANU] pts size: %zu, value: %lld, framesize: %d", mTestConfiguration.pts.size(), mTestConfiguration.pts[0], mTestConfiguration.frameSizeInBytes);
            for(const auto& pts: mTestConfiguration.pts)
            {
                /* Allocate the audio buffer */
                AVBUFFER_ALLOC_SUCCESS(mAVBufferManager, audioPool, mTestConfiguration.frameSizeInBytes, audioBufferHandle);

                AUDIOSINK_QUEUEAUDIOFRAME_SUCCESS(audioSinkController, pts, audioBufferHandle, metadata);
            }

            /* Stop the Audio sink */
            AUDIOSINK_STOP_SUCCESS(mSinkHandle, audioSinkController, true);

            /* Destroy audio pool */
            AVBUFFER_DESTROY_POOL_SUCCESS(mAVBufferManager, audioPool);

        } catch (const std::exception& ex) {
            UT_LOG_ERROR("[SinkID: %d] Exception in decodeAndConsumeAudioStreams: %s", mSinkId.value, ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("[SinkID: %d] Unknown exception in decodeAndConsumeAudioStreams", mSinkId.value);
            UT_ASSERT_TRUE(false);
        }
    }

    void audioSinkTunnelTest()
    {
        //TODO: implement the tunnel test
    }
};

class VTS_L3_AudioSink : public UTCore
{
public:
VTS_L3_AudioSink() : UTCore() {}

~VTS_L3_AudioSink() override = default;

public:
    std::vector<std::tuple<IAudioSink::Id, sp<IAudioSink>, sp<AudioSinkControllerListener>, sp<AudioSinkEventListener>>> mAudioSinkList;
    sp<IAVBuffer> mAVBufferManager;

    void SetUp() override
    {
        std::vector<IAudioSink::Id> sinkIds;

        mAVBufferManager = vts_avbuffer_getService();
        gTestConfiguration = vts_kvp_create_instance(VTS_TEST_CONFIG_PATH);

        /* Get the Audio Sink IDs */
        AUDIOSINK_MANAGER_GETAUDIOSINKIDS(sinkIds);

        android::ProcessState::self()->startThreadPool();

        for(const auto& _sinkId: sinkIds)
        {
            sp<IAudioSink> audioSink;

            sp<AudioSinkEventListener> eventListener = new AudioSinkEventListener(_sinkId.value);

            sp<AudioSinkControllerListener> controlListener = new AudioSinkControllerListener(_sinkId.value);

            /* Get Audio decoder handle */
            AUDIOSINK_MANAGER_GETAUDIOSINK(_sinkId, audioSink);

            /* Register events */
            AUDIOSINK_REGISTER_EVENT_SUCCESS(audioSink, eventListener);

            /* Get Capabilities of Audio Sink */
            //TODO: Uncomment when capabilities are implemented in the HAL
            //AUDIOSINK_GETCAPABILITIES(audioSink, capabilities);

            mAudioSinkList.push_back({_sinkId, audioSink, controlListener, eventListener});
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

    bool isSinkUsable(const IAudioSink::Id& sinkId)
    {
        for (const auto& item : gAudiosinks_hfp) {
            if (std::find(item.availableIds.begin(), item.availableIds.end(), sinkId.value) != item.availableIds.end()) {
                return true;
            }
        }
        return false;
    }

    void runSequentialSink(testConfiguration_t& testConfig, bool secureDecode, bool isTunnel)
    {
        for (const auto& [_sinkId, sinkHandle, controlListener, eventListener] : mAudioSinkList)
        {
            HeapMetrics beforeHeapMetrics, afterHeapMetrics;

            if(!isSinkUsable(_sinkId))
            {
                UT_LOG_INFO("Sink ID %d is not usable for this test configuration, skipping.", _sinkId.value);
                continue;
            }

            /* Get heap metrics */
            AVBUFFER_GETMETRICS(mAVBufferManager, secureDecode, beforeHeapMetrics);

            SinkTest sinkTest(_sinkId, sinkHandle, controlListener, eventListener,
                              mAVBufferManager, gAudiosinks_hfp[0].audioDecoderIds[0], secureDecode, isTunnel, testConfig);

            sinkTest.run();

            /* Get heap metrics */
            AVBUFFER_GETMETRICS(mAVBufferManager, secureDecode, afterHeapMetrics);

            /* check if the decoder freed all the buffer pools */
            UT_ASSERT_TRUE(beforeHeapMetrics.secure == afterHeapMetrics.secure);
            UT_ASSERT_TRUE(beforeHeapMetrics.bytesUsed == afterHeapMetrics.bytesUsed);
            UT_ASSERT_TRUE(beforeHeapMetrics.bytesTotal == afterHeapMetrics.bytesTotal);

            /* TODO: check if this required or not */
            //controlListener->shutdown();
        }
    }

    void runConcurrentSink(testConfiguration_t& testConfig, bool secureDecode, bool isTunnel)
    {
        try {
            vector<thread> threads;
            vector<SinkTest> decoderTestList;
            HeapMetrics beforeHeapMetrics, afterHeapMetrics;

            //TODO: add check of decoder metrics
            /* Get heap metrics */
            AVBUFFER_GETMETRICS(mAVBufferManager, secureDecode, beforeHeapMetrics);

            std::vector<std::unique_ptr<SinkTest>> decoderTests;
            for (const auto& [_sinkId, sinkHandle, controlListener, eventListener] : mAudioSinkList)
            {
                auto decoderTest = std::make_unique<SinkTest>(_sinkId, sinkHandle,
                                                                controlListener, eventListener,
                                                                mAVBufferManager, gAudiosinks_hfp[0].audioDecoderIds[0], secureDecode, isTunnel,
                                                                testConfig);
                threads.emplace_back(&SinkTest::run, decoderTest.get());

                decoderTests.push_back(std::move(decoderTest));
            }
            for (auto& t : threads)
            {
                if (t.joinable())
                    t.join();
            }
            /* Get heap metrics */
            AVBUFFER_GETMETRICS(mAVBufferManager, secureDecode, afterHeapMetrics);

            /* check if the decoder freed all the buffer pools */
            UT_ASSERT_TRUE(beforeHeapMetrics.secure == afterHeapMetrics.secure);
            UT_ASSERT_TRUE(beforeHeapMetrics.bytesUsed == afterHeapMetrics.bytesUsed);
            UT_ASSERT_TRUE(beforeHeapMetrics.bytesTotal == afterHeapMetrics.bytesTotal);

        } catch (const std::exception& ex) {
            UT_LOG_ERROR("Exception in runConcurrentDecode: %s", ex.what());
            UT_ASSERT_TRUE(false);
        } catch (...) {
            UT_LOG_ERROR("Unknown exception in runConcurrentDecode");
            UT_ASSERT_TRUE(false);
        }
    }

    void runSingleProcessTest(const char *testName, bool isSecureMode, bool isTunnel, bool isConcurrentMode)
    {
        vector<testConfiguration_t> testConfiguration;

        vts_audiosink_parse_testconfig(gTestConfiguration, gModuleName, gTestGroup, testName, testConfiguration);

        if(isConcurrentMode)
        {
            for(auto& test: testConfiguration)
            {
                runConcurrentSink(test, isSecureMode, isTunnel);
            }
        }
        else
        {
            for(auto& test: testConfiguration)
            {

                runSequentialSink(test, isSecureMode, isTunnel);

            }
        }
    }

    void TearDown() override
    {
        for (const auto& [_sinkId, sinkHandle, controlListener, eventListener] : mAudioSinkList)
        {
            /* Un-register the events */
            AUDIOSINK_UNREGISTER_EVENT_SUCCESS(sinkHandle, eventListener);
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
UT_ADD_TEST(VTS_L3_AudioSink, Sink_Sequential_NonTunnel_NonSecure)
{
    const char *testName = "QueueFrames";
    bool isSecureMode = false;
    bool isConcurrentMode = false;
    bool isTunnel = false;

    runSingleProcessTest(testName, isSecureMode, isTunnel, isConcurrentMode);
}

/** @} */ // End of AUDIO_SINK_HALTEST L3 File
/** @} */ // End of AUDIO_SINK_HALTEST
/** @} */ // End of AUDIO_SINK Module
/** @} */ // End of HPK