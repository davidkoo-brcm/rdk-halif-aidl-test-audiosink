# AudioSink VTS Change Review

## Latest test results

Platform: Broadcom Xione. Profile: `profiles/hfp-unified_xione.yaml`. Run date: 2026-11-12.

| Level | Total | PASS | FAIL | SKIP | Duration (s) |
| --- | ---: | ---: | ---: | ---: | ---: |
| L1 | 39 | 39 | 0 | 0 | 101.345 |
| L2 | 9 | 9 | 0 | 0 | 95.480 |
| L3 | 32 | 7 | 1 | 24 | 268.373 |
| Total | 80 | 55 | 1 | 24 | 465.198 |

Sequential Tunnel NonSecure basic, Flush, EOS and Underflow pass. Concurrent basic, Flush and EOS pass. `ASink_Concurrent_Tunnel_NonSecure_Underflow` is classified as a hardware limitation of Xione simultaneous audio playback; its raw VTS result remains FAIL. Secure and NonTunnel cases skip.

## Prepare the sink before decoder start

Expose an idempotent sink preparation stage. AVClock and AudioSink L3 open/configure the decoder first, then connect the sink and bind STC before submitting media. Both tunnel and non-tunnel run paths reuse an already prepared sink.

[`src/vts_audiosink.cpp:660`](../../src/vts_audiosink.cpp#L660)

```cpp
bool AudioSinkTest::prepare()
{
    if (mAudioSinkController != nullptr) return true;
    setupAudioSink();
    return mAudioSinkController != nullptr;
}
```

## Keep the route through clock validation

`runKeepingSession()` retains the output connection until AVClock finishes its sample. Normal run calls still close on completion. The destructor supplies cleanup for an early return, and teardown clears the controller to prevent a second close.

[`src/vts_audiosink.h:977`](../../src/vts_audiosink.h#L977)

```cpp
    // Connects and starts the sink before the decoder submits its first PTS.
    bool prepare();
    ~AudioSinkTest();
    void run();
    // Keep the route alive until an external clock measurement completes.
    void runKeepingSession() { mKeepSessionOpen = true; run(); }
    void close();
```

## Make close idempotent

A completed close clears the controller. Prepared helper destruction therefore does not close the same session again.

[`src/vts_audiosink.cpp:655`](../../src/vts_audiosink.cpp#L655)

```cpp
void AudioSinkTest::close()
{
    if (mAudioSinkController != nullptr) tearDownAudioSink();
}
```

## AudioSink L3 startup ordering

Both sequential and concurrent paths prepare the decoder, prepare the clock, connect the sink/STC, then start the decoder. Prepare the owned clock before sink start so the HAL can bind the NEXUS STC channel. Paths using an external AVClock reuse that prepared clock.

[`src/vts_audiosink.cpp:714`](../../src/vts_audiosink.cpp#L714)

```cpp
    // The clock must exist when sink start binds the decoder's STC channel.
    if(!mClockInUse)
    {
        setupClock();
    }

    /* Start the audio sink before the decoder starts playback. */
    AUDIOSINK_START_SUCCESS(mSinkHandle, mAudioSinkController, true);
```

The sequential path starts the decoder thread only after both helpers prepare successfully.

[`src/vts_audiosink_l3_tests.cpp:315`](../../src/vts_audiosink_l3_tests.cpp#L315)

```cpp
                    if (!decodeTest.prepare()) return;

                    AudioSinkTest audiosinkTest(_sinkId, std::get<1>(sinkTuple), std::get<2>(sinkTuple), std::get<3>(sinkTuple),
                                                decoderControlListener, mAVBufferManager, decoderId, clockId,
                                                secureDecode, isTunnel, testConfig, isClockInUse);
                    if (!audiosinkTest.prepare()) return;

                    std::thread decodeThread(&AudioDecoderTest::run, &decodeTest, false);
                    audiosinkTest.run();
```

The concurrent path prepares all decoders, then all sinks, before launching threads. Preparation failures return before any threads start. After completion, sink helpers are released before decoder helpers and the heap metrics check.

[`src/vts_audiosink_l3_tests.cpp:452`](../../src/vts_audiosink_l3_tests.cpp#L452)

```cpp
            for (int i = 0; i < numInstances; ++i)
                sinkThreads.emplace_back(&AudioSinkTest::run, sinkTests[i].get());
            for (int i = 0; i < numInstances; ++i)
                decoderThreads.emplace_back(&AudioDecoderTest::run, decoderTests[i].get(), false);
```
