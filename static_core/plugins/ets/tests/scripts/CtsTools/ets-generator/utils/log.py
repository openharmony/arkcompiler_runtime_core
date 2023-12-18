#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
# This file does only contain a selection of the most common options. For a
# full list see the documentation:
# http://www.sphinx-doc.org/en/master/config

import sys
import time
import traceback


def __print_formatted(severity, msg):
    timestamp = int(round(time.time() * 1000))
    print("{} {} [Host] - {}".format(timestamp, severity, msg), file=sys.stderr)


def info(msg, extra_line=False):
    if extra_line:
        print("\n", file=sys.stderr)
    __print_formatted("INFO", msg)


def exception(msg):
    (_, value, tb) = sys.exc_info()
    __print_formatted("ERROR", f"{str(value)} \n message = {msg}")
    traceback.print_tb(tb, file=sys.stderr)
