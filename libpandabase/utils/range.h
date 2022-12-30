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

#ifndef LIBPANDABASE_UTILS_RANGE_H
#define LIBPANDABASE_UTILS_RANGE_H

namespace panda {

template <class It>
class Range {
public:
    Range(It begin, It end) : begin_(begin), end_(end) {}

    It begin()
    {
        return begin_;
    }

    It end()
    {
        return end_;
    }

    size_t size() const
    {
        return std::distance(begin_, end_);
    }

private:
    It begin_;
    It end_;
};

}  // namespace panda

#endif  // LIBPANDABASE_UTILS_RANGE_H
