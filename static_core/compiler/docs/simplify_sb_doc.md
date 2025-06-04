# String Builder optimizations

## Overview

This set of optimizations targets String Builder usage specifics.

## Rationality

String Builder is used to construct a string out of smaller pieces. In some cases it is optimal to use String Builder to collect intermediate parts, but in other cases we prefer naive string concatenation, due to an overhead introduced by String Builder object.

## Dependence

* BoundsAnalysis
* AliasAnalysis
* LoopAnalysis
* DominatorsTree

## Algorithms

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

**Merge StringBuilder::append calls chain**

Consider a code sample like the following:

```TS
// String semantics
let result = str0 + str1

// StringBuilder semantics
let sb = new StringBuilder()
sb.append(str0)
sb.append(str1)
let result = sb.toString()
```

Here we call `StringBuilder::append` twice. Proposed algorith merges them into a single call to (in this case) `StringBuilder::append2`. Merging up to 4 consecutive calls supported.

Optimized example is equivalent to:

```TS
// StringBuilder semantics
let sb = new StringBuilder()
sb.append2(str0, str1)
let result = sb.toString()
```

**Merge StringBuilder objects chain**

Consider a code sample like the following:

```TS
// String semantics
let result0 = str00 + str01
let result = result0 + str10 + str11

// StringBuilder semantics
let sb0 = new StringBuilder()
sb0.append(str00)
sb0.append(str01)

let sb1 = new StringBuilder()
sb1.append(sb0.toString())
sb1.append(str10)
sb1.append(str11)
let result = sb1.toString()
```

Here we construct `result0` and `sb0` and use it only once as a first argument of the concatenation which comes next. As we can see, two `StringBuilder` objects created. Instead, we can use only one of them as follows:

```TS
// StringBuilder semantics
let sb0 = new StringBuilder()
sb0.append(str00)
sb0.append(str01)
sb0.append(str10)
sb0.append(str11)
let result = sb0.toString()
```

Proposed algorithm merges consecutive chain of `StringBuilder` objects into a single object (if possible).

## Pseudocode

**Complete algorithm**

```C#
function SimplifyStringBuilder(graph: Graph)
    foreach loop in graph
        OptimizeStringConcatenation(loop)

    OptimizeStringBuilderChain()

    foreach block in graph (in RPO)
        OptimizeStringBuilderToString(block)
        OptimizeStringConcatenation(block)
        OptimizeStringBuilderAppendChain(block)
```

Below we describe the algorithm in more details

**Remove unnecessary String Builder**

The algorithm works as follows: first we search for all the StringBuilder instances in a basic block, then we replace all their toString-call usages with instance constructor argument until we meet any other usage in RPO.
```C#
function OptimizeStringBuilderToString(block: BasicBlock)
    foreach ctor being StringBuilder constructor with String argument in block
        let instance be StringBuilder instance of ctor-call
        let arg be ctor-call string argument
        foreach usage of instance (in RPO)
            if usage is toString-call
                replace usage with arg
            else
                break
        if instance is not used
            remove ctor from block
            remove instance from block
```
**Replace String Builder with string concatenation**

The algorithm works as follows: first we search for all the StringBuilder instances in a basic block, then we check if the use of instance matches concatenation pattern for 2, 3, or 4 arguments. If so, we replace the whole use of StringBuilder object with concatenation intrinsics.
```C#
function OptimizeStringConcatenation(block: BasicBlock)
    foreach ctor being StringBuilder default constructor in block
        let instance be StringBuilder instance of ctor-call
        let match = MatchConcatenation(instance)
        let appendCount = match.appendCount be number of append-calls of instance
        let append = match.append be an array of append-calls of instance
        let toStringCall = match.toStringCall be toString-call of instance
        let concat01 = ConcatIntrinsic(append[0].input(1), append[1].input(1))
        remove append[0] from block
        remove append[1] from block
        switch appendCount
            case 2
                replace toStringCall with concat01
                break
            case 3
                let concat012 = ConcatIntrinsic(concat01, append[2].input(1))
                remove append[2] from block
                replace toStringCall with concat012
                break
            case 4
                let concat23 = ConcatIntrinsic(append[2].input(1), append[3].input(1))
                let concat0123 = ConcatIntrinsic(concat01, concat23)
                remove append[2] from block
                remove append[3] from block
                replace toStringCall with concat0123
        remove toStringCall from block
        remove ctor from block
        remove instance from block

function ConcatIntrinsic(arg0, arg1): IntrinsicInst
    return concatenation intrinsic for arg0 and arg1

type Match
    toStringCall: CallInst
    appendCount: Integer
    append: array of CallInst

function MatchConcatenation(instance: StringBuilder): Match
    let match: Match
    foreach usage of instance
        if usage is toString-call
            set match.toStringCall = usage
        elif usage is append-call
            add usage to match.append array
            increment match.appendCount
    return match
```
**Optimize concatenation loops**

The algorithm works as follows: first we recursively process all the inner loops of current loop, then we search for string concatenation patterns within a current loop. For each pattern found we reconnect StringBuilder usage instructions in a correct way (making them point to the only one instance we have chosen), move chosen String Builder object creation and initial string value appending to a loop pre-header, move chosen StringBuilder object toString-call to loop exit block. We cleanup unused instructions at the end.
```C#
function OptimizeStringConcatenation(loop: Loop)
    foreach innerLoop being inner loop of loop
        OptimizeStringConcatenation(innerLoop)
    let matches = MatchConcatenationLoop(loop)
    foreach match in matches)
        ReconnectInstructions(match)
        HoistInstructionsToPreHeader(match)
        HoistInstructionsToExitBlock(match)
    }
    Cleanup(loop, matches)

type Match
    accValue: PhiInst
    initialValue: Inst
    // instructions to be hoisted to preheader
    preheader: type
        instance: Inst
        appendAccValue: IntrinsicInst
    // instructions to be left inside loop
    loop: type
        appendIntrinsics: array of IntrinsicInst
    // instructions to be deleted
    temp: array of type
        toStringCall: Inst
        instance: Inst
        appendAccValue: IntrinsicInst
    // instructions to be hoisted to exit block
    exit: type
        toStringCall: Inst

function MatchConcatenationLoop(loop: Loop)
    let matches: array of Match
    foreach accValue being string accumulator in a loop
        let match: Match
        foreach instance being StringBuilder instance used to update accValue (in RPO)
            if match is empty
                // Fill preheader and exit parts of a match
                set match.accValue = accValue
                set match.initialValue = FindInitialValue(accValue)
                set match.exit.toStringCall = FindToStringCall(instance)
                set match.preheader.instance = instance
                set match.preheader.appendAccValue = FindAppendIntrinsic(instance, accValue)
                // Init loop part of a match
                add other append instrinsics to match.loop.appendInstrinsics array
            else
                // Fill loop and temporary parts of a match
                let temp: TemporaryInstructions
                set temp.instance = instance
                set temp.toStringCall = FindToStringCall(instance)
                foreach appendIntrinsic in FindAppendIntrinsics(instance)
                    if appendIntrinsic.input(1) is accValue
                        set temp.appendAccValue = appendIntrinsic
                    else
                        add appendIntricsic to match.loop.appendInstrinsics array
                add temp to match.temp array
        add match to matches array
    return matches

function ReconnectInstructions(match: Match)
    match.preheader.appendAcc.setInput(0, match.preheader.instance)
    match.preheader.appendAcc.setInput(1, be match.initialValue)
    match.exit.toStringCall.setInput(0, match.preheader.instance)
    foreach user being users of match.accValue outside loop
        user.replaceInput(match.accValue, match.exit.toStringCall)

function HoistInstructionsToPreHeader(match: Match)
    foreach inst in match.preheader
        hoist inst to loop preheader
    fix broken save states

function HoistInstructionsToExitBlock(match: Match)
    let exitBlock be to exit block of loop
    hoist match.exit.toStringCall to exitBlock
    foreach input being inputs of match.exit.toStringCall inside loop
        hoist input to exitBlock

function Cleanup(loop: Loop, matches: array of Match)
    foreach block in loop
        fix save states in block
    foreach match in matches
        foreach temp in match.temp
            foreach inst in temp
                remove inst
    foreach block in loop
        foreach phi in block
            if phi is not used
                remove phi from block
```

**Merge StringBuilder::append calls chain**

The algorithm works as follows. First, we find all the `StringBuilder` objects and their `append` calls in a current `block`. Second, we split vector of calls found into a groups of 2, 3, or 4 elements. We replace each group by a corresponding `StringBuilder::appendN` call.

```C#
function OptimizeStringBuilderAppendChain(block: BasicBlock)
    foreach [instance, calls] being StringBuilder instance and its vector of append calls in block
        foreach group being consicutive subvector of 2, 3, or 4 from calls
            replace group with instance.appendN call
```

**Merge StringBuilder objects chain**

The algorithm works as follows. The algorithm traverses blocks of graph in Post Order, and instructions of each block in Reverse Order, this allows us iteratively merge potentially long chains of `StringBuilder` objects into a single object. First, we search a pairs `[instance, inputInstance]` of `StringBuilder` objects which we can merge, merge condition is: last call to `inputInstance.toString()` appended as a first argument to `instance`. Then we remove first call to `StringBuilder::append` from `instance` and last call to `StringBuilder::toString` from `inputInstance`. We retarget remaining calls of `instance` to `inputInstance`.

```C#
function OptimizeStringBuilderChain()
    foreach block in graph in PO
        foreach two objects [instance, inputInstance] being a consicutive pair of Stringbuilders in block in reverse order
            if CanMerge(instance, inputInstance)
                let firstAppend be 1st StringBuilder::append call of instance
                let lastToString be last StringBuilder::toString call of inputInstance
                remove firstAppend from instance users
                remove lastToString from inputInstance users
                foreach call being user of instance
                    call.setInput(0, inputInstance) // retarget append call to inputInstance
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

(Save state and null check instructions are skipped for simplicity)
```
Method: std.core.String ETSGLOBAL::toString0(std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v5)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
    3.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v4)
    4.ref  NewObject 15300            v3, ss -> (v5, v10)
    5.void CallStatic 51211 std.core.StringBuilder::<ctor> v4, v0, ss
   10.ref  CallVirtual 51332 std.core.StringBuilder::toString v4, ss -> (v11)
   11.ref  Return                     v10
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```
IR after transformation:
```
Method: std.core.String ETSGLOBAL::toString0(std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v10)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
   10.ref  Return                     v0
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

**Replace String Builder with string concatenation**

ETS function example:
```TS
function concat0(a: String, b: String): String {
    return a + b;
}
```
IR before transformation:
```
Method: std.core.String ETSGLOBAL::concat0(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v10)
    1.ref  Parameter                  arg 1 -> (v13)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
    4.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v5)
    5.ref  NewObject 11355            v4, ss -> (v13, v10, v6)
    6.void CallStatic 60100 std.core.StringBuilder::<ctor> v5, ss
   10.ref  Intrinsic.StdCoreSbAppendString v5, v0, ss
   13.ref  Intrinsic.StdCoreSbAppendString v5, v1, ss
   16.ref  CallStatic 60290 std.core.StringBuilder::toString v5, ss -> (v17)
   17.ref  Return                     v16
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```
IR after transformation:
```
Method: std.core.String ETSGLOBAL::concat0(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v18)
    1.ref  Parameter                  arg 1 -> (v18)
succs: [bb 0]

BB 0  preds: [bb 1]
prop:
   18.ref  Intrinsic.StdCoreStringConcat2 v0, v1, ss -> (v17)
   17.ref  Return                     v18
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

**Optimize concatenation loops**

ETS function example:
```TS
function concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        str = str + a;
    return str;
}
```
IR before transformation:
```
Method: std.core.String ETSGLOBAL::concat_loop0(std.core.String, i32)

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v9p)
    1.i32  Parameter                  arg 1 -> (v10p)
    3.i64  Constant                   0x0 -> (v7p)
   30.i64  Constant                   0x1 -> (v29)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead
    4.ref  LoadString 63726           v5 -> (v8p)
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   7p.i32  Phi                        v3(bb0), v29(bb2) -> (v29, v13)
   8p.ref  Phi                        v4(bb0), v28(bb2) -> (v31, v12)
   9p.ref  Phi                        v0(bb0), v9p(bb2) -> (v12, v9p, v25)
  10p.i32  Phi                        v1(bb0), v10p(bb2) -> (v10p, v13)
   13.b    Compare GE i32             v7p, v10p -> (v14)
   14.     IfImm NE b                 v13, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   16.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v17)
   17.ref  NewObject 22178            v16, ss -> (v28, v25, v22)
   18.void CallStatic 60220 std.core.StringBuilder::<ctor> v17, ss
   22.ref  Intrinsic.StdCoreSbAppendString v17, v8p, ss
   25.ref  Intrinsic.StdCoreSbAppendString v17, v9p, ss
   28.ref  CallStatic 60410 std.core.StringBuilder::toString v17, ss -> (v11p, v8p)
   29.i32  Add                        v7p, v30 -> (v7p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   31.ref  Return                     v8p
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```
IR after transformation:
```
Method: std.core.String ETSGLOBAL::concat_loop0(std.core.String, i32)

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v25)
    1.i32  Parameter                  arg 1 -> (v40, v13)
    3.i64  Constant                   0x0 -> (v40, v7p)
   30.i64  Constant                   0x1 -> (v29)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead
    4.ref  LoadString 63726           ss -> (v22)
   16.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v17)
   17.ref  NewObject 22178            v16, ss -> (v25, v28, v22, v18)
   18.void CallStatic 60220 std.core.StringBuilder::<ctor> v17, ss
   22.ref  Intrinsic.StdCoreSbAppendString v17, v4, ss
   40.b    Compare GE i32             v3, v1 -> (v41)
   41.     IfImm NE b                 v40, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   7p.i32  Phi                        v3(bb0), v29(bb2) -> (v29)
   25.ref  Intrinsic.StdCoreSbAppendString v17, v0, ss
   29.i32  Add                        v7p, v30 -> (v13, v7p)
   13.b    Compare GE i32             v29, v1 -> (v14)
   14.     IfImm NE b                 v13, 0x0
succs: [bb 1, bb 2]

BB 1  preds: [bb 2, bb 0]
prop:
   28.ref  CallStatic 60410 std.core.StringBuilder::toString v17, ss
   31.ref  Return                     v28
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

**Merge StringBuilde::append calls chain**

ETS function example:
```TS
function append2(str0: string, str1: string): string {
    let sb = new StringBuilder();

    sb.append(str0);
    sb.append(str1);

    return sb.toString();
}
```
IR before transformation:
```
Method: std.core.String ETSGLOBAL::append2(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v13)
    1.ref  Parameter                  arg 1 -> (v14)
succs: [bb 0]

BB 0  preds: [bb 1]
    4.ref  LoadAndInitClass 'std.core.StringBuilder' v3 -> (v5)
    5.ref  NewObject 15705            v4, ss -> (v19, v16, v13, v6)
    6.void CallStatic 86153 std.core.StringBuilder::<ctor> v5, ss
   13.ref  Intrinsic.StdCoreSbAppendString v5, v0, ss
   16.ref  Intrinsic.StdCoreSbAppendString v5, v1, ss
   19.ref  Intrinsic.StdCoreSbToString v5, ss
   20.ref  Return                     v19
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```
IR after transformation:
```
Method: std.core.String ETSGLOBAL::append2(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v13)
    1.ref  Parameter                  arg 1 -> (v14)
succs: [bb 0]

BB 0  preds: [bb 1]
    4.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v5)
    5.ref  NewObject 15705            v4, ss -> (v19, v18, v6)
    6.void CallStatic 86153 std.core.StringBuilder::<ctor> v5, ss
   18.ref  Intrinsic.StdCoreSbAppendString2 v5, v0, v1, ss
   19.ref  Intrinsic.StdCoreSbToString v5, ss
   20.ref  Return                     v19
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

**Merge StringBuilder objects chain**

ETS function example:
```TS
function concat2(a: String, b: String): String {
    let sb0 = new StringBuilder()
    sb0.append(a)
    let sb1 = new StringBuilder()
    sb1.append(sb0.toString())
    sb1.append(b)
    return sb1.toString();
}
```
IR before transformation:
```
Method: std.core.String ETSGLOBAL::concat2(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v10)
    1.ref  Parameter                  arg 1 -> (v24)
succs: [bb 0]

BB 0  preds: [bb 1]
    4.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v5)
    5.ref  NewObject 12080            v4, ss -> (v18, v10, v6)
    6.void CallStatic 86229 std.core.StringBuilder::<ctor> v5, ss
   10.ref  Intrinsic.StdCoreSbAppendString v5, v0, ss
   12.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v13)
   13.ref  NewObject 12080            v12, ss -> (v27, v24, v21, v14)
   14.void CallStatic 86229 std.core.StringBuilder::<ctor> v13, ss
   18.ref  Intrinsic.StdCoreSbToString v5, ss -> (v21)
   21.ref  Intrinsic.StdCoreSbAppendString v13, v18, ss
   24.ref  Intrinsic.StdCoreSbAppendString v13, v1, ss
   27.ref  Intrinsic.StdCoreSbToString v13, ss -> (v28)
   28.ref  Return                     v27
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```
IR after transformation:
```
Method: std.core.String ETSGLOBAL::concat2(std.core.String, std.core.String)

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v10)
    1.ref  Parameter                  arg 1 -> (v24)
succs: [bb 0]

BB 0  preds: [bb 1]
    4.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v5)
    5.ref  NewObject 12080            v4, ss -> (v27, v24, v18, v10, v6)
    6.void CallStatic 86229 std.core.StringBuilder::<ctor> v5, ss
   10.ref  Intrinsic.StdCoreSbAppendString v5, v0, ss
   24.ref  Intrinsic.StdCoreSbAppendString v5, v1, ss
   27.ref  Intrinsic.StdCoreSbToString v5, ss -> (v28)
   28.ref  Return                     v27
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

## Links

* Implementation
    * [simplify_string_builder.h](../optimizer/optimizations/simplify_string_builder.h)
    * [simplify_string_builder.cpp](../optimizer/optimizations/simplify_string_builder.cpp)
* Tests
    * [ets_stringbuilder.ets](../../plugins/ets/tests/checked/ets_stringbuilder.ets)
    * [ets_string_builder_append_merge.ets](../../plugins/ets/tests/checked/ets_string_builder_append_merge.ets)
    * [ets_string_builder_merge.ets](../../plugins/ets/tests/checked/ets_string_builder_merge.ets)
    * [ets_string_concat.ets](../../plugins/ets/tests/checked/ets_string_concat.ets)
    * [ets_string_concat_loop.ets](../../plugins/ets/tests/checked/ets_string_concat_loop.ets)
