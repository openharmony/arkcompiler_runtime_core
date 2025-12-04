#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#

import os
import logging
from pathlib import Path
from typing import Dict, Optional

from vmb.platform import PlatformBase
from vmb.cli import Args
from vmb.hook import HookBase

log = logging.getLogger('vmb')


def parse_cpumask(mask_s: Optional[str]) -> str:
    """Parse cpumask string. Returns hexadecimal string without '0x' prefix."""
    if not mask_s:
        raise ValueError(f'Invalid cpumask: "{mask_s}". Must be non-empty')

    mask = mask_s.lower()
    if not (mask.startswith('0x') or mask.startswith('0b')):
        raise ValueError(f'Invalid cpumask format: "{mask_s}". Must be hexidecimal or binary number')

    try:
        num = int(mask, 0)
    except ValueError as e:
        raise ValueError(f'Invalid cpumask format: "{mask_s}". Must be hexidecimal or binary number') from e

    if num == 0:
        raise ValueError(f'Invalid cpumask: "{mask_s}". Must be non-zero')
    return format(num, 'X')


class Hook(HookBase):
    """Hook cpumask. Set cpu online/offline and freqs."""

    cpu_root = '/sys/devices/system/cpu'

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.saved_state: Dict[int, Dict[str, str]] = {}

        # Get the path to the cpumask.sh script
        this_path = Path(os.path.realpath(os.path.dirname(__file__)))
        self.script_source_path = this_path.parent.parent.joinpath('resources', 'cpumask.sh')
        self.script_target_path: Path

        self.cpumask = parse_cpumask(args.get('cpumask'))
        self.frequency = args.get('cpufreq')

    @property
    def name(self) -> str:
        return 'CPU mask'

    @classmethod
    def skipme(cls, args: Args) -> bool:
        """Do not register me on host."""
        return (
            args.platform.endswith('host') or
            not args.get('cpumask'))

    def before_suite(self, platform: PlatformBase) -> None:
        # Push the script to the device
        result = platform.x_sh.push(self.script_source_path, platform.dev_dir)
        result.log_output_with_levels()

        # set CPUs
        self.script_target_path = platform.dev_dir.joinpath('cpumask.sh')
        command = f'sh {self.script_target_path} --apply-mask 0x{self.cpumask} --cpu-path {self.cpu_root}'
        if self.frequency:
            command += f' --frequency {self.frequency}'
        result = platform.x_sh.run(command)
        result.log_output_with_levels()

        platform.set_affinity(self.cpumask)
        # actually defined in base class
        self.done = True  # pylint: disable=attribute-defined-outside-init

    def after_suite(self, platform: PlatformBase) -> None:
        result = platform.x_sh.run(f'sh {self.script_target_path} --restore --cpu-path {self.cpu_root}')
        result.log_output_with_levels()
