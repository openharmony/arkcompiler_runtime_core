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

#include "trace_dumper.h"

namespace panda::tooling::sampler {

void TraceDumper::DumpTraces(const SampleInfo &sample, size_t count)
{
    std::ofstream &stream = ResolveStream(sample);

    for (size_t i = sample.stack_info.managed_stack_size; i-- > 0;) {
        uintptr_t pf_id = sample.stack_info.managed_stack[i].panda_file_ptr;
        uint64_t file_id = sample.stack_info.managed_stack[i].file_id;

        std::string full_method_name;
        if (pf_id == helpers::ToUnderlying(FrameKind::BRIDGE)) {
            full_method_name = "System_Frame";
        } else {
            const panda_file::File *pf = nullptr;
            auto it = modules_map_->find(pf_id);
            if (it != modules_map_->end()) {
                pf = it->second.get();
            }

            full_method_name = ResolveName(pf, file_id);
        }
        stream << full_method_name << "; ";
    }
    stream << count << "\n";
}

/* static */
void TraceDumper::WriteThreadId(std::ofstream &stream, uint32_t thread_id)
{
    stream << "thread_id = " << thread_id << "; ";
}

/* static */
void TraceDumper::WriteThreadStatus(std::ofstream &stream, SampleInfo::ThreadStatus thread_status)
{
    stream << "status = ";

    switch (thread_status) {
        case SampleInfo::ThreadStatus::RUNNING: {
            stream << "active; ";
            break;
        }
        case SampleInfo::ThreadStatus::SUSPENDED: {
            stream << "suspended; ";
            break;
        }
        default: {
            UNREACHABLE();
        }
    }
}

std::string TraceDumper::ResolveName(const panda_file::File *pf, uint64_t file_id) const
{
    if (pf == nullptr) {
        return std::string("__unknown_module::" + std::to_string(file_id));
    }

    auto it = methods_map_->find(pf);
    if (it != methods_map_->end()) {
        return it->second.at(file_id);
    }

    return pf->GetFilename() + "::__unknown_" + std::to_string(file_id);
}

/* override */
std::ofstream &SingleCSVDumper::ResolveStream(const SampleInfo &sample)
{
    if (option_ == DumpType::THREAD_SEPARATION_BY_TID) {
        WriteThreadId(stream_, sample.thread_info.thread_id);
    }
    if (build_cold_graph_) {
        WriteThreadStatus(stream_, sample.thread_info.thread_status);
    }
    return stream_;
}

/* override */
std::ofstream &MultipleCSVDumper::ResolveStream(const SampleInfo &sample)
{
    auto it = thread_id_map_.find(sample.thread_info.thread_id);
    if (it == thread_id_map_.end()) {
        std::string filename_with_thread_id = AddThreadIdToFilename(filename_, sample.thread_info.thread_id);

        auto return_pair =
            thread_id_map_.insert({sample.thread_info.thread_id, std::ofstream(filename_with_thread_id)});
        it = return_pair.first;

        auto is_success_insert = return_pair.second;
        if (!is_success_insert) {
            LOG(FATAL, PROFILER) << "Failed while insert in unordored_map";
        }
    }

    WriteThreadId(it->second, sample.thread_info.thread_id);

    if (build_cold_graph_) {
        WriteThreadStatus(it->second, sample.thread_info.thread_status);
    }

    return it->second;
}

/* static */
std::string MultipleCSVDumper::AddThreadIdToFilename(const std::string &filename, uint32_t thread_id)
{
    std::string filename_with_thread_id(filename);

    std::size_t pos = filename_with_thread_id.find("csv");
    if (pos == std::string::npos) {
        LOG(FATAL, PROFILER) << "Incorrect output filename, *.csv format expected";
    }

    filename_with_thread_id.insert(pos - 1, std::to_string(thread_id));

    return filename_with_thread_id;
}

}  // namespace panda::tooling::sampler
