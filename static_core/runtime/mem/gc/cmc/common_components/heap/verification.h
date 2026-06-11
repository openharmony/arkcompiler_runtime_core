/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VERIFICATION_H
#define VERIFICATION_H

namespace ark::common_vm {

class RegionalHeap;

class WVerify {
public:
    static void VerifyAfterMark(bool isWorldStopped);
    static void VerifyAfterForward(bool isWorldStopped);
    static void VerifyAfterFix(bool isWorldStopped);
    static void EnableReadBarrierDFX(bool isWorldStopped);
    static void DisableReadBarrierDFX(bool isWorldStopped);

private:
    static void VerifyAfterMarkInternal(RegionalHeap &space);
    static void VerifyAfterForwardInternal(RegionalHeap &space);
    static void VerifyAfterFixInternal(RegionalHeap &space);
    static void EnableReadBarrierDFXInternal(RegionalHeap &space);
    static void DisableReadBarrierDFXInternal(RegionalHeap &space);
};

}  // namespace ark::common_vm

#endif  // VERIFICATION_H
