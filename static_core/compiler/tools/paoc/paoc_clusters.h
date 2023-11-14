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

#ifndef COMPILER_TOOLS_PAOC_PAOC_CLUSTERS_H
#define COMPILER_TOOLS_PAOC_PAOC_CLUSTERS_H

#include <unordered_map>
#include <string_view>
#include <string>

#include "utils/json_parser.h"
#include "utils/pandargs.h"
#include "utils/logger.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_PAOC_CLUSTERS(level) LOG(level, COMPILER) << "PAOC_CLUSTERS: "

namespace panda {

/**
 * Implements `--paoc-clusters` option.
 * Stores clusters themselves  and "function:cluster" relations
 */
class PaocClusters {
    using JsonObjPointer = JsonObject::JsonObjPointer;
    using StringT = JsonObject::StringT;
    using ArrayT = JsonObject::ArrayT;
    using NumT = JsonObject::NumT;
    using Key = JsonObject::Key;

public:
    static constexpr std::string_view CLUSTERS_MAP_OBJ_NAME {"clusters_map"};  // Contains `method-clusters` relations
    static constexpr std::string_view CLUSTERS_OBJ_NAME {"compiler_options"};  // Contains `cluster-options` relations
    class OptionsCluster;

    const std::vector<OptionsCluster *> *Find(const std::string &method)
    {
        auto iter = special_options_.find(method);
        if (iter == special_options_.end()) {
            return nullptr;
        }
        LOG_PAOC_CLUSTERS(INFO) << "Found clusters for method `" << method << "`";
        return &iter->second;
    }

    void Init(const JsonObject &obj, PandArgParser *pa_parser)
    {
        ASSERT(pa_parser != nullptr);
        InitClusters(obj, pa_parser);
        InitClustersMap(obj);
    }

    void InitClusters(const JsonObject &main_obj, PandArgParser *pa_parser)
    {
        if (main_obj.GetValue<JsonObjPointer>(Key(CLUSTERS_OBJ_NAME)) == nullptr) {
            LOG_PAOC_CLUSTERS(FATAL) << "Can't find `" << CLUSTERS_OBJ_NAME << "` object";
        }
        const auto clusters_json = main_obj.GetValue<JsonObjPointer>(Key(CLUSTERS_OBJ_NAME))->get();

        // Fill clusters_ in order of presence in JSON-obj:
        size_t n_clusters = clusters_json->GetSize();
        for (size_t idx = 0; idx < n_clusters; idx++) {
            const auto cluster_json = clusters_json->GetValue<JsonObjPointer>(idx)->get();
            if (cluster_json == nullptr) {
                LOG_PAOC_CLUSTERS(FATAL) << "Can't find a cluster (idx = " << idx << ")";
            }
            clusters_.emplace_back(clusters_json->GetKeyByIndex(idx));

            // Fill current cluster:
            const auto &options_json = cluster_json->GetUnorderedMap();
            // Add option-value pair:
            for (const auto &p : options_json) {
                const auto &option_name = p.first;
                const auto *option_value = cluster_json->GetValueSourceString(option_name);
                if (option_value == nullptr || option_value->empty()) {
                    LOG_PAOC_CLUSTERS(FATAL)
                        << "Can't find option's value (cluster `" << clusters_json->GetKeyByIndex(idx) << "`, option `"
                        << option_name << "`)";
                }

                auto *option = pa_parser->GetPandArg(option_name);
                if (option == nullptr) {
                    LOG_PAOC_CLUSTERS(FATAL) << "Unknown option: `" << option_name << "`";
                }
                auto value = OptionsCluster::ParseOptionValue(*option_value, option, pa_parser);
                clusters_.back().GetVector().emplace_back(option, std::move(value));
            }
        }
    }

    void InitClustersMap(const JsonObject &main_obj)
    {
        if (main_obj.GetValue<JsonObjPointer>(Key(CLUSTERS_MAP_OBJ_NAME)) == nullptr) {
            LOG_PAOC_CLUSTERS(FATAL) << "Can't find `" << CLUSTERS_MAP_OBJ_NAME << "` object";
        }
        const auto clusters_map_json = main_obj.GetValue<JsonObjPointer>(Key(CLUSTERS_MAP_OBJ_NAME))->get();

        // Fill special_options_:
        for (const auto &p : clusters_map_json->GetUnorderedMap()) {
            const auto &method_name = p.first;
            const auto &clusters_array = p.second;
            const auto *cur_clusters_json = clusters_array.Get<ArrayT>();

            if (cur_clusters_json == nullptr) {
                LOG_PAOC_CLUSTERS(FATAL) << "Can't get clusters array for method `" << method_name << "`";
            }
            auto &cur_clusters = special_options_.try_emplace(method_name).first->second;
            for (const auto &idx : *cur_clusters_json) {
                // Cluster may be referenced by integer number (cluster's order) or string (cluster's name):
                const auto *num_idx = idx.Get<NumT>();
                const auto *str_idx = idx.Get<StringT>();
                size_t cluster_idx = 0;
                if (num_idx != nullptr) {
                    cluster_idx = static_cast<size_t>(*num_idx);
                } else if (str_idx != nullptr) {
                    const auto clusters_json = main_obj.GetValue<JsonObjPointer>(Key(CLUSTERS_OBJ_NAME))->get();
                    cluster_idx = static_cast<size_t>(clusters_json->GetIndexByKey(*str_idx));
                    ASSERT(cluster_idx != static_cast<size_t>(-1));
                } else {
                    LOG_PAOC_CLUSTERS(FATAL) << "Incorrect reference to a cluster for `" << method_name << "`";
                    UNREACHABLE();
                }
                if (cluster_idx >= clusters_.size()) {
                    LOG_PAOC_CLUSTERS(FATAL) << "Cluster's index out of range for `" << method_name << "`";
                }
                cur_clusters.push_back(&clusters_[cluster_idx]);
                ASSERT(cur_clusters.back() != nullptr);
            }
        }
    }

    /**
     * Implements a cluster and contains info related to it.
     * (i.e. vector of "option:alternative_value" pairs)
     */
    class OptionsCluster {
    public:
        // Variant of possible PandArg types:
        using ValueVariant = std::variant<std::string, int, double, bool, arg_list_t, uint32_t, uint64_t>;

        // NOLINTNEXTLINE(modernize-pass-by-value)
        explicit OptionsCluster(const std::string &cluster_name) : cluster_name_(cluster_name) {}

        void Apply()
        {
            for (auto &pair_option_value : cluster_) {
                SwapValue(pair_option_value.first, &pair_option_value.second);
            }
        }
        void Restore()
        {
            for (auto &pair_option_value : cluster_) {
                SwapValue(pair_option_value.first, &pair_option_value.second);
            }
        }

        static ValueVariant ParseOptionValue(const std::string_view &value_string, PandArgBase *option,
                                             PandArgParser *pa_parser)
        {
            switch (option->GetType()) {
                case PandArgType::STRING:
                    return ParseOptionValue<std::string>(value_string, option, pa_parser);

                case PandArgType::INTEGER:
                    return ParseOptionValue<int>(value_string, option, pa_parser);

                case PandArgType::DOUBLE:
                    return ParseOptionValue<double>(value_string, option, pa_parser);

                case PandArgType::BOOL:
                    return ParseOptionValue<bool>(value_string, option, pa_parser);

                case PandArgType::LIST:
                    return ParseOptionValue<arg_list_t>(value_string, option, pa_parser);

                case PandArgType::UINT32:
                    return ParseOptionValue<uint32_t>(value_string, option, pa_parser);

                case PandArgType::UINT64:
                    return ParseOptionValue<uint64_t>(value_string, option, pa_parser);

                case PandArgType::NOTYPE:
                default:
                    UNREACHABLE();
            }
        }

        std::vector<std::pair<PandArgBase *const, ValueVariant>> &GetVector()
        {
            return cluster_;
        }

    private:
        template <typename T>
        static ValueVariant ParseOptionValue(const std::string_view &value_string, PandArgBase *option_base,
                                             PandArgParser *pa_parser)
        {
            ASSERT(option_base != nullptr);
            auto option = static_cast<PandArg<T> *>(option_base);
            auto option_copy = *option;
            pa_parser->ParseSingleArg(&option_copy, value_string);
            return option_copy.GetValue();
        }

        void SwapValue(PandArgBase *option, ValueVariant *value)
        {
            PandArgType type_id {static_cast<uint8_t>(value->index())};
            switch (type_id) {
                case PandArgType::STRING:
                    SwapValue<std::string>(option, value);
                    return;
                case PandArgType::INTEGER:
                    SwapValue<int>(option, value);
                    return;
                case PandArgType::DOUBLE:
                    SwapValue<double>(option, value);
                    return;
                case PandArgType::BOOL:
                    SwapValue<bool>(option, value);
                    return;
                case PandArgType::LIST:
                    SwapValue<arg_list_t>(option, value);
                    return;
                case PandArgType::UINT32:
                    SwapValue<uint32_t>(option, value);
                    return;
                case PandArgType::UINT64:
                    SwapValue<uint64_t>(option, value);
                    return;
                case PandArgType::NOTYPE:
                default:
                    UNREACHABLE();
            }
        }

        template <typename T>
        void SwapValue(PandArgBase *option_base, ValueVariant *value)
        {
            auto *option = static_cast<PandArg<T> *>(option_base);
            T temp(option->GetValue());
            ASSERT(std::holds_alternative<T>(*value));
            option->template SetValue<false>(*std::get_if<T>(value));
            *value = std::move(temp);
        }

    private:
        std::string cluster_name_;
        std::vector<std::pair<PandArgBase *const, ValueVariant>> cluster_;
    };

    /**
     * Searches for a cluster for the specified method in @param clusters_info and applies it to the compiler.
     * On destruction, restores previously applied options.
     */
    class ScopedApplySpecialOptions {
    public:
        NO_COPY_SEMANTIC(ScopedApplySpecialOptions);
        NO_MOVE_SEMANTIC(ScopedApplySpecialOptions);

        explicit ScopedApplySpecialOptions(const std::string &method, PaocClusters *clusters_info)
        {
            // Find clusters for required method:
            current_clusters_ = clusters_info->Find(method);
            if (current_clusters_ == nullptr) {
                return;
            }
            // Apply clusters:
            for (auto cluster : *current_clusters_) {
                cluster->Apply();
            }
        }
        ~ScopedApplySpecialOptions()
        {
            if (current_clusters_ != nullptr) {
                for (auto cluster : *current_clusters_) {
                    cluster->Restore();
                }
            }
        }

    private:
        const std::vector<OptionsCluster *> *current_clusters_ {nullptr};
    };

private:
    std::vector<OptionsCluster> clusters_;
    std::unordered_map<std::string, std::vector<OptionsCluster *>> special_options_;
};

#undef LOG_PAOC_CLUSTERS

}  // namespace panda

#endif  // COMPILER_TOOLS_PAOC_CLUSTERS_H
