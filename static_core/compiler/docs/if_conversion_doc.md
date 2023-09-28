# IfConversion
## Overview

`IfConversion` tries to remove branches in executable code by creating linear sections with predicate instructions. 

## Rationality

Hardware executes the program speculatively. It tries to predict the branch conditional(true or false) and starts executing instructions after the branch speculatively before executing the branch itself. If the prediction is incorrect(named branch misprediction), the pipeline stops and the state is restored. As result, several dozen cycles are lost. `IfConversion` can add several instructions but remove branch misprediction.

## Dependence

*  Dead Code Elimination(DCE)
*  Remove Empty Blocks
*  Remove Linear blocks
*  Reverse Post Order(RPO)

## Algorithm

Optimization makes a pass through the blocks by post order traversal.
Two patterns are checked for each block: `Triangle` and `Diamond`.

### Triangle

The pattern:

```
      [BB]        
       |  \       
       |  [JBB]   
       |  /       
      [PBB]       
```
`BB` -- basic block the recognition starts from  
`JBB`(Join BB) -- true or false successor of `BB`, which will be joined to BB  
`PBB`(Phi BB) -- false or true successor of `BB`, which contain PHI instruction for BB and JBB

### Diamond

```
      [BB]
     /    \
   [JBB] [JBB 2]
     \    /
      [PBB]
```

`BB` -- basic block the recognition starts from  
`JBB`(Join BB), `JBB 2` -- true and false successors of BB, which will be joined to `BB `  
`PBB`(Phi BB) -- the successor of `JBB` and `JBB 2`, which contain PHI instruction for `JBB` and `JBB 2`  


### Conditions to check

1. `JBB`(and `JBB 2` for Diamond) must have only one predecessor and one successor
2. `PBB` must have 2 or more predecessors
3. `JBB`(and `JBB 2` for Diamond) is the predecessor of the PBB
4. `JBB`(and `JBB 2` for Diamond) doesn't contain instruction with `no_ifcvt` property(for example memory instruction, call instruction, instruction with a call to runtime)
5. The number of instructions in `JBB`(and `JBB 2` for Diamond) less than the limit(set by the option `--compiler-if-conversion-limit=N` with the default value 2)
6. The number of Phi instruction in `PBB`, which have different inputs from corresponding predecessor blocks, should also be less than the limit(each of them would be converted into Select)
7. `PBB` doesn't contain float Phi with different inputs for `JBB` and `BB`(`JBB 2` for Diamond)


### Transformation

1. `If` instructions removed from `BB`(the necessary information, such as the CC, is saved)
2. Edges `BB` -> `JBB` and `JBB` -> `PBB` are removed  
3. All instruction from `JBB` are copied to `BB`  
4. Select instructions are constructed at the end of `BB`(`JBB 2` for Diamond)   
5. All Phi instructions in `PBB` are edited:
   a. If `PBB` has other predecessors, we check if inputs from `JBB` and `BB`(`JBB 2` for Diamond) are equal, then input from `JBB` is removed. Otherwise, it is also removed, but input from `BB`(`JBB 2` for Diamond) is changed to corresponding Select instruction.
   b. If `PBB` doesn't have other predecessors, all Phi inputs are copied to Select instructions and Phi instruction is deleted. 
6. For Diamond `BB` and `JBB 2` are merged
7. If `PBB` doesn't have other predecessors, `BB` and `PBB` are merged
8. Loop information is fixed

## Pseudocode

TODO

## Examples

**Triangle**:  

Before:

```
BB 2  preds: [bb 0]
    3.b    Compare B u64              v0, v1 -> (v4)
    4.     IfImm NE b                 v3, 0x0
succs: [bb 3, bb 4]

BB 3  preds: [bb 2]
    5.u64  Mul                        v0, v2 -> (v6p)
succs: [bb 4]

BB 4  preds: [bb 2, bb 3]
   6p.u64  Phi                        v0(bb2), v5(bb3) -> (v7)
    7.u64  Return                     v6p
succs: [bb 1]
```
After:

```
BB 2  preds: [bb 0]
    3.b    Compare B u64              v0, v1 -> (v8)
    5.u64  Mul                        v0, v2 -> (v8)
    8.u64  SelectImm NE b             v5, v0, v3, 0x0 -> (v7)
    7.u64  Return                     v8
succs: [bb 1]
```

**Diamond**: 

Before:

```
BB 2  preds: [bb 0]
    3.b    Compare EQ u32             v1, v2 -> (v4)          
    4.     IfImm NE b                 v3, 0x0
succs: [bb 3, bb 4]

BB 4  preds: [bb 2]
    5.u32  Add                        v0, v1 -> (v8p)
succs: [bb 5]

BB 3  preds: [bb 2]
    7.u32  Sub                        v0, v1 -> (v8p)
succs: [bb 5]

BB 5  preds: [bb 4, bb 3]
   8p.u32  Phi                        v5(bb4), v7(bb3) -> (v9)
    9.u32  Return                     v8p
succs: [bb 1]
```

After:

```
BB 2  preds: [bb 0]
    3.b    Compare EQ u32             v1, v2 -> (v10)
    7.u32  Sub                        v0, v1 -> (v10)
    5.u32  Add                        v0, v1 -> (v10)
   10.u32  SelectImm NE b             v7, v5, v3, 0x0 -> (v9)
    9.u32  Return                     v10
succs: [bb 1]
```

## Links

Source code:   
[if_conversion.cpp](../optimizer/optimizations/if_conversion.cpp)    
[if_conversion.h](../optimizer/optimizations/if_conversion.h)  

Tests:  
[if_conversion_test.cpp](../tests/if_conversion_test.cpp)
