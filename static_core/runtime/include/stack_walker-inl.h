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

#ifndef PANDA_RUNTIME_STACK_WALKER_INL_H
#define PANDA_RUNTIME_STACK_WALKER_INL_H

#include "stack_walker.h"
#include "runtime/include/cframe_iterators.h"
#include "compiler/code_info/code_info.h"

namespace panda {

template <bool OBJECTS, bool WITH_REG_INFO, class VRegRef, typename Func>
// NOLINTNEXTLINE(google-runtime-references)
bool InvokeCallback(Func func, [[maybe_unused]] compiler::VRegInfo reg_info, VRegRef &vreg)
{
    if (OBJECTS && !vreg.HasObject()) {
        return true;
    }
    if constexpr (WITH_REG_INFO) {  // NOLINT
        if (!func(reg_info, vreg)) {
            return false;
        }
    } else {  // NOLINT
        if (!func(vreg)) {
            return false;
        }
    }
    return true;
}

template <bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateAllRegsForCFrame(Func func)
{
    auto &cframe = GetCFrame();
    const auto regs =
        code_info_.GetVRegList(stackmap_, inline_depth_, mem::InternalAllocator<>::GetInternalAllocatorFromRuntime());

    for (auto &reg_info : regs) {
        if (!reg_info.IsLive()) {
            continue;
        }
        interpreter::VRegister vreg0;
        interpreter::VRegister vreg1;
        interpreter::StaticVRegisterRef res_reg(&vreg0, &vreg1);
        cframe.GetVRegValue(reg_info, code_info_, callee_stack_.stack.data(), res_reg);
        if (!InvokeCallback<false, WITH_REG_INFO>(func, reg_info, res_reg)) {
            return false;
        }
    }

    return true;
}

template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateRegsForCFrameStatic(Func func)
{
    ASSERT(IsCFrame());
    ASSERT(!IsDynamicMethod());
    auto &cframe = GetCFrame();

    if (cframe.IsNativeMethod()) {
        for (auto reg_info : CFrameStaticNativeMethodIterator<RUNTIME_ARCH>::MakeRange(&cframe)) {
            interpreter::VRegister vreg0;
            interpreter::VRegister vreg1;
            interpreter::StaticVRegisterRef res_reg(&vreg0, &vreg1);
            cframe.GetVRegValue(reg_info, code_info_, callee_stack_.stack.data(), res_reg);
            if (!InvokeCallback<OBJECTS, WITH_REG_INFO>(func, reg_info, res_reg)) {
                return false;
            }
        }
        return true;
    }

    if (OBJECTS) {
        code_info_.EnumerateStaticRoots(stackmap_, [this, &cframe, &func](VRegInfo reg_info) {
            interpreter::VRegister vreg0;
            interpreter::VRegister vreg1;
            interpreter::StaticVRegisterRef res_reg(&vreg0, &vreg1);
            cframe.GetVRegValue(reg_info, code_info_, callee_stack_.stack.data(), res_reg);
            return !res_reg.HasObject() || InvokeCallback<OBJECTS, WITH_REG_INFO>(func, reg_info, res_reg);
        });
        return true;
    }

    return IterateAllRegsForCFrame<WITH_REG_INFO>(func);
}

template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateRegsForCFrameDynamic(Func func)
{
    ASSERT(IsCFrame());
    ASSERT(IsDynamicMethod());
    auto &cframe = GetCFrame();

    if (cframe.IsNativeMethod()) {
        for (auto reg_info : CFrameDynamicNativeMethodIterator<RUNTIME_ARCH>::MakeRange(&cframe)) {
            interpreter::VRegister vreg0;
            interpreter::DynamicVRegisterRef res_reg(&vreg0);
            cframe.GetVRegValue(reg_info, code_info_, callee_stack_.stack.data(), res_reg);
            if (!InvokeCallback<OBJECTS, WITH_REG_INFO>(func, reg_info, res_reg)) {
                return false;
            }
        }
        return true;
    }

    if (OBJECTS) {
        code_info_.EnumerateDynamicRoots(stackmap_, [this, &cframe, &func](VRegInfo reg_info) {
            interpreter::VRegister vreg0;
            interpreter::DynamicVRegisterRef res_reg(&vreg0);
            cframe.GetVRegValue(reg_info, code_info_, callee_stack_.stack.data(), res_reg);
            return !res_reg.HasObject() || InvokeCallback<OBJECTS, WITH_REG_INFO>(func, reg_info, res_reg);
        });
        return true;
    }

    return IterateAllRegsForCFrame<WITH_REG_INFO>(func);
}

template <bool OBJECTS, bool WITH_REG_INFO, StackWalker::VRegInfo::Type OBJ_TYPE,
          StackWalker::VRegInfo::Type PRIMITIVE_TYPE, class F, typename Func>
bool StackWalker::IterateRegsForIFrameInternal(F frame_handler, Func func)
{
    for (size_t i = 0; i < frame_handler.GetSize(); i++) {
        auto vreg = frame_handler.GetVReg(i);
        if (OBJECTS && !vreg.HasObject()) {
            continue;
        }
        auto type = vreg.HasObject() ? OBJ_TYPE : PRIMITIVE_TYPE;
        VRegInfo reg_info(0, VRegInfo::Location::SLOT, type, VRegInfo::VRegType::VREG, i);
        if (!InvokeCallback<OBJECTS, WITH_REG_INFO>(func, reg_info, vreg)) {
            return false;
        }
    }

    auto acc = frame_handler.GetAccAsVReg();
    auto type = acc.HasObject() ? OBJ_TYPE : PRIMITIVE_TYPE;
    VRegInfo reg_info(0, VRegInfo::Location::SLOT, type, VRegInfo::VRegType::ACC, 0);
    return InvokeCallback<OBJECTS, WITH_REG_INFO>(func, reg_info, acc);
}

template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateRegsForIFrame(Func func)
{
    auto frame = GetIFrame();
    if (frame->IsDynamic()) {
        DynamicFrameHandler frame_handler(frame);
        ASSERT(IsDynamicMethod());
        return IterateRegsForIFrameInternal<OBJECTS, WITH_REG_INFO, VRegInfo::Type::ANY, VRegInfo::Type::ANY>(
            frame_handler, func);
    }
    StaticFrameHandler frame_handler(frame);
    return IterateRegsForIFrameInternal<OBJECTS, WITH_REG_INFO, VRegInfo::Type::OBJECT, VRegInfo::Type::INT64>(
        frame_handler, func);
}

template <bool OBJECTS, bool WITH_REG_INFO, typename Func>
bool StackWalker::IterateRegs(Func func)
{
    if (IsCFrame()) {
        if (IsDynamicMethod()) {
            return IterateRegsForCFrameDynamic<OBJECTS, WITH_REG_INFO>(func);
        }
        return IterateRegsForCFrameStatic<OBJECTS, WITH_REG_INFO>(func);
    }
    return IterateRegsForIFrame<OBJECTS, WITH_REG_INFO>(func);
}

}  // namespace panda

#endif  // PANDA_RUNTIME_STACK_WALKER_INL_H
