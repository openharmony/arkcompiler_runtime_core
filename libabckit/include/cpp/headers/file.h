/*
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

#ifndef CPP_ABCKIT_FILE_H
#define CPP_ABCKIT_FILE_H

#include "libabckit/include/c/abckit.h"
#include "libabckit/include/c/isa/isa_dynamic.h"
#include "libabckit/src/include_v2/c/isa/isa_static.h"
#include "libabckit/include/cpp/headers/base_classes.h"
#include "libabckit/include/cpp/headers/config.h"
#include "libabckit/include/cpp/headers/declarations.h"
#include "libabckit/include/cpp/headers/basic_block.h"
#include "libabckit/include/cpp/headers/instruction.h"
#include "libabckit/include/cpp/headers/dynamic_isa.h"
#include "libabckit/include/cpp/headers/value.h"
#include "libabckit/include/cpp/headers/literal.h"
#include "libabckit/include/cpp/headers/literal_array.h"
#include "libabckit/src/include_v2/cpp/headers/static_isa.h"

#include "libabckit/include/cpp/headers/core/module.h"
#include "libabckit/include/c/metadata_core.h"

#include <memory>
#include <vector>

namespace abckit {

class File final : public Resource<AbckitFile *> {
private:
    class FileDeleter final : public IResourceDeleter {
    public:
        FileDeleter(const ApiConfig *conf, const File &file) : conf_(conf), deleterFile_(file) {};
        FileDeleter(const FileDeleter &other) = delete;
        FileDeleter &operator=(const FileDeleter &other) = delete;
        FileDeleter(FileDeleter &&other) = delete;
        FileDeleter &operator=(FileDeleter &&other) = delete;
        ~FileDeleter() override = default;

        void DeleteResource() override
        {
            conf_->cApi_->closeFile(deleterFile_.GetResource());
        }

    private:
        const ApiConfig *conf_;
        const File &deleterFile_;
    };

public:
    explicit File(const char *path) : File(std::string(path)) {};
    explicit File(const std::string &path) : File(path, std::make_unique<DefaultErrorHandler>()) {}
    File(const File &file) = delete;
    File &operator=(const File &file) = delete;
    File(File &&file) = delete;
    File &operator=(File &&file) = delete;
    File(const std::string &path, std::unique_ptr<IErrorHandler> eh)
        : Resource(AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)->openAbc(path.c_str())), conf_(std::move(eh))
    {
        CheckError(&conf_);
        SetDeleter(std::make_unique<FileDeleter>(&conf_, *this));
    }

    ~File() override = default;

    abckit::Value CreateValueU1(bool val)
    {
        AbckitValue *value = GetApiConfig()->cMapi_->createValueU1(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Value(value, GetApiConfig());
    }

    abckit::Value CreateValueDouble(double val)
    {
        AbckitValue *value = GetApiConfig()->cMapi_->createValueDouble(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Value(value, GetApiConfig());
    }

    abckit::Literal CreateLiteralBool(bool val)
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralBool(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig());
    }

    abckit::Literal CreateLiteralDouble(double val)
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralDouble(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig());
    }

    abckit::Literal CreateLiteralU32(double val)
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralU32(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig());
    }

    abckit::Literal CreateLiteralLiteralArray(const abckit::LiteralArray &val)
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralLiteralArray(GetResource(), val.GetView());
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig());
    }

    abckit::LiteralArray CreateLiteralArray(const std::vector<abckit::Literal> &literals) const;

    void WriteAbc(const std::string &path)
    {
        GetApiConfig()->cApi_->writeAbc(GetResource(), path.c_str());
        CheckError(GetApiConfig());
    }

    // Returns a vector of core::Module's in a particular file
    std::vector<core::Module> GetModules() const;
    // Returns all functions in a file (consider to delete for mainenace reasons)
    std::vector<core::Function> GetAllFunctions() const;

    // Other API.

protected:
    const ApiConfig *GetApiConfig() const override
    {
        return &conf_;
    }

private:
    inline void GetAllFunctionsInner(std::vector<core::Function> &functions) const
    {
        for (auto &module : GetModules()) {
            for (auto &klass : module.GetClasses()) {
                for (auto &function : klass.GetAllMethods()) {
                    functions.push_back(function);
                }
            }
            for (auto &function : module.GetTopLevelFunctions()) {
                functions.push_back(function);
            }
        }
    }

    inline void GetModulesInner(std::vector<core::Module> &modules) const
    {
        const ApiConfig *conf = GetApiConfig();

        using EnumerateData = std::pair<std::vector<core::Module> *, const ApiConfig *>;
        EnumerateData enumerateData(&modules, conf);

        conf->cIapi_->fileEnumerateModules(GetResource(), (void *)&enumerateData,
                                           [](AbckitCoreModule *module, void *data) {
                                               auto *vec = static_cast<EnumerateData *>(data)->first;
                                               auto *config = static_cast<EnumerateData *>(data)->second;
                                               vec->push_back(core::Module(module, config));
                                               return true;
                                           });
    }

    const ApiConfig conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_FILE_H
