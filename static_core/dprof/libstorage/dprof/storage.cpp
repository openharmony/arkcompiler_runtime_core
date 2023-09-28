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

#include "storage.h"
#include "serializer/serializer.h"
#include "utils/logger.h"

#include <dirent.h>
#include <fstream>
#include <memory>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace panda::dprof {
/* static */
std::unique_ptr<AppData> AppData::CreateByParams(const std::string &name, uint64_t hash, uint32_t pid,
                                                 FeaturesMap &&features_map)
{
    std::unique_ptr<AppData> app_data(new AppData);

    app_data->common_info_.name = name;
    app_data->common_info_.hash = hash;
    app_data->common_info_.pid = pid;
    app_data->features_map_ = std::move(features_map);

    return app_data;
}

/* static */
std::unique_ptr<AppData> AppData::CreateByBuffer(const std::vector<uint8_t> &buffer)
{
    std::unique_ptr<AppData> app_data(new AppData);

    const uint8_t *data = buffer.data();
    size_t size = buffer.size();
    auto r = serializer::RawBufferToStruct<3>(data, size, app_data->common_info_);  // 3
    if (!r) {
        LOG(ERROR, DPROF) << "Cannot deserialize buffer to common_info. Error: " << r.Error();
        return nullptr;
    }
    ASSERT(r.Value() <= size);
    data = serializer::ToUint8tPtr(serializer::ToUintPtr(data) + r.Value());
    size -= r.Value();

    r = serializer::BufferToType(data, size, app_data->features_map_);
    if (!r) {
        LOG(ERROR, DPROF) << "Cannot deserialize features_map. Error: " << r.Error();
        return nullptr;
    }
    ASSERT(r.Value() <= size);
    size -= r.Value();
    if (size != 0) {
        LOG(ERROR, DPROF) << "Cannot deserialize all buffer, unused buffer size: " << size;
        return nullptr;
    }

    return app_data;
}

bool AppData::ToBuffer(std::vector<uint8_t> &buffer) const
{
    // 3
    if (!serializer::StructToBuffer<3>(common_info_, buffer)) {
        LOG(ERROR, DPROF) << "Cannot serialize common_info";
        return false;
    }
    auto ret = serializer::TypeToBuffer(features_map_, buffer);
    if (!ret) {
        LOG(ERROR, DPROF) << "Cannot serialize features_map. Error: " << ret.Error();
        return false;
    }
    return true;
}

/* static */
std::unique_ptr<AppDataStorage> AppDataStorage::Create(const std::string &storage_dir, bool create_dir)
{
    if (storage_dir.empty()) {
        LOG(ERROR, DPROF) << "Storage directory is not set";
        return nullptr;
    }

    struct stat stat_buffer {};
    if (::stat(storage_dir.c_str(), &stat_buffer) == 0) {
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        if (S_ISDIR(stat_buffer.st_mode)) {
            return std::unique_ptr<AppDataStorage>(new AppDataStorage(storage_dir));
        }

        LOG(ERROR, DPROF) << storage_dir << " is already exists and it is neither directory";
        return nullptr;
    }

    if (create_dir) {
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        if (::mkdir(storage_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) != 0) {
            PLOG(ERROR, DPROF) << "mkdir() failed";
            return nullptr;
        }
        return std::unique_ptr<AppDataStorage>(new AppDataStorage(storage_dir));
    }

    return nullptr;
}

bool AppDataStorage::SaveAppData(const AppData &app_data)
{
    std::vector<uint8_t> buffer;
    if (!app_data.ToBuffer(buffer)) {
        LOG(ERROR, DPROF) << "Cannot serialize AppData to buffer";
        return false;
    }

    // Save buffer to file
    std::string file_name = MakeAppPath(app_data.GetName(), app_data.GetHash(), app_data.GetPid());
    std::ofstream file(file_name, std::ios::binary);
    if (!file.is_open()) {
        LOG(ERROR, DPROF) << "Cannot open file: " << file_name;
        return false;
    }
    file.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
    if (file.bad()) {
        LOG(ERROR, DPROF) << "Cannot write AppData to file: " << file_name;
        return false;
    }

    LOG(DEBUG, DPROF) << "Save AppData to file: " << file_name;
    return true;
}

struct dirent *DoReadDir(DIR *dirp)
{
    errno = 0;
    return ::readdir(dirp);
}

void AppDataStorage::ForEachApps(const std::function<bool(std::unique_ptr<AppData> &&)> &callback) const
{
    using UniqueDir = std::unique_ptr<DIR, void (*)(DIR *)>;
    UniqueDir dir(::opendir(storage_dir_.c_str()), [](DIR *directory) {
        if (::closedir(directory) == -1) {
            PLOG(FATAL, DPROF) << "closedir() failed";
        }
    });
    if (dir.get() == nullptr) {
        PLOG(FATAL, DPROF) << "opendir() failed, dir=" << storage_dir_;
        return;
    }

    struct dirent *ent;
    while ((ent = DoReadDir(dir.get())) != nullptr) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            // Skip a valid name
            continue;
        }

        if (ent->d_type != DT_REG) {
            LOG(ERROR, DPROF) << "Not a regular file: " << ent->d_name;
            continue;
        }

        std::string path = storage_dir_ + "/" + ent->d_name;
        struct stat statbuf {};
        if (stat(path.c_str(), &statbuf) == -1) {
            PLOG(ERROR, DPROF) << "stat() failed, path=" << path;
            continue;
        }
        size_t file_size = statbuf.st_size;

        if (file_size > MAX_BUFFER_SIZE) {
            LOG(ERROR, DPROF) << "File is to large: " << path;
            continue;
        }

        // Read file
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open()) {
            LOG(ERROR, DPROF) << "Cannot open file: " << path;
            continue;
        }
        std::vector<uint8_t> buffer;
        buffer.reserve(file_size);
        buffer.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

        auto app_data = AppData::CreateByBuffer(buffer);
        if (!app_data) {
            LOG(ERROR, DPROF) << "Cannot deserialize file: " << path;
            continue;
        }

        if (!callback(std::move(app_data))) {
            break;
        }
    }
}

std::string AppDataStorage::MakeAppPath(const std::string &name, uint64_t hash, uint32_t pid) const
{
    ASSERT(!storage_dir_.empty());
    ASSERT(!name.empty());

    std::stringstream str;
    str << storage_dir_ << "/" << name << "@" << pid << "@" << hash;
    return str.str();
}
}  // namespace panda::dprof
