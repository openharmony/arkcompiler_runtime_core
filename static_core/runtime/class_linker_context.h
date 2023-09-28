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
#ifndef PANDA_RUNTIME_CLASS_LINKER_CONTEXT_H_
#define PANDA_RUNTIME_CLASS_LINKER_CONTEXT_H_

#include "libpandabase/macros.h"
#include "libpandabase/mem/object_pointer.h"
#include "libpandabase/os/mutex.h"
#include "libpandabase/utils/bit_utils.h"
#include "runtime/include/class.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_root.h"
#include "runtime/mem/object_helpers.h"

namespace panda {

class ClassLinker;
class ClassLinkerErrorHandler;

class ClassLinkerContext {
public:
    explicit ClassLinkerContext(panda_file::SourceLang lang) : lang_(lang) {}
    Class *FindClass(const uint8_t *descriptor)
    {
        os::memory::LockHolder lock(classes_lock_);
        auto it = loaded_classes_.find(descriptor);
        if (it != loaded_classes_.cend()) {
            return it->second;
        }

        return nullptr;
    }

    virtual bool IsBootContext() const
    {
        return false;
    }

    panda_file::SourceLang GetSourceLang()
    {
        return lang_;
    }

    virtual Class *LoadClass([[maybe_unused]] const uint8_t *descriptor, [[maybe_unused]] bool need_copy_descriptor,
                             [[maybe_unused]] ClassLinkerErrorHandler *error_handler)
    {
        return nullptr;
    }

    Class *InsertClass(Class *klass)
    {
        os::memory::LockHolder lock(classes_lock_);
        auto *other_klass = FindClass(klass->GetDescriptor());
        if (other_klass != nullptr) {
            return other_klass;
        }

        ASSERT(klass->GetSourceLang() == lang_);
        loaded_classes_.insert({klass->GetDescriptor(), klass});
        return nullptr;
    }

    void RemoveClass(Class *klass)
    {
        os::memory::LockHolder lock(classes_lock_);
        loaded_classes_.erase(klass->GetDescriptor());
    }

    template <class Callback>
    bool EnumerateClasses(const Callback &cb)
    {
        os::memory::LockHolder lock(classes_lock_);
        for (const auto &v : loaded_classes_) {
            if (!cb(v.second)) {
                return false;
            }
        }
        return true;
    }

    virtual void EnumeratePandaFiles(const std::function<bool(const panda_file::File &)> & /* cb */) const {}

    size_t NumLoadedClasses()
    {
        os::memory::LockHolder lock(classes_lock_);
        return loaded_classes_.size();
    }

    void VisitLoadedClasses(size_t flag)
    {
        os::memory::LockHolder lock(classes_lock_);
        for (auto &loaded_class : loaded_classes_) {
            auto class_ptr = loaded_class.second;
            class_ptr->DumpClass(GET_LOG_STREAM(ERROR, RUNTIME), flag);
        }
    }

    void VisitGCRoots(const ObjectVisitor &cb)
    {
        for (auto root : roots_) {
            cb(root);
        }
    }

    bool AddGCRoot(ObjectHeader *obj)
    {
        os::memory::LockHolder lock(classes_lock_);
        for (auto root : roots_) {
            if (root == obj) {
                return false;
            }
        }

        roots_.emplace_back(obj);
        return true;
    }

    void UpdateGCRoots()
    {
        for (auto &root : roots_) {
            if (root->IsForwarded()) {
                root = ::panda::mem::GetForwardAddress(root);
            }
        }
    }

    virtual PandaVector<std::string_view> GetPandaFilePaths() const
    {
        return PandaVector<std::string_view>();
    }

    virtual void Dump(std::ostream &os)
    {
        os << "|Class loader :\"" << this << "\" "
           << "|Loaded Classes:" << NumLoadedClasses() << "\n";
    }

    virtual bool FindClassLoaderParent([[maybe_unused]] ClassLinkerContext *parent)
    {
        parent = nullptr;
        return false;
    }

    ClassLinkerContext() = default;
    virtual ~ClassLinkerContext() = default;

    NO_COPY_SEMANTIC(ClassLinkerContext);
    NO_MOVE_SEMANTIC(ClassLinkerContext);

private:
    // Dummy fix of concurrency issues to evaluate degradation
    os::memory::RecursiveMutex classes_lock_;
    PandaUnorderedMap<const uint8_t *, Class *, utf::Mutf8Hash, utf::Mutf8Equal> loaded_classes_
        GUARDED_BY(classes_lock_);
    PandaVector<ObjectPointer<ObjectHeader>> roots_;
    panda_file::SourceLang lang_ {panda_file::SourceLang::PANDA_ASSEMBLY};
};

}  // namespace panda

#endif  // PANDA_RUNTIME_CLASS_LINKER_CONTEXT_H_
