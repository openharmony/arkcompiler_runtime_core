/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

function main(): int {
    const suite = new arktest.ArkTestsuite('Console Time API tests')
    suite.addTest('Test console timeLog with two parameters', testConsoleTimeLogTwoPara)
    suite.addTest('Test console time with Null Character', testConsoleTimeNullCharacter)
    suite.addTest('Test console timeEnd with Null Character', testConsoleTimeEndNullCharacter)
    suite.addTest('Test console timeLog with Null Character', testConsoleTimeLogNullCharacter)
    suite.addTest('Test console time with Many Null Characters', testConsoleTimeManyNullCharacters)
    suite.addTest('Test console time with Size Zero String', testConsoleTimeSizeZeroString)

    return suite.run()
}

function testConsoleTimeLogTwoPara(): void {
    arktest.expectNoThrow(() => {
        let testLog: Object[] = [];
        const str = '太好了';
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.time(str);
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.timeLog(str, testLog);
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.timeEnd(str);
    })
}

function testConsoleTimeNullCharacter(): void {
    arktest.expectError(() => {
        const str = '\0';
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.time(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.');
}

function testConsoleTimeEndNullCharacter(): void {
    arktest.expectError(() => {
        const str = '\0';
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.time(str);
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.timeEnd(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.');
}

function testConsoleTimeLogNullCharacter(): void {
    arktest.expectError(() => {
        const str = '\0';
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.time(str);
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.timeLog(str)
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.timeEnd(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.');
}

function testConsoleTimeSizeZeroString(): void {
    arktest.expectError(() => {
        let str = '';
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.time(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.')
}

function testConsoleTimeManyNullCharacters(): void {
    arktest.expectError(() => {
        let str = '\0\0';
        // CC-OFFNXT(G.AOD.03) current test suite code style
        console.time(str);
    }, 'Parameter error. The input parameters are invalid, Timer or Counter name must be not null.')
}