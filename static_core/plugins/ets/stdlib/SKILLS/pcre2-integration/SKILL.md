---
name: pcre2-integration
description: PCRE2 library integration for RegExp implementation in ETS stdlib. Use when implementing regex features, adding new regex flags, optimizing pattern matching, debugging native regex code, or extending RegExp API. Covers PCRE2 C API, pattern compilation, match execution, capture groups, UTF-8/UTF-16 handling, and native-ETS boundary.
---

# PCRE2 Integration

PCRE2 (Perl Compatible Regular Expressions 2) is the native regex engine backing ETS stdlib's `RegExp` class.

## Architecture

```
std/core/RegExp.ets (ETS API)
         |
         v
native/core/regexp/RegExp.cpp (ANI bindings)
         |
         v
native/core/regexp/regexp_executor.cpp (Executor)
         |
    +----+----+
    v         v
regexp_8.cpp  regexp_16.cpp (PCRE2 wrappers)
    |              |
    v              v
  PCRE2-8       PCRE2-16
```

## Key Files

| File | Purpose |
|------|---------|
| `std/core/RegExp.ets` | ETS public API, result types |
| `native/core/regexp/RegExp.cpp` | ANI native method bindings |
| `native/core/regexp/regexp_executor.cpp` | Compile/Execute orchestration |
| `native/core/regexp/regexp_8.cpp` | UTF-8 PCRE2 wrapper |
| `native/core/regexp/regexp_16.cpp` | UTF-16 PCRE2 wrapper |
| `native/core/regexp/regexp_exec_result.h` | Result struct definition |

## PCRE2 API Patterns

### Pattern Compilation

```cpp
// From regexp_8.cpp
Pcre2Obj RegExp8::CreatePcre2Object(const uint8_t *pattern, uint32_t flags, 
                                     uint32_t extraFlags, const int len) {
    PCRE2_SPTR patternStr = static_cast<PCRE2_SPTR>(pattern);
    PCRE2_SIZE errorOffset;
    
    auto *compileContext = pcre2_compile_context_create(nullptr);
    pcre2_set_compile_extra_options(compileContext, extraFlags);
    pcre2_set_newline(compileContext, PCRE2_NEWLINE_ANY);
    
    auto re = pcre2_compile(patternStr, len, flags, &errorNumber, 
                            &errorOffset, compileContext);
    pcre2_compile_context_free(compileContext);
    
    return reinterpret_cast<Pcre2Obj>(re);
}
```

### Match Execution

```cpp
RegExpExecResult RegExp8::Execute(Pcre2Obj re, uint32_t matchFlags, 
                                   const uint8_t *str, const int len,
                                   const int startOffset) {
    auto *expr = reinterpret_cast<pcre2_code *>(re);
    auto *matchData = pcre2_match_data_create_from_pattern(expr, nullptr);
    
    auto resultCount = pcre2_match(expr, str, len, startOffset, 
                                   matchFlags, matchData, nullptr);
    auto *ovector = pcre2_get_ovector_pointer(matchData);
    
    // Process results...
    
    pcre2_match_data_free(matchData);
    return result;
}
```

### Cleanup

```cpp
void RegExp8::FreePcre2Object(Pcre2Obj re) {
    pcre2_code_free(reinterpret_cast<pcre2_code *>(re));
}
```

## Flag Mapping

ETS flags map to PCRE2 flags in `regexp_executor.cpp`:

| ETS Flag | PCRE2 Flag |
|----------|------------|
| `m` (multiline) | `PCRE2_MULTILINE` |
| `i` (case insensitive) | `PCRE2_CASELESS \| PCRE2_UCP` |
| `y` (sticky) | `PCRE2_ANCHORED` |
| `s` (dotAll) | `PCRE2_DOTALL` |
| `u` (unicode) | `PCRE2_UTF` |
| `v` (vnicode) | `PCRE2_UCP` |

Default flags always set:
- `PCRE2_MATCH_UNSET_BACKREF`
- `PCRE2_ALLOW_EMPTY_CLASS`
- `PCRE2_EXTRA_ALT_BSUX` (extra options)

## Capture Groups

### Named Groups Extraction

```cpp
void RegExp8::ExtractGroups(Pcre2Obj expression, int count, 
                            RegExpExecResult &result, void *data) {
    PCRE2_SPTR nameTable;
    uint32_t nameEntrySize;
    uint32_t nameCount;
    
    auto *expr = reinterpret_cast<pcre2_code *>(expression);
    pcre2_pattern_info(expr, PCRE2_INFO_NAMETABLE, &nameTable);
    pcre2_pattern_info(expr, PCRE2_INFO_NAMEENTRYSIZE, &nameEntrySize);
    pcre2_pattern_info(expr, PCRE2_INFO_NAMECOUNT, &nameCount);
    
    for (uint32_t i = 0; i < nameCount; i++) {
        // Extract group name and index from nameTable
        // Map to result.groups
    }
}
```

### Result Structure

```cpp
// regexp_exec_result.h
struct RegExpExecResult {
    std::vector<std::pair<int, int>> matches;  // [start, end) pairs
    std::map<std::string, std::pair<int, int>> groups;  // named groups
    bool success;
};
```

## UTF-8 vs UTF-16

The executor selects implementation based on unicode flag or UTF-16 input:

```cpp
// regexp_executor.cpp
if (utf16_) {
    re_ = RegExp16::CreatePcre2Object(
        reinterpret_cast<const uint16_t *>(pattern.data()), 
        flags, extraFlags, len);
} else {
    re_ = RegExp8::CreatePcre2Object(pattern.data(), flags, extraFlags, len);
}
```

Width-specific constants:
```cpp
// regexp_8.cpp
constexpr int PCRE2_MATCH_DATA_UNIT_WIDTH = 2;
constexpr int PCRE2_CHARACTER_WIDTH = 1;
constexpr int PCRE2_GROUPS_NAME_ENTRY_SHIFT = 3;

// regexp_16.cpp
constexpr int PCRE2_MATCH_DATA_UNIT_WIDTH = 2;
constexpr int PCRE2_CHARACTER_WIDTH = 2;
constexpr int PCRE2_GROUPS_NAME_ENTRY_SHIFT = 4;
```

## Adding New Features

### 1. Add Native Method

In `native/core/regexp/RegExp.cpp`:
```cpp
// ANI binding
ani_long MyNewMethod(ani_env *env, /* params */) {
    // Implementation
    return result;
}

// Register in GetNMethods()
{"myNewMethod", "(Lstd/core/RegExp;)J", reinterpret_cast<void *>(MyNewMethod)},
```

### 2. Update ETS API

In `std/core/RegExp.ets`:
```typescript
public myNewFeature(): ReturnType {
    const result = native_myNewFeature()
    return processResult(result)
}

private native native_myNewFeature(): NativeResultType
```

### 3. Build and Test

```bash
cd ../../../
cmake --build build --target panda_bins etssdk
cd tests/tests-u-runner-2/
./runner.sh panda-int ets-func-tests --suite std.core.RegExp
```

## Common Patterns

### Compile Once, Execute Many

```cpp
// Compile pattern once
Pcre2Obj compiled = RegExp8::CreatePcre2Object(pattern, flags, extraFlags, len);

// Execute multiple times
for (const auto &input : inputs) {
    auto result = RegExp8::Execute(compiled, 0, input.data(), input.size(), 0);
    // Process result
}

// Free when done
RegExp8::FreePcre2Object(compiled);
```

### Error Handling

```cpp
int errorNumber = 0;
PCRE2_SIZE errorOffset = 0;
auto re = pcre2_compile(pattern, len, flags, &errorNumber, &errorOffset, nullptr);

if (re == nullptr) {
    // Get error message
    PCRE2_UCHAR buffer[256];
    pcre2_get_error_message(errorNumber, buffer, sizeof(buffer));
    // Handle error
}
```

### JIT Compilation (Optional)

```cpp
auto re = pcre2_compile(pattern, len, flags, &errorNumber, &errorOffset, nullptr);

// Enable JIT for repeated matches
int jitResult = pcre2_jit_compile(re, PCRE2_JIT_COMPLETE);
if (jitResult == 0) {
    // Use JIT-optimized matching
    resultCount = pcre2_jit_match(re, str, len, startOffset, 
                                  matchFlags, matchData, nullptr);
}
```

## Debugging

### Enable PCRE2 Debug Output

```cpp
// In CMakeLists.txt or compile flags
// -DPCRE2_DEBUG=1
```

### Pattern Info

```cpp
uint32_t captureCount;
pcre2_pattern_info(expr, PCRE2_INFO_CAPTURECOUNT, &captureCount);

uint32_t nameCount;
pcre2_pattern_info(expr, PCRE2_INFO_NAMECOUNT, &nameCount);

PCRE2_SPTR nameTable;
pcre2_pattern_info(expr, PCRE2_INFO_NAMETABLE, &nameTable);
```

## Testing

RegExp tests are in `../tests/ets_func_tests/std/core/`:
- `RegExp-test.ets` - Basic functionality
- `RegExpGroups-test.ets` - Named groups
- `RegExpFlags-test.ets` - Flag behavior

Run tests:
```bash
./runner.sh panda-int ets-func-tests --suite std.core.RegExp
```

## Performance Tips

1. **Reuse compiled patterns** - Compilation is expensive
2. **Use JIT** for repeated matches on same pattern
3. **Prefer UTF-8** when input is ASCII
4. **Limit backtracking** - Use atomic groups where possible
5. **Avoid catastrophic backtracking** - Test with pathological inputs

## References

- [PCRE2 Documentation](https://www.pcre.org/current/doc/html/)
- `native/core/regexp/regexp_8.cpp` - UTF-8 implementation
- `native/core/regexp/regexp_16.cpp` - UTF-16 implementation
- `std/core/RegExp.ets` - ETS API
