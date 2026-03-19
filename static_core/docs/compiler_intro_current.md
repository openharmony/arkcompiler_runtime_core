# Compiler Notes

This note collects current compiler-wide caveats and navigation points for `static_core`. For implementation details,
use the owning guides and focused docs.

## Current Caveats

- `es2panda` compiles `.ets` into `.abc`; `ark` executes bytecode and may hand off hot code to JIT or AOT code
- `ark_aot` uses `--paoc-*` option names such as `--paoc-panda-files`, `--paoc-output`, `--paoc-location`, and
  `--paoc-mode`
- `runtime/options.yaml` declares `--interpreter-type` with default `llvm`; the runtime may downgrade the effective
  interpreter to `irtoc` or `cpp` when LLVM or irtoc support is unavailable or restricted
- `--compiler-enable-osr` is part of the normal runtime/compiler flow
- `--compiler-hotness-threshold=0 --no-async-jit=true` is the force-jit/testing recipe; `--no-async-jit=true` also
  forces STW GC unless epsilon and cannot be combined with AOT-loaded runs
- the compiler is plugin-extensible; in this tree, core compiler support includes `PANDA_ASSEMBLY`, ETS is the main
  static-core plugin, and `plugins/ecmascript/` is optional when present
- re-check exact pass counts, handler counts, and test totals against current sources such as
  `compiler/optimizer/pipeline.cpp` and current test definitions before relying on them
- current repository docs usually describe the static language/plugin as ETS or ArkTS-Sta

## Start Here

- `../compiler/AGENTS.md` for core compiler structure, pipeline entry points, and compiler-owned docs
- `../runtime/AGENTS.md` when the issue is at the runtime/compiler boundary
- `../irtoc/AGENTS.md` for irtoc pipeline, fastpath, and boundary entrypoint work
- `../tests/AGENTS.md` for checked, runner, and validation workflows
