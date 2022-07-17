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

/*
   Compiler -> Pass(Codegen)
        Encoder = Fabric->getAsm

Define arch-specific interfaces
*/

#include "operands.h"
#include "encode.h"
#include "compiler/optimizer/code_generator/callconv.h"
#include "registers_description.h"

#ifdef PANDA_COMPILER_TARGET_AARCH32
#include "aarch32/target.h"
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
#include "aarch64/target.h"
#endif

#ifdef PANDA_COMPILER_TARGET_X86_64
#include "amd64/target.h"
#endif

#include "asm_printer.h"

#include "frame_info.h"

namespace panda::compiler {
bool BackendSupport(Arch arch)
{
    switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH32
        // NOLINTNEXTLINE(bugprone-branch-clone)
        case Arch::AARCH32: {
            return true;
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        case Arch::AARCH64: {
            return true;
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        case Arch::X86_64: {
            return true;
        }
#endif
        default:
            return false;
    }
}

Encoder *Encoder::Create([[maybe_unused]] ArenaAllocator *arena_allocator, [[maybe_unused]] Arch arch,
                         [[maybe_unused]] bool print_asm, [[maybe_unused]] bool js_number_cast)
{
    switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH32
        case Arch::AARCH32: {
            aarch32::Aarch32Encoder *enc = arena_allocator->New<aarch32::Aarch32Encoder>(arena_allocator);
            enc->SetIsJsNumberCast(js_number_cast);
            if (print_asm) {
                return arena_allocator->New<aarch32::Aarch32Assembly>(arena_allocator, enc);
            }
            return enc;
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        case Arch::AARCH64: {
            aarch64::Aarch64Encoder *enc = arena_allocator->New<aarch64::Aarch64Encoder>(arena_allocator);
            enc->SetIsJsNumberCast(js_number_cast);
            if (print_asm) {
                return arena_allocator->New<aarch64::Aarch64Assembly>(arena_allocator, enc);
            }
            return enc;
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        case Arch::X86_64: {
            amd64::Amd64Encoder *enc =
                arena_allocator->New<amd64::Amd64Encoder>(arena_allocator, Arch::X86_64, js_number_cast);
            if (print_asm) {
                return arena_allocator->New<amd64::Amd64Assembly>(arena_allocator, enc);
            }
            return enc;
        }
#endif
        default:
            return nullptr;
    }
}

RegistersDescription *RegistersDescription::Create([[maybe_unused]] ArenaAllocator *arena_allocator,
                                                   [[maybe_unused]] Arch arch)
{
    switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH32
        case Arch::AARCH32: {
            return arena_allocator->New<aarch32::Aarch32RegisterDescription>(arena_allocator);
        }
#endif

#ifdef PANDA_COMPILER_TARGET_AARCH64
        case Arch::AARCH64: {
            return arena_allocator->New<aarch64::Aarch64RegisterDescription>(arena_allocator);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        case Arch::X86_64: {
            return arena_allocator->New<amd64::Amd64RegisterDescription>(arena_allocator);
        }
#endif
        default:
            return nullptr;
    }
}

CallingConvention *CallingConvention::Create([[maybe_unused]] ArenaAllocator *arena_allocator,
                                             [[maybe_unused]] Encoder *enc,
                                             [[maybe_unused]] RegistersDescription *descr, [[maybe_unused]] Arch arch,
                                             bool is_panda_abi, bool is_osr, bool is_dyn,
                                             [[maybe_unused]] bool print_asm)
{
    [[maybe_unused]] auto mode =
        CallConvMode::Panda(is_panda_abi) | CallConvMode::Osr(is_osr) | CallConvMode::Dyn(is_dyn);
    switch (arch) {
#ifdef PANDA_COMPILER_TARGET_AARCH32
        case Arch::AARCH32: {
            if (print_asm) {
                using printer_type =
                    PrinterCallingConvention<aarch32::Aarch32CallingConvention, aarch32::Aarch32Assembly>;
                return arena_allocator->New<printer_type>(
                    arena_allocator, reinterpret_cast<aarch32::Aarch32Assembly *>(enc), descr, mode);
            }
            return arena_allocator->New<aarch32::Aarch32CallingConvention>(arena_allocator, enc, descr, mode);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_AARCH64
        case Arch::AARCH64: {
            if (print_asm) {
                using printer_type =
                    PrinterCallingConvention<aarch64::Aarch64CallingConvention, aarch64::Aarch64Assembly>;
                return arena_allocator->New<printer_type>(
                    arena_allocator, reinterpret_cast<aarch64::Aarch64Assembly *>(enc), descr, mode);
            }
            return arena_allocator->New<aarch64::Aarch64CallingConvention>(arena_allocator, enc, descr, mode);
        }
#endif
#ifdef PANDA_COMPILER_TARGET_X86_64
        case Arch::X86_64: {
            if (print_asm) {
                using printer_type = PrinterCallingConvention<amd64::Amd64CallingConvention, amd64::Amd64Assembly>;
                return arena_allocator->New<printer_type>(arena_allocator,
                                                          reinterpret_cast<amd64::Amd64Assembly *>(enc), descr, mode);
            }
            return arena_allocator->New<amd64::Amd64CallingConvention>(arena_allocator, enc, descr, mode);
        }
#endif
        default:
            return nullptr;
    }
}
}  // namespace panda::compiler
