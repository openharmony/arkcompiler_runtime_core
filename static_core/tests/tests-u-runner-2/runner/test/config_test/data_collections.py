#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
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

collections = {
    'ark_tests/ast/parser/js': {
        'parameters': {
            'extension': 'js',
            'es2panda-full-args': ['--dump-ast', '--extension=${parameters.extension}', '--thread=0', '--parse-only']
        }
    },
    'ast/parser/ts': {
        'parameters': {
            'extension': 'ts',
            'es2panda-full-args': ['--dump-ast', '--extension=${parameters.extension}', '--thread=0', '--parse-only']
        }
    },
    'ast/compiler/ets': {
        'parameters': {
            'arktsconfig': '${parameters.build}/tools/es2panda/generated/arktsconfig.json',
            'extension': 'sts',
            'es2panda-full-args': [
                '--dump-ast', '--extension=${parameters.extension}', '--thread=0',
                '--output=/dev/null', '--arktsconfig=${parameters.arktsconfig}']
        }
    }
}
