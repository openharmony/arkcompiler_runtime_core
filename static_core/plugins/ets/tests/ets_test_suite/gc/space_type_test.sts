/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

function NonMovableTest(): void throws {
    assert GC.getObjectSpaceType("spaceTypeTest") == GC.NON_MOVABLE_SPACE : "Must be non movable space";
}

function AllocateAndMoveTest(): void throws {
    let obj: Object = new Object();
    assert GC.getObjectSpaceType(obj) == GC.YOUNG_SPACE : "Must be young space";

    let gc_id = GC.startGC(GC.YOUNG_CAUSE);
    GC.waitForFinishGC(gc_id);

    assert GC.getObjectSpaceType(obj) == GC.TENURED_SPACE : "Must be tenured space";
}

function HumongousObjectTest(): void throws
{
    let h_obj = new long[1024 * 1024];
    assert GC.getObjectSpaceType(h_obj) == GC.HUMONGOUS_SPACE: "Must be humongous space";
}

function nullObjectTest(): void {
    let detected = false;
    try {
        GC.getObjectSpaceType(null);
    } catch (e: NullPointerError) {
        detected = true;
    }
    catch (e) {}
    assert detected == true: "getObjectSpaceType must throw NullPointerError for null argument";
}

/**
 * Tests for getObjectSpaceType builtin.
 */
function main(): int {
    try {
        AllocateAndMoveTest();
        NonMovableTest();
        HumongousObjectTest();
    }
    catch (e) {
        assert false: "Space allocation test must not throw exceptions";
    }
    nullObjectTest();
    return 0;
}
