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

#ifndef DPROF_CONVERTER_FEATURES_MANAGER_H
#define DPROF_CONVERTER_FEATURES_MANAGER_H

#include "dprof/storage.h"
#include "utils/logger.h"

#include <vector>
#include <unordered_map>
#include <functional>

namespace panda::dprof {
class FeaturesManager {
public:
    struct Functor {
        virtual bool operator()(const AppData &app_data, const std::vector<uint8_t> &data) = 0;
    };

    bool RegisterFeature(const std::string &feature_name, Functor &functor)
    {
        auto it = map_.find(feature_name);
        if (it != map_.end()) {
            LOG(ERROR, DPROF) << "Feature already exists, featureName=" << feature_name;
            return false;
        }
        map_.insert({feature_name, functor});
        return true;
    }

    bool UnregisterFeature(const std::string &feature_name)
    {
        if (map_.erase(feature_name) != 1) {
            LOG(ERROR, DPROF) << "Feature does not exist, featureName=" << feature_name;
            return false;
        }
        return true;
    }

    bool ProcessingFeature(const AppData &app_data, const std::string &feature_name,
                           const std::vector<uint8_t> &data) const
    {
        auto it = map_.find(feature_name);
        if (it == map_.end()) {
            LOG(ERROR, DPROF) << "Feature is not supported, featureName=" << feature_name;
            return false;
        }

        return it->second(app_data, data);
    }

    bool ProcessingFeatures(const AppData &app_data) const
    {
        for (const auto &it : app_data.GetFeaturesMap()) {
            if (!ProcessingFeature(app_data, it.first, it.second)) {
                LOG(ERROR, DPROF) << "Cannot processing feature: " << it.first << ", app: " << app_data.GetName();
                return false;
            }
        }
        return true;
    }

private:
    std::unordered_map<std::string, Functor &> map_;
};
}  // namespace panda::dprof

#endif  // DPROF_CONVERTER_FEATURES_MANAGER_H
