This directory contains an array of interoperability tests and covers a majority of ArkTS <-> JS|TS interaction cases.

## Running interop tests

### 1. Compile Panda with interop support

Set PANDA_ETS_INTEROP_JS flag to `true` while cmake'ing Panda to enable interop testing.

The whole command, ran from `runtime_core/static_core/build`, will look like this:

```
cmake .. -GNinja -DCMAKE_BUILD_TYPE=debug -DCMAKE_EXPORTCOMPILECOMMANDS=ON -DPANDA_ETS_INTEROP_JS=1
```

### 2. Trigger a Ninja target to run the whole suite

Run `ninja -v ets_interop_tests` from `runtime_core/static_core/build` to run the suite.

### 3. Run a particular subsuite if needed

If you want to run a specific subsuite, refer to the corresponding `CMakeLists.txt` file. In most cases, it is located in the suite parts' directory (e.g., `./tests/scenarios/js_to_ets/CMakeLists.txt`). If the file is missing, check one level up.

In the file, you'll find either a `panda_ets_interop_js_gtest` or a `panda_ets_interop_js_test` function called for a given set of files. First argument given to this function is the test subsuite name.

If it is a `panda_ets_interop_js_test`, simply use `ninja -v <YOUR_SUBSUITE_NAME>`.
If it is a `panda_ets_interop_js_gtest`, you'll have to add a `_gtests` suffix to a subsuite name to get it running. As follows: `ninja -v <YOUR_SUBSUITE_NAME>_gtests`.

## Contributing to interop coverage

### 0. Ensure your contribution idea is not already in the code base.

### 1. Create a folder for your subsuite in ./tests

Subdirectory name should reflect the cases covered by subsuite. E.g., `value_assignment`, if you plan a separate tests suite for cases where values are being assigned from one runtime to another.

Generally, you have to include a couple of subdirs for ArkTS -> JS and JS -> ArkTS cases. But if you expect lots of code (say, over 100 lines per file or over 10 separate cases), it might be reasonable to split your subsuite into an array of nested folders. E.g.:

```
├── tests
│   └── value_assignment
│       ├── CMakeLists.txt
│       ├── primitive
│       │    ├── CMakeLists.txt
│       │    ├── ark_calls_js
│       │    └── js_call_ark
│       ├── reference
│       │    ├── CMakeLists.txt
│       │    ├── ark_calls_js
│       │    └── js_call_ark

```

### 2. Creating and invoking test cases

### 3. Code style

- **JavaScript**:

JavaScript code should comply with the ES6 standard, and developers are encouraged to employ its features, such as:

    - const | let | var variables
    - classes
    - spread operators (both in parameter handling and object manipulation)
    - module syntax
    - destructuring etc.

Formatting rules are the following:

    - semicolons are mandatory
    - strings are enclosed in single quotes (e.g., 'value') or backticks for interpolated strings
    - strict equality only (`===` and `!==`)
    - curly braces are mandatory for `if` operators: no one-line-ifs, please.

- **TypeScript**:

TS used in interop tests, either as transpilation source or (future) as directly ran scripting languange, should match the strict requirements analogous to those enforced by ArkTS compiler. As follows:

    - no implicit or explicit any
    - explicit typing (type inference is discouraged for any cases except primitive type variable declaration)
    - no default exports, no default imports

Important:

    - if you need a private property or method, please used hash-privates (#privateProp) instead of `private` access modifier. Otherwise, TS won't keep the access modification when transpiling.

- **ArkTS**:

Refer to `runtime_core/static_core/plugins/ets/doc` for cookbook recipes and language specifications
