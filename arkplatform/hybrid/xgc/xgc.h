/**
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_ARKPLATFORM_MEM_XGC_H
#define PANDA_ARKPLATFORM_MEM_XGC_H

namespace arkplatform::mem {

class XGC {
public:
    static bool Create();
    static XGC *GetInstance();
    static bool Destroy();

    XGC(const XGC &) = delete;
    XGC &operator=(const XGC &) = delete;
    XGC(XGC &&) = delete;
    XGC &operator=(XGC &&) = delete;
    ~XGC() = default;

    void Trigger();

private:
    XGC() = default;
    static XGC *xgcInstance_;
};

}  // namespace arkplatform::mem

#endif  // PANDA_ARKPLATFORM_MEM_XGC_H
