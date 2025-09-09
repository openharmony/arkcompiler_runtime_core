/**
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "abckit_wrapper/base_module.h"
#include "logger.h"
#include "abckit_wrapper/module.h"
#include "abckit_wrapper/namespace.h"
#include "abckit_wrapper/class.h"

AbckitWrapperErrorCode abckit_wrapper::BaseModule::Init()
{
    auto rc = this->InitNamespaces();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Namespaces:" << rc;
        return rc;
    }

    rc = this->InitClasses();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Classes:" << rc;
        return rc;
    }

    rc = this->InitInterfaces();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Interfaces:" << rc;
        return rc;
    }

    rc = this->InitEnums();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to init Enums:" << rc;
        return rc;
    }

    return ERR_SUCCESS;
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitNamespaces()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreNamespace : object.GetNamespaces()) {
                auto ns = std::make_unique<Namespace>(coreNamespace);
                this->InitForObject(ns.get());
                LOG_I << "Found Namespace:" << ns->GetFullyQualifiedName();
                auto rc = ns->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Namespace:" << coreNamespace.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(ns->GetFullyQualifiedName(), ns.get());
                ns->owningModule_.value()->Add(std::move(ns));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitClasses()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreClass : object.GetClasses()) {
                auto clazz = std::make_unique<Class>(coreClass);
                this->InitForObject(clazz.get());
                LOG_I << "Found Class:" << clazz->GetFullyQualifiedName();
                auto rc = clazz->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Clazz:" << coreClass.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(clazz->GetFullyQualifiedName(), clazz.get());
                clazz->owningModule_.value()->Add(std::move(clazz));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitInterfaces()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreInterface : object.GetInterfaces()) {
                auto clazz = std::make_unique<Class>(coreInterface);
                this->InitForObject(clazz.get());
                LOG_I << "Found Interface:" << clazz->GetFullyQualifiedName();
                auto rc = clazz->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Interface:" << coreInterface.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(clazz->GetFullyQualifiedName(), clazz.get());
                clazz->owningModule_.value()->Add(std::move(clazz));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

AbckitWrapperErrorCode abckit_wrapper::BaseModule::InitEnums()
{
    return std::visit(
        [&](const auto &object) {
            for (auto &coreEnum : object.GetEnums()) {
                auto clazz = std::make_unique<Class>(coreEnum);
                this->InitForObject(clazz.get());
                LOG_I << "Found Enum:" << clazz->GetFullyQualifiedName();
                auto rc = clazz->Init();
                if (ABCKIT_WRAPPER_ERROR(rc)) {
                    LOG_E << "Failed to init Enum:" << coreEnum.GetName() << " errCode:" << rc;
                    return rc;
                }
                this->children_.emplace(clazz->GetFullyQualifiedName(), clazz.get());
                clazz->owningModule_.value()->Add(std::move(clazz));
            }

            return ERR_SUCCESS;
        },
        this->impl_);
}

bool abckit_wrapper::BaseModule::IsExternal()
{
    return std::visit([](const auto &object) { return object.IsExternal(); }, this->impl_);
}

std::string abckit_wrapper::BaseModule::GetName() const
{
    return this->GetObjectName();
}
