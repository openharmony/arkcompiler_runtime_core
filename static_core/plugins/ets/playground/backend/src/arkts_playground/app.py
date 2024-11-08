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
from typing import Optional, Tuple

from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware

from .config import get_settings
from .deps.runner import get_runner
from .routers import compile_run, options
from .routers import formatting


async def validate_config():
    get_runner()


def create_app(args: Optional[Tuple] = None) -> FastAPI:
    app = FastAPI(on_startup=[validate_config])
    settings = get_settings(args)
    if settings.server.cors:
        app.add_middleware(
            CORSMiddleware,
            allow_origins=settings.server.cors_allow_origins,
            allow_credentials=settings.server.cors_allow_credentials,
            allow_methods=settings.server.cors_allow_methods,
            allow_headers=settings.server.cors_allow_headers,
        )

    app.include_router(formatting.router)
    app.include_router(compile_run.router)
    app.include_router(options.router)
    return app
