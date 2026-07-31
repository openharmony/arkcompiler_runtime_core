# String optimizations overview

Current documents briefly describes strings optimizations implemented in JIT/AOT/BCO, connections between them, their place in pipeline, etc.

## Overview

In ArkTS language there are many ways to manipulate/concatenate strings:
* plus-operator (emitted as `StringBuilder.concatStrings` in bytecode)
* `StringBuilder.concatStrings` static method
* `string.concat` virtual method
* `StringBuilder` object

Typical example of how `StringBuilder` object is used to build complex string
```
let sb = new StringBuilder()
sb.append(some_arg)
for (let i = 0; i < some_array.length; ++i) {
    sb.append(some_array[i])
}
let result = sb.toString()
```

Having these emitted into bytecode, it is not easy to implement a common optimization supporting either way to concat.

So, at first we convert all non-`StringBuilder` concatenations to `StringBuilder` form. This is done by the `HybridStringOptimization` pass. It converts optimizations done by `StringBuilder.concatStrings` static method (supported) and `string.concat` virtual method (not yet supported) into using of `StringBuilder` objects in data flow graph.

Next, we apply optimizations over `StringBuilder` object, all of them implemented in `SimplifyStringBuilder` optimization pass. Please, refer to corresponding md-file for details.

Last step, after all the `StringBuilder` optimizations are applied to a data flow graph, we optimize each `StringBuilder` object left in a graph, by counting all `append` calls and pre-allocating `StringBuilder` internal buffer where appending parts are store, to avoid buffer reallocations (see `ReserveStringBuilderBuffer` optimization pass).

## Limitations and corner cases

Under some conditions, algorithms may not apply optimizations implemented. Here we describe possible reasons

* `HybridStringOptimization` pass
    * may crash if `StringBuilder` class is not present, but `StringBuilder.concatStrings` static method is present in current abc
    * may crash if `StringBuilder` class has no default constructor loaded via `PlatformTypes` mechanism
* `SimplifyStringBuilder` optimization pass
    * does nothing, if graph has non-intrinsic call instruction with `IsNativeFlag` set
    * loops optimization is not applied, if
        * graph has try-catch blocks
        * in `OSR` mode
    * may not apply on manually created code with `StringBuilder` class used, e.g `StringBuilder` object is still used after `StringBuilder.toString()` method called, or `StringBuilder` instance has calls of `append` function with non-string argument
    * may no apply, if pass encounters `SaveState` instruction with inputs which may not be removed
    * merging consecutive calls to `append` function into `appendN` call is not applied if IRTOC barriers are not supported at current platform
* `ReserveStringBuilderBuffer` optimization pass
    * does nothing, if `Object[]` (objects array) class declaration is not present in current abc
    * may crash, if `StringBuilder` object internal structure changes, e.g fields `buf`, `index`, `length`, `compress` are not found

## Links

* Implementation details
    * [HybridStringOptimization](./hybrid_strings_optimization.md)
    * [SimplifyStringBuilder](./simplify_sb_doc.md)
    * [ReserveStringBuilderBuffer](./reserve_sb_buffer_doc.md)
