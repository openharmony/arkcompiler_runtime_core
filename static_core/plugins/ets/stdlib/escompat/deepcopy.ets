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

package escompat;

export function deepcopy<T>(src: Nullish<T>, depth: int = -1): Nullish<T> {
	const maxDepth = (depth == -1) ? Int.MAX_VALUE : depth
	return new DeepCloner(maxDepth).clone<T>(src)
}

class DeepCloner {
	private maxDepth: int
	private currentDepth: int = 0

	private static readonly OBJ_ENTRY_NAME: int = 0
	private static readonly OBJ_ENTRY_VALUE: int = 1

	private copies = new Map<Object, Object>()

	constructor(maxDepth: int) {
		this.maxDepth = maxDepth
	}

	clone<T>(src: Nullish<T>): Nullish<T> {
		if (src === null) {
			return null
		} else if (src === undefined) {
			return undefined
		} else if (src instanceof String) {
			return src as T
		} else if (src instanceof Boolean) {
			return Boolean.valueOf(src) as T
		} else if (src instanceof Byte) {
			return Byte.valueOf(src) as T
		} else if (src instanceof Char) {
			return Char.valueOf(src) as T
		} else if (src instanceof Short) {
			return Short.valueOf(src) as T
		} else if (src instanceof Int) {
			return Int.valueOf(src) as T
		} else if (src instanceof Long) {
			return Long.valueOf(src) as T
		} else if (src instanceof Float) {
			return Float.valueOf(src) as T
		} else if (src instanceof Double) {
			return Double.valueOf(src) as T
		} else {
			const existingCopy = this.copies.get(src as Object)
			if (existingCopy !== undefined) {
				return existingCopy as T
			}

			const srcType = Type.of(src)
			if (srcType instanceof ClassType) {
				if (src instanceof Array) {
					return this.cloneArray(src) as T
				} else if (src instanceof Set) {
					return this.cloneSet(src) as T
				} else if (src instanceof Map) {
					return this.cloneMap(src) as T
				} else {
					return this.cloneObject(src as Object, srcType) as T
				}
			} else if (srcType instanceof ArrayType) {
				return this.cloneBuiltinArray(src as Object, srcType as ArrayType) as T
			} else if (srcType instanceof LambdaType) {
				return src
			} else {
				throw new Error("unsupported source object type: " + srcType)
			}
		}
	}

	private hasEmptyConstructor(type: ClassType): boolean {
		const ctorsNum = type.getConstructorsNum()
		for (let ctorIdx = 0; ctorIdx < ctorsNum; ctorIdx++) {
			const ctor = type.getConstructor(ctorIdx)
			if (ctor.getType().getParametersNum() == 0 && ctor.getAccessModifier() == AccessModifier.PUBLIC) {
				return true
			}
		}
		return false
	}

	private cloneObject(obj: Object, objType: ClassType): Object {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return obj
		}

		if (!this.hasEmptyConstructor(objType)) {
			throw new Error(`class ${objType.getName()} doesn't have default constructor`)
		}

		const objCopy = objType.make()
		this.copies.set(obj, objCopy)

		const objCopyValue = Value.of(objCopy) as ClassValue

		const objEntries = Object.entries(obj)
		for (const objEntry of objEntries) {
			const fieldName = objEntry![DeepCloner.OBJ_ENTRY_NAME]
			const field = objType.getFieldByName(fieldName)

			const objFieldValueCopy = this.clone<Object>(objEntry![DeepCloner.OBJ_ENTRY_VALUE])
			if (field.getType().assignableFrom(Type.of(objFieldValueCopy))) {
				objCopyValue.setFieldByName(fieldName, Value.of(objFieldValueCopy))
			}
		}

		this.currentDepth--

		return objCopy
	}

	private cloneBuiltinArray(array: Object, arrayType: ArrayType): Object {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return array
		}

		const arrayValue = Value.of(array) as ArrayValue
		const arrayLength: int = arrayValue.getLength() as int

		const arrayCopy = arrayType.make(arrayLength)
		this.copies.set(array, arrayCopy)

		const arrayCopyValue = Value.of(arrayCopy) as ArrayValue
		for (let i = 0; i < arrayLength; i++) {
			const arrayElementValue = arrayValue.getElement(i)

			const arrayElementCopy = this.clone<Object>(arrayElementValue.getData())
			arrayCopyValue.setElement(i, Value.of(arrayElementCopy))
		}

		this.currentDepth--

		return arrayCopy
	}

	private cloneArray<E>(array: Array<E>): Array<E> {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return array
		}

		const arrayCopy = new Array<E>()
		this.copies.set(array, arrayCopy)

		array.forEach((elem: E) => {
		 	const elemCopy = this.clone<E>(elem)
		 	arrayCopy.push(elemCopy as E)
		})

		this.currentDepth--

		return arrayCopy
	}

	private cloneSet<E>(set: Set<E>): Set<E> {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return set
		}

		const setCopy = new Set<E>()
		this.copies.set(set, setCopy)

		set.forEach((elem: E) => {
			const elemCopy = this.clone<E>(elem)
			setCopy.add(elemCopy as E)
		})

		this.currentDepth--

		return setCopy
	}

	private cloneMap<K, V>(map: Map<K, V>): Map<K, V> {
		this.currentDepth++
		if (this.currentDepth > this.maxDepth) {
			return map
		}

		const mapCopy = new Map<K, V>()
		this.copies.set(map, mapCopy)

		map.forEach((val: V, key: K) => {
			const keyCopy: K = this.clone<K>(key) as K
			const valCopy: V = this.clone<V>(val) as V

			mapCopy.set(keyCopy, valCopy)
		})

		this.currentDepth--

		return mapCopy
	}
}