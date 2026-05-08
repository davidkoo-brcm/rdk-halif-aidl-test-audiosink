# Audio Sink L2 Low Level Test Specification and Procedure Documentation

## Table of Contents

- [Acronyms, Terms and Abbreviations](#acronyms-terms-and-abbreviations)
- [Overview](#overview)
- [Definitions](#definitions)
- [References](#references)
- [Level 2 Test Procedure](#level-2-test-procedure)

### Acronyms, Terms and Abbreviations

- `HAL`    - Hardware Abstraction Layer
- `API`    - Application Programming Interface
- `L2`     - Level 2 Testing
- `L3`     - Level 3 Testing
- `NA`     - Not Applicable
- `Y`      - Yes

## Overview

This document describes the Low Level L2 Test Specification and Procedure for the Audio Sink module.

### Definitions

- `ut-core` \- Common Testing Framework <https://github.com/rdkcentral/ut-core>, which wraps a open-source framework that can be expanded to the requirements for future framework.

### References

- High Level Test Specification - [audioSink_High-Level_TestSpecification.md](audioSink_High-Level_TestSpecification.md)

## Level 2 Test Procedure

The following functions are expecting to test the module operates correctly.

### Test 1

|Title|Details|
|-----|-------|
|Function Name|`TestReadWriteProperties`|
|Description|Loop through supported audio sinkid's, set property using setProperty and verify the property using getProperty|
|Test Group|02|
|Test Case ID|001|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 1

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks id's using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Loop through audio sink id's ,open Audio Sink using `open` obtain `audioSinkController` and check the state of audio sink using `getState`|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|04|Loop through the set of properties and set the properties using `setProperty`|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|05|Get property which is set in step 04 using `getProperty`|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|06|Compare get and set properties and should be equal|None|`true`|Should be successful|
|07|Stop audio sink instance using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|08|Close audio sink instance using `close` and check the state of audio sink using `getState`|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|

### Test 2

|Title|Details|
|-----|-------|
|Function Name|`TestStateTransition`|
|Description|Loop through supported audio sink id's , change the states of the audio sink using `open`, `start`, `flush`, `stop`, `close` , and verify the same using `getState`|
|Test Group|02|
|Test Case ID|002|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 2

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks id's using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Loop through the audio sink id's , open Audio Sink using `open` obtain `audioSinkController` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|04|Start the audio sink using `start` and check the state of the audio sink using `getState` it should be in `STARTED` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|05|Flush the audio sink using `flush` and check the state of the audio sink using `getState` it should be in `STARTED` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|06|Stop the audio sink using `stop` and check the state of the audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|05|Get property which is set in step 04 using `getProperty`|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|06|Stop audio sink instance using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|07|Close audio sink instance using `close` and check the state of audio sink using `getState` it should be in `CLOSED` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|

### Test 3

|Title|Details|
|-----|-------|
|Function Name|`TestAudioSinkVolumeControl`|
|Description|Loop through supported audio sink id's , change the volume of the audio sink using `setVolume` and verify the same using `getVolume`|
|Test Group|02|
|Test Case ID|003|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 3

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks id's using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Loop through the audio sink id's , open Audio Sink using `open` obtain `audioSinkController` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|04|Set the volume using `setVolume`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|05|get the volume using `getVolume`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|06|Verify the set and get volume same |None|`true`|Should be successful|
|07|Stop audio sink instance using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|08|Close audio sink instance using `close` and check the state of audio sink using `getState` it should be in `CLOSED` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|

### Test 4

|Title|Details|
|-----|-------|
|Function Name|`TestAudioSinkMuteUnmute`|
|Description|Loop through supported audio sink id's, mute , unmute the audio sinks using `setVolume` and `getVolume` verify the same should be equal|
|Test Group|02|
|Test Case ID|004|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 4

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks id's using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Loop through the audio sink id's , open Audio Sink using `open` obtain `audioSinkController` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|04|Set the volume and mute status using `setVolume`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|05|Get the volume and mute status using `getVolume`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|06|Verify the set and get volume same |None|`true`|Should be successful|
|07|Stop audio sink instance using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|08|Close audio sink instance using `close` and check the state of audio sink using `getState` it should be in `CLOSED` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|

### Test 5

|Title|Details|
|--|--|
|Function Name|`TestAudioSinkVolumeRamp`|
|Description|Loop through supported audio sink id's, set volume ramp using `setVolumeRamp` and verify volume using `getVolume`|
|Test Group|02|
|Test Case ID|005|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 5

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks id's using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Loop through the audio sink id's , open Audio Sink using `open` obtain `audioSinkController` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|04|Set the volume ramp using `setVolumeRamp`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|05|Get the volume set by volume ramp using `getVolume`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|06|Verify the volume set by volume ramp and get volume same |None|`true`|Should be successful|
|07|Stop audio sink instance using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|08|Close audio sink instance using `close` and check the state of audio sink using `getState` it should be in `CLOSED` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|

### Test 6

|Title|Details|
|-----|-------|
|Function Name|`TestAudioSinkVolumeRampMute`|
|Description|Loop through supported audio sink id's, test the volume unmuted after volmume ramp set|
|Test Group|02|
|Test Case ID|006|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 6

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks id's using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Loop through the audio sink id's , open Audio Sink using `open` obtain `audioSinkController` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|04|Start the audio sink using `start` and audio sink state should be `STARTED`|
|05|Set the volume using `setVolume` to mute|None|`EX_NONE`|Should be successful|
|06|Set the volume ramp with target volumes(0.0,0.5,1.0), and volume ramps(i.e `LINEAR`, `IN_CUBIC`, `OUT_CUBIC`) using `setVolumeRamp`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|07|Get the volume set by volume ramp using `getVolume`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|08|Verify the volume  and mute status should be in unmute after volume ramp completed|None|`true`|Should be successful|
|09|Stop audio sink instance using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|10|Close audio sink instance using `close` and check the state of audio sink using `getState` it should be in `CLOSED` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|

### Test 7

|Title|Details|
|-----|-------|
|Function Name|`TestAudioSinkVolumeRampTwice`|
|Description|Loop through supported audio sink id's, test the volume ramp twice , volume ramp should override with new volume ramp|
|Test Group|02|
|Test Case ID|007|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 7

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks id's using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Loop through the audio sink id's , open Audio Sink using `open` obtain `audioSinkController` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|04|Start the audio sink using `start` and audio sink state should be `STARTED`|
|05|Set the volume ramp with target volume (0.0), and volume ramps(i.e `LINEAR`, `IN_CUBIC`, `OUT_CUBIC`) using `setVolumeRamp`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|06|Set the volume ramp again with target volume (0.5), and volume ramps(i.e `LINEAR`, `IN_CUBIC`, `OUT_CUBIC`) using `setVolumeRamp`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|07|Get the volume set by volume ramp using `getVolume`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|08|Verify the volume is set to latest volume ramp|None|`true`|Should be successful|
|09|Stop audio sink instance using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|10|Close audio sink instance using `close` and check the state of audio sink using `getState` it should be in `CLOSED` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|

### Test 8

|Title|Details|
|-----|-------|
|Function Name|`TestStopAudioSinkVolRampProgress`|
|Description|Loop through supported audio sink id's, stop audio sink while volume ramp in progress volume should set to the interrupted previous volume ramp|
|Test Group|02|
|Test Case ID|008|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 8

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks id's using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Loop through the audio sink id's , open Audio Sink using `open` obtain `audioSinkController` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|04|Start the audio sink using `start` and audio sink state should be `STARTED`|
|05|Set the volume ramp with target volume (0.5), and volume ramps(i.e `LINEAR`, `IN_CUBIC`, `OUT_CUBIC`) using `setVolumeRamp`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|06|Stop audio sink instance while volume ramp in progress using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|07|Get the volume set by volume ramp using `getVolume`|Audio Sink controller handle = obtained from `open`|`EX_NONE`|Should be successful|
|08|Verify the volume is equal to target volume|None|`true`|Should be successful|
|09|Stop audio sink instance using `stop` and check the state of audio sink using `getState` it should be in `READY` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|
|10|Close audio sink instance using `close` and check the state of audio sink using `getState` it should be in `CLOSED` state|Audio Sink handle = obtained from `getAudioSink`|`dsERR_NONE`|Should be successful|


### Test 9

|Title|Details|
|-----|-------|
|Function Name|`TestClockAttachDetach`|
|Description|Loop through supported audio sink IDs, attach and detach clocks in READY and STARTED states, and verify attached clock ID|
|Test Group|02|
|Test Case ID|009|
|Priority|High|

**Pre-Conditions :**
None

**Dependencies :**
None

**User Interaction :**
If user chose to run the test in interactive mode, then the test case has to be selected via console.

#### Test Procedure - Test 9

|Variation / Steps|Description|Test Data|Expected Result|Notes|
|-----------------|-----------|---------|---------------|-----|
|01|Get audio sinks IDs using `getAudioSinkIds`|None|`EX_NONE`|Should be successful|
|02|Loop through audio sinks and get audio sink using `getAudioSink`|None|`EX_NONE`|Should be successful|
|03|Open Audio Sink using `open` to obtain `audioSinkController` and check state is `READY`|Audio Sink handle = obtained from `getAudioSink`|`EX_NONE`|Should be successful|
|04|For each available clock ID, attach the clock using `attachClock` in READY state|clockId from configuration|`EX_NONE`|Should be successful|
|05|Get the attached clock using `getClock` and verify the ID matches|audioSinkController, clockId|Attached clock ID matches|Should Pass|
|06|Detach the clock using `detachClock`|audioSinkController|`EX_NONE`|Should be successful|
|07|Start the audio sink using `start` and check state is `STARTED`|audioSinkController|`EX_NONE`|Should be successful|
|08|For each available clock ID, attach the clock using `attachClock` in STARTED state|clockId from configuration|`EX_NONE`|Should be successful|
|09|Get the attached clock using `getClock` and verify the ID matches|audioSinkController, clockId|Attached clock ID matches|Should Pass|
|10|Detach the clock using `detachClock`|audioSinkController|`EX_NONE`|Should be successful|
|11|Stop the audio sink using `stop` and check state is `READY`|audioSinkController|`EX_NONE`|Should be successful|
|12|Close the audio sink using `close` and check state is `CLOSED`|audioSinkController|`EX_NONE`|Should be successful|
