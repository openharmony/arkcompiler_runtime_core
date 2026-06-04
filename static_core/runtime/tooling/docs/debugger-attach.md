# Debugger attach

Ark VM provides functionality to switch a running application into debug execution mode.

The feature is intended for late debugger attach, including applications that are already running in release configuration (IRTOC interpreter, AOT enabled). Instead of supporting separate IRTOC dispatch table with debug events emitted by handlers, the currently selected approach switches execution into `cpp` interpreter.

## API layer

Debugger attach is exposed through `ark::tooling::Tools::AttachDebugSession`.

The API is intended to be called from a native thread already attached to the VM and is guarded by the runtime option `--debugger-attach-allowed`. On success, the VM enters debug mode for all current and future execution in the process.

At a high level `AttachDebugSession` performs three actions:

1. Enables runtime debug mode through `Runtime::SetDebugMode`.
2. Deoptimizes AOT-backed execution by invalidating compiled entrypoints and preventing new methods from being linked to AOT code.
3. Switches all managed threads to debug dispatch table under `ScopedSuspendAllThreads`.

## Implementation

Switching interpreter into debug mode relies on the fact that bytecode handlers re-read the dispatch table from `ManagedThread` in selected execution points.

### CPP interpreter

CPP interpreter [defines](../../interpreter/templates/interpreter-inl_gen.h.erb) two entrypoints, which overall define 3 distinct dispatch tables:
- `ExecuteImpl`
  - Release dispatch table - common bytecode handlers, no debug events
  - Switch dispatch table - all bytecode handlers point to one single handler which invokes `ExecuteImplDebug`
- `ExecuteImplDebug`
  - Debug dispatch table - common bytecode handlers, debug events enabled

Switching to debug execution is implemented by setting value of `ManagedThread::debugDispatchTable_` into `ManagedThread::dispatchTable_`, which is re-read by interpreter in the following points:
- Handling of backward jumps
- `call*` bytecodes handlers
  - Before call - for interpreted methods invocations
  - After call - for compiled methods invocations
- `return*` bytecodes handlers

### IRTOC interpreter

IRTOC interpreter [defines](../../interpreter/templates/irtoc_interpreter_utils.h.erb) a single entrypoint for release execution only, which, similar to `ExecuteImpl`, define two dispatch tables:
- Release dispatch table - common bytecode handlers, no debug events
- Switch dispatch table - all bytecode handlers point to one single handler which invokes `ExecuteImplDebug`

Similar to CPP interpreter, switching to debug execution is done by changing the value of `ManagedThread::dispatchTable_`, which is then re-read in the following points:
- Handling of backward jumps
- `return*` bytecodes handlers
- Return from compiled code invocation
- `throw` bytecode handler
- Exception handler

Since interpreter can be re-entered (e.g. `Method::Invoke` used in an intrinsic), supporting switch between different interpreter implementations requires the debug and release dispatch tables to be restored in the end of `interpreter::ExecuteImpl` to their values prior to execution.

### Limitations

- No JIT support
  - `ark::tooling::Tools::AttachDebugSession` rejects attempts to switch when JIT is enabled
- No async deoptimization points or deoptimizations in safepoints
  - AOT code will transit to debug execution mode only after return to the caller
