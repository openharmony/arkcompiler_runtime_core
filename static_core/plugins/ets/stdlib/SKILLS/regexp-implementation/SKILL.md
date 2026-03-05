---
name: regexp-implementation
description: Comprehensive RegExp implementation covering both ETS managed API and native C++ bindings. Use when implementing, debugging, or extending RegExp functionality. Covers the full stack from ETS stdlib API through ANI bindings to PCRE2 backend.
---

# RegExp Implementation: Full Stack Guide

This skill covers the complete RegExp implementation stack from ETS managed code through native bindings to PCRE2.

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                        ETS Managed Layer                            │
│  std/core/RegExp.ets                                               │
│  - RegExp class, exec/test/match/replace/split/search              │
│  - RegExpResultArray, RegExpMatchArray, RegExpExecArray            │
└────────────────────────────────┬────────────────────────────────────┘
                                 │ native methods
                                 ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      ANI Native Bindings                            │
│  native/core/regexp/RegExp.cpp                                      │
│  - compile(), execImpl(), matchImpl()                                │
│  - ETS string → native conversion                                   │
│  - Result object construction                                       │
└────────────────────────────────┬────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      PCRE2 Executor Layer                           │
│  native/core/regexp/regexp_executor.cpp                             │
│  - Flag mapping to PCRE2                                            │
│  - Compile/Execute orchestration                                    │
│  native/core/regexp/regexp_8.cpp / regexp_16.cpp                   │
│  - UTF-8 / UTF-16 PCRE2 wrappers                                    │
└────────────────────────────────┬────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────┐
│                      Runtime Intrinsics                              │
│  plugins/ets/runtime/intrinsics/std_core_RegExp.cpp                 │
│  - StdCoreRegExpParse() - validates flags at construction          │
│  - Uses ark::RegExpParser for pattern validation                    │
└─────────────────────────────────────────────────────────────────────┘
```

## Key Files

| Layer | File | Purpose |
|-------|------|---------|
| Managed | `std/core/RegExp.ets` | ETS public API |
| Managed | `std/core/RegExp.ets` | Result types (RegExpResultArray, etc.) |
| Native | `native/core/regexp/RegExp.cpp` | ANI bindings |
| Native | `native/core/regexp/regexp_executor.cpp` | PCRE2 orchestration |
| Native | `native/core/regexp/regexp_8.cpp` | UTF-8 PCRE2 wrapper |
| Native | `native/core/regexp/regexp_16.cpp` | UTF-16 PCRE2 wrapper |
| Native | `native/core/regexp/regexp_exec_result.h` | Result structures |
| Intrinsics | `plugins/ets/runtime/intrinsics/std_core_RegExp.cpp` | Runtime parse |
| Parser | `static_core/runtime/regexp/ecmascript/regexp_parser.cpp` | Pattern validation |

## ETS API (Managed Layer)

### Core Classes

```typescript
// std/core/RegExp.ets

// Base result array - extends Array<String | undefined>
export class RegExpResultArray extends Array<String | undefined> {
    readonly isCorrect: boolean
    protected index_: int
    protected input_: String
    protected indices_: Array<Array<int>>
    public groups: RegExpGroupsContainer | undefined
}

// For String.match() and matchAll()
export class RegExpMatchArray extends RegExpResultArray {
    public get index(): int | undefined
    public get input(): String | undefined
}

// For RegExp.exec()
export class RegExpExecArray extends RegExpResultArray {
    public get index(): int
    public get input(): String
}

// Main RegExp class
export class RegExp extends Object {
    // Flags
    get flags(): String       // Sorted flags
    get global(): boolean     // 'g'
    get ignoreCase(): boolean // 'i'
    get multiline(): boolean  // 'm'
    get dotAll(): boolean    // 's'
    get unicode(): boolean   // 'u'
    get sticky(): boolean    // 'y'
    get hasIndices(): boolean // 'd'
    get unicodeSets(): boolean // 'v'
    
    get source(): String     // Pattern source
    public lastIndex: int   // Search start position
    
    // Methods
    public exec(str: String): RegExpExecArray | null
    public test(str: String): boolean
    public match(str: String): RegExpMatchArray | null
    public matchAll(str: String): IterableIterator<RegExpMatchArray>
    public replace(str: String, replaceValue: String): String
    public replace(str: String, replacer: (substr: String, args: Object[]) => String): String
    public split(str: String, limit: Int | undefined): String[]
    public search(str: String): int
    
    // Native methods
    private native execImpl(...): RegExpExecArray
    private native matchImpl(...): RegExpMatchArray
    private native static parse(pattern: String, flags: String): void
}
```

### Native Method Declarations

```typescript
// std/core/RegExp.ets lines 746-748
private native execImpl(
    pattern: String, 
    flags: String, 
    str: String, 
    patternLength: int, 
    strLength: int, 
    lastIndex: int, 
    hasSlashU: boolean
): RegExpExecArray;

private native matchImpl(
    pattern: String, 
    flags: String, 
    str: String, 
    patternLength: int, 
    strLength: int, 
    lastIndex: int, 
    hasSlashU: boolean
): RegExpMatchArray;

private native static parse(pattern: String, flags: String): void
```

## Native Bindings (ANI Layer)

### Registration (RegExp.cpp:400-416)

```cpp
// native/core/regexp/RegExp.cpp
void RegisterRegExpNativeMethods(ani_env *env)
{
    const auto regExpImpls = std::array {
        ani_native_function {"compile", ":C{std.core.RegExp}", reinterpret_cast<void *>(Compile)},
        ani_native_function {"execImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.RegExpExecArray}",
                             reinterpret_cast<void *>(Exec)},
        ani_native_function {"matchImpl",
                             "C{std.core.String}C{std.core.String}C{std.core.String}iiiz:C{std.core.RegExpMatchArray}",
                             reinterpret_cast<void *>(Match)}};
    // ... registration code
}
```

### ANI Signature Format

```
C{std.core.String}     = String parameter
i                      = int parameter  
z                      = boolean parameter
:C{std.core.RegExpExecArray} = returns RegExpExecArray
```

### String Conversion Helpers

```cpp
// native/core/regexp/RegExp.cpp lines 120-150
bool IsUtf16(ani_env *env, ani_string str)
{
    ark::ets::ani::ScopedManagedCodeFix s(env);
    auto internalString = s.ToInternalType(str);
    return internalString->IsUtf16();
}

std::vector<uint8_t> ExtractStringAsUtf16(ani_env *env, ani_string strANI, const bool isUtf16)
{
    // Converts ETS string to UTF-16 byte vector for PCRE2
}
```

### Flag Processing (RegExp.cpp:76-118)

```cpp
uint32_t CastToBitMask(ani_env *env, ani_string checkStr)
{
    uint32_t flagsBits = 0;
    for (const char &c : ConvertFromAniString(env, checkStr)) {
        switch (c) {
            case 'd': flagsBitsTemp = FLAG_HASINDICES; break;
            case 'g': flagsBitsTemp = FLAG_GLOBAL; break;
            case 'i': flagsBitsTemp = FLAG_IGNORECASE; break;
            case 'm': flagsBitsTemp = FLAG_MULTILINE; break;
            case 's': flagsBitsTemp = FLAG_DOTALL; break;
            case 'u': flagsBitsTemp = FLAG_UTF16; break;
            case 'v': flagsBitsTemp = FLAG_EXT_UNICODE; break;
            case 'y': flagsBitsTemp = FLAG_STICKY; break;
        }
    }
    return flagsBits;
}
```

## PCRE2 Executor

### Compile Flow (regexp_executor.cpp:29-63)

```cpp
bool EtsRegExp::Compile(const std::vector<uint8_t> &pattern, const bool isUtf16, const int len)
{
    uint32_t flags = 0U;
    if (flagMultiline_)    flags |= PCRE2_MULTILINE;
    if (flagCaseInsentitive_) {
        flags |= PCRE2_CASELESS;
        flags |= PCRE2_UCP;  // Unicode character properties
    }
    if (flagSticky_)       flags |= PCRE2_ANCHORED;
    if (flagDotAll_)       flags |= PCRE2_DOTALL;
    if (flagUnicode_)      flags |= PCRE2_UTF;
    if (flagVnicode_)      flags |= PCRE2_UCP;
    
    flags |= PCRE2_MATCH_UNSET_BACKREF;
    flags |= PCRE2_ALLOW_EMPTY_CLASS;
    
    if (utf16_) {
        re_ = RegExp16::CreatePcre2Object(...);
    } else {
        re_ = RegExp8::CreatePcre2Object(...);
    }
    return re_ != nullptr;
}
```

### Execute Flow (regexp_executor.cpp:65-82)

```cpp
RegExpExecResult EtsRegExp::Execute(const std::vector<uint8_t> &pattern, 
                                    const std::vector<uint8_t> &str, 
                                    const int len, const int startOffset)
{
    uint32_t matchFlags = 0U;
    if (flagUnicode_ || flagVnicode_) {
        matchFlags |= PCRE2_NO_UTF_CHECK;
    }
    
    RegExpExecResult result;
    if (utf16_) {
        result = RegExp16::Execute(re_, matchFlags, ...);
        RegExp16::EraseExtraGroups(...);
    } else {
        result = RegExp8::Execute(re_, matchFlags, ...);
        RegExp8::EraseExtraGroups(...);
    }
    return result;
}
```

### Result Construction (RegExp.cpp)

```cpp
// Converting native result to ETS object
ani_object DoExec(ani_env *env, ..., ExecData &execData)
{
    EtsRegExp executor;
    executor.SetFlags(flagsStr);
    executor.Compile(patternBytes, isUtf16Pattern, patternSize);
    
    auto execResult = executor.Execute(patternBytes, strBytes, strSize, startOffset);
    
    // Build RegExpExecArray with results
    // Populate resultRaw_ with "index,endIndex,index,endIndex,..." format
    // Populate groupsRaw_ with "name,start,end;name,start,end;..." format
}
```

## Runtime Intrinsics

### Pattern Validation (std_core_RegExp.cpp:37-84)

```cpp
extern "C" void StdCoreRegExpParse(EtsString *pattern, EtsString *flags)
{
    RegExpParser parse = RegExpParser();
    
    uint32_t nativeFlags = 0;
    for (char c : flagsStr) {
        switch (c) {
            case 'g': CheckAndSetFlag('g', ark::RegExpParser::FLAG_GLOBAL, nativeFlags); break;
            case 'i': CheckAndSetFlag('i', ark::RegExpParser::FLAG_IGNORECASE, nativeFlags); break;
            // ... other flags
        }
    }
    
    parse.Init(patternData, patternLen, nativeFlags);
    parse.Parse();
    if (parse.IsError()) {
        ThrowEtsException(..., parse.GetErrorMsg());
    }
}
```

This validates the regex pattern at construction time and throws SyntaxError for invalid patterns.

## Flow: Executing a Regex Match

```
1. User calls: regex.exec("hello world")
                    │
2. ETS RegExp.exec() [RegExp.ets:785]
                    │
3. Calls native execImpl() [RegExp.ets:763]
                    │
4. ANI binding: Exec() [RegExp.cpp]
                    │
5. Convert ETS strings to native [RegExp.cpp:ExtractStringAsUtf16]
                    │
6. Compile if needed [regexp_executor.cpp:Compile]
                    │
7. Execute against PCRE2 [regexp_executor.cpp:Execute]
                    │
8. Process results - build resultRaw_ and groupsRaw_
                    │
9. Return RegExpExecArray to ETS
                    │
10. RegExpResultArray.postExecProcessing() [RegExp.ets:97]
                    │
11. Update groups and indices from raw strings [RegExp.ets:184-219]
                    │
12. Return to user
```

## Implementing New Features

### Adding a New Native Method

1. **Add ANI binding in RegExp.cpp**:

```cpp
ani_int MyNewMethod(ani_env *env, ani_object regexp, ani_string input)
{
    // Get pattern and flags from regexp
    // Convert input string
    // Execute with PCRE2
    // Return result
}

ani_native_function {"myNewMethod", 
    "C{std.core.String}:i",  // input:String returns int
    reinterpret_cast<void *>(MyNewMethod)},
```

2. **Add ETS declaration in RegExp.ets**:

```typescript
public native myNewMethod(input: String): int
```

3. **Rebuild and test**:

```bash
cd ../../../
cmake --build build --target panda_bins etssdk
```

### Adding a New Flag

1. **ETS side - add getter in RegExp.ets**:

```typescript
get newFlag(): boolean {
    return this.flags_.contains("N", 0)
}
```

2. **ANI binding - update CastToBitMask in RegExp.cpp**:

```cpp
case 'N':
    flagsBitsTemp = FLAG_NEW_FLAG;
    break;
```

3. **PCRE2 executor - map in regexp_executor.cpp**:

```cpp
void EtsRegExp::SetFlag(const char &chr) {
    case 'N':
        SetIfNotSet(flagNew_);
        break;
}
```

4. **Map to PCRE2 in Compile()**:

```cpp
if (flagNew_) {
    flags |= PCRE2_NEW_FEATURE;
}
```

5. **Runtime intrinsics - add validation in std_core_RegExp.cpp**:

```cpp
case 'N':
    CheckAndSetFlag('N', ark::RegExpParser::FLAG_NEW, nativeFlags);
    break;
```

6. **Add to ark::RegExpParser flags in runtime**:

```cpp
// static_core/runtime/regexp/ecmascript/regexp_parser.h
static constexpr uint32_t FLAG_NEW = (1U << 7U);
```

## Result Data Flow

### Native → ETS Result Format

The native layer passes results as comma-separated strings to avoid object allocation:

```
resultRaw_ = "0,5,5,10"        // index,endIndex pairs for each capture
groupsRaw_ = "name1,0,5;name2,5,10"  // name,start,end for named groups
```

ETS parses these in `updateIndicesAndResult()` and `updateGroups()`:

```typescript
// RegExp.ets:184-219
private updateIndicesAndResult(hasIndices: boolean): void {
    const data = this.resultRaw_.split(',')
    for (let i = 0; i < data.length; i += 2) {
        const index = Int.parseInt(data[i], 10)
        const endIndex = Int.parseInt(data[i + 1], 10)
        // Create indices array and result entries
    }
}

// RegExp.ets:324-359
protected updateGroups(): void {
    for (let groupData of this.groupsRaw_.split(";")) {
        let unitData = groupData.split(",")
        let name = unitData[0]
        let index = Int.parseInt(unitData[1], 10)
        let endIndex = Int.parseInt(unitData[2], 10)
        // Populate groups object
    }
}
```

## Debugging

### Enable Logging

```cpp
// In regexp_executor.cpp - add debug output
#include <cstdio>

void EtsRegExp::Execute(...) {
    printf("RegExp Execute: pattern=%s, str=%s\n", pattern.data(), str.data());
    // ... execution
}
```

### Common Issues

| Issue | Cause | Solution |
|-------|-------|----------|
| Empty matches infinite loop | Empty string match not advancing | Use `advanceStringIndex()` for unicode |
| Groups undefined | Named groups not captured | Check PCRE2 name table extraction |
| UTF-16 issues | Wrong execution path | Ensure `utf16_` flag is set correctly |
| Pattern compilation fails | Invalid regex syntax | Check `StdCoreRegExpParse()` error |

### Test Location

Tests are in `../tests/ets_func_tests/std/core/`:
- `RegExp-test.ets` - Basic functionality
- `RegExpGroups-test.ets` - Named capture groups
- `RegExpFlags-test.ets` - Flag behavior
- `RegExpExec-test.ets` - exec() method
- `RegExpReplace-test.ets` - replace() method

Run:
```bash
cd ../../../
./runner.sh panda-int ets-func-tests --filter std/core/RegExp
```

## Performance Considerations

1. **Pattern compilation is expensive** - reuse compiled patterns
2. **UTF-8 path is faster** - use when input is ASCII
3. **JIT compilation available** - `pcre2_jit_compile()` for repeated matches
4. **Avoid catastrophic backtracking** - test with pathological inputs
5. **lastIndex matters** - reset for global matches

## References

- ETS API: `std/core/RegExp.ets`
- ANI Bindings: `native/core/regexp/RegExp.cpp`
- PCRE2 Wrapper: `native/core/regexp/regexp_executor.cpp`
- Runtime Intrinsics: `plugins/ets/runtime/intrinsics/std_core_RegExp.cpp`
- Parser: `static_core/runtime/regexp/ecmascript/regexp_parser.cpp`
- PCRE2 docs: https://www.pcre.org/current/doc/html/
