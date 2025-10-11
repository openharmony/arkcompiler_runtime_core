# STValue

## 介绍

STValue作为一个封装类，它主要提供了一系列能够在ArkTS-Dyn中操作静态类型的ArkTS-Sta中数据的接口。

在ArkTS-Dyn动态运行中实现的STValue对象会保存一个指向静态(ArkTS-Sta)对象的引用，通过这个引用就可以操作对应ArkTS-Sta内对象的值。这一流程如下图所示。

<img src='data:image/png;base64,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'/>

---

`STValue`一共提供了`accessor`、`check`、`instance`、`invoke`、`unwrap`和`wrap`六种类型接口，其中：

- `accessor`提供对ArkTS-Sta对象属性、数组元素和模块变量的访问接口。
- `check`提供了基本类型与引用类型的类型检查的接口。
- `instance`提供了实例创建、查找类型和继承关系的接口。
- `invoke`提供了ArkTS-Sta对象方法、类静态方法和模块函数动态调用相关的接口。
- `wrap`提供了将ArkTS-Dyn值转换为ArkTS-Sta值并封装为STValue对象的接口。
- `unwrap`提供了将STValue对象反解为ArkTS-Dyn值的接口。

---

此外，STValue部分接口需要指定操作的ArkTS-Sta类型。为此我们提供了类型枚举`SType`：

|  枚举名   |                       说明                       |
| :-------: | :----------------------------------------------: |
|  BOOLEAN  |        布尔类型，对应ArkTS-Sta中的boolean        |
|   CHAR    |         字符类型，对应ArkTS-Sta中的char          |
|   BYTE    |         字节类型，对应ArkTS-Sta中的byte          |
|   SHORT   |        短整数类型，对应ArkTS-Sta中的short        |
|    INT    |          整数类型，对应ArkTS-Sta中的int          |
|   LONG    |        长整数类型，对应ArkTS-Sta中的long         |
|   FLOAT   |     单精度浮点数类型，对应ArkTS-Sta中的float     |
|  DOUBLE   | 双精度浮点数类型，对应ArkTS-Sta中的double/number |
| REFERENCE |         引用类型，对应ArkTS-Sta中的引用          |
|   VOID    |          无类型，对应ArkTS-Sta中的void           |

## 引用STValue

目前可以通过以下方式获取STValue:

```ts
let STValue = globalThis.gtest.etsVm.STValue;
```

## 1 STValue_accessor

### 1.1 findClass

`static findClass(desc: string): STValue`

用于根据ArkTS-Sta类名查找类定义，接受一个类的全限定路径字符串作为参数，返回封装了该类的STValue对象。如果类不存在或参数错误，会抛出异常。

参数:

| 参数名 |  类型  | 必填 |     说明     |
| :----: | :----: | :--: | :----------: |
|  desc  | string |  是  | 类全限定路径 |

返回值:

|  类型   |          说明           |
| :-----: | :---------------------: |
| STValue | 封装了该类的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_example.Student');

// stvalue_example.ets ArkTS-Sta
export class Student {
}
```

---


### 1.2 findNamespace

`static findNamespace(desc: string): STValue`

用于根据名称查找命名空间，接受一个字符串参数（命名空间的全限定路径），返回表示该命名空间的STValue对象。如果命名空间不存在或参数错误，会抛出异常。

参数:

| 参数名 |  类型  | 必填 |        说明        |
| :----: | :----: | :--: | :----------------: |
|  desc  | string |  是  | 命名空间全限定路径 |

返回值:

|  类型   |            说明             |
| :-----: | :-------------------------: |
| STValue | 代表该命名空间的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_example.MyNamespace');

// stvalue_example.ets ArkTS-Sta
export namespace MyNamespace {}
```

---

### 1.3 findEnum

`static findEnum(desc: string): STValue`

用于根据名称查找枚举类型定义，接受一个字符串参数（枚举的全限定路径），返回表示该枚举的STValue对象。如果枚举不存在或参数错误，会抛出异常。

参数:

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
|  desc  | string |  是  | 枚举全限定路径 |


返回值:

|  类型   |          说明           |
| :-----: | :---------------------: |
| STValue | 代表该枚举的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_example.COLOR');

// stvalue_example.ets ArkTS-Sta
export enum COLOR {
    Red,
    Green
}
```

---

### 1.4 findModule

`static findModule(desc: string): STValue`

用于根据名称查找模块定义，接受一个字符串参数（模块的全限定路径），返回表示该模块的STValue对象。如果模块不存在或参数错误，会抛出异常。

参数:

| 参数名 |  类型  | 必填 |              说明               |
| :----: | :----: | :--: | :-----------------------------: |
|  desc  | string |  是  | 模块全限定路径(`文件名.模块名`) |

返回值:

|  类型   |          说明           |
| :-----: | :---------------------: |
| STValue | 代表该模块的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let mod = STValue.findModule('stvalue_example');

// stvalue_example.ets ArkTS-Sta
export function foo(): void {
}
```

---

### 1.5 classGetSuperClass

`classGetSuperClass(): STValue | null`

用于获取类的父类定义，不接受任何参数，返回表示父类的STValue对象。如果当前类没有父类（如Object类），会返回null；如果`this`不是类，则会抛出异常。

参数: 无

返回值:

|  类型   |         说明          |
| :-----: | :-------------------: |
| STValue | 代表父类的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let subClass = STValue.findClass('stvalue_example.Dog');
let parentClass = subClass.classGetSuperClass();  // 代表父类'stvalue_example.Animal'的STValue对象

// stvalue_example.ets ArkTS-Sta
export class Animal {
  static name: string = 'Animal';
}
export class Dog extends Animal {
  static name: string = 'Dog';
}
```

---

### 1.6 fixedArrayGetLength

`fixedArrayGetLength(): number`

用于获取固定长度数组的元素数量，不接受任何参数，返回表示数组长度的数字值。如果调用对象不是固定长度数组类型，会抛出异常。

参数:
无

返回值:

|  类型  |   说明   |
| :----: | :------: |
| number | 数组长度 |

示例:

```typescript
// ArkTS-Dyn
let mod = STValue.findModule('stvalue_example');
let strArray = mod.moduleGetVariable('strArray', SType.REFERENCE);
let arrayLength = strArray.fixedArrayGetLength(); // 3

// stvalue_example.ets ArkTS-Sta
export let strArray: FixedArray<string> = ['ab', 'cd', 'ef'];
```

---

### 1.7 enumGetIndexByName

`enumGetIndexByName(name: string): number`

用于根据枚举成员名称查询其在枚举中的索引位置，接受一个字符串参数（成员名称），返回表示该成员索引的数字值。如果成员不存在或参数错误，会抛出异常。


参数:

| 参数名 |  类型  | 必填 |   说明   |
| :----: | :----: | :--: | :------: |
|  name  | string |  是  | 枚举名称 |

返回值:

|  类型  |   说明   |
| :----: | :------: |
| number | 枚举索引 |

示例:

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_example.COLOR');
let redIndex = colorEnum.enumGetIndexByName('Red'); // 0

// stvalue_example.ets ArkTS-Sta
export enum COLOR{
    Red = 1,
    Green = 3
}
```

---

### 1.8 enumGetNameByIndex

`enumGetNameByIndex(index: number): string`

用于根据索引位置查询枚举成员的名称，接受一个数字参数（索引值），返回对应的枚举成员名称字符串。主要作用是通过索引定位枚举成员，支持基于索引的枚举操作。如果索引越界或参数错误，会抛出异常。

参数:

| 参数名 |  类型  | 必填 |   说明   |
| :----: | :----: | :--: | :------: |
| index  | string |  是  | 枚举索引 |

返回值:

|  类型  |   说明   |
| :----: | :------: |
| string | 枚举名称 |

示例:

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_example.COLOR');
let colorName = colorEnum.enumGetNameByIndex(0); // 'Red'


// stvalue_example.ets ArkTS-Sta
export enum COLOR{
    Red = 1,
    Green = 3
}
```

---


### 1.9 enumGetValueByName

`enumGetValueByName(name: string, valueType: SType): STValue`

用于根据成员名称获取枚举成员的值，接受两个参数：成员名称和值类型，返回对应值的STValue对象。支持整型和字符串类型的枚举值，如果成员不存在、类型不匹配或参数错误，会抛出异常。

参数:

|  参数名   |  类型  | 必填 |    说明    |
| :-------: | :----: | :--: | :--------: |
|   name    | string |  是  |  枚举名称  |
| valueType | SType  |  是  | 枚举值类型 |

返回值:

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 枚举值的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_example.COLOR');
let redValue = colorEnum.enumGetValueByName('Red', SType.INT); // 1

// stvalue_example.ets ArkTS-Sta
export enum COLOR{
    Red = 1,
    Green = 3,
}
```

---


### 1.10 enumGetValueByIndex

`enumGetValueByIndex(index: number, valueType: SType): STValue`

用于根据索引位置获取枚举成员的值，接受两个参数：索引位置和值类型，返回对应值的STValue对象。支持整型和字符串类的型枚举值，如果索引越界、类型不匹配或参数错误，会抛出异常。

参数:

|  参数名   |  类型  | 必填 |    说明    |
| :-------: | :----: | :--: | :--------: |
|   index   | number |  是  |  枚举索引  |
| valueType | SType  |  是  | 枚举值类型 |

返回值:

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 枚举值的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let colorEnum = STValue.findEnum('stvalue_example.COLOR');
let redValue = colorEnum.enumGetValueByIndex(0, SType.INT); // 1

// stvalue_example.ets ArkTS-Sta
export enum COLOR{
    Red = 1,
    Green = 3
}
```

---

### 1.11 classGetStaticField

`classGetStaticField(name: string, fieldType: SType): STValue`

用于获取类的静态字段值，接受两个参数：字段名称和字段类型，返回对应值的STValue对象。如果字段不存在、类型不匹配或参数错误，会抛出异常。

参数:

|  参数名   |  类型  | 必填 |   说明   |
| :-------: | :----: | :--: | :------: |
| fieldName | string |  是  | 字段名称 |
| fieldType | SType  |  是  | 字段类型 |

返回值:

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 字段值的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let personClass = STValue.findClass('stvalue_example.Person');
let name = personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString(); // 'Person'
let age = personClass.classGetStaticField('age', SType.INT).unwrapToNumber();  // 18

// stvalue_example.ets ArkTS-Sta
export class Person {
    static name: string = 'Person';
    static age: number = 18;
}
```

---

### 1.12 classSetStaticField

`classSetStaticField(name: string, val: STValue, fieldType: SType): void`

用于设置类的静态字段值，接受三个参数：字段名称、要设置的值和字段类型）。主要作用是修改类的静态成员变量，支持基本类型和引用类型。如果类型不匹配、字段不存在或参数错误，会抛出异常。

参数:

|  参数名   |  类型   | 必填 |    说明    |
| :-------: | :-----: | :--: | :--------: |
| fieldName | string  |  是  |  字段名称  |
|    val    | STValue |  是  | 要设置的值 |
| fieldType |  SType  |  是  |  字段类型  |

返回值: 无

示例:

```typescript
// ArkTS-Dyn
let personClass = STValue.findClass('stvalue_example.Person');
personClass.classSetStaticField('age', STValue.wrapNumber(21), SType.DOUBLE);
let name = personClass.classGetStaticField('name', SType.REFERENCE).unwrapToString(); // 'Bob'
let age = personClass.classGetStaticField('age', SType.DOUBLE).unwrapToNumber(); // 21

// stvalue_example.ets ArkTS-Sta
export class Person {
    static name: string = 'Person';
    static age: number = 18;
}
```

---

### 1.13 objectGetProperty

`objectGetProperty(name: string, propType: SType): STValue`

用于获取对象实例的属性值，接受两个参数：属性名称和属性类型，返回对应值的STValue对象。主要作用是访问对象的成员变量，支持基本类型和引用类型。如果属性不存在、类型不匹配或参数错误，会抛出异常。


参数:

|  参数名  |  类型  | 必填 |   说明   |
| :------: | :----: | :--: | :------: |
| propName | string |  是  | 属性名称 |
| propType | SType  |  是  | 属性类型 |

返回值:

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 属性值的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let mod = STValue.findModule('stvalue_example');
let alice = mod.moduleGetVariable('studentAlice', SType.REFERENCE);
let name = alice.objectGetProperty('name', SType.REFERENCE).unwrapToString(); // 'Alice'

// stvalue_example.ets ArkTS-Sta
export class Student {
    name: string;
    constructor(n: string) {
        this.name = n;
    }
}

export let studentAlice: Student = new Student('Alice');
```

---

### 1.14 objectSetProperty

`objectSetProperty(name: string, val: STValue, propType: SType): void`

用于设置对象实例的属性值，接受三个参数：属性名称、要设置的值和属性类型。主要作用是修改对象的成员变量，支持基本类型和引用类型。如果类型不匹配、属性不存在或参数错误，会抛出异常。

参数:

|  参数名  |  类型   | 必填 |    说明    |
| :------: | :-----: | :--: | :--------: |
| propName | string  |  是  |  属性名称  |
|   val    | STValue |  是  | 要设置的值 |
| propType |  SType  |  是  |  属性类型  |

返回值: 无

示例:

```typescript
// ArkTS-Dyn
let mod = STValue.findModule('stvalue_example');
let alice = mod.moduleGetVariable('studentAlice', SType.REFERENCE);
alice.objectGetProperty('name', SType.REFERENCE).unwrapToString(); // 'Alice'
let name = alice.objectSetProperty('name', STValue.wrapString('Bob'), SType.REFERENCE);
alice.objectGetProperty('name', SType.REFERENCE).unwrapToString(); // 'Bob'

// stvalue_example.ets ArkTS-Sta
export class Student {
    name: string;
    constructor(n: string) {
        this.name = n;
    }
}

export let studentAlice: Student = new Student('Alice');
```

---

### 1.15 fixedArrayGet

`fixedArrayGet(idx: number, elementType: SType): STValue`

用于获取固定长度数组中指定索引的元素值，接受两个参数：索引位置和元素类型，返回对应值的STValue对象。主要作用是访问数组元素，支持基本类型和引用类型。如果索引越界、类型不匹配或参数错误，会抛出异常。

参数:

|   参数名    |  类型  | 必填 |     说明     |
| :---------: | :----: | :--: | :----------: |
|     idx     | number |  是  |   数组索引   |
| elementType | SType  |  是  | 数组元素类型 |

返回值:

|  类型   |       说明        |
| :-----: | :---------------: |
| STValue | 元素的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let module = STValue.findModule('stvalue_example');
let strArray = module.moduleGetVariable('strArray', SType.REFERENCE);
let str = strArray.fixedArrayGet(1, SType.REFERENCE).unwrapToString(); // 'cd'

// stvalue_example.ets ArkTS-Sta
export let strArray: FixedArray<string> = ['ab', 'cd', 'ef'];
```

---

### 1.16 fixedArraySet

`fixedArraySet(idx: number, val: STValue, elementType: SType): void`

用于设置固定长度数组中指定索引的元素值，接受三个参数：索引位置、要设置的值和元素类型。主要作用是修改数组元素，支持基本类型和引用类型。如果类型不匹配、索引越界或参数错误，会抛出异常。

参数:

|   参数名    |  类型   | 必填 |     说明     |
| :---------: | :-----: | :--: | :----------: |
|     idx     | number  |  是  |   数组索引   |
|     val     | STValue |  是  |  要设置的值  |
| elementType |  SType  |  是  | 数组元素类型 |

返回值: 无

示例:

```typescript
// ArkTS-Dyn
let module = STValue.findModule('stvalue_example');
let strArray = module.moduleGetVariable('strArray', SType.REFERENCE);
strArray.fixedArraySet(1, STValue.wrapString('xy'), SType.REFERENCE);

// stvalue_example.ets ArkTS-Sta
export let strArray: FixedArray<string> = ['ab', 'cd', 'ef'];
```

---

### 1.17 moduleGetVariable

`moduleGetVariable(name: string, variableType: SType): STValue `

用于获取模块中的变量值，接受两个参数：变量名称和变量类型，返回对应值的STValue对象。主要作用是访问模块中的全局变量，支持基本类型和引用类型。如果变量不存在、类型不匹配或参数错误，会抛出异常。

参数:

|    参数名    |  类型  | 必填 |      说明      |
| :----------: | :----: | :--: | :------------: |
|     name     | string |  是  |    变量名称    |
| variableType | SType  |  是  | 变量类型枚举值 |

返回值:

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 变量值的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let strArray = module.moduleGetVariable('strArray', SType.REFERENCE);
let magicSTValueInt = module.moduleGetVariable('magic_int', SType.INT);
let magicSTValueBool = module.moduleGetVariable('magic_boolean', SType.BOOLEAN);

// stvalue_example.ets ArkTS-Sta
export let strArray: FixedArray<string> = ['ab', 'cd', 'ef'];
export let magic_int: int = 42;
export let magic_boolean: boolean = true;
```

---

### 1.18 moduleSetVariable

`moduleSetVariable(name: string, value: STValue, variableType: SType): boolean`

用于设置模块中的变量值，接受三个参数：变量名称、要设置的值和变量类型，返回操作是否成功的布尔值。主要作用是修改模块中的全局变量，支持基本类型和引用类型。如果变量不存在、类型不匹配或参数错误，会抛出异常。

参数:

|    参数名    |  类型   | 必填 |      说明      |
| :----------: | :-----: | :--: | :------------: |
|     name     | string  |  是  |    变量名称    |
|    value     | STValue |  是  |   要设置的值   |
| variableType |  SType  |  是  | 变量类型枚举值 |

返回值:

|  类型   |              说明               |
| :-----: | :-----------------------------: |
| boolean | 设置成功返回true，失败返回false |

示例:

```typescript
// ArkTS-Dyn
module.moduleSetVariable('magic_int', STValue.wrapInt(44), SType.INT);
module.moduleSetVariable('magic_boolean', STValue.wrapBoolean(false), SType.BOOLEAN);

// stvalue_example.ets ArkTS-Sta
export let magic_int: int = 42;
export let magic_boolean: boolean = true;
```

---

### 1.19 namespaceGetVariable

`namespaceGetVariable(name: string, variableType: SType): STValue | null`

用于获取命名空间中的变量值，接受两个参数：变量名称和变量类型，返回对应值的STValue对象。主要作用是访问命名空间中的全局变量，支持基本类型和引用类型。如果变量不存在、类型不匹配或参数错误，会抛出异常。


参数:

|    参数名    |  类型  | 必填 |      说明      |
| :----------: | :----: | :--: | :------------: |
|     name     | string |  是  |    变量名称    |
| variableType | SType  |  是  | 变量类型枚举值 |

返回值:

|  类型   |        说明         |
| :-----: | :-----------------: |
| STValue | 变量值的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_example.MyNamespace');
let data = ns.namespaceGetVariable('data', SType.INT);
data.unwrapToNumber();  // 42

// stvalue_example.ets ArkTS-Sta
export namespace MyNamespace {
    export let data: int = 42;
}
```

---

### 1.20 namespaceSetVariable

`namespaceSetVariable(name: string, value: STValue, variableType: SType): boolean`

用于设置命名空间中的变量值，接受三个参数：变量名称、要设置的值和变量类型，返回操作是否成功的布尔值。主要作用是修改命名空间中的全局变量，支持基本类型和引用类型。如果变量不存在、类型不匹配或参数错误，会抛出异常。

参数:

|    参数名    |  类型   | 必填 |      说明      |
| :----------: | :-----: | :--: | :------------: |
|     name     | string  |  是  |    变量名称    |
|    value     | STValue |  是  |   要设置的值   |
| variableType |  SType  |  是  | 变量类型枚举值 |

返回值:

|  类型   |              说明               |
| :-----: | :-----------------------------: |
| boolean | 设置成功返回true，失败返回false |

示例:

```typescript
// ArkTS-Dyn
let ns = STValue.findNamespace('stvalue_example.MyNamespace');
ns.namespaceSetVariable('magic_int_n', STValue.wrapInt(0), SType.INT);

// stvalue_example.ets ArkTS-Sta
export namespace MyNamespace {
    export let data: int = 42;
}
```

---

### 1.21 objectGetType

`objectGetType(): STValue`

用于获取引用类型对象的类型信息，不接受任何参数，返回表示对象类型的STValue对象。如果`this`包装的不是ArkTS-Sta对象引用，会抛出异常。

参数: 无

返回值:

|  类型   |         说明          |
| :-----: | :-------------------: |
| STValue | 类型信息的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let strWrap = STValue.wrapString('hello world');
let strType = strWrap.objectGetType();
let isString = strWrap.objectInstanceOf(strType); // true
```

---

## 2 stvalue_example

### 2.1 isString

`isString(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta字符串，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                  说明                   |
| :-----: | :-------------------------------------: |
| boolean | 如果是字符串类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let str = STValue.wrapString("Hello");
let isStr = str.isString(); // true

let num = STValue.wrapInt(42);
let isStr1 = num.isString(); // false

let mod = STValue.findModule('stvalue_example');
str = mod.moduleGetVariable('str' , SType.REFERENCE);
let isStr2 = str.isString(); // true

// stvalue_example.ets ArkTS-Sta
export let str = 'I am str';
```

---

### 2.2 isBigInt

`isBigInt(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta BigInt对象，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                  说明                   |
| :-----: | :-------------------------------------: |
| boolean | 如果是大整数类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let bigNum = STValue.wrapBigInt(1234567890n);
let isBigInt = bigNum.isBigInt(); // true
let num = STValue.wrapInt(42);
let isBigInt1 = num.isBigInt(); // false
```

---

### 2.3 isNull

`isNull(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的`null`，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                说明                 |
| :-----: | :---------------------------------: |
| boolean | 如果是null值返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let nullValue = STValue.getNull();
let isNull = nullValue.isNull(); // true
let intValue = STValue.wrapNumber(42);
let isNull1 = intValue.isNull(); // false
```

---

### 2.4 isUndefined

`isUndefined(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的`undefined`，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                   说明                   |
| :-----: | :--------------------------------------: |
| boolean | 如果是undefined值返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let undefValue = STValue.getUndefined();
let isUndef = undefValue.isUndefined(); // true
let intValue = STValue.wrapNumber(42);
let isUndef1 = intValue.isUndefined(); // false
```

---


### 2.5 isEqualTo

`isEqualTo(other: STValue): boolean`

用于比较`this`和`other`包装的ArkTS-Sta对象引用是否相等，返回布尔值结果。`this`和`other`包装需要包装ArkTS-Sta对象引用。如果参数错误或类型不匹配，会抛出异常。

参数:

| 参数名 |  类型   | 必填 |           说明            |
| :----: | :-----: | :--: | :-----------------------: |
| other  | STValue |  是  | 要比较的另一个STValue对象 |

返回值:

|  类型   |                      说明                       |
| :-----: | :---------------------------------------------: |
| boolean | 如果两个引用指向同一对象返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let mod = STValue.findModule('stvalue_example');
let ref1 = mod.moduleGetVariable('ref1', SType.REFERENCE);
let ref2 = mod.moduleGetVariable('ref2', SType.REFERENCE);
let ref3 = mod.moduleGetVariable('ref3', SType.REFERENCE);
ref1.isEqualTo(ref2); // true
ref1.isEqualTo(ref3); // false

// stvalue_example.ets ArkTS-Sta
export let ref1 = new Object();
export let ref2 = ref1;
export class A {}
export let ref3 = new A();
```

---

### 2.6 isStrictlyEqualTo

`isStrictlyEqualTo(other: STValue): boolean`

用于比较`this`和`other`包装的ArkTS-Sta对象引用是否严格相等，返回布尔值结果。`this`和`other`包装需要包装ArkTS-Sta对象引用。如果参数错误或类型不匹配，会抛出异常。

参数:

| 参数名 |  类型   | 必填 |           说明            |
| :----: | :-----: | :--: | :-----------------------: |
| other  | STValue |  是  | 要比较的另一个STValue对象 |

返回值:

|  类型   |                    说明                     |
| :-----: | :-----------------------------------------: |
| boolean | 如果两个对象严格相等返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let mod = STValue.findModule('stvalue_example');
let ref1 = mod.moduleGetVariable('ref1', SType.REFERENCE);
let ref2 = mod.moduleGetVariable('ref2', SType.REFERENCE);
let ref3 = mod.moduleGetVariable('ref3', SType.REFERENCE);
ref1.isStrictlyEqualTo(ref2); // true
ref1.isStrictlyEqualTo(ref3); // false

// stvalue_example.ets ArkTS-Sta
export let ref1 = new Object();
export let ref2 = ref1;
export class A {}
export let ref3 = new A();
```

---

### 2.7 isBoolean

`isBoolean(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的boolean值，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| boolean | 如果是布尔类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let isBool = boolValue.isBoolean(); // true
let numValue = STValue.wrapInt(1);
let isBool1 = numValue.isBoolean(); // false
```

---

### 2.8 isByte

`isByte(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的byte值，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| boolean | 如果是字节类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let byteValue = STValue.wrapByte(127);
let isByte = byteValue.isByte(); // true
let intValue = STValue.wrapInt(42);
let isByte1 = intValue.isByte(); // false
```

---

### 2.9 isChar

`isChar(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的char值，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| boolean | 如果是字符类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let charValue = STValue.wrapChar('A');
let isChar = charValue.isChar(); // true
let strValue = STValue.wrapString("Hello");
let isChar1 = strValue.isChar(); // false
```

---

### 2.10 isShort

`isShort(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的short值，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                  说明                   |
| :-----: | :-------------------------------------: |
| boolean | 如果是短整型类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let shortValue = STValue.wrapShort(32767);
let isShort = shortValue.isShort(); // true
let intValue = STValue.wrapInt(32767);
let isShort1 = intValue.isShort(); // false
```

---

### 2.11 isInt

`isInt(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的int值，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| boolean | 如果是整型类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(44);
let isInt = intValue.isInt(); // true
let longValue = STValue.wrapLong(1024);
let isInt1 = longValue.isInt(); // false
```

---

### 2.12 isLong

`isLong(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的long值，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                  说明                   |
| :-----: | :-------------------------------------: |
| boolean | 如果是长整型类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let longValue = STValue.wrapLong(1024);
let isLong = longValue.isLong(); // true
let intValue = STValue.wrapInt(44);
let isLong1 = intValue.isLong(); // false
```

---

### 2.13 isFloat

`isFloat(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的float值，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                    说明                     |
| :-----: | :-----------------------------------------: |
| boolean | 如果是单精度浮点类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let floatValue = STValue.wrapFloat(3.14);
let isFloat = floatValue.isFloat(); // true
let intValue = STValue.wrapInt(42);
let isFloat1 = intValue.isFloat(); // false
```

---

### 2.14 isNumber

`isNumber(): boolean`

用于检查STValue对象是否包装的是ArkTS-Sta的number/double值，不接受任何参数，返回布尔值结果。

参数: 无

返回值:

|  类型   |                    说明                     |
| :-----: | :-----------------------------------------: |
| boolean | 如果是双精度浮点类型返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let doubleValue = STValue.wrapNumber(3.14);
let isNumber = doubleValue.isNumber(); // true
let intValue = STValue.wrapInt(42);
let isNumber1 = intValue.isNumber(); // false
```

---

### 2.15 typeIsAssignableFrom

`static typeIsAssignableFrom(fromType: STValue, toType: STValue): boolean`

用于检查类型之间的赋值兼容性，接受两个参数：源类型（fromType）和目标类型（toType），返回布尔值结果。fromType和toType一般来源于`findClass/objectGetType/classGetSuperClass`的返回值。基于底层类型系统的规则，判断源类型的值是否可以安全赋值给目标类型的变量。

参数:

|  参数名  |  类型   | 必填 |            说明            |
| :------: | :-----: | :--: | :------------------------: |
| fromType | STValue |  是  |   源类型（被赋值的类型）   |
|  toType  | STValue |  是  | 目标类型（赋值的目标类型） |

返回值:

|  类型   |                        说明                         |
| :-----: | :-------------------------------------------------: |
| boolean | 如果fromType可以赋值给toType返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_example.Student');
let subStudentCls = STValue.findClass('stvalue_example.SubStudent');
let isAssignable = STValue.typeIsAssignableFrom(subStudentCls, studentCls); // true
typeIsAssignableFrom(studentCls, subStudentCls); // false

// stvalue_example.ets ArkTS-Sta
export class Student {}
export class SubStudent extends Student {}
```

---

### 2.16 objectInstanceOf

`objectInstanceOf(typeArg: STValue): boolean`

用于检查对象是否为指定类型的实例，接受一个参数（类型对象），返回布尔值结果。如果对象是指定类型的实例返回true，否则返回false。

参数:

| 参数名  |  类型   | 必填 |           说明           |
| :-----: | :-----: | :--: | :----------------------: |
| typeArg | STValue |  是  | 要检查的类型（类或接口） |

返回值:

|  类型   |                   说明                    |
| :-----: | :---------------------------------------: |
| boolean | 如果是该类型的实例返回true，否则返回false |

示例:

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_example.Student');
let stuObj = STValue.findModule('stvalue_example').moduleGetVariable('stuObj', SType.REFERENCE);
stuObj.objectInstanceOf(studentCls); // true

// stvalue_example.ets ArkTS-Sta
export class Student {}
export let stuObj = new Student();
```

---

## 3 STValue_instance

### 3.1 classInstantiate

`classInstantiate(ctorSignature: string, args: STValue[]): STValue`

用于通过类的构造函数实例化对象，接受两个参数：构造函数签名和参数数组，返回新创建的对象。动态创建类实例，支持带参数的构造函数调用，如果类不存在、构造函数不匹配或参数错误，会抛出异常。

参数:

|    参数名     |   类型    | 必填 |              说明               |
| :-----------: | :-------: | :--: | :-----------------------------: |
| ctorSignature |  string   |  是  | 构造函数（`参数类型:返回类型`） |
|     args      | STValue[] |  是  |        构造函数参数数组         |

返回值:

|  类型   |       说明       |
| :-----: | :--------------: |
| STValue | 新创建的对象实例 |

示例:

```typescript
// ArkTS-Dyn
let Student = STValue.findClass('stvalue_example.Student');
let intObj = Student.classInstantiate('C{std.core.String}:', [STValue.wrapString('Alice')]);

// stvalue_example.ets ArkTS-Sta
export class Student {
    name: string;
    constructor(n: string) {
        this.name = n;
    }
}
export let stuObj = new Student();
```

---

### 3.2 newFixedArrayPrimitive

`static newFixedArrayPrimitive(length: number, elementType: SType): STValue`

用于创建预分配内存固定长度的基本类型数组，接受两个参数：数组长度和元素类型（SType枚举），返回创建的数组对象。支持所有基本数据类型，如果元素类型不支持或参数错误，会抛出异常。

参数:

|   参数名    |  类型  | 必填 |           说明           |
| :---------: | :----: | :--: | :----------------------: |
|   length    | number |  是  |         数组长度         |
| elementType | SType  |  是  | 数组元素类型（数值形式） |

返回值:

|  类型   |               说明                |
| :-----: | :-------------------------------: |
| STValue | 新创建的固定长度数组的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let boolArray = STValue.newFixedArrayPrimitive(5, SType.BOOLEAN);
```

---

### 3.3 newFixedArrayReference

`static newFixedArrayReference(length: number, elementType: STValue, initialElement: STValue): STValue`

用于创建预分配内存固定长度的引用类型数组，接受三个参数：数组长度、元素类型和初始元素，返回创建的数组对象。支持类、接口等引用类型元素，数组所有元素初始化为相同的初始值，如果参数错误或类型不匹配，会抛出异常。

参数:

|     参数名     |  类型   | 必填 |       说明       |
| :------------: | :-----: | :--: | :--------------: |
|     length     | number  |  是  |     数组长度     |
|  elementType   | STValue |  是  |   数组元素类型   |
| initialElement | STValue |  是  | 数组元素的初始值 |

返回值:

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| STValue | 新创建的固定长度引用数组的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let intClass = STValue.findClass('std.core.Int');
let intObj = intClass.classInstantiate('i:', [STValue.wrapInt(5)]);
let refArray = STValue.newFixedArrayReference(3, intClass, intObj);
```

---

## 4 STValue_invoke

### 4.1 moduleInvokeFunction

`moduleInvokeFunction(functionName: string, signature: string, args: STValue[]): STValue`

用于调用模块中的函数，接受三个参数：函数名称、函数签名和参数数组，返回函数调用的结果。主要作用是执行模块中的静态函数，支持带参数的函数调用，如果函数不存在、签名不匹配或参数错误，会抛出异常。

参数:

|    参数名    |   类型    | 必填 |                 说明                  |
| :----------: | :-------: | :--: | :-----------------------------------: |
| functionName |  string   |  是  |               函数名称                |
|  signature   |  string   |  是  | 函数签名（格式：`参数类型:返回类型`） |
|     args     | STValue[] |  是  |             函数参数数组              |

返回值:

|  类型   |           说明            |
| :-----: | :-----------------------: |
| STValue | 函数调用结果的STValue对象 |

示例:

```typescript
// stvalue_invoke.ets ArkTS-Sta
// boolean type
export function BooleanInvoke(b1 : Boolean, b2 : Boolean) : Boolean
{
    return b1 & b2;
}

// char type
export function CharInvoke(c : char) : char
{
    return c;
}

// byte type
export function ByteInvoke(b : byte) : byte
{
    return b;
}

// short type
export function ShortInvoke(s : short) : short
{
    return s;
}

// int type
export function IntInvoke(a : int, b : int) : int
{
    return a + b;
}

// long type
export function LongInvoke(l1 : long, l2 : long) : long
{
    return l1 + l2;
}

// float type
export function FloatInvoke(f1 : float, f2 : float) : float
{
    return f1 + f2;
}

// double type
export function DoubleInvoke(d1 : double, d2 : double) : double
{
    return d1 + d2;
}

// number type
export function NumberInvoke(n1 : number, n2 : number) : number
{
    return n1 + n2;
}

// reference string type
export function StringInvoke(str1 : String, str2 : String) : String
{
    return str1 + str2;
}

// reference BigInt type
export function BigIntInvoke(b1 : BigInt, b2 : BigInt) : BigInt
{
    return b1 + b2;
}

// void
export function VoidInvoke(str1 : String, str2 : String)
{
    console.log(str1 + str2);
}
```

```typescript
// ArkTS-Dyn
// invoke BooleanInvoke() function
let mod = STValue.findModule('stvalue_invoke');
let b1 = STValue.wrapBoolean(false);
let b2 = STValue.wrapBoolean(false);
let b = mod.moduleInvokeFunction('BooleanInvoke', 'zz:z', [b1,b2]);

// invoke CharInvoke() function
let c = STValue.wrapChar('c');
let r = mod.moduleInvokeFunction('CharInvoke', 'c:c', [c]);
let charKlass = STValue.findClass('std.core.Char');
let charObj = charKlass.classInstantiate('c:', [r]);
let str = charObj.objectInvokeMethod('toString', ':C{std.core.String}', []);


// invoke ByteInvoke() function
let b = STValue.wrapByte(112);
let r = mod.moduleInvokeFunction('ByteInvoke', 'b:b', [b]);

// invoke ShortInvoke() function
let s = STValue.wrapShort(3456);
let r = mod.moduleInvokeFunction('ShortInvoke', 's:s', [s]);


// invoke IntInvoke() function
let a1 = STValue.wrapInt(123);
let a2 = STValue.wrapInt(345);
let r = mod.moduleInvokeFunction('IntInvoke', 'ii:i', [a1,a2]);


// invoke LongInvoke() function
let a1 = STValue.wrapLong(123);
let a2 = STValue.wrapLong(345);
let r = mod.moduleInvokeFunction('LongInvoke', 'll:l', [a1,a2]);


// invoke FloatInvoke() function
let a1 = STValue.wrapFloat(1.4567);
let a2 = STValue.wrapFloat(4.5678);
mod.moduleInvokeFunction('FloatInvoke', 'ff:f', [a1,a2]);

// invoke DoubleInvoke() function
let a1 = STValue.wrapNumber(1.4567);
let a2 = STValue.wrapNumber(4.5678);
mod.moduleInvokeFunction('DoubleInvoke', 'dd:d', [a1,a2]);

// invoke NumberInvoke() function
let a1 = STValue.wrapNumber(1.4567);
let a2 = STValue.wrapNumber(4.5678);
mod.moduleInvokeFunction('NumberInvoke', 'dd:d', [a1,a2]);


// invoke StringInvoke() function
let s1 = STValue.wrapString('ABCDEFG');
let s2 = STValue.wrapString('HIJKLMN');
let s = mod.moduleInvokeFunction('StringInvoke', 'C{std.core.String}C{std.core.String}:C{std.core.String}', [s1,s2]);


// invoke BigIntInvoke() function
let b1 = STValue.wrapBigInt(BigInt('12345678'));
let b2 = STValue.wrapBigInt(BigInt(12345678n));
let s = mod.moduleInvokeFunction('BigIntInvoke', 'C{std.core.BigInt}C{std.core.BigInt}:C{std.core.BigInt}', [b1,b2]);


// invoke VoidInvoke() function
let s1 = STValue.wrapString('ABCDEFG');
let s2 = STValue.wrapString('HIJKLMN');
mod.moduleInvokeFunction('VoidInvoke', 'C{std.core.String}C{std.core.String}:', [s1,s2]);
```

---

### 4.2 namespaceInvokeFunction

`namespaceInvokeFunction(functionName: string, signature: string, args: STValue[]): STValue`

用于在命名空间中调用指定函数，接受三个参数：函数名称、函数签名和参数数组，返回函数调用的结果。主要作用是执行命名空间中的全局函数或静态函数，支持带参数的函数调用，如果函数不存在、签名不匹配或参数错误，会抛出异常。

参数:

|    参数名    |   类型    | 必填 |                 说明                  |
| :----------: | :-------: | :--: | :-----------------------------------: |
| functionName |  string   |  是  |               函数名称                |
|  signature   |  string   |  是  | 函数签名（格式：`参数类型:返回类型`） |
|     args     | STValue[] |  是  |             函数参数数组              |

返回值:

|  类型   |           说明            |
| :-----: | :-----------------------: |
| STValue | 函数调用结果的STValue对象 |

示例:
示例参考moduleInvokeFunction(),主要的不同点在于这里的函数位于namespace中

```typescript
// stvalue_invoke.ets ArkTS-Sta
export namespace Invoke{
  // boolean type
  export function BooleanInvoke(b1 : Boolean, b2 : Boolean) : Boolean
  {
      return b1 & b2;
  }    
}
```

```typescript
// test_stvalue_invoke.ts ArkTS-Dyn
let nsp = STValue.findNamespace('stvalue_invoke.Invoke');
let b1 = STValue.wrapBoolean(false);
let b2 = STValue.wrapBoolean(false);
let b = nsp.namespaceInvokeFunction('BooleanInvoke', 'zz:z', [b1,b2]); // false
```

---

### 4.3 functionalObjectInvoke

`functionalObjectInvoke(args: STValue[]): STValue`

用于调用函数式对象（如lambda表达式或函数对象），接受一个参数数组（必须为引用类型如果需要使用primitive需要事先装箱），返回函数调用的结果。主要作用是执行函数式对象，支持带参数的调用，如果参数非引用类型或函数对象无效，会抛出异常。

参数:

| 参数名 |   类型    | 必填 |              说明              |
| :----: | :-------: | :--: | :----------------------------: |
|  args  | STValue[] |  是  | 函数参数数组（必须为引用类型） |

返回值:

|  类型   |                 说明                  |
| :-----: | :-----------------------------------: |
| STValue | 函数调用结果的STValue对象（引用类型） |

示例:

```typescript
// ArkTS-Dyn
let mod = STValue.findModule('stvalue_example');
let getNumberFn = mod.moduleGetVariable('getNumberFn', SType.REFERENCE);
let numRes = getNumberFn.functionalObjectInvoke([STValue.wrapInt(123)]);
let jsNumSTValue = numRes.objectInvokeMethod('unboxed', ':i', []); 
let jsNum = jsNumSTValue.unwrapToNumber(); // 223

// stvalue_example.ets ArkTS-Sta
export let getNumberFn = (arg: int) => { return arg + 100; }
```

---


### 4.4 objectInvokeMethod

`objectInvokeMethod(name: string, signature: string, args: Array<STValue>): STValue`

用于动态调用对象的方法，接受三个参数：方法名称、方法签名以及参数数组，返回方法调用的结果。主要作用是执行对象的实例方法，支持带参数的方法调用，如果方法不存在、签名不匹配或参数错误，会抛出异常。


参数:

|  参数名   |      类型      | 必填 |                 说明                  |
| :-------: | :------------: | :--: | :-----------------------------------: |
|   name    |     string     |  是  |               方法名称                |
| signature |     string     |  是  | 方法签名（格式：`参数类型:返回类型`） |
|   args    | Array<STValue> |  是  |             方法参数数组              |

返回值:

|  类型   |                     说明                      |
| :-----: | :-------------------------------------------: |
| STValue | 方法调用结果的STValue对象（void方法返回null） |

示例:

```typescript
// ArkTS-Dyn
let stuObj = STValue.findModule('stvalue_example').moduleGetVariable('stuObj', SType.REFERENCE); 
let stuAge = stuObj.objectInvokeMethod('getStudentAge', ':i', []);
stuAge.unwrapToNumber(); // 18

// stvalue_example.ets ArkTS-Sta
export class Student {
    constructor(age: int, name: string) {
        this.age = age;
        this.name = name;
    }
    getStudentAge(): int {
        return this.age;
    }
}

export let stuObj = new Student(18, 'Alice');
```

---

### 4.5 classInvokeStaticMethod

`classInvokeStaticMethod(name: string, signature: string, args: STValue[]): STValue`

用于调用类的静态方法，接受三个参数：方法名称、方法签名和参数数组，返回方法调用的结果。主要作用是执行类的静态方法，支持带参数的方法调用，如果方法不存在、签名不匹配或参数错误，会抛出异常。

参数:

|  参数名   |   类型    | 必填 |                 说明                  |
| :-------: | :-------: | :--: | :-----------------------------------: |
|   name    |  string   |  是  |               方法名称                |
| signature |  string   |  是  | 方法签名（格式：`参数类型:返回类型`） |
|   args    | STValue[] |  是  |             方法参数数组              |

返回值:

|  类型   |                     说明                      |
| :-----: | :-------------------------------------------: |
| STValue | 方法调用结果的STValue对象（void方法返回null） |

示例:

```typescript
// ArkTS-Dyn
let studentCls = STValue.findClass('stvalue_example.Student');
let stuId = studentCls.classInvokeStaticMethod('getStudentId', ':i', []); // 999
studentCls.classInvokeStaticMethod('setStudentId', 'i:', [STValue.wrapInt(888)]);
console.log(stuId.unwrapToNumber()); // 888

// stvalue_example.ets ArkTS-Sta
export class Student {
    private static id: int = 999;
    static getStudentId(): int {
        return Student.id;
    }
    static setStudentId(newId: int): void {
        Student.id = newId;
    }
}
```

---

## 5 STValue_unwrap

### 5.1 unwrapToNumber

`unwrapToNumber(): number`

用于将STValue对象解包为数字，不接受任何参数，返回数字值结果。支持基本数值类型(`boolean`, `byte`, `short`, `int`, `long`, `float`, `double`)的解包，如果值类型不支持或对象为引用类型，会抛出异常。

参数: 无

返回值:

|  类型  |      说明      |
| :----: | :------------: |
| number | 解包后的数字值 |

示例:

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(42);
let num = intValue.unwrapToNumber(); // 42
```

---

### 5.2 unwrapToString

`unwrapToString(): string`

用于将STValue对象解包为字符串，不接受任何参数，返回字符串结果。仅支持字符串对象（`std.core.String`）的解包，其他类型会抛出异常。

参数: 无

返回值:

|  类型  |       说明       |
| :----: | :--------------: |
| string | 解包后的字符串值 |

示例:

```typescript
// ArkTS-Dyn
let strValue = STValue.wrapString("Hello World");
let str = strValue.unwrapToString(); // "Hello World"
```

---

### 5.3 unwrapToBoolean

`unwrapToBoolean(): boolean`

用于将STValue对象解包为布尔值，不接受任何参数，返回布尔值结果。支持基本类型的解包，引用类型暂不支持。如果值类型不支持，会抛出异常。

参数: 无

返回值:

|  类型   |      说明      |
| :-----: | :------------: |
| boolean | 解包后的布尔值 |

示例:

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let bool = boolValue.unwrapToBoolean(); // true
let intValue = STValue.wrapInt(1);
let bool1 = intValue.unwrapToBoolean(); // true
let zeroValue = STValue.wrapInt(0);
let bool2 = zeroValue.unwrapToBoolean(); // false
```

---

### 5.4 unwrapToBigInt

`unwrapToBigInt(): bigint`

用于将STValue中的大整数对象解包为BigInt类型，不接受任何参数，返回大整数值结果。支持特定的大整数类（`escompat.BigInt`），其他类型会抛出异常。

参数: 无

返回值:

|  类型  |       说明       |
| :----: | :--------------: |
| bigint | 解包后的大整数值 |

示例:

```typescript
// ArkTS-Dyn
let bigIntValue = STValue.wrapBigInt(12345678901234567890n); 
let bigInt = bigIntValue.unwrapToBigInt(); // 12345678901234567890n
```

---


## 6 STValue_wrap

### 6.1 wrapByte

`static wrapByte(value: number): STValue`

将数字包装为字节类型（8位有符号整数）的STValue对象，接受一个数字参数，返回包装后的STValue对象。如果值超出字节范围（-128到127），会抛出异常。

参数:

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
| value  | number |  是  | 要包装的数字值 |

返回值:

|  类型   |            说明             |
| :-----: | :-------------------------: |
| STValue | 包装后的字节类型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let byteValue = STValue.wrapByte(127);
let isByte = byteValue.isByte(); // true
```

---


### 6.2 wrapChar

`static wrapChar(value: number): STValue`

用于将字符串包装为字符类型（16位Unicode字符）的STValue对象，接受一个字符串参数，返回包装后的STValue对象。是将单个字符的字符串转换为字符类型，如果字符串长度不为1，会抛出异常。

参数:

| 参数名 |  类型  | 必填 |        说明        |
| :----: | :----: | :--: | :----------------: |
| value  | number |  是  | 要包装的单字节字符 |

返回值:

|  类型   |            说明             |
| :-----: | :-------------------------: |
| STValue | 包装后的字符类型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let charValue = STValue.wrapChar('A');
let isChar = charValue.isChar(); // true
```

---


### 6.3 wrapShort

`static wrapShort(value: number): STValue`


参数:

| 参数名 |  类型  | 必填 |              说明               |
| :----: | :----: | :--: | :-----------------------------: |
| value  | number |  是  | 要包装的数字值（-32768到32767） |

返回值:

|  类型   |           说明            |
| :-----: | :-----------------------: |
| STValue | 包装后的短整型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let shortValue = STValue.wrapShort(32767);
let isShort = shortValue.isShort(); // true
```

---


### 6.4 wrapInt

`static wrapInt(value: number): STValue`

用于将数字包装为整型（32位有符号整数）的STValue对象，接受一个数字参数，返回包装后的STValue对象。如果值超出整型范围（-2^31到2^31-1），会抛出异常。

参数:

| 参数名 |  类型  | 必填 |              说明               |
| :----: | :----: | :--: | :-----------------------------: |
| value  | number |  是  | 要包装的数字值（-2^31到2^31-1） |

返回值:

|  类型   |          说明           |
| :-----: | :---------------------: |
| STValue | 包装后的整型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let intValue = STValue.wrapInt(123);
let isInt = intValue.isInt(); // true
```

---


### 6.5 wrapLong

`static wrapLong(value: number): STValue`

`static wrapLong(value: BigInt): STValue`

用于将数字或大整数包装为长整型（64位有符号整数）的STValue对象，接受一个数字或BigInt参数，返回包装后的STValue对象。如果输入的number类型的值超出了整数的精度范围（-(2^53-1)到2^53-1），或者输入的值超出长整型范围（-2^63到2^63-1），会抛出异常。

参数:

| 参数名 |  类型  | 必填 |              说明               |
| :----: | :----: | :--: | :-----------------------------: |
| value  | number |  是  | 要包装的数字值（-2^63到2^63-1） |

返回值:

|  类型   |           说明            |
| :-----: | :-----------------------: |
| STValue | 包装后的长整型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let longValue = STValue.wrapLong(123);
let isLong = longValue.isLong(); // true

let longValue = STValue.wrapLong(123n);
let isLong = longValue.isLong(); // true
```

---

### 6.6 wrapFloat

`static wrapFloat(value: number): STValue`

用于将数字包装为单精度浮点型（32位浮点数）的STValue对象。针对从双精度浮点数value到单精度浮点数float的转换，实际效果和c++的`static_cast<float>(value)`相同。接受一个数字参数，返回包装后的STValue对象。

参数:

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
| value  | number |  是  | 要包装的数字值 |

返回值:

|  类型   |              说明               |
| :-----: | :-----------------------------: |
| STValue | 包装后的单精度浮点型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let floatValue = STValue.wrapFloat(3.14);
let isFloat = floatValue.isFloat(); // true
```

---


### 6.7 wrapNumber

`static wrapNumber(value: number): STValue`

用于将数字包装为双精度浮点型（64位浮点数）的STValue对象，接受一个数字参数，返回包装后的STValue对象。

参数:

| 参数名 |  类型  | 必填 |      说明      |
| :----: | :----: | :--: | :------------: |
| value  | number |  是  | 要包装的数字值 |

返回值:

|  类型   |              说明               |
| :-----: | :-----------------------------: |
| STValue | 包装后的双精度浮点型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let doubleValue = STValue.wrapNumber(3.14);
let isDouble = doubleValue.isNumber(); // true
```

---

### 6.8 wrapBoolean

`static wrapBoolean(value: boolean): STValue`

用于将布尔值包装为布尔类型的STValue对象，接受一个布尔参数，返回包装后的STValue对象，支持true和false两种值。

参数:

| 参数名 |  类型   | 必填 |      说明      |
| :----: | :-----: | :--: | :------------: |
| value  | boolean |  是  | 要包装的数字值 |

返回值:

|  类型   |            说明             |
| :-----: | :-------------------------: |
| STValue | 包装后的布尔类型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let boolValue = STValue.wrapBoolean(true);
let isBool = boolValue.isBoolean(); // true
```

---

### 6.9 wrapString

`static wrapString(value: string): STValue`

用于将字符串包装为字符串类型的STValue对象，接受一个字符串参数，返回包装后的STValue对象。

参数:

| 参数名 |  类型  | 必填 |       说明       |
| :----: | :----: | :--: | :--------------: |
| value  | string |  是  | 要包装的字符串值 |

返回值:

|  类型   |             说明              |
| :-----: | :---------------------------: |
| STValue | 包装后的字符串类型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let strValue = STValue.wrapString("Hello World");
let isStr = strValue.isString(); // true
```

---

### 6.10 wrapBigInt

`static wrapBigInt(value: bigint): STValue`

用于将ArkTS-Dyn BigInt对象包装为ArkTS-Sta BigInt类型的STValue对象，接受一个BigInt参数，返回包装后的STValue对象。


参数:

| 参数名 |  类型  | 必填 |       说明       |
| :----: | :----: | :--: | :--------------: |
| value  | bigint |  是  | 要包装的大整数值 |

返回值:

|  类型   |             说明              |
| :-----: | :---------------------------: |
| STValue | 包装后的BigInt类型STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let stBigInt = STValue.wrapBigInt(12345678901234567890n);
let isBigInt = stBigInt.isBigInt(); // true
```

---

### 6.11 getNull

`static getNull(): STValue`

用于获取表示ArkTS-Sta null的STValue对象，不接受任何参数，返回一个特殊的STValue对象。该对象在所有调用中返回相同的STValue实例。

参数: 无

返回值:

|  类型   |         说明          |
| :-----: | :-------------------: |
| STValue | 表示null的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let stNull = STValue.getNull();
let isNull = stNull.isNull(); // true
let stNull1 = STValue.getNull();
stNull === stNull1; // true
```

---

### 6.12 getUndefined

`static getUndefined(): STValue`

用于获取表示ArkTS-Sta undefined的STValue对象，不接受任何参数，返回一个特殊的STValue对象。该对象在所有调用中返回相同的STValue实例。

参数: 无

返回值:

|  类型   |            说明            |
| :-----: | :------------------------: |
| STValue | 表示undefined的STValue对象 |

示例:

```typescript
// ArkTS-Dyn
let undefValue = STValue.getUndefined();
let isUndef = undefValue.isUndefined(); // true
let undefValue1 = STValue.getUndefined();
undefValue === undefValue1;  // true
```

---