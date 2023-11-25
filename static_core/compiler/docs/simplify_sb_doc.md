# String Builder optimizations

## Overview

This set of optimizations targets String Builder usage specifics.

## Rationality

String Builder is used to construct a string out of smaller pieces. In some cases it is optimal to use String Builder to collect intermediate parts, but in other cases we prefer naive string concatenation, due to an overhead introduced by String Builder object.

## Dependence

* BoundsAnalysis
* AliasAnalysis
* LoopAnalysis

## Algorithm

**Remove unnecessary String Builder**

Example:
```TS
let input: String = ...
let sb = new StringBuilder(input)
let output = sb.toString()
```
Since there are no `StringBuilder::append()`-calls in between `constructor` and `toString()`-call, `input` string is equal to `output` string. So, the example code is equivalent to
```TS
let input: String = ...
let output = input
```

**Replace String Builder with string concatenation**

String concatenation expressed as a plus-operator over string operands turned into a String Builder usage by a frontend.

Example:
```TS
let a, b: String
...
let output = a + b
```
Frontend output equivalent:
```TS
let a, b: String
...
let sb = new StringBuilder()
sb.append(a)
sb.append(b)
let output = sb.toString()
```
The overhead of String Builder object exceeds the benefits of its usage (comparing to a naive string concatenation) for a small number of operands (two in the example above). So, we replace String Builder in such a cases back to naive string concatenation.

**Optimize concatenation loops**

Consider a string accumulation loop example:
```TS
let inputs: string[] = ... // array of strings
let output = ""
for (let input in inputs)
    output += input
```
Like in **Replace String Builder with string concatenation** section, frontend replaces string accumulation `output += input` with a String Builder usage, resulting in a huge performance degradation (comparing to a naive string concatenation), because *at each loop iteration* String Builder object is constructed, used to append two operands, builds resulting string, and discarded.

The equivalent code looks like the following:
```TS
let inputs: string[] = ... // array of strings
let output = ""
for (let input in inputs) {
    let sb = new StringBuilder()
    sb.append(output)
    sb.append(input)
    output = sb.toString()
}
```
To optimize cases like this, we implement the following equivalent transformation:
```TS
let inputs: string[] = ... // array of strings
let sb = new StringBuilder()
for (let input in inputs) {
    sb.append(input)
}
let output = sb.toString()
```

## Pseudocode

**Remove unnecessary String Builder**
```C#
foreach block in graph
    foreach ctor being StringBuilder constructor with String argument in block
        let instance be StringBuilder instance
        let arg be ctor-call argument
        foreach usage of instance
            if usage is toString-call
                replace usage with arg
            else
                break
        remove ctor if instance is not used anymore
```
**Replace String Builder with string concatenation**
```
To be added later
```
**Optimize concatenation loops**
```
To be added later
```

## Examples

**Remove unnecessary String Builder**

ETS function example:
```TS
function toString0(str: String): String {
    return new StringBuilder(str).toString();
}
```

IR before transformation:
```
Method: std.core.String ETSGLOBAL::toString0(std.core.String) 0x7fb0ff9f61f0

BB 1
prop: start, bc: 0x00000000
    0.ref  Parameter                  arg 0 -> (v6, v5, v2, v1)
    1.     SafePoint                  v0(vr1), inlining_depth=0
succs: [bb 0]

BB 0  preds: [bb 1]
prop: bc: 0x00000000
    2.     SaveState                  v0(vr1), inlining_depth=0 -> (v4, v3)
    3.ref  LoadAndInitClass 'std.core.StringBuilder' v2 -> (v4)
    4.ref  NewObject 15300            v3, v2 -> (v7, v9, v6, v5)
    6.     SaveState                  v0(vr1), v4(ACC), inlining_depth=0 -> (v5)
    5.void CallStatic 51211 std.core.StringBuilder::<ctor> v4, v0, v6
    7.     SaveState                  v4(ACC), inlining_depth=0 -> (v10, v9)
    9.ref  NullCheck                  v4, v7 -> (v10)
   10.ref  CallVirtual 51332 std.core.StringBuilder::toString v9, v7 -> (v11)
   11.ref  Return                     v10
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end, bc: 0x00000009
```
IR after transformation:
```
Method: std.core.String ETSGLOBAL::toString0(std.core.String) 0x7fb0ff9f61f0

BB 1
prop: start, bc: 0x00000000
    0.ref  Parameter                  arg 0 -> (v10, v2)
succs: [bb 0]

BB 0  preds: [bb 1]
prop: bc: 0x00000000
    2.     SaveState                  v0(vr1), inlining_depth=0 -> (v3)
    3.ref  LoadAndInitClass 'std.core.StringBuilder' v2
   10.ref  Return                     v0
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end, bc: 0x00000009
```

**Replace String Builder with string concatenation**

IR before transformation:
```
To be added later
```
IR after transformation:
```
To be added later
```

**Optimize concatenation loops**

IR before transformation:
```
To be added later
```
IR after transformation:
```
To be added later
```

## Links

* Implementation
    * [simplify_string_builder.h](../optimizer/optimizations/simplify_string_builder.h)
    * [simplify_string_builder.cpp](../optimizer/optimizations/simplify_string_builder.cpp)
* Tests
    * [ets_string_builder.ets](../../plugins/ets/tests/checked/ets_string_builder.ets)
    * ets_string_concat.ets (To be added later)
    * ets_string_concat_loop.ets (To be added later)

