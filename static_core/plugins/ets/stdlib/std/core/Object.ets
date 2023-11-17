/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

package std.core;

// TODO(kprokopenko): remove when undefined becomes available; function to not use before declaration
export function __undefined<T>(): T | null { return null }

// TODO(shumilov-petr): Temporary solution
type Tuple = Object[]

/**
 * `object` is an alias for type `Object`
 */
// export type object = Object;

/**
 * Common ancestor amongst all other classes
 */
export class Object {
  /**
   * Constructs a new blank Object
   */
  constructor () {};

  /**
   * Converts this object to a string
   *
   * @returns result of the conversion
   */
  public toString(): String {
    return Value.of(this).toString()
  }

  /**
   * Returns a hash code (integer representation) for this instance
   *
   * @returns representation of this instance
   */
  public hashCode(): int {
    return runtime.getHashCode(this);
  }

  /**
   * Performs default reference equality comparison
   *
   * @param to object to be checked against
   *
   * @returns true if provided object and this instance have same references, false otherwise
   */
  public equals(to: NullishType): boolean {
    return runtime.equals(this, to);
  }

  /**
   * Returns the names of the fields of an object
   *
   * @param o an object
   *
   * @returns an array of strings representing the given object's own string-keyed field keys.
   */
  public static keys(o: Object): string[] {
    // Char, Boolean and Numeric types doesn't have keys
    if (o instanceof Char ||
        o instanceof Boolean ||
        o instanceof Byte ||
        o instanceof Short ||
        o instanceof Int ||
        o instanceof Long ||
        o instanceof Float ||
        o instanceof Double) {
      return new string[0]
    }
    // "Keys" for the string type is enumeration from 0 to str.length - 1
    if (o instanceof String) {
      let sv = o as string
      let len = sv.length()
      let res = new string[len as int]
      for (let i = 0; i < len; i++) {
        // TODO(shumilov-petr): need to apply more effective way for int to String conversion
        res[i] = new Int(i).toString()
      }
      return res
    }
    let t = Type.of(o)
    if (t instanceof ClassType) {
      let ct = t as ClassType
      let fnum = ct.getFieldsNum()
      let n: int = 0
      for (let i = 0; i < fnum; i++) {
        if (!ct.getField(i).isStatic()) {
          n++
        }
      }
      let res = new string[n]
      let j: int = 0
      for (let i = 0; i < fnum; i++) {
        let f = ct.getField(i)
        if (!f.isStatic()) {
          res[j] = f.getName()
          j++
        }
      }
      return res
    } else if (t instanceof ArrayType) {
      let av = Value.of(o) as ArrayValue
      let len = av.getLength()
      let res = new string[len as int]
      for (let i = 0; i < len; i++) {
        res[i] = new Int(i).toString()
      }
      return res
    } else if (t instanceof LambdaType) {
      return new string[0]
    } else if (t instanceof EnumType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof UnionType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof TupleType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    }
    assert(false)
  }

  /**
   * Returns the values of the fields of an object
   *
   * @param o an object
   *
   * @returns an array containing the given object's own string-keyed field values
   */
  // TODO(shumilov-petr): Need to rename into `values`. Blocked by Object's childs
  public static values__(o: Object): Object[] {
    if (o instanceof Char ||
        o instanceof Boolean ||
        o instanceof Byte ||
        o instanceof Short ||
        o instanceof Int ||
        o instanceof Long ||
        o instanceof Float ||
        o instanceof Double) {
      return new Object[0]
    }
    if (o instanceof String) {
      let sv = o as string
      let len = sv.length()
      let res = new Object[len]
      for (let i = 0; i < len; i++) {
        res[i] = new StringBuilder().append(sv.charAt(i)).toString()
      }
      return res
    }
    let t = Type.of(o)
    if (t instanceof ClassType) {
      let cv = Value.of(o) as ClassValue
      let keys = Object.keys(o)
      let len = keys.length
      let res = new Object[len]
      for (let i = 0; i < len; i++) {
        res[i] = cv.getFieldByName(keys[i]).getData()
      }
      return res
    } else if (t instanceof ArrayType) {
      let av = Value.of(o) as ArrayValue
      let len = av.getLength()
      let res = new Object[len as int]
      for (let i = 0; i < len; i++) {
        res[i] = av.getElement(i).getData()
      }
      return res
    } else if (t instanceof LambdaType) {
      return new Object[0]
    } else if (t instanceof EnumType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof UnionType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof TupleType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    }
    assert(false)
  }

  /**
   * Returns an array of key/values of properties of an object
   *
   * @param o object that contains the fields
   *
   * @returns array representation of key/values
   */
  public static entries(o: Object): Tuple[] {
    if (o instanceof Char ||
        o instanceof Boolean ||
        o instanceof Byte ||
        o instanceof Short ||
        o instanceof Int ||
        o instanceof Long ||
        o instanceof Float ||
        o instanceof Double) {
      return new Tuple[0]
    }
    if (o instanceof String) {
      let sv = o as string
      let len = sv.length()
      let res = new Tuple[len]
      for (let i = 0; i < len; i++) {
        res[i] = new Object[2]
        res[i][0] = new Int(i).toString()
        res[i][1] = new StringBuilder().append(sv.charAt(i)).toString()
      }
      return res
    }
    let t = Type.of(o)
    if (t instanceof ClassType) {
      let cv = Value.of(o) as ClassValue
      let keys = Object.keys(o)
      let len = keys.length
      let res = new Tuple[len]
      for (let i = 0; i < len; i++) {
        res[i] = new Object[2]
        res[i][0] = keys[i]
        res[i][1] = cv.getFieldByName(keys[i]).getData()
      }
      return res
    } else if (t instanceof ArrayType) {
      let av = Value.of(o) as ArrayValue
      let len = av.getLength()
      let res = new Tuple[len as int]
      for (let i = 0; i < len; i++) {
        res[i] = new Object[2]
        res[i][0] = new Int(i).toString()
        res[i][1] = av.getElement(i).getData()
      }
      return res
    } else if (t instanceof LambdaType) {
      return new Tuple[0]
    } else if (t instanceof EnumType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof UnionType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof TupleType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    }
    assert(false)
  }

  /**
   * Returns the names of the own fields of an object
   *
   * @param o object that contains the own fields
   *
   * @returns array representation of names
   */
  public static getOwnPropertyNames(o: Object): string[] {
    if (o instanceof Char ||
        o instanceof Boolean ||
        o instanceof Byte ||
        o instanceof Short ||
        o instanceof Int ||
        o instanceof Long ||
        o instanceof Float ||
        o instanceof Double) {
      return new string[0]
    }
    let t = Type.of(o)
    if (t instanceof StringType || t instanceof ArrayType) {
      let keys = Object.keys(o)
      let len = keys.length
      let res = new string[len + 1]
      for (let i = 0; i < len; i++) {
        res[i] = keys[i]
      }
      res[len] = "length"
      return res
    }
    if (t instanceof ClassType) {
      return Object.keys(o)
    } else if (t instanceof LambdaType) {
      return ["length", "name"]
    } else if (t instanceof EnumType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof UnionType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof TupleType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    }
    assert(false)
  }

  /**
   * Determines whether an object has a field with the specified name
   *
   * @param key the string name of the field to test
   *
   * @returns true if the object has the specified field; false otherwise
   */
  public hasOwnProperty(key: string): boolean {
    let keys = Object.getOwnPropertyNames(this)
    let len = keys.length
    for(let i = 0; i < len; i++) {
      if (keys[i] == key) {
        return true
      }
    }
    return false
  }

  /**
   * Determines whether an object has a element with the specified index
   *
   * @param index the number index of the element to test
   *
   * @returns true if the object has the specified element; false otherwise
   */
  public hasOwnProperty(index: number): boolean {
    if ((this) instanceof String) {
      let sv = this as String
      let len = sv.length()
      let idx = index as long
      return (0 <= idx && idx < len)
    }
    let t = Type.of(this)
    if (t instanceof ArrayType) {
      let av = Value.of(this) as ArrayValue
      let len = av.getLength()
      let idx = index as long
      return (0 <= idx && idx < len)
    } else if (t instanceof EnumType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof UnionType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    } else if (t instanceof TupleType) {
      // TODO(shumilov-petr): Not implemented
      throw new Error("Not implemented")
    }
    return false
  }

  /**
   * Determines whether an object has a field with the specified name
   *
   * @param target an object
   *
   * @param key the string name of the field to test
   *
   * @returns true if the object has the specified field; false otherwise
   */
  public static hasOwn(target: Object, key: string): boolean {
    return target.hasOwnProperty(key)
  }

  /**
   * Determines whether an object has a element with the specified index
   *
   * @param target an object
   *
   * @param index the number index of the element to test
   *
   * @returns true if the object has the specified element; false otherwise
   */
  public static hasOwn(target: Object, index: number): boolean {
    return target.hasOwnProperty(index)
  }
}

export type NullishType = Object | null
