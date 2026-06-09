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

#include <algorithm>
#include <atomic>
#include <sstream>
#include <thread>

#include "libarkfile/file_items.h"
#include "libarkfile/file_reader.h"
#include "libarkbase/os/mutex.h"

#include "linker.h"
#include "linker_context.h"

namespace ark::static_linker {

namespace {

using ReaderEntry = std::pair<const std::string *, ark::panda_file::FileReader *>;

struct ReadWorkerData {
    const std::vector<ReaderEntry> *readers;
    std::vector<uint8_t> *readResults;
    std::atomic<size_t> *nextReader;
};

void ReadContainers(ReadWorkerData *data)
{
    // Atomic with relaxed order reason: counter only gives each thread a separate reader index
    for (auto idx = data->nextReader->fetch_add(1, std::memory_order_relaxed); idx < data->readers->size();
         idx = data->nextReader->fetch_add(1, std::memory_order_relaxed)) {
        (*data->readResults)[idx] = (*data->readers)[idx].second->ReadContainer(false) ? 1U : 0U;
    }
}

void DemangleName(std::ostream &o, std::string_view s)
{
    if (s.empty()) {
        return;
    }
    if (s.back() == 0) {
        s = s.substr(0, s.size() - 1);
    }
    if (s.empty()) {
        return;
    }

    if (s.front() == '[') {
        DemangleName(o, s.substr(1));
        o << "[]";
        return;
    }

    if (s.size() == 1) {
        auto ty = panda_file::Type::GetTypeIdBySignature(s.front());
        o << ty;
        return;
    }

    if (s.front() == 'L' && s.back() == ';') {
        s = s.substr(1, s.size() - 2UL);
        while (!s.empty()) {
            const auto to = s.find('/');
            o << s.substr(0, to);
            if (to != std::string::npos) {
                o << ".";
                s = s.substr(to + 1);
            } else {
                break;
            }
        }
        return;
    }

    o << "<unknown>";
}

void ReprItem(std::ostream &o, const panda_file::BaseItem *i);

void ReprMethod(std::ostream &o, panda_file::StringItem *name, panda_file::BaseClassItem *clz, panda_file::ProtoItem *p)
{
    auto typs = Helpers::BreakProto(p);
    auto refs = p->GetRefTypes();
    size_t numRefs = 0;
    auto reprType = [&typs, &refs, &numRefs, &o](size_t ii) {
        auto &t = typs[ii];
        if (t.IsPrimitive()) {
            o << t;
        } else {
            ReprItem(o, refs[numRefs++]);
        }
    };
    reprType(0);
    o << " ";
    ReprItem(o, clz);
    o << "." << name->GetData();
    o << "(";
    for (size_t ii = 1; ii < typs.size(); ii++) {
        reprType(ii);
        if (ii + 1 != typs.size()) {
            o << ", ";
        }
    }
    o << ")";
}

void ReprValueItem(std::ostream &o, const panda_file::BaseItem *i)
{
    auto j = static_cast<const panda_file::ValueItem *>(i);
    switch (j->GetType()) {
        case panda_file::ValueItem::Type::ARRAY: {
            auto *arr = static_cast<const panda_file::ArrayValueItem *>(j);
            const auto &its = arr->GetItems();
            o << "[";
            for (size_t k = 0; k < its.size(); k++) {
                if (k != 0) {
                    o << ", ";
                }
                ReprItem(o, &its[k]);
            }
            o << "]";
            break;
        }
        case panda_file::ValueItem::Type::INTEGER: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            o << scalar->GetValue<uint32_t>() << " as int";
            break;
        }
        case panda_file::ValueItem::Type::LONG: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            o << scalar->GetValue<uint64_t>() << " as long";
            break;
        }
        case panda_file::ValueItem::Type::FLOAT: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            o << scalar->GetValue<float>() << " as float";
            break;
        }
        case panda_file::ValueItem::Type::DOUBLE: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            o << scalar->GetValue<double>() << " as double";
            break;
        }
        case panda_file::ValueItem::Type::ID: {
            auto *scalar = static_cast<const panda_file::ScalarValueItem *>(j);
            ReprItem(o, scalar->GetIdItem());
            break;
        }
        default:
            UNREACHABLE();
    }
}

void ReprAnnotationItem(std::ostream &o, const panda_file::BaseItem *i)
{
    auto j = static_cast<const panda_file::AnnotationItem *>(i);
    ReprItem(o, j->GetClassItem());
    o << "<";
    bool first = true;
    for (auto &a : *j->GetElements()) {
        if (first) {
            first = false;
        } else {
            o << ", ";
        }
        o << a.GetName()->GetData() << "=";
        ReprItem(o, a.GetValue());
    }
    o << ">";
}

void ReprStringItem(std::ostream &o, const panda_file::BaseItem *i)
{
    auto str = static_cast<const panda_file::StringItem *>(i);
    auto view = std::string_view(str->GetData());
    o << '"';
    while (!view.empty()) {
        auto pos = view.find_first_of("\"\\\n");
        o << view.substr(0, pos);
        if (pos != std::string::npos) {
            if (view[pos] == '\n') {
                o << "\\n";
            } else {
                o << '\\' << view[pos];
            }
            view = view.substr(pos + 1);
        } else {
            view = "";
        }
    }
    o << '"';
}

template <typename T>
void ReprFieldItem(std::ostream &o, const T *j)
{
    ReprItem(o, j->GetTypeItem());
    o << " ";
    ReprItem(o, j->GetClassItem());
    o << "." << j->GetNameItem()->GetData();
}

// CC-OFFNXT(G.FUN.01, huge_method) big switch case
void ReprItem(std::ostream &o, const panda_file::BaseItem *i)
{
    if (i == nullptr) {
        o << "<null>";
        return;
    }
    switch (i->GetItemType()) {
        case panda_file::ItemTypes::FOREIGN_CLASS_ITEM: {
            auto j = static_cast<const panda_file::ForeignClassItem *>(i);
            DemangleName(o, j->GetNameItemData());
            break;
        }
        case panda_file::ItemTypes::CLASS_ITEM: {
            auto j = static_cast<const panda_file::ClassItem *>(i);
            DemangleName(o, j->GetNameItemData());
            break;
        }
        case panda_file::ItemTypes::FOREIGN_FIELD_ITEM: {
            auto j = static_cast<const panda_file::ForeignFieldItem *>(i);
            ReprFieldItem(o, j);
            break;
        }
        case panda_file::ItemTypes::FIELD_ITEM: {
            auto j = static_cast<const panda_file::FieldItem *>(i);
            ReprFieldItem(o, j);
            break;
        }
        case panda_file::ItemTypes::PRIMITIVE_TYPE_ITEM: {
            auto j = static_cast<const panda_file::PrimitiveTypeItem *>(i);
            o << j->GetType();
            break;
        }
        case panda_file::ItemTypes::FOREIGN_METHOD_ITEM: {
            auto j = static_cast<const panda_file::ForeignMethodItem *>(i);
            ReprMethod(o, j->GetNameItem(), j->GetClassItem(), j->GetProto());
            break;
        }
        case panda_file::ItemTypes::METHOD_ITEM: {
            auto j = static_cast<const panda_file::MethodItem *>(i);
            ReprMethod(o, j->GetNameItem(), j->GetClassItem(), j->GetProto());
            break;
        }
        case panda_file::ItemTypes::ANNOTATION_ITEM: {
            ReprAnnotationItem(o, i);
            break;
        }
        case panda_file::ItemTypes::VALUE_ITEM: {
            ReprValueItem(o, i);
            break;
        }
        case panda_file::ItemTypes::STRING_ITEM: {
            ReprStringItem(o, i);
            break;
        }
        default:
            o << "<unknown:" << i->GetOffset() << ">";
    }
}
}  // namespace

Context::Context(Config conf) : conf_(std::move(conf)) {}

Context::~Context() = default;

void Context::ReleasePreWriteState()
{
    result_.stats.itemsCount = knownItems_.size();
    result_.stats.classCount = cont_.GetClassMap()->size();

    // Drop input-owned buffers before the final write, but leave hash maps to Context destruction.
    // Releasing thousands of hash nodes here adds allocator churn and does not materially lower peak RSS.
    preWriteStateReleased_ = true;
    deferredFailedAnnotations_.clear();
    ClearAndReleaseStorage(codeDatas_);
    patcher_.Clear();
    readers_.clear();
}

void Context::Write(const std::string &out)
{
    auto writer = panda_file::FileWriter(out);
    if (!writer) {
        Error("Can't write", {ErrorDetail("file", out)});
        return;
    }
    if (!cont_.Write(&writer, true, false,
                     panda_file::ItemContainer::RegionSectionMode::REUSE_REQUIRED_UNCHANGED_DEPENDENCIES)) {
        Error("Can't write", {ErrorDetail("file", out)});
        return;
    }

    if (result_.stats.itemsCount == 0) {
        result_.stats.itemsCount = knownItems_.size();
    }
    result_.stats.classCount = cont_.GetClassMap()->size();
}

void Context::Read(const std::vector<std::string> &input)
{
    std::vector<ReaderEntry> readers;
    readers.reserve(input.size());

    for (const auto &i : input) {
        auto rd = panda_file::File::Open(i);
        if (rd == nullptr) {
            Error("Can't open file", {ErrorDetail("location", i)});
            break;
        }
        auto reader = &readers_.emplace_front(std::move(rd));
        readers.emplace_back(&i, reader);
    }

    if (HasErrors() || readers.empty()) {
        return;
    }

    auto hardwareThreads = std::max(1U, std::thread::hardware_concurrency());
    auto threadCount = std::min(readers.size(), static_cast<size_t>(hardwareThreads));
    std::atomic<size_t> nextReader {0};
    std::vector<uint8_t> readResults(readers.size(), 0);
    ReadWorkerData workerData {&readers, &readResults, &nextReader};
    if (threadCount <= 1) {
        ReadContainers(&workerData);
    } else {
        std::vector<std::thread> threads;
        threads.reserve(threadCount);
        for (size_t i = 0; i < threadCount; i++) {
            threads.emplace_back(ReadContainers, &workerData);
        }
        for (auto &thread : threads) {
            thread.join();
        }
    }

    for (size_t i = 0; i < readers.size(); i++) {
        if (readResults[i] != 0) {
            continue;
        }
        Error("can't read container", {ErrorDetail("location", *readers[i].first)}, readers[i].second);
        break;
    }

    ASSERT(input.size() == readers.size());
}

void Context::ErrorToStringWrapper::PrintInfo(std::ostream &o, const std::string &info) const
{
    if (!info.empty()) {
        o << ": " << info;
    }
}

void Context::ErrorToStringWrapper::PrintInfo(std::ostream &o, const panda_file::BaseItem *info) const
{
    o << ": ";
    ReprItem(o, info);
    for (const auto &[item, reader] : ctx_->cameFrom_) {
        if (item != info) {
            continue;
        }
        o << "\n";
        std::fill_n(std::ostream_iterator<char>(o), indent_ + 1, '\t');
        o << "declared at `" << reader->GetFilePtr()->GetFilename() << "`";
    }
}

void Context::ErrorToStringWrapper::Print(std::ostream &o) const
{
    std::fill_n(std::ostream_iterator<char>(o), indent_, '\t');
    o << error_.GetName();
    std::visit([this, &o](const auto &info) { PrintInfo(o, info); }, error_.GetInfo());
}

std::ostream &operator<<(std::ostream &o, const static_linker::Context::ErrorToStringWrapper &self)
{
    self.Print(o);
    return o;
}

void Context::Error(const std::string &msg, const std::vector<ErrorDetail> &details,
                    const panda_file::FileReader *reader)
{
    auto o = std::stringstream();
    o << msg;

    for (const auto &d : details) {
        o << "\n" << ErrorToString(d, 1);
    }
    if (reader != nullptr) {
        o << "\n\tat `" << reader->GetFilePtr()->GetFilename() << "`";
    }

    result_.errors.emplace_back(o.str());
}

}  // namespace ark::static_linker
