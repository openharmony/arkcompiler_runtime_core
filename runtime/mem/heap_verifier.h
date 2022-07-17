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
#ifndef PANDA_MEM_HEAP_VERIFIER_H
#define PANDA_MEM_HEAP_VERIFIER_H

#include "libpandabase/utils/bit_utils.h"
#include "libpandabase/utils/logger.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/rendezvous.h"

namespace panda::mem {

class HeapManager;
class GCRoot;

/**
 * HeapReferenceVerifier checks reference checks if the referent is with the heap and it is live.
 */
template <LangTypeT LangType = LANG_TYPE_STATIC>
class HeapReferenceVerifier {
public:
    explicit HeapReferenceVerifier(HeapManager *heap, size_t *count) : HEAP(heap), FAIL_COUNT(count) {}

    void operator()(ObjectHeader *object_header, ObjectHeader *referent);

    void operator()(const GCRoot &root);

private:
    HeapManager *const HEAP {nullptr};
    size_t *const FAIL_COUNT {nullptr};
};

/**
 * HeapObjectVerifier iterates over HeapManager's allocated objects. If an object contains reference, it checks if the
 * referent is with the heap and it is live.
 */
template <LangTypeT LangType = LANG_TYPE_STATIC>
class HeapObjectVerifier {
public:
    HeapObjectVerifier() = delete;

    HeapObjectVerifier(HeapManager *heap, size_t *count) : HEAP(heap), FAIL_COUNT(count) {}

    void operator()(ObjectHeader *obj);

    size_t GetFailCount() const
    {
        return *FAIL_COUNT;
    }

private:
    HeapManager *const HEAP {nullptr};
    size_t *const FAIL_COUNT {nullptr};
};

class HeapVerifierBase {
public:
    explicit HeapVerifierBase(HeapManager *heap) : heap_(heap) {}
    DEFAULT_COPY_SEMANTIC(HeapVerifierBase);
    DEFAULT_MOVE_SEMANTIC(HeapVerifierBase);
    ~HeapVerifierBase() = default;

protected:
    HeapManager *heap_ = nullptr;  // NOLINT(misc-non-private-member-variables-in-classes)
};

/**
 * A class to query address validity.
 */
template <class LanguageConfig>
class HeapVerifier : public HeapVerifierBase {
public:
    explicit HeapVerifier(HeapManager *heap) : HeapVerifierBase(heap) {}
    DEFAULT_COPY_SEMANTIC(HeapVerifier);
    DEFAULT_MOVE_SEMANTIC(HeapVerifier);
    ~HeapVerifier() = default;

    bool IsValidObjectAddress(void *addr) const;

    bool IsHeapAddress(void *addr) const;

    size_t VerifyRoot() const;

    size_t VerifyHeap() const;

    size_t VerifyAll() const
    {
        return VerifyRoot() + VerifyHeap();
    }

    size_t VerifyAllPaused() const;
};

/**
 * Fast heap verifier for check heap and stack
 */
template <class LanguageConfig>
class FastHeapVerifier : public HeapVerifierBase {
public:
    explicit FastHeapVerifier(HeapManager *heap) : HeapVerifierBase(heap) {}
    DEFAULT_COPY_SEMANTIC(FastHeapVerifier);
    DEFAULT_MOVE_SEMANTIC(FastHeapVerifier);
    ~FastHeapVerifier() = default;

    size_t VerifyAll() const;

private:
    struct ObjectCache {
        const ObjectHeader *heap_object;
        const ObjectHeader *referent;
    };
};

class ObjectVerificationInfo {
public:
    explicit ObjectVerificationInfo(ObjectHeader *referent);
    DEFAULT_COPY_SEMANTIC(ObjectVerificationInfo);
    DEFAULT_NOEXCEPT_MOVE_SEMANTIC(ObjectVerificationInfo);
    ~ObjectVerificationInfo() = default;

    /**
     * Check that updated reference \p updated_ref from \p object has same state as before
     * collecting and refs updating
     *
     * @param object the object which contains verifying reference
     * @param updated_ref new referent position after moving
     * @param in_alive_space whether the object places in space which was not moved (region promotion, for example)
     * @return true if reference is correct and false otherwise
     */
    bool VerifyUpdatedRef(ObjectHeader *object, ObjectHeader *updated_ref, bool in_alive_space) const;

private:
    void *class_address_ = nullptr;
    ObjectHeader *old_address_ = nullptr;
};

template <class LanguageConfig>
class HeapVerifierIntoGC : public HeapVerifierBase {
public:
    explicit HeapVerifierIntoGC(HeapManager *heap) : HeapVerifierBase(heap) {}
    DEFAULT_COPY_SEMANTIC(HeapVerifierIntoGC);
    DEFAULT_MOVE_SEMANTIC(HeapVerifierIntoGC);
    ~HeapVerifierIntoGC() = default;

    void CollectVerificationInfo(PandaVector<MemRange> &&collectable_mem_ranges);

    size_t VerifyAll(PandaVector<MemRange> &&alive_mem_ranges = PandaVector<MemRange>());

private:
    using VerifyingRefs = PandaUnorderedMap<size_t, ObjectVerificationInfo>;
    using RefsVerificationInfo = PandaUnorderedMap<ObjectHeader *, VerifyingRefs>;

    bool InCollectableSpace(const ObjectHeader *object) const;
    bool InAliveSpace(const ObjectHeader *object) const;
    void AddToVerificationInfo(RefsVerificationInfo &verification_info, size_t ref_number, ObjectHeader *object_header,
                               ObjectHeader *referent);

    RefsVerificationInfo collectable_verification_info_;
    RefsVerificationInfo permanent_verification_info_;
    PandaVector<MemRange> collectable_mem_ranges_;
    PandaVector<MemRange> alive_mem_ranges_;
};
}  // namespace panda::mem

#endif  // PANDA_MEM_HEAP_VERIFIER_H
