/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "locations_builder.h"
#include "compiler/optimizer/code_generator/callconv.h"
#include "compiler/optimizer/ir/graph.h"
#include "compiler/optimizer/ir/locations.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOCATIONS_BUILDER(type) \
    template <Arch arch>        \
    type LocationsBuilder<arch>

namespace panda::compiler {
bool RunLocationsBuilder(Graph *graph)
{
    if (graph->GetCallingConvention() == nullptr) {
        return true;
    }
    auto pinfo = graph->GetCallingConvention()->GetParameterInfo(0);
    switch (graph->GetArch()) {
        case Arch::X86_64: {
            return graph->RunPass<LocationsBuilder<Arch::X86_64>>(pinfo);
        }
        case Arch::AARCH64: {
            return graph->RunPass<LocationsBuilder<Arch::AARCH64>>(pinfo);
        }
        case Arch::AARCH32: {
            return graph->RunPass<LocationsBuilder<Arch::AARCH32>>(pinfo);
        }
        default:
            break;
    }
    LOG(ERROR, COMPILER) << "Unknown target architecture";
    return false;
}

LOCATIONS_BUILDER()::LocationsBuilder(Graph *graph, ParameterInfo *pinfo) : Optimization(graph), params_info_(pinfo)
{
    if (graph->SupportManagedCode()) {
        /* We skip first parameter location in managed mode, because calling convention for Panda methods requires
           pointer to the called method as a first parameter. So we reserve one native parameter for it. */
        GetParameterInfo()->GetNextLocation(DataType::POINTER);
    }
}

LOCATIONS_BUILDER(const ArenaVector<BasicBlock *> &)::GetBlocksToVisit() const
{
    return GetGraph()->GetBlocksRPO();
}

LOCATIONS_BUILDER(bool)::RunImpl()
{
    VisitGraph();
    return true;
}

LOCATIONS_BUILDER(void)::ProcessManagedCall(Inst *inst, ParameterInfo *pinfo)
{
    ArenaAllocator *allocator = GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);

    if (pinfo == nullptr) {
        pinfo = GetResetParameterInfo();
        if (inst->GetOpcode() != Opcode::CallResolvedVirtual && inst->GetOpcode() != Opcode::CallResolvedStatic) {
            pinfo->GetNextLocation(GetWordType());
        }
    }

    size_t inputs_count = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);
    size_t stack_args = 0;
    for (size_t i = 0; i < inputs_count; i++) {
        ASSERT(inst->GetInputType(i) != DataType::NO_TYPE);
        auto param = pinfo->GetNextLocation(inst->GetInputType(i));
        if (i == 0 && inst->IsIntrinsic() && inst->CastToIntrinsic()->HasIdInput()) {
            param = Location::MakeRegister(GetTarget().GetParamRegId(0));  // place id input before imms
        }
        locations->SetLocation(i, param);
        if (param.IsStackArgument()) {
            stack_args++;
        }
    }
    if (inst->IsIntrinsic() && stack_args > 0) {
        inst->CastToIntrinsic()->SetArgumentsOnStack();
    }
    if (!inst->NoDest()) {
        locations->SetDstLocation(GetLocationForReturn(inst));
    }
    GetGraph()->UpdateStackSlotsCount(stack_args);
}

LOCATIONS_BUILDER(void)::ProcessManagedCallStackRange(Inst *inst, size_t range_start, ParameterInfo *pinfo)
{
    ArenaAllocator *allocator = GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);

    /* Reserve first parameter for the callee method. It will be set by codegen. */
    if (pinfo == nullptr) {
        pinfo = GetResetParameterInfo();
        pinfo->GetNextLocation(GetWordType());
    }

    size_t inputs_count = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);
    size_t stack_args = 0;
    ASSERT(inputs_count >= range_start);
    for (size_t i = 0; i < range_start; i++) {
        ASSERT(inst->GetInputType(i) != DataType::NO_TYPE);
        auto param = pinfo->GetNextLocation(inst->GetInputType(i));
        locations->SetLocation(i, param);
        if (param.IsStackArgument()) {
            stack_args++;
        }
    }
    if (inst->IsIntrinsic() && stack_args > 0) {
        inst->CastToIntrinsic()->SetArgumentsOnStack();
    }
    for (size_t i = range_start; i < inputs_count; i++) {
        locations->SetLocation(i, Location::MakeStackArgument(stack_args++));
    }
    locations->SetDstLocation(GetLocationForReturn(inst));
    GetGraph()->UpdateStackSlotsCount(stack_args);
}

LOCATIONS_BUILDER(void)::VisitResolveStatic([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToResolveStatic()->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitCallResolvedStatic(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToCallResolvedStatic()->IsInlined()) {
        return;
    }
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst);
}

LOCATIONS_BUILDER(void)::VisitResolveVirtual([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToResolveVirtual()->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitCallResolvedVirtual(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToCallResolvedVirtual()->IsInlined()) {
        return;
    }
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst);
}

LOCATIONS_BUILDER(void)::VisitCallStatic(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToCallStatic()->IsInlined()) {
        return;
    }
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst);
}

LOCATIONS_BUILDER(void)::VisitCallVirtual(GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToCallVirtual()->IsInlined()) {
        return;
    }
    static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst);
}

LOCATIONS_BUILDER(void)::VisitCallDynamic(GraphVisitor *visitor, Inst *inst)
{
    ArenaAllocator *allocator = static_cast<LocationsBuilder *>(visitor)->GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);

    if (inst->CastToCallDynamic()->IsInlined()) {
        ASSERT(inst->GetInputType(CallConvDynInfo::REG_METHOD) == DataType::ANY);
        auto param = Location::MakeRegister(GetTarget().GetParamRegId(CallConvDynInfo::REG_METHOD));
        locations->SetLocation(CallConvDynInfo::REG_METHOD, param);
        return;
    }

    auto pinfo = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();
    for (uint8_t i = 0; i < CallConvDynInfo::REG_COUNT; i++) {
        [[maybe_unused]] Location loc = pinfo->GetNextLocation(GetWordType());
        ASSERT(loc.IsRegister());
    }
    size_t inputs_count = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);

    for (size_t i = 0; i < inputs_count; ++i) {
        ASSERT(inst->GetInputType(i) == DataType::ANY);
        auto param = Location::MakeStackArgument(i + CallConvDynInfo::FIXED_SLOT_COUNT);
        locations->SetLocation(i, param);
    }
    locations->SetDstLocation(GetLocationForReturn(inst));
    static_cast<LocationsBuilder *>(visitor)->GetGraph()->UpdateStackSlotsCount(inputs_count);
}

LOCATIONS_BUILDER(void)::VisitCallIndirect(GraphVisitor *visitor, Inst *inst)
{
    ArenaAllocator *allocator = static_cast<LocationsBuilder *>(visitor)->GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    auto params = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();

    /* First input is an address of the memory to call. So it hasn't exact location and maybe be any register */
    locations->SetLocation(0, Location::RequireRegister());

    /* Inputs, starting from 1, are the native call arguments */
    for (size_t i = 1; i < inst->GetInputsCount(); i++) {
        auto param = params->GetNextLocation(inst->GetInputType(i));
        locations->SetLocation(i, param);
    }
    if (!inst->NoDest()) {
        locations->SetDstLocation(GetLocationForReturn(inst));
    }
}

LOCATIONS_BUILDER(void)::VisitCall(GraphVisitor *visitor, Inst *inst)
{
    ArenaAllocator *allocator = static_cast<LocationsBuilder *>(visitor)->GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    auto params = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();

    for (size_t i = 0; i < inst->GetInputsCount(); i++) {
        auto param = params->GetNextLocation(inst->GetInputType(i));
        locations->SetLocation(i, param);
    }
    if (!inst->NoDest()) {
        locations->SetDstLocation(GetLocationForReturn(inst));
    }
}

LOCATIONS_BUILDER(void)::VisitIntrinsic(GraphVisitor *visitor, Inst *inst)
{
    auto graph = static_cast<LocationsBuilder *>(visitor)->GetGraph();
    auto intrinsic = inst->CastToIntrinsic();
    if (intrinsic->IsNativeCall() || IntrinsicNeedsParamLocations(intrinsic->GetIntrinsicId())) {
        auto pinfo = static_cast<LocationsBuilder *>(visitor)->GetResetParameterInfo();
        if (intrinsic->IsMethodFirstInput()) {
            pinfo->GetNextLocation(GetWordType());
        }
        if (intrinsic->HasImms() && graph->SupportManagedCode()) {
            for ([[maybe_unused]] auto imm : intrinsic->GetImms()) {
                pinfo->GetNextLocation(DataType::INT32);
            }
        }
        size_t explicit_args;
        if (IsStackRangeIntrinsic(intrinsic->GetIntrinsicId(), &explicit_args)) {
            static_cast<LocationsBuilder *>(visitor)->ProcessManagedCallStackRange(inst, explicit_args, pinfo);
        } else {
            static_cast<LocationsBuilder *>(visitor)->ProcessManagedCall(inst, pinfo);
        }
        return;
    }

    ArenaAllocator *allocator = static_cast<LocationsBuilder *>(visitor)->GetGraph()->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    auto inputs_count = inst->GetInputsCount() - (inst->RequireState() ? 1 : 0);
    for (size_t i = 0; i < inputs_count; i++) {
        locations->SetLocation(i, Location::RequireRegister());
    }
}

LOCATIONS_BUILDER(void)::VisitNewArray([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToNewArray()->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(0)));
    inst->CastToNewArray()->SetLocation(1, Location::MakeRegister(GetTarget().GetParamRegId(1)));
    inst->CastToNewArray()->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitNewObject([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToNewObject()->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(0)));
    inst->CastToNewObject()->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitParameter([[maybe_unused]] GraphVisitor *visitor, [[maybe_unused]] Inst *inst)
{
    /* Currently we can't process parameters in the Locations Builder, because Parameter instructions may be removed
     * during optimizations pipeline. Thus, locations is set by IR builder before optimizations.
     * TODO(compiler): we need to move Parameters' locations here, thereby we get rid of arch structures in the Graph,
     * such as ParameterInfo, CallingConvention, etc.
     */
}

LOCATIONS_BUILDER(void)::VisitReturn([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    auto return_reg =
        DataType::IsFloatType(inst->GetType()) ? GetTarget().GetReturnRegId() : GetTarget().GetReturnFpRegId();
    inst->CastToReturn()->SetLocation(0, Location::MakeRegister(return_reg));
}

LOCATIONS_BUILDER(void)::VisitThrow([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToThrow()->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(0)));
}

LOCATIONS_BUILDER(void)::VisitLiveOut([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    inst->CastToLiveOut()->SetLocation(0, Location::MakeRegister(inst->GetDstReg()));
}

LOCATIONS_BUILDER(void)::VisitMultiArray(GraphVisitor *visitor, Inst *inst)
{
    auto graph = static_cast<LocationsBuilder *>(visitor)->GetGraph();
    ArenaAllocator *allocator = graph->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    locations->SetLocation(0, Location::RequireRegister());

    for (size_t i = 1; i < inst->GetInputsCount() - 1; i++) {
        locations->SetLocation(i, Location::MakeStackArgument(i - 1));
    }
    auto stack_args = inst->GetInputsCount() - 2U;
    graph->UpdateStackSlotsCount(RoundUp(stack_args, 2U));
    locations->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitCallLaunchStatic(GraphVisitor *visitor, Inst *inst)
{
    auto graph = static_cast<LocationsBuilder *>(visitor)->GetGraph();
    ArenaAllocator *allocator = graph->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    // First input is second parameter(first is pointer to a method)
    locations->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(1)));

    for (size_t i = 1; i < inst->GetInputsCount() - 1; i++) {
        locations->SetLocation(i, Location::MakeStackArgument(i - 1));
    }
    auto stack_args = inst->GetInputsCount() - 2U;
    graph->UpdateStackSlotsCount(RoundUp(stack_args, 2U));
    locations->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitCallLaunchVirtual(GraphVisitor *visitor, Inst *inst)
{
    auto graph = static_cast<LocationsBuilder *>(visitor)->GetGraph();
    ArenaAllocator *allocator = graph->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    // First input is second parameter(first is pointer to a method)
    locations->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(1)));
    // This object
    locations->SetLocation(1, Location::MakeRegister(GetTarget().GetParamRegId(3)));

    for (size_t i = 2; i < inst->GetInputsCount() - 1; i++) {
        locations->SetLocation(i, Location::MakeStackArgument(i - 2));
    }
    auto stack_args = inst->GetInputsCount() - 3U;
    graph->UpdateStackSlotsCount(RoundUp(stack_args, 2U));
    locations->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitCallResolvedLaunchStatic(GraphVisitor *visitor, Inst *inst)
{
    auto graph = static_cast<LocationsBuilder *>(visitor)->GetGraph();
    ArenaAllocator *allocator = graph->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    // Pointer to the method
    locations->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(0)));
    // Launch object
    locations->SetLocation(1, Location::MakeRegister(GetTarget().GetParamRegId(1)));

    for (size_t i = 2; i < inst->GetInputsCount() - 1; i++) {
        locations->SetLocation(i, Location::MakeStackArgument(i - 2));
    }
    auto stack_args = inst->GetInputsCount() - 3U;
    graph->UpdateStackSlotsCount(RoundUp(stack_args, 2U));
    locations->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitCallResolvedLaunchVirtual(GraphVisitor *visitor, Inst *inst)
{
    auto graph = static_cast<LocationsBuilder *>(visitor)->GetGraph();
    ArenaAllocator *allocator = graph->GetAllocator();
    LocationsInfo *locations = allocator->New<LocationsInfo>(allocator, inst);
    // Pointer to the method
    locations->SetLocation(0, Location::MakeRegister(GetTarget().GetParamRegId(0)));
    // Launch object
    locations->SetLocation(1, Location::MakeRegister(GetTarget().GetParamRegId(1)));
    // This object
    locations->SetLocation(2, Location::MakeRegister(GetTarget().GetParamRegId(3)));

    for (size_t i = 3; i < inst->GetInputsCount() - 1; i++) {
        locations->SetLocation(i, Location::MakeStackArgument(i - 3));
    }
    auto stack_args = inst->GetInputsCount() - 4U;
    graph->UpdateStackSlotsCount(RoundUp(stack_args, 2U));
    locations->SetDstLocation(GetLocationForReturn(inst));
}

LOCATIONS_BUILDER(void)::VisitStoreStatic([[maybe_unused]] GraphVisitor *visitor, Inst *inst)
{
    if (inst->CastToStoreStatic()->GetNeedBarrier()) {
        inst->SetFlag(inst_flags::REQUIRE_TMP);
    }
}

template <Arch ARCH>
Location LocationsBuilder<ARCH>::GetLocationForReturn(Inst *inst)
{
    auto return_reg =
        DataType::IsFloatType(inst->GetType()) ? GetTarget().GetReturnFpRegId() : GetTarget().GetReturnRegId();
    return Location::MakeRegister(return_reg, inst->GetType());
}

template <Arch ARCH>
ParameterInfo *LocationsBuilder<ARCH>::GetResetParameterInfo()
{
    params_info_->Reset();
    return params_info_;
}
}  // namespace panda::compiler
