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
/**
 * NEWARR with a union element type that mixes a USER interface with std.core
 * types. The instruction id becomes a canonical union array string of the
 * form `{U<user>,std.core.X,...}[]`, which is registered in
 * program.recordTable but typically absent from file->nameToClass. Before
 * the inst_modifier fix, ModifyInstClass would prematurely return when the
 * full union-array string is missed in nameToClass, leaving the NEWARR id
 * stale. After obfuscation the recordTable key has been rewritten via the
 * component classes' refreshed names, so the original id no longer matches
 * any entry in classItems and AsmEmitter::AddBytecodeIndexDependencies
 * crashes on UNREACHABLE() while resolving the type ref.
 */

interface BaseI100 {
    id: int;
}

class A implements BaseI100 {
    public id: int = 1;
}

function main(): void {
    let arr: (BaseI100 | Date | string | null)[] = [new A()];
    // CC-OFFNXT(G.AOD.03) for test purposes
    console.log('OK', arr.length);
}
