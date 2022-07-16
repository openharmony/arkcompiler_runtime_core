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

#ifndef PANDA_VERIFIER_JOB_QUEUE_CACHE_H_
#define PANDA_VERIFIER_JOB_QUEUE_CACHE_H_

#include "verification/cache/file_entity_cache.h"
#include "verification/util/flags.h"
#include "verification/util/synchronized.h"
#include "verification/util/descriptor_string.h"
#include "verification/util/enum_array.h"
#include "verification/util/optional_ref.h"

#include "runtime/core/core_language_context.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/language_context.h"

#include "libpandafile/file_items.h"
#include "libpandafile/file.h"
#include "libpandafile/method_data_accessor.h"
#include "libpandafile/field_data_accessor.h"

#include "libpandabase/utils/hash.h"

#include <functional>
#include <cstdint>
#include <variant>
#include <optional>

namespace panda::verifier {
class LibCache {
public:
    using Id = uint64_t;

    struct LangContext;

    struct CachedClass;
    struct CachedMethod;
    struct CachedField;

    using CachedClassRef = std::reference_wrapper<CachedClass>;
    using CachedMethodRef = std::reference_wrapper<CachedMethod>;
    using CachedFieldRef = std::reference_wrapper<CachedField>;

    using DescriptorString = panda::verifier::DescriptorString<mode::ExactCmp>;

    using CachedClassRefOrDescriptor = std::variant<CachedClassRef, DescriptorString>;
    using CachedMethodRefOrEntityId = std::variant<CachedMethodRef, panda_file::File::EntityId>;
    using CachedFieldRefOrEntityId = std::variant<CachedFieldRef, panda_file::File::EntityId>;

    /**
     * @tparam CachedEntity CachedClass, CachedMethod, or CachedField
     * @tparam DescriptorOrEntityId DescriptorString or EntityId (depends on CachedEntity)
     * @tparam RefLambda ReturnType(const CachedEntity &)
     * @tparam DescriptorOrEntityIdLambda `ReturnType(const DescriptorString &)` or `ReturnType(EntityId)`
     */
    template <typename CachedEntity, typename DescriptorOrEntityId, typename RefLambda,
              typename DescriptorOrEntityIdLambda>
    static auto Visit(const std::variant<std::reference_wrapper<CachedEntity>, DescriptorOrEntityId> &item,
                      RefLambda ref_lambda, DescriptorOrEntityIdLambda descr_or_id_lambda)
    {
        if (std::holds_alternative<std::reference_wrapper<CachedEntity>>(item)) {
            return ref_lambda(std::get<std::reference_wrapper<CachedEntity>>(item).get());
        } else {
            return descr_or_id_lambda(std::get<DescriptorOrEntityId>(item));
        }
    }

    template <typename CachedEntity, typename DescriptorOrEntityId>
    static bool IsRef(const std::variant<std::reference_wrapper<CachedEntity>, DescriptorOrEntityId> &item)
    {
        return std::holds_alternative<std::reference_wrapper<CachedEntity>>(item);
    }

    template <typename CachedEntity, typename DescriptorOrEntityId>
    static CachedEntity &GetRef(const std::variant<std::reference_wrapper<CachedEntity>, DescriptorOrEntityId> &item)
    {
        ASSERT(IsRef(item));
        return std::get<std::reference_wrapper<CachedEntity>>(item).get();
    }

    // prevent accidental calls where e.g. `const CachedClass &` is implicitly converted to `CachedClassRefOrDescriptor`
    template <typename T>
    static T &GetRef(const T &) = delete;

    static const DescriptorString &GetDescriptor(const CachedClassRefOrDescriptor &item)
    {
        std::reference_wrapper<const DescriptorString> descriptor_ref = Visit(
            item, [](const CachedClass &klass) { return std::cref(klass.descriptor); },
            [](const DescriptorString &descriptor) { return std::cref(descriptor); });
        return descriptor_ref.get();
    }

    inline static const CachedClassRefOrDescriptor InvalidDescriptor = DescriptorString {};
    static bool IsValid(const CachedClassRefOrDescriptor &item)
    {
        return Visit(
            item, []([[maybe_unused]] const CachedClass &klass) { return true; },
            [](const DescriptorString &descriptor) { return descriptor.IsValid(); });
    }

    static PandaString GetName(const CachedClassRefOrDescriptor &item)
    {
        return Visit(
            item, [](const CachedClass &klass) { return GetName(klass); },
            [](const DescriptorString &descriptor) { return GetName(descriptor); });
    }

    template <typename CachedEntity>
    static bool IsLinked(std::reference_wrapper<CachedEntity> item)
    {
        return item.get().linked;
    }

    template <typename CachedEntity, typename DescriptorOrEntityId>
    static bool IsLinked(const std::variant<std::reference_wrapper<CachedEntity>, DescriptorOrEntityId> &item)
    {
        return IsRef(item) && IsLinked(GetRef(item));
    }

    template <typename CachedEntity>
    static bool IsLinked(const CachedEntity &item)
    {
        return item.linked;
    }

    using MethodHash = uint64_t;
    using FieldHash = uint64_t;

    using PrimitiveClassesArray =
        EnumArray<OptionalRef<CachedClass>, panda_file::Type::TypeId, panda_file::Type::TypeId::VOID,
                  panda_file::Type::TypeId::U1, panda_file::Type::TypeId::U8, panda_file::Type::TypeId::I8,
                  panda_file::Type::TypeId::U16, panda_file::Type::TypeId::I16, panda_file::Type::TypeId::U32,
                  panda_file::Type::TypeId::I32, panda_file::Type::TypeId::U64, panda_file::Type::TypeId::I64,
                  panda_file::Type::TypeId::F32, panda_file::Type::TypeId::F64, panda_file::Type::TypeId::TAGGED>;

    static PandaString GetName(const CachedClass &);
    static PandaString GetName(const CachedMethod &);
    static PandaString GetName(const CachedField &);
    static PandaString GetName(const DescriptorString &);

    template <typename T>
    static PandaString GetName(OptionalConstRef<T> item)
    {
        if (item.HasRef()) {
            return GetName(item.Get());
        }

        return "NOT_IN_CACHE";
    }

    struct CachedClass {
        enum class Flag {
            DYNAMIC_CLASS,
            PUBLIC,
            FINAL,
            ANNOTATION,
            ENUM,
            ARRAY_CLASS,
            OBJECT_ARRAY_CLASS,
            STRING_CLASS,
            VARIABLESIZE,
            PRIMITIVE,
            ABSTRACT,
            INTERFACE,
            INSTANTIABLE,
            OBJECT_CLASS,
            CLASS_CLASS,
            PROXY,
            SUPER,
            SYNTHETIC
        };
        using FlagsValue =
            FlagsForEnum<unsigned int, Flag, Flag::DYNAMIC_CLASS, Flag::PUBLIC, Flag::FINAL, Flag::ANNOTATION,
                         Flag::ENUM, Flag::ARRAY_CLASS, Flag::OBJECT_ARRAY_CLASS, Flag::STRING_CLASS,
                         Flag::VARIABLESIZE, Flag::PRIMITIVE, Flag::ABSTRACT, Flag::INTERFACE, Flag::INSTANTIABLE,
                         Flag::OBJECT_CLASS, Flag::CLASS_CLASS, Flag::PROXY, Flag::SUPER, Flag::SYNTHETIC>;
        Id id;
        DescriptorString descriptor;
        panda_file::SourceLang source_lang;
        panda_file::Type::TypeId type_id;
        PandaVector<CachedClassRefOrDescriptor> ancestors;
        // special case: invalid descriptor indicates this class is not an array
        CachedClassRefOrDescriptor array_component;
        FlagsValue flags;
        PandaUnorderedMap<MethodHash, CachedMethodRef> methods;
        PandaUnorderedMap<FieldHash, CachedFieldRef> fields;
        bool linked;
        OptionalConstRef<panda_file::File> file;
        panda_file::File::EntityId file_id;

        CachedClass(Id id_, DescriptorString descriptor_, panda_file::SourceLang source_lang_,
                    panda_file::Type::TypeId type_id_, FlagsValue flags_, OptionalConstRef<panda_file::File> file_,
                    panda_file::File::EntityId file_id_)
            : id {id_},
              descriptor {descriptor_},
              source_lang {source_lang_},
              type_id {type_id_},
              ancestors {},
              array_component {InvalidDescriptor},
              flags {flags_},
              methods {},
              fields {},
              linked {false},
              file {file_},
              file_id {file_id_}
        {
        }

        // should only be accessed from inside the cache, so there won't be inconsistent copies
        NO_COPY_SEMANTIC(CachedClass);

        PandaString GetName() const
        {
            return LibCache::GetName(*this);
        }

        OptionalConstRef<CachedClass> GetArrayComponent() const
        {
            return Visit(
                array_component, [](const CachedClass &klass) { return OptionalConstRef {klass}; },
                []([[maybe_unused]] const DescriptorString &component_descriptor) {
                    ASSERT(!component_descriptor.IsValid() &&
                           "GetArrayComponent should never be called with unresolved descriptor");
                    return OptionalConstRef<CachedClass> {};
                });
        }

        bool IsSamePackage(const CachedClass &that_class) const
        {
            // get and compare packet names from the fully qualified class names
            auto this_name = GetName();
            auto that_name = that_class.GetName();
            auto this_name_packet_length = this_name.find_last_of('.');
            auto that_name_packet_length = that_name.find_last_of('.');
            if (this_name_packet_length == std::string::npos) {
                return that_name_packet_length == std::string::npos;
            }
            if (this_name_packet_length != that_name_packet_length) {
                return false;
            }
            return this_name.compare(0, this_name_packet_length, that_name, 0, this_name_packet_length) == 0;
        }

        OptionalRef<CachedMethod> ResolveMethod(MethodHash);
        OptionalRef<CachedField> ResolveField(FieldHash);
    };

    struct CachedCatchBlock {
        const uint8_t *try_block_start;
        const uint8_t *try_block_end;
        // special case: invalid descriptor indicates catch_all section
        CachedClassRefOrDescriptor exception_type;
        const uint8_t *handler_bytecode;
        size_t handler_bytecode_size;
    };

    struct Indexes {
        PandaVector<CachedClassRefOrDescriptor> classes;
        PandaVector<CachedMethodRefOrEntityId> methods;
        PandaVector<CachedFieldRefOrEntityId> fields;
    };

    struct CachedMethod {
        enum class Flag {
            STATIC,
            PUBLIC,
            PRIVATE,
            PROTECTED,
            NATIVE,
            INTRINSIC,
            SYNTHETIC,
            ABSTRACT,
            FINAL,
            SYNCHRONIZED,
            HAS_SINGLE_IMPLEMENTATION,
            DEFAULT_INTERFACE_METHOD,
            CONSTRUCTOR,
            INSTANCE_CONSTRUCTOR,
            STATIC_CONSTRUCTOR,
            ARRAY_CONSTRUCTOR
        };
        using FlagsValue =
            FlagsForEnum<unsigned int, Flag, Flag::STATIC, Flag::PUBLIC, Flag::PRIVATE, Flag::PROTECTED, Flag::NATIVE,
                         Flag::INTRINSIC, Flag::SYNTHETIC, Flag::ABSTRACT, Flag::FINAL, Flag::SYNCHRONIZED,
                         Flag::HAS_SINGLE_IMPLEMENTATION, Flag::DEFAULT_INTERFACE_METHOD, Flag::CONSTRUCTOR,
                         Flag::INSTANCE_CONSTRUCTOR, Flag::STATIC_CONSTRUCTOR, Flag::ARRAY_CONSTRUCTOR>;
        Id id;
        MethodHash hash;
        DescriptorString name;
        CachedClass &klass;
        PandaVector<CachedClassRefOrDescriptor> signature;
        PandaVector<CachedCatchBlock> catch_blocks;
        Indexes &indexes;
        size_t num_vregs;
        size_t num_args;
        FlagsValue flags;
        const uint8_t *bytecode;
        size_t bytecode_size;
        bool linked;
        /*
        Keep here extended verification result in debug mode:
        in case of verification problems, save bitmap of instructions
        that were successfully verified with contexts on beginnings of
        unverified blocks to debug them
        */
        OptionalConstRef<panda_file::File> file;
        panda_file::File::EntityId file_id;

        CachedMethod(Id id_, MethodHash hash_, DescriptorString name_, CachedClass &klass_, Indexes &indexes_,
                     FlagsValue flags_, OptionalConstRef<panda_file::File> file_, panda_file::File::EntityId file_id_)
            : id {id_},
              hash {hash_},
              name {name_},
              klass {klass_},
              signature {},
              catch_blocks {},
              indexes {indexes_},
              num_vregs {0},
              num_args {0},
              flags {flags_},
              bytecode {nullptr},
              bytecode_size {0},
              linked {false},
              file {file_},
              file_id {file_id_}
        {
        }

        NO_COPY_SEMANTIC(CachedMethod);
        CachedMethod(CachedMethod &&) = default;
        CachedMethod &operator=(CachedMethod &&) = default;

        PandaString GetName() const
        {
            return LibCache::GetName(*this);
        }

        bool IsStatic() const
        {
            return flags[Flag::STATIC];
        }

        panda_file::SourceLang GetSourceLang() const
        {
            return klass.source_lang;
        }
    };

    struct CachedField {
        enum class Flag { PUBLIC, PRIVATE, PROTECTED, STATIC, VOLATILE, FINAL };
        using FlagsValue = FlagsForEnum<unsigned int, Flag, Flag::PUBLIC, Flag::PRIVATE, Flag::PROTECTED, Flag::STATIC,
                                        Flag::VOLATILE, Flag::FINAL>;
        Id id;
        FieldHash hash;
        DescriptorString name;
        CachedClass &klass;
        CachedClassRefOrDescriptor type;
        FlagsValue flags;
        bool linked;
        const panda_file::File &file;
        panda_file::File::EntityId file_id;

        CachedField(Id id_, DescriptorString name_, CachedClass &klass_, FlagsValue flags_,
                    const panda_file::File &file_, panda_file::File::EntityId file_id_)
            : id {id_},
              hash {},
              name {name_},
              klass {klass_},
              type {InvalidDescriptor},
              flags {flags_},
              linked {false},
              file {file_},
              file_id {file_id_}
        {
        }

        PandaString GetName() const
        {
            return LibCache::GetName(*this);
        }

        const CachedClass &GetType() const
        {
            ASSERT(IsRef(type));
            return GetRef(type);
        }

        NO_COPY_SEMANTIC(CachedField);
    };

    LibCache();
    LibCache(const LibCache &) = delete;
    LibCache(LibCache &&) = delete;
    LibCache &operator=(const LibCache &) = delete;
    LibCache &operator=(LibCache &&) = delete;
    ~LibCache() = default;

    using ClassCache = PandaUnorderedMap<Id, CachedClass>;
    using MethodCache = PandaUnorderedMap<Id, CachedMethod>;
    using FieldCache = PandaUnorderedMap<Id, CachedField>;
    using DescriptorLookup = PandaUnorderedMap<DescriptorString, CachedClassRef>;
    using FileCache = FileEntityCache<CachedClass, CachedMethod, CachedField>;

    struct LangContext {
        ClassCache class_cache;
        MethodCache method_cache;
        FieldCache field_cache;
        PrimitiveClassesArray primitive_classes;
        DescriptorLookup descr_lookup;
        FileCache file_cache;
        PandaUnorderedMap<const panda_file::File::IndexHeader *, Indexes> indexes_cache;
        DescriptorString string_descr;
        DescriptorString object_descr;
        DescriptorString string_array_descr;

        CachedClass &GetPrimitiveClass(panda_file::Type::TypeId type_id)
        {
            return primitive_classes[type_id].Get();
        }

        CachedClass &GetPrimitiveClass(panda_file::Type type)
        {
            return GetPrimitiveClass(type.GetId());
        }
    };

    struct Data {
        EnumArray<LangContext, panda_file::SourceLang, LANG_ENUM_LIST> contexts;
        PandaUnorderedMap<uint32_t, const panda_file::File *> processed_files;
    };

    enum class Access { READ_ONLY, READ_WRITE };

    template <Access Mode>
    class FastAPIClass;

    using SyncData = Synchronized<Data, FastAPIClass<Access::READ_ONLY>, FastAPIClass<Access::READ_WRITE>>;

    template <Access Mode>
    class FastAPIClass {
        SyncData &data_;
        explicit FastAPIClass(SyncData &data) : data_ {data}
        {
            if constexpr (Mode == Access::READ_ONLY) {
                data_.ReadLock();
            } else {
                data_.WriteLock();
            }
        }

        friend class LibCache;

    public:
        ~FastAPIClass()
        {
            data_.Unlock();
        }

        const LangContext &GetContext(panda_file::SourceLang src_lang) const
        {
            return data_.GetObj().contexts[src_lang];
        }
        LangContext &GetContext(panda_file::SourceLang src_lang)
        {
            return data_.GetObj().contexts[src_lang];
        }

        // signatures for convenience, see job_fill_gen.h.erb
        OptionalConstRef<CachedClass> GetStringClass(const CachedMethod &cachedMethod);
        OptionalConstRef<CachedClass> GetStringArrayClass(panda_file::SourceLang src_lang);

        void ProcessFile(const panda_file::File &pf);

        // TODO(romanov) convenience API used as ProcessFiles(class_linker.GetBootPandaFiles()), may replace argument
        // e.g. with `const PandaVector<panda_file::File> &` when ClassLinker dependency is removed
        void ProcessFiles(const PandaVector<const panda_file::File *> &pfs);

        OptionalRef<CachedMethod> GetMethod(panda_file::SourceLang src_lang, Id id, bool reportError = false);

        template <typename CachedEntity>
        OptionalConstRef<CachedEntity> GetFromCache(const CachedMethod &cachedMethod, panda_file::File::Index idx);

        OptionalRef<CachedClass> ResolveAndLink(panda_file::SourceLang src_lang, const DescriptorString &descriptor,
                                                bool reportError = false);

    private:
        CachedClass &MakeSyntheticClass(panda_file::SourceLang src_lang, const uint8_t *descriptor,
                                        panda_file::Type::TypeId type_id, uint32_t flags);

        /**
         * @tparam SignatureFiller  void(CachedClass &, CachedMethod &)
         */
        template <typename SignatureFiller>
        CachedMethod &MakeSyntheticMethod(CachedClass &cachedClass, const uint8_t *name, bool is_static,
                                          SignatureFiller sig_filler);

        CachedClass &AddArray(panda_file::SourceLang src_lang, const uint8_t *descr);

        void InitializeRootClasses(panda_file::SourceLang src_lang);

        bool Link(CachedClass *cachedClass, bool reportError = false);
        bool Link(CachedMethod *cachedMethod, bool reportError = false);
        bool Link(CachedField *cachedField, bool reportError = false);

        template <typename CachedEntity>
        OptionalRef<CachedEntity> GetFromCacheAndLink(PandaUnorderedMap<Id, CachedEntity> *cache, Id id,
                                                      bool reportError = false)
        {
            auto it = cache->find(id);
            if (it != cache->end() && Link(&(it->second), reportError)) {
                return it->second;
            }
            return {};
        }

        OptionalRef<CachedClass> ResolveByDescriptor(panda_file::SourceLang src_lang,
                                                     const DescriptorString &descriptor, bool reportError = false);

        bool ResolveAndLink(panda_file::SourceLang src_lang, CachedClassRefOrDescriptor *ref_or_descriptor,
                            bool reportError = false);

        void ProcessClass(const panda_file::File &pf, panda_file::File::EntityId entity_id);
        CachedMethod &ProcessMethod(CachedClass *cachedClass, const panda_file::File &pf,
                                    const panda_file::MethodDataAccessor &mda);
        CachedField &ProcessField(CachedClass *cachedClass, const panda_file::File &pf,
                                  const panda_file::FieldDataAccessor &fda);

        template <typename CachedEntity>
        void TryResolveAndLinkElement(CachedEntity *entity, panda_file::SourceLang src_lang,
                                      LibCache::CachedClassRefOrDescriptor *ref_or_descriptor, bool reportError = false)
        {
            if (!ResolveAndLink(src_lang, ref_or_descriptor, reportError)) {
                entity->linked = false;
            }
        }

        OptionalRef<CachedMethod> ResolveMethod(const CachedMethod &, panda_file::File::EntityId id);
    };

    FastAPIClass<Access::READ_WRITE> FastAPI()
    {
        return {data_};
    }

    const FastAPIClass<Access::READ_ONLY> FastAPI() const
    {
        return {const_cast<SyncData &>(data_)};
    }

private:
    SyncData data_;
};

using CachedClass = LibCache::CachedClass;
using CachedMethod = LibCache::CachedMethod;
using CachedField = LibCache::CachedField;

}  // namespace panda::verifier

#endif  // !PANDA_VERIFIER_JOB_QUEUE_CACHE_H_
