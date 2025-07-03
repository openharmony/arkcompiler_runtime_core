#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

import dataclasses
from contextlib import asynccontextmanager
from dataclasses import dataclass
from inspect import getfullargspec
from typing import Any, AsyncIterator, Callable, Dict, List, Literal, Optional, Tuple, Type, TypeAlias

import trio
import trio_cdp
from cdp import debugger, runtime, profiler

from arkdb.compiler import StringCodeCompiler
from arkdb.debug_connection import ArkConnection, Proxy, ScriptsCache, SourcesCache, connect_cdp
from arkdb.extensions import debugger as ext_debugger
from arkdb.extensions import profiler as ext_profiler

T: TypeAlias = trio_cdp.T


@dataclass
class DebuggerConfig:
    pause_on_exceptions_mode: Literal["none", "caught", "uncaught", "all"] = "none"


class DebuggerClient:
    def __init__(
        self,
        connection: ArkConnection,
        config: DebuggerConfig,
        debugger_id: runtime.UniqueDebuggerId,
        scripts: ScriptsCache,
        sources: SourcesCache,
        context: runtime.ExecutionContextDescription,
        code_compiler: StringCodeCompiler,
    ) -> None:
        self.connection = connection
        self.config = config
        self.debugger_id = debugger_id
        self.scripts = scripts
        self.sources = sources
        self.context = context
        self.code_compiler = code_compiler

    async def configure(self, nursery: trio.Nursery):
        self._listen(nursery, self._create_on_execution_contexts_cleared(nursery))
        await self.set_pause_on_exceptions()

    async def run_if_waiting_for_debugger(self) -> debugger.Paused:
        return await self.connection.send_and_wait_for(
            runtime.run_if_waiting_for_debugger(),
            debugger.Paused,
        )

    @asynccontextmanager
    async def wait_for(self, event_type: Type[T], buffer_size=1):
        proxy: Proxy[T]
        async with self.connection.wait_for(event_type=event_type, buffer_size=buffer_size) as proxy:
            yield proxy

    async def set_pause_on_exceptions(
        self,
        mode: Optional[Literal["none", "caught", "uncaught", "all"]] = None,
    ):
        await self.connection.send(
            debugger.set_pause_on_exceptions(
                mode if mode is not None else self.config.pause_on_exceptions_mode,
            ),
        )

    async def resume(self) -> debugger.Resumed:
        return await self.connection.send_and_wait_for(
            debugger.resume(),
            debugger.Resumed,
        )

    async def resume_and_wait_for_paused(self) -> debugger.Paused:
        async with self.wait_for(debugger.Paused) as proxy:
            await self.resume()
        await trio.lowlevel.checkpoint()
        return proxy.value

    async def send_and_wait_for_paused(self, send_arg) -> debugger.Paused:
        async with self.wait_for(debugger.Paused) as proxy:
            await self.connection.send(send_arg)
        await trio.lowlevel.checkpoint()
        return proxy.value

    async def continue_to_location(
        self,
        script_id: runtime.ScriptId,
        line_number: int,
    ) -> debugger.Paused:
        return await self.connection.send_and_wait_for(
            debugger.continue_to_location(
                location=debugger.Location(
                    script_id=script_id,
                    line_number=line_number,
                ),
            ),
            debugger.Paused,
        )

    async def get_script_source(
        self,
        script_id: runtime.ScriptId,
    ) -> str:
        return await self.connection.send(debugger.get_script_source(script_id))

    async def get_script_source_cached(
        self,
        script_id: runtime.ScriptId,
    ) -> str:
        return await self.sources.get(script_id, self.get_script_source)

    async def get_properties(
        self,
        object_id: runtime.RemoteObjectId,
        own_properties: Optional[bool] = None,
        accessor_properties_only: Optional[bool] = None,
        generate_preview: Optional[bool] = None,
    ) -> Tuple[
        List[runtime.PropertyDescriptor],
        Optional[List[runtime.InternalPropertyDescriptor]],
        Optional[List[runtime.PrivatePropertyDescriptor]],
        Optional[runtime.ExceptionDetails],
    ]:
        return await self.connection.send(
            runtime.get_properties(
                object_id=object_id,
                own_properties=own_properties,
                accessor_properties_only=accessor_properties_only,
                generate_preview=generate_preview,
            )
        )

    async def set_breakpoint(
        self,
        location: debugger.Location,
        condition: Optional[str] = None,
    ) -> Tuple[debugger.BreakpointId, debugger.Location]:
        return await self.connection.send(
            debugger.set_breakpoint(
                location=location,
                condition=condition,
            ),
        )

    async def remove_breakpoint(self, breakpoint_id: debugger.BreakpointId) -> None:
        return await self.connection.send(
            debugger.remove_breakpoint(
                breakpoint_id=breakpoint_id,
            ),
        )

    async def remove_breakpoints_by_url(self, url: str) -> None:
        await self.connection.send(ext_debugger.remove_breakpoints_by_url(url))

    async def set_breakpoint_by_url(self, *args, **kwargs) -> Tuple[debugger.BreakpointId, List[debugger.Location]]:
        return await self.connection.send(debugger.set_breakpoint_by_url(*args, **kwargs))

    async def get_possible_breakpoints(
        self,
        start: debugger.Location,
        end: Optional[debugger.Location] = None,
        restrict_to_function: Optional[bool] = None,
    ) -> List[debugger.BreakLocation]:
        return await self.connection.send(
            debugger.get_possible_breakpoints(
                start=start,
                end=end,
                restrict_to_function=restrict_to_function,
            )
        )

    async def get_possible_and_set_breakpoint_by_url(
        self,
        locations: List[ext_debugger.UrlBreakpointRequest],
    ) -> List[ext_debugger.CustomUrlBreakpointResponse]:
        return await self.connection.send(ext_debugger.get_possible_and_set_breakpoint_by_url(locations))

    async def set_breakpoints_active(self, active: bool) -> None:
        await self.connection.send(debugger.set_breakpoints_active(active=active))

    async def set_skip_all_pauses(self, skip: bool) -> None:
        await self.connection.send(debugger.set_skip_all_pauses(skip=skip))

    async def evaluate_on_call_frame(
        self,
        call_frame_id: debugger.CallFrameId,
        expression: str,
        object_group: Optional[str] = None,
        include_command_line_api: Optional[bool] = None,
        silent: Optional[bool] = None,
        return_by_value: Optional[bool] = None,
    ) -> Tuple[runtime.RemoteObject, Optional[runtime.ExceptionDetails]]:
        return await self.connection.send(
            debugger.evaluate_on_call_frame(
                call_frame_id=call_frame_id,
                expression=expression,
                object_group=object_group,
                include_command_line_api=include_command_line_api,
                silent=silent,
                return_by_value=return_by_value,
            )
        )

    async def evaluate(self, expression: str) -> tuple[runtime.RemoteObject, runtime.ExceptionDetails | None]:
        return await self.connection.send(runtime.evaluate(expression))

    async def restart_frame(self, frame_number: int) -> debugger.Paused:
        return await self.send_and_wait_for_paused(debugger.restart_frame(debugger.CallFrameId(str(frame_number))))

    async def step_into(self) -> debugger.Paused:
        return await self.send_and_wait_for_paused(debugger.step_into())

    async def step_out(self) -> debugger.Paused:
        return await self.send_and_wait_for_paused(debugger.step_out())

    async def step_over(self) -> debugger.Paused:
        return await self.send_and_wait_for_paused(debugger.step_over())

    async def profiler_enable(self) -> None:
        await self.connection.send(profiler.enable())

    async def profiler_set_sampling_interval(self, interval: int) -> None:
        await self.connection.send(profiler.set_sampling_interval(interval))

    async def profiler_start(self) -> None:
        await self.connection.send(profiler.start())

    async def profiler_stop(self) -> ext_profiler.ProfileArray:
        return await self.connection.send(ext_profiler.profiler_stop())

    async def profiler_disable(self) -> None:
        await self.connection.send(profiler.disable())

    def _create_on_execution_contexts_cleared(self, nursery: trio.Nursery):
        def _on_execution_contexts_cleared(_: runtime.ExecutionContextsCleared):
            # A deadlock can occur when client awaits a response after server's disconnect.
            # ArkTS debugger implementation notifies about execution end via `runtime.ExecutionContextsCleared` event,
            # which is used here to force client disconnect.
            nursery.cancel_scope.cancel()

        return _on_execution_contexts_cleared

    def _listen(
        self,
        nursery: trio.Nursery,
        handler: Callable[[T], None],
    ):
        async def _t():
            args_annotations = getfullargspec(handler).annotations
            event_type = list(args_annotations.values())[0]
            # Passing `T` as event type will not work
            async for event in self.connection.listen(event_type):
                handler(event)

        nursery.start_soon(_t)


@asynccontextmanager
async def create_debugger_client(
    connection: ArkConnection,
    scripts: ScriptsCache,
    sources: SourcesCache,
    code_compiler: StringCodeCompiler,
    debugger_config: DebuggerConfig = DebuggerConfig(),
) -> AsyncIterator[DebuggerClient]:
    context = await connection.send_and_wait_for(
        runtime.enable(),
        runtime.ExecutionContextCreated,
    )
    debugger_id = await connection.send(
        debugger.enable(),
    )
    yield DebuggerClient(
        connection=connection,
        config=debugger_config,
        debugger_id=debugger_id,
        scripts=scripts,
        sources=sources,
        context=context.context,
        code_compiler=code_compiler,
    )


class BreakpointManager:

    def __init__(self, client: DebuggerClient) -> None:
        self._lock = trio.Lock()
        self.client = client
        self._breaks: Dict[debugger.BreakpointId, List[debugger.Location]] = dict()

    async def set_by_url(self, line_number: int, url: Optional[str]) -> None:
        await self.client.set_breakpoints_active(True)
        br, locs = await self.client.set_breakpoint_by_url(line_number=line_number, url=url)
        async with self._lock:
            self._breaks[br] = locs

    async def get(self, bp_id: debugger.BreakpointId) -> Optional[List[debugger.Location]]:
        async with self._lock:
            return self._breaks.get(bp_id)

    @asynccontextmanager
    async def get_all(self):
        async with self._lock:
            breaks = self._breaks.copy()
        for br, locs in breaks:
            yield (br, locs)
            await trio.lowlevel.checkpoint()

    async def get_possible_breakpoints(self) -> Dict[debugger.BreakpointId, List[debugger.BreakLocation]]:
        # Сhrome does this after set_breakpoint_by_url
        async with self.get_all() as pair:
            return {
                br: br_locs
                async for br, locs in pair
                for br_locs in [
                    await self.client.get_possible_breakpoints(
                        start=dataclasses.replace(loc, column_number=0),
                        end=dataclasses.replace(loc, column_number=1),
                    )
                    for loc in locs
                ]
            }


class DebugLocator:
    scripts: ScriptsCache
    sources: SourcesCache

    def __init__(self, code_compiler: StringCodeCompiler, url: Any) -> None:
        self.code_compiler = code_compiler
        self.url = url
        self.scripts = ScriptsCache()
        self.sources = SourcesCache()

    @asynccontextmanager
    async def connect(self, nursery: trio.Nursery) -> AsyncIterator[DebuggerClient]:
        cdp = await connect_cdp(nursery, self.url, 10)
        async with cdp:
            connection = ArkConnection(cdp, nursery)
            async with create_debugger_client(
                connection,
                self.scripts,
                self.sources,
                self.code_compiler,
            ) as debugger_client:
                yield debugger_client
