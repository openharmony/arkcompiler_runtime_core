/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef ARK_GUARD_NAME_CACHE_NAME_CACHE_PARSER_H
#define ARK_GUARD_NAME_CACHE_NAME_CACHE_PARSER_H

#include "name_cache_constants.h"

#include "libarkbase/utils/json_parser.h"

namespace ark::guard {

/**
 * @brief NameCacheParser
 */
class NameCacheParser {
public:
    /**
     * @brief constructor, initializes applyNameCache
     * @param applyNameCache apply name cache
     */
    explicit NameCacheParser(std::string applyNameCache) : applyNameCache_(std::move(applyNameCache)) {}

    /**
     * @brief read and parse name cache
     * read apply_name_cache first, if the content is empty, then read default_name_cache
     */
    void Parse();

    /**
     * @brief get name caches
     * @return name cache
     */
    const ObjectNameCacheTable &GetNameCache();

private:
    void ParseNameCache(const std::string &content);

    void ParseObjectNameCache(const JsonObject *object, const std::string &objectName,
                              ObjectNameCacheTable &objectCaches);

    std::string applyNameCache_;

    ObjectNameCacheTable moduleCaches_;
};
}  // namespace ark::guard

#endif  // ARK_GUARD_NAME_CACHE_NAME_CACHE_PARSER_H
