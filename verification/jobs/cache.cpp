/*
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

#include "cache.h"

#include "file_items.h"
#include "macros.h"
#include "runtime/include/runtime.h"
#include "runtime/include/class.h"
#include "runtime/include/method.h"
#include "runtime/include/field.h"
#include "runtime/include/class_helper.h"
#include "runtime/include/language_context.h"
#include "libpandabase/utils/logger.h"
#include "libpandabase/utils/utf.h"
#include "libpandabase/utils/hash.h"
#include "libpandafile/method_data_accessor-inl.h"
#include "libpandafile/class_data_accessor.h"
#include "libpandafile/class_data_accessor-inl.h"
#include "libpandafile/code_data_accessor.h"
#include "libpandafile/code_data_accessor-inl.h"
#include "libpandafile/field_data_accessor-inl.h"
#include "libpandafile/proto_data_accessor.h"
#include "libpandafile/proto_data_accessor-inl.h"
#include "libpandafile/modifiers.h"
#include "plugins.h"
#include "verifier_messages.h"
#include "file_items.h"

namespace panda::verifier {

using FastAPIClassRW = LibCache::FastAPIClass<LibCache::Access::READ_WRITE>;
using FastAPIClassRO = LibCache::FastAPIClass<LibCache::Access::READ_ONLY>;

PandaString LibCache::GetName(const LibCache::CachedClass &cachedClass)
{
    if (cachedClass.type_id == panda_file::Type::TypeId::REFERENCE) {
        return ClassHelper::GetName<PandaString>(cachedClass.descriptor);
    }
    return {ClassHelper::GetPrimitiveTypeStr(cachedClass.type_id)};
}

PandaString LibCache::GetName(const DescriptorString &descriptor)
{
    return ClassHelper::GetName<PandaString>(descriptor);
}

PandaString LibCache::GetName(const LibCache::CachedMethod &cachedMethod)
{
    PandaString str = GetName(cachedMethod.klass);
    str += "::";
    str += utf::Mutf8AsCString(cachedMethod.name);
    str += " : ";
    size_t idx = 0;
    for (const auto &arg : cachedMethod.signature) {
        if (idx > 1) {
            str += ", ";
        }
        str += GetName(arg);
        if (idx == 0) {
            str += "(";
        }
        ++idx;
    }
    str += ")";
    return str;
}

PandaString LibCache::GetName(const LibCache::CachedField &cachedField)
{
    auto str = GetName(cachedField.klass);
    str += ".";
    str += utf::Mutf8AsCString(cachedField.name);
    str += " : ";
    str += GetName(cachedField.type);
    return str;
}

template <>
bool FastAPIClassRW::Link(LibCache::CachedClass *cachedClass, bool reportError);

template <>
bool FastAPIClassRW::Link(LibCache::CachedMethod *cachedMethod, bool reportError);

template <>
bool FastAPIClassRW::Link(LibCache::CachedField *cachedField, bool reportError);

template <>
OptionalRef<LibCache::CachedMethod> FastAPIClassRW::GetMethod(panda_file::SourceLang src_lang, LibCache::Id id,
                                                              bool reportError)
{
    return GetFromCacheAndLink(&GetContext(src_lang).method_cache, id, reportError);
}

namespace {

std::string StringDataToString(panda_file::File::StringData sd)
{
    std::string res {reinterpret_cast<char *>(const_cast<uint8_t *>(sd.data))};
    return res;
}

LibCache::CachedClass::FlagsValue GetClassFlags(uint32_t raw_flags)
{
    LibCache::CachedClass::FlagsValue flags;
    flags[LibCache::CachedClass::Flag::PUBLIC] = (raw_flags & ACC_PUBLIC) != 0;
    flags[LibCache::CachedClass::Flag::FINAL] = (raw_flags & ACC_FINAL) != 0;
    flags[LibCache::CachedClass::Flag::ANNOTATION] = (raw_flags & ACC_ANNOTATION) != 0;
    flags[LibCache::CachedClass::Flag::ENUM] = (raw_flags & ACC_ENUM) != 0;

    flags[LibCache::CachedClass::Flag::ABSTRACT] = (raw_flags & ACC_ABSTRACT) != 0;
    flags[LibCache::CachedClass::Flag::INTERFACE] = (raw_flags & ACC_INTERFACE) != 0;

    return flags;
}

LibCache::CachedMethod::FlagsValue GetMethodFlags(const panda_file::File &file,
                                                  const panda_file::MethodDataAccessor &mda,
                                                  panda_file::SourceLang src_lang)
{
    LibCache::CachedMethod::FlagsValue flags;

    flags[LibCache::CachedMethod::Flag::STATIC] = mda.IsStatic();
    flags[LibCache::CachedMethod::Flag::NATIVE] = mda.IsNative();
    flags[LibCache::CachedMethod::Flag::PUBLIC] = mda.IsPublic();
    flags[LibCache::CachedMethod::Flag::PRIVATE] = mda.IsPrivate();
    flags[LibCache::CachedMethod::Flag::PROTECTED] = mda.IsProtected();
    flags[LibCache::CachedMethod::Flag::SYNTHETIC] = mda.IsSynthetic();
    flags[LibCache::CachedMethod::Flag::ABSTRACT] = mda.IsAbstract();
    flags[LibCache::CachedMethod::Flag::FINAL] = mda.IsFinal();

    const auto method_name_raw = StringDataToString(file.GetStringData(mda.GetNameId()));
    std::string ctor_name = panda::panda_file::GetCtorName(src_lang);
    std::string static_ctor_name = panda::panda_file::GetCctorName(src_lang);

    flags[LibCache::CachedMethod::Flag::CONSTRUCTOR] = (method_name_raw == ctor_name);
    flags[LibCache::CachedMethod::Flag::STATIC_CONSTRUCTOR] = (method_name_raw == static_ctor_name);

    return flags;
}

LibCache::CachedField::FlagsValue GetFieldFlags(const panda_file::FieldDataAccessor &fda)
{
    LibCache::CachedField::FlagsValue flags;
    flags[LibCache::CachedField::Flag::STATIC] = fda.IsStatic();
    flags[LibCache::CachedField::Flag::VOLATILE] = fda.IsVolatile();
    flags[LibCache::CachedField::Flag::PUBLIC] = fda.IsPublic();
    flags[LibCache::CachedField::Flag::PROTECTED] = fda.IsProtected();
    flags[LibCache::CachedField::Flag::FINAL] = fda.IsFinal();
    flags[LibCache::CachedField::Flag::PRIVATE] = fda.IsPrivate();
    return flags;
}

void LogConflictingClassDefinitions(const LibCache::CachedClass &cachedClass,
                                    const LibCache::CachedClass &already_cached_class)
{
    auto get_file = [](const LibCache::CachedClass &cached_class1) {
        if (auto file = cached_class1.file; file.HasRef()) {
            PandaString result {"in "};
            result.append(file->GetFullFileName());
            return result;
        }
        return PandaString {"as a synthetic class"};
    };

    auto &options = Runtime::GetCurrent()->GetVerificationOptions().Debug.GetMethodOptions().GetOptions("default");
    options.Msg(VerifierMessage::ConflictingClassDefinitions).IfNotHidden([&]() {
        LOG_VERIFIER_CONFLICTING_CLASS_DEFINITIONS(LibCache::GetName(cachedClass), get_file(already_cached_class),
                                                   get_file(cachedClass));
    });
}

}  // namespace

template <>
LibCache::CachedClass &FastAPIClassRW::MakeSyntheticClass(panda_file::SourceLang src_lang, const uint8_t *descriptor,
                                                          panda_file::Type::TypeId type_id, uint32_t flags)
{
    auto &data = GetContext(src_lang);

    auto id = Class::CalcUniqId(descriptor);

    auto emplace_result =
        data.class_cache.try_emplace(id, id, descriptor, src_lang, type_id, GetClassFlags(flags),
                                     OptionalConstRef<panda_file::File> {}, panda_file::File::EntityId {});
    ASSERT(emplace_result.second);
    CachedClass &cachedClass = emplace_result.first->second;

    auto descr_emplace_result = data.descr_lookup.try_emplace(cachedClass.descriptor, std::ref(cachedClass));
    if (descr_emplace_result.second) {
        LOG(DEBUG, VERIFIER) << "CACHE: Added synthetic class " << GetName(cachedClass) << std::hex << " with id 0x"
                             << id;
    } else {
        // See the comment on the other `if (descr_emplace_result.second)`
        LibCache::CachedClass &already_cached_class = descr_emplace_result.first->second.get();
        LogConflictingClassDefinitions(cachedClass, already_cached_class);
        // do not return already_cached_class, it might give wrong results in InitializeRoots
    }

    return cachedClass;
}

static LibCache::MethodHash CalcMethodHash(const LibCache::CachedMethod &cachedMethod);

template <>
template <typename SignatureFiller>
LibCache::CachedMethod &FastAPIClassRW::MakeSyntheticMethod(LibCache::CachedClass &cachedClass, const uint8_t *name,
                                                            bool is_static, SignatureFiller sig_filler)
{
    auto id = Method::CalcUniqId(cachedClass.descriptor, name);

    static Indexes empty_index;

    // TODO(romanov) ideally we want not to construct cachedMethod here but directly in the try_emplace call, like in
    // MakeSyntheticClass. Currently done this way because id gets modified after construction, so we don't know the key
    // to use for try_emplace
    CachedMethod cachedMethod {id, 0, name, cachedClass, empty_index, {}, {}, {}};

    sig_filler(cachedClass, cachedMethod);
    cachedMethod.flags[CachedMethod::Flag::STATIC] = is_static;
    cachedMethod.hash = CalcMethodHash(cachedMethod);
    auto &data = GetContext(cachedClass.source_lang);
    const auto MASK = 0xFFFFULL;
    const auto SHIFT = 0x8ULL;
    cachedMethod.id += ((cachedMethod.hash & MASK) << SHIFT) + cachedMethod.num_args;
    auto &result = data.method_cache.try_emplace(cachedMethod.id, std::move(cachedMethod)).first->second;
    cachedClass.methods.insert_or_assign(result.hash, std::ref(result));
    LOG(DEBUG, VERIFIER) << "CACHE: Added synthetic method '" << GetName(result) << std::hex << " with id 0x"
                         << result.id << ", hash 0x" << result.hash;
    return result;
}

template <>
LibCache::CachedClass &FastAPIClassRW::AddArray(panda_file::SourceLang src_lang, const uint8_t *descr)
{
    auto &data = GetContext(src_lang);
    auto &array =
        MakeSyntheticClass(src_lang, descr, panda_file::Type::TypeId::REFERENCE, ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    array.flags[CachedClass::Flag::ARRAY_CLASS] = true;
    array.ancestors.push_back(data.object_descr);
    // add Clonable in ancestors for java
    // add Serializeable in ancestors for java
    auto comp_descr = DescriptorString(ClassHelper::GetComponentDescriptor(descr));
    array.array_component = comp_descr;
    if (comp_descr.GetLength() > 1) {
        array.flags[CachedClass::Flag::OBJECT_ARRAY_CLASS] = true;
    }
    const uint8_t *ctor_name = panda::plugins::GetLanguageContextBase(src_lang)->GetCtorName();
    CachedClassRefOrDescriptor i32_class {data.GetPrimitiveClass(panda_file::Type::TypeId::I32)};
    for (size_t dimensions = 0; dimensions <= ClassHelper::GetDimensionality(descr); ++dimensions) {
        auto &ctor = MakeSyntheticMethod(array, ctor_name, true, [&](CachedClass &c, CachedMethod &cm) {
            cm.num_args = dimensions;
            // method return type first
            cm.signature.push_back(std::ref(c));
            cm.signature.insert(cm.signature.end(), dimensions, i32_class);
        });
        ctor.flags[CachedMethod::Flag::ARRAY_CONSTRUCTOR] = true;
    }
    return array;
}

template <>
void FastAPIClassRW::InitializeRootClasses(panda_file::SourceLang lang)
{
    const LanguageContextBase *ctx = panda::plugins::GetLanguageContextBase(lang);
    DescriptorString obj_descriptor = ctx->GetObjectClassDescriptor();

    auto &data = GetContext(lang);

    data.object_descr = obj_descriptor;
    data.string_descr = ctx->GetStringClassDescriptor();
    data.string_array_descr = ctx->GetStringArrayClassDescriptor();

    VerificationInitAPI v_api = ctx->GetVerificationInitAPI();

    // primitive
    for (panda_file::Type::TypeId type_id : v_api.primitive_roots_for_verification) {
        auto &C = MakeSyntheticClass(lang, ClassHelper::GetPrimitiveTypeDescriptorStr(type_id), type_id,
                                     ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
        C.flags[CachedClass::Flag::PRIMITIVE] = true;
        data.primitive_classes[type_id] = C;
    }

    for (const uint8_t *el : v_api.array_elements_for_verification) {
        AddArray(lang, el);
    }

    // object
    if (v_api.is_need_object_synthetic_class) {
        MakeSyntheticClass(lang, obj_descriptor, panda_file::Type::TypeId::REFERENCE,
                           ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
    }

    if (v_api.is_need_string_synthetic_class) {
        auto &Str = MakeSyntheticClass(lang, data.string_descr, panda_file::Type::TypeId::REFERENCE,
                                       ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
        Str.ancestors.push_back(obj_descriptor);
    }

    if (v_api.is_need_class_synthetic_class) {
        auto &Class = MakeSyntheticClass(lang, ctx->GetClassClassDescriptor(), panda_file::Type::TypeId::REFERENCE,
                                         ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT);
        Class.ancestors.push_back(obj_descriptor);
    }
}

LibCache::LibCache()
{
    static bool initialized = false;
    if (initialized) {
        return;
    }

    FastAPIClassRW FAPI = FastAPI();
    for (panda::panda_file::SourceLang lang : panda::panda_file::LANG_ITERATOR) {
        FAPI.InitializeRootClasses(lang);
    }

    initialized = true;
}

template <typename Handler>
static LibCache::MethodHash CalcMethodHash(const uint8_t *name, Handler &&handler)
{
    uint64_t name_hash = StdHash(LibCache::DescriptorString {name});
    uint64_t sig_hash = FNV_INITIAL_SEED;
    auto hash_str = [&sig_hash](const LibCache::DescriptorString &descr) {
        sig_hash = PseudoFnvHashItem(StdHash(descr), sig_hash);
    };
    handler(hash_str);
    auto constexpr SHIFT = 32U;
    return (name_hash << SHIFT) | sig_hash;
}

static LibCache::MethodHash CalcMethodHash(const panda_file::File &pf, const panda_file::MethodDataAccessor &mda)
{
    return CalcMethodHash(pf.GetStringData(mda.GetNameId()).data, [&](auto hash_str) {
        if (mda.IsStatic()) {
            hash_str(utf::CStringAsMutf8("static"));
        }
        const_cast<panda_file::MethodDataAccessor &>(mda).EnumerateTypesInProto(
            [&](auto type, auto class_file_id) {
                if (type.GetId() == panda_file::Type::TypeId::REFERENCE) {
                    auto string_data = pf.GetStringData(class_file_id).data;
                    hash_str(string_data);
                } else {
                    hash_str(ClassHelper::GetPrimitiveTypeDescriptorStr(type.GetId()));
                }
            },
            // skip 'this' parameter of method
            true);
    });
}

static LibCache::MethodHash CalcMethodHash(const LibCache::CachedMethod &cachedMethod)
{
    return CalcMethodHash(cachedMethod.name, [&](auto hash_str) {
        bool is_static = cachedMethod.flags[LibCache::CachedMethod::Flag::STATIC];
        if (is_static) {
            hash_str(utf::CStringAsMutf8("static"));
        }
        size_t idx = 0;
        for (const auto &arg : cachedMethod.signature) {
            if (idx == 1) {
                // skip 'this' parameter of method
                ++idx;
                continue;
            }
            hash_str(LibCache::GetDescriptor(arg));
            if (!is_static) {
                idx++;
            }
        }
    });
}

static LibCache::FieldHash CalcFieldNameAndTypeHash(const panda_file::File &pf,
                                                    const panda_file::FieldDataAccessor &fda)
{
    uint64_t name_hash = PseudoFnvHashString(pf.GetStringData(fda.GetNameId()).data);

    auto type = panda_file::Type::GetTypeFromFieldEncoding(fda.GetType());

    uint64_t type_hash;
    if (type.GetId() != panda_file::Type::TypeId::REFERENCE) {
        type_hash = PseudoFnvHashItem(ClassHelper::GetPrimitiveTypeDescriptorChar(type.GetId()));
    } else {
        auto type_class_id = panda_file::File::EntityId(fda.GetType());
        const uint8_t *descr = pf.GetStringData(type_class_id).data;
        type_hash = PseudoFnvHashString(descr);
    }

    auto constexpr SHIFT = 32U;

    uint64_t hash = (name_hash << SHIFT) | type_hash;

    return hash;
}

static OptionalRef<LibCache::CachedMethod> TryResolveMethod(LibCache::CachedClass *klass,
                                                            LibCache::MethodHash method_hash)
{
    auto it = klass->methods.find(method_hash);
    if (it == klass->methods.end()) {
        // search in ancestors
        for (auto &ancestor : klass->ancestors) {
            ASSERT(LibCache::IsRef(ancestor));
            auto method_ref = TryResolveMethod(&LibCache::GetRef(ancestor), method_hash);
            if (method_ref.HasRef()) {
                // place found method in topmost methods table
                // to speedup future lookups
                // Think about global cache: (class_id, method_hash) -> method_ref,
                // it may be more effectife in terms of mem consumption, but less
                // effective in terms of cpu d-cache locality
                klass->methods.insert_or_assign(method_hash, *method_ref);
                return method_ref;
            }
        }
        return {};
    }

    return it->second.get();
}

OptionalRef<LibCache::CachedMethod> LibCache::CachedClass::ResolveMethod(LibCache::MethodHash method_hash)
{
    // assumption: ResolveMethod is called on linked class
    ASSERT(linked);
    auto method_ref = TryResolveMethod(this, method_hash);
    if (!method_ref.HasRef()) {
        LOG(WARNING, VERIFIER) << std::hex << "Failed to resolve method with hash 0x" << method_hash << " in class "
                               << GetName();
    }

    return method_ref;
}

static OptionalRef<LibCache::CachedField> TryResolveField(LibCache::CachedClass *klass, LibCache::FieldHash field_hash)
{
    auto it = klass->fields.find(field_hash);
    if (it == klass->fields.end()) {
        // search in ancestors
        for (auto &ancestor : klass->ancestors) {
            ASSERT(LibCache::IsRef(ancestor));
            auto field_ref = TryResolveField(&LibCache::GetRef(ancestor), field_hash);
            if (field_ref.HasRef()) {
                // see comment in ResolveMethod(MethodHash)
                klass->fields.insert_or_assign(field_hash, *field_ref);
                return field_ref;
            }
        }
        return {};
    }

    return it->second.get();
}

OptionalRef<LibCache::CachedField> LibCache::CachedClass::ResolveField(LibCache::FieldHash field_hash)
{
    // assumption: ResolveField is called on linked class
    ASSERT(linked);

    auto field_ref = TryResolveField(this, field_hash);
    if (!field_ref.HasRef()) {
        LOG(WARNING, VERIFIER) << std::hex << "Failed to resolve field with hash 0x" << field_hash << " in class "
                               << GetName();
    }

    return field_ref;
}

static void InitializeIndex(LibCache::Indexes *indexes, LibCache::LangContext *data, const panda_file::File &pf,
                            const panda_file::File::IndexHeader *header)
{
    auto map_span_to_vector = [](auto *vector, auto entity_id_span, auto func) {
        // all vectors we pass must be empty
        ASSERT(vector->empty());
        vector->reserve(entity_id_span.Size());
        for (auto id : entity_id_span) {
            vector->emplace_back(func(id));
        }
        vector->shrink_to_fit();
    };

    // extracted to a variable to avoid a CodeCheck bug
    auto id_to_ref_or_descriptor = [&](auto class_id) -> LibCache::CachedClassRefOrDescriptor {
        auto type = panda_file::Type::GetTypeFromFieldEncoding(class_id.GetOffset());

        if (type.IsReference()) {
            return DescriptorString {pf.GetStringData(class_id).data};
        }

        return data->GetPrimitiveClass(type);
    };

    map_span_to_vector(&(indexes->classes), pf.GetClassIndex(header), id_to_ref_or_descriptor);
    auto id_to_ref_or_id = [](auto entity_id) { return entity_id; };
    map_span_to_vector(&(indexes->methods), pf.GetMethodIndex(header), id_to_ref_or_id);
    map_span_to_vector(&(indexes->fields), pf.GetFieldIndex(header), id_to_ref_or_id);
}

static void InitializeSignature(LibCache::CachedMethod *cachedMethod, const panda_file::MethodDataAccessor &mda,
                                LibCache::LangContext *data)
{
    auto &pf = cachedMethod->file.Get();

    const_cast<panda_file::MethodDataAccessor &>(mda).EnumerateTypesInProto([&](auto type, auto class_file_id) {
        auto type_id = type.GetId();
        if (type_id == panda_file::Type::TypeId::REFERENCE) {
            DescriptorString descr {pf.GetStringData(class_file_id).data};
            cachedMethod->signature.push_back(descr);
        } else {
            cachedMethod->signature.push_back(data->GetPrimitiveClass(type_id));
        }
    });
}

static bool InitializeTryBlock(const panda_file::CodeDataAccessor::TryBlock &try_block,
                               LibCache::CachedMethod *cachedMethod)
{
    auto try_block_start = reinterpret_cast<const uint8_t *>(reinterpret_cast<uintptr_t>(cachedMethod->bytecode) +
                                                             static_cast<uintptr_t>(try_block.GetStartPc()));
    auto try_block_end = reinterpret_cast<const uint8_t *>(reinterpret_cast<uintptr_t>(try_block_start) +
                                                           static_cast<uintptr_t>(try_block.GetLength()));
    const_cast<panda_file::CodeDataAccessor::TryBlock &>(try_block).EnumerateCatchBlocks([&](const auto &catch_block) {
        auto handler_pc_ptr = reinterpret_cast<const uint8_t *>(reinterpret_cast<uintptr_t>(cachedMethod->bytecode) +
                                                                static_cast<uintptr_t>(catch_block.GetHandlerPc()));
        auto &cached_catch_block = cachedMethod->catch_blocks.emplace_back(LibCache::CachedCatchBlock {
            try_block_start, try_block_end, LibCache::InvalidDescriptor, handler_pc_ptr, catch_block.GetCodeSize()});
        auto type_idx = catch_block.GetTypeIdx();
        if (type_idx != panda_file::INVALID_INDEX) {
            auto &class_index = cachedMethod->indexes.classes;
            if (type_idx < class_index.size()) {
                cached_catch_block.exception_type = class_index[type_idx];
            } else {
                LOG(WARNING, VERIFIER) << "Exception type out of bounds in " << cachedMethod->GetName()
                                       << " class index, index " << type_idx << ", size " << class_index.size();
            }
        }
        return true;
    });
    return true;
}

static void InitializeCode(LibCache::CachedMethod *cachedMethod, const panda_file::MethodDataAccessor &mda)
{
    auto code_id = const_cast<panda_file::MethodDataAccessor &>(mda).GetCodeId();
    if (!code_id) {
        cachedMethod->num_vregs = 0;
        cachedMethod->num_args = 0;
        cachedMethod->bytecode = nullptr;
        cachedMethod->bytecode_size = 0;
        return;
    }

    auto &pf = cachedMethod->file.Get();
    panda_file::CodeDataAccessor cda {pf, *code_id};
    cachedMethod->num_vregs = cda.GetNumVregs();
    cachedMethod->num_args = cda.GetNumArgs();
    cachedMethod->bytecode = cda.GetInstructions();
    cachedMethod->bytecode_size = cda.GetCodeSize();
    cda.EnumerateTryBlocks([&](const auto &try_block) { return InitializeTryBlock(try_block, cachedMethod); });
    cachedMethod->catch_blocks.shrink_to_fit();
}

template <>
LibCache::CachedMethod &FastAPIClassRW::ProcessMethod(LibCache::CachedClass *cachedClass, const panda_file::File &pf,
                                                      const panda_file::MethodDataAccessor &mda)
{
    auto file_id = mda.GetMethodId();

    auto id = Method::CalcUniqId(&pf, file_id);

    panda_file::SourceLang src_lang =
        const_cast<panda_file::MethodDataAccessor &>(mda).GetSourceLang().value_or(cachedClass->source_lang);

    auto &data = GetContext(src_lang);

    auto cached_method_ref = GetFromCacheAndLink(&data.method_cache, id);

    if (cached_method_ref.HasRef()) {
        return *cached_method_ref;
    }

    const panda_file::File::IndexHeader *header = pf.GetIndexHeader(file_id);
    auto [iter, is_new] = data.indexes_cache.try_emplace(header);
    Indexes &indexes = iter->second;
    if (is_new) {
        InitializeIndex(&indexes, &data, pf, header);
    }

    auto emplace_result =
        data.method_cache.try_emplace(id,
                                      // CachedMethod constructor args
                                      id, CalcMethodHash(pf, mda), pf.GetStringData(mda.GetNameId()).data, *cachedClass,
                                      indexes, GetMethodFlags(pf, mda, src_lang), pf, file_id);
    ASSERT(emplace_result.second);
    CachedMethod &cachedMethod = emplace_result.first->second;

    InitializeSignature(&cachedMethod, mda, &data);
    InitializeCode(&cachedMethod, mda);

    data.file_cache.AddToCache(pf, file_id, cachedMethod);

    LOG(DEBUG, VERIFIER) << "CACHE: Added method '" << GetName(cachedMethod) << std::hex << "' with id 0x" << id
                         << ", hash 0x" << cachedMethod.hash;

    return cachedMethod;
}

template <>
LibCache::CachedField &FastAPIClassRW::ProcessField(LibCache::CachedClass *cachedClass, const panda_file::File &pf,
                                                    const panda_file::FieldDataAccessor &fda)
{
    auto file_id = fda.GetFieldId();

    auto id = Field::CalcUniqId(&pf, file_id);

    auto src_lang = cachedClass->source_lang;

    auto &data = GetContext(src_lang);

    OptionalRef<CachedField> cached_field_ref = GetFromCacheAndLink(&data.field_cache, id);

    if (cached_field_ref.HasRef()) {
        return *cached_field_ref;
    }

    auto emplace_result = data.field_cache.try_emplace(id,
                                                       // CachedField constructor args
                                                       id, pf.GetStringData(fda.GetNameId()).data, *cachedClass,
                                                       GetFieldFlags(fda), pf, file_id);
    ASSERT(emplace_result.second);
    CachedField &cachedField = emplace_result.first->second;

    auto type = panda_file::Type::GetTypeFromFieldEncoding(fda.GetType());

    // NB! keep hashing in sync with CalcFieldNameAndTypeHash
    uint64_t name_hash = PseudoFnvHashString(cachedField.name);

    uint64_t type_hash;

    if (type.GetId() != panda_file::Type::TypeId::REFERENCE) {
        cachedField.type = data.GetPrimitiveClass(type);
        type_hash = PseudoFnvHashItem(ClassHelper::GetPrimitiveTypeDescriptorChar(type.GetId()));
    } else {
        auto type_class_id = panda_file::File::EntityId(fda.GetType());
        const uint8_t *descr = pf.GetStringData(type_class_id).data;
        cachedField.type = DescriptorString(descr);
        type_hash = PseudoFnvHashString(descr);
    }

    auto constexpr SHIFT = 32U;

    cachedField.hash = (name_hash << SHIFT) | type_hash;

    LOG(DEBUG, VERIFIER) << "CACHE: Added field '" << GetName(cachedField) << std::hex << "' with id 0x" << id
                         << ", hash 0x" << cachedField.hash;

    data.file_cache.AddToCache(pf, file_id, cachedField);

    return cachedField;
}

static void AddAncestors(LibCache::CachedClass *cachedClass, panda_file::ClassDataAccessor *cda,
                         const LibCache::LangContext &data)
{
    auto &pf = cda->GetPandaFile();

    auto add_ancestor = [&](auto entity_id) {
        DescriptorString descr =
            entity_id.GetOffset() == 0 ? data.object_descr : DescriptorString {pf.GetStringData(entity_id).data};
        if (descr != cachedClass->descriptor) {
            cachedClass->ancestors.emplace_back(descr);
        }
    };

    cda->EnumerateInterfaces(add_ancestor);

    auto super_class_id = cda->GetSuperClassId();

    add_ancestor(super_class_id);
}

template <>
void FastAPIClassRW::ProcessClass(const panda_file::File &pf, panda_file::File::EntityId entity_id)
{
    auto id = Class::CalcUniqId(&pf, entity_id);

    panda_file::ClassDataAccessor cda {pf, entity_id};

    panda_file::SourceLang src_lang = cda.GetSourceLang().value_or(panda_file::SourceLang::PANDA_ASSEMBLY);

    auto &data = GetContext(src_lang);

    auto cached_class_ref = GetFromCacheAndLink(&data.class_cache, id);

    if (cached_class_ref.HasRef()) {
        return;
    }

    auto emplace_result =
        data.class_cache.try_emplace(id, id, cda.GetDescriptor(), src_lang, panda_file::Type::TypeId::REFERENCE,
                                     GetClassFlags(cda.GetAccessFlags()), pf, entity_id);
    ASSERT(emplace_result.second);
    CachedClass &cachedClass = emplace_result.first->second;

    cachedClass.ancestors.reserve(cda.GetIfacesNumber() + 1);

    AddAncestors(&cachedClass, &cda, data);

    cachedClass.methods.reserve(cda.GetMethodsNumber());
    cachedClass.fields.reserve(cda.GetFieldsNumber());

    cda.EnumerateMethods([&](const panda_file::MethodDataAccessor &mda) {
        if (LIKELY(!pf.IsExternal(mda.GetMethodId()))) {
            auto &cachedMethod = ProcessMethod(&cachedClass, pf, mda);
            cachedClass.methods.insert_or_assign(cachedMethod.hash, cachedMethod);
        }
    });

    cda.EnumerateFields([&](const panda_file::FieldDataAccessor &fda) {
        if (LIKELY(!pf.IsExternal(fda.GetFieldId()))) {
            auto &cachedField = ProcessField(&cachedClass, pf, fda);
            cachedClass.fields.insert_or_assign(cachedField.hash, cachedField);
        }
    });

    data.file_cache.AddToCache(pf, entity_id, cachedClass);

    auto descr_emplace_result = data.descr_lookup.try_emplace(cachedClass.descriptor, std::ref(cachedClass));
    if (descr_emplace_result.second) {
        LOG(DEBUG, VERIFIER) << "CACHE: Added class '" << GetName(cachedClass) << std::hex << "' with id 0x" << id;
    } else {
        // TODO(romanov) Not sure the new definition can't be accessed, which is desirable; to fix this we could
        // add OptionalRef<CachedClass> first_definition to CachedClass, and make sure everything is looked up there
        // first
        LibCache::CachedClass &already_cached_class = descr_emplace_result.first->second.get();
        LogConflictingClassDefinitions(cachedClass, already_cached_class);
    }
}

template <>
OptionalRef<LibCache::CachedClass> FastAPIClassRW::ResolveByDescriptor(panda_file::SourceLang src_lang,
                                                                       const DescriptorString &descriptor,
                                                                       bool reportError)
{
    auto &data = GetContext(src_lang);

    const auto it = data.descr_lookup.find(descriptor);
    if (it != data.descr_lookup.cend()) {
        LOG(DEBUG, VERIFIER) << "CACHE: Descriptor " << descriptor << " resolved for language " << src_lang;
        return it->second.get();
    }

    // check if it is an array descr
    if (ClassHelper::IsArrayDescriptor(descriptor)) {
        return AddArray(src_lang, descriptor);
    }

    reportError
        ? LOG(WARNING, VERIFIER)
        : LOG(ERROR, VERIFIER) << "CACHE: Descriptor " << descriptor << " not resolved for language " << src_lang;
    return {};
}

template <>
OptionalRef<LibCache::CachedClass> FastAPIClassRW::ResolveAndLink(panda_file::SourceLang src_lang,
                                                                  const DescriptorString &descriptor, bool reportError)
{
    auto resolved = ResolveByDescriptor(src_lang, descriptor, reportError);
    if (resolved.HasRef()) {
        if (Link(resolved.AsPointer())) {
            return *resolved;
        }
    }
    return {};
}

template <>
bool FastAPIClassRW::ResolveAndLink(panda_file::SourceLang src_lang,
                                    LibCache::CachedClassRefOrDescriptor *ref_or_descriptor, bool reportError);

template <>
bool FastAPIClassRW::Link(LibCache::CachedClass *cachedClass, bool reportError)
{
    if (IsLinked(*cachedClass)) {
        return true;
    }

    cachedClass->linked = true;
    auto src_lang = cachedClass->source_lang;

    for (auto &ancestor : cachedClass->ancestors) {
        TryResolveAndLinkElement(cachedClass, src_lang, &ancestor, reportError);
    }

    if (cachedClass->flags[CachedClass::Flag::ARRAY_CLASS]) {
        auto &array_component = cachedClass->array_component;
        ASSERT(GetDescriptor(array_component).IsValid());
        TryResolveAndLinkElement(cachedClass, src_lang, &array_component, reportError);
    }

    return IsLinked(*cachedClass);
}

template <>
bool FastAPIClassRW::Link(LibCache::CachedMethod *cachedMethod, bool reportError)
{
    if (IsLinked(*cachedMethod)) {
        return true;
    }

    auto &class_ref = cachedMethod->klass;
    if (!Link(&class_ref)) {
        return false;
    }

    auto src_lang = class_ref.source_lang;

    cachedMethod->linked = true;

    for (CachedClassRefOrDescriptor &arg : cachedMethod->signature) {
        TryResolveAndLinkElement(cachedMethod, src_lang, &arg, reportError);
    }

    for (auto &catch_block : cachedMethod->catch_blocks) {
        auto exc_type = catch_block.exception_type;
        if (IsLinked(exc_type)) {
            continue;
        }
        // the catch-all case, nothing to resolve
        if (!IsValid(exc_type)) {
            continue;
        }
        TryResolveAndLinkElement(cachedMethod, src_lang, &exc_type, reportError);
    }

    return IsLinked(*cachedMethod);
}

template <>
bool FastAPIClassRW::Link(LibCache::CachedField *cachedField, [[maybe_unused]] bool reportError)
{
    if (IsLinked(*cachedField)) {
        return true;
    }

    auto &class_ref = cachedField->klass;
    if (!Link(&class_ref)) {
        return false;
    }

    bool resolved = ResolveAndLink(class_ref.source_lang, &(cachedField->type));
    if (resolved) {
        cachedField->linked = true;
        return true;
    }

    return false;
}

template <>
bool FastAPIClassRW::ResolveAndLink(panda_file::SourceLang src_lang,
                                    LibCache::CachedClassRefOrDescriptor *ref_or_descriptor, bool reportError)
{
    return Visit(
        *ref_or_descriptor, [this](CachedClass &klass) { return Link(&klass); },
        [&](const DescriptorString &descriptor) {
            // logic here is similar to the other ResolveAndLink overload, but we don't combine them due to assigning
            // *ref_or_descriptor even if it can't be linked
            auto resolved = ResolveByDescriptor(src_lang, descriptor, reportError);
            if (resolved.HasRef()) {
                *ref_or_descriptor = *resolved;
                return Link(resolved.AsPointer());
            }
            return false;
        });
}

template <>
template <>
OptionalConstRef<LibCache::CachedClass> FastAPIClassRW::GetFromCache<LibCache::CachedClass>(
    const LibCache::CachedMethod &cachedMethod, panda_file::File::Index idx)
{
    auto &index = cachedMethod.indexes.classes;
    if (idx >= index.size()) {
        LOG(WARNING, VERIFIER) << "Class index out of bounds for cached method " << cachedMethod.GetName() << ", index "
                               << idx << ", size " << index.size();
        return {};
    }

    auto &item = index[idx];

    if (ResolveAndLink(cachedMethod.GetSourceLang(), &item)) {
        return GetRef(item);
    }

    return {};
}

template <>
OptionalRef<LibCache::CachedMethod> FastAPIClassRW::ResolveMethod(const LibCache::CachedMethod &cachedMethod,
                                                                  panda_file::File::EntityId id)
{
    const panda_file::File &file = *cachedMethod.file;
    panda_file::MethodDataAccessor mda {file, id};

    DescriptorString descr {file.GetStringData(mda.GetClassId()).data};

    auto src_lang = cachedMethod.GetSourceLang();

    panda_file::SourceLang method_src_lang = mda.GetSourceLang().value_or(src_lang);

    auto class_ref = ResolveAndLink(method_src_lang, descr);

    if (!class_ref.HasRef()) {
        LOG(WARNING, VERIFIER) << "Failed to resolve or link class by descriptor " << descr << " for language "
                               << method_src_lang;
        return {};
    }

    auto &data = GetContext(src_lang);

    MethodHash method_hash = CalcMethodHash(file, mda);

    auto resolved_method = class_ref->ResolveMethod(method_hash);

    if (!resolved_method.HasRef()) {
        LOG(DEBUG, VERIFIER) << std::hex << "CACHE: cannot resolve method with hash 0x" << method_hash << " in "
                             << class_ref->GetName();
        return {};
    }

    LOG(DEBUG, VERIFIER) << std::hex << "CACHE: method with hash 0x" << method_hash << " in " << class_ref->GetName()
                         << "found: " << resolved_method->GetName();

    if (!Link(resolved_method.AsPointer())) {
        LOG(WARNING, VERIFIER) << "Failed to link " << resolved_method->GetName();
        return {};
    }

    data.file_cache.AddToCache(file, id, *resolved_method);
    return resolved_method;
}

template <>
template <>
OptionalConstRef<LibCache::CachedMethod> FastAPIClassRW::GetFromCache<LibCache::CachedMethod>(
    const LibCache::CachedMethod &cachedMethod, panda_file::File::Index idx)
{
    auto &index = cachedMethod.indexes.methods;
    if (idx >= index.size()) {
        LOG(WARNING, VERIFIER) << "Method index out of bounds for cached method " << cachedMethod.GetName()
                               << ", index " << idx << ", size " << index.size();
        return {};
    }
    auto &item = index[idx];

    using ReturnType = OptionalConstRef<CachedMethod>;
    return Visit(
        item, [](const CachedMethod &already_cached) -> ReturnType { return already_cached; },
        [&](const panda_file::File::EntityId id) -> ReturnType {
            auto src_lang = cachedMethod.GetSourceLang();
            auto &data = GetContext(src_lang);
            auto method_ref = data.file_cache.GetCached<CachedMethod>(*cachedMethod.file, id);

            if (method_ref.HasRef()) {
                if (!Link(method_ref.AsPointer())) {
                    return {};
                }
                item = *method_ref;
                return method_ref;
            }

            auto res = ResolveMethod(cachedMethod, id);
            if (res.HasRef()) {
                item = res.AsWrapper();
            }
            return res;
        });
}

template <>
template <>
OptionalConstRef<LibCache::CachedField> FastAPIClassRW::GetFromCache<LibCache::CachedField>(
    const LibCache::CachedMethod &cachedMethod, panda_file::File::Index idx)
{
    auto &index = cachedMethod.indexes.fields;
    if (idx >= index.size()) {
        LOG(WARNING, VERIFIER) << "Field index out of bounds for cached method " << cachedMethod.GetName() << ", index "
                               << idx << ", size " << index.size();
        return {};
    }
    auto &item = index[idx];

    using ReturnType = OptionalConstRef<CachedField>;
    return Visit(
        item, [](const CachedField &already_cached) -> ReturnType { return already_cached; },
        [&](const panda_file::File::EntityId id) -> ReturnType {
            auto src_lang = cachedMethod.GetSourceLang();
            const panda_file::File &file = *cachedMethod.file;

            auto &data = GetContext(src_lang);

            auto field_ref = data.file_cache.GetCached<CachedField>(file, id);

            if (field_ref.HasRef()) {
                if (!Link(field_ref.AsPointer())) {
                    return {};
                }
                item = field_ref.AsWrapper();
                return field_ref;
            }

            panda_file::FieldDataAccessor fda {file, id};

            DescriptorString descr {file.GetStringData(fda.GetClassId()).data};

            auto class_ref = ResolveAndLink(src_lang, descr);

            if (!class_ref.HasRef()) {
                LOG(WARNING, VERIFIER) << "Failed to resolve or link class by descriptor " << descr << " for language "
                                       << src_lang;
                return {};
            }

            FieldHash field_hash = CalcFieldNameAndTypeHash(file, fda);

            auto resolved_field = class_ref->ResolveField(field_hash);

            if (!resolved_field.HasRef()) {
                LOG(DEBUG, VERIFIER) << std::hex << "CACHE: cannot resolve field with hash 0x" << field_hash << " in "
                                     << class_ref->GetName();
                return {};
            }

            LOG(DEBUG, VERIFIER) << std::hex << "CACHE: field with hash 0x" << field_hash << " in "
                                 << class_ref->GetName() << " found: " << resolved_field->GetName();

            if (!Link(resolved_field.AsPointer())) {
                LOG(WARNING, VERIFIER) << "Failed to link " << resolved_field->GetName();
                return {};
            }

            data.file_cache.AddToCache(file, id, *resolved_field);
            item = resolved_field.AsWrapper();
            return resolved_field;
        });
}

template <>
OptionalConstRef<LibCache::CachedClass> FastAPIClassRW::GetStringClass(const LibCache::CachedMethod &cachedMethod)
{
    panda_file::SourceLang src_lang = cachedMethod.GetSourceLang();
    auto descriptor = GetContext(src_lang).string_descr;
    return ResolveAndLink(src_lang, descriptor);
}

template <>
OptionalConstRef<LibCache::CachedClass> FastAPIClassRW::GetStringArrayClass(panda_file::SourceLang src_lang)
{
    auto descriptor = GetContext(src_lang).string_array_descr;
    return ResolveAndLink(src_lang, descriptor);
}

template <>
void FastAPIClassRW::ProcessFile(const panda_file::File &pf)
{
    auto &processed_files = data_.GetObj().processed_files;
    auto [iter, emplaced] = processed_files.try_emplace(pf.GetUniqId(), &pf);
    if (!emplaced) {
        const std::string &old_path = iter->second->GetFullFileName();
        const std::string &new_path = pf.GetFullFileName();

        if (old_path == new_path) {
            LOG(INFO, VERIFIER) << "CACHE: " << new_path << " is already processed, skipping";
        } else {
            LOG(ERROR, VERIFIER) << "CACHE: Two Panda files at '" << old_path << "' and '" << new_path
                                 << "' have the same filename/header hash, ignoring the second one";
        }

        return;
    }

    LOG(INFO, VERIFIER) << "CACHE: Processing " << pf.GetFullFileName();
    auto classes_indexes = pf.GetClasses();
    for (auto idx : classes_indexes) {
        panda_file::File::EntityId entity_id {idx};
        if (!pf.IsExternal(entity_id)) {
            ProcessClass(pf, entity_id);
        }
    }
}

template <>
void FastAPIClassRW::ProcessFiles(const PandaVector<const panda_file::File *> &pfs)
{
    for (const auto &pf : pfs) {
        ProcessFile(*pf);
    }
}

}  // namespace panda::verifier
