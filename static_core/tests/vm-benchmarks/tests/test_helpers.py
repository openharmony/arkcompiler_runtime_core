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

# flake8: noqa
# pylint: skip-file

import json
import inspect
from typing import Optional
from dataclasses import dataclass, field
from collections import namedtuple
from vmb.helpers import Singleton, remove_prefix, read_list_file, Jsonable


NameVal = namedtuple("NameVal", "name value")


def test_singleton() -> None:
    class S(metaclass=Singleton):
        pass

    a = S()
    b = S()
    assert a is b


def test_prefix() -> None:
    assert "There" == remove_prefix("HelloThere", "Hello")
    assert "NotMatched" == remove_prefix("NotMatched", "nOT")


def test_jsonable() -> None:

    @dataclass
    class A(Jsonable):
        x: str = 'XXX'
        y: int = 123
        z: NameVal = NameVal('N', 'V')

        @classmethod
        def from_obj(cls, **kwargs):
            kwargs = {
                k: v for k, v in kwargs.items()
                if k in inspect.signature(cls).parameters
            }
            for k, v in kwargs.items():
                if 'z' == k:
                    kwargs[k] = NameVal(*v)
            return cls(**kwargs)

    @dataclass
    class B(Jsonable):
        a: A = field(default_factory=A)
        b: str = 'BBB'

    a = A()
    assert a.z.name == 'N'
    assert a.z.value == 'V'

    assert '{"a": {"x": "XXX", "y": 123, "z": ["N", "V"]}, "b": "BBB"}' == \
        B().js(indent=None)

    obj = json.loads('{"x": "ok", "y": 5, "z": [3, 4]}')
    a = A.from_obj(**obj)
    assert a.z.name == 3
    assert a.z.value == 4


@dataclass
class C(Jsonable):
    m: int
    n: Optional[int]


def test_json_loads() -> None:
    obj = json.loads('{"n": 1, "m":2}')
    assert C(**obj).n == 1
    obj = json.loads('{"n": null, "m":2}')
    assert C(**obj).n is None
