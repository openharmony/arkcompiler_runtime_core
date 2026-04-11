..
    Copyright (c) 2021-2026 Huawei Device Co., Ltd.
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    http://www.apache.org/licenses/LICENSE-2.0
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


.. _Experimental Features:

实验性特性
################################################################################

.. meta:
    frontend_status: Partly

本章介绍 |LANG| 中已经被视为语言一部分、但在 |TS| 中没有对应物的特性。
因此，对于希望让同一份源代码同时适用于 |TS| 和 |LANG| 的开发者而言，
这些特性并不推荐使用。

本章介绍的部分特性仍在讨论中，可能会从 |LANG| 规范的最终版本中移除。
一旦本章中的某个特性获得批准和/或实现，相应的小节会按适当方式迁移到
规范主体中。

:ref:`Resizable Array Creation Expressions` 中引入的数组创建特性允许程序员
在运行时通过提供以下参数来创建数组类型对象：

- 数组大小；
- 用于填充数组的单个元素，或一个用于生成一组填充值的 lambda。

这一补充对语言中的其他数组相关特性很有帮助，例如数组字面量。
它也可以用来创建数组的数组。

对函数、方法或构造函数进行重载，是编写“逻辑相似但实现不同”的程序动作的一种
实用而方便的方式。|LANG| 使用 :ref:`Explicit Overload Declarations`
作为一种创新的“受管重载”形式。

.. index::
   implementation
   array creation
   runtime expression
   array
   array literal
   constructor
   function
   method
   array type
   runtime
   array size
   function overloading
   method overloading
   implementation
   constructor overloading
   overload declaration

:ref:`Native Functions and Methods` 一节介绍了把其他语言编写的组件纳入
|LANG| 程序的、在实践中很重要也很有用的机制。

:ref:`Final Classes` 和 :ref:`Final Methods` 两节讨论了一个众所周知的特性：
它在很多 OOP 语言中都用于限制类继承和方法重写。将一个类声明为 *final*，
会禁止定义从该类派生的类；而将一个方法声明为 *final*，则会阻止它在派生类中
被重写。

:ref:`Adding Functionality to Existing Types` 一节讨论了向已经定义好的类型
添加新功能的方法。

.. index::
   native function
   native method
   function overloading
   method overloading
   final class
   final method
   object-oriented programming (OOP)
   OOP (object-oriented programming)
   inheritance

|

.. _Type char:

``char`` 类型
*******************************************************************************

.. meta:
    frontend_status: Partly

``char`` 类型的值是 16 位 Unicode 码元。
任意 Unicode 码点都可以编码为一个或两个 ``char`` 值。

.. list-table::
   :width: 100%
   :widths: 15 60
   :header-rows: 1

   * - 类型
     - 该类型的值集合
   * - char（16 位）
     - 码值从 \U+0000 到 \U+FFFF 的符号（码元）

``char`` 类型的预定义构造函数、方法和常量都是 |LANG| :ref:`Standard Library`
的一部分。

``char`` 类型是 :ref:`Standard Library` 中的一种类类型。这意味着
``char`` 是 ``Object`` 的子类型，并且可以在任何需要类名的地方使用。

.. code-block:: typescript
   :linenos:

    let a_char: char = c'a'
    console.log (a_char)
    // Output is: a
    let o: Object = a_char // OK

.. index::
   char type
   Unicode code point
   set of values
   predefined constructor
   predefined method
   predefined constant
   char type

|

.. _char Literals:

char 字面量
===============================================================================

.. meta:
    frontend_status: Done

char 字面量表示一个 16 位 Unicode 码元，它可以写成单个 UTF-16 符号，
也可以写成单个转义序列；前面必须有字符“单引号”（U+0027）和字母 c（U+0063），
后面跟一个“单引号”。

字符字面量的语法如下：

.. code-block:: abnf

      CharLiteral:
          'c\'' SingleQuoteCharacter '\''
          ;

      SingleQuoteCharacter:
          ~['\\\r\n]
          | '\\' EscapeSequence
          ;

示例如下：

.. code-block:: typescript
   :linenos:

      c'a'
      c'\n'
      c'\x7F'
      c'\u0000'

如果某个字面量无法表示为无符号 16 位值，则会发生
:index:`compile-time`：

.. code-block:: typescript
   :linenos:

      c'\u{FFFFF}' // Compile-time error


char 字面量的类型为 ``char``。

.. index::
   char literal
   value
   character
   syntax
   escape sequence
   single quote
   type char
   value

|

.. _char Operations:

``char`` 运算
===============================================================================

.. meta:
    frontend_status: Partly

相等运算符（见 :ref:`Equality Expressions`）和关系运算符
（见 :ref:`Relational Expressions`）可以在下列情况下使用：

- 两个操作数都属于 ``char`` 类型；或
- 一个操作数属于 ``char`` 类型，另一个属于数值类型
  （见 :ref:`char Conversions for Relational and Equality Operands`）；
- 否则，会发生 :index:`compile-time error`。

在第一种情况下，运算作为两个无符号 16 位值的整数比较来执行。
在第二种情况下，运算作为对应数值类型的整数比较来执行。

.. code-block:: typescript
   :linenos:

   let c: char = c'A'
   let c1 = new char
   c1 = c'A'

   // The following lines both print true as values are equal
   console.log(c == c1)  // true
   console.log(c === c1) // true

   console.log(c == 0x41) // true

   c1 = c'B'
   console.log(c < c1)  // true
   console.log(c < 0x41)  // false

   console.log(c > 3.14)  // true

|

.. _Fixed-Size Array Types:

固定大小数组类型
*******************************************************************************

.. meta:
    frontend_status: Partly

*固定大小数组类型* 写作 ``FixedArray<T>``，它是具有以下特征的内建类型：

- 任意数组类型实例都包含元素。元素个数称为 *数组长度*，可通过属性 ``length``
  访问。
- 数组长度是非负整数。
- 数组长度在运行时设置一次，之后不能更改。
- 数组元素通过其索引访问。*索引* 是从 *0* 到 *数组长度减 1* 的整数。
- 按索引访问元素是常量时间操作。
- 如果传递到非 |LANG| 环境中，数组会表示为一段连续内存位置。
- 每个数组元素的类型都必须可赋值给数组声明中指定的元素类型
  （见 :ref:`Assignability`）。

*固定大小数组* 与 *可变大小数组* 的区别如下：

- 固定大小数组的长度只设置一次，以获得更好的性能；
- 固定大小数组在 :ref:`Type Erasure` 期间会保留元素类型；
- 固定大小数组没有定义任何方法；
- 固定大小数组具有若干构造函数（见 :ref:`Fixed-Size Array Creation`）；
- 固定大小数组与 *可变大小数组* 不兼容。

下面的示例展示了可变大小数组和固定大小数组之间的不兼容性：

.. code-block:: typescript
   :linenos:

    function foo(a: FixedArray<number>, b: Array<number>) {
        a = b // Compile-time error
        b = a // Compile-time error
    }

.. index::
   resizable array
   fixed-size array
   fixed-size array type
   built-in type
   instance
   array type
   length property
   array length
   index
   runtime
   access
   index
   integer number
   constant-time operation
   memory location
   assign
   assignability
   array declaration
   compatibility
   incompatibility

|

.. _Fixed-Size Array Creation:

固定大小数组创建
===============================================================================

.. meta:
    frontend_status: Partly

*固定大小数组* 可以通过 :ref:`Array Literal` 或为 ``FixedArray<T>``
定义的构造函数来创建，其中 ``T`` 必须是 :ref:`Type Erasure`
后仍被 *保留* 的类型。

使用 *数组字面量* 创建数组如下所示：

.. code-block:: typescript
   :linenos:

    let a : FixedArray<number> = [1, 2, 3]
      /* create array with 3 elements of type number */
    a[1] = 7 /* put 7 as the 2nd element of the array, index of this element is 1 */
    let y = a[2] /* get the last element of array 'a' */
    let count = a.length // get the number of array elements
    y = a[3] // Will cause a runtime error - attempt to access non-existing array element

.. code-block:: typescript
   :linenos:

    function foo<T>(v: T): FixedArray<T | number> {
      return [v] // Compile-time error, T | number is not preserved by type erasure
    }
    let arr: FixedArray<string | number> = foo("a")

.. index::
   fixed-size array type
   array length
   array literal
   constructor
   fixed-size array
   integer
   array element
   access
   assignability
   resizable array
   runtime error

下面的构造函数创建一个指定长度、并用单个值 ``elem`` 填充的
``FixedArray<T>`` 实例：

- ``constructor(len: int, elem: T)``

.. code-block:: typescript
   :linenos:

    let a = new FixedArray<string>(3, "a") // creates array ["a", "a", "a"]

.. index::
   constructor
   array instance

|

.. _Value Array Types:

值数组类型
*******************************************************************************

.. meta:
    frontend_status: None

值数组类型是写作 ``ValueArray<T>`` 的内建类型，具有以下特征：

- 任意数组类型实例都包含 ``T`` 类型的元素。``T`` 必须是
  值类型（见 :ref:`Value Types`）。
- 元素个数称为 *数组长度*，可通过 ``length`` 属性访问。
- 数组长度是非负整数。
- 数组长度在运行时设置一次，之后不能更改。
- 数组元素通过其索引访问。*索引* 是从 *0* 到 *数组长度减 1* 的整数。
- 按索引访问元素是常量时间操作。
- 如果传递到非 |LANG| 环境中，数组会表示为一段连续内存位置，其中存放的是
  原始值而非引用。
- 每个数组元素的类型都等于数组声明中指定的元素类型。
- 除类型实参完全相同的情况外，两个 ``ValueArray`` 类型之间不存在子类型关系。

.. note::

    - ``ValueArray<T>`` 虽然使用了与泛型相同的记法，但它并不是泛型类型。

    - ``ValueArray`` 的子类型限制使它相比 :ref:`Fixed-Size Array Types`
      具有更好的性能。

.. index::
   value array type
   built-in type
   length property
   array length
   index
   runtime
   access
   index
   integer number
   constant-time operation

*值数组* 可以通过 :ref:`Array Literal` 或为 ``ValueArray<T>`` 定义的构造函数
（见下文）来创建。

使用 *数组字面量* 创建数组如下所示：

.. code-block:: typescript
   :linenos:

    let a : ValueArray<int> = [1, 2, 3]
      /* create array with 3 elements of type int */
    a[1] = 7 /* put 7 as the 2nd element of the array, index of this element is 1 */
    let y = a[2] /* get the last element of array 'a' */
    let count = a.length // get the number of array elements
    y = a[3] // runtime error, attempt to access non-existing array element

如果 ``ValueArray`` 使用了非值类型实参，则会发生
:index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    let x: ValueArray<string> = ["aa"]   // Compile-time error
    type A = ValueArray<int | undefined> // Compile-time error

下面的构造函数创建一个指定长度、并用单个值 ``elem`` 填充的
``ValueArray<T>`` 实例：

- ``constructor(len: int, elem: T)``

.. code-block:: typescript
   :linenos:

    let a = new ValueArray<double>(3, 7.) // creates array [7., 7., 7.]

.. index::
   constructor
   array instance

|

.. _Resizable Array Creation Expressions:

可变大小数组创建表达式
*******************************************************************************

.. meta:
    frontend_status: Done

数组创建表达式用于创建可变大小数组（见 :ref:`Resizable Array Types`）
的新对象实例。数组实例也可以通过 :ref:`Array literal` 创建。

数组创建表达式的语法如下：

.. code-block:: abnf

    newArrayInstance:
        'new' arrayElementType dimensionExpression '(' arrayElement ')'
        ;

    arrayElementType:
        typeReference
        | '(' type ')'
        ;

    dimensionExpression:
        '[' expression ']'
        ;

    arrayElement:
      expression
    ;

.. code-block:: typescript
   :linenos:

    let x = new number[3] (7) // create array instance: [7, 7, 7]

.. index::
   resizable array
   array creation expression
   object
   instance
   array
   array instance
   array literal
   syntax
   expression

*数组创建表达式* 会创建一个新数组对象，其元素类型由 ``arrayElementType``
指定。

*维度表达式* 的类型必须可赋值（见 :ref:`Assignability`）给 ``int`` 类型。
否则，会发生 :index:`compile-time error`。

如果 *维度表达式* 是常量表达式，且在编译时求值为负整数，则会发生
:index:`compile-time error`。

.. index::
   array creation expression
   array
   type
   dimension expression
   assignment
   conversion
   integer
   integer type
   negative integer value
   int type
   assignability
   type
   integer value
   type int
   constant expression
   compile time

``arrayElement`` ``expression`` 的类型必须可赋值（见
:ref:`Assignability`）给 ``arrayElementType``。
否则，会发生 :index:`compile-time error`。

.. index::
   dimension expression
   floating-point type
   compile-time error
   runtime error
   expression
   array element
   array dimension


.. code-block:: typescript
   :linenos:

      let x = new number[-3] (0) // Compile-time error

      let y = new number[3.14] (0) // Compile-time error

      function foo (length: int) {
         let y = new number[length] (0)  // runtime error
      }
      foo (-3)


.. index::
   class
   accessibility
   access
   parameterless constructor
   constructor
   parameter
   optional parameter
   default value

如果 ``arrayElementType`` 是类型参数，则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

      class A<T> {
         foo(element: T) {
            new T[2] (element) // Compile-time error, 'T' is a type parameter
         }
      }

.. index::
   compile-time error
   constructor
   type parameter
   array

下面给出了创建已知元素个数数组的示例：

.. code-block:: typescript
   :linenos:

      class A {
        constructor (x: number) {}
      }

      let array_size = 5

      let array = new A[array_size] (new A(1))
         /* Create array of 'array_size' elements and all of them will have
            initial value equal to an object created by new A expression */

下面给出了创建“异构”数组（其元素类型各不相同）的示例：

.. index::
   array
   array creation
   parameterless constructor
   default value
   type
   lambda function
   index

.. code-block:: typescript
   :linenos:

    let array_of_union = new (Object|undefined) [5] (undefined) // filled with undefined

    type Functor = () => void
    let array_of_functor = new Functor[5] ( (): void => {}) // filled with lambda

    type Arr = number []
    let array_of_array = new Arr [5] ( [3.14] ) // filled with array literal

|

.. _Runtime Evaluation of Array Creation Expressions:

数组创建表达式的运行时求值
===============================================================================

.. meta:
    frontend_status: Partly
    todo: initialize array elements properly - #14963, #15610

数组创建表达式在运行时按如下方式求值：

#. 对维度表达式求值。如果维度表达式的求值异常终止，那么
   *数组创建表达式* 也会异常终止。

#. 检查维度表达式的值。如果其值小于零，则抛出 ``NegativeArraySizeError``。

#. 为新数组分配空间。如果可用空间不足以分配该数组，则抛出
   ``OutOfMemoryError``，数组创建表达式的求值异常终止。

#. 然后创建一个一维数组。该数组的每个元素，要么使用传入的值进行初始化，
   要么通过调用用于生成一组值的 lambda 进行初始化。

.. index::
   runtime evaluation
   array
   array creation
   array creation expression
   evaluation
   dimension expression
   constructor
   abrupt completion
   expression
   space allocation
   class type
   runtime
   runtime evaluation
   evaluation
   initialization

|

.. _Indexable Types:

可索引类型
*******************************************************************************

.. meta:
    frontend_status: Done

如果某个类或接口声明了一个或两个名称分别为 ``$_get`` 和 ``$_set`` 的函数，
其签名分别为 ``(index: Type1): Type2`` 和 ``(index: Type1, value: Type2)``，
那么就可以对该类型的变量使用索引表达式（见 :ref:`Indexing Expressions`）：

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class SomeClass {
       $_get (index: number): SomeClass { return this }
       $_set (index: number, value: SomeClass) { }
    }
    let x = new SomeClass
    x = x[1] // This notation implies a call: x = x.$_get (1)
    x[1] = x // This notation implies a call: x.$_set (1, x)

如果只存在其中一个函数，那么只能使用与之对应的那种索引表达式形式
（见 :ref:`Indexing Expressions`）：

.. index::
   indexable type
   interface
   class
   declaration
   function name
   function
   signature
   indexing expression
   variable
   type

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    class ClassWithGet {
       $_get (index: number): ClassWithGet { return this }
    }
    let getClass = new ClassWithGet
    getClass = getClass[0]
    getClass[0] = getClass // Error - no $_set function available

    class ClassWithSet {
       $_set (index: number, value: ClassWithSet) { }
    }
    let setClass = new ClassWithSet
    setClass = setClass[0] // Error - no $_get function available
    setClass[0] = setClass

``string`` 也可以作为索引参数的类型：

.. index::
   function
   indexing expression
   string
   string type
   type
   index parameter

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class SomeClass {
       $_get (index: string): SomeClass { return this }
       $_set (index: string, value: SomeClass) { }
    }
    let x = new SomeClass
    x = x["index string"]
       // This notation implies a call: x = x.$_get ("index string")
    x["index string"] = x
       // This notation implies a call: x.$_set ("index string", x)

``$_get`` 和 ``$_set`` 是具有编译器已知签名的普通函数。
它们可以像任何其他函数一样使用。这些函数可以是抽象的，也可以先在接口中定义，
随后再实现。它们还可以被重写，从而在索引表达式求值时提供动态分派
（见 :ref:`Indexing Expressions`）。为了获得更好的灵活性，
它们也可以用于泛型类和泛型接口中。如果这些函数被标记为 ``async``，
则会发生 :index:`compile-time error`。

.. index::
   function
   ordinary function
   compiler
   compiler-known signature
   abstract function
   signature
   overriding
   interface
   implementation
   dynamic dispatch
   implementation
   indexing expression
   indexing expression evaluation
   generic class
   generic interface
   evaluation
   flexibility
   async function

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

    interface ReadonlyIndexable<K, V> {
       $_get (index: K): V
    }

    interface Indexable<K, V> extends ReadonlyIndexable<K, V> {
       $_set (index: K, value: V)
    }

    class IndexableByNumber<V> implements Indexable<number, V> {
       private data: V[] = []
       $_get (index: number): V { return this.data [index] }
       $_set (index: number, value: V) { this.data[index] = value }
    }

    class IndexableByString<V> implements Indexable<string, V> {
       private data = new Map<string, V>
       $_get (index: string): V { return this.data [index] }
       $_set (index: string, value: V) { this.data[index] = value }
    }

    class BadClass extends IndexableByNumber<boolean> {
       override $_set (index: number, value: boolean) { index / 0 }
    }

    let x: IndexableByNumber<boolean> = new BadClass
    x[42] = true // This will be dispatched at runtime to the overridden
       // version of the $_set method
    x.$_get (15)  // $_get and $_set can be called as ordinary
       // methods

|

.. _Iterable Types:

可迭代类型
*******************************************************************************

.. meta:
    frontend_status: Done

如果一个类或接口实现了 :ref:`Standard Library` 中定义的接口 ``Iterable``，
并因此拥有一个可访问的无参方法 ``$_iterator``，且其返回类型是
:ref:`Subtyping` 所定义的、在 :ref:`Standard Library` 中定义的 ``Iterator`` 类型的子类型，那么该类或接口就是
*可迭代的*。这保证了 ``$_iterator`` 方法返回的对象实现了 ``Iterator``，
从而可以遍历该 *可迭代* 类型的对象。

可迭代类型的联合类型同样是 *可迭代的*。这意味着该类类型的实例可以用于
``for-of`` 语句（见 :ref:`For-Of Statements`）。

数组类型（见 :ref:`Array Types`）和字符串类型（见 :ref:`Type string`）
也是可迭代的。

下面给出一个 *可迭代* 类 ``C`` 的示例：

.. index::
   iterable class
   class
   iterable interface
   interface
   parameterless method
   access
   accessibility
   subtyping
   subtype
   iterator
   instance
   for-of statement
   return type
   traversing
   assignability
   type Iterator
   implementation
   iterable type
   union
   for-of statement
   object

.. code-block:: typescript
   :linenos:

      class C implements Iterable<string> {
        data: string[] = ['a', 'b', 'c']
        $_iterator() { // Return type is inferred from the method body
          return new CIterator(this)
        }
      }

      class CIterator implements Iterator<string> {
        index = 0
        base: C
        constructor (base: C) {
          this.base = base
        }
        next(): IteratorResult<string> {
          return {
            done: this.index >= this.base.data.length,
            value: this.index >= this.base.data.length ? "" : this.base.data[this.index++]
          }
        }
      }

      let c = new C()
      for (let x of c) {
            console.log(x)
      }

在上面的示例中，类 ``C`` 的方法 ``$_iterator`` 返回 ``CIterator<string>``，
它实现了 ``Iterator<string>``。如果执行该代码，将输出：

.. code-block:: typescript

    "a"
    "b"
    "c"

方法 ``$_iterator`` 是具有编译器已知签名的普通方法。它可以像其他任何方法一样使用。
该方法可以是抽象的，也可以先在接口中定义，之后再实现。如果该方法被标记为
``async``，则会发生 :index:`compile-time error`。

.. index::
   type inference
   inferred type
   method
   method body
   ordinary method
   class
   iterator
   compiler-known signature
   compiler
   signature
   implementation
   async method

.. note::
   为了支持与 |TS| 兼容的代码，方法名 ``$_iterator`` 也可以写成
   ``[Symbol.iterator]``。在这种情况下，上面示例中的类 ``C`` 可写为：

   .. code-block-meta:

   .. code-block:: typescript
      :linenos:

         class C implements Iterable<string>  {
           data: string[] = ['a', 'b', 'c'];
           [Symbol.iterator]() {
             return new CIterator(this)
           }
         }

名称 ``[Symbol.iterator]`` 的用法被视为已弃用，
未来版本的语言中可能会移除。

.. index::
   compatibility
   compatible code
   name
   class
   method
   iterator
   iterable class

|

.. _Callable Types:

可调用类型
*******************************************************************************

.. meta:
    frontend_status: Partly
    todo: add $_ to names

如果一个类型的名称可以出现在调用表达式中，那么它就是 *可调用的*。
使用类型名称的调用表达式称为 *类型调用表达式*。只有类类型可以是可调用的。
若要使一个类型变为可调用，必须定义名为 ``$_invoke`` 或 ``$_instantiate``
的静态方法：

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class C {
        static $_invoke() { console.log("invoked") }
    }
    C() // prints: invoked
    C.$_invoke() // also prints: invoked

在上述示例中，``C()`` 就是 *类型调用表达式*。它是普通方法调用
``C.$_invoke()`` 的简写形式。对于 ``$_invoke`` 和 ``$_instantiate``，
显式调用始终是合法的。

一个类可以定义多个具有不同签名的 ``$_invoke`` 或 ``$_instantiate`` 实现：

..  code-block:: typescript
    :linenos:

    // OK, two $_invoke with different signatures
    class B {
        static $_invoke(p: int): int { return p; }
        static $_invoke(): string { return "hello"; }
    }

普通 :ref:`Static Methods` 不会被继承，也不能从子类调用；但是 *类型调用表达式*
允许使用超类中定义的 ``$_invoke`` 或 ``$_instantiate`` 方法，并允许对它们进行重载。
见 :ref:`Overload Set for Methods $_invoke and $_instantiate`。

如果出现以下情况，则发生 :index:`compile-time error`：

- 一个类同时定义 ``$_invoke()`` 和 ``$_instantiate`` 方法；
- 一个类定义了其中一个方法，而另一个方法定义在其超类中。

..  code-block:: typescript
    :linenos:

    class A {
        static $_instantiate(factory: () => A): A { return factory(); }
    }

    class B {
        static $_invoke(i: int): int { return i; }
    }

    // Compile-time error, both $_invoke and $_instantiate defined
    class C extends B {
        static $_instantiate(factory: () => A): A { return factory(); }
    }

|LANG| 中的静态方法无法访问泛型的类型参数。这意味着泛型类型不能声明
``$_instantiate`` 方法。``$_invoke`` 方法可以声明，但 *类型调用表达式*
或对 ``$_invoke()`` 的显式调用都不能使用类型参数。


.. index::
   callable type
   call expression
   type name
   expression
   instantiation
   invocation
   type call expression
   callable class type
   callable type
   class type
   type call expression
   method call
   inheritance
   static method
   normal method call
   call
   explicit call
   method

.. note::
   在 *new 表达式* 中被调用的只有构造函数，而不是 ``$_invoke`` 或
   ``$_instantiate``：

   .. code-block-meta:

   .. code-block:: typescript
      :linenos:

       class C {
           static $_invoke() { console.log("invoked") }
           constructor() { console.log("constructed") }
       }
       let x = new C() // constructor is called

类可以声明实例方法 ``$_invoke`` 或 ``$_instantiate``，也可以二者都声明，
但这不会使该类成为 *可调用的*。

下文将说明 ``$_invoke`` 和 ``$_instantiate`` 的相似之处与不同之处。

.. index::
   constructor
   method
   instantiation
   invocation
   call
   new expression
   callable type

|

.. _Callable Types with $_invoke Method:

具有 ``$_invoke`` 方法的可调用类型
===============================================================================

.. meta:
    frontend_status: Done

静态方法 ``$_invoke`` 可以具有任意签名。它既可以在 *类型调用表达式* 中被隐式调用，
也可以被显式调用。一个类可以拥有多个具有不同签名的 ``$_invoke`` 方法。
如果其签名包含参数，则调用时必须提供相应的实参。

.. code-block-meta:

.. code-block:: typescript
   :linenos:

    class Add {
        static $_invoke(a: number, b: number): number {
            return a + b
        }
        static $_invoke(a: string, b: string): string {
            return a + b
        }
    }
    console.log(Add(2, 2)) // prints: 4
    console.log(Add.$_invoke(2, 2)) // prints: 4
    console.log(Add("Number ", "one")) // prints "Number one"


.. index::
   static method
   invocation
   callable type
   arbitrary signature
   signature
   parameter
   method
   type call expression
   argument
   instance method
   type

|

.. _Callable Types with $_instantiate Method:

具有 ``$_instantiate`` 方法的可调用类型
===============================================================================

.. meta:
    frontend_status: Done

静态方法 ``$_instantiate`` 自身可以具有任意签名。
如果它要在 *类型调用表达式* 中使用，那么它的第一个参数必须是一个 *factory*，
即一个无参函数类型，返回声明 ``$_instantiate`` 方法的那个类类型。
该方法可以带有或不带其他参数，其他参数可以是任意的。``$_instantiate`` 的返回类型
通常与 factory 的返回类型相同，但也可以不同。一个类可以包含多个具有不同参数集的
静态 ``$_instantiate`` 方法。如果某个类声明了两个参数集相同但返回类型不同的
``$_instantiate`` 方法，则会发生 :index:`compile-time error`。

在 *类型调用表达式* 中，与 ``factory`` 参数对应的实参会被隐式传入：

.. code-block:: typescript
   :linenos:

    class C {
        // #1, parameterless
        static $_instantiate(factory: () => C): C {
            return factory()
        }

        // #2. As #1, but with another return type
        // If uncommented, then a compile-time error occurs
        // static $_instantiate(factory: () => C): int {
        //     return 1
        // }

        // #3, with string parameter
        static $_instantiate(factory: () => C, s: string): string {
            return "hello " + s
        }
    }

    let x = C() // #1 called, factory is passed implicitly

    // Explicit call of #1 requires explicit 'factory':
    let y = C.$_instantiate(() => { return new C()})

    let s: string = C("world") // #3 called, factory is passed implicitly


.. index::
   static method
   callable type
   method
   instantiation
   signature
   arbitrary signature
   type call expression
   parameter
   factory parameter
   parameterless function type
   class type
   type call expression

如果 ``$_instantiate`` 方法还有其他参数，那么调用时必须提供相应实参：

.. code-block:: typescript
   :linenos:

    class C {
        name = ""
        static $_instantiate(factory: () => C, name: string): C {
            let x = factory()
            x.name = name
            return x
        }
    }
    let x = C("Bob") // factory is passed implicitly

在 *类型调用表达式* 中，如果类型 ``T`` 满足以下任一条件，则会发生
:index:`compile-time error`：

- ``T`` 既没有 ``$_invoke`` 方法，也没有 ``$_instantiate`` 方法；或
- ``T`` 具有 ``$_instantiate`` 方法，但其第一个参数不是 ``factory``。


.. code-block-meta:
    expect-cte

.. code-block:: typescript
   :linenos:

    class C {
        static $_instantiate(factory: string): C {
            return factory()
        }
    }
    let x = C() // Compile-time error, wrong '$_instantiate' 1st parameter

当 ``$_instantiate`` 在 *类型调用表达式* 中被隐式使用时：

- 如果 ``$_instantiate`` 没有把 ``factory`` 声明为 *可选参数*，
  那么会自动生成一个 ``factory`` 实现。

- 如果 ``$_instantiate`` 将 ``factory`` 声明为 *可选参数*
  （参见 :ref:`Optional Parameters`），那么对 ``factory`` 使用其默认实现。

..  code-block:: typescript
    :linenos:

    class A {
        static $_instantiate(
            factory: () => A): A { return factory() }
    }
    class B {
        static $_instantiate(
            factory: () => B = () =>{
                console.log("default factory");
                return new B; } ): B
            { return factory() }
    }

    A() // Automatically generated factory is used
    B() // Default implementation is used for factory

*类型调用表达式* 不会向 ``factory`` 传入任何参数，而后者会使用无参类构造函数。
如果某个类没有无参构造函数，则会发生 :index:`compile-time error`：

..  code-block:: typescript
    :linenos:

    class A {
        constructor(p: int) {}
        static $_instantiate(factory: () => A): A { return factory() }
    }

    A() // Compile-time error, no parameterless constructor


..  note::
    对于这样的类，仍然可以显式调用 ``$_instantiate``，
    或为 factory 提供一个使用带参构造函数的默认实现，
    尽管这通常没有实际意义：

    ..  code-block:: typescript
        :linenos:

        class A {
            constructor(p: int) {}
            static $_instantiate(
                factory: () => A = () => { return new A(1); }
                ): A { return factory() }
        }

        A() // OK, default is used for the optional factory
        A.$_instantiate(() => { return new A(1); }) // OK, explicit call


.. index::
   method
   call
   factory
   type call expression
   instantiation
   invocation
   parameter
   callable type
   instance method
   instance

|

.. _Overload Set for Methods $_invoke and $_instantiate:

方法 ``$_invoke`` 和 ``$_instantiate`` 的重载集合
===============================================================================

.. meta:
    frontend_status: None

名称 ``$_invoke`` 或 ``$_instantiate`` 的重载集合由以下内容形成：

- 隐式重载的静态方法（见 :ref:`Implicit Method Overloading`）；
- :ref:`Explicit Class Method Overload` 中列出的显式重载静态方法；
- 直接超类中的静态方法（如果存在）。

形成重载集合时按以下步骤处理：

#. 当前类中定义的显式和隐式重载方法按照 :ref:`Forming an Overload Set`
   描述的顺序加入重载集合，包括覆盖超类方法的方法。

#. 直接超类的重载集合（如果存在）追加到当前类重载集合末尾。
   已经加入重载集合的方法不会再次加入。

上述步骤与 :ref:`Overload Set for Class Instance Methods` 中描述的步骤接近，
区别在于：

- 只考虑名称为 ``$_invoke`` 或 ``$_instantiate`` 的静态方法；
- 不考虑接口中的方法，因为接口中不允许静态方法。

下面的示例展示了 ``$_invoke`` 的 *重载集合* 如何形成：

..  code-block:: typescript
    :linenos:

    class A {
        static $_invoke(i: int) { console.log(i) } // #1
    }
    class B extends A {
        static $_invoke(s: string) { console.log(s) } // #2
        // The overload set for '$_invoke' is {$_invoke#2, $_invoke#1}
    }

    B(42)    // Calls A.$_invoke, output: 42
    B("abc") // Calls B.$_invoke, output: "abc"

|

.. _Statements Experimental:

实验性语句
******************************************************************************

.. meta:
    frontend_status: Done

|

.. _For-of Explicit Type Annotation:

For-of 显式类型注解
===============================================================================

.. meta:
    frontend_status: Partly
    todo: check assignability

ForVariable（参见 :ref:`For-Of Statements`）允许显式类型注解：

.. code-block:: typescript
   :linenos:

      // explicit type is used for a new variable,
      let x: string[] = ["aaa", "bbb", "ccc"]
      for (let str: string of x) {
        console.log(str)
      }

``for-of`` 表达式中的元素类型必须可赋值（参见 :ref:`Assignability`）
给该变量的类型。否则，会发生 :index:`compile-time error`。

.. index::
   type annotation
   annotation
   for-variable
   expression
   assignability
   variable
   for-of type statement

|

.. _Explicit Overload Declarations:

显式重载声明
******************************************************************************

.. meta:
    frontend_status: None

|LANG| 支持传统的函数、方法和构造函数重载
（即同名实体的隐式重载），同时也支持一种创新性的显式重载声明形式，
允许开发者显式指定一组重载实体，并控制重载决议过程。

无论使用隐式重载还是显式重载，最终被调用的实体都在编译时确定。
因此，*重载* 与 *按名称的编译时多态* 有关。语义细节见 :ref:`Overloading`。

.. index::
    polymorphism
    polymorphism by name
    entity
    overloading
    overloaded entity
    compile time
    compatibility
    semantics

*显式重载声明* 可用于：

- 函数（见 :ref:`Explicit Function Overload`），包括命名空间中的函数；
- 类或接口方法（见 :ref:`Explicit Class Method Overload` 和
  :ref:`Explicit Interface Method Overload`）。

*重载声明* 以关键字 ``overload`` 开始，并声明一个 *有序重载集合*。
该声明的确切语法在相应的小节中给出。

.. index::
    overload declaration
    overloaded entity
    entity
    function
    method
    constructor
    overload declaration
    namespace
    class method
    interface method
    method declaration
    overload keyword
    overload set
    entity

下面的示例展示了显式重载声明的使用方式：

.. code-block:: typescript
   :linenos:

    function max2(a: number, b: number): number {
        return  a > b ? a : b
    }
    function maxN(...a: number[]): number {
        // return max element
    }

    // declare 'max' as an ordered set of functions max2 and maxN
    overload max { max2, maxN }

    max(1, 2)     // max2 is called
    max(3, 2, 4)  // maxN is called
    max("a", "b") // Compile-time error, no function to call

被纳入 *重载集合* 的实体，其语义不会改变。
这些实体仍遵循普通的可访问性规则，也可以像下面这样显式调用：

.. code-block:: typescript
   :linenos:

    maxN(1, 2) // maxN is explicitly called
    max2(2, 3) // max2 is explicitly called

调用 *显式重载* 时，会按列出的顺序检查 *重载集合* 中的实体，
并调用第一个签名匹配的实体（见 :ref:`Overload Resolution`）。
如果不存在任何签名适配的实体，则会发生 :index:`compile-time error`：

.. index::
    function
    semantics
    entity
    overload
    accessibility
    overload set
    overload resolution
    signature

.. code-block-meta:
    expect-cte

.. code-block:: typescript
   :linenos:

    max(1)    // maxN is called
    max(1, 2) // max2 is called, as is the first appropriate in the set

    max("a", "b") // Compile-time error, no function to call

每个 *显式重载声明* 中的标识符都必须且只能表示一个可访问实体。

如果某个标识符表示的是一个隐式重载名称，则会发生
:index:`compile-time error`。

显式重载声明永远不会把一个隐式重载名称展开成多个实体。

显式重载声明中的被重载实体可以是泛型（见 :ref:`Generics`）。

如果在调用被重载实体时显式提供了类型实参（见
:ref:`Explicit Generic Instantiations`），那么在 :ref:`Overload Resolution`
过程中，只会考虑那些“类型实参数量与必需和可选类型参数数量兼容”的实体
（即，具有可选类型参数的实体，是指那些带有类型参数默认值的实体）：

.. code-block:: typescript
   :linenos:

    // Resolution with explicit type arguments
    function one<T>() { console.log("one") }
    function two<T, U=string>() { console.log("two") }
    function three<T, U=string, V=number>() { console.log("three") }

    overload numbers { one, two, three }

    numbers<string, number, int>() // Only 'three` considered

    numbers<string, number>() // 'two' and 'three; considered as both
                              // allow 2 type arguments

    numbers<int>()  // 'one', 'two' and 'three; considered as all
                    // allow 1 type argument


如果没有显式提供类型实参（见
:ref:`Implicit Generic Instantiations`），那么会考虑所有实体，
如下例所示：

.. index::
    entity
    call
    call site
    function call
    overloaded entity
    overload declaration
    generic
    generic instantiation
    type argument
    type parameter
    overload resolution

.. code-block:: typescript
   :linenos:

    function foo1(s: string) {}
    function foo2<T>(x: T) {}

    overload foo { foo1, foo2 }

    foo("aa")   // foo1 is called
    foo(1) // foo2 is called, implicit generic instantiation
    foo<string>("aa") // foo2 is called

一个实体可以列在多个 *显式重载声明* 中：

.. code-block:: typescript
   :linenos:

    function max2i(a: int, b: int): int {
        return  a > b ? a : b
    }
    function maxNi(...a: int[]): int {
        // return max element
    }
    function maxN(...a: number[]): number {
        // return max element
    }

    overload maxi { max2i, maxNi }
    overload max { max2i, maxNi, maxN }

.. index::
    entity
    function
    overload declaration
    generic instantiation

|

.. _Explicit Function Overload:

显式函数重载
===============================================================================

.. meta:
    frontend_status: None

*显式函数重载* 允许为一组函数声明一个名称（见 :ref:`Function Declarations`）。
其语法如下：

.. code-block:: abnf

    explicitFunctionOverload:
        'overload' identifier overloadList
        ;
    overloadList:
        '{' identifier (',' identifier)* ','? '}'
        ;

.. index::
    explicit function overload
    set of functions
    function declaration
    function
    syntax
    qualified name

如果列表中的某个 *identifier*：

- 没有指向任何可访问函数；
- 指向一个隐式重载的函数名；或
- 指向一个非函数实体；

则会发生 :index:`compile-time error`。

所有被重载函数必须位于同一个模块或命名空间作用域（见 :ref:`Scopes`）。
否则会发生 :index:`compile-time error`。错误示例如下：

.. code-block:: typescript
   :linenos:

    import {foo1} from "something"

    function foo2() {}
    overload foo {foo1, foo2} // Compile-time error

    namespace N {
        export function fooN() {}
        namespace M {
            export function fooM() {}
        }
        overload goo {M.fooM, fooN} // Compile-time error
    }
    overload bar {foo2, N.fooN} // Compile-time error

.. index::
    overloaded function
    module
    namespace
    namespace scope
    scope
    overload declaration
    import

*显式函数重载* 的名称可以与同一作用域中的某个函数名称相同，
前提是该名称只表示一个可访问函数，且该函数已出现在重载列表中。
如果该名称表示的是一个隐式重载函数集，则会发生 :index:`compile-time error`。
如下所示：

.. code-block:: typescript
   :linenos:

    function foo(n: number): number {/*body1*/}
    function fooString(s: string): string {/*body2*/}

    overload foo {foo, fooString} // valid overload

    foo(1)    // function 'foo' is called
    foo("aa") // function 'fooString' is called

    function bar(): void {}

    // Invalid overload, as 'bar' does not appear in the list:
    overload bar {foo, fooString} // Compile-time error

    function baz(n: number): number {/*body1*/}
    function baz(s: string): string {/*body2*/}

    // Invalid overload, as 'baz' denotes an implicitly overloaded function set:
    overload baz {baz, fooString} // Compile-time error

    let name: string = "abc"

    // Invalid overload, as 'name' refers to a variable:
    overload name {foo, fooString, name} // Compile-time error

把某个函数名用作显式重载的名称不会造成歧义，因为它只在调用点才会被当作
显式重载处理。也就是说，下列场景 **不会** 把 *显式重载* 的名称纳入考虑：

- 被重载实体列表；

- :ref:`Function Reference`。

.. index::
    name
    overloaded function
    function
    entity
    function reference

.. code-block:: typescript
   :linenos:

    function foo(n: number): number {/*body1*/}
    function fooString(s: string): string {/*body2*/}
    overload foo {foo, fooString}

    let func1 = foo // 'foo' refers to function, not to explicit overload

如果某个 *显式重载* 被导出，而其被重载函数中有一个没有导出，则会发生
:index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    export function foo1(p: string) {}
    function foo2(p: number) {}
    export overload foo { foo1, foo2 } // Compile-time error, 'foo2' is not exported
    overload bar { foo1, foo2 } // OK, as 'bar' is not exported

如果对一个 *显式重载* 使用“带接收者的函数调用”形式，也就是使用方法调用语法
（见 :ref:`Functions with Receiver`），则会发生
:index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    function bar1(this: string) {}
    function bar2(this: number) {}

    overload bar { bar1, bar2 }

    let s: string = "";
    let n: number = 1;

    bar(s) // OK
    bar(n) // OK
    s.bar()  // Compile-time error
    n.bar()  // Compile-time error

|

.. _Explicit Class Method Overload:

显式类方法重载
===============================================================================

.. meta:
    frontend_status: None

*显式类方法重载* 允许为一组类方法声明一个名称
（见 :ref:`Method Declarations`）。其语法如下：

.. code-block:: abnf

    explicitClassMethodOverload:
        explicitClassMethodOverloadModifier*
        'overload' identifier overloadList
        ;

    explicitClassMethodOverloadModifier: 'static' | 'async';

下面的示例展示了 *显式类方法重载* 的用法：

.. index::
    class method
    class member
    static method
    instance method
    method
    explicit class method overload
    syntax
    set of methods
    identifier

.. code-block:: typescript
   :linenos:

    class Processor {
        overload process { processNumber, processString }
        processNumber(n: number) {/*body*/}
        processString(s: string) {/*body*/}
    }

    let c = new C()
    c.process(42) // calls processNumber
    c.process("aa") // calls processString

*显式方法重载* 的名称可以与同一类中的某个方法名称相同
（见 :ref:`Explicit Overload Name Same As Method Name`）。

下面是 *静态显式重载* 的示例：

.. code-block:: typescript
   :linenos:

    class C {
        static one(n: number) {/*body*/}
        static two(s: string) {/*body*/}
        static overload foo { one, two }
    }

如果出现以下任一情况，则会发生 :index:`compile-time error`：

- 方法修饰符在显式方法重载中重复出现；
- 重载方法列表中的 *Identifier* 没有指向当前类的任何可访问方法
  （无论是声明的还是继承的）；
- 重载方法列表中的 *Identifier* 指向一个隐式重载的方法名；
- *显式重载*：

  - 被声明为 *静态*，但其被重载方法是 *非静态*；
  - 被声明为 *非静态*，但其被重载方法是 *静态*；
  - 被标记为 ``async``，但其被重载方法不是；
  - 没有标记为 ``async``，但其被重载方法是。


.. index::
    method modifier
    explicit method overload
    identifier
    accessible method
    declaration
    inheritance
    overloaded method

*显式重载* 与其被重载方法可以具有不同的访问修饰符。
但在下列情况下会发生 :index:`compile-time error`：

- *显式重载* 为 ``public``，但至少一个被重载方法不是 ``public``；
- *显式重载* 为 ``protected``，但至少一个被重载方法是 ``private``。


下面的示例给出了合法与非法的显式重载：

.. index::
    overloaded method
    explicit overload
    access modifier
    public
    protected
    private

.. code-block:: typescript
   :linenos:

    class C {
        private foo1(x: number) {/*body*/}
        protected foo2(x: string) {/*body*/}
        public foo3(x: boolean) {/*body*/}
        foo4() {/*body*/} // implicitly public

        public overload foo { foo3, foo4 } // OK
        protected overload bar { foo2, foo3 } // OK
        private overload goo { foo1, foo2, foo3 } // OK

        public overload err1 {foo2, foo3} // Compile-time error, foo2 is not public
        protected overload err2 {foo2, foo1} // Compile-time error, foo1 is private
    }

部分或全部被重载函数可以是 ``native``：

.. code-block:: typescript
   :linenos:

    class C {
        native foo1(x: number)
        foo2(x: string) {/*body*/}
        overload foo { foo1, foo2 }
    }

.. index::
    public
    overload
    private
    overloaded function
    native

如果某个超类拥有 *显式重载*，则该声明可以在子类中被重写。
如果子类没有重写该 *显式重载*，那么会继承来自超类的该重载。

如果子类重写了一个 *显式重载*，那么该声明必须列出超类中该 *显式重载*
包含的全部方法；否则会发生 :index:`compile-time error`。

此外，子类中的 *显式重载* 在重写时还可以加入新方法，并改变方法顺序。

*显式重载* 的使用方式与普通类方法类似，只不过在编译时，
它会根据 *对象引用* 的类型被替换成某个具体的被重载方法。
下面的示例展示了子类型中的 *显式重载*：

.. index::
    superclass
    overload declaration
    overriding
    subclass
    inheritance
    declaration
    superclass
    overloaded method
    object reference
    method

.. code-block:: typescript
   :linenos:

    class Base {
        overload process { processNumber, processString }
        processNumber(n: number) {/*body*/}
        processString(s: string) {/*body*/}
    }

    class D1 extends Base {
        // method is overridden
        override processNumber(n: number) {/*body*/}
        // overload declaration is inherited
    }

    class D2 extends Base {
        // method is added:
        processInt(n: int) {/*body*/}
        // new order for overloaded methods is specified:
        overload process { processInt, processNumber, processString }
    }

    new D1().process(1)   // calls processNumber from D1

    new D2().process(1)   // calls processInt from D2 (as it is listed earlier)
    new D2().process(1.0) // calls processNumber from Base (first appropriate)

带有特殊名称的方法（见 :ref:`Indexable Types`、:ref:`Iterable Types`
和 :ref:`Callable Types`）也可以像普通方法一样进行重载：

.. index::
    overloaded method
    overriding
    method
    name
    iterable type
    callable type
    inheritance
    ordinary method
    name

.. code-block:: typescript
   :linenos:

    class C {
        getByNumber(n: number): string {...}
        getByString(s: string): string {...}
        overload $_get { getByNumber, getByString }
    }

    let c = new C()

    c[1]     // getByNumber is used
    c["abc"] // getByString is used

如果一个类实现了某些具有同名 *显式重载* 的接口，那么新的 *显式重载*
必须包含所有被重载方法；否则会发生 :index:`compile-time error`。

.. index::
    overloaded method
    class
    interface
    overload declaration
    alias

.. code-block:: typescript
   :linenos:

    interface I1 {
        overload foo {f1, f2}
        // f1 and f2 are declared in I1
    }
    interface I2 {
        overload foo {f3, f4}
        // f3 and f4 are declared in I2
    }
    class C implements I1, I2 {
       // Compile-time error as no new overload is defined
    }
    class D implements I1, I2 {
        overload foo { f2, f3, f1, f4 } // OK, as new overload is defined
    }
    class E implements I1, I2 {
        overload foo { f2, f4 } // Compile-time error as not all methods are used
    }

    const i1: I1 = new D
    i1.foo(<arguments>) // call is valid if arguments fit first signature of {f1, f2} set

    const i2: I2 = new D
    i2.foo(<arguments>) // call is valid if arguments fit first signature of {f3, f4} set

    const d: D = new D
    d.foo(<arguments>) // call is valid if arguments fit first signature of {f2, f3, f1, f4} set

.. index::
    overloaded interface
    declaration
    method
    argument
    signature

|

.. _Explicit Interface Method Overload:

显式接口方法重载
===============================================================================

.. meta:
    frontend_status: None

*显式接口方法重载* 允许为一组接口方法声明一个名称
（参见 :ref:`Interface Method Declarations`）。其语法如下：

.. code-block:: abnf

    explicitInterfaceMethodOverload:
        'overload' identifier overloadList
        ;

*显式接口方法重载* 列表中的每个标识符都必须且只能表示一个可访问接口方法。

如果某个标识符没有指向任何可访问方法、指向非方法实体，或者指向隐式重载方法名，
则会发生 :index:`compile-time error`。

下面的示例展示了 *显式接口方法重载* 的用法：

.. code-block:: typescript
   :linenos:

    interface I {
        foo(): void
        bar(n?: string): void
        overload goo { foo, bar }
    }

    function example(i: I) {
        i.goo()        // calls i.foo()
        i.goo("hello") // calls i.bar("hello")
        i.bar()        // explicit call: i.bar(undefined)
    }

.. index::
    interface method
    explicit overload
    interface

*显式方法重载* 的名称可以与同一接口中的某个方法名称相同
（详见 :ref:`Explicit Overload Name Same As Method Name`）。

*显式重载* 的使用方式与普通接口方法类似，只不过在调用时，
会在编译期根据 *对象引用* 的类型将其替换为某个被重载方法。

*显式接口重载* 可以在类中被重写。在这种情况下，类中的 *显式重载*
必须包含接口中被重载的所有方法；否则会发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

   class D implements I {
        foo(): void {/*body*/}
        bar(n?: string): void {/*body*/}
        overload goo( bar, foo) // order is changes
   }

   let d = new D()
   d.goo() // d.bar(undefined) is used, as it is the first appropriate method

如果某个类没有重写接口中声明的 *显式重载*，
那么它会继承该重载：

.. code-block:: typescript
   :linenos:

   // Using interface overload declaration
   class C implements I {
        foo(): void {/*body*/}
        bar(n?: string): void {/*body*/}
   }

   let c = new C()
   c.goo() // calls c.foo()

.. index::
    ordinary method
    interface method
    call
    compile time
    overloaded method
    object reference
    type
    class
    implementation

定义在超接口中的 *显式重载* 可以在子接口中被重写。
此时，子接口中的 *重载声明* 必须包含超接口中该重载的全部方法；
否则会发生 :index:`compile-time error`。

如果一个接口继承了多个同名 *显式重载*，那么它必须在子接口中重写这些
*显式重载*；否则会发生 :index:`compile-time error`。

.. index::
    interface
    class
    explicit overload
    superinterface
    method
    subinterface
    overloaded method
    interface
    override
    inheritance

.. code-block:: typescript
   :linenos:

    interface I1 {
        overload foo {f1, f2}
        // f1 and f2 are declared in I1
    }
    interface I2 {
        overload foo {f3, f4}
        // f3 and f4 are declared in I2
    }
    interface I3 extends I1, I2 {
       // Compile-time error as no new overload for 'foo' is defined
    }
    interface I4 extends I1, I2 {
        overload foo { f4, f1, f3, f2 } // OK, as new overload is defined
    }
    interface I5 extends I1, I2 {
        overload foo { f1, f3 } // Compile-time error as not all methods are included
    }

|

.. _Explicit Overload Name Same As Method Name:

显式重载名与方法名相同
===============================================================================

.. meta:
    frontend_status: None

类或接口中的 *显式重载* 名称可以与被重载方法的名称相同。
例如，定义在超类中的某个方法，可以作为同名子类 *显式重载* 中的一个被重载方法。
这一重要情况如下所示：

.. code-block:: typescript
   :linenos:

    class C {
        foo(n: number): number {/*body*/}
    }
    class D extends C {
        fooString(s: string): string {/*body*/}

        overload foo {
            foo, // method 'foo' from C
            fooString
        }
    }

    let d = new D()
    let c: C = d

    d.foo(1)    // 'foo' from C is called
    d.foo("aa") // 'fooString' from D is called
    c.foo(1)    // method 'foo' from is called (no overload)

.. index::
    method name
    explicit overload
    overloaded method
    superclass
    subclass

如果方法名和 *显式重载* 名称相同，那么该方法仍可按通常规则被重写：

.. code-block:: typescript
   :linenos:

    class C {
        foo(n: number): number {/*body*/}
    }
    class D extends C {
        foo(n: number): number {/*body*/} // method is overridden
        fooString(s: string): string {/*body*/}

        overload foo { foo, fooString }
    }

这一特性同样适用于接口之间，或接口与实现该接口的类之间：

.. index::
    method
    name
    method name
    overriding
    overridden method
    interface
    class
    implementation

.. code-block:: typescript
   :linenos:

    interface I {
        foo(n: number): number {/*body*/}
    }
    interface J extends I {
        fooString(s: string): string
        overload foo { foo, fooString }
    }

    class K implements I {
        foo(n: number): number {/*body*/}
        fooString(s: string): string {/*body*/}

        overload foo { foo, fooString }
    }

使用 *显式重载* 不会带来歧义，因为它只在调用点才会被考虑。
在下列场景中，*显式重载* 名称 **不会** 被纳入考虑：

- :ref:`Overriding`；
- 被重载实体列表（见 :ref:`Explicit Class Method Overload`
  和 :ref:`Explicit Interface Method Overload`）；
- :ref:`Method Reference`。

.. index::
    number
    interface
    string
    overload
    call site
    overriding
    overloaded entity
    method reference
    class method overload declaration
    method reference

.. code-block:: typescript
   :linenos:

    class C {
        foo(n: number): number {/*body*/}
    }

    class D extends C {
        fooString(s: string): string {/*body*/}

        overload foo { foo, fooString }
    }

    let d = new D()
    let c: C = d

    let func1 = c.foo // method 'foo' is used
    let func2 = d.foo // method 'foo' is used

如果某个 *显式重载* 的名称与一个方法名称相同（且静态/非静态修饰符也相同），
但该方法没有列在被重载方法列表中，则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    class C {
        foo(n: number) {/*body*/}
        fooString(s: string) {/*body*/}
        fooBoolean(b: boolean) {/*body*/}

        overload foo { // Compile-time error
            fooBoolean, fooString
        }
    }

.. index::
    number
    string
    method
    static modifier
    non-static modifier
    overloaded method

|


.. _Native Functions and Methods:

原生函数与方法
******************************************************************************

.. meta:
    frontend_status: Done


.. _Native Functions:

原生函数
===============================================================================

.. meta:
    frontend_status: Done

*原生函数* 是用关键字 ``native`` 标记的函数
（参见 :ref:`Function Declarations`）。

*原生函数* 通常在与平台相关的代码中实现，并且往往使用其他编程语言
（例如 *C*）编写。如果原生函数带有函数体，则会发生
:index:`compile-time error`。

.. index::
   native keyword
   function
   native function
   native method
   function body

|

.. _Native Methods Experimental:

原生方法
===============================================================================

.. meta:
    frontend_status: Done

*原生方法* 是用关键字 ``native`` 标记的方法
（参见 :ref:`Method Declarations`）。

*原生方法* 是在平台相关代码中实现的方法，通常使用其他编程语言
（例如 *C*）编写。

如果满足以下任一条件，则会发生 :index:`compile-time error`：

- 方法声明同时包含 ``abstract`` 和 ``native`` 关键字；
- *原生方法* 具有函数体（参见 :ref:`Method Body`），且其形式为代码块，
  而不是单个分号或空体。


.. index::
   native method
   method
   implementation
   platform-dependent code
   native keyword
   method body
   block
   method declaration
   abstract keyword
   semicolon
   empty body

|

.. _Native Constructors:

原生构造函数
===============================================================================

.. meta:
    frontend_status: Done

*原生构造函数* 是用关键字 ``native`` 标记的构造函数
（参见 :ref:`Constructor Declaration`）。

*原生构造函数* 是在平台相关代码中实现的构造函数，通常使用其他编程语言
（例如 *C*）编写。

如果 *原生构造函数* 具有非空函数体（参见 :ref:`Constructor Body`），
则会发生 :index:`compile-time error`。

.. index::
   native constructor
   constructor
   constructor declaration
   platform-dependent code
   native keyword
   implementation
   non-empty body

|

.. _Classes Experimental:

实验性类特性
******************************************************************************

.. meta:
    frontend_status: Done


.. _Final Classes:

最终类
===============================================================================

.. meta:
    frontend_status: Done

类可以声明为 ``final`` 以阻止扩展。也就是说，被声明为 ``final`` 的类不能拥有
子类。``final`` 类中的任何方法都不能被重写。

如果类类型表达式 ``F`` 被声明为 *final*，那么它的值只能是类 ``F`` 的对象。

如果某个类声明的 ``extends`` 子句中包含另一个被声明为 ``final`` 的类，
则会发生 :index:`compile-time error`。

.. index::
   final class
   class
   class type
   subclass
   object
   extension
   method
   overriding
   class
   class extension
   extends clause
   class declaration

|

.. _Final Methods:

最终方法
===============================================================================

.. meta:
    frontend_status: Done

方法可以声明为 ``final``，以阻止它在子类中被重写
（参见 :ref:`Overriding Methods`）。

如果出现以下任一情况，则会发生 :index:`compile-time error`：

- 方法声明同时包含 ``abstract`` 或 ``static`` 关键字与 ``final`` 关键字；
- 被声明为 ``final`` 的方法被重写。

.. index::
   final method
   overriding
   instance method
   final method
   overridden method
   subclass
   method declaration
   abstract keyword
   static keyword
   final keyword

.. |
   .. _Sealed Classes:
   Sealed Classes
   ==============
   .. meta:
   frontend_status: None
   A class can be declared ``sealed`` to prevent an extension outside of the
   current module or namespace, i.e., a class declared ``sealed`` can have
   subclasses only within its module or namespace. It limits the number of
   subclasses to subclasses defined within the same module or namespace.
   A ``sealed`` class is ``final`` outside of its module or namespace.
   A :index:`compile-time error` occurs if the ``extends`` clause of a class
   declaration outside of the current module or namespace contains another class
   that is ``sealed``.
   .. code-block:: typescript
   :linenos:
   // File1
   export sealed class A{}
   class B extends A {} // OK as A and B are in the same module
   export namespace X {
   export sealed class A{}
   class B extends A {} // OK as A and B are in the same scope
   }
   // File2
   import {A, X.A} from "File1"
   class C extends A {} // Compile-time error, A is final while imported
   .. index::
   sealed class
   class
   class type
   extension
   namespace
   module
   subclass
   class extension
   extends clause
   class declaration

|

.. _Default Interface Method Declarations:

默认接口方法声明
******************************************************************************

.. meta:
    frontend_status: Done

*接口默认方法* 的语法如下：

.. code-block:: abnf

    interfaceDefaultMethodDeclaration:
        'private'? identifier signature block
        ;

默认方法可以在接口体中显式声明为 ``private``。

接口中表示默认方法体的代码块，为那些没有重写实现该接口方法的类提供默认实现。

.. index::
   method declaration
   interface method declaration
   default method
   private method
   implementation
   interface
   block
   class
   method body
   interface body
   default implementation
   overriding
   syntax

|

.. _Adding Functionality to Existing Types:

为现有类型添加功能
******************************************************************************

.. meta:
    frontend_status: Done

|LANG| 支持向已经定义好的类型添加函数和访问器。这类函数的使用方式看起来就像
它们是该类型的方法和访问器一样。这一机制称为 :ref:`Functions with Receiver`。
这一特性常用于在不继承某个类、也不实现某个接口的前提下，为该类或接口新增功能。
不过，它不仅适用于类和接口，也可用于其他类型。

此外，还可以定义并使用 :ref:`Function Types with Receiver` 和
:ref:`Lambda Expressions with Receiver` 来提高代码灵活性。

.. index::
   functionality
   function
   type
   accessor
   method
   function with receiver
   interface
   inheritance
   class
   implementation
   function type
   lambda expression
   lambda expression with receiver
   flexibility

|

.. _Functions with Receiver:

带接收者的函数
===============================================================================

.. meta:
    frontend_status: Done

*带接收者的函数* 声明是一种顶层声明（见 :ref:`Top-Level Declarations`），
其形式与 :ref:`Function Declarations` 几乎完全相同，唯一区别是：
第一个必选参数必须使用关键字 ``this`` 作为名称。

*带接收者的函数* 的语法如下：

.. code-block:: abnf

    functionWithReceiverDeclaration:
        'function' identifier typeParameters? signatureWithReceiver block
        ;

    signatureWithReceiver:
        '(' receiverParameter (', ' parameterList)? ')' returnType?
        ;

    receiverParameter:
        annotationUsage? 'this' ':' type
        ;

.. index::
   function with receiver
   function with receiver declaration
   declaration
   top-level declaration
   function declaration
   parameter
   this keyword

调用 *带接收者的函数* 有两种方式：

- 作为普通函数调用（见 :ref:`Function Call Expression`），
  此时第一个实参就是接收者对象；
- 作为方法调用（见 :ref:`Method Call Expression`），
  此时接收者作为函数名前的 ``objectReference`` 传入，并成为调用的第一个参数。

所有其他参数都按普通方式处理。

.. index::
   function with receiver
   function call
   expression
   parameter
   method call
   method call expression
   derived class
   derived interface
   argument
   object reference
   receiver
   function name

关键字 ``this`` 只能用于参数列表中的第一个参数。如果它用于其他参数，
则会发生 :index:`compile-time error`。

在 *带接收者的函数* 内部可以使用关键字 ``this``，
它对应于第一个参数。参数 ``this`` 的类型称为 *接收者类型*
（见 :ref:`Receiver Type`）：

.. code-block:: typescript
   :linenos:

      class A {
        num: number = 1
        foo(): void { console.log(this.num); }
      }
      function bar(this: A) {
        this.num = 5
      }
      let a = new A()
      a.foo() // method is called
      a.bar() // Function with receiver is called
      a.foo() // method is called

名称为 ``this`` 的第一个参数是只读的。

如果 *接收者类型* 是类或接口类型，那么在 *带接收者的函数* 的函数体中，
``private`` 或 ``protected`` 成员不可访问（见 :ref:`Accessible`），
只有 ``public`` 成员可访问：

.. index::
   this keyword
   function with receiver
   receiver type
   type parameter
   call
   interface type
   public member
   private member
   protected member
   access
   accessibility
   parameter

.. code-block:: typescript
   :linenos:

      class A {
          foo () { ... this.bar() ... }
                       // function bar() is accessible here
          protected member_1 ...
          private member_2 ...
      }
      function bar(this: A) { ...
         this.foo() // Method foo() is accessible as it is public
         this.member_1 // Compile-time error as member_1 is not accessible
         this.member_2 // Compile-time error as member_2 is not accessible
         ...
      }
      let a = new A()
      a.foo() // Ordinary class method is called
      a.bar() // Function with receiver is called

派生类或派生接口也可以作为接收者：

.. code-block:: typescript
   :linenos:

      class C {}

      function foo(this: C) {}
      function bar(this: C, n: number): void {}

      let c = new C()

      // as a function call:
      foo(c)
      bar(c, 1)

      // as a method call:
      c.foo()
      c.bar(1)

      interface D {}
      function foo1(this: D) {}
      function bar1(this: D, n: number): void {}

      function demo (d: D) {
         // as a function call:
         foo1(d)
         bar1(d, 1)

         // as a method call:
         d.foo1()
         d.bar1(1)
      }

      class E implements D {}
      const e = new E

      // derived class is used as a receiver for a method call:
      e.foo1()
      e.bar1(1)

      // the same as a function call:
      foo1(e)
      bar1(e, 1)

*带接收者的函数* 可以是泛型，如下所示：

.. index::
   function with receiver
   access
   accessibility
   instance method
   derived class
   name
   method
   receiver type
   generic function

.. code-block:: typescript
   :linenos:

    class G<T> {}

    function foo<T>(this: G<T>, p: T) {
        console.log (p)
    }

    class C {}

    let g = new G<C>
    g.foo(new C)    // implicit instantiation
    g.foo<C>(new C) // explicit instantiation

如果接收者类型中存在一个与带接收者函数同名的可访问实例方法
（见 :ref:`Accessible`），那么实例方法优先于隐式调用的带接收者函数。
但该带接收者函数仍然可以显式调用：

.. code-block:: typescript
   :linenos:

      class A {
          foo (): int { return 1; }
      }

      function foo(this: A): int { return 2; }

      console.log((new A).foo())  // instance method called, prints '1'
      console.log(foo(new A)) // explicit call of a receiver function, prints '2'


*带接收者的函数* 使用静态分派。调用哪个函数，在编译时就根据声明中的接收者类型确定。
某个 *带接收者的函数* 可以应用于任意派生类的接收者，直到它在该派生类中被重写：

.. code-block:: typescript
   :linenos:

      class Base { ... }
      class Derived extends Base { ... }

      function foo(this: Base) { console.log ("Base.foo is called") }

      let b: Base = new Base()
      b.foo() // `Base.foo is called` to be printed
      b = new Derived()
      b.foo() // `Base.foo is called` to be printed

*带接收者的函数* 可以定义在与接收者类型定义所在模块不同的模块中。
如下例所示：

.. index::
   function with receiver
   static dispatch
   function call
   compile time
   receiver type
   declaration
   receiver
   derived class
   class
   module

.. code-block:: typescript
   :linenos:

      // file a.ets
      class A {
          foo() { ... }
      }

      // file ext.ets
      import {A} from "a.ets" // name 'A' is imported
      function bar(this: A) () {
         this.foo() // Method foo() is called
      }

*带接收者的函数* 也可以定义在命名空间中（见 :ref:`Namespace Declarations`）。
不过，在命名空间外部不能使用 *方法调用* 语法来调用它，因为从命名空间导出的实体
只能以 ``qualifiedName`` 的形式访问。

如下例所示：

.. code-block:: typescript
   :linenos:

    namespace NS {
        export function foo(this: int) {}
        function bar(i: int) {
            i.foo() // OK, method call is used
        }
    }

    let i = 1
    NS.foo(i)  // OK, function call is used
    i.foo()    // Compile-time error, 'foo' is not resolved
    i.NS.foo() // Compile-time error, 'NS' is not defined for 'int'

.. note::
    虽然带接收者的函数可以出现在显式重载列表中，
    但这样的重载不能像 :ref:`Explicit Function Overload` 的示例那样使用
    方法访问语法来调用。

|

.. _Receiver Type:

接收者类型
===============================================================================

.. meta:
    frontend_status: Done

*接收者类型* 是函数、函数类型以及带接收者 lambda 中
*接收者参数* 的类型。*接收者类型* 可以是接口类型、类类型或数组类型。
否则会发生 :index:`compile-time error`。

下面的示例展示了使用数组类型作为 *接收者类型*：

.. code-block:: typescript
   :linenos:

      function addElements(this: number[], ...s: number[]) {
       ...
      }

      let x: number[] = [1, 2]
      x.addElements(3, 4)

.. index::
   receiver type
   receiver parameter
   type
   function
   function type
   lambda with receiver
   interface type
   class type
   array type
   type parameter
   array type

|

.. _Function Types with Receiver:

带接收者的函数类型
===============================================================================

.. meta:
    frontend_status: Done

带接收者的函数类型用于指定带接收者函数或 lambda 的签名。
它与普通的函数类型（见 :ref:`Function Types`）几乎完全相同，
区别在于第一个参数是必选的，并且必须使用关键字 ``this`` 作为其名称：

带接收者的函数类型的语法如下：

.. code-block:: abnf

    functionTypeWithReceiver:
        '(' receiverParameter (',' ftParameterList)? ')' ftReturnType
        ;

接收者参数的类型称为接收者类型（见 :ref:`Receiver Type`）。

.. index::
   function type with receiver
   signature
   function
   lambda
   function with receiver
   lambda with receiver
   function type
   this keyword
   syntax
   parameter
   receiver type
   receiver parameter

.. code-block:: typescript
   :linenos:

      class A {...}

      type FA = (this: A) => boolean
      type FN = (this: number[], max: number) => number

*带接收者的函数类型* 也可以是泛型，如下所示：

.. code-block:: typescript
   :linenos:

      class B<T> {...}

      type FB<T> = (this: B<T>, x: T): void
      type FBS = (this: B<string>, x: string): void

普通的函数类型兼容规则（见 :ref:`Subtyping for Function Types`）
同样适用于 *带接收者的函数类型*，并且参数名会被忽略。

.. index::
   function type with receiver
   generic
   function type
   compatibility
   subtyping
   parameter name

.. code-block:: typescript
   :linenos:

      class A {...}

      type F1 = (this: A) => boolean
      type F2 = (a: A) => boolean

      function foo(this: A): boolean {}
      function goo(a: A): boolean {}

      let f1: F1 = foo // OK
      f1 = goo // OK

      let f2: F2 = goo // OK
      f2 = foo // OK
      f1 = f2 // OK

唯一区别在于：只有 *带接收者的函数类型* 的实体，
而不是仅仅与之兼容的普通 *函数类型* 实体，才能用于
:ref:`Method Call Expression`。

.. code-block:: typescript
   :linenos:

      let a = new A()
      a.f1() // OK, function type with receiver
      f1(a)  // OK

      a.f2() // Compile-time error
      f2(a) // OK

.. index::
   entity
   function type with receiver
   method call
   expression
   compile-time error


.. note::
    这种方法调用语法的限制，很容易通过把普通函数赋给一个兼容的
    *带接收者的函数类型* 来绕过。下面的示例展示了这种参数形式。

带接收者的函数类型可以作为参数类型使用。下面给出相应示例：

.. code-block:: typescript
   :linenos:

    function foo(p: number, f: (this: number)=> number) {
        console.log(p.f(), f(p))
    }

    function goo(this: number) { return this - 1 }
    function bar(this: number) { return this + 1 }
    function compat(n: number) { return n }

    let n: number = 1
    foo(n, goo)  // prints `0 0`
    foo(n, bar)  // prints `2 2`
    foo(n, compat)  // prints `1 1`


把实际实体赋给 *带接收者的函数类型* 变量时，不能使用方法调用语法；
尝试这样做会导致 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

   function foo<T extends Object>(this: T, functor: (this: T)=> void): void {
      // following two calls are equivalent
      functor(this)
      this.functor()
   }

   function bar<T>(this: T): void {
      console.log(this)
   }

   let x = 5
   x.foo(bar<int>) // OK
   let y = bar<int> // OK
   x.foo(y) // OK

   // compile time error - can not assign entity with method call syntax
   // to a function type
   x.foo(x.bar)
   x.foo(x.bar<int>)
   let z = x.bar
   let y = x.bar<int>

|

.. _Lambda Expressions with Receiver:

带接收者的 Lambda 表达式
===============================================================================

.. meta:
    frontend_status: Done

*带接收者的 lambda 表达式* 定义了一个 *带接收者的函数类型* 实例
（见 :ref:`Function Types with Receiver`）。它与普通 lambda 表达式
（见 :ref:`Lambda Expressions`）几乎完全相同，区别在于第一个参数是必选的，
并且必须使用关键字 ``this`` 作为参数名：

*带接收者的 lambda 表达式* 的语法如下：

.. code-block:: abnf

    lambdaExpressionWithReceiver:
        annotationUsage?
        '(' receiverParameter (',' lambdaParameterList)? ')'
        returnType? '=>' lambdaBody
        ;

注解的使用见 :ref:`Using Annotations`。

关键字 ``this`` 可以在 *带接收者的 lambda 表达式* 内部使用，
它对应于第一个参数：

.. index::
   lambda expression with receiver
   lambda expression
   instance
   function type with receiver
   lambda expression
   parameter
   this keyword
   annotation

.. code-block:: typescript
   :linenos:

      class A { name = "Bob" }

      let show = (this: A): void {
          console.log(this.name)
      }

lambda 可以通过两种语法形式调用，如下所示：

.. code-block:: typescript
   :linenos:

      class A {
        name: string
        constructor (n: string) {
            this.name = n
        }
      }

      function foo(aa: A[], f: (this: A) => void) {
        for (let a of aa) {
            a.f() // first way
            f (a) // second way
        }
      }

      let aa: A[] = [new A("aa"), new A("bb")]
      foo(aa, (this: A) => { console.log(this.name)} ) // output: "aa" "bb"

.. index::
   lambda
   syntax
   constructor
   function
   class

.. note::
   如果 *带接收者的 lambda 表达式* 声明在类或接口内部，
   那么 lambda 体中对 ``this`` 的使用指向的是第一个 lambda 参数，
   而不是外围的类或接口。类外部对 lambda 的调用必须使用普通的参数语法，
   如下例所示：


   .. code-block:: typescript
      :linenos:

         class B {
           foo() { console.log ("foo() from B is called") }
         }
         class A {
           foo() { console.log ("foo() from A is called") }
           bar() {
               let lambda1 = (this: B): void => { this.foo() } // local lambda
               new B().lambda1()
           }
           lambda2 = (this: B): void => { this.foo() } // class field lambda
         }
         new A().bar() // Output is 'foo() from B is called'
         new A().lambda2 (new B) // Argument is to be provided in its usual place

         interface I {
            lambda: (this: B) => void // Property of the function type
         }
         function foo (i: I) {
            i.lambda(new B) // Argument is to be provided in its usual place
         }

.. index::
   lambda expression with receiver
   class
   interface
   this keyword
   lambda body
   lambda parameter
   surrounding class
   surrounding interface
   syntax
   argument
   function type

|

.. _Trailing Lambdas:

尾随 Lambda
******************************************************************************

.. meta:
    frontend_status: Done

*尾随 lambda* 是函数或方法调用的一种特殊记法：当某个函数或方法的最后一个参数
是函数类型时，可以用 :ref:`Block` 记法把该实参作为 lambda 传入。
*尾随 lambda* 的语法形式如下：

.. code-block:: abnf

    trailingLambda:
        'async'? block
        ;

.. index::
   trailing lambda
   notation
   function call
   method call
   parameter
   function type
   method
   parameter
   lambda
   block notation

修饰符 ``async`` 可选，用来标记 :ref:`Async Lambdas`。

尾随 lambda 的使用如下例所示：

.. code-block:: typescript
   :linenos:

      class A {
          foo (f: ()=>void) { ... }
      }

      let a = new A()
      a.foo() { console.log ("method lambda argument is activated") }
      // method foo receives last argument as the trailing lambda

目前，尾随 lambda 的类型中不能显式写出参数，
唯一例外是接收者参数（见 :ref:`Lambda Expressions with Receiver`）。
否则，会发生 :index:`compile-time error`。

调用之后紧跟的代码块总会被解释为 *尾随 lambda*。
如果被调用实体的最后一个参数不是函数类型，则会发生
:index:`compile-time error`。

调用与代码块之间可以用分号 ``;`` 作为分隔符，
以表明该代码块不是 *尾随 lambda*。当调用带有最后一个可选参数的实体时
（见 :ref:`Optional Parameters`），这表示调用必须使用该参数的默认值。

.. index::
   trailing lambda
   syntax
   parameter
   receiver parameter
   optional parameter
   lambda expression with receiver
   block
   function type
   lambda
   semicolon
   separator
   default value
   call

.. code-block:: typescript
   :linenos:

      function foo (f: ()=>void) { ... }

      foo() { console.log ("trailing lambda") }
      // 'foo' receives last argument as the trailing lambda

      function bar(f?: ()=>void) { ... }

      bar() { console.log ("trailing lambda") }
      // function 'bar' receives last argument as the trailing lambda,
      bar(); { console.log ("that is the block code") }
      // function 'bar' is called with parameter 'f' set to 'undefined'

      function goo(n: number) { ... }

      goo() { console.log("aa") } // Compile-time error as goo() requires an argument
      goo(); { console.log("aa") } // Compile-time error as goo() requires an argument


如果某个函数或方法在一个可选函数类型参数之前还有其他可选参数，
那么调用时可以跳过这些可选实参，仅保留尾随 lambda。
这意味着所有被跳过参数的值都会是 ``undefined``。

.. code-block:: typescript
   :linenos:

    function foo (p1?: number, p2?: string, f?: ()=>string) {
        console.log (p1, p2, f?.())
    }

    foo()                           // undefined undefined undefined
    foo() { return "lambda" }       // undefined undefined lambda
    foo(1) { return "lambda" }      // 1 undefined lambda
    foo(1, "a") { return "lambda" } // 1 a lambda

.. index::
   optional parameter
   optional argument
   trailing lambda
   argument
   operational function
   function
   function type
   parameter
   method
   function call
   method call
   string
   lambda

|

.. _Accessor Declarations:

访问器声明
******************************************************************************

.. meta:
    frontend_status: None

访问器既可以是顶层声明（见 :ref:`Top-Level Declarations`），
也可以是命名空间内部的声明（见 :ref:`Namespace Declarations`）。
它声明 getter、setter，或具有预定义签名的函数。
访问器的使用形式模仿变量访问模式，也就是获取或设置变量值的写法。

*访问器声明* 的语法如下：

.. code-block:: abnf

    accessorDeclaration:
        'native'?
        ( 'get' identifier '(' ')' returnType? block?
        | 'set' identifier '(' requiredParameter ')' block?
        )
        ;

.. index::
   accessor
   accessor declaration
   top-level declaration
   variable
   control
   getter
   setter
   value

修饰符 ``native`` 表示该访问器是 *原生访问器*
（与 :ref:`Native Functions` 类似）。

非原生访问器必须具有函数体。如果出现以下任一情况，则会发生
:index:`compile-time error`：

- 原生访问器具有函数体；或
- 非原生访问器没有函数体。

get-accessor（getter）必须具有显式返回类型且不能有参数；
或者也可以不写返回类型，但前提是能从 getter 函数体中推断该类型
（见 :ref:`Return Type Inference`）。
set-accessor（setter）必须恰好有一个参数，且不能有返回类型。

.. note::
   如果 *accessor* 是命名空间实体，那么在导出以及使用限定名时，
   适用与其他命名空间实体相同的规则（见 :ref:`Namespace Declarations`）。

如果满足以下任一条件，则会发生 :index:`compile-time error`：

- 把 getter 或 setter 当作函数用于调用表达式；
- 无法从 getter 函数体推断 getter 的返回类型；或
- set-accessor（setter）具有可选参数（见
  :ref:`Optional Parameters`）。

.. index::
   native modifier
   accessor
   native accessor
   native function
   non-native accessor
   get-accessor
   set-accessor
   getter
   setter
   return type
   accessor declaration
   top-level declaration
   parameter
   type inference


下面的示例展示了使用访问器控制赋值：

.. code-block:: typescript
   :linenos:

    let saved_age = 0

    export get age(): number { return saved_age }
    export set age(a: number) {
        if (a < 0) { throw new Error("wrong age") }
        saved_age = a
    }

调用 getter 还是 setter 由使用位置决定：

.. code-block:: typescript
   :linenos:

   get name(): string { return "" }
   set name(x: string) { }

   console.log (name) // Getter is called
   name = "some string" // Setter is called

.. index::
   accessor
   value setting
   control
   getter
   setter
   function

但是，访问器声明必须与其他实体可区分。如果出现以下情况，
则会发生 :index:`compile-time error`：

- 访问器名称与同一作用域中的另一个实体名称相同；
- 同一作用域中两个 getter 的名称相同，或两个 setter 的名称相同。

.. code-block:: typescript
   :linenos:

   let name = "Bob"
   get name(): string { return "Alice" } // Compile-time error

同名 getter 和 setter 的签名不受额外限制。

.. code-block:: typescript
   :linenos:

   set hashCode(x: string) {/*body*/}
   get hashCode(): long {/*body*/} // OK

   hashCode = "some string"
   const l: long = hashCode

.. index::
   accessor declaration
   accessor
   entity
   scope
   getter
   setter
   name
   restriction
   signature

getter 和 setter 的使用方式与变量完全一致。
如果满足以下任一条件，则会发生 :index:`compile-time error`：

- 在 :ref:`Assignment` 中，把 getter 用在 *左值表达式* 位置；
- 试图从 setter 获取值。

.. code-block:: typescript
   :linenos:

    get magicNumber(): number { return 42 }
    set randomSeed(a: number) {}

    console.log(magicNumber) // OK, getter is used
    magicNumber = 15 // Compile-time error, setter is not defined

    randomSeed = 42 // OK, setter is used
    console.log(randomSeed) // Compile-time error, getter is not defined

.. index::
   getter
   setter
   variable
   expression
   assignment
   value

访问器可以声明在所有允许 :ref:`Top-Level Declarations` 出现的位置，
包括命名空间内部：

.. code-block:: typescript
   :linenos:

    namespace N {
        let saved_age = 0

        export get age(): number { return saved_age }
        export set age(a: number) {
            if (a < 0) { throw new Error("wrong age") }
            saved_age = a
        }
    }

    N.age = 18
    console.log(N.age)

.. index::
   accessor
   declaration
   top-level declaration

|

.. _Pattern Matching:

模式匹配
******************************************************************************

.. meta:
    frontend_status: None

*模式匹配* 是一组强大的特性，大多数现代编程语言都支持它。
通常来说，*模式匹配* 允许把一个值与某个模式进行匹配，并在匹配成功后执行
相应动作。成功的匹配还可以把一个值解构为其组成部分。

当前版本的 |LANG| 支持一种简单的 *模式匹配* 特性，称为 *解构赋值*。
其他特性将在本规范后续修订中加入。

|

.. _Destructuring Assignment:

解构赋值
===============================================================================

*解构赋值* 允许从数组或元组中提取值，并把它们赋给不同的变量。

*解构赋值* 的语法如下：

.. code-block:: abnf

    destructuringAssignment:
        '[' lhsExpression? (',' lhsExpression?)* ']' '=' rhsExpression
        ;

``lhsExpression`` 和 ``rhsExpression`` 的定义见 :ref:`Assignment`。

``rhsExpression`` 必须是数组类型或元组类型。否则会发生
:index:`compile-time error`。

*解构赋值* 可以看作一组赋值（参见 :ref:`Simple Assignment Operator`）
的紧凑写法。*左值表达式* 中的各项（无论是 ``lhsExpression`` 还是空项）
都对应 ``rhsExpression`` 中的元素序列，从数组或元组的第一个元素
（即索引为 ``0`` 的元素）开始：

.. code-block:: typescript
   :linenos:

    function foo(x: string[]) {
        let a = ""
        let b = "";
        [a, , b] = x
        // this line works the same as the previous line:
        a = x[0]; b = x[2]
    }

在上面的示例中，``a`` 接收 ``x[0]`` 的值，``b`` 接收 ``x[2]`` 的值。
如果试图访问一个索引大于等于数组长度的元素，那么会像
:ref:`Array Indexing Expression` 一样抛出 *RangeError*。

如果缺少 ``lhsExpression``，则会忽略数组或元组中对应的那个元素。

如果满足以下任一条件，则会发生 :index:`compile-time error`：

- 数组元素或元组元素的类型不能赋值给对应 ``lhsExpression`` 的类型
  （参见 :ref:`Assignability`）；
- ``rhsExpression`` 是元组类型，而某个 ``lhsExpression`` 对应的是缺失的
  元组元素。

下面给出了合法与非法的解构示例：

.. code-block:: typescript
   :linenos:

    function foo(x: [string, number, string]) {
        let a: string
        let b: number
        [a] = x; // OK
        [, b, a] = x; // OK
        [, b, a,,,,] = x; // OK
        [b] = x; // Compile-type error, x[0] is not assignable to 'b'
        [, b] = x; // OK
        [,,,b] = x; // Compile-time error, there is no element for variable 'b'
    }


.. raw:: pdf

   PageBreak
