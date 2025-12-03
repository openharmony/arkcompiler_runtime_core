/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "libarkbase/os/file.h"
#include "libarkbase/utils/type_helpers.h"

#include <errhandlingapi.h>
#include <fcntl.h>
#include <fileapi.h>
#include <libloaderapi.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <share.h>
#include <filesystem>
#include <windows.h>

namespace ark::os::file {

static int GetFlags(Mode mode)
{
    switch (mode) {
        case Mode::READONLY:
            return _O_RDONLY | _O_BINARY;

        case Mode::READWRITE:
            return _O_RDWR | _O_BINARY;

        case Mode::WRITEONLY:
            return _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY;  // NOLINT(hicpp-signed-bitwise)

        case Mode::READWRITECREATE:
            return _O_RDWR | _O_CREAT | _O_BINARY;  // NOLINT(hicpp-signed-bitwise)

        default:
            break;
    }

    UNREACHABLE();
}

File Open(std::string_view filename, Mode mode)
{
    int fh;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    const auto PERM = _S_IREAD | _S_IWRITE;
    _sopen_s(&fh, filename.data(), GetFlags(mode), _SH_DENYNO, PERM);
    return File(fh);
}

}  // namespace ark::os::file

namespace ark::os::windows::file {

Expected<std::string, Error> File::GetTmpPath()
{
    WCHAR tempPathBuffer[MAX_PATH];
    DWORD dwRetVal = GetTempPathW(MAX_PATH, tempPathBuffer);
    if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
        return Unexpected(Error(GetLastError()));
    }
    std::wstring ws(tempPathBuffer);
    return std::string(ws.begin(), ws.end());
}

Expected<std::string, Error> File::GetExecutablePath()
{
    WCHAR path[MAX_PATH];
    DWORD dwRetVal = GetModuleFileNameW(NULL, path, MAX_PATH);
    if (dwRetVal > MAX_PATH || (dwRetVal == 0)) {
        return Unexpected(Error(GetLastError()));
    }
    std::wstring ws(path);
    std::string::size_type pos = std::string(ws.begin(), ws.end()).find_last_of(File::GetPathDelim());

    return (pos != std::string::npos) ? std::string(ws.begin(), ws.end()).substr(0, pos) : std::string("");
}

bool File::HasStatMode(const std::string &path, uint16_t mode)
{
    DWORD attrs = GetFileAttributesA(path.c_str());
    if (attrs == INVALID_FILE_ATTRIBUTES) {
        return false;  // does not exist
    }

    bool isDir = (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;

    bool wantFile = (mode & _S_IFREG) != 0;
    bool wantDir = (mode & _S_IFDIR) != 0;

    if (wantFile && isDir)
        return false;
    if (wantDir && !isDir)
        return false;

    bool isReadOnly = (attrs & FILE_ATTRIBUTE_READONLY) != 0;
    bool wantWrite = (mode & _S_IWRITE) != 0;  // Write permission, owner

    if (wantWrite && isReadOnly) {
        return false;
    }
    // Does not check _S_IREAD because read is always allowed on Windows if the file exists.

    // NOTE: check for _S_IFMT, _S_IFCHR, _S_IEXEC, are not implemented. Not required for current use cases.
    // If requested something unknown like _S_IFBLK, ignore it. On Windows these modes are not applicable.
    return true;
}

}  // namespace ark::os::windows::file
