# Checks Elimination
## Overview
**Checks Elimination** - optimization which try to reduce number of checks(NullCheck, ZeroCheck, NegativeCheck, BoundsCheck).

## Rationality
Reduce number of instructions and removes unnecessary data-flow dependencies.

## Dependences
* RPO
* BoundsAnalysis
* DomTree
* LoopAnalysis

## Algorithm
Visit all check instructions in RPO order and check specific rules.
If one of the rules is applicable, the instruction is deleted.

Instruction deleted in two steps:
1. Check instruction inputs connects to check users.
2. Check instruction replace on `NOP` instruction.

### Rules
#### All checks
All the same checks that are dominated by the current one are deleted and consecutive checks deleted.
#### NullCheck: 
1. NullChecks with allocation instruction input is deleted.
2. If based on bounds analysis input of NullCheck is more then 0, check is deleted.

Loop invariant NullChecks replaced by `DeoptimiseIf` instruction before loop.

#### ZeroCheck:
If based on bounds analysis input of ZeroCheck isn't equal than 0, check is deleted.
#### NegativeCheck: 
If based on bounds analysis input of NegativeCheck is more then 0, check is deleted.
#### BoundsChecks: 
If based on bounds analysis input of BoundsCheck is more or equal than 0 and less than length of array, check is deleted.

If BoundsCheck isn't deleted, it is grouped for later BCE transformations.
The main grouping key is `(loop, len_array, parent_index)`.

- `parent_index` is the original index for `index +/- const`, or the index itself for non-offset expressions.
- Constant indexes are stored in a separate `parent_index == nullptr` group.
- Each group keeps:
  - checks that dominate all loop back edges or the graph end
  - checks that do not dominate all back edges, grouped by block
  - maximal upper offset and minimal lower offset

```
// {check_inst, check_upper, check_lower, cached_offset}
using BoundsCheckInfo = std::tuple<Inst *, bool, bool, int64_t>;
using NotDominateInstInfoVector = ArenaVector<BoundsCheckInfo>;
// {check_block, NotDominateInstInfoVector}
using NotDominateBoundsChecksByBlock = ArenaVector<std::pair<BasicBlock *, NotDominateInstInfoVector>>;
// {parent_index, dominate_checks, not_dominate_checks_by_block, max_val, min_val}
using ParentIndexUsedBoundsChecks =
    std::tuple<Inst *, InstVector, NotDominateBoundsChecksByBlock, int64_t, int64_t>;
using GroupedBoundsChecks = ArenaVector<ParentIndexUsedBoundsChecks>;
// loop -> len_array -> GroupedBoundsChecks
using NotFullyRedundantBoundsCheck = ArenaVector<std::pair<Loop *, ArenaVector<std::pair<Inst *, GroupedBoundsChecks>>>>;
```

For example, for this method:
```
int Foo(array a, int index) {
  BoundCheck(len_array(a), index); // id = 1
  BoundCheck(len_array(a), index+1); // id = 2
  BoundCheck(len_array(a), index-2); // id = 3
  return a[index] + a[index+1] + a[index-2];
}
```
NotFullyRedundantBoundsCheck will be filled in this way:
```
Root_loop->len_array(a) ->
  {parent_index = index,
   dominate_checks = {BoundsChecks 1,2,3},
   not_dominate_checks_by_block = {},
   max_val = 1,
   min_val = -2}
```

Current BCE has three main bounds-check deopt paths.

1. `BeforeLoop`

For countable loops, if grouped checks use the loop phi as `parent_index`, BCE may replace the loop-global bounds checks by `DeoptimiseIf` instructions before the loop.

Example:
```
for ( int i = 0; i < x; i++) {
  BoundCheck(i);
  a[i] = 0;
}
```
can be transformed to
```
deoptimizeIf(x >= len_array(a));
for ( int i = 0; i < x; i++) {
  a[i] = 0;
}
```

2. `InLoop`

For groups that are not handled by `BeforeLoop`, BCE may replace more than 2 loop-global grouped bounds checks by `DeoptimiseIf` inside the loop.

Example:
```
int Foo(array a, int index) {
  BoundCheck(len_array(a), index); // id = 1
  BoundCheck(len_array(a), index+1); // id = 2
  BoundCheck(len_array(a), index-2); // id = 3
  return a[index] + a[index+1] + a[index-2];
}
```
will be transformed to:
```
int Foo(array a, int index) {
  deoptimizeIf(index-2 < 0);
  deoptimizeIf(index+1 >= len_array(a));
  return a[index] + a[index+1] + a[index-2];
}
```

Only checks whose instruction blocks dominate all loop back edges are treated as loop-global candidates.

3. `Block-local`

For checks that do not dominate all loop back edges, BCE does not merge them into a loop-global guard.
Instead, it tries a local merge inside one block.

If several such bounds checks are in the same block, BCE may insert local `DeoptimiseIf` instructions in that block and remove the covered checks there.
After a local guard is inserted, covered checks in dominated successor blocks may also be eliminated.

This keeps insertion local to the proven path and avoids creating loop-global guards for branch-local checks.

Example:
```
for (int i = 0; i < n; i++) {
  if (cond) {
    BoundCheck(len_array(a), i);     // id = 1
    BoundCheck(len_array(a), i + 1); // id = 2
    x += a[i] + a[i + 1];
  }
}
```
can be transformed to:
```
for (int i = 0; i < n; i++) {
  if (cond) {
    deoptimizeIf(i < 0);
    deoptimizeIf(i + 1 >= len_array(a));
    x += a[i] + a[i + 1];
  }
}
```

## Pseudocode
  TODO

## Examples

Before Checks Elimination:
```
1.ref Parameter  -> v3, v4
2.i64 Constant 1 -> v5, v6

3.ref NullCheck v1 -> v7
4.ref NullCheck v1 -> v8
5.i32 ZeroCheck v2 -> v9
6.i32 NegativeCheck v2 -> v10 
```
After Checks Elimination:
```
1.ref Parameter  -> v3
2.i64 Constant 1 -> v9, v10

3.ref NullCheck v1 -> v7, v8
4.ref NOP
5.i32 NOP
6.i32 NOP
```

## Links
Source code:   
[checks_elimination.h](../optimizer/optimizations/checks_elimination.h)  
[checks_elimination.cpp](../optimizer/optimizations/checks_elimination.cpp)

Tests:  
[checks_elimination_test.cpp](../tests/checks_elimination_test.cpp)
