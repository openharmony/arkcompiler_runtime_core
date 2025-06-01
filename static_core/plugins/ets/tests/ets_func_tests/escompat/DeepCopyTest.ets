/*
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

abstract class TagBase {
	val: string

	constructor(val: string) {
		this.val = val
	}
}

class ObjTag extends TagBase {
	constructor(val: string = "") {
		super(val)
	}
}

class Obj {
	id: int
	name: string | undefined

	next: Obj | undefined

	tag: ObjTag | undefined

	b: boolean = true
	bt: byte = 0
	c: char = c'a'
	s: short = 0
	l: long = 0
	f: float = 0.0
	d: double = 0.0

	constructor(id: int = 0, name: string | undefined = undefined) {
		this.id = id
		this.name = name
	}
}

class RefCycle {
	ref: RefCycle | undefined
}

class FnBox {
	fn: (() => boolean) | undefined = undefined
}

function testObjectClone(): void {
	const obj = new Obj(1, "foo")
	obj.tag = new ObjTag("object")

	const objCopy = deepcopy(obj)
	assertTrue(obj !== objCopy, "cloned object is the same as source object")
	assertTrue(objCopy != null, "cloned object is null")

	assertEQ(obj.id,  objCopy!.id, "cloned object 'id' field unexpected value")
	assertEQ(obj.name,  objCopy!.name, "cloned object 'name' field unexpected value")

	assertTrue(objCopy!.next === undefined, "cloned object 'next' field unexpected value")
	assertTrue(obj.tag !== objCopy!.tag, "object tag is not cloned")
}

function testObjectShallowCopy(): void {
	const obj = new Obj(2, "bar")
	obj.tag = new ObjTag("shallow")

	const objCopy = deepcopy(obj, 1)
	assertTrue(obj !== objCopy, "cloned object is the same as source object")
	assertTrue(objCopy != null, "cloned object is null")

	assertTrue(obj.tag === objCopy!.tag, "object tag is cloned also")
}

function testObjectNoCopy(): void {
	const obj = new Obj(3, "spam")
	obj.tag = new ObjTag("no copy")

	const objCopy = deepcopy(obj, 0)
	assertTrue(obj === objCopy, "returned object should be same as the source object")
}

function testObjectCloneDepth2(): void {
	const obj = new Obj(4, "eggs")
	obj.tag = new ObjTag("depth 2")

	const next = new Obj(5, "nest")
	next.tag = new  ObjTag("nested")

	obj.next = next

	const objCopy = deepcopy(obj, 2)

	assertTrue(objCopy != null, "cloned object is null")
	assertTrue(obj !== objCopy, "cloned object is the same as source")

	assertTrue(obj.next !== objCopy!.next, "one level nested object is the same as source")
	assertTrue(obj.next!.tag === objCopy!.next!.tag, "2 level nested objects should not be cloned")
}

function testObjectRefCycleClone(): void {
	const ref = new RefCycle()
	ref.ref = ref

	const refCopy = deepcopy(ref)

	assertTrue(ref === ref.ref, "not a reference cycle")
	assertTrue(ref !== refCopy, "source object returned")
	assertTrue(refCopy != null, "null returned")

	assertTrue(refCopy === refCopy!.ref, "cloned object has broken reference cycle")
}

function testBuiltinArrayClone(): void {
	const obj = new Obj()
	const objs: Obj[] = [obj, obj]

	const objsCopy = deepcopy(objs)
	assertTrue(objsCopy != null, "cloned array is null")

	assertEQ(objs.length,  objsCopy!.length, "cloned array invalid length")
	assertTrue(objs !== objsCopy!, "source array returned")
	assertTrue(objs[0] !== objsCopy![0], "array elements are not cloned")
	assertTrue(objsCopy![0] === objsCopy![1], "cloned array elements are different")
}

function testArrayClone(): void {
	const objs = new Array<Nullish<Obj>>()
	objs.push(new Obj())
	objs.push(null)
	objs.push(undefined)

	const objsCopy = deepcopy(objs)
	assertTrue(objsCopy != null, "cloned array is null")

	assertEQ(objs.length,  objsCopy!.length, "cloned array length is not the as source length")
	assertTrue(objs !== objsCopy, "cloned array is the same as source array")
	assertTrue(objs[0] !== objsCopy![0], "array elements are not cloned")
	assertTrue(objs[1] === null, "null element copied incorrectly")
	assertTrue(objs[2] === undefined, "undefined element copied incorrectly")
}

function testObjectWithLambdaClone(): void {
	const fnBox = new FnBox()
	fnBox.fn = () => true

	const fnBoxCopy = deepcopy(fnBox)
	assertTrue(fnBoxCopy != null, "cloned object is null")

	assertTrue(fnBox !== fnBoxCopy, "cloned object is the same as source object")
	assertTrue(fnBox.fn === fnBoxCopy!.fn, "lambda field is cloned")
	assertEQ(fnBox.fn!(),  fnBoxCopy!.fn!(), "not the same results")
}

function testSetClone(): void {
	const set = new Set<Obj>()

	const obj = new Obj()
	set.add(obj)

	const setCopy = deepcopy(set)
	assertTrue(setCopy != null, "cloned set is null")

	assertTrue(set !== setCopy, "cloned set is the same as source set")
	assertEQ(set.size,  setCopy!.size, "cloned set size is not the same as source set size")
	assertFalse(setCopy!.has(obj), "cloned set contains original element")
}

function testMapClone(): void {
	const map = new Map<string, Nullish<number>>()

	const key = "1.0"
	map.set(key, 1.0)
	map.set("null", null)
	map.set("undefined", undefined)

	const mapCopy = deepcopy(map)
	assertTrue(mapCopy != null, "cloned map is null")

	assertTrue(map != mapCopy, "cloned map is the same as source map")
	assertEQ(map.size,  mapCopy!.size, "cloned map size != source map size")
	assertTrue(mapCopy!.has(key), "existing key not found in cloned map")

	assertTrue(mapCopy!.get("null") === null, "'null' key has wrong value")
	assertTrue(mapCopy!.get("undefined") === undefined, "'undefined' key has wrong value")
}

function testDateClone(): void {
	const date = new Date()

	const dateCopy = deepcopy(date)
	assertTrue(dateCopy != null, "cloned date is null")

	assertTrue(date != dateCopy, "cloned date is the same as source date")
	assertEQ(date.valueOf(),  dateCopy!.valueOf(), "cloned date value is not equal to source date value")
}

function testWeakRefClone(): void {
	const obj = new Obj(6, "ref")
	const ref = new WeakRef<Obj>(obj)

	expectError(() => { 
		const refCopy = deepcopy(ref) 
	}, "class std.core.WeakRef doesn't have default constructor")
}

async function fn(): Promise<string> {
	return "foo"
}

function testPromiseClone(): void {
	const promise: Promise<string> = fn()

	expectError(() => {
		const promiseCopy = deepcopy(promise)
	}, "class std.core.Promise doesn't have default constructor")
}

function main(): int {
	const suite = new ArkTestsuite("deepcopy() API")

	suite.addTest("object deep copy", testObjectClone)
	suite.addTest("object shallow copy", testObjectShallowCopy)
	suite.addTest("object deep copy depth == 0", testObjectNoCopy)
	suite.addTest("object deep copy depth == 2", testObjectCloneDepth2)
	suite.addTest("object with ref cycle copy", testObjectRefCycleClone)
	suite.addTest("builtin array deep copy", testBuiltinArrayClone)
	suite.addTest("stdlib Array deep copy", testArrayClone)
	suite.addTest("object with lambda field", testObjectWithLambdaClone)
	suite.addTest("stdlib Set deep copy", testSetClone)
	suite.addTest("stdlib Map deep copy", testMapClone)
	suite.addTest("stdlib Date deep copy", testDateClone)
	suite.addTest("WeakRef clone", testWeakRefClone)
	suite.addTest("Promise clone", testPromiseClone)

	return suite.run()
}