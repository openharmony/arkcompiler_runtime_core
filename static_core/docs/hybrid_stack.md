# ArkTS 1.2 crash with hybrid stack


## 1.2 crash stack
in 1.2 application, the crash offten happened in cpp code. when we have cpp crash, we will have a cpp crash stack, for example:
```
...
ProcessMemory(kB):...
Device Memory(kB):...
Reason: ...
Fault thread info:
TID:..., Name:...
#00 pc 0000000000XXXXXX /system/lib64/libarkruntime.so
#01 pc 0000000000XXXXXX /system/lib64/libarkruntime.so
#02 pc 0000000000XXXXXX /system/lib64/libarkruntime.so
#03 pc 0000000000XXXXXX /system/lib64/platformsdk/libace_compatible.z.so
...
```
The main reasons of crash are fatal check and abnormal pointer
#### 1. Caused by fatal check reason
If a crash is caused by fatal reason. Usually it happened when running a wrong branch or exception scenarios. There will be additional error infomation in the crash log in `LastFatalMessage`.

For example:
```
...
Reason:Signal:SIGABRT(SI_TKILL)...
LastFatalMessage:runtim:Invalid class state transition XXX -> XXX;
Fault thread info:
Tid:..., Name:...
...
```
We can get the error reason and more info in `LastFatalMessage` field. Like `class state` in the example.

And can find the error location in the code by fatal info. In the example, the error code is in `arkcompiler/runtime_core/static_core/runtime/class.cpp` file, method `SetState` (`void Class::SetState(Class::State state)`).

```
void Class::SetState(Class::State state)
{
    if (...) {
        LOG(FATAL, RUNTIME) << "Invalid class state transition " << state_ << " -> " << state;
    }

    state_ = state;
}
```

#### 2. Abnormal pointer
Another main kind of crash usually caused by abnormal pointer, for example:
```
···
Reason:Signal:SIGSEGV(SEGV_MAPERR)@0x0000000000000044 probably caused by NULL pointer dereference
Fatul thread info:
Tid:..., Name:...
···
```
It may caused by some memory problem, and no extra fatal info. We can get info in log, or we can disassemble the so and find the crash place by pc in the stack top.

### Disassemble so to obtain the code location
The stack top usually in a specially so file. and we can get the crash location by the info from stack and disassemble the so file.

In the example, the stack top is in libarkruntime,so, so we can disassemble this so first:

```
aarch64-linux-gnu-objdump -S -d -l libarkruntime.so > libarkruntime.so.dump
```

We can use `aarch64-linux-gnu-objdump` or other objdump tools.

And we can see the disassemble result in `libarkruntime.so.dump`.

For example:
```
...
0000000000XXXXXX <_ZN3ark5Class8SetStateENS0_5StateE>:
_ZN3ark5Class8SetStateENS0_5StateE():
  1cXXXX:    d105XXXX    sub   sp, sp, #x170
  1cXXXX:    a913XXXX    stp   x29, x30, [sp, #304]
...
```

In the dumped file, the first column is PC value. Then find the assembly info and method name by the PC value in stack top.

### ets/js language stack
The current 1.2 crash stack can only provide native stack. So we can't get the ets call stack in the crash file. And when has interop with 1.1, there will be no js stack in the crash log. So if we want get ets/js language stack, we should use other interfaces, like show in blow.

## Get 1.2 ets stack
The ets stack info in 1.2 is store in the stack in each thread, and we can get it by `StackWalker`.

`StackWalker` can create by the thread (`StackWalker::Create(ManagedThread::GetCurrent())`).
And `StackWalker` has a function for Dump: `void StackWalker::Dump(std::ostream &os, bool print_vregs /* = false */)` .

The function can be called in cpp runtime code for printing ets call stack. And we can choose if we should print vregs by `print_vregs` parameter (`false` by default).

For Example:
```
    auto thread = ManagedThread::GetCurrent();
    auto stack = StackWalker::Create(thread);
    std::stringstream ss;
    stack.Dump(ss);
```

Output Example:

```
Panda call stack:
   0: 0x7f000XXXX in std.core.Array::slice (managed)
   1: 0x7f000XXXX in std.core.Array::shift (managed)
   2: 0x7f000XXXX in std.concurrency.TimerTable::lambda_invoke-1 (managed)
   3: 0x7f000XXXX in std.concurrentcy.%%lambda-lambda_invoke-1::$_invoke (managed)
   4: 0x7f000XXXX in std.concurrency.ConcurrencyHelpers::spinlockGuard (managed)
   5: 0x7f000XXXX in std.concurrency.TimerTable::removeClearedTimers (managed)
...
```

Add the code in the place you want, and use `StackWalker` get 1.2 ets stack.

## Get 1.1 js stack in interop scenarios

When run the pure 1.2 application, there is no 1.1 js stack in the application. So it does not involve the js call stack.

When run the interop application, we can't get the js stack in crash log, so we should use the method in 1.1 DFX.

`BuildJsStackTrace` is an method in `DFXJSNapi`: `bool DFXJSNapi::BuildJsStackTrace(const EcmaVM *vm, std::string &stackTraceStr)`

This function can get the js stack info by current vm. We should get the current vm first and get the string with stack info by `BuildJsStackTrace`.

For Example:
```
  auto engine = reinterpret_cast<NativeEngine*>(env);
  auto vm = engine->GetEcmaVm();
  std::string stack;
  DFXJSNapi::BuildJsStackTrace(vm, stack);
```

Output Example:

```
  at anonymous (entry|entry|1.0.0|src/main/ets/workers/wrokerNewEnvTest.ts:15:12)
  at add (entry|entry|1.0.0|src/main/ets/workers/workerNewEnvTest.ts:21:16)
```

Add the code in the place you want, and use the method `BuildJsStackTrace` get the js stack in 1.1 vm.
