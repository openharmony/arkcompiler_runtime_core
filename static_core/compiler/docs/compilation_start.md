# How Compilation Starts

The current tree has two task-runner shapes:

- in-place compilation, where the compile step runs immediately in the current thread
- background compilation, where the compile step is queued through `taskmanager::TaskQueueInterface`

Both are implemented on top of `ark::TaskRunner` from `libarkbase/task_runner.h`. The important current callback rule
is that callbacks and finalizers receive `ContextT &`, not a zero-argument lambda.

## In-Place Flow

Current in-place entry points include:

- `compiler/tools/paoc/paoc.cpp` for offline `ark_aot --paoc-mode=jit|osr`
- `runtime/compiler_thread_pool_worker.cpp` for synchronous JIT compilation in the current thread

The in-place runner type is `compiler::InPlaceCompilerTaskRunner` from `compiler/inplace_task_runner.h`.

Current context fields are:

- `Method *`
- OSR flag
- `PandaVM *` when runtime-side compilation needs it
- arena allocator and local allocator
- method name

Typical current pattern:

```cpp
compiler::InPlaceCompilerTaskRunner taskRunner;
auto &taskCtx = taskRunner.GetContext();
taskCtx.SetMethod(method);
taskCtx.SetOsr(isOsr);
taskCtx.SetAllocator(&allocator);
taskCtx.SetLocalAllocator(&graphLocalAllocator);
taskCtx.SetMethodName(methodName);

taskRunner.AddFinalize(
    [&graph](compiler::InPlaceCompilerContext &compilerCtx) { graph = compilerCtx.GetGraph(); });
taskRunner.AddCallbackOnFail(
    [&success]([[maybe_unused]] compiler::InPlaceCompilerContext &compilerCtx) { success = false; });

compiler::CompileInGraph<compiler::INPLACE_MODE>(runtimeIface, isDynamic, arch, std::move(taskRunner));
```

`InPlaceCompilerTaskRunner::StartTask(...)` still exists, but current in-tree call sites usually call
`CompileInGraph<compiler::INPLACE_MODE>(...)` or `CompileMethodLocked<compiler::INPLACE_MODE>(...)` directly.

## Background Flow

Current background orchestration lives in `runtime/compiler_task_manager_worker.cpp`.

The runner type is `compiler::BackgroundCompilerTaskRunner` from `compiler/background_task_runner.h`. It is constructed
with:

- `taskmanager::TaskQueueInterface *`
- the current compiler `Mutator *`
- `RuntimeInterface *`

The background context stores owned wrappers around:

- `CompilerTask`
- compiler thread (`Mutator`)
- arena allocators
- graph / pipeline state

Typical current pattern:

```cpp
compiler::BackgroundCompilerTaskRunner taskRunner(queue, compilerThread.get(), runtimeIface);
auto &taskCtx = taskRunner.GetContext();
taskCtx.SetCompilerThread(std::move(compilerThread));
taskCtx.SetCompilerTask(std::move(compilerTask));

taskRunner.AddFinalize([this]([[maybe_unused]] compiler::BackgroundCompilerContext &taskContext) {
    // In runtime/compiler_task_manager_worker.cpp this finalize callback pops the
    // finished task from the deque and schedules the next queued compilation, if any.
});

auto backgroundTask = [this](compiler::BackgroundCompilerTaskRunner runner) {
    if (runner.GetContext().GetMethod()->AtomicSetCompilationStatus(Method::WAITING, Method::COMPILATION)) {
        compiler_->StartCompileMethod<compiler::BACKGROUND_MODE>(std::move(runner));
        return;
    }
    compiler::BackgroundCompilerTaskRunner::EndTask(std::move(runner), false);
};

compiler::BackgroundCompilerTaskRunner::StartTask(std::move(taskRunner), std::move(backgroundTask));
```

`StartTask(...)` now queues work by wrapping the callback and setting the compiler thread on the runtime interface
around task execution.

## What To Read Next

- `compiler/inplace_task_runner.h` - current in-place context and `StartTask(...)`
- `compiler/background_task_runner.h` - current background context, ownership model, and queue integration
- `libarkbase/task_runner.h` - callback, finalize, and success/fail chaining semantics
- `compiler/tools/paoc/paoc.cpp` - current offline in-place call site
- `runtime/compiler_thread_pool_worker.cpp` - current synchronous runtime call site
- `runtime/compiler_task_manager_worker.cpp` - current queued background call site
