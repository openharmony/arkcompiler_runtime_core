/**
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "verification/config/context/context.h"

#include "utils/logger.h"

namespace panda::verifier::debug {

void DebugConfig::AddWhitelistMethodConfig(WhitelistKind kind, const PandaString &name)
{
    whitelist_names[static_cast<size_t>(kind)].push_back(name);
    whitelist_not_empty = true;
}

bool DebugContext::InWhitelist(WhitelistKind kind, uint64_t id) const
{
    return config->whitelist_not_empty && whitelist.id[static_cast<size_t>(kind)]->count(id) > 0;
}

void DebugContext::InsertIntoWhitelist(const PandaString &name, bool is_class_name, Method::UniqId id)
{
    auto kinds_to_add = is_class_name
                            ? std::initializer_list<WhitelistKind> {WhitelistKind::CLASS}
                            : std::initializer_list<WhitelistKind> {WhitelistKind::METHOD, WhitelistKind::METHOD_CALL};

    for (auto kind : kinds_to_add) {
        auto k = static_cast<size_t>(kind);
        auto &ids = whitelist.id[k];
        auto &names = config->whitelist_names[k];
        if (std::find(names.begin(), names.end(), name) != names.end()) {
            ids->insert(id);
            LOG(DEBUG, VERIFIER) << "Method with " << (is_class_name ? "class " : "") << "name " << name << ", id 0x"
                                 << std::hex << id << " was added to whitelist";
        }
    }
}

}  // namespace panda::verifier::debug
