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

from typing import Annotated

from fastapi import APIRouter, Depends

from ..deps.runner import get_runner, Runner
from ..models.common import VersionsResponse
from ..models.compile import CompileResponse, CompileRequestModel, RunResponse

router = APIRouter()


@router.post("/compile",
             response_model=CompileResponse,
             tags=["run"],
             description="Compile provided code and returns the result")
async def compile_arkts_code(body: CompileRequestModel,
                             runner: Annotated[Runner, Depends(get_runner)]) -> CompileResponse:
    result = await runner.compile_arkts(
        body.code,
        options=runner.parse_compile_options(body.options),
        disasm=body.disassemble
    )
    return CompileResponse(**result)


@router.post("/run",
             response_model=RunResponse,
             tags=["run"],
             description="Compile and run provided code")
async def compile_run_arkts_code(body: CompileRequestModel,
                                 runner: Annotated[Runner, Depends(get_runner)]) -> RunResponse:
    result = await runner.compile_run_arkts(
        body.code,
        options=runner.parse_compile_options(body.options),
        disasm=body.disassemble
    )
    return RunResponse(**result)


@router.get("/versions",
            response_model=VersionsResponse,
            tags=["run"],
            description="Return playground version and Ark compiler version")
async def get_versions(runner: Annotated[Runner, Depends(get_runner)]) -> VersionsResponse:
    playground_ver, ark_ver = await runner.get_versions()
    return VersionsResponse(backendVersion=playground_ver, arktsVersion=ark_ver)
