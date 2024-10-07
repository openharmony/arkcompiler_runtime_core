#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

import typing
from contextlib import asynccontextmanager
from typing import Any, AsyncGenerator, Callable, Coroutine, Dict, Generator, Optional, TypeAlias

import cdp
import cdp.util
import trio
import trio_cdp
from cdp import debugger, runtime

T: TypeAlias = trio_cdp.T
E = typing.TypeVar("E")
T_JSON_DICT: TypeAlias = cdp.util.T_JSON_DICT


class Proxy(typing.Generic[E]):
    def __init__(self) -> None:
        self._value: Optional[E] = None

    @property
    def value(self) -> E:
        if self._value is None:
            raise ValueError()
        return self._value

    @value.setter
    def value(self, value: E):
        self._value = value


class ArkConnection:

    def __init__(self, conn: trio_cdp.CdpConnection, nursery: trio.Nursery) -> None:
        self._conn = conn
        self._nursery = nursery
        self.context_id: runtime.ExecutionContextId

    @property
    def connection(self) -> trio_cdp.CdpConnection:
        return self._conn

    def listen(self, *event_types, buffer_size=10) -> trio.MemoryReceiveChannel:
        return self._conn.listen(*event_types, buffer_size=buffer_size)

    async def send(self, cmd: Generator[dict, T, Any]) -> T:
        return await self._conn.execute(cmd)

    @asynccontextmanager
    async def wait_for(self, event_type: typing.Type[T], buffer_size=10) -> AsyncGenerator[Proxy[T], None]:
        cmd_proxy: trio_cdp.CmEventProxy
        proxy = Proxy[T]()
        async with self._conn.wait_for(event_type, buffer_size) as cmd_proxy:
            yield proxy
        proxy.value = cmd_proxy.value  # type: ignore[attr-defined]

    async def send_and_wait_for(
        self,
        cmd: Generator[dict, T, Any],
        event_type: typing.Type[E],
        buffer_size=10,
    ) -> E:
        async with self.wait_for(event_type, buffer_size) as proxy:
            await self._conn.execute(cmd)
        return proxy.value

    def _listen(self, event_type: T, handler):
        async def a():
            async for event in self._conn.listen(event_type):
                handler(event)

        self._nursery.start_soon(a)


class DebugConnection(trio_cdp.CdpConnection):

    async def aclose(self):
        await super().aclose()
        self._close_channels()

    async def reader_task(self):
        try:
            await super()._reader_task()
        finally:
            self._close_channels()

    def _close_channels(self):
        channels: set[trio.MemorySendChannel] = set([c for s in self.channels.values() for c in s])
        self.channels.clear()
        for ch in channels:
            ch.close()


async def connect_cdp(nursery: trio.Nursery, url, max_retries: int) -> DebugConnection:
    counter = max_retries
    while True:
        try:
            conn = DebugConnection(
                await trio_cdp.connect_websocket_url(
                    nursery,
                    url,
                    max_message_size=trio_cdp.MAX_WS_MESSAGE_SIZE,
                )
            )
            nursery.start_soon(conn.reader_task)
            return conn
        except OSError:
            counter -= 1
            if counter == 0:
                raise
            await trio.sleep(1)


class ScriptsCache:
    def __init__(self) -> None:
        self._lock = trio.Lock()
        self._scripts: Dict[runtime.ScriptId, debugger.ScriptParsed] = {}

    async def __getitem__(self, script_id: runtime.ScriptId) -> debugger.ScriptParsed:
        async with self._lock:
            if script_id not in self._scripts:
                raise KeyError(script_id)
            return self._scripts[script_id]

    async def get(self, script_id: runtime.ScriptId) -> Optional[debugger.ScriptParsed]:
        async with self._lock:
            return self._scripts.get(script_id)

    async def get_by_url(self, url: str) -> Optional[debugger.ScriptParsed]:
        async with self._lock:
            for s in self._scripts.values():
                if url == s.url:
                    return s
            return None

    async def add(self, *scripts: debugger.ScriptParsed) -> None:
        async with self._lock:
            for s in scripts:
                self._scripts[s.script_id] = s


class SourcesCache:

    def __init__(
        self,
    ) -> None:
        self._lock = trio.Lock()
        self._scripts: Dict[runtime.ScriptId, str] = {}

    async def get(
        self,
        script_id: runtime.ScriptId,
        get_source: Optional[Callable[[runtime.ScriptId], Coroutine[Any, Any, str]]] = None,
    ) -> str:
        async with self._lock:
            result = self._scripts.get(script_id)
            if result is None:
                if get_source is not None:
                    result = await get_source(script_id)
                    self._scripts[script_id] = result
                else:
                    raise IndexError()
            return result
