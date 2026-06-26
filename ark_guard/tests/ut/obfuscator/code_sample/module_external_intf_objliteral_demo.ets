/**
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

/**
 * ObjectLiteral implementing an interface that lives in an EXTERNAL module
 * (std.core.Intl.DateTimeFormatOptions). The OL's host is therefore not
 * reachable from this module's m->ct / m->it, so without
 * ModuleRefreshOwnedObjectLiterals the OL record name and the
 * constructor's parameter type stay anchored to the old module name after
 * obfuscation, producing a hybrid bytecode id that crashes
 * AsmEmitter::AddBytecodeIndexDependencies on INITOBJ_* (infinite recursion).
 *
 * The ObjectLiteral is created inside a user class method so that the frontend places
 * its record under the user module's recordTable (owningModule == user
 * module), matching the original crash pattern.
 */

class IntlHolder {
    public opts: Intl.DateTimeFormatOptions;
    constructor() {
        this.opts = {
            dateStyle: 'short',
        };
    }
}

function main(): void {
    let h = new IntlHolder();
    let date = new Date();
    let fmt = new Intl.DateTimeFormat('en-US', h.opts);
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('OK', fmt.format(date).length > 0);
}
