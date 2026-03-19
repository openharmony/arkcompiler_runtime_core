# Paoc

`ark_aot` is the offline compiler driver built from the paoc toolchain. It compiles [panda binary
files](../../docs/file_format.md) and can run the compiler pipeline in four current modes:

1. `aot` - normal AOT compilation that writes `.an` output
2. `llvm` - LLVM-backed AOT compilation when the build enables it
3. `jit` - offline validation of the normal JIT-shaped pipeline without writing the usual `.an` output
4. `osr` - offline validation of the OSR-specific pipeline shape without writing the usual `.an` output

Use `compiler/tools/paoc/paoc.yaml` as the option source of truth and `compiler/tools/paoc/paoc.cpp` for current
behavior. This note is the compact workflow map.

## Current Option Groups

All regular compiler options are also applicable, for example `--compiler-regex`, `--compiler-non-optimizing`, and
`--compiler-cross-arch`.

### Mandatory Input

#### `--paoc-panda-files`

- Comma-separated list of panda files to compile
- This option is mandatory

### Mode Selection

#### `--paoc-mode`

- Compiler mode: `aot`, `llvm`, `jit`, or `osr`
- Default: `aot`

### Output and Location Metadata

#### `--paoc-output`

- Output `.an` file path
- Default: `out.an`

#### `--paoc-boot-output`

- Output boot `.an` file path
- Incompatible with `--paoc-output`

#### `--paoc-location`

- Location path of the input panda file written into the AOT file
- Use it when compile-time host paths and runtime-visible target paths differ
- The recorded location participates in AOT class-context verification, so runtime-visible file names must match this
  path layout

#### `--paoc-boot-location`

- Location path of boot panda files written into the AOT file

#### `--paoc-boot-panda-locations`

- Per-file location paths for boot panda files, including file names

#### `--paoc-generate-symbols`

- Generate symbols for compiled methods
- Always enabled in debug builds

### Method Selection

If no method filter is provided, paoc compiles every method from `--paoc-panda-files`.

#### `--paoc-methods-from-file:path=<file>[,iswhite=true|false]`

- Load a white-list or black-list of full method names from a file

#### `--paoc-skip-until`

- Set the first method to compile and skip all previous methods

#### `--paoc-compile-until`

- Set the last method to compile and skip all following methods

### PGO and Statistics

#### `--paoc-use-profile:path=<profile.ap>[,force]`

- Load runtime profile data from a `.ap` file for AOT PGO
- `force` makes paoc fail if the profile is missing or invalid
- Mirror the application files from `--paoc-panda-files` through `--panda-files` before the profile is applied; for
  multiple inputs, pass the same file set with `:` separators

#### `--paoc-dump-stats-csv`

- Dump allocator usage statistics in a smaller CSV form than `--compiler-dump-stats-csv`

### Clustered Compiler Options

#### `--paoc-clusters`

- Path to a JSON config that applies clusters of compiler options to selected methods

Format summary:

```json
{
  "clusters_map": {
    "Class::method1": [1],
    "Class::method2": [0, "cluster_1", 2]
  },
  "compiler_options": {
    "cluster_0": {
      "compiler_option_1": "arg",
      "compiler_option_2": "arg"
    },
    "cluster_1": {
      "compiler_option_3": "arg"
    }
  }
}
```

## Typical Commands

Compile one application in normal AOT mode:

```bash
./out/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=./out/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --paoc-output=app.an
```

Validate one method in OSR shape without a normal `.an` output:

```bash
./out/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=./out/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --paoc-mode=osr \
  --compiler-regex='App::hotLoop'
```

Compile with a saved profile:

```bash
./out/bin/ark_aot \
  --load-runtimes=ets \
  --boot-panda-files=./out/plugins/ets/etsstdlib.abc \
  --paoc-panda-files=app.abc \
  --panda-files=app.abc \
  --paoc-output=app.an \
  --paoc-use-profile:path=workload.ap
```

## Source Files

- `../tools/paoc/paoc.cpp`
- `../tools/paoc/paoc.yaml`
- `../tools/paoc/paoc_clusters.h`
