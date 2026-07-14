/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2025 RDK Management
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
* AudioSinkService.cpp
*
* Main entry point for the AudioSinkService service
*/

#include "AudioSink.h"
#include <binder/ProcessState.h>
#include <binder/IPCThreadState.h>
#include <signal.h>
#include <stdio.h>

using namespace com::rdk::hal::audiosink;

int main(int , char* [])
{
    signal(SIGPIPE, SIG_IGN);

    /* Start the binder thread pool first so the service manager connection
     * is established before we try to register.  Without this, a failing
     * defaultServiceManager() call can cause joinThreadPool() to return
     * immediately. */
    android::sp<android::ProcessState> ps = android::ProcessState::self();
    ps->startThreadPool();

    android::status_t err = AudioSinkManager::publish();
    if (err != android::OK) {
        fprintf(stderr, "ERROR: Failed to register AudioSinkManager with servicemanager (status=%d). "
                        "Is servicemanager running?\n", err);
        return 1;
    }

    printf("AudioSinkManager service registered successfully. Waiting for clients...\n");
    fflush(stdout);

    /* Block this thread in the binder thread pool until the process is killed. */
    android::IPCThreadState::self()->joinThreadPool();

    return 0;
}