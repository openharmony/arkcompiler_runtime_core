/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <fstream>
#include <sstream>
#include <iomanip>
#include <memory>
#include <algorithm>
#include <unwind.h>

#include "stacktrace.h"
#include "os/mutex.h"

#include <cxxabi.h>
#include <dlfcn.h>
#include "debug_info.h"

namespace ark {

struct VmaEntry {
    enum DebugInfoStatus { NOT_READ, VALID, BAD };

    // NOLINTNEXTLINE(modernize-pass-by-value)
    VmaEntry(uintptr_t paramStartAddr, uintptr_t paramEndAddr, uintptr_t paramOffset, const std::string &fname)
        : startAddr(paramStartAddr), endAddr(paramEndAddr), offset(paramOffset), filename(fname)
    {
    }

    ~VmaEntry() = default;

    uintptr_t startAddr;                // NOLINT(misc-non-private-member-variables-in-classes)
    uintptr_t endAddr;                  // NOLINT(misc-non-private-member-variables-in-classes)
    uintptr_t offset;                   // NOLINT(misc-non-private-member-variables-in-classes)
    std::string filename;               // NOLINT(misc-non-private-member-variables-in-classes)
    DebugInfoStatus status {NOT_READ};  // NOLINT(misc-non-private-member-variables-in-classes)
    DebugInfo debugInfo;                // NOLINT(misc-non-private-member-variables-in-classes)

    DEFAULT_MOVE_SEMANTIC(VmaEntry);
    NO_COPY_SEMANTIC(VmaEntry);
};

class Tokenizer {
public:
    // NOLINTNEXTLINE(modernize-pass-by-value)
    explicit Tokenizer(const std::string &str) : str_(str) {}

    std::string Next(char delim = ' ')
    {
        while (pos_ < str_.length() && str_[pos_] == ' ') {
            ++pos_;
        }
        size_t pos = str_.find(delim, pos_);
        std::string token;
        if (pos == std::string::npos) {
            token = str_.substr(pos_);
            pos_ = str_.length();
        } else {
            token = str_.substr(pos_, pos - pos_);
            pos_ = pos + 1;  // skip delimiter
        }
        return token;
    }

private:
    std::string str_;
    size_t pos_ {0};
};

class StackPrinter {
public:
    static StackPrinter &GetInstance()
    {
        static StackPrinter printer;
        return printer;
    }

    std::ostream &Print(const std::vector<uintptr_t> &stacktrace, std::ostream &out)
    {
        os::memory::LockHolder lock(mutex_);
        ScanVma();
        for (size_t frameNum = 0; frameNum < stacktrace.size(); ++frameNum) {
            PrintFrame(frameNum, stacktrace[frameNum], out);
        }
        return out;
    }

    NO_MOVE_SEMANTIC(StackPrinter);
    NO_COPY_SEMANTIC(StackPrinter);

private:
    explicit StackPrinter() = default;
    ~StackPrinter() = default;

    void PrintFrame(size_t frameNum, uintptr_t pc, std::ostream &out)
    {
        std::ios_base::fmtflags f = out.flags();
        auto w = out.width();
        out << "#" << std::setw(2U) << std::left << frameNum << ": 0x" << std::hex << pc << " ";
        out.flags(f);
        out.width(w);

        VmaEntry *vma = FindVma(pc);
        if (vma == nullptr) {
            vmas_.clear();
            ScanVma();
            vma = FindVma(pc);
        }
        if (vma != nullptr) {
            uintptr_t pcOffset = pc - vma->startAddr + vma->offset;
            // pc points to the instruction after the call
            // Decrement pc to get source line number pointing to the function call
            --pcOffset;
            std::string function;
            std::string srcFile;
            unsigned int line = 0;
            if (ReadDebugInfo(vma) && vma->debugInfo.GetSrcLocation(pcOffset, &function, &srcFile, &line)) {
                PrintFrame(function, srcFile, line, out);
                return;
            }
            uintptr_t offset = 0;
            if (ReadSymbol(pc, &function, &offset)) {
                PrintFrame(function, offset, out);
                return;
            }
        }
        out << "??:??\n";
    }

    void PrintFrame(const std::string &function, const std::string &srcFile, unsigned int line, std::ostream &out)
    {
        if (function.empty()) {
            out << "??";
        } else {
            Demangle(function, out);
        }
        out << "\n     at ";
        if (srcFile.empty()) {
            out << "??";
        } else {
            out << srcFile;
        }
        out << ":";
        if (line == 0) {
            out << "??";
        } else {
            out << line;
        }

        out << "\n";
    }

    void PrintFrame(const std::string &function, uintptr_t offset, std::ostream &out)
    {
        std::ios_base::fmtflags f = out.flags();
        Demangle(function, out);
        out << std::hex << "+0x" << offset << "\n";
        out.flags(f);
    }

    bool ReadSymbol(uintptr_t pc, std::string *function, uintptr_t *offset)
    {
        Dl_info info {};
        if (dladdr(reinterpret_cast<void *>(pc), &info) != 0 && info.dli_sname != nullptr) {
            *function = info.dli_sname;
            *offset = pc - reinterpret_cast<uintptr_t>(info.dli_saddr);
            return true;
        }
        return false;
    }

    void Demangle(const std::string &function, std::ostream &out)
    {
        size_t length = 0;
        int status = 0;
        char *demangledFunction = abi::__cxa_demangle(function.c_str(), nullptr, &length, &status);
        if (status == 0) {
            out << demangledFunction;
            free(demangledFunction);  // NOLINT(cppcoreguidelines-no-malloc)
        } else {
            out << function;
        }
    }

    VmaEntry *FindVma(uintptr_t pc)
    {
        VmaEntry el(pc, pc, 0, "");
        auto it = std::upper_bound(vmas_.begin(), vmas_.end(), el,
                                   [](const VmaEntry &e1, const VmaEntry &e2) { return e1.endAddr < e2.endAddr; });
        if (it != vmas_.end() && (it->startAddr <= pc && pc < it->endAddr)) {
            return &(*it);
        }
        return nullptr;
    }

    bool ReadDebugInfo(VmaEntry *vma)
    {
        if (vma->status == VmaEntry::VALID) {
            return true;
        }
        if (vma->status == VmaEntry::BAD) {
            return false;
        }
        if (!vma->filename.empty() && vma->debugInfo.ReadFromFile(vma->filename.c_str()) == DebugInfo::SUCCESS) {
            vma->status = VmaEntry::VALID;
            return true;
        }
        vma->status = VmaEntry::BAD;
        return false;
    }

    void ScanVma()
    {
        static const int HEX_RADIX = 16;
        static const size_t MODE_FIELD_LEN = 4;
        static const size_t XMODE_POS = 2;

        if (!vmas_.empty()) {
            return;
        }

        std::stringstream fname;
        fname << "/proc/self/maps";
        std::string filename = fname.str();
        std::ifstream maps(filename.c_str());

        while (maps) {
            std::string line;
            std::getline(maps, line);
            Tokenizer tokenizer(line);
            std::string startAddr = tokenizer.Next('-');
            std::string endAddr = tokenizer.Next();
            std::string rights = tokenizer.Next();
            if (rights.length() == MODE_FIELD_LEN && rights[XMODE_POS] == 'x') {
                std::string offset = tokenizer.Next();
                tokenizer.Next();
                tokenizer.Next();
                std::string objFilename = tokenizer.Next();
                vmas_.emplace_back(stoul(startAddr, nullptr, HEX_RADIX), stoul(endAddr, nullptr, HEX_RADIX),
                                   stoul(offset, nullptr, HEX_RADIX), objFilename);
            }
        }
    }

private:
    std::vector<VmaEntry> vmas_;
    os::memory::Mutex mutex_;
};

class Buf {
public:
    Buf(uintptr_t *buf, size_t skip, size_t capacity) : buf_(buf), skip_(skip), capacity_(capacity) {}

    void Append(uintptr_t pc)
    {
        if (skip_ > 0) {
            // Skip the element
            --skip_;
            return;
        }
        if (size_ >= capacity_) {
            return;
        }
        buf_[size_++] = pc;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }

    int Size() const
    {
        return static_cast<int>(size_);
    }

private:
    uintptr_t *buf_;
    size_t skip_;
    size_t size_ {0};
    size_t capacity_;
};

static _Unwind_Reason_Code FrameHandler(struct _Unwind_Context *ctx, [[maybe_unused]] void *arg)
{
    Buf *buf = reinterpret_cast<Buf *>(arg);
    uintptr_t pc = _Unwind_GetIP(ctx);
    // _Unwind_GetIP returns 0 pc at the end of the stack. Ignore it
    if (pc != 0) {
        buf->Append(pc);
    }
    return _URC_NO_REASON;
}

static size_t GetStacktrace(uintptr_t *buf, size_t size)
{
    static constexpr int SKIP_FRAMES = 2;  // backtrace
    Buf bufWrapper(buf, SKIP_FRAMES, size);
    _Unwind_Reason_Code res = _Unwind_Backtrace(FrameHandler, &bufWrapper);
    if (res != _URC_END_OF_STACK || bufWrapper.Size() < 0) {
        return 0;
    }

    return bufWrapper.Size();
}

std::vector<uintptr_t> GetStacktrace()
{
    static constexpr size_t BUF_SIZE = 100;
    std::vector<uintptr_t> buf;
    buf.resize(BUF_SIZE);
    size_t size = GetStacktrace(buf.data(), buf.size());
    buf.resize(size);
    return buf;
}

std::ostream &PrintStack(const std::vector<uintptr_t> &stacktrace, std::ostream &out)
{
    return StackPrinter::GetInstance().Print(stacktrace, out);
}

}  // namespace ark
