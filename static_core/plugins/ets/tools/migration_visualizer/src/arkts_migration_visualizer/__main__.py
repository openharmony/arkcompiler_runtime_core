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

from __future__ import annotations

from collections.abc import Callable
from pathlib import Path
import runpy
from typing import cast


def _load_main() -> Callable[[], None]:
    module_globals = runpy.run_path(str(Path(__file__).resolve().with_name("cli.py")))
    main_func = module_globals.get("main")
    if not callable(main_func):
        raise RuntimeError("cli.py does not export a callable main()")
    return cast(Callable[[], None], main_func)


main = _load_main()


if __name__ == "__main__":
    main()
