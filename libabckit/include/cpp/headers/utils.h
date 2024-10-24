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

#ifndef CPP_ABCKIT_UTILS_H
#define CPP_ABCKIT_UTILS_H

#include "libabckit/include/c/statuses.h"
#include "statuses_impl.h"
#include "cpp/headers/declarations.h"

#include <string>

#ifdef ABCKIT_USE_EXCEPTIONS
#include <stdexcept>
#endif

namespace abckit {

class ApiConfig;

// CC-OFFNXT(G.FUD.06) perf critical
// NOLINTBEGIN(performance-unnecessary-value-param)
inline std::string StatusToString(AbckitStatus status)
{
    switch (status) {
        case ABCKIT_STATUS_NO_ERROR:
            return "No error";
        case ABCKIT_STATUS_BAD_ARGUMENT:
            return "Bad argument";
        case ABCKIT_STATUS_MEMORY_ALLOCATION:
            return "Memory allocation error";
        case ABCKIT_STATUS_WRONG_MODE:
            return "Wrong mode";
        case ABCKIT_STATUS_WRONG_TARGET:
            return "Wrong target";
        case ABCKIT_STATUS_WRONG_LITERAL_TYPE:
            return "Wrong literal type";
        case ABCKIT_STATUS_UNSUPPORTED:
            return "Unsupported feature";
        case ABCKIT_STATUS_WRONG_CTX:
            return "Wrong context";
        case ABCKIT_STATUS_TODO:
        default:
            break;
    }
    return "Unsupported status type";
}

#ifdef ABCKIT_USE_EXCEPTIONS
class Exception : public std::runtime_error {
public:
    explicit Exception(AbckitStatus e) : std::runtime_error(StatusToString(e)) {}

    virtual const char *what() const noexcept override
    {
        return std::runtime_error::what();
    }
};
#else
class Exception {
public:
    explicit Exception(AbckitStatus e) : whatMessage_(StatusToString(e)) {}

    const char *What() const noexcept
    {
        return whatMessage_.c_str();
    }

private:
    std::string whatMessage_;
};
#endif

class IErrorHandler {
public:
    IErrorHandler() = default;
    IErrorHandler(const IErrorHandler &other) = default;
    IErrorHandler &operator=(const IErrorHandler &other) = default;
    IErrorHandler(IErrorHandler &&other) = default;
    IErrorHandler &operator=(IErrorHandler &&other) = default;
    virtual ~IErrorHandler() = default;

    virtual void HandleError(Exception &&e) = 0;
};

class DefaultErrorHandler final : public IErrorHandler {
public:
    DefaultErrorHandler() = default;
    DefaultErrorHandler(const DefaultErrorHandler &other) = default;
    DefaultErrorHandler &operator=(const DefaultErrorHandler &other) = default;
    DefaultErrorHandler(DefaultErrorHandler &&other) = default;
    DefaultErrorHandler &operator=(DefaultErrorHandler &&other) = default;
    ~DefaultErrorHandler() override = default;

    void HandleError([[maybe_unused]] Exception &&e) override
    {
// Default behaviour - do nothing.
// If at compile time exceptions are enabled - re-throw.
#ifdef ABCKIT_USE_EXCEPTIONS
        throw e;
#endif
    }
};

class IResourceDeleter {
public:
    IResourceDeleter() = default;
    IResourceDeleter(const IResourceDeleter &other) = default;
    IResourceDeleter &operator=(const IResourceDeleter &other) = default;
    IResourceDeleter(IResourceDeleter &&other) = default;
    IResourceDeleter &operator=(IResourceDeleter &&other) = default;
    virtual ~IResourceDeleter() = default;

    virtual void DeleteResource() = 0;
};

class DefaultResourceDeleter final : public IResourceDeleter {
public:
    DefaultResourceDeleter() = default;
    DefaultResourceDeleter(const DefaultResourceDeleter &other) = default;
    DefaultResourceDeleter &operator=(const DefaultResourceDeleter &other) = default;
    DefaultResourceDeleter(DefaultResourceDeleter &&other) = default;
    DefaultResourceDeleter &operator=(DefaultResourceDeleter &&other) = default;
    ~DefaultResourceDeleter() override = default;

    void DeleteResource() override
    { /* Do nothing by default. Debug log here, probably? */
    }
};

}  // namespace abckit

#endif  // CPP_ABCKIT_UTILS_H
