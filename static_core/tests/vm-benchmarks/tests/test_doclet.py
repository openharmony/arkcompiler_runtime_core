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

import sys
import pytest  # type: ignore
from unittest.mock import patch
from vmb.doclet import DocletParser, TemplateVars
from vmb.helpers import get_plugin
from vmb.cli import Args

ETS_VALID = '''/**
 * @State
 * @Bugs gitee987654321, blah0102030405
 */

export class ArraySort {

    /**
     * @Param 4, 8, 16
     */
    size: int;

    /**
     * Specifics of the array to be sorted.
     * @Param "A", "B"
     */

    code: String;

    ints: int[];
    intsInitial: int[];

    /**
     * Foo
     */ 

    /**
     * Prepare array depending on the distribution variant.
     * @Setup
     */
    public prepareArray(): void {
        /* blah */
    }

    /**
     * @Benchmark -it 3
     * @Tags Ohos
     * @Bugs gitee0123456789
     */

    public sort(): void throws {
    }

    /**
     * @Benchmark
     * @Tags StdLib, StdLib_String, Ohos
     */
    public baseline(): void throws {
    }
}
'''

ETS_DUP = '''
/**
 * @State
 */
export class ArraySort {
  /**
   * @Param 7
   * @Param 4
   */
  size: int;
}
'''

ETS_NOSTATE_BENCH = '''
class X {
  /**
   * @Benchmark
   */
  x(): int {
}
'''

ETS_NOSTATE_SETUP = '''
class X {
  /**
   * @Setup
   */
  x(): int {
}
'''

ETS_NOSTATE_PARAM = '''
class X {
  /**
   * @Param 1
   */
  size: int;
}
'''

ETS_NOLIST_PARAM = '''
/**
 * @State
 */
class X {
  /**
   * @Param
   */
  size: int;
}
'''

ETS_BENCH_LIST = '''
/**
 * @State
 */
class X {
  /**
   * Help me
   * @Benchmark -mi 10 -wi 11 -it 3 -wt 4 -fi 6 -gc 300
   * -mi 999 -wi 999 -it 999 -wt 999 -fi 999 -gc 999
   */
  public testme(): int {
}
'''

ETS_STATE_TAGS = '''

/**
 * @State
 * @Tags StdLib, StdLib_Math
 */
class MathFunctions {
'''

ETS_STATE_COMMENT = '''

/**
 * @State
 */

 // blah
// foo bar
class XXX {
'''

ETS_PARAM_INIT = '''
/**
 * @State
 */
class X {
/**
 * Array size.
 * @Param 1024
 */
  size: int = 1024;
'''

ETS_MEASURE_OVERRIDES = '''
    /**
    * @State
    * @Benchmark -mi 33 -wi 44 -it 55 -wt 66
    */
    class X {
    /**
    * @Benchmark -mi 1 -wi 2 -wt 4 -fi 5 -gc 6
    */
    public one(): int {
    }
    /**
    * @Benchmark
    */
    public two(): bool {
    }
    '''

ets_mod = get_plugin('langs', 'ets')


def test_valid_ets():
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(ETS_VALID, ets).parse()
    assert parser.state is not None
    assert 'ArraySort' == parser.state.name
    assert 2 == len(parser.state.params)
    assert 2 == len(parser.state.benches)


def test_duplicate_doclets():
    ets = ets_mod.Lang()
    assert ets is not None
    with pytest.raises(ValueError):
        DocletParser.create(ETS_DUP, ets).parse()


def test_no_state():
    ets = ets_mod.Lang()
    assert ets is not None
    for src in (ETS_NOSTATE_BENCH, ETS_NOSTATE_SETUP, ETS_NOSTATE_PARAM):
        with pytest.raises(ValueError):
            DocletParser.create(src, ets).parse()


def test_bench_list():
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(ETS_BENCH_LIST, ets).parse()
    assert parser.state is not None
    assert 'X' == parser.state.name
    b = parser.state.benches
    assert 1 == len(b)
    assert 'testme' == b[0].name
    args = vars(b[0].args)
    # -mi 10 -wi 11 -it 3 -wt 4 -fi 6 -gc 300
    assert 10 == args['measure_iters']
    assert 11 == args['warmup_iters']
    assert 3 == args['iter_time']
    assert 4 == args['warmup_time']
    assert 6 == args['fast_iters']
    assert 300 == args['sys_gc_pause']


def test_bugs():
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(ETS_VALID, ets).parse()
    assert parser.state is not None
    assert 'ArraySort' == parser.state.name
    b = parser.state.benches
    assert 2 == len(b)
    assert 'sort' == b[0].name
    assert ['gitee0123456789'] == b[0].bugs
    assert 'gitee987654321' in parser.state.bugs
    assert 'blah0102030405' in parser.state.bugs


def test_state_tags():
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(ETS_STATE_TAGS, ets).parse()
    assert parser.state is not None
    tags = parser.state.tags
    assert 2 == len(tags)
    assert 'StdLib' in tags
    assert 'StdLib_Math' in tags


def test_tags_before_state():
    src = '''
    /**
     * @Tags Before, More
     */

    /**
     * @State
     * @Tags StdLib
     */
    class MathFunctions {
    '''
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(src, ets).parse()
    assert parser.state is not None
    tags = parser.state.tags
    assert 3 == len(tags)
    for t in ('Before', 'More', 'StdLib'):
        assert t in tags


def test_state_comment():
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(ETS_STATE_COMMENT, ets).parse()
    assert parser.state is not None
    assert 'XXX' in parser.state.name


def test_param_init():
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(ETS_PARAM_INIT, ets).parse()
    assert parser.state is not None


def test_measure_overrides():
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(ETS_MEASURE_OVERRIDES, ets).parse()
    assert parser.state is not None
    assert 'X' == parser.state.name
    b = parser.state.benches
    assert 2 == len(b)
    assert 'one' == b[0].name
    args = vars(b[0].args)
    assert 1 == args['measure_iters']
    assert 2 == args['warmup_iters']
    # Should be default
    assert args['iter_time'] is None
    assert 4 == args['warmup_time']
    assert 5 == args['fast_iters']
    assert 6 == args['sys_gc_pause']
    assert 2 == len(b)
    assert 'two' == b[1].name
    args = vars(parser.state.bench_args)
    assert 33 == args['measure_iters']
    assert 44 == args['warmup_iters']
    assert 55 == args['iter_time']
    assert 66 == args['warmup_time']
    # Should be default
    assert args['fast_iters'] is None
    # Should be default
    assert args['sys_gc_pause'] is None
    # Test cmdline overrides
    with patch.object(sys, 'argv', 'vmb gen --lang ets -fi 123 blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed('', parser.state, args))
        assert 2 == len(tpl_vars)
        var1, var2 = tpl_vars
        # Bench one
        assert var1.FIX_ID == 0
        assert var1.METHOD_NAME == 'one'
        assert var1.METHOD_RETTYPE == 'int'
        assert var1.BENCH_NAME == 'X_one'
        assert var1.MI == 1
        assert var1.WI == 2
        # Should use class level
        assert var1.IT == 55
        assert var1.WT == 4
        assert var1.FI == 5
        assert var1.GC == 6
        # Bench two
        assert var2.FIX_ID == 0
        assert var2.METHOD_NAME == 'two'
        assert var2.METHOD_RETTYPE == 'bool'
        assert var2.BENCH_NAME == 'X_two'
        assert var2.MI == 33
        assert var2.WI == 44
        assert var2.IT == 55
        assert var2.WT == 66
        # Should use sys.cmdline
        assert var2.FI == 123
        # Should be default
        assert var2.GC == -1


def test_tags():
    src = '''
    /**
    * @State
    * @Tags Cool, Fast
    */
    class X {
    /**
    * @Benchmark
    * @Tags Fast, One
    */
    public one(): int {
    }
    /**
    * @Benchmark
    */
    public two(): bool {
    }
    '''
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(src, ets).parse()
    assert parser.state is not None
    assert 'X' == parser.state.name
    assert set(parser.state.tags) == {'Cool', 'Fast'}
    b = parser.state.benches
    assert 2 == len(b)
    assert set(b[0].tags) == {'Fast', 'One'}
    assert set(b[1].tags) == set()
    # w/o --tags
    with patch.object(sys, 'argv', 'vmb gen --lang ets blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed(
            '', parser.state, args))
        assert 2 == len(tpl_vars)
        # bench + state params merged
        assert tpl_vars[0].TAGS == {'Cool', 'Fast', 'One'}
        assert tpl_vars[1].TAGS == {'Cool', 'Fast'}
    # with --tags filter
    with patch.object(sys, 'argv',
                      'vmb gen --lang ets --tags=One blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed(
            '', parser.state, args))
        assert 1 == len(tpl_vars)
        # bench + state params merged
        assert tpl_vars[0].TAGS == {'Cool', 'Fast', 'One'}
    # filter all
    with patch.object(
            sys,
            'argv', 'vmb gen --lang ets --tags=Unexistent blah'.split()):
        args = Args()
        tpl_vars = list(TemplateVars.params_from_parsed(
            '', parser.state, args))
        assert 0 == len(tpl_vars)


def test_import():
    src = '''
    /**
     * @Import hello from world
     */

    /**
     * @Import x from y
     * @State
     * @Import a from b
     */
    class Blah {
    }
    /**
     * @Benchmark
     * @Import m from n
     */
     public x(): int {
     }
    '''
    ets = ets_mod.Lang()
    assert ets is not None
    parser = DocletParser.create(src, ets).parse()
    assert parser.state is not None
    imports = parser.state.imports
    assert 4 == len(imports)
    for i in ('hello from world', 'x from y', 'a from b', 'm from n'):
        assert i in imports
