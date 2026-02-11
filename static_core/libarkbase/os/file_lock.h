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

#ifndef PANDA_LIBPANDABASE_PBASE_OS_FILE_LOCK_H_
#define PANDA_LIBPANDABASE_PBASE_OS_FILE_LOCK_H_

#include "libarkbase/macros.h"

#if !defined(PANDA_TARGET_WINDOWS)

#include "libarkbase/os/file.h"

#include <sys/file.h>

namespace ark::os::file_lock {

// RAII wrapper for flock-based advisory file locking.
// Acquires lock on construction, releases on destruction.
// Used to coordinate concurrent access to files between processes.
class ScopedFileLock {
public:
    ScopedFileLock(const char *path, int lockType)
    {
        auto mode = (lockType == LOCK_EX) ? file::Mode::READWRITECREATE : file::Mode::READONLY;
        file_ = file::Open(path, mode);
        if (!file_.IsValid()) {
            return;
        }
        if (flock(file_.GetFd(), lockType) != 0) {
            file_.Close();
            file_ = file::File(-1);
        }
    }

    ~ScopedFileLock()
    {
        if (file_.IsValid()) {
            flock(file_.GetFd(), LOCK_UN);
            file_.Close();
        }
    }

    bool IsLocked() const
    {
        return file_.IsValid();
    }

    NO_COPY_SEMANTIC(ScopedFileLock);
    NO_MOVE_SEMANTIC(ScopedFileLock);

private:
    file::File file_ {-1};
};

}  // namespace ark::os::file_lock

#endif  // !defined(PANDA_TARGET_WINDOWS)

#endif  // PANDA_LIBPANDABASE_PBASE_OS_FILE_LOCK_H_
