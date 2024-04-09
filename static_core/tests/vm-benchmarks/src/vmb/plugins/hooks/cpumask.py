#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2024 Huawei Device Co., Ltd.
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
from typing import Dict

from vmb.platform import PlatformBase
from vmb.cli import Args
from vmb.helpers import remove_prefix
from vmb.hook import HookBase

log = logging.getLogger('vmb')

all_props = (
    'cpufreq/scaling_min_freq',
    'cpufreq/scaling_max_freq',
    'cpufreq/scaling_governor',
    'online'
)


class Hook(HookBase):
    """Hook cpumask. Set cpu online/offline and freqs."""

    cpu_root = '/sys/devices/system/cpu'

    def __init__(self, args: Args) -> None:
        super().__init__(args)
        self.saved_state: Dict[int, Dict[str, str]] = {}
        mask = args.get('cpumask', '')
        self.cpumask = list(map(lambda x: x == '1', mask))
        if not any(self.cpumask):
            raise ValueError(f'Wrong cpumask: {mask}')

    @classmethod
    def skipme(cls, args: Args) -> bool:
        """Do not register me on host."""
        return (
            args.platform.endswith('host') or
            not args.get('cpumask'))

    @property
    def name(self) -> str:
        return 'CPU mask'

    def before_suite(self, platform: PlatformBase) -> None:
        def get_prop(cpu: int, prop: str) -> str:
            f = f'{self.cpu_root}/cpu{cpu}/{prop}'
            # return first 1234 or blah, avoiding __RET_VAL__
            return platform.x_sh.grep_output(f'cat {f}', r'^[\s\w]+$')

        def set_prop(cpu: int, prop: str, val: str) -> None:
            f = f'{self.cpu_root}/cpu{cpu}/{prop}'
            platform.x_sh.run(f'echo {val} > {f}')

        r = platform.x_sh.run(
            f'ls -1 -d {os.path.join(self.cpu_root, "cpu[0-9]*")}')
        if not r.out:
            raise RuntimeError('Get cpus failed!')
        all_cores = sorted([
            int(remove_prefix(os.path.split(c)[1], 'cpu'))
            for c in r.out.split("\n") if self.cpu_root in c])
        log.debug(all_cores)
        if len(all_cores) > len(self.cpumask):
            log.warning('Wrong cpumask length for '
                        '%d cores: %s', len(all_cores), self.cpumask)
            self.cpumask += [False] * (len(all_cores) - len(self.cpumask))
        # save initial cpu settings
        max_freqs = {}
        self.saved_state = {cpu: {} for cpu in all_cores}
        for c in all_cores:
            max_freqs[c] = get_prop(c, 'cpufreq/cpuinfo_max_freq')
            for p in all_props:
                self.saved_state[c][p] = get_prop(c, p)
        log.debug(max_freqs)
        log.debug(self.saved_state)
        # update cpu settings
        on_cores = [index for index, on in enumerate(self.cpumask) if on]
        off_cores = [index for index, on in enumerate(self.cpumask) if not on]
        for c in on_cores:
            set_prop(c, 'online', '1')
            set_prop(c, 'cpufreq/scaling_governor', 'performance')
            # set max freq
            avail = get_prop(c, 'cpufreq/scaling_available_frequencies')
            f_avail = [int(x) for x in avail.split(' ') if x]
            if not f_avail:
                raise RuntimeError('Failed to get available freqs')
            f_avail.sort()
            f_max = str(f_avail[-1])
            set_prop(c, 'cpufreq/scaling_min_freq', f_max)
            set_prop(c, 'cpufreq/scaling_max_freq', f_max)
            # why we do this? isn't it the same as above?
            platform.x_sh.run(
                f'cat {self.cpu_root}/cpu{c}/cpufreq/scaling_max_freq > '
                f'{self.cpu_root}/cpu{c}/cpufreq/scaling_min_freq')
        for c in off_cores:
            set_prop(c, 'online', '0')
        # actually defined in base class
        self.done = True  # pylint: disable=attribute-defined-outside-init

    def after_suite(self, platform: PlatformBase) -> None:
        def set_prop(cpu: int, prop: str, val: str) -> None:
            f = f'{self.cpu_root}/cpu{cpu}/{prop}'
            platform.x_sh.run(f'echo {val} > {f}')

        if self.done and self.saved_state:
            # first back ALL cores online
            for c, _ in self.saved_state.items():
                set_prop(c, 'online', '1')
            # then restore all props (online is last)
            for c, state in self.saved_state.items():
                for p in all_props:
                    set_prop(c, p, state[p])
