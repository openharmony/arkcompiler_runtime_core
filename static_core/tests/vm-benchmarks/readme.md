# VM Benchmarks

VMB is a tool for running benchmarks for variety Virtual Machines, platforms and languages.

## Quick start

The only requirement is python3 (3.7+), no additional modules needed.

#### Option 1: Using wrapper script

```shell
# Run all js and ts examples on Node (needs node and tsc):
./run-vmb.sh all -p node_host `pwd`/examples/benchmarks

# Run all only js examples on Node (needs node installed):
./run-vmb.sh all -l js -p node_host `pwd`/examples/benchmarks

# See all options explained:
./run-vmb.sh help

# List all available plugins:
./run-vmb.sh list
```

#### Option2: Build and install vmb module

Wrapper script `run-vmb.sh` has all the module functionality,
but it requires absolute paths in cmdline.
Using python package, built and installed, provides more flexibiliy
in dealing with various benchmark sources and repos.
Once installed on host it exposes single `vmb` command for generating,
running and reporting tests.

```sh
python3 -m pip install build
# inside current repo:
python3 -m build
python3 -m pip install dist/vmb-*-py3-none-any.whl
# add user bin in PATH if needed:
export PATH=$HOME/.local/bin:$PATH
# Run all js and ts tests in current dir
vmb all -p node_host
# Run only js tests with tag 'sanity' from 'examples'
vmb all -p node_host -l js -t sanity ./examples
```

#### Usage Example: Compare 2 runs

```shell
export PANDA_BUILD=~/arkcompiler/runtime_core/static_core/build
# Run ets and ts tests on ArkTS in
# 1) interpretation mode
vmb all -p arkts_host --aot-skip-libs \
    --mode=int --report-json=int.json ./examples/benchmarks/
# 2) in JIT mode
vmb all -p arkts_host --aot-skip-libs \
    --mode=jit --report-json=jit.json ./examples/benchmarks/
# Compare results
vmb report --compare int.json jit.json

    Comparison: arkts_host-int vs arkts_host-jit
    ========================================
    Time: 1.99e+05->3.80e+04(better -80.9%); Size: 9.17e+04->9.17e+04(same); RSS: 4.94e+04->5.34e+04(worse +8.1%)
    =============================================================================================================
```

## Commands:
`vmb` cli supports following commands:
- `gen` - generate benchmark tests from [doclets](#doclet-format)
- `run` - run generated tests
- `all` = `gen` + `run`
- `report` - works with json report (display, compare)
- `list` - show info supported langs and platforms

## Selecting platform
Required `-p` (`--platform`) option to `run` or `all` command should be the name
of plugin from `<vmb-package-root>/plugins/platforms`
or from `<extra-plugins-dir>/platforms` if specified.
F.e. `ark_js_vm_host` - for ArkHz on host machine,
or `arkts_ohos` - for ArkTS on OHOS device


## Selecting language and source files
`gen` command requires `-l` (`--langs`) option.
F.e. `vmb gen -l ets,swift,ts,js ./examples/benchmarks`
will generate benches for all 4 languages in examples.

Then provided to `all` command `--langs` will override langs, supported by platform.
F.e. `vmb all -l js -p v_8_host ./tests` will skip `ts` (typescript)

Source files (doclets) could be overriden
by `--src-langs` option to `all` or `run` command.

Each platform defines languages it can deal with,
and each language defines its source file extention.
Defaults are:

| platform      | langs      | sources         |
|---------------|------------|-----------------|
| `arkts_*`     | `ets`      | `*.ets`, `*.ts` |
| `ark_js_vm_*` | `ts`       | `*.ts`          |
| `swift_*`     | `swift`    | `*.swift`       |
| `v_8_*`       | `ts`, `js` | `*.ts`, `*.js`  |
| `node_*`      | `ts`, `js` | `*.ts`, `*.js`  |

## Selecting and filtering tests:
- Any positional argument to `all` or `gen` command would be treated
as path to doclets: `vmb all -p x ./test1/test1.js ./more-tests/ ./even-more`
- Files with `.lst` extention would be treated as lists of paths, relative to `CWD`
- `--tags=sanity,my` will generate and run only tests tagged with `sanity` OR `my`
- `--tests=my_test` will run only tests containing `my_test` in name
- To exclude some tests from run point `--exclude-list` to the file with test names to exclude
- Any positional argument to `run` command would be treated
as path to generated tests: `vmb run -p x ./generated`

## Benchmark's measurement options:
* `-wi` (`--warmup-iters`) controls the number of warmup iterations, default is 2.
* `-mi` (`--measure-iters`) controls the number of measurement iterations, default is 3.
* `-it` (`--iter-time`) controls the duration of iterations in seconds, deafult is 1.
* `-wt` (`--warmup-time`) controls the duration of warmup iterations in seconds, deafault is 1.
* `-gc` (`--sys-gc-pause`) Non-negative value forces GC (twice) and <value> milliseconds sleep before each iteration (GC finish can't be guaranteed on all VM's), default is -1 (no forced GC).
* `-fi` (`--fast-iters`) Number of 'fast' iterations (no warmup, no tuning cycles). Benchmark will run this number of iterations, reagardless of time elapsed.

## Controlling log:
There are several log levels wich could be set via `--log-level` option.
- `fatal`: only critical errors will be printed
- `pass`: print single line for each test after it finishes
- `error`
- `warn`
- `info`: default level
- `debug`
- `trace`: most verbose

Each level prints in its own color.
`--no-color` disables color and adds `[%LEVEL]` prefix to each message.

## Extra (custom) plugins:

Providing option `--extra-plugins` you can add any
language, tool, platform or hook.
This parameter should point to the directory containing any combination of
`hooks`, `langs`, `platforms`, `tools` sub-dirs.
As the simplest case you can copy any of the existing plugins
and safely experiment modifying it.
`extra-plugins` has higher 'priority', so any tools, platforms and langs
could be 'overriden' by custom ones.

```sh
# Example:
# 2 demo plugins: Tool=dummy and Platform=dummy_host
mkdir bu_Fake
touch bu_Fake/bench_Fake.txt
vmb run -p dummy_host \
    --extra-plugins=./examples/plugins \
    ./bu_Fake/bench_Fake.txt
```

## Doclet format:
This is a mean for benchmark declaration in source file.
In a single root class it is possible to define any number of benchmarks,
setup method, benchmarks parameters, run options and some additional meta-info.
See examples in `examples/benchamrks` directory.

Supported doclets are:
* `@State` on a root class which contains benchmarks tests as its methods.
* `@Setup` on an initialization method of a state.
* `@Benchmark` on a method that is measured.
   It should not accept any parameters and 
   (preferably) it may return a value which is consumed.
* `@Param  p1 [, p2...]` on a state field.
   Attribute define values to create several benchmarks using same code,
   and all combinations of params.
   Value can be an int, a string or a comma separated list of ints or strings.
* `@Tags t1 [, t2...]` on a root class or method. List of benchmark tags.
* `@Bugs b1 [, b2...]` on a root class or method. List of associated issues.

JS specific doclets (JSDoc-like) are:
* `@arg {<State name>} <name>` on a benchmark method.
   Links a proper state class to each parameter.
* `@returns {Bool|Char|Int|Float|Obj|Objs}` optionally on a benchmark method.
   Helps to build automatic `Consumer` usage.

## Self tests and linters
```sh
# install pip if needed
wget https://bootstrap.pypa.io/get-pip.py
python3 get-pip.py
# install tox module
python3 -m pip install tox
python3 -m tox
```
