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

import re
import os
import argparse
import logging

script_dir = os.path.dirname(os.path.realpath(__file__))

logging.basicConfig(format='%(message)s', level=logging.DEBUG)


def get_args():
    parser = argparse.ArgumentParser(description="Abckit status script")
    parser.add_argument(
        "--print-implemented",
        action="store_true",
        default=False,
        help=f"Print list of implemented API and exit")
    return parser.parse_args()
args = get_args()

API_PATTERN = "^[\w,\d,_, ,*]+\(\*([\w,\d,_]+)\)\(.*"
domain_patterns = ["struct Abckit\S*Api\s\{", "struct Abckit\S*ApiStatic\s\{", "struct Abckit\S*ApiDynamic\s\{"]
libabckit_dir = os.path.join(script_dir, "..")
libabckit_tests = os.path.join(libabckit_dir, "tests")

sources = {
    "include/c/ir_core.h",
    "include/c/metadata_core.h",
    "include/c/extensions/arkts/metadata_arkts.h",
    "include/c/extensions/js/metadata_js.h",
    "src/include_v2/c/isa/isa_static.h",
    "include/c/isa/isa_dynamic.h",
    "include/c/abckit.h"
}

TS = 'TS'
JS = 'JS'
ARKTS1 = 'ArkTS1'
ARKTS2 = 'ArkTS2'
NO_ABC = 'NoABC'


def check(cond, msg=''):
    if not cond:
        raise Exception(msg)


class API:
    def __init__(self, name, domain):
        self.name = name
        self.domain = domain
        self.dynamic_positive_tests = 0
        self.static_positive_tests = 0
        self.arkts1_tests = 0
        self.arkts2_tests = 0
        self.js_tests = 0
        self.ts_tests = 0
        self.no_abc_tests = 0
        self.positive_tests = 0
        self.negative_tests = 0
        self.negative_nullptr_tests = 0
        self.negative_ctx_tests = 0
        self.negative_mode_tests = 0
        self.other_tests = 0


def domain_match(line):
    for pattern in domain_patterns:
        if re.fullmatch(pattern, line.strip()):
            return line
    return None


def update_domain(old_domain, l):
    new_domain = domain_match(l.strip())
    if not new_domain:
        return old_domain
    return re.search('struct Abckit(.*)\s\{',
            new_domain.strip()).group(1)


def collect_api(path):
    api = {}
    domain = ""
    with open(path) as f:
        for l in f.readlines():
            domain = update_domain(domain, l)

            if not re.fullmatch(API_PATTERN, l.strip()):
                continue
            # CC-OFFNXT(G.FIO.05) regexp false positive
            if re.fullmatch('\/\/' + API_PATTERN, l.strip()):
                continue
            func_name = re.search('^[\w,\d,_, ,*]+\(\*([\w,\d,_]+)\)\(.*', l.strip()).group(1)
            if func_name == "cb":
                continue
            check(domain)
            api[f'{domain}Impl::{func_name}'] = API(func_name, domain)
    return api


def check_test_anno_line(line):
    mul_lines = False
    anno_line = False
    if re.fullmatch("^\/\/ Test: test-kind=.*\n", line):
        anno_line = True
        mul_lines = line.strip()[-1] == ','
    return {"is_annotation_line" : anno_line, "mul_annotation_lines": mul_lines}


def get_full_annotation(it, annotation_start):
    annotation = annotation_start.strip()
    next_anno_line = True
    while next_anno_line:
        new_line = next(it)
        annotation += new_line.replace('//', '').strip()
        if not re.fullmatch("^\/\/.*,$", new_line):
            next_anno_line = False
    return annotation


class Test:
    def __init__(self, s):
        err = f'Wrong test annotation: "{s}"'

        self.kind = ''
        self.abc_kind = ''
        self.api = ''
        self.category = ''

        check('// Test:' in s, err)
        s = s.replace('// Test:', '')
        entries = [x.strip() for x in s.split(',') if x]
        for entry in entries:
            key, value = entry.strip().split('=')
            if key == 'test-kind':
                check(value in ['api', 'scenario', 'internal'], err)
                self.kind = value
            elif key == 'abc-kind':
                check(value in [ARKTS1, ARKTS2, JS, TS, NO_ABC], err)
                self.abc_kind = value
            elif key == 'api':
                self.api = value
            elif key == 'category':
                possible_values = [
                    'positive',
                    'negative-mode',
                    'negative-nullptr',
                    'negative-file',
                    'internal',
                    'negative'
                ]
                check(value in possible_values, err)
                self.category = value
            else:
                check(False, f'Wrong key: {key}')

        check(self.kind, err)
        check(self.abc_kind, err)
        check(self.category, err)
        if self.kind == 'api':
            check(self.api, err)


def is_first_test_line(line):
    if re.fullmatch("TEST_F\(.*,.*\n", line):
        return True
    return False


def get_test_from_annotation(api, annotation):
    test = Test(annotation)
    if "api=" in annotation and test.api != 'ApiImpl::GetLastError':
        check(test.api in api, f'No such API: {test.api}')
    return test


def collect_tests(path, api):
    with open(path, "r") as f:
        lines = f.readlines()
        it = iter(lines)
        tests = []

        ano_count, test_count = 0, 0
        annotation = ''
        line = '\n'
        while True:
            line = next(it, None)
            if (line is None):
                break

            if is_first_test_line(line):
                test_count += 1
                check(test_count <= ano_count, f'Test has no annotation:\n{path}\n{line}\n')
                continue

            info = check_test_anno_line(line)
            if not info.get("is_annotation_line"):
                annotation = ''
                continue
            ano_count += 1
            annotation += line.strip()

            if info.get("mul_annotation_lines"):
                annotation = get_full_annotation(it, line)
            else:
                annotation = line.strip()

            if annotation:
                tests.append(get_test_from_annotation(api, annotation))

    check(ano_count == test_count, f'Annotation without test:\n{path}\n')
    return tests


def get_tests_statistics(api, tests):
    for test in tests:
        if test.kind != 'api' or test.api == 'ApiImpl::GetLastError':
            continue
        if test.abc_kind == ARKTS1:
            api[test.api].arkts1_tests += 1
        if test.abc_kind == ARKTS2:
            api[test.api].arkts2_tests += 1
        elif test.abc_kind == JS:
            api[test.api].js_tests += 1
        elif test.abc_kind == TS:
            api[test.api].ts_tests += 1
        elif test.abc_kind == NO_ABC:
            api[test.api].no_abc_tests += 1

        if test.category == 'positive':
            api[test.api].positive_tests += 1
            if test.abc_kind == TS or test.abc_kind == JS or test.abc_kind == ARKTS1:
                api[test.api].dynamic_positive_tests += 1
            if test.abc_kind == ARKTS2:
                api[test.api].static_positive_tests += 1
        elif test.category == 'negative':
            api[test.api].negative_tests += 1
        elif test.category == 'negative-nullptr':
            api[test.api].negative_nullptr_tests += 1
        elif test.category == 'negative-file':
            api[test.api].negative_ctx_tests += 1
        elif test.category == 'negative-mode':
            api[test.api].negative_mode_tests += 1
        else:
            api[test.api].other_tests += 1
    return api


def main(api) :
    tests = []
    for dirpath, _, filenames in os.walk(libabckit_tests):
        for name in filenames:
            if name.endswith(".cpp"):
                tests += collect_tests(os.path.join(dirpath, name), api)

    api = get_tests_statistics(api, tests)

    csv = (
        'api,dynamic_positive_tests,static_positive_tests,'
        'arkts1_tests,arkts2_tests,js_tests,ts_tests,no_abc_tests,positive_tests,'
        'negative_tests,negative_nullptr_tests,negative_ctx_tests,other_tests\n'
    )

    for name in api:
        csv += (
            f'{name},{api[name].dynamic_positive_tests},{api[name].static_positive_tests},'
            f'{api[name].arkts1_tests},{api[name].arkts2_tests},{api[name].js_tests},{api[name].ts_tests},'
            f'{api[name].no_abc_tests},{api[name].positive_tests},{api[name].negative_tests},'
            f'{api[name].negative_nullptr_tests},{api[name].negative_ctx_tests},{api[name].other_tests}\n'
        )

    with os.fdopen(os.open(os.path.join(libabckit_dir, 'scripts/abckit_status.csv'),
                           os.O_CREAT | os.O_WRONLY | os.O_TRUNC, 0o755), 'w') as f:
        f.write(csv)

    def api_test_category(name):
        return len(list(filter(lambda t: t.kind == 'api' and t.category == name, tests)))

    def api_lang(name):
        return len(list(filter(lambda t: t.kind == 'api' and t.abc_kind == name, tests)))

    def scenario_lang(name):
        return len(list(filter(lambda t: t.kind == 'scenario' and t.abc_kind == name, tests)))

    logging.debug('Total API:                                              %s', len(api))
    logging.debug('')
    logging.debug('Total Tests:                                            %s', len(tests))
    logging.debug('')
    logging.debug('Total API tests:                                        %s',
                  len(list(filter(lambda t: t.kind == "api", tests))))
    logging.debug('Positive/Negative/NullArg/WrongCtx/WrongMode API tests: %s/%s/%s/%s/%s',
                  api_test_category("positive"),
                  api_test_category("negative"),
                  api_test_category("negative-nullptr"),
                  api_test_category("negative-file"),
                  api_test_category("negative-mode"))
    logging.debug('ArkTS1/ArkTS2/JS/TS/NoABC API tests:                    %s/%s/%s/%s/%s',
                  api_lang(ARKTS1), api_lang(ARKTS2), api_lang(JS), api_lang(TS), api_lang(NO_ABC))
    logging.debug('')
    logging.debug('Total scenario tests:                                   %s',
                  len(list(filter(lambda t: t.kind == "scenario", tests))))
    logging.debug('ArkTS1/ArkTS2/JS/TS scenario tests:                     %s/%s/%s/%s',
                  scenario_lang(ARKTS1), scenario_lang(ARKTS2), scenario_lang(JS), scenario_lang(TS))
    logging.debug('')
    logging.debug('Internal tests:                                         %s',
                  len(list(filter(lambda t: t.kind == "internal", tests))))


collected_api = {}
for src in sources:
    collected_api = dict(collected_api.items() | collect_api(f'{libabckit_dir}/' + src).items())

if args.print_implemented:
    logging.debug('\n'.join(collected_api))
else:
    main(collected_api)

