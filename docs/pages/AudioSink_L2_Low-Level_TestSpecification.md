# AUDIOSINK L2 Low Level Test Specification and Procedure Documentation

## Table of Contents

    -[AUDIOSINK L2 Low Level Test Specification and Procedure Documentation](#audiosink-l2-low-level-test-specification-and-procedure-documentation)

    -[Table of Contents](#table-of-contents)
         - [Overview](#overview)
           - [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
           - [Definitions](#definitions)
           - [References](#references)
         - [Level 2 Test Procedure](#level-2-test-procedure)

## Overview

    This document describes the level 2 testing suite for the AUDIOSINK module.

### Acronyms, Terms and Abbreviations

    -`HAL` \- Hardware Abstraction Layer, may include some common components
       - `UT`  \- Unit Test(s)
       - `OEM`  \- Original Equipment Manufacture
       - `SoC`  \- System on a Chip

### Definitions

    -`ut-core` \- Common Testing Framework [https://github.com/rdkcentral/ut-core](https://github.com/rdkcentral/ut-core), which wraps a open-source framework that can be expanded to the requirements for future framework.

### References

- `High Level Test Specification` - [audioSink_High-Level_TestSpecification.md](audioSink_High-Level_TestSpecification.md)

## Level 2 Test Procedure

    The following functions are expecting to test the module operates correctly.

### Test 1

| Title         | Details                                                                                                                                                                            |
| ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_audiosinkbase_test_TestReadWriteProperties`                                                                                                                             |
| Description   | Read supported audio sinks using `IAudioSinkManager` class. Loop through supported audio sinks, set read-write properties (`Property`) of sinks and verify using get function. |
| Test Group    | Module : 02                                                                                                                                                                        |
| Test Case ID  | 001                                                                                                                                                                                |
| Priority      | High                                                                                                                                                                               |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                                        | Test Data                                       | Expected Result                            | Notes                |
| ----------------- | ---------------------------------------------------------------------------------- | ----------------------------------------------- | ------------------------------------------ | -------------------- |
| 01                | Retrieve audio sink IDs using `getAudioSinkIds`                                  | ids = empty vector                              | status.isOk() = true                       | Should be successful |
| 02                | Loop through each audio sink ID and retrieve the audio sink using `getAudioSink` | sinkId = valid ID                               | status.isOk() = true                       | Should be successful |
| 03                | Register event listener for each audio sink using `registerEventListener`        | listener = valid listener object                | regStatus.isOk() = true, registered = true | Should be successful |
| 04                | Set property `REFERENCE_LEVEL` using `setProperty`                             | property = REFERENCE_LEVEL, propertyValue = -20 | setStatus.isOk() = true, setResult = true  | Should be successful |
| 05                | Get property `REFERENCE_LEVEL` using `getProperty`                             | property = REFERENCE_LEVEL                      | getStatus.isOk() = true, getValue = -20    | Should be successful |
| 06                | Set property `MIXER_ID` using `setProperty`                                    | property = MIXER_ID, propertyValue = 1          | setStatus.isOk() = true, setResult = true  | Should be successful |
| 07                | Get property `MIXER_ID` using `getProperty`                                    | property = MIXER_ID                             | getStatus.isOk() = true, getValue = 1      | Should be successful |
| 08                | Set property `AV_SOURCE` using `setProperty`                                   | property = AV_SOURCE, propertyValue = 2         | setStatus.isOk() = true, setResult = true  | Should be successful |
| 09                | Get property `AV_SOURCE` using `getProperty`                                   | property = AV_SOURCE                            | getStatus.isOk() = true, getValue = 2      | Should be successful |
| 10                | Set property `DOLBY_ATMOS_LOCK` using `setProperty`                            | property = DOLBY_ATMOS_LOCK, propertyValue = 1  | setStatus.isOk() = true, setResult = true  | Should be successful |
| 11                | Get property `DOLBY_ATMOS_LOCK` using `getProperty`                            | property = DOLBY_ATMOS_LOCK                     | getStatus.isOk() = true, getValue = 1      | Should be successful |
| 12                | Unregister event listener using `unregisterEventListener`                        | listener = valid listener                       | status.isOk() = true, unregistered = true  | Should be successful |

### Test 2

| Title         | Details                                                                                                                                                                                                                                                                                                                                                                |
| ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_audiosink_StateTransitionTest`                                                                                                                                                                                                                                                                                                                              |
| Description   | Read supported audio sinks using `IAudioSinkManager` class. Loop through supported audio sinks. Register event listeners using `registerEventListener`. Ensure state transitions (`open` -> `start` -> `flush` -> `stop` -> `close`) audio sink and verify the state using `getState`. Validate event listener notifications using `onStateChanged`. |
| Test Group    | Module : 02                                                                                                                                                                                                                                                                                                                                                            |
| Test Case ID  | 002                                                                                                                                                                                                                                                                                                                                                                    |
| Priority      | High                                                                                                                                                                                                                                                                                                                                                                   |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                 | Test Data                                                 | Expected Result                                          | Notes                |
| ----------------- | ----------------------------------------------------------- | --------------------------------------------------------- | -------------------------------------------------------- | -------------------- |
| 01                | Retrieve audio sink IDs using `getAudioSinkIds`           | ids = empty vector                                        | status.isOk() = true                                     | Should be successful |
| 02                | Retrieve audio sink using `getAudioSink` for each sink ID | sinkId = valid ID, sink = nullptr                         | status.isOk() = true                                     | Should be successful |
| 03                | Register event listener using `registerEventListener`     | listener = valid listener, registered = false             | regStatus.isOk() = true, registered = true               | Should be successful |
| 04                | Open audio sink using `open`                              | controllerlistener = valid listener, controller = nullptr | status.isOk() = true, controller != nullptr              | Should be successful |
| 05                | Wait for state transition from `CLOSED` to `OPENING`    | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 06                | Wait for state transition from `OPENING` to `READY`     | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 07                | Verify current state using `getState`                     | currentState = valid state                                | statestatus.isOk() = true, currentState = State::READY   | Should be successful |
| 08                | Start audio sink using `start`                            | controller = valid controller                             | status.isOk() = true                                     | Should be successful |
| 09                | Wait for state transition from `READY` to `STARTING`    | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 10                | Wait for state transition from `STARTING` to `STARTED`  | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 11                | Verify current state using `getState`                     | currentState = valid state                                | statestatus.isOk() = true, currentState = State::STARTED | Should be successful |
| 12                | Flush audio sink using `flush`                            | controller = valid controller                             | status.isOk() = true                                     | Should be successful |
| 13                | Wait for state transition from `STARTED` to `FLUSHING`  | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 14                | Wait for state transition from `FLUSHING` to `STARTED`  | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 15                | Verify current state using `getState`                     | currentState = valid state                                | statestatus.isOk() = true, currentState = State::STARTED | Should be successful |
| 16                | Stop audio sink using `stop`                              | controller = valid controller                             | status.isOk() = true                                     | Should be successful |
| 17                | Wait for state transition from `STARTED` to `STOPPING`  | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 18                | Wait for state transition from `STOPPING` to `READY`    | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 19                | Verify current state using `getState`                     | currentState = valid state                                | statestatus.isOk() = true, currentState = State::READY   | Should be successful |
| 20                | Close audio sink using `close`                            | controller = valid controller, closed = false             | status.isOk() = true, closed = true                      | Should be successful |
| 21                | Wait for state transition from `READY` to `CLOSING`     | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 22                | Wait for state transition from `CLOSING` to `CLOSED`    | listener = valid listener                                 | waitForState() = true                                    | Should be successful |
| 23                | Verify current state using `getState`                     | currentState = valid state                                | statestatus.isOk() = true, currentState = State::CLOSED  | Should be successful |
| 24                | Unregister event listener using `unregisterEventListener` | listener = valid listener                                 | status.isOk() = true, unregistered = true                |                      |

### Test 3

| Title         | Details                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_audiosink_TestAudioSinkVolumeControl`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Description   | Read supported audio sinks using `IAudioSinkManager` class. Loop through supported audio sinks. Register event listeners using `registerEventListener`, open audio sink and verify the state using `getState` state should be in `READY`. Set volume ranging from 0.0 to 1.0 by increasing 0.1 on each iteration on opened sink using `setVolume` and verify using `getVolume` set and get volume should be equal. After verification, `close` the sink and check state should be in `CLOSED`. Unregister event listeners using `unregisterEventListener`. |
| Test Group    | Module : 02                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |
| Test Case ID  | 003                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Priority      | High                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                               | Test Data                           | Expected Result                                         | Notes                |
| ----------------- | ------------------------------------------------------------------------- | ----------------------------------- | ------------------------------------------------------- | -------------------- |
| 01                | Get audio sink IDs using `getAudioSinkIds`                              | ids = valid buffer                  | status.isOk() = true                                    | Should be successful |
| 02                | Loop through each audio sink ID and get audio sink using `getAudioSink` | sinkId = valid ID                   | status.isOk() = true                                    | Should be successful |
| 03                | Register event listener using `registerEventListener`                   | listener = valid listener           | regStatus.isOk() = true, registered = true              | Should be successful |
| 04                | Open audio sink using `open`                                            | controllerlistener = valid listener | status.isOk() = true, controller != nullptr             | Should be successful |
| 05                | Get state of audio sink using `getState`                                | currentState = valid state buffer   | statestatus.isOk() = true, currentState = State::READY  | Should be successful |
| 06                | Set volume using `setVolume` for each volume level from 0.0 to 1.0      | volume = 0.0 to 1.0                 | status.isOk() = true, volumeSet = true                  | Should be successful |
| 07                | Get volume using `getVolume` and verify                                 | currentVol = valid volume buffer    | status.isOk() = true, currentVol.volume = volume        | Should be successful |
| 08                | Close audio sink using `close`                                          | controller = valid controller       | status.isOk() = true, closed = true                     | Should be successful |
| 09                | Get state of audio sink using `getState` after closing                  | currentState = valid state buffer   | statestatus.isOk() = true, currentState = State::CLOSED | Should be successful |
| 10                | Unregister event listener using `unregisterEventListener`               | listener = valid listener           | status.isOk() = true, unregistered = true               | Should be successful |

### Test 4

| Title         | Details                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_audiosink_AudioSinkOperations`                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| Description   | Read supported audio sinks using IAudioSinkManager class /n Loop through supported audio sinks /n Register event listeners using registerEventListener /n open audio sink and verify the state using getState state should be in READY /n set mute on sinks using setVolume, verify mute status using getVolume and set unmute setVolume, verify the mute status, volume using getVolume /nAfter verification close the sink check state should be in CLOSED /nUnregister event listeners using unregisterEventListener |
| Test Group    | Module : 02                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Test Case ID  | 004                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Priority      | High                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                               | Test Data                                                            | Expected Result                                                                                  | Notes                |
| ----------------- | ------------------------------------------------------------------------- | -------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------ | -------------------- |
| 01                | Get audio sink IDs using `getAudioSinkIds`                              | ids = valid buffer                                                   | status.isOk() = true                                                                             | Should be successful |
| 02                | Loop through each audio sink ID and get audio sink using `getAudioSink` | sinkId = valid ID, sink = valid buffer                               | status.isOk() = true                                                                             | Should be successful |
| 03                | Register event listener using `registerEventListener`                   | listener = valid listener, registered = valid buffer                 | regStatus.isOk() = true, registered = true                                                       | Should be successful |
| 04                | Open audio sink using `open` and verify state using `getState`        | controllerlistener = valid listener, controller = valid buffer       | status.isOk() = true, currentState = State::READY                                                | Should be successful |
| 05                | Set mute on sink using `setVolume` and verify using `getVolume`       | vol.volume = 0.1 to 1.0, vol.muted = true, volumeSet = valid buffer  | status.isOk() = true, volumeSet = true, currentVol.muted = true                                  | Should be successful |
| 06                | Set unmute on sink using `setVolume` and verify using `getVolume`     | vol.volume = 0.1 to 1.0, vol.muted = false, volumeSet = valid buffer | status.isOk() = true, volumeSet = true, currentVol.muted = false, currentVol.volume = vol.volume | Should be successful |
| 07                | Close audio sink using `close` and verify state using `getState`      | controller = valid controller, closed = valid buffer                 | status.isOk() = true, closed = true, currentState = State::CLOSED                                | Should be successful |
| 08                | Unregister event listener using `unregisterEventListener`               | listener = valid listener, unregistered = valid buffer               | status.isOk() = true, unregistered = true                                                        | Should be successful |

### Test 5

| Title         | Details                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| ------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_audiosink_TestAudioSinkVolumeRamp`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Description   | Read supported audio sinks using `IAudioSinkManager` class. Loop through supported audio sinks. Register event listeners using `registerEventListener`. Open audio sink and verify the state using `getState`, state should be in `READY`. Start opened audio sink, verify the state using `getState` and sink state should be in `STARTED`. Set volumeRamp on sinks using `setVolumeRamp`, loop through `getVolume` until volume is set to same as target volume. After verification, stop, check state should be in READY, and close the sink, check state should be in CLOSED. Unregister event listeners using `unregisterEventListener`. |
| Test Group    | Module : 02                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Test Case ID  | 005                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Priority      | High                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                 | Test Data                                                                                            | Expected Result                                     | Notes                |
| ----------------- | ----------------------------------------------------------- | ---------------------------------------------------------------------------------------------------- | --------------------------------------------------- | -------------------- |
| 01                | Get audio sink IDs using `getAudioSinkIds`                | ids = valid buffer                                                                                   | status.isOk() = true                                | Should be successful |
| 02                | Get audio sink using `getAudioSink`                       | sinkId = valid ID, sink = valid buffer                                                               | status.isOk() = true                                | Should be successful |
| 03                | Register event listener using `registerEventListener`     | listener = valid listener, registered = valid buffer                                                 | regStatus.isOk() = true, registered = true          | Should be successful |
| 04                | Open audio sink using `open`                              | controllerlistener = valid listener, controller = valid buffer                                       | status.isOk() = true, controller != nullptr         | Should be successful |
| 05                | Get state using `getState`                                | currentState = valid buffer                                                                          | status.isOk() = true, currentState = State::READY   | Should be successful |
| 06                | Start audio sink using `start`                            | None                                                                                                 | status.isOk() = true                                | Should be successful |
| 07                | Get state using `getState`                                | currentState = valid buffer                                                                          | status.isOk() = true, currentState = State::STARTED | Should be successful |
| 08                | Set volume ramp using `setVolumeRamp`                     | targetVolume = 0.5, overMs = 1000, volumeRamp = VolumeRamp::LINEAR, volumeRampStarted = valid buffer | status.isOk() = true, volumeRampStarted = true      | Should be successful |
| 09                | Get volume using `getVolume`                              | volume = valid buffer                                                                                | status.isOk() = true, volume.volume = targetVolume  | Should be successful |
| 10                | Stop audio sink using `stop`                              | None                                                                                                 | status.isOk() = true                                | Should be successful |
| 11                | Get state using `getState`                                | currentState = valid buffer                                                                          | status.isOk() = true, currentState = State::READY   | Should be successful |
| 12                | Close audio sink using `close`                            | controller = valid controller, closed = valid buffer                                                 | status.isOk() = true, closed = true                 | Should be successful |
| 13                | Get state using `getState`                                | currentState = valid buffer                                                                          | status.isOk() = true, currentState = State::CLOSED  | Should be successful |
| 14                | Unregister event listener using `unregisterEventListener` | listener = valid listener, unregistered = valid buffer                                               | status.isOk() = true, unregistered = true           | Should be successful |

### Test 6

| Title         | Details                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_audiosink_AudioSinkOperations`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Description   | Read supported audio sinks using `IAudioSinkManager` class. Loop through supported audio sinks. Register event listeners using `registerEventListener`. Open audio sink and verify the state using `getState`, state should be in `READY`. Start opened audio sink, verify the state using `getState` and sink state should be in `STARTED`. Set volume to mute using `setVolume`, set volumeRamp on sinks using `setVolumeRamp`, verify volume by looping through `getVolume` until volume is same as target volume and mute status should be unmute. After verification, stop, check state should be in READY, and close the sink, check state should be in CLOSED. Unregister event listeners using `unregisterEventListener`. |
| Test Group    | Module : 02                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| Test Case ID  | 006                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Priority      | High                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                 | Test Data                                                                                | Expected Result                                                          | Notes                |
| ----------------- | ----------------------------------------------------------- | ---------------------------------------------------------------------------------------- | ------------------------------------------------------------------------ | -------------------- |
| 01                | Get audio sink IDs using `getAudioSinkIds`                | ids = valid buffer                                                                       | status.isOk() = true                                                     | Should be successful |
| 02                | Get audio sink using `getAudioSink`                       | sinkId = valid ID, sink = valid buffer                                                   | status.isOk() = true                                                     | Should be successful |
| 03                | Register event listener using `registerEventListener`     | listener = valid listener, registered = valid buffer                                     | regStatus.isOk() = true, registered = true                               | Should be successful |
| 04                | Open audio sink using `open`                              | controllerlistener = valid listener, controller = valid buffer                           | status.isOk() = true, controller != nullptr                              | Should be successful |
| 05                | Get state using `getState`                                | currentState = valid buffer                                                              | status.isOk() = true, currentState = State::READY                        | Should be successful |
| 06                | Start audio sink using `start`                            | None                                                                                     | status.isOk() = true                                                     | Should be successful |
| 07                | Get state using `getState`                                | currentState = valid buffer                                                              | status.isOk() = true, currentState = State::STARTED                      | Should be successful |
| 08                | Set volume using `setVolume`                              | volume = {volume: 0.0, muted: true}, volumeSet = valid buffer                            | status.isOk() = true, volumeSet = true                                   | Should be successful |
| 09                | Set volume ramp using `setVolumeRamp`                     | targetVolume = 1.0, overMs = 1000, rampType = VolumeRamp::LINEAR, rampSet = valid buffer | status.isOk() = true, rampSet = true                                     | Should be successful |
| 10                | Get volume using `getVolume`                              | volume = valid buffer                                                                    | status.isOk() = true, volume.volume = targetVolume, volume.muted = false | Should be successful |
| 11                | Stop audio sink using `stop`                              | None                                                                                     | status.isOk() = true                                                     | Should be successful |
| 12                | Get state using `getState`                                | currentState = valid buffer                                                              | status.isOk() = true, currentState = State::READY                        | Should be successful |
| 13                | Close audio sink using `close`                            | controller = valid controller, closed = valid buffer                                     | status.isOk() = true, closed = true                                      | Should be successful |
| 14                | Get state using `getState`                                | currentState = valid buffer                                                              | status.isOk() = true, currentState = State::CLOSED                       | Should be successful |
| 15                | Unregister event listener using `unregisterEventListener` | listener = valid listener, unregistered = valid buffer                                   | status.isOk() = true, unregistered = true                                | Should be successful |

### Test 7

| Title         | Details                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
| ------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_audiosink_TestAudioSinkVolumeRampTwice`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| Description   | Read supported audio sinks using `IAudioSinkManager` class. Loop through supported audio sinks. Register event listeners using `registerEventListener`. Open audio sink and verify the state using `getState`, state should be in `READY`. Start opened audio sink, verify the state using `getState` and sink state should be in `STARTED`. Set volumeRamp on sink using `setVolumeRamp`, immediately call set volumeRamp again on sink using `setVolumeRamp`, verify the volume by looping `getVolume` until volume is same as set target volume. After verification, stop and check state should be in READY, and close the sink, check state should be in CLOSED. Unregister event listeners using `unregisterEventListener`. |
| Test Group    | Module : 02                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| Test Case ID  | 007                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   |
| Priority      | High                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                 | Test Data                                                                                      | Expected Result                                     | Notes                |
| ----------------- | ----------------------------------------------------------- | ---------------------------------------------------------------------------------------------- | --------------------------------------------------- | -------------------- |
| 01                | Get audio sink IDs using `getAudioSinkIds`                | ids = valid buffer                                                                             | status.isOk() = true                                | Should be successful |
| 02                | Get audio sink using `getAudioSink`                       | sinkId = valid ID, sink = valid buffer                                                         | status.isOk() = true                                | Should be successful |
| 03                | Register event listener using `registerEventListener`     | listener = valid listener, registered = valid buffer                                           | regStatus.isOk() = true, registered = true          | Should be successful |
| 04                | Open audio sink using `open`                              | controllerlistener = valid listener, controller = valid buffer                                 | status.isOk() = true, controller != nullptr         | Should be successful |
| 05                | Get state using `getState`                                | currentState = valid buffer                                                                    | status.isOk() = true, currentState = State::READY   | Should be successful |
| 06                | Start audio sink using `start`                            | None                                                                                           | status.isOk() = true                                | Should be successful |
| 07                | Get state using `getState`                                | currentState = valid buffer                                                                    | status.isOk() = true, currentState = State::STARTED | Should be successful |
| 08                | Set volume ramp using `setVolumeRamp`                     | targetVolume = 0.5, overMs = 1000, volumeRamp = VolumeRamp::LINEAR, rampStarted = valid buffer | status.isOk() = true, rampStarted = true            | Should be successful |
| 09                | Set volume ramp again using `setVolumeRamp`               | targetVolume = 0.7, overMs = 1000, volumeRamp = VolumeRamp::LINEAR, rampStarted = valid buffer | status.isOk() = true, rampStarted = true            | Should be successful |
| 10                | Get volume using `getVolume`                              | volume = valid buffer                                                                          | status.isOk() = true, volume.volume = targetVolume  | Should be successful |
| 11                | Stop audio sink using `stop`                              | None                                                                                           | status.isOk() = true                                | Should be successful |
| 12                | Get state using `getState`                                | currentState = valid buffer                                                                    | status.isOk() = true, currentState = State::READY   | Should be successful |
| 13                | Close audio sink using `close`                            | controller = valid controller, closed = valid buffer                                           | status.isOk() = true, closed = true                 | Should be successful |
| 14                | Get state using `getState`                                | currentState = valid buffer                                                                    | status.isOk() = true, currentState = State::CLOSED  | Should be successful |
| 15                | Unregister event listener using `unregisterEventListener` | listener = valid listener, unregistered = valid buffer                                         | status.isOk() = true, unregistered = true           | Should be successful |

### Test 8

| Title         | Details                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 |
| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Function Name | `test_l2_AudioSinkTest_TestAudioSinkLifecycle`                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        |
| Description   | Read supported audio sinks using `IAudioSinkManager` class. Loop through supported audio sinks. Register event listeners using `registerEventListener`. Open audio sink and verify the state using `getState`, state should be in `READY`. Start opened audio sink, verify the state using `getState` and sink state should be in `STARTED`. Set volumeRamp on sink using `setVolumeRamp`, immediately call `stop` audio sink and verify volume is set to target volume using `getVolume`. After verification, `stop` check state should be in `READY`, and `close` the sink check state should be in `CLOSED`. Unregister event listeners using `unregisterEventListener`. |
| Test Group    | Module : 02                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                             |
| Test Case ID  | 008                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| Priority      | High                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                    |

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure :

| Variation / Steps | Description                                                                   | Test Data                                                          | Expected Result                           | Notes                |
| ----------------- | ----------------------------------------------------------------------------- | ------------------------------------------------------------------ | ----------------------------------------- | -------------------- |
| 01                | Open audio sink using `open` and verify the state using `getState`        | controllerlistener = valid object, controller = valid object       | State::READY                              | Should be successful |
| 02                | Start audio sink using `start` and verify the state using `getState`      | None                                                               | State::STARTED                            | Should be successful |
| 03                | Set volume ramp using `setVolumeRamp` and verify volume using `getVolume` | targetVolume = 0.5, overMs = 1000, volumeRamp = VolumeRamp::LINEAR | targetVolume = 0.5                        | Should be successful |
| 04                | Stop audio sink using `stop` and verify the state using `getState`        | None                                                               | State::READY                              | Should be successful |
| 05                | Close audio sink using `close` and verify the state using `getState`      | controller = valid object                                          | State::CLOSED                             | Should be successful |
| 06                | Unregister event listener using `unregisterEventListener`                   | listener = valid listener, unregistered = valid buffer             | status.isOk() = true, unregistered = true | Should be successful |
