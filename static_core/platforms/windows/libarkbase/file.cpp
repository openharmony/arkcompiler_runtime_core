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

namespace {
bool PathEqualsCaseSensitive(const std::wstring &a, const std::wstring &b)
{
    if (a == b) {
        return true;
    }
    if (a.size() != b.size()) {
        return false;
    }

    bool drivePos = (a.size() > 1U && a[1U] == L':' && b[1U] == L':');
    if (drivePos) {
        return std::towlower(a[0U]) == std::towlower(b[0U]) &&
               a.compare(1U, std::wstring::npos, b, 1U, std::wstring::npos) == 0;
    }
    return false;
}
}  // namespace

bool File::HasStatModeCaseSensitive(const std::string &path, uint16_t mode)
{
    std::filesystem::path p = std::filesystem::absolute(path);
    std::wstring input = p.wstring();
    std::replace(input.begin(), input.end(), L'/', L'\\');

    // Prefix with `\\?\` to bypass MAX_PATH (up to 32767). FILE_FLAG_BACKUP_SEMANTICS
    // is required so that directories can be opened as well as regular files.
    std::wstring openPath = L"\\\\?\\" + input;
    HANDLE h = CreateFileW(openPath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                           OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    if (h == INVALID_HANDLE_VALUE) {
        return false;
    }

    bool result = false;
    constexpr DWORD BUF_LEN = 32767U;
    std::wstring realBuf(BUF_LEN, L'\0');
    // FILE_NAME_NORMALIZED returns the canonical path with the real on-disk casing.
    DWORD len = GetFinalPathNameByHandleW(h, realBuf.data(), BUF_LEN, FILE_NAME_NORMALIZED);
    CloseHandle(h);
    if (len == 0U || len >= BUF_LEN) {
        return false;
    }
    realBuf.resize(len);

    // realBuf is "\\?\C:\proj\...\file" (correct case). Strip the \\?\ prefix.
    const std::wstring prefix = L"\\\\?\\";
    if (realBuf.compare(0U, prefix.size(), prefix) == 0) {
        realBuf.erase(0U, prefix.size());
    }

    if (PathEqualsCaseSensitive(realBuf, input)) {
        DWORD attr = GetFileAttributesW(openPath.c_str());
        bool isDir = (attr != INVALID_FILE_ATTRIBUTES) && ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0);
        if ((mode & _S_IFDIR) && isDir) {
            result = true;
        } else if ((mode & _S_IFREG) && !isDir) {
            result = true;
        }
    }
    return result;
}

}  // namespace ark::os::windows::file
