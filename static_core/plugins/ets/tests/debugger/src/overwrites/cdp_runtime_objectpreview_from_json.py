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

import cdp.runtime as runtime


def overwrite() -> None:
    """
    The debugger does not return the required fields `overflow` and `properties`.
    """

    def fix_object_preview_from_json(json: typing.Dict[str, typing.Any]) -> runtime.ObjectPreview:
        return runtime.ObjectPreview(
            type_=str(json["type"]),
            overflow=bool(json["overflow"]) if "overflow" in json else False,  # overwrite
            properties=(
                [runtime.PropertyPreview.from_json(i) for i in json["properties"]] if "properties" in json else []
            ),  # overwrite
            subtype=str(json["subtype"]) if "subtype" in json else None,
            description=str(json["description"]) if "description" in json else None,
            entries=[runtime.EntryPreview.from_json(i) for i in json["entries"]] if "entries" in json else None,
        )

    setattr(runtime.ObjectPreview, "from_json", fix_object_preview_from_json)
