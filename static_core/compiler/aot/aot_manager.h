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

#ifndef COMPILER_AOT_AOT_MANAGER_H
#define COMPILER_AOT_AOT_MANAGER_H

#include "aot_file.h"
#include "libarkfile/file.h"
#include "libarkbase/utils/arena_containers.h"
#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/mem/panda_string.h"
#include "runtime/include/method.h"
#include "libarkbase/utils/expected.h"

namespace ark::compiler {
class RuntimeInterface;

struct ProfilePathInfo {
    PandaString curProfilePath;
    PandaString baselineProfilePath;
};

class AotManager {
    using BitSetElement = uint32_t;
    static constexpr size_t MASK_WIDTH = BITS_PER_BYTE * sizeof(BitSetElement);

public:
    explicit AotManager() = default;

    NO_MOVE_SEMANTIC(AotManager);
    NO_COPY_SEMANTIC(AotManager);
    ~AotManager() = default;

    Expected<bool, std::string> AddFile(const std::string &fileName, RuntimeInterface *runtime, uint32_t gcType,
                                        bool force = false);

    const AotFile *GetFile(const std::string &fileName) const;

    const AotPandaFile *FindPandaFile(const std::string &fileName);

    bool IsFileInAotClassContext(const std::string &fileName, bool isAotVerifyAbsPath) const;

    PandaString GetBootClassContext() const
    {
        os::memory::LockHolder lock {profiledFilesLock_};
        return bootClassContext_;
    }

    void ParseClassContextToFile(std::string_view context);

    void SetBootClassContext(PandaString context, bool isArkAot)
    {
        {
            os::memory::LockHolder lock {profiledFilesLock_};
            bootClassContext_ = std::move(context);
            ReparseProfiledPandaFilesLocked();
        }
        UpdatePandaFilesSnapshot(isArkAot, true, false);
    }

    PandaString GetAppClassContext() const
    {
        os::memory::LockHolder lock {profiledFilesLock_};
        return appClassContext_;
    }

    void SetAppClassContext(PandaString context, bool isArkAot)
    {
        {
            os::memory::LockHolder lock {profiledFilesLock_};
            appClassContext_ = std::move(context);
            ReparseProfiledPandaFilesLocked();
        }
        UpdatePandaFilesSnapshot(isArkAot, false, true);
    }

    void AddProfiledPandaFile(const panda_file::File &pf)
    {
        auto path = PandaString(pf.GetFullFileName());
        {
            os::memory::LockHolder mapLock {profilePathMapLock_};
            if (profiledPandaFilesMap_.empty() || profiledPandaFilesMap_.find(path) == profiledPandaFilesMap_.end()) {
                return;
            }
        }
        os::memory::LockHolder lock {profiledFilesLock_};
        auto entry = path + HASH_DELIM + PandaString(pf.GetPaddedChecksum());
        if (appClassContext_.find(entry) != PandaString::npos) {
            return;
        }
        if (!appClassContext_.empty()) {
            appClassContext_.push_back(DELIM);
        }
        appClassContext_.append(entry);
        ReparseProfiledPandaFilesLocked();
    }

    void VerifyClassHierarchy();

    uint32_t GetAotStringRootsCount()
    {
        // use counter to get roots count without acquiring vector's lock
        // Atomic with acquire order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after
        // the load which should become visible
        return aotStringGcRootsCount_.load(std::memory_order_acquire);
    }

    void RegisterAotStringRoot(ObjectHeader **slot, bool isYoung);

    template <typename Callback>
    void VisitAotStringRoots(Callback cb, bool visitOnlyYoung)
    {
        ASSERT(aotStringGcRoots_.empty() ||
               (aotStringYoungSet_.size() - 1) == (aotStringGcRoots_.size() - 1) / MASK_WIDTH);

        if (!visitOnlyYoung) {
            for (auto root : aotStringGcRoots_) {
                cb(root);
            }
            return;
        }

        if (!hasYoungAotStringRefs_) {
            return;
        }

        // Atomic with acquire order reason: data race with aot_string_gc_roots_count_ with dependecies on reads after
        // the load which should become visible
        size_t totalRoots = aotStringGcRootsCount_.load(std::memory_order_acquire);
        for (size_t idx = 0; idx < aotStringYoungSet_.size(); idx++) {
            auto mask = aotStringYoungSet_[idx];
            if (mask == 0) {
                continue;
            }
            for (size_t offset = 0; offset < MASK_WIDTH && idx * MASK_WIDTH + offset < totalRoots; offset++) {
                if ((mask & (1ULL << offset)) != 0) {
                    cb(aotStringGcRoots_[idx * MASK_WIDTH + offset]);
                }
            }
        }
    }

    bool InAotFileRange(uintptr_t pc)
    {
        for (auto &aotFile : aotFiles_) {
            auto code = reinterpret_cast<uintptr_t>(aotFile->GetCode());
            if (pc >= code && pc < code + reinterpret_cast<uintptr_t>(aotFile->GetCodeSize())) {
                return true;
            }
        }
        return false;
    }

    bool HasAotFiles()
    {
        return !aotFiles_.empty();
    }

    void TryAddMethodToProfile(Method *method)
    {
        auto pfName = method->GetPandaFile()->GetFullFileName();
        bool shouldCollectMethod = false;
        {
            os::memory::LockHolder filesLock {profiledFilesLock_};
            shouldCollectMethod = profiledPandaFiles_.find(pfName) != profiledPandaFiles_.end();
        }
        if (!shouldCollectMethod) {
            return;
        }
        os::memory::LockHolder methodsLock {profiledMethodsLock_};
        profiledMethods_.push_back(method);
    }

    bool HasProfiledMethods()
    {
        os::memory::LockHolder lock {profiledMethodsLock_};
        return !profiledMethods_.empty();
    }

    PandaList<Method *>::const_iterator GetProfiledMethodsFinal() const
    {
        os::memory::LockHolder lock {profiledMethodsLock_};
        return --profiledMethods_.cend();
    }

    PandaList<Method *> &GetProfiledMethods()
    {
        return profiledMethods_;
    }

    using ProfilePathEntry = std::pair<PandaString, ProfilePathInfo>;
    void RegisterProfilePaths(PandaVector<ProfilePathEntry> &&entries);

    PandaUnorderedMap<PandaString, ProfilePathInfo> GetProfiledPandaFilesMapSnapshot() const;

    PandaUnorderedSet<std::string> GetProfiledPandaFiles()
    {
        os::memory::LockHolder lock {profiledFilesLock_};
        return profiledPandaFiles_;
    }

    using PandaFileLoadData = std::pair<const panda_file::File *, ClassLinkerContext *>;

    void UpdatePandaFilesSnapshot(const panda_file::File *pf, ClassLinkerContext *ctx, bool isArkAot);
    uint32_t GetPandaFileSnapshotIndex(const std::string &fileName);
    const panda_file::File *GetPandaFileBySnapshotIndex(uint32_t index) const;

private:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr char HASH_DELIM = '*';
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr char DELIM = ':';
    PandaVector<std::unique_ptr<AotFile>> aotFiles_;
    PandaUnorderedMap<std::string, AotPandaFile> filesMap_;
    PandaString bootClassContext_ GUARDED_BY(profiledFilesLock_);
    PandaString appClassContext_ GUARDED_BY(profiledFilesLock_);

    mutable os::memory::Mutex profiledMethodsLock_;
    PandaList<Method *> profiledMethods_ GUARDED_BY(profiledMethodsLock_);

    mutable os::memory::Mutex profiledFilesLock_;
    PandaUnorderedSet<std::string> profiledPandaFiles_ GUARDED_BY(profiledFilesLock_);

    mutable os::memory::Mutex profilePathMapLock_;
    PandaUnorderedMap<PandaString, ProfilePathInfo> profiledPandaFilesMap_ GUARDED_BY(profilePathMapLock_);

    os::memory::RecursiveMutex aotStringRootsLock_;
    PandaVector<ObjectHeader **> aotStringGcRoots_;
    std::atomic_uint32_t aotStringGcRootsCount_ {0};
    bool hasYoungAotStringRefs_ {false};
    PandaVector<BitSetElement> aotStringYoungSet_;

    void ReparseProfiledPandaFiles()
    {
        os::memory::LockHolder lock {profiledFilesLock_};
        ReparseProfiledPandaFilesLocked();
    }

    mutable os::memory::Mutex snapshotFilesLock_;
    PandaVector<std::pair<std::string_view, PandaFileLoadData>> pandaFilesSnapshot_ GUARDED_BY(snapshotFilesLock_);
    PandaUnorderedMap<std::string_view, PandaFileLoadData> pandaFilesLoaded_ GUARDED_BY(snapshotFilesLock_);

    void UpdatePandaFilesSnapshot(bool isArkAot, bool bootContext, bool appContext);
    void ReparseProfiledPandaFilesLocked() REQUIRES(profiledFilesLock_)
    {
        profiledPandaFiles_.clear();
        ParseClassContextToFileLocked(bootClassContext_);
        ParseClassContextToFileLocked(appClassContext_);
    }
    void ParseClassContextToFileLocked(std::string_view context) REQUIRES(profiledFilesLock_);
};

class AotClassContextCollector {
public:
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr char DELIMETER = ':';
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    static constexpr char HASH_DELIMETER = '*';
    explicit AotClassContextCollector(PandaString *acc, bool useAbsPath = true) : acc_(acc), useAbsPath_(useAbsPath) {};
    bool operator()(const panda_file::File &pf);

    DEFAULT_MOVE_SEMANTIC(AotClassContextCollector);
    DEFAULT_COPY_SEMANTIC(AotClassContextCollector);
    ~AotClassContextCollector() = default;

private:
    PandaString *acc_;
    bool useAbsPath_;
};
}  // namespace ark::compiler

#endif  // COMPILER_AOT_AOT_MANAGER_H
