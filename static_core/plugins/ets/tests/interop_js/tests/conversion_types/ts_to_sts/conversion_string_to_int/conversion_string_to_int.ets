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

let global = ESValue.getGlobal();
let gtestEnv = global.getProperty("gtest_env");
let module = ESValue.load('../../plugins/ets/tests/interop_js/tests/conversion_types/ts_to_sts/main');

let tsIntString = module.getProperty('tsIntString');

function checkConversionStringToInt(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsIntString);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionStringToNumber(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsIntString);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionStringToFloat(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsIntString);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionStringToByte(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsIntString);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionStringToShort(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsIntString);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionStringToLong(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsIntString);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionStringToDouble(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsIntString);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}

function checkConversionStringToChar(): boolean {
    try {
        const parseInt = global.getProperty('parseInt');
        const val = parseInt.invoke(tsIntString);
        return val.typeOf() == 'number';
    } catch (e) {
        return false;
    }
}