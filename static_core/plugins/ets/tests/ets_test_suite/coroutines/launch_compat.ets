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

class Payload {
    public value: string;

    constructor(value: string) {
        this.value = value;
    }
}

type ThreeArgsFun = (a0: string, a1: int, a2: Payload) => string;
type FourArgsFun = (a0: string, a1: int, a2: Payload, a3: boolean) => string;
type FiveArgsFun = (a0: int, a1: int, a2: int, a3: int, a4: int) => int;
type SixteenArgsFun = (
    a0: int, a1: int, a2: int, a3: int,
    a4: int, a5: int, a6: int, a7: int,
    a8: int, a9: int, a10: int, a11: int,
    a12: int, a13: int, a14: int, a15: int
) => int;

function noArg(): string {
    return 'noArg';
}

function oneArg(a0: string): string {
    return a0;
}

function twoArgs(a0: string, a1: int): string {
    return a0 + ':' + a1;
}

function threeArgs(a0: string, a1: int, a2: Payload): string {
    return a0 + ':' + a1 + ':' + a2.value;
}

function fourArgs(a0: string, a1: int, a2: Payload, a3: boolean): string {
    return a0 + ':' + a1 + ':' + a2.value + ':' + a3;
}

function fiveArgs(a0: int, a1: int, a2: int, a3: int, a4: int): int {
    return a0 + a1 + a2 + a3 + a4;
}

function sum4(a0: int, a1: int, a2: int, a3: int): int {
    return a0 + a1 + a2 + a3;
}

function getWorkerId(): int {
    return CoroutineExtras.getWorkerId();
}

function sixteenArgs(
    a0: int, a1: int, a2: int, a3: int,
    a4: int, a5: int, a6: int, a7: int,
    a8: int, a9: int, a10: int, a11: int,
    a12: int, a13: int, a14: int, a15: int
): int {
    return a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14 + a15;
}

function testLaunchWithOldArgsSyntax(): void {
    let noArgJob: Job<string> = launch<string, () => string>(noArg);
    let oneArgJob: Job<string> = launch<string, (a0: string) => string>(oneArg, 'one');
    let twoArgsJob: Job<string> = launch<string, (a0: string, a1: int) => string>(twoArgs, 'two', 2);
    let threeArgsJob: Job<string> = launch<string, ThreeArgsFun>(
        threeArgs,
        'three',
        3,
        new Payload('payload')
    );
    let fourArgsJob: Job<string> = launch<string, FourArgsFun>(
        fourArgs,
        'four',
        4,
        new Payload('payload'),
        true
    );
    let fiveArgsJob = launch<int, FiveArgsFun>(fiveArgs, 1, 2, 3, 4, 5);
    let sixteenArgsJob = launch<int, SixteenArgsFun>(
        sixteenArgs,
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15
    );

    arktest.assertEQ(noArgJob.Await(), 'noArg');
    arktest.assertEQ(oneArgJob.Await(), 'one');
    arktest.assertEQ(twoArgsJob.Await(), 'two:2');
    arktest.assertEQ(threeArgsJob.Await(), 'three:3:payload');
    arktest.assertEQ(fourArgsJob.Await(), 'four:4:payload:true');
    arktest.assertEQ(fiveArgsJob.Await(), 15);
    arktest.assertEQ(sixteenArgsJob.Await(), 120);
}

function testLaunchWithOldParamsSyntax(): void {
    let launchParams = new LaunchParams();
    let noArgJob: Job<string> = launch<string, () => string>(noArg, launchParams);
    let oneArgJob: Job<string> = launch<string, (a0: string) => string>(oneArg, launchParams, 'one');
    let twoArgsJob: Job<string> = launch<string, (a0: string, a1: int) => string>(twoArgs, launchParams, 'two', 2);
    let threeArgsJob: Job<string> = launch<string, ThreeArgsFun>(
        threeArgs,
        launchParams,
        'three',
        3,
        new Payload('payload')
    );
    let fourArgsJob: Job<string> = launch<string, FourArgsFun>(
        fourArgs,
        launchParams,
        'four',
        4,
        new Payload('payload'),
        false
    );
    let fiveArgsJob = launch<int, FiveArgsFun>(fiveArgs, launchParams, 1, 2, 3, 4, 5);
    let sixteenArgsJob = launch<int, SixteenArgsFun>(
        sixteenArgs,
        launchParams,
        0, 1, 2, 3,
        4, 5, 6, 7,
        8, 9, 10, 11,
        12, 13, 14, 15
    );

    arktest.assertEQ(noArgJob.Await(), 'noArg');
    arktest.assertEQ(oneArgJob.Await(), 'one');
    arktest.assertEQ(twoArgsJob.Await(), 'two:2');
    arktest.assertEQ(threeArgsJob.Await(), 'three:3:payload');
    arktest.assertEQ(fourArgsJob.Await(), 'four:4:payload:false');
    arktest.assertEQ(fiveArgsJob.Await(), 15);
    arktest.assertEQ(sixteenArgsJob.Await(), 120);
}

function testLaunchWithOldParamsSyntaxResolution(): void {
    const mainWorkerId: int = getWorkerId();
    const launchParams: LaunchParams = {
        abortFlag: false,
        workerGroupId: WorkerGroup.generateGroupId(WorkerDomain.MAIN)
    };

    let argsJob: Job<int> = launch<int, (a0: int, a1: int, a2: int, a3: int) => int>(
        sum4,
        launchParams,
        1, 2, 3, 4
    );
    let paramsJob: Job<int> = launch<int>(getWorkerId, launchParams);

    arktest.assertEQ(argsJob.Await(), 10);
    arktest.assertEQ(paramsJob.Await(), mainWorkerId);
}

function main(): int {
    let testSuite = new arktest.ArkTestsuite('coroutines.launch_compat');
    testSuite.addTest('testLaunchWithOldArgsSyntax', testLaunchWithOldArgsSyntax);
    testSuite.addTest('testLaunchWithOldParamsSyntax', testLaunchWithOldParamsSyntax);
    testSuite.addTest('testLaunchWithOldParamsSyntaxResolution', testLaunchWithOldParamsSyntaxResolution);
    return testSuite.run();
}
