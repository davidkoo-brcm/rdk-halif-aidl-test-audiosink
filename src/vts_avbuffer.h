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
 * @addtogroup AV_BUFFER AV Buffer Module
 * @{
 */

/**
 * @addtogroup AV_BUFFER_HALTEST AV Buffer HAL Tests
 * @{
 */

/**
 * @defgroup AV_BUFFER_HALTEST_HEADER AV Buffer HAL Tests helper functions Header File
 * @{
 * @parblock
 *
 * ### Common functions/macros required for the AV Buffer module :
 *
 * Common functions/macros required for the AV Buffer module .
 *
 * **Pre-Conditions:**  None @n
 * **Dependencies:** None @n
 *
 * @endparblock
 *
 */

/**
 * @file vts_avbuffer.h
 *
 */

#ifndef __VTS_AVBUFFER_H__
#define __VTS_AVBUFFER_H__

#include "com/rdk/hal/avbuffer/IAVBuffer.h"
#include "com/rdk/hal/avbuffer/BnAVBufferSpaceListener.h"

using namespace com::rdk::hal;
using namespace com::rdk::hal::avbuffer;

#ifdef DISABLE_UT_DEBUG_LOGGING
#define UT_LOG_DEBUG(...)
#endif

#define AVBUFFER_GETMETRICS(avBufferManager, secureHeap, heapMetrics) \
    do { \
        UT_LOG_DEBUG("Calling getHeapMetrics(IN:secureHeap:[%s], OUT:heapMetrics[])", \
                    UT_Control_GetMapString(bool_mapTable, secureHeap)); \
        Status status = avBufferManager->getHeapMetrics(secureHeap, &heapMetrics); \
        UT_LOG_DEBUG("Result getHeapMetrics(IN:secureHeap:[%s], OUT:heapMetrics[secure:%s, bytesUsed:%d, bytesTotal:%d]), status:[isOK:%s, exception:%s]", \
                    UT_Control_GetMapString(bool_mapTable, secureHeap), \
                    UT_Control_GetMapString(bool_mapTable, heapMetrics.secure), \
                    heapMetrics.bytesUsed, \
                    heapMetrics.bytesTotal, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_CREATE_VIDEOPOOL_SUCCESS(avBufferManager, secureHeap, videoDecoderId, listener, videoPool) \
do { \
    UT_LOG_DEBUG("Calling createVideoPool(IN:secureHeap:[%d], IN:videoDecoderId:[%d], IN:listener:[%p], OUT:videoPool)", \
                 secureHeap, videoDecoderId.value, listener.get()); \
    Status status = avBufferManager->createVideoPool(secureHeap, videoDecoderId, listener, &videoPool); \
    UT_LOG_DEBUG("Result createVideoPool(IN:secureHeap:[%d], IN:videoDecoderId:[%d], IN:listener:[%p], OUT:videoPool.handle:[%d]), status:[isOK:%s, exception:%s]", \
                 secureHeap, videoDecoderId.value, listener.get(), videoPool.handle, \
                 UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
    UT_ASSERT_TRUE(status.isOk()); \
    UT_ASSERT_NOT_EQUAL(videoPool.handle, Pool::INVALID); \
    UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
} while (0)

#define AVBUFFER_CREATE_VIDEOPOOL_FAILURE(avBufferManager, secureHeap, videoDecoderId, listener, videoPool, exception) \
    do { \
        UT_LOG_DEBUG("Calling createVideoPool(IN:secureHeap:[%d], IN:videoDecoderId:[%d], IN:listener:[%p], OUT:videoPool)", \
            secureHeap, videoDecoderId.value, listener.get()); \
        Status status = avBufferManager->createVideoPool(secureHeap, videoDecoderId, listener, &videoPool); \
        UT_LOG_DEBUG("Result createVideoPool(IN:secureHeap:[%d], IN:videoDecoderId:[%d], IN:listener:[%p], OUT:videoPool.handle:[%d]), status:[isOK:%s, exception:%s]", \
                    secureHeap, videoDecoderId.value, listener.get(), videoPool.handle, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
    } while (0)

#define AVBUFFER_CREATE_AUDIOPOOL_SUCCESS(avBufferManager, secureHeap, audioDecoderId, listener, audioPool) \
    do { \
        UT_LOG_DEBUG("Calling createAudioPool(IN:secureHeap:[%d], IN:audioDecoderId:[%d], IN:listener:[%p], OUT:audioPool)", \
                    secureHeap, audioDecoderId.value, listener.get()); \
        Status status = gAVBufferManager->createAudioPool(secureHeap, audioDecoderId, listener, &audioPool); \
        UT_LOG_DEBUG("Result createAudioPool(IN:secureHeap:[%d], IN:audioDecoderId:[%d], IN:listener:[%p], OUT:audioPool.handle:[%d]), status:[isOK:%s, exception:%s]", \
                    secureHeap, audioDecoderId.value, listener.get(), audioPool.handle, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_NOT_EQUAL(audioPool.handle, Pool::INVALID); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_CREATE_AUDIOPOOL_FAILURE(avBufferManager, secureHeap, audioDecoderId, listener, audioPool, exception) \
    do { \
        UT_LOG_DEBUG("Calling createAudioPool(IN:secureHeap:[%d], IN:audioDecoderId:[%d], IN:listener:[%p], OUT:audioPool)", \
                    secureHeap, audioDecoderId.value, listener.get()); \
        Status status = gAVBufferManager->createAudioPool(secureHeap, audioDecoderId, listener, &audioPool); \
        UT_LOG_DEBUG("Result createAudioPool(IN:secureHeap:[%d], IN:audioDecoderId:[%d], IN:listener:[%p], OUT:audioPool.handle:[%d]), status:[isOK:%s, exception:%s]", \
                    secureHeap, audioDecoderId.value, listener.get(), audioPool.handle, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
    } while (0)

#define AVBUFFER_DESTROY_POOL_SUCCESS(avBufferManager, pool) \
    do { \
        bool _aidl_return = false; \
        Status status; \
        UT_LOG_DEBUG("Calling destroyPool(IN:poolHandle:[%d], OUT:_aidl_return:[])", pool.handle); \
        status = avBufferManager->destroyPool(pool, &_aidl_return); \
        UT_LOG_DEBUG("Result destroyPool with (IN:poolHandle:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    pool.handle, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_TRUE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_DESTROY_POOL_FAILURE(avBufferManager, pool, exception) \
    do { \
        bool _aidl_return = false; \
        Status status; \
        UT_LOG_DEBUG("Calling destroyPool(IN:poolHandle:[%d], OUT:_aidl_return:[])", pool.handle); \
        status = avBufferManager->destroyPool(pool, &_aidl_return); \
        UT_LOG_DEBUG("Result destroyPool with (IN:poolHandle:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    pool.handle, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(_aidl_return); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
    } while (0)

#define AVBUFFER_GETPOOL_METRICS_SUCCESS(avBufferManager, pool, poolMetrics) \
    do { \
        UT_LOG_DEBUG("Calling getPoolMetrics(IN:poolHandle:[%d], OUT:PoolMetrics[])", pool.handle); \
        Status status = avBufferManager->getPoolMetrics(pool, &poolMetrics); \
        UT_LOG_DEBUG("Result getPoolMetrics(IN:poolHandle:[%d], OUT:PoolMetrics[bytesUsed:%d, bytesTotal:%d]), status:[isOK:%s, exception:%s]", \
                    pool.handle, poolMetrics.bytesUsed, poolMetrics.bytesTotal, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_GETPOOL_METRICS_FAILURE(avBufferManager, pool, poolMetrics, exception) \
    do { \
        UT_LOG_DEBUG("Calling getPoolMetrics(IN:poolHandle:[%d], OUT:PoolMetrics[])", pool.handle); \
        Status status = avBufferManager->getPoolMetrics(pool, &poolMetrics); \
        UT_LOG_DEBUG("Result getPoolMetrics(IN:poolHandle:[%d], OUT:PoolMetrics[bytesUsed:%d, bytesTotal:%d]), status:[isOK:%s, exception:%s]", \
                    pool.handle, poolMetrics.bytesUsed, poolMetrics.bytesTotal, \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
    } while (0)

#define AVBUFFER_GETALLPOOL_METRICS_SUCCESS(avBufferManager, secureHeap, poolMetrics) \
    do { \
        UT_LOG_DEBUG("Calling getAllPoolMetrics(IN:secureHeap:[%s], OUT:poolMetrics[])", UT_Control_GetMapString(bool_mapTable, secureHeap)); \
        Status status = avBufferManager->getAllPoolMetrics(secureHeap, &poolMetrics); \
        UT_LOG_DEBUG("Result getAllPoolMetrics(IN:secureHeap:[%s], OUT:poolMetrics[%zu]), status:[isOK:%s, exception:%s]", \
            UT_Control_GetMapString(bool_mapTable, secureHeap), poolMetrics.size(), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                     UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_ALLOC_SUCCESS(avBufferManager, pool, size, bufferHandle) \
    do { \
        UT_LOG_DEBUG("Calling alloc(IN:poolHandle:[%d], IN:size:[%d], OUT:bufferHandle)", pool.handle, size); \
        Status status = avBufferManager->alloc(pool, size, &bufferHandle); \
        UT_LOG_DEBUG("Result alloc(IN:poolHandle:[%d], IN:size:[%d], OUT:bufferHandle:[0x%X]), status:[isOK:%s, exception:%s]", \
                    pool.handle, size, bufferHandle, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_NOT_EQUAL(bufferHandle, IAVBuffer::INVALID_HANDLE); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_ALLOC_FAILURE(avBufferManager, pool, size, bufferHandle, exception) \
    do { \
        UT_LOG_DEBUG("Calling alloc(IN:poolHandle:[%d], IN:size:[%d], OUT:bufferHandle)", pool.handle, size); \
        Status status = avBufferManager->alloc(pool, size, &bufferHandle); \
        UT_LOG_DEBUG("Result alloc(IN:poolHandle:[%d], IN:size:[%d], OUT:bufferHandle:[0x%X]), status:[isOK:%s, exception:%s]", \
                    pool.handle, size, bufferHandle, UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(bufferHandle, IAVBuffer::INVALID_HANDLE); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
    } while (0)

#define AVBUFFER_FREE_SUCCESS(avBufferManager, bufferHandle) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_DEBUG("Calling free(IN:bufferHandle:[0x%X], OUT:_aidl_return)", bufferHandle); \
        Status status = avBufferManager->free(bufferHandle, &_aidl_return); \
        UT_LOG_DEBUG("Result free(IN:bufferHandle:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    bufferHandle, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(_aidl_return); \
    } while (0)

#define AVBUFFER_FREE_FAILURE(avBufferManager, bufferHandle, exception) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_DEBUG("Calling free(IN:bufferHandle:[0x%X], OUT:_aidl_return)", bufferHandle); \
        Status status = avBufferManager->free(bufferHandle, &_aidl_return); \
        UT_LOG_DEBUG("Result free(IN:bufferHandle:[0x%X], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    bufferHandle, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVBUFFER_NOTIFYSPACE_SUCCESS(avBufferManager, pool, size) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_DEBUG("Calling notifyWhenSpaceAvailable(IN:poolHandle:[%d], IN:size:[%d], OUT:_aidl_return)", pool.handle, size); \
        Status status = avBufferManager->notifyWhenSpaceAvailable(pool, size, &_aidl_return); \
        UT_LOG_DEBUG("Result notifyWhenSpaceAvailable(IN:poolHandle:[%d], IN:size:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    pool.handle, size, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(_aidl_return); \
    } while (0)

#define AVBUFFER_NOTIFYSPACE_FAILURE(avBufferManager, pool, size, exception) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_DEBUG("Calling notifyWhenSpaceAvailable(IN:poolHandle:[%d], IN:size:[%d], OUT:_aidl_return)", pool.handle, size); \
        Status status = avBufferManager->notifyWhenSpaceAvailable(pool, size, &_aidl_return); \
        UT_LOG_DEBUG("Result notifyWhenSpaceAvailable(IN:poolHandle:[%d], IN:size:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    pool.handle, size, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVBUFFER_TRIMSIZE_SUCCESS(avBufferManager, bufferHandle, newSize) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_DEBUG("Calling trimSize with video buffer handle(IN:bufferHandle:[%lld], IN:newSize:[%d], OUT:_aidl_return)", bufferHandle, newSize); \
        Status status = avBufferManager->trimSize(bufferHandle, newSize, &_aidl_return); \
        UT_LOG_DEBUG("Result trimSize(IN:bufferHandle:[%lld], IN:newSize:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    bufferHandle, newSize, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
        UT_ASSERT_TRUE(_aidl_return); \
    } while (0)

#define AVBUFFER_TRIMSIZE_FAILURE(avBufferManager, bufferHandle, newSize, exception) \
    do { \
        bool _aidl_return = false; \
        UT_LOG_DEBUG("Calling trimSize with video buffer handle(IN:bufferHandle:[%lld], IN:newSize:[%d], OUT:_aidl_return)", bufferHandle, newSize); \
        Status status = avBufferManager->trimSize(bufferHandle, newSize, &_aidl_return); \
        UT_LOG_DEBUG("Result trimSize(IN:bufferHandle:[%lld], IN:newSize:[%d], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    bufferHandle, newSize, UT_Control_GetMapString(bool_mapTable, _aidl_return), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_FALSE(_aidl_return); \
    } while (0)

#define AVBUFFER_ISVALID_SUCCESS(avBufferManager, bufferHandle, isValidResult) \
    do { \
        UT_LOG_DEBUG("Calling isValid(IN:bufferHandle:[%lld], OUT:_aidl_return[])", bufferHandle); \
        Status status = avBufferManager->isValid(bufferHandle, &isValidResult); \
        UT_LOG_DEBUG("Result isValid(IN:bufferHandle:[%lld], OUT:_aidl_return:[%s]), status:[isOK:%s, exception:%s]", \
                    bufferHandle, UT_Control_GetMapString(bool_mapTable, isValidResult), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_GETALLOCLIST_SUCCESS(avBufferManager, poolHandle, allocList) \
    do { \
        UT_LOG_DEBUG("Calling getAllocList with video pool handle (IN:poolHandle:[%d], OUT:allocList[])", poolHandle.handle); \
        Status status = avBufferManager->getAllocList(poolHandle, &allocList); \
        UT_LOG_DEBUG("Result getAllocList(IN:poolHandle:[%d], OUT:allocList[size:%zu]), status:[isOK:%s, exception:%s]", \
                    poolHandle.handle, allocList.size(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_TRUE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
    } while (0)

#define AVBUFFER_GETALLOCLIST_FAILURE(avBufferManager, poolHandle, allocList, exception) \
    do { \
        UT_LOG_DEBUG("Calling getAllocList with video pool handle (IN:poolHandle:[%d], OUT:allocList[])", poolHandle.handle); \
        Status status = avBufferManager->getAllocList(poolHandle, &allocList); \
        UT_LOG_DEBUG("Result getAllocList(IN:poolHandle:[%d], OUT:allocList[size:%zu]), status:[isOK:%s, exception:%s]", \
                    poolHandle.handle, allocList.size(), \
                    UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        UT_ASSERT_FALSE(status.isOk()); \
        UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
        UT_ASSERT_TRUE(allocList.empty()); \
    } while (0)

#define AVBUFFER_CALCULATESHA_SUCCESS(avBufferManager, poolHandle, sha1Result) \
    do { \
        UT_LOG_DEBUG("Calling calculateSHA1 with video buffer handle (IN:bufferHandle:[%lld], OUT:sha1Result[])", poolHandle); \
        Status status = avBufferManager->calculateSHA1(poolHandle, &sha1Result); \
        UT_LOG_DEBUG("Result calculateSHA1(IN:bufferHandle:[%lld], OUT:sha1Result[size:%zu]), status:[isOK:%s, exception:%s]", \
                    poolHandle, sha1Result.size(), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        if (status.isOk()) { \
            UT_ASSERT_TRUE(status.isOk()); \
            UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_NONE); \
            UT_ASSERT_FALSE(sha1Result.empty()); \
        } else { \
            UT_ASSERT_EQUAL(status.exceptionCode(), Status::EX_UNSUPPORTED_OPERATION); \
            UT_ASSERT_TRUE(sha1Result.empty()); \
        } \
    } while (0)

#define AVBUFFER_CALCULATESHA_FAILURE(avBufferManager, poolHandle, sha1Result, exception) \
    do { \
        UT_LOG_DEBUG("Calling calculateSHA1 with video buffer handle (IN:bufferHandle:[%lld], OUT:sha1Result[])", poolHandle); \
        Status status = avBufferManager->calculateSHA1(poolHandle, &sha1Result); \
        UT_LOG_DEBUG("Result calculateSHA1(IN:bufferHandle:[%lld], OUT:sha1Result[size:%zu]), status:[isOK:%s, exception:%s]", \
                    poolHandle, sha1Result.size(), UT_Control_GetMapString(bool_mapTable, status.isOk()), \
                    UT_Control_GetMapString(statusException_mapTable, status.exceptionCode())); \
        if(status.exceptionCode() != Status::EX_UNSUPPORTED_OPERATION) { \
            UT_ASSERT_FALSE(status.isOk()); \
            UT_ASSERT_EQUAL(status.exceptionCode(), exception); \
            UT_ASSERT_TRUE(sha1Result.empty()); \
        } \
    } while (0)

#endif //__VTS_AVBUFFER_H__

/** @} */ // End of AV_BUFFER_HALTEST_HEADER
/** @} */ // End of AV_BUFFER_HALTEST
/** @} */ // End of AV_BUFFER
/** @} */ // End of HPK