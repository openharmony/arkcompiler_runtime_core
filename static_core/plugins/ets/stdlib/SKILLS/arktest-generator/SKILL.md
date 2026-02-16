---
name: arktest-generator
description: Generate ArkTest unit tests for stdlib APIs. Use when creating tests for ArkTS/ETS stdlib APIs, writing test cases, or generating test code. This skill handles the ArkTest framework including test suite creation with ArkTestsuite, test functions with assertions (assertEQ, assertTrue, expectError), proper main() returning suite.run(), async tests with addAsyncTest, and error handling tests. Places tests in ../tests/ets_func_tests/std/.
---

# ArkTest Generator

Generate unit tests for ArkTS stdlib APIs using the ArkTest framework.

## Prerequisites

**Review the API source code** in `std/` directory first to understand the interface, methods, parameters, return types, and behavior.

**Workflow**:
1. Review existing source code in `std/` directory
2. Generate test file using patterns from this skill
3. Place test in `../tests/ets_func_tests/std/` matching the API location

## Quick Start

Basic test file structure:

```typescript
function main(): int {
    let suite = new arktest.ArkTestsuite('MyAPITest');
    suite.addTest('basicTest', basicTest);
    return suite.run();
}

function basicTest() {
    let expected = <some_expected_result>;
    let result = API.method();
    arktest.assertEQ(result, expected);
}
```

## Test File Template

### Required Header

Every test file must include the Apache 2.0 license header:

```typescript
/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
```

### Main Function Pattern

Always return `suite.run()` - never constants:

```typescript
function main(): int {
    let suite = new arktest.ArkTestsuite('APINameTest');
    suite.addTest('testName', testFunction);
    return suite.run();
}
```

**Critical**: Always return the suite result from `main()`. Never return `true`, `0`, or constants.

## Assertions Reference

### Basic Assertions

- `arktest.assertEQ(value1, value2, comment?)` - Equality (===)
- `arktest.assertNE(value1, value2, comment?)` - Inequality (!==)
- `arktest.assertTrue(condition, comment?)` - Condition is true
- `arktest.assertFalse(condition, comment?)` - Condition is false
- `arktest.assertLT(value1, value2, comment?)` - Less than
- `arktest.assertLE(value1, value2, comment?)` - Less than or equal
- `arktest.assertDoubleEQ(v1, v2, absError, comment?)` - Float comparison

### Error Testing

- `arktest.expectError(fn, expect?)` - Expects Error thrown
  - `expect` can be string message or Error instance
- `arktest.expectThrow(fn, test?)` - Expects exception with custom test
- `arktest.expectNoThrow(fn)` - Ensures no error thrown

## Test Patterns

### Basic Functionality Test

```typescript
function stringIndexTest() {
    let str: string = 'abcd';
    arktest.assertEQ(str[0], 'a');
    arktest.assertEQ(str[1], 'b');
}
```

### Edge Case Test

```typescript
function boundsTest() {
    let str: string = 'test';
    try {
        let x = str[-1];
    } catch (e) {
        arktest.assertTrue(e instanceof StringIndexOutOfBoundsError);
    }
}
```

### Async Test

```typescript
function main(): int {
    let suite = new arktest.ArkTestsuite('AsyncTest');
    suite.addAsyncTest('asyncOp', asyncTest);
    return suite.run();
}

async function asyncTest(): Promise<void> {
    let result = await API.asyncMethod();
    arktest.assertEQ(result, expected);
}
```

For Promise<int> functions (0=success, 1=failure):

```typescript
suite.addTest('operationTest', () => {
    arktest.assertEQ(waitForCompletion(testFunction), 0);
});
```

### Container Test

```typescript
function containerTest() {
    let map = new HashMap<int, string>();
    map.put(1, 'one');
    arktest.assertEQ(map.get(1), 'one');
    arktest.assertEQ(map.size(), 1);
}
```

### Error Test

```typescript
function errorTest() {
    arktest.expectError(() => {
        throw new Error('Specific message');
    }, 'Specific message');
}
```

## File Organization

### Location

Place tests in `../tests/ets_func_tests/std/` matching stdlib structure:
- `std/core/` → `tests/ets_func_tests/std/core/`
- `std/containers/` → `tests/ets_func_tests/std/containers/`
- `std/concurrency/` → `tests/ets_func_tests/std/concurrency/`

### Naming

- Test file: `<APIName>Test.ets` (e.g., `StringGetTest.ets`)
- Test suite: Descriptive name matching API
- Test functions: `camelCase` descriptive names

### Multiple Suites

```typescript
function main(): int {
    let suite1 = new arktest.ArkTestsuite('APIPart1');
    let suite2 = new arktest.ArkTestsuite('APIPart2');

    suite1.addTest('test1', test1);
    suite2.addTest('test2', test2);

    let failures = 0;
    failures += suite1.run();
    failures += suite2.run();
    return failures;
}
```

## Code Style

- 4-space indentation (no tabs)
- PascalCase for types, camelCase for functions
- Explicit `public`/`private` modifiers
- `const` over `let` when possible
- Space after colon: `let x: number`
- Single empty line at end of file
- Always add `override` if method overrides Base class method

## Test Coverage

Include tests for:

1. **Basic functionality** - Normal usage, happy path
2. **Edge cases** - Boundary values, empty inputs, special characters
3. **Error handling** - Invalid inputs, type mismatches, index errors
4. **Async behavior** - Concurrent operations, timing (if applicable)
