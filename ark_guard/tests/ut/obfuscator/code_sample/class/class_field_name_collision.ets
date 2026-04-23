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

// Regression: classA declares a static and an instance field sharing the same
// simple name. The fully-qualified key `owner.name + "." + field.name` used by
// libabckit::AbckitFile::nameToField is identical for the two fields; writing
// back the obfuscated abc previously hit UNREACHABLE() in
// AsmEmitter::AddBytecodeIndexDependencies because ModifyInstField rewrote an
// instance-field instruction operand with the renamed static field's name.
class ClassA {
    public static longField: long = 1000;
    public longField: long = 2000;

    public getInstance(): long {
        return this.longField;
    }

    public static getStatic(): long {
        return ClassA.longField;
    }
}

let a = new ClassA();
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('instance:', a.getInstance());
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('static:', ClassA.getStatic());
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('instance direct:', a.longField);
// CC-OFFNXT(G.AOD.03) for test purposes
console.log('static direct:', ClassA.longField);
