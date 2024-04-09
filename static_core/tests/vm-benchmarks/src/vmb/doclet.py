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

from __future__ import annotations

import re
import logging
import argparse
from typing import List, Dict, Optional, Iterable, Any
from itertools import combinations, product
from dataclasses import dataclass, field
from collections import namedtuple
from vmb.lang import LangBase
from vmb.helpers import StringEnum, split_params
from vmb.cli import Args, add_measurement_opts

log = logging.getLogger('vmb')
NameVal = namedtuple("NameVal", "name value")


class LineParser:

    def __init__(self, lines, pos, lang):
        self.lines = lines
        self.pos = pos
        self.lang = lang

    def next(self) -> None:
        if self.pos + 1 > len(self.lines):
            raise IndexError('No lines left')
        self.pos += 1

    def skip_empty(self) -> None:
        for _ in range(4):
            self.next()
            line = self.current.strip()
            if line and not line.startswith('//'):
                break

    @property
    def current(self) -> str:
        if 0 <= self.pos < len(self.lines):
            return self.lines[self.pos]
        return ''

    @classmethod
    def create(cls, text: str, lang: LangBase):
        return cls(text.split("\n"), -1, lang)


class Doclet(StringEnum):
    # Lang-dependent
    STATE = "State"
    BENCHMARK = "Benchmark"
    PARAM = "Param"
    SETUP = "Setup"
    RETURNS = "returns"
    # Lang-agnostic
    IMPORT = "Import"
    TAGS = "Tags"
    BUGS = "Bugs"

    @staticmethod
    def exclusive_doclets() -> Iterable[Doclet]:
        return Doclet.STATE, Doclet.BENCHMARK, Doclet.SETUP, Doclet.PARAM


@dataclass
class BenchFunc:
    name: str
    return_type: Optional[str] = None
    args: Optional[argparse.Namespace] = None
    tags: List[str] = field(default_factory=list)
    bugs: List[str] = field(default_factory=list)


@dataclass
class BenchClass:
    name: str
    setup: Optional[str] = None
    params: Dict[str, List[str]] = field(default_factory=dict)
    benches: List[BenchFunc] = field(default_factory=list)
    bench_args: Optional[argparse.Namespace] = None
    imports: List[str] = field(default_factory=list)
    tags: List[str] = field(default_factory=list)
    bugs: List[str] = field(default_factory=list)


class DocletParser(LineParser):

    # Note: strictly this should be `/**` not just `/*`
    re_comment_open = re.compile(r'^\s*/\*(\*)?\s*$')
    re_comment_close = re.compile(r'^\s*\*/\s*$')
    re_doclet = re.compile(r'@+(\w+)\s*(.+)?$')
    re_returns = re.compile(r'{+(\w+)}')

    def __init__(self, lines, pos, lang) -> None:
        super().__init__(lines, pos, lang)
        self.state: Optional[BenchClass] = None
        self.opts_parser = argparse.ArgumentParser()
        add_measurement_opts(self.opts_parser)
        self.__pending_tags: List[str] = []
        self.__pending_bugs: List[str] = []
        self.__pending_imports: List[str] = []

    def doclist(self, comment: str) -> List[NameVal]:
        doclets = []
        for line in comment.split("\n"):
            m = self.re_doclet.search(line)
            if m:
                doclets.append(NameVal(*m.groups()))
        return doclets

    @staticmethod
    def validate_comment(doclets: List[NameVal]) -> None:
        doclet_names = [v.name for v in doclets]
        for d in Doclet.exclusive_doclets():
            if doclet_names.count(d.value) > 1:
                raise ValueError(f'Multiple @{d} doclets in same comment')
        for d1, d2 in combinations(Doclet.exclusive_doclets(), 2):
            # allow @State + @Benchmark for compatibility reasons
            if sorted((d1, d2)) == [Doclet.BENCHMARK, Doclet.STATE]:
                continue
            if doclet_names.count(d1.value) > 0 and \
                    doclet_names.count(d2.value) > 0:
                raise ValueError(f'@{d1} and @{d2} doclets in same comment')

    def ensure_state(self) -> BenchClass:
        if not self.state:
            raise ValueError('No state found!')
        return self.state

    @staticmethod
    def ensure_value(val: Optional[str]) -> str:
        if not val:
            raise ValueError('Empty value!')
        return val

    @staticmethod
    def get_rettype(value: str) -> str:
        m = DocletParser.re_returns.search(value)
        if m:
            return m.group(1)
        return ''

    def parse_bench_overrides(self, line: str) -> Optional[argparse.Namespace]:
        """Parse @Benchmark options."""
        overrides = None
        if line:
            overrides, unknown = \
                self.opts_parser.parse_known_args(line.split())
            if unknown:
                raise ValueError(f'Unknown arg to @Benchmark: {unknown}')
        return overrides

    def process_state(self, benchmarks: List[NameVal]) -> None:
        self.skip_empty()
        class_name = self.lang.parse_state(self.current)
        if not class_name:
            raise ValueError('Bench class declaration not found!')
        self.state = BenchClass(name=class_name, tags=self.__pending_tags,
                                bugs=self.__pending_bugs,
                                imports=self.__pending_imports)
        self.__pending_tags, self.__pending_bugs = [], []
        self.__pending_imports = []
        # check if there are overrides for whole class
        for _, value in benchmarks:
            self.state.bench_args = self.parse_bench_overrides(value)

    def process_benchmark(self, value: str, returns: List[NameVal]) -> None:
        self.skip_empty()
        f = self.lang.parse_func(self.current)
        if not f:
            raise ValueError('Bench func declaration not found!')
        overrides = self.parse_bench_overrides(value)
        ret_type = f[1]
        if returns:
            typ = self.get_rettype(returns[0].value)
            if typ:
                ret_type = typ
        self.ensure_state().benches.append(
            BenchFunc(name=f[0], return_type=ret_type, args=overrides,
                      tags=self.__pending_tags, bugs=self.__pending_bugs))
        self.__pending_tags, self.__pending_bugs = [], []

    def process_param(self, param_values: str) -> None:
        self.skip_empty()
        p = self.lang.parse_param(self.current)
        if not p:
            raise ValueError('Param declaration not found!')
        self.ensure_state().params[p[0]] = \
            split_params(self.ensure_value(param_values))

    def process_setup(self) -> None:
        self.skip_empty()
        f = self.lang.parse_func(self.current)
        if not f:
            raise ValueError('Setup func declaration not found!')
        self.ensure_state().setup = f[0]

    def process_tag(self, value: str, states: List[NameVal]) -> None:
        self.__pending_tags += split_params(value)
        if self.state and states:
            # only for @State + @Tags
            self.state.tags += self.__pending_tags
            self.__pending_tags = []

    def process_bug(self, value: str, states: List[NameVal]) -> None:
        self.__pending_bugs += split_params(value)
        if self.state and states:
            # only for @State + @Bugs
            self.state.bugs += self.__pending_bugs
            self.__pending_bugs = []

    def parse_comment(self, comment: str) -> None:
        """Process all the @Stuff in multiline comment.

        Assuming only one exclusive doclet in same comment
        Except for @State + @Benchmark (wich is allowed by legacy)
        @Tags, @Bugs, @Import could co-exist with other @Stuff
        """
        doclets = self.doclist(comment)
        self.validate_comment(doclets)

        def filter_doclets(t: Doclet) -> List[NameVal]:
            return list(filter(lambda x: x.name == t.value, doclets))

        states = filter_doclets(Doclet.STATE)[:1]
        benchmarks = filter_doclets(Doclet.BENCHMARK)[:1]
        for _, value in filter_doclets(Doclet.TAGS):
            self.process_tag(value, states)
        for _, value in filter_doclets(Doclet.BUGS):
            self.process_bug(value, states)
        for _, value in filter_doclets(Doclet.IMPORT):
            if self.state:
                self.state.imports.append(value)
            else:
                self.__pending_imports.append(value)
        for _ in states:
            self.process_state(benchmarks)
            return
        for _, param_values in filter_doclets(Doclet.PARAM)[:1]:
            self.process_param(param_values)
            return
        for _ in filter_doclets(Doclet.SETUP)[:1]:
            self.process_setup()
            return
        for _, value in benchmarks:
            self.process_benchmark(value, filter_doclets(Doclet.RETURNS))
            return

    def parse(self) -> DocletParser:
        """Search and parse doclet comments."""
        comment = ''
        try:
            while True:
                self.next()
                if not comment and \
                        re.search(self.re_comment_open, self.current):
                    comment += "\n"
                    continue
                if comment and re.search(self.re_comment_close, self.current):
                    self.parse_comment(comment)
                    comment = ''
                    continue
                if comment and '@' in self.current:
                    comment += "\n" + self.current
        except IndexError:
            pass
        return self


@dataclass
class TemplateVars:  # pylint: disable=invalid-name
    """Params for bench template.

    Names of class props are same as of variables inside template,
    so this could be provided `as dict` to Template
    """

    # Full bench source to be pasted into template
    SRC: str
    # Name of main class
    STATE_NAME: str = ''
    # Setup method call: bench.SomeMethod();'
    STATE_SETUP: str = ''
    # '\n'-joined list of 'bench.param1=5;'
    STATE_PARAMS: str = ''
    # ';'-joined param list of 'param1=5'
    FIXTURE: str = ''
    FIX_ID: int = 0
    # Name of test method
    METHOD_NAME: str = ''
    METHOD_RETTYPE: str = ''
    METHOD_CALL: str = ''
    BENCH_NAME: str = ''
    COMMON: str = ''  # common feature is obsoleted
    # this should be the only place with defaults
    MI: int = 3
    WI: int = 2
    IT: int = 1
    WT: int = 1
    FI: int = 0
    GC: int = -1
    TAGS: Any = None
    BUGS: Any = None
    IMPORTS: Any = None

    @classmethod
    def params_from_parsed(cls,
                           src: str,
                           parsed: BenchClass,
                           args: Optional[Args] = None
                           ) -> Iterable[TemplateVars]:
        """Produce all combinations of Benches and Params."""
        tags_filter = args.tags if args else []
        tests_filter = args.tests if args else []
        # list of lists of tuples (param_name, param_value)
        # sorting by param name to keep fixture indexing
        params = [
            [(p, v) for v in vals]
            for p, vals
            in sorted(parsed.params.items())
        ]
        fixtures = list(product(*params))
        for b in parsed.benches:
            # check tags filter:
            tags = set(parsed.tags + b.tags)  # @State::@Tags + @Bench::@Tags
            if tags_filter and not set.intersection(tags, tags_filter):
                continue
            # if no params fixtures will be [()]
            fix_id = 0
            for f in fixtures:
                tp = cls(src, parsed.name)
                fix_str = ';'.join([f'{x[0]}={x[1]}' for x in f])
                if not fix_str:
                    fix_str = 'No Params'
                tp.METHOD_NAME = b.name
                tp.METHOD_RETTYPE = b.return_type if b.return_type else ''
                # strictly speaking, this should be lang specific
                tp.STATE_PARAMS = "\n    ".join([
                    f'bench.{p} = {v};' for p, v in f])
                tp.FIXTURE = fix_str
                tp.FIX_ID = fix_id
                tp.BENCH_NAME = f'{parsed.name}_{b.name}'
                if tp.FIX_ID > 0:
                    tp.BENCH_NAME = f'{tp.BENCH_NAME}_{tp.FIX_ID}'
                if tests_filter and \
                        not any((x in tp.BENCH_NAME) for x in tests_filter):
                    fix_id += 1
                    continue
                tp.STATE_SETUP = f'bench.{parsed.setup}();' \
                    if parsed.setup else ''
                tp.TAGS = tags
                tp.BUGS = set(parsed.bugs + b.bugs)
                # Override measure settings in following order:
                # Defaults -> CmdLine -> Class -> Bench
                tp.set_measure_overrides(args, parsed.bench_args, b.args)
                tp.IMPORTS = parsed.imports
                yield tp
                fix_id += 1

    def set_measure_overrides(self, *overrides) -> None:
        """Override all measurement options."""
        for ovr in overrides:
            if not ovr:
                continue
            if ovr.measure_iters is not None:
                self.MI = ovr.measure_iters
            if ovr.warmup_iters is not None:
                self.WI = ovr.warmup_iters
            if ovr.iter_time is not None:
                self.IT = ovr.iter_time
            if ovr.warmup_time is not None:
                self.WT = ovr.warmup_time
            if ovr.fast_iters is not None:
                self.FI = ovr.fast_iters
            if ovr.sys_gc_pause is not None:
                self.GC = ovr.sys_gc_pause
