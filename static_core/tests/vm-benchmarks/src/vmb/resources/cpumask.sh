#!/bin/sh

# Copyright (c) 2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Script constanst
SCRIPT_DIR=$(dirname "$0")
SCRIPT_NAME=$(basename "$0")
SCRIPT_VERSION="1.1.0"

# Paths presets
CPU_PATH="/sys/devices/system/cpu"
STATE_PATH="online"
FREQ_AVAILABLE_PATH="cpufreq/scaling_available_frequencies"
CUR_FREQ_PATH="cpufreq/cpuinfo_cur_freq"
MIN_FREQ_PATH="cpufreq/scaling_min_freq"
MAX_FREQ_PATH="cpufreq/scaling_max_freq"
SCALING_GOVERNOR="cpufreq/scaling_governor"
BACKUP_FILE="cpu_settings_backup.csv"
SETTINGS_FILE="cpu_settings.csv"
SETTINGS_PATH="$SCRIPT_DIR/$SETTINGS_FILE"


# Script variables presets
ACTION=""
CPU_COUNT=""
CPU_FREQ="100%"
CPU_FREQ_LIST=""
CPU_STATES=""
MASK=""

check_value_parameter() {
    if [ -z "$2" ] || [ "$(echo "$2" | cut -c1)" = "-" ]; then
        echo "ERROR: Argument required for '$1'." >&2
        exit 1
    fi
}

hex_char_to_bits() {
    val="$1"
    bits=$(( (val >> 0) & 1 ))
    for j in 1 2 3; do
        bit=$(( (val >> j) & 1 ))
        bits="$bits$bit"
    done
    echo "$bits"
}

parse_cpumask() {
    case "$MASK" in
        0x*|0X*)
            hex_number="${MASK#??}"
            ;;
        *)
            echo "ERROR: Invalid cpumask value '$MASK' - must start with '0x'." >&2
            exit 1
            ;;
    esac

    array=""
    for i in $(seq 1 ${#hex_number}); do
        char=$(echo "$hex_number" | cut -c"$i")
        case "$char" in
            a|A) char=10 ;;
            b|B) char=11 ;;
            c|C) char=12 ;;
            d|D) char=13 ;;
            e|E) char=14 ;;
            f|F) char=15 ;;
        esac
        array="$(hex_char_to_bits "$char")$array"
    done

    case "$array" in
        *1*)
            CPU_STATES="${array%1*}1"
            ;;
        *)
            echo "ERROR: Invalid cpumask value '$MASK' - all CPUs are OFF." >&2
            exit 1
            ;;
    esac
}

print_cpu_state() {
    i=0
    while [ "$i" -lt "$CPU_COUNT" ]; do
        cpu="cpu$i"
        path="$CPU_PATH/$cpu"
        state=$(cat "$path/$STATE_PATH")
        cur_freq=$(cat "$path/$CUR_FREQ_PATH")
        min_freq=$(cat "$path/$MIN_FREQ_PATH")
        max_freq=$(cat "$path/$MAX_FREQ_PATH")
        governor=$(cat "$path/$SCALING_GOVERNOR")
        if [ "$state" -eq 1 ]; then
            state=" ON"
        else
            state="OFF"
        fi
        echo "INFO: $cpu is $state: Freq=$cur_freq; MinFreq=$min_freq; MaxFreq = $max_freq; governor = $governor"
        i=$((i+1))
    done
}

check_cpu_settings() {
    file_path=$1
    result=""
    while IFS=',' read -r cpu state min_freq max_freq governor; do
        error=""
        path="$CPU_PATH/$cpu"
        cpu_state=$(cat "$path/$STATE_PATH")
        cpu_min_freq=$(cat "$path/$MIN_FREQ_PATH")
        cpu_max_freq=$(cat "$path/$MAX_FREQ_PATH")
        cpu_governor=$(cat "$path/$SCALING_GOVERNOR")

        if [ -d "$path" ]; then
            if [ "$state" -ne "$cpu_state" ]; then
                error="$error; state: $cpu_state != $state"
            fi
            if [ "$governor" != "$cpu_governor" ]; then
                error="$error; governor: $cpu_governor != $governor"
            fi
            if [ "$min_freq" -ne "$cpu_min_freq" ]; then
                error="$error; min_freq: $cpu_min_freq != $min_freq"
            fi
            if [ "$max_freq" -ne "$cpu_max_freq" ]; then
                error="$error; max_freq: $cpu_max_freq != $max_freq"
            fi
        else
            error="CPU path was not found: '$path'."
        fi

        if [ "$error" = "" ]; then
            result="INFO: $cpu: OK\n$result"
        else
            result="WARNING: $cpu: ${error#; }\n$result"
        fi
    done < "$file_path"
    echo "$result"
}

verify_cpu_settings() {
    if [ -f "$SETTINGS_PATH" ]; then
        echo "INFO: Check CPU actual settings:"
        check_cpu_settings "$SETTINGS_PATH"
        rm "$SETTINGS_PATH"
    else
        echo "INFO: CPU settings before restoring:"
        print_cpu_state
    fi
}

find_nearest_frequency() {
    frequencies="$1"
    target="$2"
    nearest=""
    min_diff=""
    for num in $frequencies; do
        diff=$(( num > target ? num - target : target - num ))
        if [ -z "$min_diff" ] || [ "$diff" -lt "$min_diff" ] || { [ "$diff" -eq "$min_diff" ] && [ "$num" -gt "$nearest" ]; }; then
            nearest="$num"
            min_diff="$diff"
        fi
    done
    echo "$nearest"
}

get_max_from_list() {
    list="$1"
    max=-1
    for num in $list; do
        if [ "$num" -gt "$max" ]; then
            max=$num
        fi
    done
    echo "$max"
}

prepare_cpu_freq_list() {
    i=0
    while [ "$i" -lt "$CPU_COUNT" ]; do
        cpu_frequency=""
        file="$CPU_PATH/cpu$i/$FREQ_AVAILABLE_PATH"
        frequency_list=$(sed 's/,/ /g; s/  +/ /g' "$file")
        case "$CPU_FREQ" in
            *%)
                max_frequency=$(get_max_from_list "$frequency_list")
                ratio="${CPU_FREQ%?}"
                target_frequency=$((max_frequency*$ratio/100 ))
                ;;
            *)
                target_frequency="$CPU_FREQ"
                ;;
        esac
        cpu_frequency=$(find_nearest_frequency "$frequency_list" "$target_frequency")
        if [ -z "$CPU_FREQ_LIST" ]; then
            CPU_FREQ_LIST="$cpu_frequency"
        else
            CPU_FREQ_LIST="$CPU_FREQ_LIST $cpu_frequency"
        fi
        i=$((i+1))
    done
}

apply_cpu_settings() {
    file_path=$1
    while IFS=',' read -r cpu state min_freq max_freq governor; do
        path="$CPU_PATH/$cpu"
        if [ -d "$path" ]; then
            echo "$governor" > "$path/$SCALING_GOVERNOR"
            echo "$state" > "$path/$STATE_PATH"
            echo "$min_freq" > "$path/$MIN_FREQ_PATH"
            echo "$max_freq" > "$path/$MAX_FREQ_PATH"
        else
            echo "WARNING: The CPU path was not found: '$path'."
        fi
    done < "$file_path"
}

if [ $# -eq 0 ]; then
    echo "ERROR: No arguments! Run 'sh $SCRIPT_NAME --help' to get help." >&2
    exit 1
fi

while [ $# -gt 0 ]; do
    case "$1" in
        # Options without arguments
        -r|--restore)
            ACTION="restore"
            shift
            ;;
        # Options with mandatory arguments
        -b|--backup-file)
            check_value_parameter "$1" "$2"
            BACKUP_FILE="$2"
            shift 2
            ;;
        -p|--cpu-path)
            check_value_parameter "$1" "$2"
            CPU_PATH="$2"
            shift 2
            ;;
        -c|--check)
            ACTION="check"
            shift
            ;;
        -f|--frequency)
            check_value_parameter "$1" "$2"
            CPU_FREQ="$2"
            shift 2
            ;;
        -m|--apply-mask)
            check_value_parameter "$1" "$2"
            ACTION="apply-mask"
            MASK=$2
            shift 2
            ;;
        # Special option
        -v|--version)
            echo "$SCRIPT_NAME $SCRIPT_VERSION"
            exit 0
            ;;
        -h|--help)
            echo "$SCRIPT_NAME v$SCRIPT_VERSION"
            echo
            echo "Script to turn on and off CPUs, set CPUs frequencies and restore previous settings."
            echo "CPU frequency is set for enabled CPUs. The frequency can be set in Hz or as ratio."
            echo
            echo "Usage: $SCRIPT_NAME [-v | --version] [-h | --help] <action> [optional_parameters]"
            echo
            echo "Actions:"
            echo "  -m|--apply-mask <mask> - turn on and off CPUs according to the mask; mask is a hex or binary number"
            echo "  -r|--restore           - Restore settings from backup file"
            echo "  -c|--check             - Compare actual CPU settings with $SETTINGS_FILE)"
            echo "  -h|--help              - Show this help"
            echo "  -v|--version           - Show script version"
            echo
            echo "Optional parameters:"
            echo "  -b|--backup-file <name> - set backup file name or full path (default: '$BACKUP_FILE')"
            echo "  -p|--cpu-path <path>    - set full path to CPUs directory (default: '$CPU_PATH')"
            echo "  -f|--frequency <freq>   - set cpu frequency or ratio (integer percents with % sign). Default value: $CPU_FREQ"
            echo
            echo "Examples:"
            echo "    $SCRIPT_NAME --set true,false,true,false"
            echo "    $SCRIPT_NAME -s true,false,false,false,false --backup-file my_backup.csv"
            echo "    $SCRIPT_NAME -r --cpu-path /sys/devices/system/cpu -b saved_state.csv"
            echo "    $SCRIPT_NAME --restore"
            exit 0
            ;;
        -*)
            echo "ERROR: Unknown option: '$1'." >&2
            exit 1
            ;;
        *)
            echo "ERROR: No positional arguments expected '$1'." >&2
            exit 1
            ;;
    esac
done

# Check if CPU directory exists
if [ ! -d "$CPU_PATH" ]; then
    echo "ERROR: CPU path '$CPU_PATH' does not exist." >&2
    exit 1
fi

# Check CPUs list path
CPU_PATH_LIST=$(ls -1 -d "$CPU_PATH"/cpu[0-9]* 2>/dev/null)
if [ "$CPU_PATH_LIST" = "" ]; then
    echo "ERROR: No CPU directory in '$CPU_PATH' path." >&2
    exit 1
fi
CPU_COUNT=$(echo "$CPU_PATH_LIST" | wc -l)

# Resolve backup file path
bf_path=$(dirname "$BACKUP_FILE")
if [ "$bf_path" = "." ]; then
    backup_path="$SCRIPT_DIR/$BACKUP_FILE"
elif [ -d "$bf_path" ]; then
    backup_path=$BACKUP_FILE
else
    name=$(basename "$BACKUP_FILE")
    backup_path="$SCRIPT_DIR/$name"
    echo "WARNING: Unreachable path '$bf_path' detected for backup file. Script path will be used." >&2
fi

# Change CPUs states
case "$ACTION" in
    # Set CPUs new states
    "apply-mask")
        parse_cpumask

        # Check input CPU settings
        cpu_states_count=${#CPU_STATES}
        if [ "$CPU_COUNT" -lt "$cpu_states_count" ]; then
            echo "ERROR: cpumask exeeds CPU amount: $cpu_states_count settings for $CPU_COUNT CPUs (mask: '$MASK')." >&2
            exit 1
        fi

        # Fill remaining CPU states with zeros
        if [ "$CPU_COUNT" -gt "$cpu_states_count" ]; then
            while [ "$CPU_COUNT" -gt "$cpu_states_count" ]; do
                CPU_STATES="${CPU_STATES}0"
                cpu_states_count=${#CPU_STATES}
            done
        fi

        # Remove old backup file if exists
        if [ -f "$backup_path" ]; then
            rm "$backup_path"
        fi

        # Save CPU settings to the backup file
        for path in $CPU_PATH_LIST; do
            cpu=$(basename "$path")
            state=$(cat "$path/$STATE_PATH")
            min_freq=$(cat "$path/$MIN_FREQ_PATH")
            max_freq=$(cat "$path/$MAX_FREQ_PATH")
            governor=$(cat "$path/$SCALING_GOVERNOR")
            echo "$cpu,$state,$min_freq,$max_freq,$governor" >> "$backup_path"
        done
        echo "Current CPU settings saved to '$backup_path'."

        # Remove old settings file if exists
        if [ -f "$SETTINGS_PATH" ]; then
            rm "$SETTINGS_PATH"
        fi

        # Prepare CPU frequency settings file
        prepare_cpu_freq_list
        i=$CPU_COUNT
        while [ "$i" -gt 0 ]; do
            state=$(echo "$CPU_STATES" | cut -c "$i")
            freq=$(echo "$CPU_FREQ_LIST" | cut -d " " -f "$i")
            i=$((i-1))
            echo "cpu$i,$state,$freq,$freq,performance" >> "$SETTINGS_PATH"
        done
        echo "INFO: CPU settings file prepared: '$SETTINGS_PATH'"

        # Set CPUs states and frequencies
        apply_cpu_settings "$SETTINGS_PATH"
        echo "INFO: CPU settings applied."
        ;;
    "check")
        verify_cpu_settings
        exit 0
        ;;
    "restore")
        # Print current cpu state
        verify_cpu_settings

        # Check if backup file exists
        if [ ! -f "$backup_path" ]; then
            echo "WARNING: The CPU backup settings file was not found: '$backup_path'."
            exit 0
        fi

        # Restore CPU settings
        apply_cpu_settings "$backup_path"
        rm "$backup_path"

        echo "Initial CPU settings restored from '$backup_path'."
        ;;
    *)
        echo "ERROR: No action was set. Run 'sh $SCRIPT_NAME --help' to get help." >&2
        exit 1
        ;;
esac

# Print CPU settings
echo "INFO: CPU settings:"
print_cpu_state

exit 0
