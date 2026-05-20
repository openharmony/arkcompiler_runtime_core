#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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
#

import json
from pathlib import Path
from signal import SIGTERM
from typing import Final

import trio

from arkdb.compiler import Compiler, CompilerArguments, Options
from arkdb.debug_client import DebugLocator
from arkdb.runnable_module import ScriptFile
from arkdb.runtime import Runtime
from arkdb.source_meta import parse_source_meta


CODE: Final[
    str
] = """\
function main(): int {
    let value: int = 1; // #BP{0}
    value += 1;
    return 0;
}
"""

ARKTSCONFIG_MODULE_NAME: Final[str] = "debugger_paths_alias"
EXPECTED_DEBUG_URL: Final[str] = f"{ARKTSCONFIG_MODULE_NAME}.ets"


async def test_debugger_uses_arktsconfig_paths_key_as_source_file_url(
    tmp_path: Path,
    ark_compiler: Compiler,
    ark_runtime: Runtime,
    nursery: trio.Nursery,
    debug_locator: DebugLocator,
) -> None:
    source_file = tmp_path / "entry.ets"
    source_file.write_text(CODE)

    arktsconfig = tmp_path / "arktsconfig.paths.json"
    config = json.loads(ark_compiler.options.arktsconfig.read_text())
    compiler_options = config["compilerOptions"]
    compiler_options["baseUrl"] = str(ark_compiler.options.arktsconfig.parent.parent)
    compiler_options["rootDir"] = str(tmp_path)
    compiler_options["paths"][ARKTSCONFIG_MODULE_NAME] = [str(source_file)]
    arktsconfig.write_text(
        json.dumps(config)
    )

    panda_file = tmp_path / f"{ARKTSCONFIG_MODULE_NAME}.abc"
    Compiler(
        Options(
            app_path=ark_compiler.options.app_path,
            arktsconfig=arktsconfig,
        )
    ).compile(
        source_file=source_file,
        output_file=panda_file,
        cwd=tmp_path,
        arguments=CompilerArguments(),
    )
    script_file = ScriptFile(source_file, panda_file)
    breakpoint = parse_source_meta(CODE).breakpoints[0]

    async with ark_runtime.run(nursery, module=script_file) as process:
        async with debug_locator.connect(nursery) as client:
            await client.configure(nursery)
            await client.run_if_waiting_for_debugger()

            _, locations = await client.set_breakpoint_by_url(
                url=EXPECTED_DEBUG_URL,
                line_number=breakpoint.line_number,
            )
            assert len(locations) == 1

            paused = await client.resume_and_wait_for_paused()
            assert paused.call_frames[0].url == EXPECTED_DEBUG_URL

            process.terminate()
            with trio.CancelScope(deadline=1, shield=True):
                await process.wait()
    assert process.returncode == -SIGTERM
