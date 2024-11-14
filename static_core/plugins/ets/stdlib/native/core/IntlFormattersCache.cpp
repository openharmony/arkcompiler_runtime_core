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

#include "plugins/ets/stdlib/native/core/IntlFormattersCache.h"

namespace ark::ets::stdlib {

IntlFormattersCache::IntlFormattersCache()
{
    // NOLINTNEXTLINE(cert-msc51-cpp)
    std::srand(time(nullptr));
    ASSERT(MAX_SIZE_CACHE > 0U);
    ASSERT(ERASE_RATIO > 0);
    ASSERT(ERASE_RATIO < 1);
}

icu::number::LocalizedNumberFormatter &IntlFormattersCache::NumFmtsCacheInvalidation(const std::string &locTag,
                                                                                     const icu::Locale &loc)
{
    CacheUMapIterator it;
    {
        os::memory::LockHolder lh(mtx_);
        it = cache_.find(locTag);
        if (it == cache_.end()) {
            EraseRandFmtsGroupByEraseRatio();
            // Create new number formatter, number range formatter is empty
            // Number range formatter will be created via call RangeIntlFormattersCacheInvalidation
            auto *ptr = new icu::number::LocalizedNumberFormatter(icu::number::NumberFormatter::withLocale(loc));
            NumberFormatters f;
            f.numFmt.reset(ptr);
            auto [iter, isNumFmtInserted] = cache_.insert_or_assign(locTag, std::move(f));
            ASSERT(isNumFmtInserted);
            it = iter;
        } else if (it->second.numFmt == nullptr) {
            // Create new number formatter, range formatter is not changed
            auto *ptr = new icu::number::LocalizedNumberFormatter(icu::number::NumberFormatter::withLocale(loc));
            it->second.numFmt.reset(ptr);
        }
    }
    ASSERT(it->second.numFmt != nullptr);
    return *(it->second.numFmt);
}

icu::number::LocalizedNumberRangeFormatter &IntlFormattersCache::NumRangeFmtsCacheInvalidation(
    const std::string &locTag, const icu::Locale &loc)
{
    CacheUMapIterator it;
    {
        os::memory::LockHolder lh(mtx_);
        it = cache_.find(locTag);
        if (it == cache_.end()) {
            EraseRandFmtsGroupByEraseRatio();
            // Create new number range formatter, number formatter is empty
            // Number formatter will be created via call IntlFormattersCacheInvalidation
            auto *ptr =
                new icu::number::LocalizedNumberRangeFormatter(icu::number::NumberRangeFormatter::withLocale(loc));
            NumberFormatters f;
            f.numRangeFmt.reset(ptr);
            auto [iter, isNumRangeFmtInserted] = cache_.insert_or_assign(locTag, std::move(f));
            ASSERT(isNumRangeFmtInserted);
            it = iter;
        } else if (it->second.numRangeFmt == nullptr) {
            // Create new number range formatter, number formatter is not changed
            auto *ptr =
                new icu::number::LocalizedNumberRangeFormatter(icu::number::NumberRangeFormatter::withLocale(loc));
            it->second.numRangeFmt.reset(ptr);
        }
    }
    ASSERT(it->second.numRangeFmt != nullptr);
    return *(it->second.numRangeFmt);
}

void IntlFormattersCache::EraseRandFmtsGroupByEraseRatio()
{
    os::memory::LockHolder lh(mtx_);
    // Remove random N "neighbours" items (group) from cache_ if size is maximum
    if (cache_.size() == MAX_SIZE_CACHE) {
        constexpr auto ERASE_AMOUNT = std::max(1U, static_cast<uint32_t>(MAX_SIZE_CACHE * ERASE_RATIO));
        // NOLINTNEXTLINE(cert-msc50-cpp)
        static auto delta = static_cast<uint32_t>(std::rand() % (MAX_SIZE_CACHE - ERASE_AMOUNT + 1U));
        // delta is in range [0; MAX_SIZE_CACHE - ERASE_AMOUNT]
        auto firstIt = cache_.begin();
        std::advance(firstIt, delta);
        auto lastIt = firstIt;
        std::advance(lastIt, ERASE_AMOUNT);
        cache_.erase(firstIt, lastIt);
    }
}

}  // namespace ark::ets::stdlib
