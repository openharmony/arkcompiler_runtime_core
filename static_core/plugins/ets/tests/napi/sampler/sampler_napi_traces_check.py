#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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
import argparse

parser = argparse.ArgumentParser(description="Napi trace test")
parser.add_argument("--file", type=str)
args = parser.parse_args()
file_name = args.file

trace_list = ["LETSGLOBAL::CallNativeSlowFunction; LETSGLOBAL::NativeSlowFunction;",
    "LETSGLOBAL::CallNativeFastFunction; LETSGLOBAL::NativeFastFunction;",
    "LETSGLOBAL::CallNativeNAPIFastFunction; LETSGLOBAL::NativeNAPIFastFunction;",
    "LETSGLOBAL::CallNativeNAPISlowFunction; LETSGLOBAL::NativeNAPISlowFunction; LETSGLOBAL::SlowETSFunction;"]

ALL_TRACES_FOUND = True

with open(file_name, 'r') as my_file:
    content = my_file.read()
    for string in trace_list:
        if not string in content:
            ALL_TRACES_FOUND = False
    if not ALL_TRACES_FOUND:
        print("Actual stack trace")
        print(content)
        raise Exception("Not all native traces found")
