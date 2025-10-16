#!/usr/bin/env python
# -*- coding: utf-8 -*-

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

import pytest
from src.arkts_playground.deps.runner import Runner


@pytest.fixture
def fake_saved_stsfile(monkeypatch) -> str:
    async def _fake_save_code(_tempdir, _code):
        return "/tmp/fixed_ast"

    monkeypatch.setattr(Runner, "_save_code", staticmethod(_fake_save_code))
    return "/tmp/fixed_ast"
