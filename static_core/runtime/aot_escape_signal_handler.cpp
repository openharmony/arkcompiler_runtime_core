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

#include "signal_handler.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libarkbase/os/file.h"
#include "include/runtime.h"
#include "libarkbase/utils/logger.h"
#include "runtime/on_stack_string.h"
#include "securec.h"

namespace ark {

std::array<char, PATH_MAX> AotEscapeSignalHandler::escapeSignalFlagFilePath_ = {
    "/data/storage/ark-profile/aot_escape.txt"};
bool AotEscapeSignalHandler::isEscaped_ = false;

void AotEscapeSignalHandler::SetEscaped(bool isEscape)
{
    AotEscapeSignalHandler::isEscaped_ = isEscape;
}

void AotEscapeSignalHandler::SetEscapedFlagFilePath(const std::string &escapeFlagPath)
{
    size_t copySize = std::min(escapeFlagPath.size(), escapeSignalFlagFilePath_.size() - 1U);
    errno_t result =
        strncpy_s(escapeSignalFlagFilePath_.data(), escapeSignalFlagFilePath_.size(), escapeFlagPath.c_str(), copySize);
    if (result != EOK) {
        escapeSignalFlagFilePath_[0U] = '\0';
    }
}

bool AotEscapeSignalHandler::IsTargetSignal(int sig)
{
    return sig == SIGSEGV || sig == SIGABRT || sig == SIGBUS || sig == SIGFPE || sig == SIGILL;
}

bool AotEscapeSignalHandler::Action(int sig, [[maybe_unused]] siginfo_t *siginfo, void *context)
{
    return HandleAction(sig, siginfo, context);
}

#if defined(PANDA_TARGET_OHOS) || (defined(PANDA_TARGET_AMD64))
constexpr size_t AOT_ESCAPE_LINE_STRING_SIZE = 128;
constexpr size_t AOT_ESCAPE_FILE_CONTENT_BUFFER_SIZE = 1024;

static bool WriteAotEscapeBuffer(int fd, const char *buffer, size_t bufferSize)
{
    while (bufferSize > 0) {
        ssize_t writeSize = write(fd, buffer, bufferSize);
        if (writeSize <= 0) {
            return false;
        }
        buffer += writeSize;
        bufferSize -= static_cast<size_t>(writeSize);
    }
    return true;
}

static size_t ReadAotEscapeContent(const char *filePath, char *buffer, size_t bufferSize)
{
    size_t contentSize = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ark::os::file::File readFd(open(filePath, O_RDONLY | O_CLOEXEC));
    if (!readFd.IsValid()) {
        return contentSize;
    }
    ark::os::file::FileHolder readFdGuard(readFd);
    while (contentSize < bufferSize) {
        ssize_t readSize = read(readFd.GetFd(), buffer + contentSize, bufferSize - contentSize);
        if (readSize <= 0) {
            break;
        }
        contentSize += static_cast<size_t>(readSize);
    }
    return contentSize;
}

bool AotEscapeSignalHandler::HandleAction(int sig, [[maybe_unused]] siginfo_t *siginfo, void *context)
{
    if (!IsTargetSignal(sig)) {
        return false;
    }
    if (IsEscapeSignalFlagExists()) {
        return false;
    }
    if (!Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->HasAotFiles()) {
        return false;
    }
    // Signal handler path: diagnostics below must stay at DEBUG level only.
    // Raising them to INFO/ERROR would make logging active in release builds and reintroduce signal-unsafe calls.
    LOG(DEBUG, RUNTIME) << "AotEscapeSignalHandler handle AOT escape type, signal = " << sig;

    uintptr_t const pc = SignalContext(context).GetPC();
    auto isInAot = Runtime::GetCurrent()->GetClassLinker()->GetAotManager()->InAotFileRange(pc);
    OnStackString<AOT_ESCAPE_LINE_STRING_SIZE> line;
    line << "Signal: " << sig << ", PC: " << pc << ", isInAot: " << (isInAot ? "true\n" : "false\n");
    std::array<char, AOT_ESCAPE_FILE_CONTENT_BUFFER_SIZE> contentBuf {};
    size_t contentSize = ReadAotEscapeContent(escapeSignalFlagFilePath_.data(), contentBuf.data(), contentBuf.size());
    const auto filePerm = 0644;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const auto perm = O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ark::os::file::File fd(open(escapeSignalFlagFilePath_.data(), perm, filePerm));
    if (!fd.IsValid()) {
        // Keep DEBUG level only. strerror()/LOG are not async-signal-safe.
        LOG(DEBUG, RUNTIME) << "AotEscapeSignalHandler handle AOT escape. Failed to open: " << strerror(errno);
        return false;
    }
    ark::os::file::FileHolder fdGuard(fd);
    if ((contentSize > 0 && !WriteAotEscapeBuffer(fd.GetFd(), contentBuf.data(), contentSize)) ||
        !WriteAotEscapeBuffer(fd.GetFd(), line.data(), line.size())) {
        // Keep DEBUG level only. strerror()/LOG are not async-signal-safe.
        LOG(DEBUG, RUNTIME) << "AotEscapeSignalHandler handle AOT escape. Failed to write: " << strerror(errno);
    }
    return false;
}

bool AotEscapeSignalHandler::IsEscapeSignalFlagExists()
{
    if (isEscaped_) {
        return true;
    }
    ark::os::file::File fd(open(escapeSignalFlagFilePath_.data(), O_RDONLY | O_CLOEXEC));
    if (!fd.IsValid()) {
        return false;
    }
    ark::os::file::FileHolder fdGuard(fd);
    int lineCount = 0;
    char buf;
    bool hasUnterminatedLine = false;

    while (true) {
        ssize_t readSize = read(fd.GetFd(), &buf, 1);
        if (readSize <= 0) {
            break;
        }
        if (buf == '\n') {
            lineCount++;
            hasUnterminatedLine = false;
            if (lineCount >= CRASH_SIGNAL_THRESHOLD) {
                isEscaped_ = true;
                break;
            }
        } else {
            hasUnterminatedLine = true;
        }
    }
    if (!isEscaped_ && hasUnterminatedLine) {
        lineCount++;
        if (lineCount >= CRASH_SIGNAL_THRESHOLD) {
            isEscaped_ = true;
        }
    }
    return isEscaped_;
}
#else
bool AotEscapeSignalHandler::HandleAction([[maybe_unused]] int sig, [[maybe_unused]] siginfo_t *siginfo,
                                          [[maybe_unused]] void *context)
{
    return isEscaped_;
}

bool AotEscapeSignalHandler::IsEscapeSignalFlagExists()
{
    return isEscaped_;
}
#endif

}  // namespace ark
