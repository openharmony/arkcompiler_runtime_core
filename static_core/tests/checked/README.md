# Checked Tests

Checked tests is the tests that have additional functionality to check result of the test being run.
For example, check some event was raised or some IR instruction is appeared after specific optimization.

Each checker's command should start with special token (`'#!'` for panda assembly language, `//!` for other languages) at the
beginning of the line.

Allowed multiple checkers in a single file. Each checker starts with command `CHECKER` and ends with line without
command token at the zero position.

Each command is a valid code in the `ruby` language.

## List of commands

* **CHECKER** (description: string) begin new Checker with specified description
* **RUN** run panda application, following named arguments are allowed:
    - *options: string* - additional options for Panda VM
    - *entry: string* - entry point, default - `_GLOBAL::main`
    - *result: int* - expected value to be returned by the `panda` application
    - *abort: int* - expected terminal signal
    - *env: string* - environment variables setup string to be used as execution command prefix
    - *force_jit: bool* - run jit compilation for every executed method
    - *force_profiling: bool* - enables profdata collection for every interpreter-executed method
    - *pgo_emit_profdata: bool* - enables on-disk generating for profile data to be used in PGO
* **RUN_PAOC** run paoc application on the compiled panda file. Output panda file will be passed to the following panda
    run. Thus, `RUN_PAOC` command must be placed before `RUN` command. Following options are allowed:
    - *options: string* - additional options for paoc
    - *bool: bool* - enables boot output
    - *result: int* - expected value to be returned by the `paoc`
    - *abort: int* - expected terminal signal
    - *env: string* - environment variables setup string to be used as execution command prefix
    - *inputs: string* - list of panda files to compile
    - *output: string* - name of AOT file to be generated (incompatible to subsequent `RUN` call for now)
    - *pgo_use_profdata: bool* - usese profile data for PGO. Should be used after `RUN` call with `pgo_emit_profdata` enabled
* **RUN_PGO_PROF** - runs panda application with forces profiling and PGO profdata emitting. Output profdata will be passed to the following AOT run that use PGO. Syntax sugar for `RUN` command with `pgo_emit_profdata` enabled
* **RUN_PGO_PAOC** - runs paoc to compile panda file using collected profdata. Should be run after `RUN` with `pgo_emit_profdata` enabled. Syntax sugar for `RUN_PAOC` call
* **RUN_LLVM** - runs paoc application on the compiled panda file with LLVM AOT backend. Requires LLVM support at `paoc` and passing `--with-llvm` to `checker.rb`
* **RUN_AOT** - runs paoc application on the compiled panda file with various AOT modes. Currently, command creates separate checkes for `RUN_PAOC` and `RUN_LLVM` backends
* **RUN_BCO** - runs frontend with bytecode optimizer
* **EVENT** (event: pattern) search event within all events
* **EVENT_NEXT** (event: pattern) ordered search event, i.e. search from position of the last founded event
* **EVENT_NOT** (event: pattern) ensure event is not occurred
* **EVENT_NEXT_NOT** (event: pattern) ensure event is not occurred after current position
* **METHOD** (name: string) start check of specified method, all following checks that require specific method will use method specified by this command
* **PASS_AFTER** (pass_name: string) specify pass after which IR commands should operate
* **PASS_BEFORE** (pass_name: string) select pass that is right before the specified one
* **INST** (inst: pattern) search specified instruction in the ir dump file specified by commands `METHOD` and `PASS_AFTER`
* **INST_NOT** (inst: pattern) equal to `NOT INST`, i.e. check that instruction is not exist
* **INST_NEXT_NOT** (event: pattern) ensure instruction is not occurred after current position
* **IR_COUNT** (inst: string) search specified phrase and counts the number in the ir dump file specified by commands `METHOD` and `PASS_AFTER`, returns the value
* **BLOCK_COUNT** () equal to `IR_COUNT ("BB ")`, i.e. search specified basic blocks and counts the number
* **TRUE** (condition) ensure the condition is correct
* **SKIP_IF** (condition) if condition is `true`, skip all commands from that to end of this checker
* **ASM_METHOD** (name: string) select a specified method in disasm file, next "ASM*" checks will be applied only for this method's code.
* **ASM_INST** (inst: pattern) select a specified instruction in disasm file, next "ASM*" checks will be applied only for this instruction's code.
* **ASM/ASM_NEXT/ASM_NOT/ASM_NEXT_NOT** (inst: pattern) same as other similar checks, but search only in a current disasm scope, defined by `ASM_METHOD` or `ASM_INST`.
If none of these checks were specified, then search will be applied in the whole disasm file.
* **IN_BLOCK** (block: pattern) limits the search for instructions to one block. The block is defined by lines "props: ..." and "succs: ...". The search pattern is found in the first line "props: rest\_of\_line\_for\_matching". You can define only one block for searching and you can't return to the search in the entire method after in the currently analized compiler pass.

*pattern* can be a string(surrounded by quotes) or regex(surrounded by slashes): string - `"SearchPattern"`, regex - `/SearchPattern/`.
