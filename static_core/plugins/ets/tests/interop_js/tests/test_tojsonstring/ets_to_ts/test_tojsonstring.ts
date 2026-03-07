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

const etsVm = globalThis.gtest.etsVm;
const STValue = etsVm.STValue;

// Get SimpleClass and related functions
const SimpleClass = etsVm.getClass('Ltest_tojsonstring/SimpleClass;');
const serializeSimpleClass = etsVm.getFunction('Ltest_tojsonstring/ETSGLOBAL;', 'serializeSimpleClass');

// Get ClassWithArray
const ClassWithArray = etsVm.getClass('Ltest_tojsonstring/ClassWithArray;');
const serializeClassWithArray = etsVm.getFunction('Ltest_tojsonstring/ETSGLOBAL;', 'serializeClassWithArray');

// Get ClassWithNullFields
const ClassWithNullFields = etsVm.getClass('Ltest_tojsonstring/ClassWithNullFields;');
const serializeClassWithNullFields = etsVm.getFunction('Ltest_tojsonstring/ETSGLOBAL;', 'serializeClassWithNullFields');

// Get EmptyClass
const EmptyClass = etsVm.getClass('Ltest_tojsonstring/EmptyClass;');
const serializeEmptyClass = etsVm.getFunction('Ltest_tojsonstring/ETSGLOBAL;', 'serializeEmptyClass');

// Get ClassWithSpecialChars
const ClassWithSpecialChars = etsVm.getClass('Ltest_tojsonstring/ClassWithSpecialChars;');
const serializeClassWithSpecialChars = etsVm.getFunction('Ltest_tojsonstring/ETSGLOBAL;', 'serializeClassWithSpecialChars');

// Get ClassWithNumberEdgeCases
const ClassWithNumberEdgeCases = etsVm.getClass('Ltest_tojsonstring/ClassWithNumberEdgeCases;');
const serializeClassWithNumberEdgeCases = etsVm.getFunction('Ltest_tojsonstring/ETSGLOBAL;', 'serializeClassWithNumberEdgeCases');

// Get complex nested classes
const Task = etsVm.getClass('Ltest_tojsonstring/Task;');
const Project = etsVm.getClass('Ltest_tojsonstring/Project;');
const serializeProject = etsVm.getFunction('Ltest_tojsonstring/ETSGLOBAL;', 'serializeProject');

function testSimpleClass(): void {
    let obj = new SimpleClass();
    obj.name = 'Alice';
    obj.age = 30;
    obj.active = false;

    let jsonResult = STValue.toJSON(obj);
    let expected = serializeSimpleClass(obj);

    ASSERT_TRUE(jsonResult === expected);
}

function testClassWithArray(): void {
    let obj = new ClassWithArray();
    obj.id = 1;
    let arrItems = STValue.newSTArray();
    arrItems.push(10);
    arrItems.push(20);
    arrItems.push(30);
    obj.items = arrItems;
    let arrTags = STValue.newSTArray();
    arrTags.push('tag1');
    arrTags.push('tag2');
    arrTags.push('tag3');
    obj.tags = arrTags;

    let jsonResult = STValue.toJSON(obj);
    let expected = serializeClassWithArray(obj);

    ASSERT_TRUE(jsonResult === expected);
}

function testClassWithNullFields(): void {
    let obj = new ClassWithNullFields();
    obj.name = 'Test';
    obj.value = null;
    obj.flag = true;

    let jsonResult = STValue.toJSON(obj);
    let expected = serializeClassWithNullFields(obj);

    ASSERT_TRUE(jsonResult === expected);
}

function testEmptyClass(): void {
    let obj = new EmptyClass();

    let jsonResult = STValue.toJSON(obj);
    let expected = serializeEmptyClass(obj);

    ASSERT_TRUE(jsonResult === expected);
}

function testClassWithSpecialChars(): void {
    let obj = new ClassWithSpecialChars();
    obj.quote = 'He said \'Hello\'';
    obj.newLine = 'Line1\nLine2';
    obj.unicode = '中文测试';

    let jsonResult = STValue.toJSON(obj);
    let expected = serializeClassWithSpecialChars(obj);

    ASSERT_TRUE(jsonResult === expected);
}

function testClassWithNumberEdgeCases(): void {
    let obj = new ClassWithNumberEdgeCases();
    obj.zero = 0; 
    obj.negative = -999;
    obj.num = 3.14159;
    obj.large = 1000000;

    let jsonResult = STValue.toJSON(obj);
    let expected = serializeClassWithNumberEdgeCases(obj);

    ASSERT_TRUE(jsonResult === expected);
}

function testComplexNestedObject(): void {
    let project = new Project();
    project.name = 'E-Commerce Platform';
    project.budget = 500000;
    project.active = true;

    let task1 = new Task();
    task1.id = 1;
    task1.title = 'Design Database Schema';
    task1.status = 'Completed';
    task1.hours = 40;

    let task2 = new Task();
    task2.id = 2;
    task2.title = 'Implement API';
    task2.status = 'In Progress';
    task2.hours = 80;

    let task3 = new Task();
    task3.id = 3;
    task3.title = 'Write Tests';
    task3.status = 'Pending';
    task3.hours = 60;

    let tasks = STValue.newSTArray();
    tasks.push(task1);
    tasks.push(task2);
    tasks.push(task3);
    project.tasks = tasks;

    let jsonResult = STValue.toJSON(project);
    let expected = serializeProject(project);

    ASSERT_TRUE(jsonResult === expected);
}

function testThrowError(): void {
    let checkRes = false;
    try {
        let jsonResult = STValue.toJSON();
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('Expect 1 args');
        checkRes = checkRes && e.message.includes('but got 0 args');
    }
    ASSERT_TRUE(checkRes);

    try {
        let jsonResult = STValue.toJSON(1, 2);
    } catch (e: Error) {
        checkRes = true;
        checkRes = checkRes && e.message.includes('Expect 1 args');
        checkRes = checkRes && e.message.includes('but got 2 args');
    }
    ASSERT_TRUE(checkRes);
}

function main(): void {
    testSimpleClass();
    testClassWithArray();
    testClassWithNullFields();
    testEmptyClass();
    testClassWithSpecialChars();
    testClassWithNumberEdgeCases();
    testComplexNestedObject();
    testThrowError();
}

main();
