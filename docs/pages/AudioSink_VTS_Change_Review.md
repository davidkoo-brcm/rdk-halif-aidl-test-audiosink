# AudioSink VTS Change Review

## Latest test results

Platform: Broadcom Xione. Profile: `profiles/hfp-unified_xione.yaml`.

| Check | Latest result |
| --- | --- |
| AVClock regression with DSOLA off | PASS — confirmed by the user |
| `seq_tun_nonsec_AutoAudioBeforeVideo` (2026-11-12) | PASS — 81.119 s |

An AVClock PASS verifies priming and clock progression, not simultaneous audible output or pitch-corrected trick playback.

## Prepare the sink before decoder start

Expose an idempotent preparation stage so AVClock can open the decoder, connect the sink and bind STC before submitting media. Both tunnel and non-tunnel run paths reuse an already prepared sink.

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
