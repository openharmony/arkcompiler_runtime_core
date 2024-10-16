/**
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLFORMATTERSCACHE_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLFORMATTERSCACHE_H

#include "libpandabase/macros.h"
#include "libpandabase/os/mutex.h"
#include "unicode/numberformatter.h"
#include "unicode/numberrangeformatter.h"
#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/unistr.h"

#include <string>
#include <unordered_map>
#include <memory>
#include <iterator>
#include <cstdlib>

namespace ark::ets::stdlib {

/// Contains information about cache with LocalizedNumberFormatter and LocalizedNumberRangeFormatter
class IntlFormattersCache final {
public:
    NO_COPY_SEMANTIC(IntlFormattersCache);
    NO_MOVE_SEMANTIC(IntlFormattersCache);

    IntlFormattersCache();
    ~IntlFormattersCache() = default;

    icu::number::LocalizedNumberFormatter &NumFmtsCacheInvalidation(const std::string &locTag, const icu::Locale &loc);

    icu::number::LocalizedNumberRangeFormatter &NumRangeFmtsCacheInvalidation(const std::string &locTag,
                                                                              const icu::Locale &loc);

private:
    using LocNumFmt = icu::number::LocalizedNumberFormatter;
    using LocNumRangeFmt = icu::number::LocalizedNumberRangeFormatter;

    struct NumberFormatters {
        std::unique_ptr<LocNumFmt> numFmt;
        std::unique_ptr<LocNumRangeFmt> numRangeFmt;
    };

    using CacheUMap = std::unordered_map<std::string, NumberFormatters>;
    using CacheUMapIterator = CacheUMap::iterator;

    os::memory::RecursiveMutex mtx_;
    CacheUMap cache_ GUARDED_BY(mtx_);
    static constexpr uint32_t MAX_SIZE_CACHE = 512U;
    static constexpr double ERASE_RATIO = 0.1;  // Erase 10% of MAX_SIZE_CACHE

    void EraseRandFmtsGroupByEraseRatio();
};

}  // namespace ark::ets::stdlib

#endif  //  PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLFORMATTERSCACHE_H
