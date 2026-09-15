/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

function main() {
    // Loading the regular plugin initializes the ArkJS N-API function table.
    requireNapiPreview('ets_interop_js_napi', true);
    const runner = requireNapiPreview('ets_ani_hybrid_runner', true);
    const result = runner.runEtsMain();
    if (result !== 20) {
        throw new Error(`ANI ETS-main test expected 20, got ${result}`);
    }
    print('PASS ANI/C++ hybrid runner');
}

main();
