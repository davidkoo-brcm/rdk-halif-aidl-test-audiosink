#Audio Sink High Level Test Specification Document

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Introduction](#introduction)
- [Module Description](#module-description)
- [L2 Test Specification](#l2-test-specification)
- [L3 Test Specification](#l3-test-specification)

## Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `API`    - Application Programming Interface
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `NA`     - Not Applicable
- `Y`      - Yes
- `CB`     - Call-back function (suffix)
- `RDK`    - Reference Design Kit
- `PCM`    - Pulse Code Modulation

## Introduction

This document provides an overview of the testing requirements for the Audio Sink module. It outlines the scope of testing, objectives, testing levels and approaches, specific test requirements, and expected deliverables.

- Audio Sink Interface Definitions - [Audio Sink HAL](https://github.com/rdkcentral/rdk-halif-aidl/tree/v0.6/audiosink/current/com/rdk/hal/audiosink)

- Audio Sink Overview - [Audio Sink Overview](https://wiki.rdkcentral.com/display/RDKHAL/Audio+Sink)

## Module Description

The audio sink `HAL` provides an interface to sink non-tunnelled `PCM` audio frame buffers to the audio mixer and provide app level controls for mixing levels and fade controls.

The `RDK` middleware GStreamer pipeline contains an `RDK` Audio Sink element designed specifically to work with the Audio Sink HALinterface.

Audio Sink HAL interface takes decoded Audio frame buffers.

## L2 Test Specification

|Test|Description|HAL APIs|
|----|-----------|--------|
|Set and Get Audio Sink Properties|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>set read-write properties (`Property`) of sinks and verify using get function</li><li>get read only properties of sinks and verify with default/supported values read from `hfp_audiosink.yaml`</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, getProperty, close </li><li>IAudioSinkController - setProperty</li></ul>|
|States of Audio Sink|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>Register event listeners using `registerEventListener`</li><li>Ensure state transitions (`open`->`start`->`flush`->`stop`->`close`) audio sink and verify the state using `getState`. Validate event listener notifications using `onStateChanged`</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, registerEventListener, unregisterEventListener, close </li><li>IAudioSinkController - start, stop, flush</li></ul>|
|Set and Get Audio Sink Volume|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>set volume on sinks and verify using get function</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, close </li><li>IAudioSinkController - setVolume getVolume</li></ul>|
|Set Audio Sink VolumeRamp|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>set volumeRamp on sinks</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, close </li><li>IAudioSinkController - setVolumeRamp</li></ul>|

## L3 Test Specification
|Test|Description|HAL APIs|
|----|-----------|--------|
|Render Audio FirstFrame|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>`open` audio sink</li><li>register control and event listeners</li><li>`start` `queueAudioFrame` the sink</li><li>Provide first audio frames to opened audio sink instance</li><li>Verify the first frame received from `onFirstFrameRendered`</li><li>After verification `stop` and `close` the sink</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, registerEventListener, close </li><li>IAudioSinkController - start, queueAudioFrame, stop<li>IAudioSinkControllerListener - onFirstFrameRendered<li>IAVBuffer - createAudioPool, alloc, free, destroyPool</li></ul>|
|End of Audio Stream|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>`open` audio sink</li><li>register control and event listeners</li><li>`start` `queueAudioFrame` the sink</li><li>Provide End of stream frame to opened audio sink instance</li><li>Verify the End of stream frame received from `onEndOfStream`</li><li>After verification `stop` and `close` the sink</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, registerEventListener, close </li><li>IAudioSinkController - start, queueAudioFrame, stop<li>IAudioSinkControllerListener - onEndOfStream<li>IAVBuffer - createAudioPool, alloc, free, destroyPool</li></ul>|
|Audio Underflow|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>`open` audio sink</li><li>register control and event listeners</li><li>`start` `queueAudioFrame` the sink</li><li>Provide underflow frame to opened audio sink instance</li><li>Verify the End of stream frame received from `onAudioUnderflow`</li><li>After verification `stop` and `close` the sink</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, registerEventListener, close </li><li>IAudioSinkController - start, queueAudioFrame, stop<li>IAudioSinkControllerListener - onAudioUnderflow<li>IAVBuffer - createAudioPool, alloc, free, destroyPool</li></ul>|
|Audio Resumed|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>`open` audio sink</li><li>register control and event listeners</li><li>`start` `queueAudioFrame` `stop` `start` the sink</li><li>Provide audio frame to opened audio sink instance</li><li>Verify the Audio Resumed received from `onAudioResumed`</li><li>After verification `stop` and `close` the sink</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, registerEventListener, close </li><li>IAudioSinkController - start, queueAudioFrame, stop<li>IAudioSinkControllerListener - onAudioUnderflow<li>IAVBuffer - createAudioPool, alloc, free, destroyPool</li></ul>|
|Audio Flush|<ul><li>Read supported audio sinks using `IAudioSinkManager` class</li><li>Loop through supported audio sinks</li><li>`open` audio sink</li><li>register control and event listeners</li><li>`start` `queueAudioFrame` `flush`the sink</li><li>Provide audio frame to opened audio sink instance</li><li>Verify the Audio flush received from `onFlushComplete`</li><li>Afterverification `stop` and `close` the sink</li></ul>|<ul><li>IAudioSinkManager - getAudioSinkIds, getAudioSink</li><li>IAudioSink - open, registerEventListener, close </li><li>IAudioSinkController - start, queueAudioFrame, stop<li>IAudioSinkControllerListener - onAudioUnderflow<li>IAVBuffer - createAudioPool, alloc, free, destroyPool</li></ul>|
