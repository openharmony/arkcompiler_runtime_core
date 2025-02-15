#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2025 Huawei Device Co., Ltd.
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

from signal import SIGTERM
from typing import Final

import trio

from arkdb.compiler import StringCodeCompiler
from arkdb.debug_client import DebugLocator
from arkdb.extensions import debugger as ext_debugger
from arkdb.runtime import Runtime
from arkdb.source_meta import parse_source_meta
from arkdb.walker import BreakpointWalkerType


CODE: Final[
    str
] = """\
function main(): int {
    let a = 0;
    a += 1; // #BP{1}
    let b = 1;
    b += 1; // #BP{2}
    let c = a + b;
    c *= 2; // #BP{3}
    return 0;
}
"""


async def test_remove_breakpoint_by_url(
    breakpoint_walker: BreakpointWalkerType,
) -> None:
    async with breakpoint_walker(CODE) as walker:
        paused_step = await anext(walker)
        bps = paused_step.hit_breakpoints()
        assert len(bps) == 1
        assert bps[0].label == "1"

        # Remove all breakpoints
        await paused_step.client.remove_breakpoints_by_url(paused_step.data.call_frames[0].url)

        # Reset BP3
        assert bps[0].locations is not None
        assert len(bps[0].locations) == 1
        bp3_loc = bps[0].locations[0]
        bp3_loc.line_number += 4
        bp3_id, _ = await paused_step.client.set_breakpoint(bp3_loc)

        # Resume and pause on BP3
        paused = await paused_step.client.resume_and_wait_for_paused()
        assert bool(paused.hit_breakpoints)
        assert len(paused.hit_breakpoints) == 1
        assert paused.hit_breakpoints[0] == bp3_id


async def test_get_possible_and_set_breakpoint_by_url(
    code_compiler: StringCodeCompiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
) -> None:
    script_file = code_compiler.compile(CODE)
    meta = parse_source_meta(CODE)

    async with ark_runtime.run(nursery, module=script_file) as process:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            await client.run_if_waiting_for_debugger()

            meta_breakpoints_lines = set(bp.line_number for bp in meta.breakpoints)
            set_breakpoints = await client.get_possible_and_set_breakpoint_by_url(
                [
                    ext_debugger.UrlBreakpointRequest(line_number, url=str(script_file.source_file))
                    for line_number in meta_breakpoints_lines
                ]
            )
            # Decrease, as debugger returns 1-based line numbers
            assert meta_breakpoints_lines == set(bp.line_number - 1 for bp in set_breakpoints)

        process.terminate()
    assert process.returncode == -SIGTERM
