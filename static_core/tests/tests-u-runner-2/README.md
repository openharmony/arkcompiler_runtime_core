# Universal test runner, version 2

## Description
Universal test runner, version 2, for Open Harmony.

## Prerequisites
-   Panda build
-   Python3 version at least 3.8. 3.10 is recommended.
-   Make sure to run `sudo static_core/scripts/install-deps-ubuntu -i=test` to create a ~/.venv-panda virtual environment with
all required python libraries  (`tqdm`, `dataclasses`, `python-dotenv`, etc). 
-   Suite `ets-es-checked` requires [node and some packages](#sts-es-checked-dependencies)

## Quick run

It is possible to run tests either using script `runner.sh` or `main.py` directly.

### Before the first start
- Create at your home the file `.urunner.env`
- Specify there following environment variables:
```bash
PANDA_SOURCE_PATH=<your path to panda source folder arkcompiler>
PANDA_BUILD=<your path to build folder>
WORK_DIR=<your path to temporary folder where all intermediate files and reports will be kept>
```

### Shell script
Script `runner.sh` activates the virtual environment with all required libraries
installed by `sudo static_core/scripts/install-deps-ubuntu -i=test` and then runs test(s).
This way is preferable.

After install script finishes you can run

```bash
urunner2> ./runner.sh <workflow-name> <test-suite-name> <other-options>
```

### Python script
You can run `main.py` directly. In order to do so you have to activate
the virtual environment `$HOME/.venv-panda` manually or propose all required
libraries in your working environment. Then `main.py` will run test(s) for you.

```bash
urunner>source ~/.venv-panda/bin/activate
urunner>python3 main.py <workflow-name> <test-suite-name> <other-options>
urunner>deactivate
```

## Reproducing CI test run

In case of fail on CI runner will show its options and used configurations files - use it to run test locally.


## Yaml configuration files

A lot of options can be set through yaml configuration file as `--config $YAML_CONFIG_FILE`. 
It can be mentioned several times and the data are merged according to following rules:
- If some keys are repeated the latest value is taken (from the last specified config file)
- If for a key different types are specified the first-met type is taken
- If for a key different lists are specified they are merged and repeated values are excluded.

The folder `cfg` contains several example config files which should be edited to set the real paths.

To see full list of supported options use CLI option `--generate-config $YAML_CONFIG_FILE`:

`urunner> ./runner.sh --generate-config $YAML_CONFIG_FILE --build-dir $BUILD`

The generated file will contain all supported options with default values.

> **Note**: if an option is set both in yaml config file and in command line interface the latter value will
> be applied. So CLI value has a higher priority.

## Important runner options

Note: after `/` the option name from config file is specified.

-   `--build-dir`/`general.build` - the path to the compiled project. Referenced as $BUILD in this readme.
-   `--work-dir`/`general.work-dir` - path to the working temp folder with gen, intermediate and report folders. As well 
here tests are generated from templates.
-   `--test-root`/`custom.test-root` - the folder where test suite is located. It must exist before runner starts. 
-   `--list-root`/`custom.list-root` - the folder where test lists are located. It must exist before runner starts.


## Test lists

Runner supports following kinds of test lists:

-   **excluded** test lists - tests specified here are excluded from execution. "Don't try to run"
-   **ignored** test lists - tests specified here run, but failures are ignored. Such lists are named "known failures list" or KFL as well.

Test list name usually has the following format: `<test-suite-name>[-<additional info>]-<kind>[-<OS>][-<architecture>][-<configuration>][-<sanitizer>][-<opt-level>][-REPEATS][-<build-type>].txt`

-   `kind` is one of `excluded` or `ignored`
-   `OS` is one of `LIN`, `WIN`, `MAC`.   If an operating system is set explicitly, the test list is applied only to this OS. If none is set, the list is applied to any OS.
-   `architecture` is one of `ARM32`, `ARM64`, `AMD32`, `AMD64`.  If an architecture is set explicitly, the test list is applied only to this architecture. If none is set, the list is applied to any architecture.
-   `configuration` is one of `INT`, `AOT`, `AOT-FULL`, `IRTOC`, `LLVM`, `JIT` or other used value for `interpreter-type` option. If a configuration is set explicitly, the test list is applied only to this configuration. If none is set, the list is applied to any configuration.
-   `sanitizer` is one of `ASAN` or `TSAN`. If a sanitizer is set explicitly, the test list is applied only to this build configuration. If none is set, the list is applied to any configuration.
-   `opt-level` is `OLx`, where `x` is opt-level, usually 0 or 2.
-   `REPEATS` is set if the test list should apply to runs with the option `--jit-repeats` sets number of repeats more than 1.
-   `build-type` is one of `DEBUG`, `RELEASE`, `FAST-VERIFY`.   If a build type is set explicitly, the test list is applied only to this build type. If none is set, the list is applied to any build type.

Examples:

-   `test262-flaky-ignored-JIT.txt` - list of ignored tests from Test262 suite, applied only in JIT configuration. `flaky` is additional info, it's only for more clear description what tests are collected there.
-   `hermes-excluded.txt` - list of excluded tests from Hermes suite, applied in any configuration.
-   `parser-js-ignored.txt` - list of ignored tests from JS Parser suite, applied in any configuration.
-   `ets-func-tests-ignored.txt` - list of ignored tests in `ets-func-tests` test suite
-   `ets-cts-FastVerify-ignored-OL2.txt` - list of ignored tests for `ets-cts` test suite and for opt-level=2.  `FastVerify` is additional info.

In any test list the test is specified as path to the test file relative from the `test_root`: Examples:

-   array-large.js
-   built-ins/Date/UTC/fp-evaluation-order.js
-   tests/stdlib/std/math/sqrt-negative-01.ets

Test file specified in the option `--test-file`/`test-lists.explicit-file` should be set in this format too.
By default, ignored or excluded lists are located near tests themselves.

All test lists are loaded automatically from the specified `LIST_ROOT` and based on following options:
- architecture:
  - from cli one of: `--test-list-arch=ARCH`, where ARCH is one of `amd64`, `arm32`, `arm64`
  - from config file: `test-lists.architecture: ARCH`. Values are the same.
- sanitizer:
  - from cli on of: `--test-list-san=SAN`, where SAN in one of `asan` or `tsan`
  - from config file: `test-lists.sanitizer: SAN`. Values are the same

> **Note**: these options just specifies what test lists to load and do not affect on how and where to start the runner
> itself and binaries used within.

## Utility runner options:

-   `--skip-test-lists`/`test-lists.skip-test-lists: True` - do not use ignored or excluded lists, run all available tests, report all found failures
-   `--test-list TEST_LIST`/`test-lists.explicit-list: TEST_LIST` - run tests ONLY listed in TEST\_LIST file.
-   `--test-file TEST_FILE`/`test-lists.explicit-file: TEST_FILE` - run ONLY ONE specified test. **Attention** - not test suite, but the single test from the suite.
-   `--update-excluded`/`test-lists.update-excluded: True` - regenerates excluded test lists
-   `--update-expected`/`test-lists.update-expected: True` - regenerates expected test lists (applied only for JS Parser test suite)
-   `--report-format`/`general.report-format` - specifies in what format to generate failure reports. By default, `md`. Possible value: `html`. As well reports in the plain text format with `.log` extension are always generated.
-   `--filter FILTER`/`test-lists.filter: FILTER` - test filter regexp
-   `--show-progress`/`general.show-progress: True` - show progress bar during test execution
-   `--time-report`/`time-report.enable: True` - generates report with grouping tests by execution time.

## Detailed report

Detailed report shows test statistics for every folder
- `--detailed-report`/`general.detailed-report: True` - if it's specified the report is generated
- `general.detailed-report-file: FILE` - specifies file/path where the report should be saved to

## Verbose and logging options:

-  `--verbose`, `-v` - Enable verbose output. Possible values one of:
   - `all` - the most detailed output,
   - `short` - test status and output.
   - if none specified (by default): only test status for new failed tests
   - in config file use `general.verbose` property with the save values.
-  `--verbose-filter` - Filter for what kind of tests to output stdout and stderr.
   Supported values:
   - `all` - for all executed tests both passed and failed.
   - `ignored` - for new failures and tests from ignored test lists including both passed and failed. '
   - `new` - only for new failures. Default value.
   - in config file use `general.verbose-filter` property with the same values.

## Generation options:

-   `--generate-only`/`general.generate-only` - only generate tests without running them. Tests are run as usual without this option.
-   `--force-generate`/`ets.force-generate` - force ETS tests generation from templates

## Other options:

To get runner all options use: `main.py --help` or `runner.sh --help`

## Execution time report

It is possible to collect statistics how long separate tests work. In the result report tests are grouped by execution time.

The grouping edges are set in seconds. For example, the value `1 5 10` specifies 4 groups - less than 1 second, from 1 second to 5 seconds, from 5 seconds to 10 seconds and from 10 seconds and more. For the last group the report contains real durations.

-   Specify the option `--time-report`/`time-report.enable: True`
-   Specify in yaml config file `time-report.time-edges: [1, 5, 10]` where value is a list of integers.
-   After test run the short report will be output to the console
-   And full report will be created at the `<work-dir/report>/time_report.txt`


## Linter and type checks

A script running linter and type checks starts as:

`urunner>./linter.sh`

It performs checks by running `pylint` and `mypy`.

For `pylint` settings see `.pylintrc` file. For `mypy` settings see `mypy.ini` file.

## Custom suite
If you want to run arbitrary set of ETS tests with URunner you can use a custom suite like (minimal configuration):

`urunner>./runner.sh --config suite.yaml --build-dir $BUILD`

suite.yaml configuration file with these options can be looked as follows:
```yaml
  custom:
    suite-name: SUITE_NAME
    test-root: TEST_ROOT
    list-root: LIST_ROOT
    extension: EXTENSION
```

where:
- SUITE_NAME - a tag used to name the suite
- TEST_ROOT - a path to the folder with tests (test files or templates)
- LIST_ROOT - a path to the folder with ignored/excluded lists. Note: SUITE_NAME is used as a <test-suite-name> in the 
name of the test list (see #Test lists for details)
- EXTENSION - extension what test files should be selected. For example: `ets`

### Custom generator
It is possible to specify a custom generator. It should be executable file accepting following parameters:

`custom_generator --source SOURCE_PATH --target TARGET_PATH <other-options>`

Here:
- SOURCE_PATH - path where template files are located. URunner fills this parameter with TEST_ROOT specified above.
- TARGET_PATH - path where generated tests should be put. URunner fills this parameter with value from --work-dir option ($WorkDir/gen)
- other-options - other options that might be required by the generator (optional)

To specify generator and its options use:
- `custom.generator: PATH` - path to the executable file-generator
- `custom.generator-options: [OPTIONS]` - multiple value option (can be specified several times)

## Steps

If set option `customScheme` in the yaml configuration file like:
```yaml
test-suites:
    - customScheme
```
It's possible to specify steps what should be executed:
```yaml
steps:
    es2abc: # step name, user-defined value
        order: 10 # order of execution the step with the less number is executed earlier
        executable-path: ${general.build}/arkcompiler/ets_frontend/es2abc # the binary to execute
        extra: # a dictionary of user-defined options
            opt-level: 2 # both the option name and value are defined by user
            output-file: "${general.work-dir}/intermediate/${test-id}.abc" # possible types of value: int, bool, str
        env: # a dictionary of key-value pairs to add to environment variables before the binary to run
            LD_LIBRARY_PATH: # sample variable name
                - "${general.build}/thirdparty/icu:" # the variable value can be anything, if it's a list, the items joined without separator
                - "${general.build}/arkcompiler/ets_runtime"
        args: # a list of options to transfer to the binary
            - "--opt-level=${steps.es2abc.extra.opt-level}"
            - "--output=${steps.es2abc.extra.output-file}"
            - "${general.work-dir}/gen/${test-id}"
        timeout: 30 # is seconds
```
The values can contain macros detected by `${}`. The macro's name is yaml path to the wished property.

The special macro `${test-id}` refers to the test file name (or path to the test file if some folder hierarchy is used).
It will be expanded during the binary executing.
