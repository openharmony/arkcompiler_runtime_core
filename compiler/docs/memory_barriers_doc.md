# Optimize memory barriers

## Overview 

We need to encode barriers after the instructions NewArray, NewObject, NewMultiArray so that if the created objects are used in another thread, the initialization is fully completed.
We can remove the barrier if we prove that the created object cannot be passed to another thread before the next barrier. 
This can happen if we save the object to memory or pass it to another method

## Rationality

Reducing the number of instructions and speed up execution. 

## Dependence 

RPO analysis

## Algorithm

There is instruction flag `MEM_BARRIER`. The flag is set to `true` for the instructions NewObject, NewArray and NewMultiArray.  
The pass `OptimizeMemoryBarriers` try remove the flag(set false) from the instruction.  
We pass through all instructions in PRO order. If the instruction has flag `MEM_BARRIER` we add the instruction in special vector `barriers_insts_`.
If we visit an instruction that can pass an object to another thread(Store instruction, Call instruction e.t.c) we check the instruction inputs.  
If the instruction has input from the `barriers_insts_`, we call function `MergeBarriers`.  
The function set `false` for the flag `MEM_BARRIER`, exclude last instruction from the vector. 
So we will only set the barrier in the last instruction before potentially passing the created objects to another thread

The function `MergeBarriers` also is called at end of the basic block.

Codegen checks the flag `MEM_BARRIER` for the instructions NewObject, NewArray and NewMultiArray and encode memory barrier if the flag `true`

## Pseudocode

```
bool OptimizeMemoryBarriers::RunImpl()
{
    barriers_insts.clear();
    for (auto bb : GetGraph()->GetBlocksRPO()) {
        for (auto inst : bb->Insts()) {
            if (inst->GetFlag(inst_flags::MEM_BARRIER)) {
                barriers_insts.push_back(inst);
            }
            if (InstCanMoveObjectInAnotherthread(inst) && InstHasBarrierInput(inst, barriers_insts)) {
                MergeBarriers(barriers_insts);
            }
        }
        MergeBarriers(barriers_insts);
    }
    return true;
}

void MemoryBarriersVisitor::MergeBarriers(InstVector& barriers_insts)
{
    if (barriers_insts.empty()) {
        return;
    }
    auto last_barrier_inst = barriers_insts.back();
    for (auto inst : barriers_insts) {
        inst->ClearFlag(inst_flags::MEM_BARRIER);
    }
    last_barrier_inst->SetFlag(inst_flags::MEM_BARRIER);
    barriers_insts.clear();
}
```

## Examples

```
BB 0
prop: start
    0.i64  Constant                   0x2a -> (v6, v3, v1, v2, v8, v11)
succs: [bb 2]

BB 2  preds: [bb 0]
    1.     SaveState                  v0(vr0) -> (v2)
    2.ref  NewArray 1                 v0, v1 -> (v6, v3, v8, v11, v12)
    3.     SaveState                  v0(vr0), v2(vr1) -> (v5, v4)
    4.ref  LoadAndInitClass 'A'       v3 -> (v5)
    5.ref  NewObject 2                v4, v3 -> (v6, v8, v11, v12)
    6.     SaveState                  v0(vr0), v2(vr1), v5(vr2) -> (v7, v12)
    7.void CallStatic 3               v6
    8.     SaveState                  v0(vr0), v2(vr1), v5(vr2) -> (v9, v10)
    9.ref  LoadAndInitClass 'B'       v8 -> (v10)
   10.ref  NewObject 4                v9, v8 -> (v11, v13)
   11.     SaveState                  v0(vr0), v2(vr1), v5(vr2), v10(vr3)
   12.i64  CallVirtual 5              v2, v5, v6
   13.ref  Return                     v10
succs: [bb 1]

BB 1  preds: [bb 2]
prop: end
```

Instructions `2.ref  NewArray`, `5.ref  NewObject` and `10.ref  NewObject` have flag `MEM_BARRIER` by default.
`7.void CallStatic` don't have the instructions  `2.ref  NewArray`, `5.ref  NewObject` as inputs. 
So the pass `OptimizeMemoryBarriers` will remove the flag from these instructions and skip in `10.ref  NewObject`.  

## Links

Source code:   
[memory_barriers.cpp](../optimizer/optimizations/memory_barriers.cpp)  
[memory_barriers.h](../optimizer/optimizations/memory_barriers.h)  

Tests:  
[memory_barriers_test.cpp](../tests/memory_barriers_test.cpp)