#!/usr/bin/env python3
# -*- coding: utf-8 -*-

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

from pathlib import Path
import subprocess
import os
import pytest

# Tests folder path
TESTS_PATH = os.path.dirname(os.path.abspath(__file__))
# Shell script path
SCRIPT_DIR = os.path.abspath(os.path.join(TESTS_PATH, "../src/vmb/resources"))
SCRIPT_PATH = os.path.join(SCRIPT_DIR, "cpumask.sh")
BACKUP_PATH = os.path.join(SCRIPT_DIR, "cpu_settings_backup.csv")


def prepare_cpu_common_test_data(online: int, f_min: int, f_max: int, count: int, test_dir: str) -> None:
    for i in range(count):
        # Create CPU directory
        cpu_path = f"{test_dir}/cpu{i}"
        os.makedirs(cpu_path, exist_ok=True)
        os.makedirs(f"{cpu_path}/cpufreq", exist_ok=True)

        # Create test CPU files with predefined content
        files = {
            "online": str(online),
            "cpufreq/scaling_min_freq": str(f_min),
            "cpufreq/scaling_max_freq": str(f_max),
            "cpufreq/scaling_governor": "X"
        }

        for filename, content in files.items():
            with open(os.path.join(cpu_path, filename), "w") as f:
                f.write(content)


@pytest.fixture(scope="function")
def setup_test_files(tmp_path: Path):
    """Prepare test data structure for cpumask tests"""
    freq_list = [
        "51, 20, 30, 40, 10",
        "10 ,  52,  30   , 40 , 20",
        "10  20   53   40 30",
        "10 20 , 30 ,54, 40",
        "55 55",
        "56",
        "10 57",
        "58, ,40",
        "10 ,59, 30 59 40",
        "10 20 30 40 60",
        "61 20 30 40, 61",
        "1210000 1325000 1440000 1594000 1748000 1882000 1997000 2112000 2228000 2304000 2400000 2500000"
    ]

    # Create test directory
    test_dir = os.path.join(tmp_path, "cpumask")
    os.makedirs(test_dir, exist_ok=True)

    count = len(freq_list)
    prepare_cpu_common_test_data(1, 20, 40, count, test_dir)

    for i in range(count):
        file_path = f"{test_dir}/cpu{i}/cpufreq/scaling_available_frequencies"
        with open(file_path, "w") as f:
            f.write(freq_list[i])

    yield test_dir

    if Path(BACKUP_PATH).exists():
        os.remove(BACKUP_PATH)


@pytest.fixture(scope="function")
def setup_test_files_mini(tmp_path: Path):
    """Prepare redused test data structure for cpumask tests"""
    freq_list = ["10,20,30", "40,50,60", "70,80,90"]

    # Create test directory
    test_dir = os.path.join(tmp_path, "test")
    os.makedirs(test_dir, exist_ok=True)

    count = len(freq_list)
    prepare_cpu_common_test_data(1, 10, 90, count, test_dir)

    for i in range(count):
        file_path = f"{test_dir}/cpu{i}/cpufreq/scaling_available_frequencies"
        with open(file_path, "w") as f:
            f.write(freq_list[i])

    yield test_dir

    if Path(BACKUP_PATH).exists():
        os.remove(BACKUP_PATH)


def check_file_content(cpu_path: str, file_name: str, value: str):
    file_path = os.path.join(cpu_path, file_name)
    with open(file_path, "r") as f:
        content = f.read().strip()
        assert content == value, f"{file_name}: expected {value}, but {content} found"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
@pytest.mark.parametrize('mask, online', [
    ("0x1", [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
    ("0X2", [0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
    ("0x4", [0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0]),
    ("0x8", [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]),
    ("0X10", [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0]),
    ("0x20", [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0]),
    ("0x40", [0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0]),
    ("0x80", [0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0]),
    ("0x100", [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0]),
    ("0x200", [0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0]),
    ("0x400", [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0]),
    ("0X800", [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]),
    ("0x801", [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1]),
    ("0xFfF", [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]),
])
def test_cpumask_script_apply_mask(setup_test_files: str, mask: str, online: list[int]):
    test_dir = setup_test_files
    freq = [51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 2500000]

    subprocess.run(["sh", SCRIPT_PATH, "--apply-mask", mask, "--cpu-path", test_dir], capture_output=True, check=True)

    for i in range(len(online)):
        path = os.path.join(test_dir, f"cpu{i}")
        check_file_content(path, "online", str(online[i]))
        check_file_content(path, "cpufreq/scaling_min_freq", str(freq[i]))
        check_file_content(path, "cpufreq/scaling_max_freq", str(freq[i]))
        check_file_content(path, "cpufreq/scaling_governor", "performance")


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
@pytest.mark.parametrize('freq, freq_list', [
    ("100%", [51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 2500000]),
    ("90%",  [40, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 2228000]),
    ("75%",  [40, 40, 40, 40, 55, 56, 57, 40, 40, 40, 40, 1882000]),
    ("50%",  [30, 30, 30, 30, 55, 56, 10, 40, 30, 30, 30, 1210000]),
    ("30%",  [20, 20, 20, 20, 55, 56, 10, 40, 10, 20, 20, 1210000]),
    ("10%",  [10, 10, 10, 10, 55, 56, 10, 40, 10, 10, 20, 1210000]),
    ("0%",   [10, 10, 10, 10, 55, 56, 10, 40, 10, 10, 20, 1210000]),
    ("0",    [10, 10, 10, 10, 55, 56, 10, 40, 10, 10, 20, 1210000]),
    ("20",   [20, 20, 20, 20, 55, 56, 10, 40, 30, 20, 20, 1210000]),
    ("25",   [30, 30, 30, 30, 55, 56, 10, 40, 30, 30, 30, 1210000]),
    ("54",   [51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 1210000]),
    ("2250000", [51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 2228000]),
])
def test_cpumask_script_frequency(setup_test_files: str, freq: str, freq_list: list[int]):
    test_dir = setup_test_files
    mask = "0x112"
    online = [0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0]

    subprocess.run(["sh", SCRIPT_PATH, "--apply-mask", mask, "-f", freq, "--cpu-path", test_dir], capture_output=True, check=True)

    for i in range(len(online)):
        path = os.path.join(test_dir, f"cpu{i}")
        check_file_content(path, "online", str(online[i]))
        check_file_content(path, "cpufreq/scaling_min_freq", str(freq_list[i]))
        check_file_content(path, "cpufreq/scaling_max_freq", str(freq_list[i]))
        check_file_content(path, "cpufreq/scaling_governor", "performance")


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
@pytest.mark.parametrize('mask, count', [
    ("0x8", 4),
    ("0xf", 4),
    ("0x41", 7)
])
def test_cpumask_script_long_set_list_error(setup_test_files_mini: str, mask: str, count: int):
    test_dir = setup_test_files_mini
    expected_error = f"ERROR: cpumask exeeds CPU amount: {count} settings for 3 CPUs (mask: '{mask}')."

    res = subprocess.run(["sh", SCRIPT_PATH, "--apply-mask", mask, "--cpu-path", test_dir], capture_output=True, check=False)

    assert res.returncode > 0
    err = res.stderr.decode().strip()
    assert err == expected_error, f"Expected output '{expected_error}' but '{err}' received"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
@pytest.mark.parametrize('mask', [
    ("0x0"),
    ("0x000")
])
def test_cpumask_script_zero_mask(setup_test_files_mini: str, mask: str):
    test_dir = setup_test_files_mini
    expected_error = f"ERROR: Invalid cpumask value '{mask}' - all CPUs are OFF."

    res = subprocess.run(["sh", SCRIPT_PATH, "--apply-mask", mask, "--cpu-path", test_dir], capture_output=True, check=False)

    assert res.returncode > 0
    err = res.stderr.decode().strip()
    assert err == expected_error, f"Expected output '{expected_error}' but '{err}' received"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
@pytest.mark.parametrize('backup, expected', [
    (
        "cpu0,1,2,3,a\ncpu1,0,10,200,b\ncpu2,1,50,50,c",
        [['1', '2', '3', 'a'], ['0', '10', '200', 'b'], ['1', '50', '50', 'c']]
    ),
    (
        "cpu0,1,2,3,a\ncpu2,1,50,50,c\ncpu1,0,10,200,b",
        [['1', '2', '3', 'a'], ['0', '10', '200', 'b'], ['1', '50', '50', 'c']]
    ),
    (
        "cpu2,1,50,50,c\ncpu0,1,2,3,a\ncpu1,0,10,200,b",
        [['1', '2', '3', 'a'], ['0', '10', '200', 'b'], ['1', '50', '50', 'c']]
    ),
    (
        "cpu2,1,50,50,c\ncpu1,0,10,200,b\ncpu0,1,2,3,a",
        [['1', '2', '3', 'a'], ['0', '10', '200', 'b'], ['1', '50', '50', 'c']]
    ),
    (
        "cpu0,1,2,3,a",
        [['1', '2', '3', 'a'], ['1', '10', '90', 'X'], ['1', '10', '90', 'X']]
    ),
    (
        "cpu1,0,10,200,b",
        [['1', '10', '90', 'X'], ['0', '10', '200', 'b'], ['1', '10', '90', 'X']]
    ),
    (
        "cpu2,1,50,50,c",
        [['1', '10', '90', 'X'], ['1', '10', '90', 'X'], ['1', '50', '50', 'c']]
    ),
    (
        "cpu0,1,2,3,a\ncpu2,1,50,50,c",
        [['1', '2', '3', 'a'], ['1', '10', '90', 'X'], ['1', '50', '50', 'c']]
    ),
    (
        "",
        [['1', '10', '90', 'X'], ['1', '10', '90', 'X'], ['1', '10', '90', 'X']]
    ),
    (
        "cpu0,1,2,3,a\ncpu1,0,10,200,b\ncpu5,1,50,50,c",
        [['1', '2', '3', 'a'], ['0', '10', '200', 'b'], ['1', '10', '90', 'X']]
    )
])
def test_cpumask_script_restore_cpu(setup_test_files_mini: str, backup: str, expected: list[list[str]]):
    test_dir = setup_test_files_mini
    backup_file = f"{test_dir}/backup.csv"
    count = len(expected)

    with open(backup_file, "w") as f:
        f.write(f"{backup}\n")

    subprocess.run(["sh", SCRIPT_PATH, "--restore", "--cpu-path", test_dir, "-b", backup_file], capture_output=True, check=True)

    for i in range(count):
        path = os.path.join(test_dir, f"cpu{i}")
        check_file_content(path, "online", expected[i][0])
        check_file_content(path, "cpufreq/scaling_min_freq", expected[i][1])
        check_file_content(path, "cpufreq/scaling_max_freq", expected[i][2])
        check_file_content(path, "cpufreq/scaling_governor", expected[i][3])

    assert not Path(backup_file).exists(), f"Backup file was not remove after revert: '{backup_file}'"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
@pytest.mark.parametrize('settings, expected', [
    (
        'cpu2,1,10,90,X\ncpu1,1,10,90,X\ncpu0,1,10,90,X',
        'INFO: cpu0: OK\nINFO: cpu1: OK\nINFO: cpu2: OK'
    ),
    (
        'cpu2,1,10,90,X\ncpu1,1,10,90,A\ncpu0,1,10,90,X',
        'INFO: cpu0: OK\nWARNING: cpu1: governor: X != A\nINFO: cpu2: OK'
    ),
    (
        'cpu2,1,10,90,X\ncpu1,1,10,90,X\ncpu0,1,30,90,X',
        'WARNING: cpu0: min_freq: 10 != 30\nINFO: cpu1: OK\nINFO: cpu2: OK'
    ),
    (
        'cpu2,1,30,90,X\ncpu1,1,10,90,X\ncpu0,1,10,90,X',
        'INFO: cpu0: OK\nINFO: cpu1: OK\nWARNING: cpu2: min_freq: 10 != 30'
    ),
    (
        'cpu2,0,10,90,X\ncpu1,1,10,90,X\ncpu0,1,10,90,X',
        'INFO: cpu0: OK\nINFO: cpu1: OK\nWARNING: cpu2: state: 1 != 0'
    ),
    (
        'cpu2,1,10,90,X\ncpu1,1,10,50,X\ncpu0,1,10,90,X',
        'INFO: cpu0: OK\nWARNING: cpu1: max_freq: 90 != 50\nINFO: cpu2: OK'
    ),
    (
        'cpu2,1,10,90,X\ncpu1,0,10,15,b\ncpu0,1,10,90,X',
        'INFO: cpu0: OK\nWARNING: cpu1: state: 1 != 0; governor: X != b; max_freq: 90 != 15\nINFO: cpu2: OK'
    )
])
def test_cpumask_script_check_cpu_settings(setup_test_files_mini: str, settings: str, expected: str):
    test_dir = setup_test_files_mini
    settings_file = f"{SCRIPT_DIR}/cpu_settings.csv"
    expected = f'INFO: Check CPU actual settings:\n{expected}'

    with open(settings_file, "w") as f:
        f.write(f"{settings}\n")

    res = subprocess.run(["sh", SCRIPT_PATH, "-c", "--cpu-path", test_dir], capture_output=True, check=True)

    output = res.stdout.decode().strip()
    assert output == expected, f"Expected output '{expected}' but '{output}' received"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
@pytest.mark.parametrize('mask, online', [
    ("0x008", [0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0]),
    ("0xfff", [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1])
])
def test_cpumask_script_set_and_restore_cpu(setup_test_files: str, mask: str, online: list[int]):
    test_dir = setup_test_files
    freq = [51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 2500000]
    count = len(online)

    subprocess.run(["sh", SCRIPT_PATH, "--apply-mask", mask, "--cpu-path", test_dir], capture_output=True, check=True)

    for i in range(count):
        path = os.path.join(test_dir, f"cpu{i}")
        check_file_content(path, "online", str(online[i]))
        check_file_content(path, "cpufreq/scaling_min_freq", str(freq[i]))
        check_file_content(path, "cpufreq/scaling_max_freq", str(freq[i]))
        check_file_content(path, "cpufreq/scaling_governor", "performance")

    assert Path(BACKUP_PATH).exists(), f"No backup file '{BACKUP_PATH}'"

    subprocess.run(["sh", SCRIPT_PATH, "--restore", "--cpu-path", test_dir], capture_output=True, check=True)

    for i in range(count):
        path = os.path.join(test_dir, f"cpu{i}")
        check_file_content(path, "online", "1")
        check_file_content(path, "cpufreq/scaling_min_freq", "20")
        check_file_content(path, "cpufreq/scaling_max_freq", "40")
        check_file_content(path, "cpufreq/scaling_governor", "X")

    assert not Path(BACKUP_PATH).exists(), f"Backup file was not removed after restore: '{BACKUP_PATH}'"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
@pytest.mark.parametrize('arg, expected_error', [
    ("-b", "ERROR: Argument required for '-b'."),
    ("-p", "ERROR: Argument required for '-p'."),
    ("-f", "ERROR: Argument required for '-f'."),
    ("-m", "ERROR: Argument required for '-m'."),
    ("--backup-file", "ERROR: Argument required for '--backup-file'."),
    ("--cpu-path", "ERROR: Argument required for '--cpu-path'."),
    ("--frequency", "ERROR: Argument required for '--frequency'."),
    ("--apply-mask", "ERROR: Argument required for '--apply-mask'."),
    ("-backup-file", "ERROR: Unknown option: '-backup-file'."),
    ("-cpu-path", "ERROR: Unknown option: '-cpu-path'."),
    ("-e", "ERROR: Unknown option: '-e'."),
    ("-", "ERROR: Unknown option: '-'."),
    ("--", "ERROR: Unknown option: '--'."),
    ("*", "ERROR: No positional arguments expected '*'."),
    ("m", "ERROR: No positional arguments expected 'm'."),
    ("/f", "ERROR: No positional arguments expected '/f'."),
    ("", "ERROR: No positional arguments expected ''.")
])
def test_cpumask_script_wrong_args(arg: str, expected_error: str):
    res = subprocess.run(["sh", SCRIPT_PATH, arg], capture_output=True, check=False)
    err = res.stderr.decode().strip()

    assert res.returncode > 0
    assert err == expected_error, f"Expected output '{expected_error}' but '{err}' received"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
def test_cpumask_script_no_parameters():
    expected_error = "ERROR: No arguments! Run 'sh cpumask.sh --help' to get help."
    res = subprocess.run(["sh", SCRIPT_PATH], capture_output=True, check=False)

    err = res.stderr.decode().strip()
    assert res.returncode > 0
    assert err == expected_error, f"Expected output '{expected_error}' but '{err}' received"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
def test_cpumask_script_no_action():
    expected_error = "ERROR: No action was set. Run 'sh cpumask.sh --help' to get help."
    res = subprocess.run(["sh", SCRIPT_PATH, "-b", "temp.csv"], capture_output=True, check=False)

    err = res.stderr.decode().strip()
    assert res.returncode > 0
    assert err == expected_error, f"Expected output '{expected_error}' but '{err}' received"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
def test_cpumask_script_non_existent_cpu_path():
    path = "/anywhere/elsewhere"
    expected_error = f"ERROR: CPU path '{path}' does not exist."
    res = subprocess.run(["sh", SCRIPT_PATH, "-p", path], capture_output=True, check=False)

    err = res.stderr.decode().strip()
    assert res.returncode > 0
    assert err == expected_error, f"Expected output '{expected_error}' but '{err}' received"


@pytest.mark.skipif(os.name != 'posix', reason="POSIX system is required for test execution.")
def test_cpumask_script_wrong_cpu_path(tmp_path):
    expected_error = f"ERROR: No CPU directory in '{tmp_path}' path."
    res = subprocess.run(["sh", SCRIPT_PATH, "-p", tmp_path], capture_output=True, check=False)

    err = res.stderr.decode().strip()
    assert res.returncode > 0
    assert err == expected_error, f"Expected output '{expected_error}' but '{err}' received"
