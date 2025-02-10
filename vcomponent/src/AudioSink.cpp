/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
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
 
/** @brief 
* AudioSink.cpp
* 
* Implementation of AudioSink service
*/
#include "AudioSink.h" 

namespace com {
    namespace rdk {
        namespace hal {
            namespace audiosink {

                  AudioSinkManager::AudioSinkManager() 
                  {
                      printf("AudioSinkManager service initialized\n");
                  }
                  
                  AudioSinkManager::~AudioSinkManager() 
                  {
                      printf("AudioSinkManager service destroyed\n");
                  }

                  android::binder::Status AudioSinkManager::getAudioSinkIds(  std::vector<IAudioSink::Id>* _aidl_return  )
                  {
                      IAudioSink::Id id1;
                      id1.value = 0; // Set value
                      IAudioSink::Id id2;
                      id2.value = 1; // Set value
                      _aidl_return->push_back(id1);
                      _aidl_return->push_back(id2);
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkManager::getPlatformCapabilities(  PlatformCapabilities* _aidl_return  )
                  {
                      if (_aidl_return == nullptr) 
                      {
                          return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                      }

                      _aidl_return->supportsLowLatency = true;
                      _aidl_return->systemMixerSampleRateHz = 48000;  // Example sample rate
                      _aidl_return->systemMixerPCMFormat = com::rdk::hal::audiodecoder::PCMFormat::S16LE;  // Example PCM format
                      _aidl_return->supportsPlanarFormat = false;

                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkManager::getAudioSink(  const IAudioSink::Id& audioSinkId, android::sp<IAudioSink>* _aidl_return  )
                  {
                      android::sp<IAudioSink> audioSink = new AudioSink();
                      *_aidl_return = audioSink;
                      return android::binder::Status::ok();
                  }
                  
                  AudioSink::AudioSink() 
                  {
                      printf("AudioSink service initialized\n");
                  }
                  
                  AudioSink::~AudioSink() 
                  {
                      printf("AudioSink service destroyed\n");
                  }

                  android::binder::Status AudioSink::getCapabilities(  audiosink::Capabilities* _aidl_return  ) 
                  {
                      return android::binder::Status::ok();
                  }


                  android::binder::Status AudioSink::getProperty(audiosink::Property property, std::optional<com::rdk::hal::PropertyValue>* _aidl_return)
                  {
                    if (!_aidl_return)
                    {
                        return android::binder::Status::fromExceptionCode(android::binder::Status::EX_NULL_POINTER);
                    }
                    com::rdk::hal::PropertyValue value;
                    switch (property) 
                    {
                        case audiosink::Property::RESOURCE_ID:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(42);
                            break;
                        case audiosink::Property::REFERENCE_LEVEL:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(-31);
                            break;
                        case audiosink::Property::AV_SOURCE:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(/* your AV source value here */ 0);
                            break;
                        case audiosink::Property::RENDER_LATENCY_NS:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(0);
                            break;
                        case audiosink::Property::QUEUE_DEPTH_NS:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(0);
                            break;
                        case audiosink::Property::MIXER_ID:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(0);
                            break;
                        case audiosink::Property::DOLBY_ATMOS_LOCK:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(1);
                            break;
                        case audiosink::Property::METRIC_xxxx:
                            value.value = com::rdk::hal::PropertyValue::Value::make<com::rdk::hal::PropertyValue::Value::intValue>(/* metric value */ 100);
                            break;
                        default:
                            return android::binder::Status::fromExceptionCode(android::binder::Status::EX_ILLEGAL_ARGUMENT);
                    }
                    *_aidl_return = value;
                    return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSink::setProperty(  audiosink::Property property, const PropertyValue& propertyValue, bool* _aidl_return  ) 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSink::getState(  State* _aidl_return  )
                  {
                      *_aidl_return = State::CLOSED;
                      return android::binder::Status::ok();
                  }


                  android::binder::Status AudioSink::open(  const android::sp<audiosink::IAudioSinkControllerListener>& audioSinkControllerListener, android::sp<audiosink::IAudioSinkController>* _aidl_return  )
                  {
                      *_aidl_return = new AudioSinkController();
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSink::close(  const android::sp<audiosink::IAudioSinkController>& audioSinkController, bool* _aidl_return  ) 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSink::registerEventListener(  const android::sp<audiosink::IAudioSinkEventListener>& audioSinkEventListener, bool* _aidl_return  ) 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSink::unregisterEventListener(  const android::sp<audiosink::IAudioSinkEventListener>& audioSinkEventListener, bool* _aidl_return  ) 
                  {
                      return android::binder::Status::ok();
                  }
                  AudioSinkController::AudioSinkController() 
                  {
                      printf("AudioSinkController service initialized\n");
                  }
                  
                  AudioSinkController::~AudioSinkController() 
                  {
                      printf("AudioSinkController service destroyed");
                  }

                  android::binder::Status AudioSinkController::setAudioDecoder(  const ::com::rdk::hal::audiodecoder::IAudioDecoder::Id& audioDecoderId, bool* _aidl_return  )
                  {
                      *_aidl_return = true;
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkController::getAudioDecoder(  ::com::rdk::hal::audiodecoder::IAudioDecoder::Id* _aidl_return  ) 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkController::start() 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkController::stop() 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkController::queueAudioFrame(  int64_t nsPresentationTime, int64_t bufferHandle, const ::com::rdk::hal::audiodecoder::FrameMetadata& metadata, bool* _aidl_return  ) 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkController::flush() 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkController::getVolume(  com::rdk::hal::audiosink::Volume* _aidl_return) 
                  {
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkController::setVolume(  const com::rdk::hal::audiosink::Volume& volume, bool* _aidl_return) 
                  {
                      *_aidl_return = true;
                      return android::binder::Status::ok();
                  }

                  android::binder::Status AudioSinkController::setVolumeRamp(  double targetVolume, int32_t overMs, com::rdk::hal::audiosink::VolumeRamp volumeRamp, bool* _aidl_return  ) 
                  {
                      return android::binder::Status::ok();
                  }
                }
            }
        }
    }
	
