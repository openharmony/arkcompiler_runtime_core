
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
let ProtectedClass = module.getProperty('ProtectedClass');
let createProtectedClassFromTs = module.getProperty('createProtectedClassFromTs');
let instanceProtectedClass = module.getProperty('instanceProtectedClass');

function checkProtectedClassMethod(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (ProtectedClass.isUndefined() || ProtectedClass.isNull()) {
            return false;
        }

        const TSClass = ProtectedClass.instantiate();

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

function checkCreateProtectedClassFromTs(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (createProtectedClassFromTs.isUndefined() || createProtectedClassFromTs.isNull()) {
            return false;
        }

        const TSClass = createProtectedClassFromTs.invoke();

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

function checkInstanceProtectedClass(): boolean {
    try {
        if (module.isUndefined() || module.isNull()) {
            return false;
        }

        if (instanceProtectedClass.isUndefined() || instanceProtectedClass.isNull()) {
            return false;
        }

        const TSClass = instanceProtectedClass;

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