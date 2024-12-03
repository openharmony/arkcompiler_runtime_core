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

#include "./base_classes.h"
#include "./value.h"
#include "./literal.h"
#include "./literal_array.h"

#include "./core/module.h"

#include <functional>
#include <memory>
#include <string_view>
#include <vector>

namespace abckit {

/**
 * @brief File
 */
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
    /**
     * @brief Constructor
     * @param path
     */
    explicit File(const char *path) : File(std::string(path)) {};

    /**
     * @brief Constructor
     * @param path
     */
    explicit File(const std::string &path) : File(path, std::make_unique<DefaultErrorHandler>()) {}

    /**
     * @brief Deleted constructor
     * @param file
     */
    File(const File &file) = delete;

    /**
     * @brief Deleted constructor
     * @param file
     * @return File
     */
    File &operator=(const File &file) = delete;

    /**
     * @brief Deleted constructor
     * @param file
     */
    File(File &&file) = delete;

    /**
     * @brief Deleted constructor
     * @param file
     * @return File
     */
    File &operator=(File &&file) = delete;

    /**
     * @brief Constructor
     * @param path
     * @param eh
     */
    File(const std::string &path, std::unique_ptr<IErrorHandler> eh)
        : Resource(AbckitGetApiImpl(ABCKIT_VERSION_RELEASE_1_0_0)->openAbc(path.c_str())), conf_(std::move(eh))
    {
        CheckError(&conf_);
        SetDeleter(std::make_unique<FileDeleter>(&conf_, *this));
    }
    /**
     * @brief Destructor
     */
    ~File() override = default;

    /**
     * @brief Creates value item `AbckitValue` containing the given boolean value `value`.
     * @param val
     * @return `Value`
     */
    abckit::Value CreateValueU1(bool val) const
    {
        AbckitValue *value = GetApiConfig()->cMapi_->createValueU1(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Value(value, GetApiConfig(), this);
    }

    /**
     * @brief Creates value item containing the given double value `value`.
     * @param val
     * @return `Value`
     */
    abckit::Value CreateValueDouble(double val) const
    {
        AbckitValue *value = GetApiConfig()->cMapi_->createValueDouble(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Value(value, GetApiConfig(), this);
    }

    /**
     * @brief Creates value item containing the given bool value `value`.
     * @param val
     * @return `Value`
     */
    abckit::Literal CreateLiteralBool(bool val) const
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralBool(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig(), this);
    }

    /**
     * @brief Creates value item containing the given bool value `value`.
     * @param val
     * @return `Value`
     */
    abckit::Literal CreateLiteralDouble(double val) const
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralDouble(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig(), this);
    }

    /**
     * @brief Creates literal containing the given double value `value`.
     * @param val
     * @return `Literal`
     */
    abckit::Literal CreateLiteralU32(double val) const
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralU32(GetResource(), val);
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig(), this);
    }

    /**
     * @brief Creates literal containing the given literal array `litarr`.
     * @param val
     * @return `Literal`
     */
    abckit::Literal CreateLiteralLiteralArray(const abckit::LiteralArray &val) const
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralLiteralArray(GetResource(), val.GetView());
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig(), this);
    }

    /**
     * @brief Creates literal array value item from the given value items array `literals`.
     * @param literals
     * @return `LiteralArray`
     */
    abckit::LiteralArray CreateLiteralArray(const std::vector<abckit::Literal> &literals) const;

    /**
     * @brief Creates string item containing the given C-style null-terminated string `val`.
     * @return `string_view`.
     * @param [ in ] val - string to be set.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `val` is empty.
     * @note Allocates
     */
    std::string_view CreateString(const std::string &val) const
    {
        const ApiConfig *conf = GetApiConfig();
        AbckitString *cString = conf->cMapi_->createString(GetResource(), val.c_str());
        CheckError(conf);
        std::string_view view = conf->cIapi_->abckitStringToString(cString);
        CheckError(conf);
        return view;
    }

    /**
     * @brief Creates literal containing the given string `val`.
     * @return `Literal`.
     * @param [ in ] val - string that will be stored in the literal to be created.
     * @note Set `ABCKIT_STATUS_BAD_ARGUMENT` error if `val` is empty.
     * @note Allocates
     */
    abckit::Literal CreateLiteralString(const std::string &val) const
    {
        AbckitLiteral *literal = GetApiConfig()->cMapi_->createLiteralString(GetResource(), val.c_str());
        CheckError(GetApiConfig());
        return abckit::Literal(literal, GetApiConfig(), this);
    }

    /**
     * @brief Writes `file` to the specified `path`.
     * @param path - path to file
     */
    void WriteAbc(const std::string &path) const
    {
        GetApiConfig()->cApi_->writeAbc(GetResource(), path.c_str());
        CheckError(GetApiConfig());
    }

    /**
     * @brief Returns a vector of core::Module's in a particular file
     * @return vector of `Module`.
     */
    std::vector<core::Module> GetModules() const;

    /**
     * @brief Returns all functions in a file (consider to delete for mainenace reasons)
     * @return vector of `Function`.
     */
    std::vector<core::Function> GetAllFunctions() const;

    /**
     * @brief EnumerateModules
     * @return bool
     * @param cb
     */
    bool EnumerateModules(const std::function<bool(core::Module)> &cb) const;

    // Other API.

protected:
    /**
     * @brief Get api config
     * @return ApiConfig
     */
    const ApiConfig *GetApiConfig() const override
    {
        return &conf_;
    }

    /**
     * @brief Struct for using in callbacks
     */
    template <typename D>
    struct Payload {
        /**
         * @brief data
         */
        D data;
        /**
         * @brief config
         */
        const ApiConfig *config;
        /**
         * @brief resource
         */
        const File *resource;
    };

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
        Payload<std::vector<core::Module> *> payload {&modules, GetApiConfig(), this};

        GetApiConfig()->cIapi_->fileEnumerateModules(GetResource(), &payload, [](AbckitCoreModule *module, void *data) {
            const auto &payload = *static_cast<Payload<std::vector<core::Module> *> *>(data);
            payload.data->push_back(core::Module(module, payload.config, payload.resource));
            return true;
        });
    }

    const ApiConfig conf_;
};

}  // namespace abckit

#endif  // CPP_ABCKIT_FILE_H
