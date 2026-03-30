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

#include "pgo_class_context_utils.h"

#include <map>

#include "compiler/aot/aot_manager.h"

namespace ark::pgo {
namespace {
using ClassContextCollector = ark::compiler::AotClassContextCollector;

// CC-OFFNXT(G.NAM.03-CPP) project code style
constexpr char CLASS_CTX_ENTRY_DELIM = ClassContextCollector::DELIMETER;
// CC-OFFNXT(G.NAM.03-CPP) project code style
constexpr char CLASS_CTX_HASH_DELIM = ClassContextCollector::HASH_DELIMETER;

bool ParseToken(std::string_view token, std::string &path, std::string &checksum, std::string *error)
{
    // Expected token format: "<path>*<checksum>".
    size_t hash = token.rfind(CLASS_CTX_HASH_DELIM);
    if (hash == std::string_view::npos || hash == 0 || hash + 1 >= token.size()) {
        if (error != nullptr) {
            *error = "invalid class context entry '" + std::string(token) + "'";
        }
        return false;
    }
    path.assign(token.substr(0, hash));
    checksum.assign(token.substr(hash + 1));
    return true;
}

}  // namespace

bool PgoClassContextUtils::Parse(std::string_view ctx, std::map<std::string, std::string> &entries, std::string *error)
{
    entries.clear();
    if (ctx.empty()) {
        return true;
    }

    // Parse "<path>*<checksum>:..." into a map and detect conflicts.
    size_t start = 0;
    while (start < ctx.size()) {
        size_t end = ctx.find(CLASS_CTX_ENTRY_DELIM, start);
        // Ignore empty tokens (e.g. ":" or trailing ':') and treat them as no-op.
        if (end == start) {
            start = end + 1;
            continue;
        }
        auto token = (end == std::string_view::npos) ? ctx.substr(start) : ctx.substr(start, end - start);
        std::string path;
        std::string checksum;
        if (!ParseToken(token, path, checksum, error)) {
            return false;
        }
        // Insert by path, validate checksum for the same path.
        auto [it, inserted] = entries.try_emplace(path, checksum);
        if (!inserted && it->second != checksum) {
            if (error != nullptr) {
                *error = "class context checksum mismatch for '" + it->first + "'";
            }
            return false;
        }
        if (end == std::string_view::npos) {
            break;
        }
        start = end + 1;
    }
    return true;
}

bool PgoClassContextUtils::Merge(const std::vector<std::string_view> &contexts, std::string &merged, std::string *error)
{
    merged.clear();
    // Canonical ordering: output is sorted by full path.
    std::map<std::string, std::string> mergedEntries;
    for (const auto &ctx : contexts) {
        std::map<std::string, std::string> entries;
        if (!Parse(ctx, entries, error)) {
            return false;
        }
        // Merge by path, validate checksum for the same path.
        for (const auto &[path, checksum] : entries) {
            auto [it, inserted] = mergedEntries.try_emplace(path, checksum);
            // No conflict if inserted or the same checksum already exists for the path.
            if (inserted || it->second == checksum) {
                continue;
            }
            // Conflict for the same path with different checksums.
            if (error != nullptr) {
                *error = "class context checksum mismatch for '" + path + "'";
            }
            return false;
        }
    }
    // Serialize as "<path>*<checksum>:...".
    bool isFirst = true;
    for (const auto &[path, checksum] : mergedEntries) {
        if (!isFirst) {
            merged.push_back(CLASS_CTX_ENTRY_DELIM);
        }
        merged.append(path);
        merged.push_back(CLASS_CTX_HASH_DELIM);
        merged.append(checksum);
        isFirst = false;
    }
    return true;
}

}  // namespace ark::pgo
