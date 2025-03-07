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

const stsVm = globalThis.gtest.etsVm;

const throwErrorFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwErrorFromSts');
const throwErrorWithCauseFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwErrorWithCauseFromSts');
const throwCustomErrorFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwCustomErrorFromSts');

const throwExceptionFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwExceptionFromSts');
const throwExceptionWithCauseFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwExceptionWithCauseFromSts');
const throwCustomExceptionFromSts = stsVm.getFunction('Lexceptions/test/ETSGLOBAL;', 'throwCustomExceptionFromSts');

function catchErrorFromSts() {
    try {
        throwErrorFromSts();
    } catch (e) {
        ASSERT_TRUE(e.name === 'Error');
        ASSERT_TRUE(e.message === 'Throw Error from ets!');
    }
}

function catchErrorWithCauseFromSts() {
    try {
        throwErrorWithCauseFromSts();
    } catch (e) {
        ASSERT_TRUE(e.name === 'Error');
        ASSERT_TRUE(e.message === 'Throw Error with cause from ets!');
        ASSERT_TRUE(e.cause === 'Test cause');
    }
}

function catchCustomErrorFromSts() {
    try {
        throwCustomErrorFromSts();
    } catch (e) {
        ASSERT_TRUE(e.name === 'CustomError');
        ASSERT_TRUE(e.message === 'Throw Custom Error from ets!');
        ASSERT_TRUE(e.code === 12345);
    }
}

function catchExceptionFromSts() {
    try {
        throwExceptionFromSts();
    } catch (e) {
        ASSERT_TRUE(e.message === 'Throw Exception from ets!');
    }
}

function catchExceptionWithCauseFromSts() {
    try {
        throwExceptionWithCauseFromSts();
    } catch (e) {
        ASSERT_TRUE(e.message === 'Throw Exception with cause from ets!');
        ASSERT_TRUE(e.getCause().reason === 'Test cause');
    }
}

function catchCustomExceptionFromSts() {
    try {
        throwCustomExceptionFromSts();
    } catch (e) {
        ASSERT_TRUE(e.message === 'Throw Custom Exception from ets!');
        ASSERT_TRUE(e.code === 54321);
    }
}

catchErrorFromSts();
catchErrorWithCauseFromSts();
catchCustomErrorFromSts();

catchExceptionFromSts();
catchExceptionWithCauseFromSts();
catchCustomExceptionFromSts();
