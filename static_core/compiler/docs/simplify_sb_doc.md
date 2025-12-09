# String Builder optimizations

## Overview

This set of optimizations targets String Builder usage specifics.
All the optimizations described below preserve original behavior of String/StringBuilder operations and their side effects. This is tested by the wide set of tests (see Links Section below).

## Rationality

String Builder is used to construct a string out of smaller pieces. In some cases it is optimal to use String Builder to collect intermediate parts, but in other cases we prefer naive string concatenation, due to an overhead introduced by String Builder object.

## Dependence

### Dependence on analysis

* BoundsAnalysis
* AliasAnalysis
* LoopAnalysis
* DominatorsTree

### Dependence on stdlib

Optimizations depends on the following stdlib API classes and their methods:

* ```std.core.StringBuilder```
  * ```<ctor>```, may optimize out or emit in ```BCO, AOT, JIT```
  * ```append```, may optimize out or emit in ```BCO, AOT, JIT```
  * ```toString```, may optimize out or emit in ```BCO, AOT, JIT```
  * ```length```, field accessed by IR LoadObject when possible in ```AOT, JIT```
  * ```stringLength```, may emit this call instead of length field access in ```BCO```
* ```std.core.String```
  * ```length```, may replace with StringBuilder stringLength() in ```BCO``` or length in ```AOT, JIT```
  * ```getLength```, may replace with StringBuilder stringLength() in ```BCO```or length in ```AOT, JIT```

### Dependence on compiler intrinsics

Optimizations depends on the following compiler intrinsics:

* ```Intrinsic```
  * ```StdCoreSbAppendString```, may optimize out or emit in ```AOT, JIT```
  * ```StdCoreSbAppendString2```, may emit in ```AOT, JIT```
  * ```StdCoreSbAppendString3```, may emit in ```AOT, JIT```
  * ```StdCoreSbAppendString4```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat2```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat3```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat4```, may emit in ```AOT, JIT```

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

**Replace String length accessors with StringBuilder versions**

While performing above optimizations calls to std.core.String.length or getLength() will be replaced with StringBuilder.length field access or StringBuilder.stringLength() method.
Consider a code sample like the following:

```TS
    let input: String = ...
    let output: String = "";
    for (let i = 0; i < n; ++i) {
        output += output.length; // or += output.getLength();
        output += input;
    }
```

Equivalent code after transformation will be the following:

```TS
    let input: String = ...
    let sb = new StringBuilder()
    for (let i = 0; i < n; ++i) {
        sb.append(sb.stringLength()); // or sb.length in AOT, JIT
        sb.append(input);
    }
    let output = sb.toString()
```

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

ETS checked test ets_stringbuilder.ets, function example:

```TS
function toString0(str: String): String {
    return new StringBuilder(str).toString();
}
```

BCO IR before transformation:

(Save state and null check instructions are skipped for simplicity)

```
Method: ets_stringbuilder.ETSGLOBAL::toString0

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v4)
r253 -> r253 [ref]
succs: [bb 0]

BB 0  preds: [bb 1]
prop: bc
    2.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v3)
    3.ref  NewObject 544              v2, ss -> (v7, v4)
    4.void CallStatic 762 std.core.StringBuilder::<ctor> v3, v0, ss
    7.ref  CallStatic 809 std.core.StringBuilder::toString v3, ss -> (v9)
    9.ref  Return                     v7
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

BCO IR after transformation:

```
Method: ets_stringbuilder.ETSGLOBAL::toString0

BB 1
prop: start
    0.ref  Parameter                  arg 0 -> (v9)
r253 -> r253 [ref]
succs: [bb 0]

BB 0  preds: [bb 1]
prop
    9.ref  Return                     v0
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

AOT IR before transformation:

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
    4.ref  NewObject 15300             v3, ss -> (v5, v10)
    5.void CallStatic 51211 std.core.StringBuilder::<ctor> v4, v0, ss
   10.ref  Intrinsic.StdCoreSbToString v4, ss -> (v11)
   11.ref  Return                      v10
succs: [bb 2]

BB 2  preds: [bb 0]
prop: end
```

AOT IR after transformation:

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

ETS checked test ets_string_concat.ets, function example:

```TS
function concat0(a: String, b: String): String {
    return a + b;
}
```

Optimization in BCO is not supported since there are no append intrinsics yet to replace them with Intrinsic.StdCoreStringConcat2.

AOT IR before transformation:

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

AOT IR after transformation:

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

ETS checked test ets_string_concat_loop.ets, function example:

```TS
function concat_loop0(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i)
        str = str + a;
    return str;
}
```

BCO IR before transformation:

```
Method: ets_string_concat_loop.ETSGLOBAL::concat_loop0

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v24)
r252 -> r252 [ref]
    1.i32  Parameter                  arg 1 -> (v12)
r253 -> r253 [u32]
    5.i32  Constant                   0x0 -> (v8p)
   28.i32  Constant                   0x1 -> (v29)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    2.     SaveState                  inlining_depth=0
    4.     SaveState                  inlining_depth=0 -> (v3)
    3.ref  LoadString 857             v4 -> (v7p)
    6.     SaveStateDeoptimize        inlining_depth=0
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   7p.ref  Phi                        v3(bb0), v27(bb2) -> (v31, v21)
   8p.i32  Phi                        v5(bb0), v29(bb2) -> (v29, v12)
   12.b    Compare LE i32             v1, v8p -> (v13)
   13.     IfImm NE b                 v12, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   14.     SaveState                  inlining_depth=0 -> (v16, v15)
   15.ref  LoadAndInitClass 'std.core.StringBuilder' v14 -> (v16)
   16.ref  NewObject 645              v15, v14 -> (v27, v24, v21, v17)
   18.     SaveState                  inlining_depth=0 -> (v17)
   17.void CallStatic 761 std.core.StringBuilder::<ctor> v16, v18
   19.     SaveState                  inlining_depth=0 -> (v21)
   21.ref  CallStatic 799 std.core.StringBuilder::append v16, v7p, v19
   22.     SaveState                  inlining_depth=0 -> (v24)
   24.ref  CallStatic 799 std.core.StringBuilder::append v16, v0, v22
   25.     SaveState                  inlining_depth=0 -> (v27)
   27.ref  CallStatic 829 std.core.StringBuilder::toString v16, v25 -> (v7p)
   29.i32  Add                        v8p, v28 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   30.     SaveState                  inlining_depth=0
   31.ref  Return                     v7p
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

BCO IR after transformation:

```
Method: ets_string_concat_loop.ETSGLOBAL::concat_loop0

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v24)
r252 -> r252 [ref]
    1.i32  Parameter                  arg 1 -> (v12)
r253 -> r253 [u32]
    5.i32  Constant                   0x0 -> (v8p)
   28.i32  Constant                   0x1 -> (v29)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    2.     SaveState                  inlining_depth=0
    4.     SaveState                  inlining_depth=0 -> (v3)
    3.ref  LoadString 857             v4 -> (v21)
    6.     SaveStateDeoptimize        inlining_depth=0
   32.     SaveState                  inlining_depth=0 -> (v15)
   15.ref  LoadAndInitClass 'std.core.StringBuilder' v32 -> (v16)
   33.     SaveState                  inlining_depth=0 -> (v16)
   16.ref  NewObject 645              v15, v33 -> (v21, v27, v24, v17)
   34.     SaveState                  inlining_depth=0 -> (v17)
   17.void CallStatic 761 std.core.StringBuilder::<ctor> v16, v34
   35.     SaveState                  inlining_depth=0 -> (v21)
   21.ref  CallStatic 799 std.core.StringBuilder::append v16, v3, v35
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   8p.i32  Phi                        v5(bb0), v29(bb2) -> (v29, v12)
   12.b    Compare LE i32             v1, v8p -> (v13)
   13.     IfImm NE b                 v12, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   22.     SaveState                  inlining_depth=0 -> (v24)
   24.ref  CallStatic 799 std.core.StringBuilder::append v16, v0, v22
   29.i32  Add                        v8p, v28 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   30.     SaveState                  inlining_depth=0 -> (v27)
   27.ref  CallStatic 829 std.core.StringBuilder::toString v16, v30 -> (v31)
   31.ref  Return                     v27
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end

```

AOT IR before transformation:

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

AOT IR after transformation:

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

**Merge StringBuilder::append calls chain**

ETS checked test ets_string_builder_append_merge_part4.ets, function example:

```TS
function append2(str0: string, str1: string): string {
    let sb = new StringBuilder();

    sb.append(str0);
    sb.append(str1);

    return sb.toString();
}
```

Optimization in BCO is not supported since there are no append intrinsics yet to replace them with Intrinsic.StdCoreSbAppendString2.

AOT IR before transformation:

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

AOT IR after transformation:

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

ETS checked test ets_string_concat.ets, function example:

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

Optimization in BCO is not supported since there are no append intrinsics yet to replace them with Intrinsic.StdCoreStringConcat2.

AOT IR before transformation:

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

AOT IR after transformation:

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

**Replace String length accessors with StringBuilder versions**

ETS checked test ets_string_concat_loop.ets, function example:

```TS
function reuse_concat_loop1(a: String, n: int): String {
    let str: String = "";
    for (let i = 0; i < n; ++i) {
        str += str.length;
        str += a;
    }
    return str;
}
```

BCO IR before transformation:

```
Method: ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v44)
r252 -> r252 [ref]
    1.i32  Parameter                  arg 1 -> (v12)
r253 -> r253 [u32]
    5.i32  Constant                   0x0 -> (v8p)
   51.i32  Constant                   0x1 -> (v52)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    2.     SaveState                  inlining_depth=0
    4.     SaveState                  inlining_depth=0 -> (v3)
    3.ref  LoadString 857             v4 -> (v7p)
    6.     SaveStateDeoptimize        inlining_depth=0
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   7p.ref  Phi                        v3(bb0), v47(bb2) -> (v24, v54, v21)
   8p.i32  Phi                        v5(bb0), v52(bb2) -> (v52, v12)
   12.b    Compare LE i32             v1, v8p -> (v13)
   13.     IfImm NE b                 v12, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   14.     SaveState                  inlining_depth=0 -> (v16, v15)
   15.ref  LoadAndInitClass 'std.core.StringBuilder' v14 -> (v16)
   16.ref  NewObject 645              v15, v14 -> (v30, v27, v21, v17)
   18.     SaveState                  inlining_depth=0 -> (v17)
   17.void CallStatic 761 std.core.StringBuilder::<ctor> v16, v18
   19.     SaveState                  inlining_depth=0 -> (v21)
   21.ref  CallStatic 799 std.core.StringBuilder::append v16, v7p, v19
   22.     SaveState                  inlining_depth=0 -> (v24)
   24.i32  CallStatic 732 std.core.String::%%get-length v7p, v22 -> (v27)
   25.     SaveState                  inlining_depth=0 -> (v27)
   27.ref  CallStatic 789 std.core.StringBuilder::append v16, v24, v25
   28.     SaveState                  inlining_depth=0 -> (v30)
   30.ref  CallStatic 829 std.core.StringBuilder::toString v16, v28 -> (v41, v33)
   31.     SaveState                  inlining_depth=0 -> (v33, v32)
   32.ref  LoadClass 'std.core.String' v31 -> (v33)
   33.     CheckCast 626              v30, v32, v31
   34.     SaveState                  inlining_depth=0 -> (v36, v35)
   35.ref  LoadAndInitClass 'std.core.StringBuilder' v34 -> (v36)
   36.ref  NewObject 645              v35, v34 -> (v47, v44, v41, v37)
   38.     SaveState                  inlining_depth=0 -> (v37)
   37.void CallStatic 761 std.core.StringBuilder::<ctor> v36, v38
   39.     SaveState                  inlining_depth=0 -> (v41)
   41.ref  CallStatic 799 std.core.StringBuilder::append v36, v30, v39
   42.     SaveState                  inlining_depth=0 -> (v44)
   44.ref  CallStatic 799 std.core.StringBuilder::append v36, v0, v42
   45.     SaveState                  inlining_depth=0 -> (v47)
   47.ref  CallStatic 829 std.core.StringBuilder::toString v36, v45 -> (v7p, v50)
   48.     SaveState                  inlining_depth=0 -> (v50, v49)
   49.ref  LoadClass 'std.core.String' v48 -> (v50)
   50.     CheckCast 626              v47, v49, v48
   52.i32  Add                        v8p, v51 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   53.     SaveState                  inlining_depth=0
   54.ref  Return                     v7p
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

BCO IR after transformation:

```
Method: ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v44)
r252 -> r252 [ref]
    1.i32  Parameter                  arg 1 -> (v12)
r253 -> r253 [u32]
    5.i32  Constant                   0x0 -> (v8p)
   51.i32  Constant                   0x1 -> (v52)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    2.     SaveState                  inlining_depth=0
    4.     SaveState                  inlining_depth=0 -> (v3)
    3.ref  LoadString 857             v4 -> (v21)
    6.     SaveStateDeoptimize        inlining_depth=0
   57.     SaveState                  inlining_depth=0 -> (v15)
   15.ref  LoadAndInitClass 'std.core.StringBuilder' v57 -> (v16)
   58.     SaveState                  inlining_depth=0 -> (v16)
   16.ref  NewObject 645              v15, v58 -> (v55, v44, v21, v30, v27, v17)
   59.     SaveState                  inlining_depth=0 -> (v17)
   17.void CallStatic 761 std.core.StringBuilder::<ctor> v16, v59
   60.     SaveState                  inlining_depth=0 -> (v21)
   21.ref  CallStatic 799 std.core.StringBuilder::append v16, v3, v60
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1, bc: 0x0000000b
hotness=0
   8p.i32  Phi                        v5(bb0), v52(bb2) -> (v52, v12)
   12.b    Compare LE i32             v1, v8p -> (v13)
   13.     IfImm NE b                 v12, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1, bc: 0x00000014
hotness=0
   56.     SaveState                  inlining_depth=0 -> (v55)
   55.i32  CallStatic 770 std.core.StringBuilder::%%get-stringLength v16, v56 -> (v27)
   25.     SaveState                  inlining_depth=0 -> (v27)
   27.ref  CallStatic 789 std.core.StringBuilder::append v16, v55, v25
   34.     SaveState                  inlining_depth=0 -> (v35)
   35.ref  LoadAndInitClass 'std.core.StringBuilder' v34
   42.     SaveState                  inlining_depth=0 -> (v44)
   44.ref  CallStatic 799 std.core.StringBuilder::append v16, v0, v42
   48.     SaveState                  inlining_depth=0 -> (v49)
   49.ref  LoadClass 'std.core.String' v48
   52.i32  Add                        v8p, v51 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop: bc: 0x00000074
hotness=0
   53.     SaveState                  inlining_depth=0 -> (v30)
   30.ref  CallStatic 829 std.core.StringBuilder::toString v16, v53 -> (v54, v33)
   62.     SaveState                  inlining_depth=0 -> (v32)
   32.ref  LoadClass 'std.core.String' v62 -> (v33)
   61.     SaveState                  inlining_depth=0 -> (v33)
   33.     CheckCast 626              v30, v32, v61
   54.ref  Return                     v30
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

AOT IR before transformation:

```
Method: std.core.String ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1(std.core.String, i32)

BB 4
prop: start
hotness=0
    0.ref  Parameter                  arg 0 -> (v16, v20, v21, v24, v29, v32, v35, v38, v42, v43, v48, v52, v57, v46, v49, v13, v7, v6, v3, v2)
    1.i32  Parameter                  arg 1 -> (v14, v16, v20, v21, v24, v29, v32, v35, v38, v42, v43, v57, v49, v46, v52, v7, v6, v3)
    2.     SafePoint                  v0(vr4), inlining_depth=0
    4.i64  Constant                   0x0 -> (v8p, v7, v6)
   27.i64  Constant                   0x2 -> (v28)
   56.i64  Constant                   0x1 -> (v55)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    3.     SaveState                  v0(vr4), v1(vr5), inlining_depth=0
    6.     SaveState                  v4(vr0), v0(vr4), v1(vr5), inlining_depth=0 -> (v5)
    5.ref  LoadString 869             v6 -> (v9p, v7, v7)
    7.     SaveStateDeoptimize        v4(vr0), v5(vr1), v0(vr4), v1(vr5), v5(ACC), inlining_depth=0
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   8p.i32  Phi                        v4(bb0), v55(bb2) -> (v55, v52, v49, v46, v43, v42, v38, v35, v32, v29, v24, v21, v20, v16, v16, v14)
   9p.ref  Phi                        v5(bb0), v51(bb2) -> (v58, v57, v32, v29, v25, v24, v23, v21, v20, v16, v13)
   13.     SafePoint                  v0(vr4), v9p(vr1), inlining_depth=0
   14.b    Compare GE i32             v8p, v1 -> (v15)
   15.     IfImm NE b                 v14, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   16.     SaveState                  v8p(vr0), v9p(vr1), v0(vr4), v1(vr5), v8p(ACC), inlining_depth=0 -> (v18, v17)
   17.ref  LoadAndInitClass 'std.core.StringBuilder' v16 -> (v18)
   18.ref  NewObject 657              v17, v16 -> (v33, v32, v30, v29, v24, v22, v21, v21, v20, v19)
   20.     SaveState                  v8p(vr0), v9p(vr1), v0(vr4), v1(vr5), v18(ACC), inlining_depth=0 -> (v19)
   19.void CallStatic 773 std.core.StringBuilder::<ctor> v18, v20
   21.     SaveState                  v8p(vr0), v9p(vr1), v18(vr3), v0(vr4), v1(vr5), v18(ACC), inlining_depth=0 -> (v23, v22)
   22.ref  NullCheck                  v18, v21 -> (v23)
   23.ref  Intrinsic.StdCoreSbAppendString v22, v9p, v21
   24.     SaveState                  v8p(vr0), v9p(vr1), v18(vr3), v0(vr4), v1(vr5), inlining_depth=0 -> (v25)
   25.ref  NullCheck                  v9p, v24 -> (v26)
   
   #       std.core.String.length
   26.i32  LenArray                   v25 -> (v28)
   28.i32  Shr                        v26, v27 -> (v31, v29)

   29.     SaveState                  v8p(vr0), v9p(vr1), v18(vr3), v0(vr4), v1(vr5), v28(ACC), inlining_depth=0 -> (v31, v30)
   30.ref  NullCheck                  v18, v29 -> (v31)
   31.ref  Intrinsic.StdCoreSbAppendInt v30, v28, v29
   32.     SaveState                  v8p(vr0), v9p(vr1), v18(vr3), v0(vr4), v1(vr5), inlining_depth=0 -> (v34, v33)
   33.ref  NullCheck                  v18, v32 -> (v34)
   34.ref  Intrinsic.StdCoreSbToString v33, v32 -> (v35, v38, v45, v43, v42, v38, v37, v35)
   35.     SaveState                  v8p(vr0), v34(vr1), v34(ACC), v0(vr4), v1(vr5), inlining_depth=0 -> (v37, v36)
   36.ref  LoadClass 'std.core.String' v35 -> (v37)
   37.     CheckCast 638              v34, v36, v35
   38.     SaveState                  v8p(vr0), v34(vr1), v34(ACC), v0(vr4), v1(vr5), inlining_depth=0 -> (v40, v39)
   39.ref  LoadAndInitClass 'std.core.StringBuilder' v38 -> (v40)
   40.ref  NewObject 657              v39, v38 -> (v42, v43, v50, v49, v47, v46, v44, v43, v41)
   42.     SaveState                  v8p(vr0), v34(vr1), v40(ACC), v0(vr4), v1(vr5), inlining_depth=0 -> (v41)
   41.void CallStatic 773 std.core.StringBuilder::<ctor> v40, v42
   43.     SaveState                  v8p(vr0), v34(vr1), v40(vr2), v40(ACC), v0(vr4), v1(vr5), inlining_depth=0 -> (v45, v44)
   44.ref  NullCheck                  v40, v43 -> (v45)
   45.ref  Intrinsic.StdCoreSbAppendString v44, v34, v43
   46.     SaveState                  v8p(vr0), v1(vr5), v40(vr2), v0(vr4), inlining_depth=0 -> (v48, v47)
   47.ref  NullCheck                  v40, v46 -> (v48)
   48.ref  Intrinsic.StdCoreSbAppendString v47, v0, v46
   49.     SaveState                  v8p(vr0), v1(vr5), v40(vr2), v0(vr4), inlining_depth=0 -> (v51, v50)
   50.ref  NullCheck                  v40, v49 -> (v51)
   51.ref  Intrinsic.StdCoreSbToString v50, v49 -> (v52, v9p, v54, v52)
   52.     SaveState                  v8p(vr0), v51(vr1), v1(vr5), v51(ACC), v0(vr4), inlining_depth=0 -> (v54, v53)
   53.ref  LoadClass 'std.core.String' v52 -> (v54)
   54.     CheckCast 638              v51, v53, v52
   55.i32  Add                        v8p, v56 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   57.     SaveState                  v1(vr5), v9p(vr1), v0(vr4), inlining_depth=0
   58.ref  Return                     v9p
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

AOT IR after transformation:

```
Method: std.core.String ets_string_concat_loop.ETSGLOBAL::reuse_concat_loop1(std.core.String, i32)

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v66, v65, v64, v60, v61, v62, v63, v29, v38, v43, v21, v24, v48, v52, v57, v46, v49, v13, v7, v6, v2)
    1.i32  Parameter                  arg 1 -> (v66, v65, v64, v60, v61, v62, v63, v29, v38, v43, v21, v14, v24, v57, v49, v46, v52, v7, v6)
    2.     SafePoint                  v0(vr4), inlining_depth=0
    4.i64  Constant                   0x0 -> (v60, v61, v62, v63, v8p, v7, v6)
   56.i64  Constant                   0x1 -> (v55)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    6.     SaveState                  v4(vr0), v0(vr4), v1(vr5), inlining_depth=0 -> (v5)
    5.ref  LoadString 869             v6 -> (v60, v61, v62, v63, v23, v7, v7)
    7.     SaveStateDeoptimize        v4(vr0), v5(vr1), v0(vr4), v1(vr5), v5(ACC), inlining_depth=0
   60.     SaveState                  v4(vr0), v0(vr4), v1(vr5), v5(bridge), inlining_depth=0 -> (v17)
   17.ref  LoadAndInitClass 'std.core.StringBuilder' v60 -> (v18)
   61.     SaveState                  v4(vr0), v0(vr4), v1(vr5), v5(bridge), inlining_depth=0 -> (v18)
   18.ref  NewObject 657              v17, v61 -> (v49, v46, v52, v38, v13, v21, v24, v29, v43, v57, v64, v52, v38, v46, v49, v43, v43, v62, v63, v29, v43, v21, v21, v46, v49, v13, v25, v44, v47, v50, v23, v33, v30, v24, v22, v19)
   62.     SaveState                  v4(vr0), v0(vr4), v1(vr5), v18(bridge), v5(bridge), inlining_depth=0 -> (v19)
   19.void CallStatic 773 std.core.StringBuilder::<ctor> v18, v62
   63.     SaveState                  v4(vr0), v0(vr4), v1(vr5), v18(bridge), v5(bridge), inlining_depth=0 -> (v23)
   23.ref  Intrinsic.StdCoreSbAppendString v18, v5, v63
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
   8p.i32  Phi                        v4(bb0), v55(bb2) -> (v29, v38, v43, v21, v55, v52, v49, v46, v24, v14)
   13.     SafePoint                  v0(vr4), v18(bridge), v18(bridge), inlining_depth=0
   14.b    Compare GE i32             v8p, v1 -> (v15)
   15.     IfImm NE b                 v14, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   21.     SaveState                  v8p(vr0), v18(bridge), v18(vr3), v0(vr4), v1(vr5), v18(ACC), inlining_depth=0 -> (v22)
   22.ref  NullCheck                  v18, v21
   24.     SaveState                  v8p(vr0), v18(bridge), v18(vr3), v0(vr4), v1(vr5), inlining_depth=0 -> (v25)
   25.ref  NullCheck                  v18, v24 -> (v59)
   59.i32  LoadObject 786196 std.core.StringBuilder.length v25 -> (v29, v31)
   29.     SaveState                  v8p(vr0), v18(bridge), v18(vr3), v0(vr4), v1(vr5), v59(ACC), inlining_depth=0 -> (v31, v30)
   30.ref  NullCheck                  v18, v29 -> (v31)
   31.ref  Intrinsic.StdCoreSbAppendInt v30, v59, v29
   38.     SaveState                  v8p(vr0), v18(bridge), v18(bridge), v0(vr4), v1(vr5), inlining_depth=0 -> (v39)
   39.ref  LoadAndInitClass 'std.core.StringBuilder' v38
   43.     SaveState                  v8p(vr0), v18(bridge), v18(vr2), v18(ACC), v0(vr4), v1(vr5), v18(bridge), inlining_depth=0 -> (v44)
   44.ref  NullCheck                  v18, v43
   46.     SaveState                  v8p(vr0), v1(vr5), v18(vr2), v0(vr4), v18(bridge), v18(bridge), inlining_depth=0 -> (v48, v47)
   47.ref  NullCheck                  v18, v46 -> (v48)
   48.ref  Intrinsic.StdCoreSbAppendString v47, v0, v46
   49.     SaveState                  v8p(vr0), v1(vr5), v18(vr2), v0(vr4), v18(bridge), v18(bridge), inlining_depth=0 -> (v50)
   50.ref  NullCheck                  v18, v49
   52.     SaveState                  v8p(vr0), v18(bridge), v1(vr5), v18(bridge), v0(vr4), inlining_depth=0 -> (v53)
   53.ref  LoadClass 'std.core.String' v52
   55.i32  Add                        v8p, v56 -> (v8p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop:
   57.     SaveState                  v1(vr5), v18(bridge), v0(vr4), inlining_depth=0 -> (v34)
   64.     SaveState                  v1(vr5), v18(bridge), v0(vr4), inlining_depth=0 -> (v33)
   33.ref  NullCheck                  v18, v64 -> (v34)
   34.ref  Intrinsic.StdCoreSbToString v33, v57 -> (v65, v66, v58, v37)
   66.     SaveState                  v1(vr5), v34(bridge), v0(vr4), inlining_depth=0 -> (v36)
   36.ref  LoadClass 'std.core.String' v66 -> (v37)
   65.     SaveState                  v1(vr5), v34(bridge), v0(vr4), inlining_depth=0 -> (v37)
   37.     CheckCast 638              v34, v36, v65
   58.ref  Return                     v34
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

## Links

* Implementation
  * [simplify_string_builder.h](../optimizer/optimizations/simplify_string_builder.h)
  * [simplify_string_builder.cpp](../optimizer/optimizations/simplify_string_builder.cpp)
* Depends on
  * Runtime specialization [runtime_adapter_ets.h](../../plugins/ets/compiler/runtime_adapter_ets.h)
  * Intrinsics [ets_libbase_runtime.yaml](../../plugins/ets/runtime/ets_libbase_runtime.yaml)
* Tests
  * [ets_stringbuilder.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_stringbuilder.ets)
  * [ets_string_builder_append_merge_part1.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_builder_append_merge_part1.ets)
  * [ets_string_builder_append_merge_part2.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_builder_append_merge_part2.ets)
  * [ets_string_builder_append_merge_part3.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_builder_append_merge_part3.ets)
  * [ets_string_builder_append_merge_part4.ets](../../plugins/ets/tests/checked_urunner/simple_run/ets_string_builder_append_merge_part4.ets)
  * [ets_string_builder_merge.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_builder_merge.ets)
  * [ets_string_concat.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_concat.ets)
  * [ets_string_concat_loop.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_string_concat_loop.ets)
  * [ets_stringbuilder_length.ets](../../plugins/ets/tests/checked_urunner/ets_strings_concat/ets_stringbuilder_length.ets)
  * other checked tests
