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
#include "abckit_wrapper/member.h"
#include "logger.h"

AbckitWrapperErrorCode abckit_wrapper::Member::RefreshSignature()
{
    if (this->cachedName_ == this->GetName()) {
        return AbckitWrapperErrorCode::ERR_SUCCESS;
    }

    const auto rc = this->InitSignature();
    if (ABCKIT_WRAPPER_ERROR(rc)) {
        LOG_E << "Failed to refresh signature:" << rc;
        return rc;
    }

    return AbckitWrapperErrorCode::ERR_SUCCESS;
}

std::string abckit_wrapper::Member::GetDescriptor()
{
    if (ABCKIT_WRAPPER_ERROR(this->RefreshSignature())) {
        return "";
    }

    return this->descriptor_;
}

std::string abckit_wrapper::Member::GetRawName()
{
    if (ABCKIT_WRAPPER_ERROR(this->RefreshSignature())) {
        return "";
    }

    return this->rawName_;
}

std::string abckit_wrapper::Member::GetSignature()
{
    if (ABCKIT_WRAPPER_ERROR(this->RefreshSignature())) {
        return "";
    }

    return this->rawName_ + this->descriptor_;
}

bool abckit_wrapper::Member::MemberAccept(MemberVisitor &visitor)
{
    return visitor.Visit(this);
}
