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

#include "aot_file.h"
#include "compiler/optimizer/ir/inst.h"
#include "optimizer/ir/runtime_interface.h"
#include "utils/logger.h"
#include "code_info/code_info.h"
#include "mem/gc/gc_types.h"
#include "trace/trace.h"
#include "entrypoints/entrypoints.h"

// In some targets, runtime library is not linked, so linker will fail while searching CallStaticPltResolver symbol.
// To solve this issue, we define this function as weak.
// TODO(msherstennikov): find a better way instead of weak function, e.g. make aot_manager library static.
extern "C" void CallStaticPltResolver([[maybe_unused]] void *slot) __attribute__((weak));
extern "C" void CallStaticPltResolver([[maybe_unused]] void *slot) {}

namespace panda::compiler {
static inline Expected<const uint8_t *, std::string> LoadSymbol(const panda::os::library_loader::LibraryHandle &handle,
                                                                const char *name)
{
    auto sym = panda::os::library_loader::ResolveSymbol(handle, name);
    if (!sym) {
        return Unexpected(sym.Error().ToString());
    }
    return reinterpret_cast<uint8_t *>(sym.Value());
}

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOAD_AOT_SYMBOL(name)                                           \
    auto name = LoadSymbol(handle, #name);                              \
    if (!name) {                                                        \
        return Unexpected("Cannot load name section: " + name.Error()); \
    }

Expected<std::unique_ptr<AotFile>, std::string> AotFile::Open(const std::string &file_name, uint32_t gc_type,
                                                              bool for_dump)
{
    trace::ScopedTrace scoped_trace("Open aot file " + file_name);
    auto handle_load = panda::os::library_loader::Load(file_name);
    if (!handle_load) {
        return Unexpected("AOT elf library open failed: " + handle_load.Error().ToString());
    }
    auto handle = std::move(handle_load.Value());

    LOAD_AOT_SYMBOL(aot);
    LOAD_AOT_SYMBOL(aot_end);
    LOAD_AOT_SYMBOL(code);
    LOAD_AOT_SYMBOL(code_end);

    if (code_end.Value() < code.Value() || aot_end.Value() <= aot.Value()) {
        return Unexpected(std::string("Invalid symbols"));
    }

    auto aot_header = reinterpret_cast<const AotHeader *>(aot.Value());
    if (aot_header->magic != MAGIC) {
        return Unexpected(std::string("Wrong AotHeader magic"));
    }

    if (aot_header->version != VERSION) {
        return Unexpected(std::string("Wrong AotHeader version"));
    }

    if (!for_dump && aot_header->environment_checksum != RuntimeInterface::GetEnvironmentChecksum(RUNTIME_ARCH)) {
        return Unexpected(std::string("Compiler environment checksum mismatch"));
    }

    if (!for_dump && aot_header->gc_type != gc_type) {
        return Unexpected(std::string("Wrong AotHeader gc-type: ") +
                          std::string(mem::GCStringFromType(static_cast<mem::GCType>(aot_header->gc_type))) + " vs " +
                          std::string(mem::GCStringFromType(static_cast<mem::GCType>(gc_type))));
    }
    return std::make_unique<AotFile>(std::move(handle), Span(aot.Value(), aot_end.Value() - aot.Value()),
                                     Span(code.Value(), code_end.Value() - code.Value()));
}

void AotFile::InitializeGot(RuntimeInterface *runtime)
{
    size_t minus_first_slot = static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) + 1;
    auto *table = const_cast<uintptr_t *>(
        reinterpret_cast<const uintptr_t *>(code_.data() - minus_first_slot * PointerSize(RUNTIME_ARCH)));

    while (*table != 0) {
        switch (*table) {
            case AotSlotType::PLT_SLOT:
                table -= 2U;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[1] = reinterpret_cast<uintptr_t>(CallStaticPltResolver);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[2U] = reinterpret_cast<uintptr_t>(
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                    table + 1 - runtime->GetCompiledEntryPointOffset(RUNTIME_ARCH) / sizeof(uintptr_t));
                break;
            case AotSlotType::VTABLE_INDEX:
                table--;       // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[1] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            case AotSlotType::CLASS_SLOT:
                table -= 2U;    // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[1] = 0;   // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[2U] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            case AotSlotType::STRING_SLOT:
                table--;       // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                table[1] = 0;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                break;
            case AotSlotType::INLINECACHE_SLOT:
                break;
            default:
                UNREACHABLE();
                break;
        }
        table--;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
}

void AotFile::PatchTable(RuntimeInterface *runtime)
{
    auto *table = const_cast<uintptr_t *>(reinterpret_cast<const uintptr_t *>(
        code_.data() - static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT) * PointerSize(RUNTIME_ARCH)));
    for (size_t i = 0; i < static_cast<size_t>(RuntimeInterface::IntrinsicId::COUNT); i++) {
        IntrinsicInst inst(Opcode::Intrinsic, static_cast<RuntimeInterface::IntrinsicId>(i));
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        table[i] = runtime->GetIntrinsicAddress(inst.IsRuntimeCall(), static_cast<RuntimeInterface::IntrinsicId>(i));
    }
}

AotClass AotPandaFile::GetClass(uint32_t class_id) const
{
    auto classes = aot_file_->GetClassHeaders(*header_);
    auto it = std::lower_bound(classes.begin(), classes.end(), class_id,
                               [](const auto &a, uintptr_t klass_id) { return a.class_id < klass_id; });
    if (it == classes.end() || it->class_id != class_id) {
        return {};
    }
    ASSERT(it->methods_count != 0 && "AOT file shall not contain empty classes");
    return AotClass(aot_file_, &*it);
}

const void *AotClass::FindMethodCodeEntry(size_t index) const
{
    auto method_header = FindMethodHeader(index);
    if (method_header == nullptr) {
        return nullptr;
    }

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return aot_file_->GetMethodCode(method_header) + CodeInfo::GetCodeOffset(RUNTIME_ARCH);
}

Span<const uint8_t> AotClass::FindMethodCodeSpan(size_t index) const
{
    auto method_header = FindMethodHeader(index);
    if (method_header == nullptr) {
        return {};
    }
    auto code = Span(aot_file_->GetMethodCode(method_header), method_header->code_size);
    return CodeInfo(code).GetCodeSpan();
}

const MethodHeader *AotClass::FindMethodHeader(size_t index) const
{
    auto bitmap = GetBitmap();
    CHECK_LT(index, bitmap.size());
    if (!bitmap[index]) {
        return nullptr;
    }
    auto method_index = bitmap.PopCount(index);
    ASSERT(method_index < header_->methods_count);
    return aot_file_->GetMethodHeader(header_->methods_offset + method_index);
}

BitVectorSpan AotClass::GetBitmap() const
{
    // TODO(msherstennikov): remove const_cast once BitVector support constant storage
    auto bitmap_base = const_cast<uint32_t *>(reinterpret_cast<const uint32_t *>(aot_file_->GetMethodsBitmap()));
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return BitVectorSpan(bitmap_base + header_->methods_bitmap_offset, header_->methods_bitmap_size);
}

}  // namespace panda::compiler
