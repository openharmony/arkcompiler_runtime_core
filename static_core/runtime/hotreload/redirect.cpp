/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "runtime/hotreload/redirect.h"

#include <unordered_map>

#include "libarkbase/os/mutex.h"
// `PublishRedirects` is declared there: its argument type is the transaction's own container, and
// keeping `redirect.h` free of runtime container headers keeps the fast-path include cheap
#include "runtime/hotreload/hotreload.h"

namespace ark::hotreload {

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
PANDA_PUBLIC_API std::atomic<bool> g_redirectActive {false};

namespace {

/*
 * Plain std containers behind a plain reader-writer lock, on purpose.
 *
 * Readers are ANI callers, which are in NATIVE state and are therefore NOT stopped by the commit's
 * STW --- so this really is a concurrent structure and not something the world stop protects. The
 * read section is a single hash lookup with no allocation and no other lock, so the write side,
 * which runs inside STW, waits for a bounded time and no lock cycle exists.
 *
 * The maps are deliberately not `PandaUnorderedMap`: they outlive individual transactions and must
 * be destructible without the runtime's internal allocator being alive.
 */
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
os::memory::RWLock g_lock;
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::unordered_map<Method *, Method *> g_methods GUARDED_BY(g_lock);
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
std::unordered_map<Field *, Field *> g_fields GUARDED_BY(g_lock);

/**
 * @brief Merge one generation's old->new pairs into @a table, keeping it transitive.
 *
 * Existing entries are rewritten first: an entry `a -> b` where this generation replaces `b` by
 * `c` becomes `a -> c`. Only then are this generation's own pairs added. That keeps every lookup a
 * single hop, so the read path stays O(1) no matter how many reloads have happened.
 */
template <class T, class Batch>
void MergeGeneration(std::unordered_map<T *, T *> *table, const Batch &batch) REQUIRES(g_lock)
{
    for (auto &entry : *table) {
        auto it = batch.find(entry.second);
        if (it != batch.end()) {
            entry.second = it->second;
        }
    }
    for (const auto &pair : batch) {
        // A key can only be introduced once: it names an entity that this transaction just moved
        // onto the obsolete class, and an obsolete entity is never swapped again
        (*table)[pair.first] = pair.second;
    }
}

}  // namespace

Method *ResolveMethodRedirect(Method *method)
{
    if (method == nullptr) {
        return nullptr;
    }
    os::memory::ReadLockHolder lock(g_lock);
    auto it = g_methods.find(method);
    return it == g_methods.end() ? method : it->second;
}

Field *ResolveFieldRedirect(Field *field)
{
    if (field == nullptr) {
        return nullptr;
    }
    os::memory::ReadLockHolder lock(g_lock);
    auto it = g_fields.find(field);
    return it == g_fields.end() ? field : it->second;
}

void PublishRedirects(const PandaUnorderedMap<Method *, Method *> &methods,
                      const PandaUnorderedMap<Field *, Field *> &fields)
{
    if (methods.empty() && fields.empty()) {
        return;
    }
    {
        os::memory::WriteLockHolder lock(g_lock);
        MergeGeneration(&g_methods, methods);
        MergeGeneration(&g_fields, fields);
    }
    // Atomic with release order reason: publishes the maps above to `IsRedirectActive` readers
    g_redirectActive.store(true, std::memory_order_release);
}

void ResetRedirects()
{
    // Atomic with release order reason: turns the fast path back off before the maps are emptied,
    // so no reader can be sent to an entry that is about to disappear
    g_redirectActive.store(false, std::memory_order_release);
    os::memory::WriteLockHolder lock(g_lock);
    g_methods.clear();
    g_fields.clear();
}

}  // namespace ark::hotreload
