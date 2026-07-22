#* ******************************************************************************
#*  If not stated otherwise in this file or this component's LICENSE
#*  file the following copyright and licenses apply:
#*
#*  Copyright 2025 RDK Management
#*
#*  Licensed under the Apache License, Version 2.0 (the License);
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*  http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an AS IS BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#*
#* ******************************************************************************

DEV_BINDER='/dev/binder'
RUN_DIR=`realpath $(dirname $0)`

# Function to check if a process is running
check_process() {
    local process_name=$1
    pgrep -f "$process_name" > /dev/null
    return $?
}

# Function to prompt user with yes/no question
prompt_yes_no() {
    local prompt=$1
    while true; do
        read -p "$prompt (y/n): " response
        case $response in
            [Yy]* ) return 0;;
            [Nn]* ) return 1;;
            * ) echo "Please answer y or n";;
        esac
    done
}

if [ ! -w $DEV_BINDER ]; then
    echo "No permission to access $DEV_BINDER, consider execute this as root"
    exit 1
fi

export LD_LIBRARY_PATH=$RUN_DIR:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH=/usr/lib:/lib:/home/root:./

# Check and start servicemanager (must run if not running)
if check_process "servicemanager"; then
    echo "servicemanager is already running"
    servicemanager_pid=$(pgrep -f "servicemanager")
else
    echo "Execute [servicemanager]"
    ${RUN_DIR}/servicemanager &
    servicemanager_pid=$!
    # Wait for service to be ready
    sleep 2
fi

# Always restart mockservice to avoid stale process state/binary reuse.
if check_process "MockAudioSinkService"; then
    echo "Restarting existing MockAudioSinkService"
    pkill -f "MockAudioSinkService"
    sleep 1
fi

# A running platform AudioSinkService can mask the mock vdevice behavior.
if check_process "AudioSinkService"; then
    echo "Stopping existing AudioSinkService so VTS uses MockAudioSinkService"
    pkill -f "AudioSinkService"
    sleep 1
fi

echo "Execute [MockAudioSinkService]"
${RUN_DIR}/MockAudioSinkService &
AudioSinkService_pid=$!
# Wait for service to be ready
sleep 2

echo "Eexecute [vts_audiosink_hal_test]"
${RUN_DIR}/vts_audiosink_hal_test $@
