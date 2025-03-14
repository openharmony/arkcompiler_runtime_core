/*
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

import { Ability } from "./ability";

native function getAppAbcFiles(): string[];

function getNativeLibName(): string {
    return StdProcess.ProcessManager.getEnvironmentVar("NATIVE_LIB_NAME");
}

function getAppMainAbilityClassName(): string {
    return StdProcess.ProcessManager.getEnvironmentVar("APP_MAIN_ABILITY_CLASS");
}

function loadAndInvokeAbility(mainAbilityClassName: string): void {
    const appRuntimeLinker = new AbcRuntimeLinker(undefined, getAppAbcFiles());
    const mainAbilityClass = appRuntimeLinker.loadClass(mainAbilityClassName);
    const mainAbility = mainAbilityClass.createInstance() as Ability;
    mainAbility.invoke();
}

function main() {
    loadLibrary(getNativeLibName());

    loadAndInvokeAbility(getAppMainAbilityClassName());

    return 0;
}

function abilityExpectFailure(cb: (e: Error) => void) {
    loadLibrary(getNativeLibName())
    try {
        loadAndInvokeAbility(getAppMainAbilityClassName())
    } catch (e: Error) {
        cb(e)
        return 0
    }
    throw new Error("Error expected, but nothing was thrown")
}

function abilityExpectVerificationError(expectedMessage: string) {
    abilityExpectFailure((e: Error) => {
        assertTrue(e instanceof LinkerVerificationError)
        assertEQ(e.message, expectedMessage);
    })
}

function mainCatchLinkerUnresolvedClassError() {
    abilityExpectFailure((e: Error) => {
        assertTrue(e instanceof LinkerVerificationError)
        console.log(e.message);
    })
}

function mainCatchLinkerBadSupertypeError() {
    abilityExpectFailure((e: Error) => {
        assertTrue(e instanceof LinkerBadSupertypeError)
        console.log(e.message);
    })
}

function mainMisatchedExtendsFinal() {
    abilityExpectVerificationError("@mismatched.extends_final.ExtendsFinal")
}

function mainMisatchedOverrideFinal() {
    abilityExpectVerificationError("@app.module5.MainAbility::invoke")
}

function mainCircularityExtends() {
    abilityExpectVerificationError("@app.module6.MainAbility::invoke")
}

function mainMultipleOverrideBase() {
    abilityExpectVerificationError("@app.module.MainAbility::invoke");
}

function mainMultipleOverrideBase1() {
    abilityExpectVerificationError("@app.module1.MainAbility::invoke");
}

function mainVerificationModeOnTheFly() {
    abilityExpectVerificationError("@verification_mode.bar.ETSGLOBAL::bar");
}

function mainVerificationModeAheadOfTime() {
    abilityExpectVerificationError("L@verification_mode/bar/ETSGLOBAL;");
}

function mainMultipleImplement() {
    abilityExpectVerificationError("@app.module.MainAbility::invoke");
}

function mainMultipleImplement1() {
    abilityExpectVerificationError("@app.module1.MainAbility::invoke");
}
