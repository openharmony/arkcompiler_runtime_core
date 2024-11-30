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

#ifndef CPP_ABCKIT_CORE_ANNOTATION_IMPL_H
#define CPP_ABCKIT_CORE_ANNOTATION_IMPL_H

#include "./annotation.h"
#include "./annotation_element.h"
#include "./annotation_interface.h"

namespace abckit::core {

inline void Annotation::EnumerateElements(const std::function<bool(abckit::core::AnnotationElement)> &cb) const
{
    using PayloadT = Payload<const std::function<bool(core::AnnotationElement)> &>;
    PayloadT payload {cb, GetApiConfig(), GetResource()};

    GetApiConfig()->cIapi_->annotationEnumerateElements(
        GetView(), &payload, [](AbckitCoreAnnotationElement *el, void *data) -> bool {
            const auto &payload = *static_cast<PayloadT *>(data);
            return payload.data(core::AnnotationElement(el, payload.config, payload.resource));
        });
    CheckError(GetApiConfig());
}

inline core::AnnotationInterface Annotation::GetInterface() const
{
    AnnotationInterface iface(GetApiConfig()->cIapi_->annotationGetInterface(GetView()), GetApiConfig(), GetResource());
    CheckError(GetApiConfig());
    return iface;
}

}  // namespace abckit::core

#endif  // CPP_ABCKIT_CORE_ANNOTATION_IMPL_H
