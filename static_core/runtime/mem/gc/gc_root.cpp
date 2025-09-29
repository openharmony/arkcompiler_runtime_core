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

#include "runtime/mem/gc/gc_root.h"
#include <atomic>

#include "libarkfile/panda_cache.h"
#include "runtime/include/object_header.h"
#include "runtime/include/runtime.h"
#include "runtime/include/stack_walker-inl.h"
#include "runtime/mem/refstorage/reference_storage.h"
#include "runtime/mem/gc/card_table-inl.h"
#include "runtime/mem/gc/gc.h"
#include "runtime/mem/gc/gc_root_type.h"
#include "runtime/mem/object_helpers-inl.h"
#include "runtime/mem/refstorage/global_object_storage.h"
#include "runtime/include/panda_vm.h"

namespace ark::mem {

std::ostream &operator<<(std::ostream &os, const GCRoot &root)
{
    switch (root.GetType()) {
        case RootType::ROOT_CLASS:
            os << "ROOT CLASS";
            break;
        case RootType::ROOT_FRAME:
            os << "ROOT FRAME";
            break;
        case RootType::ROOT_THREAD:
            os << "ROOT THREAD";
            break;
        case RootType::ROOT_TENURED:
            os << "ROOT TENURED";
            break;
        case RootType::ROOT_NATIVE_GLOBAL:
            os << "ROOT NATIVE_GLOBAL";
            break;
        case RootType::ROOT_NATIVE_LOCAL:
            os << "ROOT NATIVE_LOCAL";
            break;
        case RootType::ROOT_AOT_STRING_SLOT:
            os << "ROOT AOT_STRING_SLOT";
            break;
        case RootType::ROOT_UNKNOWN:
        default:
            LOG(FATAL, GC) << "ROOT UNKNOWN";
            break;
    }
    os << std::hex << " " << root.GetObjectHeader() << std::endl;
    return os;
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitNonHeapRoots(const GCRootVisitor &gcRootVisitor, VisitGCRootFlags flags)
{
    VisitLocalRoots(gcRootVisitor);
    VisitClassRoots(gcRootVisitor, flags);
    VisitAotStringRoots(gcRootVisitor, flags);
    VisitClassLinkerContextRoots(gcRootVisitor);
    VisitVmRoots(gcRootVisitor);
    auto *storage = vm_->GetGlobalObjectStorage();
    if (storage != nullptr) {
        storage->VisitObjects(gcRootVisitor, mem::RootType::ROOT_NATIVE_GLOBAL);
    }
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitCardTableRoots(CardTable *cardTable, ObjectAllocatorBase *allocator,
                                                      GCRootVisitor rootVisitor, MemRangeChecker rangeChecker,
                                                      ObjectChecker rangeObjectChecker, ObjectChecker fromObjectChecker,
                                                      uint32_t processedFlag) const
{
    cardTable->VisitMarked(
        [&allocator, &rootVisitor, &rangeChecker, &rangeObjectChecker, &fromObjectChecker,
         &cardTable](MemRange memRange) {
            if (rangeChecker(memRange)) {
                auto objectsInRangeVisitor = [&rootVisitor, &rangeObjectChecker,
                                              &fromObjectChecker](ObjectHeader *objectHeader) {
                    auto traverseObjectInRange = [&rootVisitor, &rangeObjectChecker](ObjectHeader *fromObject,
                                                                                     ObjectHeader *objectToTraverse,
                                                                                     [[maybe_unused]] uint32_t offset,
                                                                                     [[maybe_unused]] bool isVolatile) {
                        if (!rangeObjectChecker(objectToTraverse))
                            return false;
                        // The weak references from dynobjects should not be regarded as roots.
                        TaggedValue value(objectToTraverse);
                        if (value.IsWeak())
                            return false;
                        // In concurrent phase some other thread can overide the field of fromObject with offset
                        // This way fromObject->GetFieldObject(offset) can be different from objectToTraverse
                        // For that reason we use atomic::compare_exchange_weak
                        ObjectPointerType ref = ToUintPtr(objectToTraverse);
                        ASSERT(ref == ToUintPtr(objectToTraverse));
                        rootVisitor(GCRoot(RootType::ROOT_TENURED, fromObject, &ref));
                        // Atomic with relaxed order reason: ordering constraints are not required
                        if (ref != ToUintPtr(objectToTraverse)) {
                            reinterpret_cast<std::atomic<ObjectHeader *> *>(ToUintPtr(fromObject) + offset)
                                ->compare_exchange_weak(objectToTraverse, reinterpret_cast<ObjectHeader *>(ref),
                                                        std::memory_order_relaxed);
                        }
                        return false;
                    };
                    if (objectHeader->ClassAddr<BaseClass>() != nullptr && fromObjectChecker(objectHeader)) {
                        // The class may be null in the situation when a new object is allocated in the card
                        // we are visiting now, but the class is not set yet.
                        ObjectHelpers<LanguageConfig::LANG_TYPE>::template TraverseAllObjectsWithInfo<false>(
                            objectHeader, traverseObjectInRange);
                    }
                };
                allocator->IterateOverObjectsInRange(memRange, objectsInRangeVisitor);
            } else {
                auto *card = cardTable->GetCardPtr(memRange.GetStartAddress());
                // Use SetCard instead of MarkCard because the card can be IsProcessed
                card->SetCard(CardTable::Card::GetMarkedValue());
            }
        },
        processedFlag);
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitRootsForThread(ManagedThread *thread, const GCRootVisitor &gcRootVisitor) const
{
    LOG(DEBUG, GC) << "Start collecting roots for thread " << thread->GetId();

    thread->VisitGCRoots([&gcRootVisitor](GCRoot root) {
        LOG(DEBUG, GC) << " Found root for thread" << GetDebugInfoAboutObject(root.GetObjectHeader());
        gcRootVisitor(root);
    });
    LOG(DEBUG, GC) << "Finish collecting roots for thread " << thread->GetId();
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitLocalRoots(const GCRootVisitor &gcRootVisitor)
{
    auto threadVisitor = [this, &gcRootVisitor](ManagedThread *thread) {
        VisitRootsForThread(thread, gcRootVisitor);
        for (auto stack = StackWalker::Create(thread); stack.HasFrame(); stack.NextFrame()) {
            auto currentMethod = stack.GetMethod();
            if (currentMethod->IsCriticalNative()) {
                LOG(FATAL, GC) << "GC cannot be triggered in CriticalNative method. Method name: "
                               << currentMethod->GetFullName(true);
            }
            LOG(DEBUG, GC) << " VisitRoots frame " << std::hex << stack.GetFp();
            stack.IterateObjectsWithInfo([this, &gcRootVisitor, &stack](auto &reginfo, auto &vreg) {
                this->VisitRegisterRoot(vreg, reginfo, stack, gcRootVisitor);
                return true;
            });
        }
        return true;
    };
    vm_->GetThreadManager()->EnumerateThreads(threadVisitor);
}

template <class LanguageConfig>
template <class VRegRef, class VRegInfo>
void RootManager<LanguageConfig>::VisitRegisterRoot(VRegRef &vRegister, VRegInfo &regInfo, StackWalker &pframe,
                                                    const GCRootVisitor &gcRootVisitor)
{
    if (LIKELY(!vRegister.HasObject()))
        return;
    ObjectHeader *objectHeader = vRegister.GetReference();
    ObjectHeader *oldRef = objectHeader;
    if (objectHeader == nullptr)
        return;
    LOG(DEBUG, GC) << " Found root for register" << GetDebugInfoAboutObject(objectHeader);

    gcRootVisitor({RootType::ROOT_FRAME, &objectHeader});

    if (oldRef != objectHeader) {
        if (!pframe.IsCFrame() && regInfo.IsAccumulator()) {
            LOG(DEBUG, GC) << "^ acc updated";
            vRegister.template SetReference<false>(objectHeader);
        } else {
            pframe.template SetVRegValue<std::is_same_v<decltype(vRegister), interpreter::DynamicVRegisterRef &>>(
                regInfo, objectHeader);
        }
    }
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitVmRoots(const GCRootVisitor &gcRootVisitor) const
{
    vm_->VisitVmRoots(gcRootVisitor);
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitAotStringRoots(const GCRootVisitor &gcRootVisitor, VisitGCRootFlags flags) const
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    LOG(DEBUG, GC) << "Start collecting AOT string slot roots";
    Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->VisitAotStringRoots(
        [&gcRootVisitor](ObjectHeader **slot) {
            gcRootVisitor({RootType::ROOT_AOT_STRING_SLOT, slot});
        },
        (flags & VisitGCRootFlags::ACCESS_ROOT_AOT_STRINGS_ONLY_YOUNG) != 0);
    LOG(DEBUG, GC) << "Finish collecting AOT string slot roots";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateAotStringRoots(const GCRootUpdater &gcRootUpdater)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    LOG(DEBUG, GC) << "=== AOT string slot roots update. BEGIN ===";
    auto hm = vm_->GetHeapManager();
    Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->UpdateAotStringRoots(
        [&gcRootUpdater](ObjectHeader **root) { gcRootUpdater(root); },
        [&hm](const ObjectHeader *root) { return hm->IsObjectInYoungSpace(root); });
    LOG(DEBUG, GC) << "=== AOT string slot roots update. END ===";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateVmRefs(const GCRootUpdater &gcRootUpdater)
{
    vm_->UpdateVmRefs(gcRootUpdater);
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateGlobalObjectStorage(const GCRootUpdater &gcRootUpdater)
{
    auto globalStorage = vm_->GetGlobalObjectStorage();
    if (globalStorage != nullptr) {
        globalStorage->UpdateMovedRefs(gcRootUpdater);
    }
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitClassRoots(const GCRootVisitor &gcRootVisitor, VisitGCRootFlags flags) const
{
    LOG(DEBUG, GC) << "Start collecting roots for classes";
    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto classRootVisitor = [&gcRootVisitor](Class *cls) {
        gcRootVisitor(cls->GetGCRoot());
        LOG(DEBUG, GC) << " Found class root " << GetDebugInfoAboutObject(cls->GetManagedObject());
        return true;
    };
    auto *extension = classLinker->GetExtension(LanguageConfig::LANG);
    extension->EnumerateClasses(classRootVisitor, flags);

    LOG(DEBUG, GC) << "Finish collecting roots for classes";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateRefsToMovedObjects(const GCRootUpdater &gcRootUpdater)
{
    auto cb = [this, &gcRootUpdater](ManagedThread *thread) {
        UpdateRefsInVRegs(thread, gcRootUpdater);
        return true;
    };
    // Update refs in vregs
    vm_->GetThreadManager()->EnumerateThreads(cb);
    if constexpr (LanguageConfig::MT_MODE != MT_MODE_SINGLE) {
        // Update refs inside monitors
        vm_->GetMonitorPool()->EnumerateMonitors([&gcRootUpdater](Monitor *monitor) {
            ObjectHeader *objectHeader = monitor->GetObject();
            if (objectHeader == nullptr) {
                return true;
            }
            ObjectHeader *newObjectHeader = objectHeader;
            if (gcRootUpdater(&newObjectHeader)) {
                LOG(DEBUG, GC) << "Update monitor " << std::hex << monitor << " object, old val = " << objectHeader
                               << ", new val = " << newObjectHeader;
                monitor->SetObject(reinterpret_cast<ObjectHeader *>(newObjectHeader));
            }
            return true;
        });
    }
    if (vm_->UpdateMovedStrings(gcRootUpdater)) {
        // AOT string slots are pointing to strings from the StringTable,
        // so we should update it only if StringTable's pointers were updated.
        UpdateAotStringRoots(gcRootUpdater);
    }
    // Update thread locals
    UpdateThreadLocals(gcRootUpdater);
    // Update refs in vm
    UpdateVmRefs(gcRootUpdater);
    // Update refs in class linker contexts
    UpdateClassLinkerContextRoots(gcRootUpdater);
    // Update global refs
    UpdateGlobalObjectStorage(gcRootUpdater);
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateRefsInVRegs(ManagedThread *thread, const GCRootUpdater &gcRootUpdater)
{
    LOG(DEBUG, GC) << "Update frames for thread: " << thread->GetId();
    for (auto pframe = StackWalker::Create(thread); pframe.HasFrame(); pframe.NextFrame()) {
        LOG(DEBUG, GC) << "Frame for method " << pframe.GetMethod()->GetFullName();
        auto iterator = [&pframe, &gcRootUpdater](auto &regInfo, auto &vreg) {
            ObjectHeader *objectHeader = vreg.GetReference();
            if (objectHeader == nullptr) {
                return true;
            }
            ObjectHeader *newObjectHeader = objectHeader;
            if (!gcRootUpdater(&newObjectHeader)) {
                return true;
            }
            LOG(DEBUG, GC) << "Update vreg, vreg old val = " << objectHeader << ", new val = " << newObjectHeader;
            LOG_IF(regInfo.IsAccumulator(), DEBUG, GC) << "^ acc reg";
            if (!pframe.IsCFrame() && regInfo.IsAccumulator()) {
                LOG(DEBUG, GC) << "^ acc updated";
                vreg.SetReference(newObjectHeader);
            } else {
                pframe.template SetVRegValue<std::is_same_v<decltype(vreg), interpreter::DynamicVRegisterRef &>>(
                    regInfo, newObjectHeader);
            }
            return true;
        };
        pframe.IterateObjectsWithInfo(iterator);
    }
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateThreadLocals(const GCRootUpdater &gcRootUpdater)
{
    LOG(DEBUG, GC) << "=== ThreadLocals Update moved. BEGIN ===";
    vm_->GetThreadManager()->EnumerateThreads([&gcRootUpdater](ManagedThread *thread) {
        thread->UpdateGCRoots(gcRootUpdater);
        return true;
    });
    LOG(DEBUG, GC) << "=== ThreadLocals Update moved. END ===";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::VisitClassLinkerContextRoots(const GCRootVisitor &gcRootVisitor) const
{
    LOG(DEBUG, GC) << "Start collecting roots for class linker contexts";
    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *extension = classLinker->GetExtension(LanguageConfig::LANG);

    extension->EnumerateContexts([&gcRootVisitor](ClassLinkerContext *ctx) {
        ctx->VisitGCRoots([&gcRootVisitor](GCRoot root) {
            LOG(DEBUG, GC) << " Found root for class linker context "
                           << GetDebugInfoAboutObject(root.GetObjectHeader());
            gcRootVisitor(root);
        });
        return true;
    });
    LOG(DEBUG, GC) << "Finish collecting roots for class linker contexts";
}

template <class LanguageConfig>
void RootManager<LanguageConfig>::UpdateClassLinkerContextRoots(const GCRootUpdater &gcRootUpdater)
{
    auto classLinker = Runtime::GetCurrent()->GetClassLinker();
    auto *extension = classLinker->GetExtension(LanguageConfig::LANG);
    extension->EnumerateContexts([&gcRootUpdater](ClassLinkerContext *ctx) {
        ctx->UpdateGCRoots(gcRootUpdater);
        return true;
    });
}

uint32_t operator&(VisitGCRootFlags left, VisitGCRootFlags right)
{
    return static_cast<uint32_t>(left) & static_cast<uint32_t>(right);
}

VisitGCRootFlags operator|(VisitGCRootFlags left, VisitGCRootFlags right)
{
    return static_cast<VisitGCRootFlags>(static_cast<uint32_t>(left) | static_cast<uint32_t>(right));
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(RootManager);

}  // namespace ark::mem
