#!/usr/bin/env python3
# -- coding: utf-8 --
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

import subprocess
import sys
import shutil
import glob
import re
import random
from collections import namedtuple

HELP_TEXT = """
Script to help find minimal subsets of compiled methods, inlined methods and compiler passes
to reproduce compiler bugs.

Usage:
  Write a shell command or a bash script which launches a test and exits with non-zero code
  when the test fails. Use `$GEN_OPTIONS` variable in your script/command where
  compiler options are needed (probably next to other arguments of `ark`/`ark_aot`),
  and this script will set it to different values and run your script to find minimal subsets
  of options which lead to failure.
  Run `./compiler_bisect.py [COMMAND] [ARGS]...`

Example usage:
    cd $ARK_BUILD
    $ARK_ROOT/scripts/compiler_bisect.py bin/ark --boot-panda-files=plugins/ets/etsstdlib.abc --load-runtimes=ets  \
    --compiler-enable-jit=true --no-async-jit=true --compiler-hotness-threshold=0 --compiler-ignore-failures=false \
    \$GEN_OPTIONS test.abc ETSGLOBAL::main

  Or put command in `run.sh` and do `./compiler_bisect ./run.sh`

  Note that you need escaped \$GEN_OPTIONS when passing command with arguments, and not escaped
  $GEN_OPTIONS inside your script
"""


class Colors:
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'


pass_logs = {
    'lowering': 'lowering',
    'code-sink': 'code-sink',
    'balance-expressions': 'balance-expr',
    'branch-elimination': 'branch-elim',
    'checks-elimination': 'checks-elim',
    'deoptimize-elimination': 'deoptimize-elim',
    'licm': 'licm-opt',
    'licm-conditions': 'licm-cond-opt',
    'loop-unswitch': 'loop-unswitch',
    'loop-idioms': None,
    'loop-peeling': None,
    'loop-unroll': None,
    'lse': 'lse-opt',
    'cse': 'cse-opt',
    'vn': 'vn-opt',
    'memory-coalescing': 'memory-coalescing',
    'inlining': 'inlining',
    'if-conversion': 'ifconversion',
    'adjust-refs': None,
    'scalar-replacement': 'pea',
    'simplify-string-builder': 'simplify-sb',
    'enable-fast-interop': None,
    'interop-intrinsic-optimization': 'interop-intrinsic-opt',
    'peepholes': 'peephole',
    'move-constants': None,
    'if-merging': 'if-merging',
    'redundant-loop-elimination': 'rle-opt',
    'scheduling': 'scheduler',
    'optimize-memory-barriers': None,
}
pass_list = pass_logs.keys()


def print_color(color, *args, **kwargs):
    print(color, end='')
    print(*args, **kwargs)
    print(Colors.ENDC, end='')


def exit_fail(message='', usage=False):
    print(Colors.FAIL, end='')
    if message:
        print(message)
    if usage:
        print(f"Usage: {sys.argv[0]} COMMAND [ARG]..., COMMAND or ARGS should use $GEN_OPTIONS variable.")
        print(f"{sys.argv[0]} -h for help")
    exit(1)


def get_run_options(compiled_methods, noinline_methods, passes, dump, verbose):
    if compiled_methods is None:
        compiler_regex = '.*'
    else:
        compiler_regex = '(' + '|'.join(compiled_methods) + ')'
    inline_exclude = ','.join(noinline_methods)

    if passes is None:
        passes = pass_list
    options = [f'--compiler-regex={compiler_regex}', f'--compiler-inlining-blacklist={inline_exclude}']
    options += [f'--compiler-{opt}={str(opt in passes).lower()}' for opt in pass_list]
    if dump or verbose:
        options.append('--compiler-dump')
    if verbose:
        options.append('--compiler-disasm-dump:single-file')
        options.append('--log-debug=compiler')
        compiler_log = set(pass_logs.get(opt) for opt in passes) - {None}
        if any('loop' in opt for opt in passes):
            compiler_log.add('loop-transform')
        if compiler_log:
            options.append('--compiler-log=' + ','.join(compiler_log))
    return options


# compiled_methods - methods to compile or None if all
# noinline - methods to exclude in inlining
# passes - compiler options from `pass_list` to enable or None if all
# dump - whether to collect compiler dump
# expect_fail - if not None, print additional info for unexpected run result
def run(compiled_methods, noinline_methods, passes, dump=False, verbose=False, expect_fail=None):
    options = get_run_options(compiled_methods, noinline_methods, passes, dump, verbose)
    options_str = ' '.join(options)
    if not verbose:
        print(f"GEN_OPTIONS='{options_str}'")
    shutil.rmtree('ir_dump', ignore_errors=True)
    cmd_and_args = []
    for arg in sys.argv[1:]:
        if arg == '$GEN_OPTIONS':
            cmd_and_args += options
        else:
            cmd_and_args.append(arg)

    env = {"GEN_OPTIONS": options_str}
    if verbose or (expect_fail is not None):
        env['SHELLOPTS'] = 'xtrace'

    try:
        res = subprocess.run(cmd_and_args, env=env, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except Exception as e:
        exit_fail(str(e), usage=True)
    print_color(Colors.WARNING if res.returncode else Colors.OKGREEN, "return value:", res.returncode)
    failed = res.returncode != 0
    if (expect_fail is not None) and failed != expect_fail:
        verbose = True
    if verbose:
        print_out(res)
        print(f"GEN_OPTIONS='{options_str}'")
        print('Command:', ' '.join(cmd_and_args))
    return res


class Option:
    def __init__(self, values):
        self.all_values = set(values)
        self.values = list(values)

    @property
    def neg_values(self):
        return self.all_values - set(self.values)


class Runner:
    Options = namedtuple('Options', ['compiled', 'inline', 'passes'])

    def __init__(self, compiled_methods, inline, passes):
        self.opts = Runner.Options(*map(Option, [compiled_methods, inline, passes]))
        self.current_option = ''

    def set_option(self, name, value):
        self.opts = self.opts._replace(**{name: value})

    def run(self, verbose=False):
        return run(self.opts.compiled.values, self.opts.inline.neg_values,
                self.opts.passes.values, verbose=verbose).returncode != 0

    # If script failed, updates the option and returns True;
    # otherwise rolls the option back and returns False
    def try_run(self, opt, new_values):
        print_color(Colors.OKCYAN, f'Try {self.current_option}={new_values}')
        old_values = opt.values
        opt.values = new_values
        if self.run():
            return True
        opt.values = old_values
        return False

    def bisect(self, option_name):
        self.current_option = option_name
        opt = getattr(self.opts, option_name)
        while True:
            values = opt.values
            if len(values) == 0:
                break
            if len(values) > 3:
                mid = len(values) // 2
                left, right = values[:mid], values[mid:]
                # Try to reduce option set by half
                if self.try_run(opt, left):
                    continue
                if self.try_run(opt, right):
                    continue
            # If we were unable to reduce option set by half,
            # try to remove each individual option
            random.shuffle(values)
            for i in range(len(values)):
                new_values = values[:i] + values[i + 1:]
                if self.try_run(opt, new_values):
                    # continue `while True`
                    break
            else:
                # Cannot reduce option set
                break
        print_color(Colors.OKGREEN + Colors.BOLD, f'fixed {self.current_option}={opt.values}')


def parse_methods():
    compiled = set()
    inlined = set()
    dumps = glob.glob('ir_dump/*IrBuilder.ir')
    for dump in dumps:
        try:
            method = re.match(r'ir_dump/\d+_pass_\d+_(.*)_IrBuilder.ir', dump).group(1)
        except Exception:
            exit_fail('Failed to parse IR dumps')
        parts = re.split('(?<!_)_(?!_*$)', method)
        if parts[-1] in ['_ctor_', '_cctor_']:
            parts[-1] = f'<{parts[-1].strip("_")}>'

        dest = compiled
        if parts[0] == 'inlined':
            dest = inlined
            parts.pop(0)
        qname = '::'.join(('.'.join(parts[:-1]), parts[-1]))
        dest.add(qname)
    return (compiled, inlined)


def print_out(res):
    print('stdout:', res.stdout.decode('unicode_escape'), sep='\n')
    print('stderr:', res.stderr.decode('unicode_escape'), sep='\n')


def main():
    if len(sys.argv) < 2:
        exit_fail(usage=True)
    if sys.argv[1] in ('-h', '--help'):
        print(HELP_TEXT)
        exit(0)

    print_color(Colors.OKCYAN, f'Run without compiled methods')
    res = run([], [], [], expect_fail=False)
    if res.returncode:
        exit_fail("Script failed without compiled methods")

    print_color(Colors.OKCYAN, f'Run with all methods compiled and all optimizations enabled')
    res = run(None, [], None, dump=True, expect_fail=True)
    if not res.returncode:
        exit_fail("Script didn't fail with default args")

    compiled, _ = parse_methods()

    runner = Runner(compiled, [], pass_list)
    runner.bisect('compiled')

    run(runner.opts.compiled.values, [], None, dump=True)
    _, inlined = parse_methods()
    runner.set_option('inline', Option(inlined))
    runner.bisect('inline')

    runner.bisect('passes')

    print_color(Colors.OKGREEN + Colors.BOLD, 'Found $GEN_OPTIONS:')
    print(Colors.BOLD, end='')
    if runner.run(verbose=True):
        print_color(Colors.OKGREEN + Colors.BOLD, 'options:',
                *[f'{name}: {opt.values}' for name, opt in runner.opts._asdict().items()], sep='\n')
    else:
        exit_fail("Didn't fail with found options")


if __name__ == "__main__":
    main()
