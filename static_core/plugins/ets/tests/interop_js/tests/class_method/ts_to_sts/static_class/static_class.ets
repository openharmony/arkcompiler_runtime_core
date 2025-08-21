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

let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/class_method/ts_to_sts/main');
let StaticClass = module.getProperty('StaticClass');
let createStaticClassFromTs = module.getProperty('createStaticClassFromTs');

function checkStaticClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (StaticClass.isUndefined() || StaticClass.isNull()) {
            return false;
        }

        const TSClass = StaticClass;

        const getMethod = TSClass.getPropertySafe('get');
        if (getMethod.isUndefined()) {
            return false;
        }

        const compareMethod = TSClass.getPropertySafe('compare');
        if (compareMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', getMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean();
        }

        return false;

    } catch (e) {
        return false;
    }
}

function checkCreateStaticClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createStaticClassFromTs.isUndefined() || createStaticClassFromTs.isNull()) {
            return false;
        }

        const TSClass = createStaticClassFromTs.invoke();

        if (TSClass.isUndefined() || TSClass.isNull()) {
            return false;
        }

        const getMethod = TSClass.getPropertySafe('get');
        if (getMethod.isUndefined()) {
            return false;
        }

        const compareMethod = TSClass.getPropertySafe('compare');
        if (compareMethod.isUndefined()) {
            return false;
        }

        const compareResult = TSClass.invokeMethod('compare', getMethod);

        if (compareResult.isBoolean()) {
            return compareResult.toBoolean();
        }

        return false;

    } catch (e) {
        return false;
    }
}