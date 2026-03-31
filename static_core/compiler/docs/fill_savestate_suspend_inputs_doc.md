# FillSaveStateSuspendInputs

## Overview

FillSaveStateSuspendInputs is an optimization pass that fills the inputs of `SaveStateSuspend` instructions before register allocation. `SaveStateSuspend` is used at coroutine suspend points; at these points all live values must be captured so that GC can scan roots while the coroutine is suspended and values can be restored on resume.

## Rationality

The IR builder adds VReg (virtual register) inputs to SaveStateSuspend. This pass adds the remaining **non-VReg (bridge) inputs**: all values that are live at the suspend point but not yet present in the instruction. Thus, the instruction ends up with a complete set of inputs for GC and resume.

## Dependence

* RPO order.
* LivenessAnalyzer - pass runs the default target-independent part internally. Target-dependent part is not run, but is invalidated by the pass.

## Placement

The pass runs **before** Register Allocation:
- Register Allocation can re-use target-independent Liveness Analysis, run by this pass;
- Register Allocation must recompute target-dependent Liveness Analysis based on modifications done by this pass.

## Algorithm

1. Run target-independent LivenessAnalysis, if not already computed.
2. If the graph has no SaveStateSuspend instructions, return without changes.
3. For each SaveStateSuspend instruction:
   - Mark all existing inputs with a marker (for O(1) duplicate detection).
   - Mark the SaveStateSuspend instruction itself so it is not added as its own input.
   - Get the set of live values at this point via `LivenessAnalyzer::GetLiveValuesAtPoint`.
   - For each live value: if not already marked (i.e. not in inputs and not the SSS itself), add it as a bridge input and mark it.
   - Call `OptimizeSaveStateConstantInputs` to convert constant inputs to immediates where applicable.
4. Set the graph flag `SaveStateSuspendInputsAllocated`.

The pass only **appends** inputs; it never removes existing ones.

Constant-input optimization for other SaveState variants (SaveState, SafePoint, SaveStateOsr, SaveStateDeoptimize) is performed in **Lowering** via the common visitor.
SaveStateSuspend has no visitor there, so it is optimized explicitly in this pass after bridge inputs are added.

## Links

- [Lowering](lowering_doc.md) — performs `OptimizeSaveStateConstantInputs` for SaveState, SafePoint, SaveStateOsr, SaveStateDeoptimize via the visitor.
- [Instructions: SaveStateSuspend](../optimizer/ir/instructions.yaml)
- Source: [fill_savestate_suspend_inputs.cpp](../optimizer/optimizations/fill_savestate_suspend_inputs.cpp), [fill_savestate_suspend_inputs.h](../optimizer/optimizations/fill_savestate_suspend_inputs.h)
- Tests: [fill_savestate_suspend_inputs_test.cpp](../tests/fill_savestate_suspend_inputs_test.cpp)
