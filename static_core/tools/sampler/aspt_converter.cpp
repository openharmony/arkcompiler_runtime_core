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

#include "tools/sampler/aspt_converter.h"

namespace panda::tooling::sampler {

size_t AsptConverter::CollectTracesStats()
{
    stack_traces_.clear();

    size_t sample_counter = 0;
    SampleInfo sample;
    while (reader_.GetNextSample(&sample)) {
        ++sample_counter;

        if (dump_type_ == DumpType::WITHOUT_THREAD_SEPARATION) {
            // NOTE: zeroing thread_id to make samples indistinguishable in mode without thread separation
            sample.thread_info.thread_id = 0;
        }

        if (!build_cold_graph_) {
            // NOTE: zeroing thread_status to make samples indistinguishable
            // in mode without building cold flamegraph
            sample.thread_info.thread_status = SampleInfo::ThreadStatus::UNDECLARED;
        }

        auto it = stack_traces_.find(sample);
        if (it == stack_traces_.end()) {
            stack_traces_.insert({sample, 1});
            continue;
        }
        ++it->second;
    }
    return sample_counter;
}

bool AsptConverter::CollectModules()
{
    FileInfo m_info;
    while (reader_.GetNextModule(&m_info)) {
        modules_.push_back(m_info);
    }

    return !modules_.empty();
}

bool AsptConverter::BuildModulesMap()
{
    for (auto &mdl : modules_) {
        std::string filepath = mdl.pathname;

        if (substitute_directories_.has_value()) {
            SubstituteDirectories(&filepath);
        }

        if (!panda::os::IsFileExists(filepath)) {
            LOG(ERROR, PROFILER) << "Module not found, path: " << filepath;
        }

        if (modules_map_.find(mdl.ptr) == modules_map_.end()) {
            auto pf_unique = panda_file::OpenPandaFileOrZip(filepath.c_str());

            if (mdl.checksum != pf_unique->GetHeader()->checksum) {
                LOG(FATAL, PROFILER) << "Сhecksum of panda files isn't equal";
                return false;
            }

            modules_map_.insert({mdl.ptr, std::move(pf_unique)});
        }
    }

    return !modules_map_.empty();
}

void AsptConverter::SubstituteDirectories(std::string *pathname) const
{
    for (size_t i = 0; i < substitute_directories_->source.size(); ++i) {
        auto pos = pathname->find(substitute_directories_->source[i]);
        if (pos != std::string::npos) {
            pathname->replace(pos, substitute_directories_->source[i].size(), substitute_directories_->destination[i]);
            break;
        }
    }
}

bool AsptConverter::DumpResolvedTracesAsCSV(const char *filename)
{
    std::unique_ptr<TraceDumper> dumper;

    switch (dump_type_) {
        case DumpType::WITHOUT_THREAD_SEPARATION:
        case DumpType::THREAD_SEPARATION_BY_TID:
            dumper = std::make_unique<SingleCSVDumper>(filename, dump_type_, &modules_map_, &methods_map_,
                                                       build_cold_graph_);
            break;
        case DumpType::THREAD_SEPARATION_BY_CSV:
            dumper = std::make_unique<MultipleCSVDumper>(filename, &modules_map_, &methods_map_, build_cold_graph_);
            break;
        default:
            UNREACHABLE();
    }
    for (auto &[sample, count] : stack_traces_) {
        ASSERT(sample.stack_info.managed_stack_size <= SampleInfo::StackInfo::MAX_STACK_DEPTH);
        dumper->DumpTraces(sample, count);
    }
    return true;
}

void AsptConverter::BuildMethodsMap()
{
    for (const auto &pf_pair : modules_map_) {
        const panda_file::File *pf = pf_pair.second.get();
        if (pf == nullptr) {
            continue;
        }
        auto classes_span = pf->GetClasses();
        for (auto id : classes_span) {
            if (pf->IsExternal(panda_file::File::EntityId(id))) {
                continue;
            }
            panda_file::ClassDataAccessor cda(*pf, panda_file::File::EntityId(id));
            cda.EnumerateMethods([&](panda_file::MethodDataAccessor &mda) {
                std::string method_name = utf::Mutf8AsCString(mda.GetName().data);
                std::string class_name = utf::Mutf8AsCString(cda.GetDescriptor());
                if (class_name[class_name.length() - 1] == ';') {
                    class_name.pop_back();
                }
                std::string full_name = class_name + "::";
                full_name += method_name;
                methods_map_[pf][mda.GetMethodId().GetOffset()] = std::move(full_name);
            });
        }
    }
}

bool AsptConverter::DumpModulesToFile(const std::string &outname) const
{
    std::ofstream out(outname, std::ios::binary);
    if (!out) {
        LOG(ERROR, PROFILER) << "Can't open " << outname;
        out.close();
        return false;
    }

    for (auto &mdl : modules_) {
        out << mdl.checksum << " " << mdl.pathname << "\n";
    }

    if (out.fail()) {
        LOG(ERROR, PROFILER) << "Failbit or the badbit (or both) was set";
        out.close();
        return false;
    }

    return true;
}

/* static */
DumpType AsptConverter::GetDumpTypeFromOptions(const Options &cli_options)
{
    const std::string dump_type_str = cli_options.GetCsvTidSeparation();

    DumpType dump_type = DumpType::WITHOUT_THREAD_SEPARATION;
    if (dump_type_str == "single-csv-single-tid") {
        dump_type = DumpType::WITHOUT_THREAD_SEPARATION;
    } else if (dump_type_str == "single-csv-multi-tid") {
        dump_type = DumpType::THREAD_SEPARATION_BY_TID;
    } else if (dump_type_str == "multi-csv") {
        dump_type = DumpType::THREAD_SEPARATION_BY_CSV;
    } else {
        std::cerr << "unknown value of csv-tid-distribution option: '" << dump_type_str
                  << "' single-csv-multi-tid will be set" << std::endl;
        dump_type = DumpType::THREAD_SEPARATION_BY_TID;
    }

    return dump_type;
}

bool AsptConverter::RunDumpModulesMode(const std::string &outname)
{
    if (CollectTracesStats() == 0) {
        LOG(ERROR, PROFILER) << "No samples found in file";
        return false;
    }

    if (!CollectModules()) {
        LOG(ERROR, PROFILER) << "No modules found in file, names would not be resolved";
        return false;
    }

    if (!DumpModulesToFile(outname)) {
        LOG(ERROR, PROFILER) << "Can't dump modules to file";
        return false;
    }

    return true;
}

bool AsptConverter::RunDumpTracesInCsvMode(const std::string &outname)
{
    if (CollectTracesStats() == 0) {
        LOG(ERROR, PROFILER) << "No samples found in file";
        return false;
    }

    if (!CollectModules()) {
        LOG(ERROR, PROFILER) << "No modules found in file, names would not be resolved";
    }

    if (!BuildModulesMap()) {
        LOG(ERROR, PROFILER) << "Can't build modules map";
        return false;
    }

    BuildMethodsMap();
    DumpResolvedTracesAsCSV(outname.c_str());

    return true;
}

bool AsptConverter::RunWithOptions(const Options &cli_options)
{
    std::string outname = cli_options.GetOutput();

    dump_type_ = GetDumpTypeFromOptions(cli_options);
    build_cold_graph_ = cli_options.IsColdGraphEnable();

    if (cli_options.IsSubstituteModuleDir()) {
        substitute_directories_ = {cli_options.GetSubstituteSourceStr(), cli_options.GetSubstituteDestinationStr()};
        if (substitute_directories_->source.size() != substitute_directories_->destination.size()) {
            LOG(FATAL, PROFILER) << "different number of strings in substitute option";
        }
    }

    if (cli_options.IsDumpModules()) {
        return RunDumpModulesMode(outname);
    }

    return RunDumpTracesInCsvMode(outname);
}

}  // namespace panda::tooling::sampler
