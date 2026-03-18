/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_KLASS_HELPER_H_
#define PANDA_RUNTIME_KLASS_HELPER_H_

#include <cstdint>
#include <string>
#include <string_view>

#include "libarkbase/utils/span.h"
#include "libarkbase/utils/utf.h"
#include "libarkfile/file_items.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/signature_utils.h"
#include "runtime/object_header_config.h"

namespace ark {

// Small helper
template <class Config>
class ClassConfig {
public:
    using ClassWordSize = typename Config::Size;
};

class Class;
class ClassLinker;
class ClassLinkerContext;
class ClassLinkerExtension;
class ClassLinkerErrorHandler;

class ClassHelper : private ClassConfig<MemoryModelConfig> {
public:
    using ClassWordSize = typename ClassConfig::ClassWordSize;  // To be visible outside

    static constexpr size_t OBJECT_POINTER_SIZE = sizeof(ClassWordSize);
    // In general for any T: sizeof(T*) != OBJECT_POINTER_SIZE
    static constexpr size_t POINTER_SIZE = sizeof(uintptr_t);

    PANDA_PUBLIC_API static const uint8_t *GetDescriptor(const uint8_t *name, PandaString *storage);

    static const uint8_t *GetReferenceDescriptor(const PandaString &name, PandaString *storage);

    static const uint8_t *GetPrimitiveDescriptor(panda_file::Type type, PandaString *storage);

    static const uint8_t *GetArrayDescriptor(const uint8_t *componentName, size_t rank, PandaString *storage);

    static const uint8_t *GetPrimitiveArrayDescriptor(panda_file::Type type, size_t rank, PandaString *storage);

    PANDA_PUBLIC_API static std::string GetName(const uint8_t *descriptor);

    static bool IsArrayDescriptor(const uint8_t *descriptor)
    {
        Span<const uint8_t> sp(descriptor, 1);
        return sp[0] == '[';
    }

    static const uint8_t *GetComponentDescriptor(const uint8_t *descriptor)
    {
        ASSERT(IsArrayDescriptor(descriptor));
        Span<const uint8_t> sp(descriptor, 1);
        return sp.cend();
    }

    static bool IsPrimitive(const uint8_t *descriptor);
    static bool IsReference(const uint8_t *descriptor);
    static bool IsUnionDescriptor(const uint8_t *descriptor);
    static Span<const uint8_t> GetUnionComponent(const uint8_t *descriptor);

private:
    static char GetPrimitiveTypeDescriptorChar(panda_file::Type::TypeId typeId);

    static const uint8_t *GetPrimitiveTypeDescriptorStr(panda_file::Type::TypeId typeId);

    static size_t GetDimensionality(const uint8_t *descriptor)
    {
        ASSERT(IsArrayDescriptor(descriptor));
        size_t dim = 0;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        while (*descriptor++ == '[') {
            ++dim;
        }
        return dim;
    }

    friend class RuntimeDescriptorParser;
    friend class ClassPublicNameParser;

    // NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
    inline static const std::unordered_map<char, std::string_view> PRIMITIVE_RUNTIME_NAMES = {
        {'V', "void"}, {'Z', "u1"},  {'B', "i8"},  {'H', "u8"},  {'S', "i16"}, {'C', "u16"}, {'I', "i32"}, {'U', "u32"},
        {'J', "i64"},  {'Q', "u64"}, {'F', "f32"}, {'D', "f64"}, {'A', "any"}, {'Y', "Y"},   {'N', "N"}};
};

}  // namespace ark

#endif  // PANDA_RUNTIME_KLASS_HELPER_H_
