# Hybrid Strings optimizations

## Overview

Current optimization described below preserve original behavior of ```String``` operations and their side effects. This is tested by the wide set of tests (see Links Section below).

## Rationality

We have several types of strings possibly instantiated by runtime during program execution:
1) ```Flat``` - string data is stored in a buffer within a string object.
2) ```Tree``` - string object consist of two parts: left and right, each part is a string object as well. Result of simple concatenation operation.
3) ```Sliced``` - string object is an offset, length and a reference to a parent string containing characters. Result of substring operation.

By default, frontend emits ```StringBuilder.concatStrings``` static calls for string concatenation, for a complex string calculation expressions it is sometimes non-optimal solution (see example below), when intermediate string objects are created and collected by GC immediately after single use.

In this document we propose an algorithm to match such a cases, and describe an optimization to replace a string concatenation expressions by a ```StringBuilder``` instance.

## Dependence

### Dependence on analysis

* DominatorsTree

### Dependence on stdlib

Optimizations depends on the following stdlib API classes and their methods:

* ```std.core.StringBuilder```
  * ```<ctor>```, may emit in ```AOT, JIT```
  * ```append```, may emit in ```AOT, JIT```
  * ```toString```, may emit in ```AOT, JIT```
  * ```concatStrings```, may optimize out in ```AOT, JIT```

### Dependence on compiler intrinsics

Optimizations depends on the following compiler intrinsics:

* ```Intrinsic```
  * ```StdCoreSbAppendString```, may emit in ```AOT, JIT```
  * ```StdCoreSbToString```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat2```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat3```, may emit in ```AOT, JIT```
  * ```StdCoreStringConcat4```, may emit in ```AOT, JIT```

## Algorithm

Example:

```TS
function hybridStringTest01(arg: string): string {
    let var1: string = 'var1'

    var1 += arg

    for (let i = 0; i < 10; i++) {
        var1 += arg
    }

    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}
```

All the concatenations in the example above are emitted as ```StringBuilder.concatStrings``` by frontend, this results in ```tree```-string type being used to store intermediate string objects. The algorithm analyzes dataflow of the code and decides whether to switch to ```StringBuilder``` object, append string parts and call to ```StringBuilder.toString()``` at the end, or to keep using ```tree```-strings, based on the dataflow patterns found.

## Pseudocode

**Complete algorithm**

```C#
type
    ConcatenationMatch
        IsValid(): bool
        concatInsts: array of Inst
        width: size_t
        requireFlattening: bool
        usedByConcatOnly: bool
        usedInTryCatch: bool

function HybridStringsOptimization(graph: Graph)
    let matches be an array if String concatenation matches
    foreach block in graph (in PO)
        foreach inst in block
            if inst is concatStrings-call
                let match be ConcatenationMatch
                MatchConcatenationExpression(inst, match)
                add match to matches

    foreach match in matches
        if match is valid
            replace match with StringBuilder

function MatchConcatenationExpression(inst: Inst, match: ConcatenationMatch)
    let numConcatStringsInputs = 0
    foreach input in inst
        if input is phi
            MatchConcatenationExpression(input, match)
            continue
        if input is concatStrings-call
            numConcatStringsInputs++
            MatchConcatenationExpression(input, match)

    if inst is concatStrings-call
        match.width += numConcatStringsInputs - 1
    else
        return

    // Collect match validity information
    match.requireFlattening |= true if inst has user require inst to be flattened
    match.usedByConcatOnly &= false if inst has user other than concat instruction
    match.usedInTryCatch |= true if inst is in try-catch block
    add inst into match.concatInsts

function ReplaceEachWithStringBuilder(match: ConcatenationMatch)
    foreach concatInst in match.concatInsts
        insert new StringBuilder instance into graph
        insert StringBuilder.ctor call into graph for instance
        foreach input in concatInst
            insert StringBuilder.append call into graph for instance and input
        insert StringBuilder.toString call into graph for instance
        replace concatInst with StringBuilder.toString
```

Below we describe the algorithm in more details

An algorithm finds all string concatenations within a graph and matches the entire string expression being calculated. Then for each concatenation match found we check if it is ```valid``` - i.e. dataflow pattern corresponds to a long-chain concatenation, and replace it with ```StringBuilder``` constructor-, append()- and toString()-calls. Generated ```StringBuilders``` are then optimized by ```SimplifyStringBuilder``` optimization pass.

## Examples

ETS checked test ets_hybrid_strings.ets, function example:

```TS
function hybridStringTest01(arg: string): string {
    let var1: string = 'var1'

    var1 += arg

    for (let i = 0; i < 10; i++) {
        var1 += arg
    }

    var1 += arg

    return var1 // switch to SB due to long chain concatenation
}
```

IR before transformation:

(Save state and null check instructions are skipped for simplicity)

```
Method: std.core.String ets_hybrid_strings.ETSGLOBAL::hybridStringTest01(std.core.String)

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v20, v26, v9)
    3.i64  Constant                   0xa -> (v17)
    4.i64  Constant                   0x0 -> (v12p)
   23.i64  Constant                   0x1 -> (v22)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    5.ref  LoadString                ss -> (v9)
    9.ref  CallStatic std.core.StringBuilder::concatStrings v5, v0, ss -> (v13p)
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
  12p.i32  Phi                        v4(bb0), v22(bb2) -> (v22, v17)
  13p.ref  Phi                        v9(bb0), v20(bb2) -> (v26, v20)
   17.b    Compare GE i32             v12p, v3 -> (v18)
   18.     IfImm NE b                 v17, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   20.ref  CallStatic std.core.StringBuilder::concatStrings v13p, v0, ss -> (v13p)
   22.i32  Add                        v12p, v23 -> (v12p)
succs: [bb 3]

BB 1  preds: [bb 3]
   26.ref  CallStatic std.core.StringBuilder::concatStrings v13p, v0, ss -> (v28)
   28.ref  Return                     v26
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end

```

Notice concat calls 8, 20, and 26.

IR after Hybrid Strings optimization transformation:

```
Method: std.core.String ets_hybrid_strings.ETSGLOBAL::hybridStringTest01(std.core.String)

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v61, v49, v37)
    3.i64  Constant                   0xa -> (v17)
    4.i64  Constant                   0x0 -> (v12p)
   23.i64  Constant                   0x1 -> (v22)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    5.ref  LoadString                 ss -> (v35)
   30.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v32)
   32.ref  NewObject                  v30, ss -> (v39, v37, v35, v33)
   33.void CallStatic  std.core.StringBuilder::<ctor> v32, ss
   35.ref  Intrinsic.StdCoreSbAppendString v32, v5, ss
   37.ref  Intrinsic.StdCoreSbAppendString v32, v0, ss
   39.ref  Intrinsic.StdCoreSbToString v32, ss -> (v13p)
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
  12p.i32  Phi                        v4(bb0), v22(bb2) -> (v22, v17)
  13p.ref  Phi                        v39(bb0), v51(bb2) -> (v59, v47)
   17.b    Compare GE i32             v12p, v3 -> (v18)
   18.     IfImm NE b                 v17, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
hotness=0
   42.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v44)
   44.ref  NewObject                  v42, ss -> (v51, v49, v47, v45)
   45.void CallStatic std.core.StringBuilder::<ctor> v44, ss
   47.ref  Intrinsic.StdCoreSbAppendString v44, v13p, ss
   49.ref  Intrinsic.StdCoreSbAppendString v44, v0, ss
   51.ref  Intrinsic.StdCoreSbToString v44, ss -> (v13p)
   22.i32  Add                        v12p, v23 -> (v12p)
succs: [bb 3]

BB 1  preds: [bb 3]
prop: bc: 0x0000001f
hotness=0
   54.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v56)
   56.ref  NewObject                  v54, ss -> (v63, v61, v59, v57)
   57.void CallStatic std.core.StringBuilder::<ctor> v56, ss
   59.ref  Intrinsic.StdCoreSbAppendString v56, v13p, ss
   61.ref  Intrinsic.StdCoreSbAppendString v56, v0, ss
   63.ref  Intrinsic.StdCoreSbToString v56, ss -> (v28)
   28.ref  Return                     v63
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

Concat call 8 is replaced with instructions 30-39, concat call 20 is replaced with instructions 42-51, concat call 26 is replaced with instructions 54-63.

IR after Simplify String Builder transformation:

```
Method: std.core.String ets_hybrid_strings.ETSGLOBAL::hybridStringTest01(std.core.String)

BB 4
prop: start
    0.ref  Parameter                  arg 0 -> (v70, v61, v49)
    3.i64  Constant                   0xa -> (v17)
    4.i64  Constant                   0x0 -> (v78, v77, v12p)
   23.i64  Constant                   0x1 -> (v79, v22)
   73.i64  Constant                   0x10 -> (v75)
succs: [bb 0]

BB 0  preds: [bb 4]
prop: prehead (loop 1)
    5.ref  LoadString                 ss -> (v70)
   30.ref  LoadAndInitClass 'std.core.StringBuilder' ss -> (v32)
   32.ref  NewObject                  v30, ss -> (v79, v78, v77, v76, v70, v49, v61, v63)
   72.ref  LoadAndInitClass 'std.core.Object[]' ss -> (v75)
   75.ref  NewArray (size=16)         v72, v73, ss -> (v76)
   76.ref  StoreObject std.core.StringBuilder.buf v32, v75
   77.i32  StoreObject std.core.StringBuilder.index v32, v4
   78.i32  StoreObject std.core.StringBuilder.length v32, v4
   79.b    StoreObject std.core.StringBuilder.compress v32, v23
   70.ref  Intrinsic.StdCoreSbAppendString2 v32, v5, v0, ss
succs: [bb 3]

BB 3  preds: [bb 0, bb 2]
prop: head, loop 1, depth 1
  12p.i32  Phi                        v4(bb0), v22(bb2) -> (v22, v17)
   17.b    Compare GE i32             v12p, v3 -> (v18)
   18.     IfImm NE b                 v17, 0x0
succs: [bb 1, bb 2]

BB 2  preds: [bb 3]
prop: loop 1, depth 1
   49.ref  Intrinsic.StdCoreSbAppendString v32, v0, ss
   22.i32  Add                        v12p, v23 -> (v12p)
succs: [bb 3]

BB 1  preds: [bb 3]
   61.ref  Intrinsic.StdCoreSbAppendString v32, v0, ss
   63.ref  Intrinsic.StdCoreSbToString v32, ss -> (v28)
   28.ref  Return                     v63
succs: [bb 5]

BB 5  preds: [bb 1]
prop: end
```

StringBuilder instructions are simplified as follows:
  1) Only one (of three) instance left
  2) Allocation and ctor call hoisted to pre-header block
  3) Loop contains only one StringBuilder.append() call per iteration
  4) Final StringBuilder.toString() hoisted to post-exit block

## Links

* Implementation
  * [hybrid_strings_optimization.h](../optimizer/optimizations/hybrid_strings_optimization.h)
  * [hybrid_strings_optimization.cpp](../optimizer/optimizations/hybrid_strings_optimization.cpp)
* Depends on
  * Simplify String Builder optimization pass [simplify_string_builder.cpp](../optimizer/optimizations/simplify_string_builder.cpp)
  * Runtime specialization [runtime_adapter_ets.h](../../plugins/ets/compiler/runtime_adapter_ets.h)
  * Intrinsics [ets_libbase_runtime.yaml](../../plugins/ets/runtime/ets_libbase_runtime.yaml)
* Tests
  * [ets_hybrid_strings.ets](../../plugins/ets/tests/checked_urunner/ets_tests/ets_hybrid_strings.ets)
