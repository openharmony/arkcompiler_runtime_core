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


.. _Semantic Rules:

语义规则
################################################################################

.. meta:
    frontend_status: Done

本章包含将在整个规范文档中使用的语义规则。
这些规则的描述在一定程度上是非形式化的；为便于理解，省略了一些细节。

.. index::
   semantic rule

|

.. _Semantic Essentials:

语义基础
*******************************************************************************

.. meta:
    frontend_status: Partly

本节简要介绍主要语义术语，以及它们在若干上下文中的用法。

.. index::
   semantic term
   context

|

.. _Type of Standalone Expression:

独立表达式的类型
===============================================================================

.. meta:
    frontend_status: Done

独立表达式（参见 :ref:`Type of Expression`）是指在其使用上下文中没有目标类型的表达式。

独立表达式的类型按如下规则确定：

- 对于 :ref:`Constant Expressions`，包括 :ref:`Numeric Literals`，其类型通过
  :ref:`Type Inference for Constant Expressions` 推断得到。

- 对于 :ref:`Array Literal`，其类型由元素推断得到
  （参见 :ref:`Array Type Inference from Types of Elements`）。

- 对于 :ref:`Object Literal`，其类型无法推断，因此会发生 :index:`compile-time error`。

- 否则，其类型按 :ref:`Type of Expression` 中的定义处理。

示例如下：

.. index::
   standalone expression
   type of expression
   expression
   target type
   context
   type
   integer literal
   floating-point literal
   constant expression
   inferred type
   operand type
   operand
   array literal
   object literal

.. code-block:: typescript
   :linenos:

       function foo() {
         1    // type is 'int'
         1.0  // type is 'number'
         [1.0, 2.0]  // type is number[]
         [1, "aa"] // type is (int | string)
       }

|

.. Specifics of Assignment-like Contexts:

赋值类上下文的特性
===============================================================================

赋值类上下文（参见 :ref:`Assignment-like Contexts`）可以看作一个赋值 ``x = expr``，
其中 ``x`` 是左值表达式，``expr`` 是右值表达式。比如，在调用 ``foo(expr)``
时，存在把 ``expr`` 隐式赋给形式参数 ``foo`` 的过程；在 :ref:`Array Literal`
和 :ref:`Object Literal` 中，也存在把值隐式赋给元素或属性的过程。

赋值类上下文的特殊之处在于：左值表达式的类型是已知的，但右值表达式的类型
在该上下文中不一定已知：

- 如果右值表达式的类型能从表达式本身得出，那么会执行 :ref:`Assignability`
  检查，如下例所示：

.. index::
   assignment-like context
   context
   assignment
   expression
   type
   assign
   assignability

.. code-block:: typescript
   :linenos:

       function foo(x: string, y: string) {
           x = y // OK, assignability is checked
       }

- 否则，会尝试将左值表达式的类型施加到右值表达式上。
  如果该尝试失败，则发生 :index:`compile-time error`，如下所示：

.. code-block:: typescript
   :linenos:

    function foo(x: int, y: double[]) {
        x = 1 // OK, type of '1' is inferred from type of 'x'
        y = [1, 2] // OK, array literal is evaluated as [1.0, 2.0]
    }

.. index::
   assignability
   expression
   type

|

.. Specifics of Variable Initialization Context:

变量初始化上下文的特性
===============================================================================

如果变量或常量声明（参见 :ref:`Variable and Constant Declarations`）带有显式类型注解，
那么适用与 *赋值类上下文* 相同的规则。否则，对于 ``let x = expr``
（参见 :ref:`Type Inference from Initializer`），有两种情况：

- 如果右值表达式的类型能从表达式本身得出，那么该类型就成为变量的类型，
  如下例所示：

.. code-block:: typescript
   :linenos:

       function foo(x: int) {
           let y = x // type of 'y' is 'int'
       }

- 否则，``expr`` 的类型按独立表达式的类型来求值，如下例所示：

.. code-block:: typescript
   :linenos:

       function foo() {
           let x = 1 // x is of type 'int' (default type of '1')
           let y = [1, 2] // x is of type 'number[]'
       }

.. index::
   variable
   initialization
   context
   constant declaration
   assignment-like context
   annotation
   declaration
   type inference
   initializer
   expression
   standalone expression
   function

|

.. _Specifics of Numeric Operator Contexts:

数值运算符上下文的特性
===============================================================================

.. meta:
    frontend_status: Done

后缀和前缀的自增、自减运算符在求值 ``byte`` 和 ``short`` 操作数时不会做拓宽。
对赋值运算符也是如此（把赋值视作二元运算符时）。

对于其他数值运算符，一元和二元数值表达式的操作数都会被拓宽到更大的数值类型，
其最小类型为 ``int``。这些运算符都不会在不拓宽的情况下求值 ``byte`` 和
``short`` 类型的值。具体运算符的细节在规范的相应章节中讨论。

.. index::
   numeric operator
   context
   numeric operator context
   operand
   unary numeric expression
   binary numeric expression
   widening
   numeric type
   type

|

.. _Specifics of String Operator Contexts:

字符串运算符上下文的特性
===============================================================================

.. meta:
    frontend_status: Done

如果二元运算符 ``+`` 的一个操作数是 ``string`` 类型，那么会对另一个非字符串
操作数执行字符串转换，把它转换为字符串（参见 :ref:`String Concatenation` 和
:ref:`String Operator Contexts`）。

.. index::
   string operator
   string operator context
   context
   operand
   binary operator
   string type
   string conversion
   non-string operand
   string concatenation

|

.. _Other Contexts:

其他上下文
===============================================================================

.. meta:
    frontend_status: Done

所有其他上下文，尤其是 :ref:`Overriding`，唯一适用的语义规则就是使用 :ref:`Subtyping`。

.. index::
   context
   semantic rule
   overriding
   subtyping

|

.. _Specifics of Type Parameters:

类型参数的特性
===============================================================================

.. meta:
    frontend_status: Done

如果 *赋值类上下文* 中左值表达式的类型是类型参数，那么即使设置了类型参数约束，
它也不会为类型推断提供额外信息。

如果某个表达式的 *目标类型* 是 *类型参数*，那么该表达式的类型会按
*独立表达式* 的类型来推断。

语义如下例所示：

.. index::
   type parameter
   type
   assignment-like context
   context
   type inference
   constraint
   target type
   expression
   standalone expression
   semantics

.. code-block:: typescript
   :linenos:

       function foo<T extends number>(x: T) {}

       foo(1) // Compile-time error

在上面的示例中，值 1 的类型被推断为 int，也就是整数字面量的默认类型。
该表达式等价于“以 int 作为类型实参调用 foo”，这会导致
:index:`compile-time error`，因为 int 不是 number（类型参数约束）的子类型。

要修正该代码，必须显式写出以 number 为类型实参的调用形式。

.. index::
   inferred type
   default type
   integer literal
   expression
   subtype
   parameter constraint
   type
   argument

|

.. _Semantic Essentials Summary:

语义基础摘要
===============================================================================

主要语义术语如下：

- :ref:`Type of Expression`；
- :ref:`Assignment-like Contexts`；
- :ref:`Type Inference from Initializer`；
- :ref:`Numeric Operator Contexts`；
- :ref:`String Operator Contexts`；
- :ref:`Subtyping`；
- :ref:`Assignability`；
- :ref:`Overriding`；
- :ref:`Overloading`；
- :ref:`Type Inference`。

.. index::
   semantics
   type inference
   initializer
   string operator
   context
   subtyping
   assignment-like context
   expression
   assignability
   numeric operator
   overriding
   overloading

|

.. _Subtyping:

子类型关系
*******************************************************************************

.. meta:
    frontend_status: Done

类型 ``S`` 与 ``T`` 之间的 *子类型* 关系（记作 ``S<:T``）表示：
任何 ``S`` 类型对象都可以在任何需要 ``T`` 类型对象的上下文中被安全地替换使用。
其相反关系（记作 ``T:>S``）称为 *超类型* 关系。每个类型都是自己的子类型和超类型
（``S<:S`` 与 ``S:>S``）。

根据 ``S<:T`` 的定义，类型 ``T`` 属于类型 ``S`` 的 *超类型* 集合。
这个 *超类型* 集合包括所有 *直接超类型*（在后续小节讨论）以及它们各自的
所有 *超类型*。更形式化地说，这个集合是对直接超类型关系做自反闭包和传递闭包
得到的。

在下文中，讨论非泛型类、泛型类或接口类型时，术语 *subclass*、*subinterface*、
*superclass* 和 *superinterface* 会作为 *subtype* 与 *supertype* 的同义词使用。

如果两种类型之间的关系没有在后续某一节中描述，那么它们彼此无关。
特别地，两个 :ref:`Resizable Array Types` 与两个 :ref:`Tuple Types` 彼此无关，
除非它们是相同类型（见 :ref:`Type Identity`）。

.. code-block:: typescript
   :linenos:

       class Base {}
       class Derived extends Base {}

       function not_a_subtype (
          ab: Array<Base>, ad: Array<Derived>,
          tb: [Base, Base], td: [Derived, Derived],
       ) {
          ab = ad // Compile-time error
          tb = td // Compile-time error
       }



.. index::
   subtyping
   subtype
   subclass
   subinterface
   type
   object
   closure
   supertype
   superclass
   superinterface
   direct supertype
   reflexive closure
   transitive closure
   array type
   array
   resizable array
   fixed-size array
   tuple type

|

.. _Subtyping for Non-Generic Classes and Interfaces:

非泛型类与接口的子类型关系
===============================================================================

.. meta:
    frontend_status: Partly

对非泛型类与接口而言，当满足下列任一条件时，``S`` 是 ``T`` 的直接 subclass
或 direct subinterface（或 ``Object`` 的直接子类型）：

- 如果 ``T`` 出现在 ``S`` 的 ``extends`` 子句中
  （参见 :ref:`Class Extension Clause`），那么类 ``S`` 是类 ``T`` 的
  直接子类型（``S<:T``）：

  .. code-block:: typescript
     :linenos:

           // Illustrating S<:T
           class T {}
           class S extends T {}
           function foo(t: T) {}

           // Using T
           foo(new T)

           // Using S (S<:T)
           foo(new S)

- 如果 ``S`` 没有 :ref:`Class Extension Clause`，那么类 ``S`` 是类 ``Object``
  的直接子类型（``S<:Object``）：

  .. code-block:: typescript
     :linenos:

           // Illustrating S<:Object
           class S {}
           function foo(o: Object) {}

           // Using Object
           foo(new Object)

           // Using S (S<:Object)
           foo(new S)

- 如果 ``T`` 出现在 ``S`` 的 ``implements`` 子句中
  （参见 :ref:`Class Implementation Clause`），那么类 ``S`` 是接口 ``T`` 的
  直接子类型（``S<:T``）：

  .. code-block:: typescript
     :linenos:

           // Illustrating S<:T
           // S is class, T is interface
           interface T {}
           class S implements T {}
           function foo(t: T) {}
           let s: S = new S

           // Using T
           let t: T = s
           foo(t)

           // Using S (S<:T)
           foo(s)

- 如果 ``T`` 出现在接口 ``S`` 的 ``extends`` 子句中
  （参见 :ref:`Superinterfaces and Subinterfaces`），那么接口 ``S`` 是接口 ``T`` 的
  直接子类型（``S<:T``）：

  .. code-block:: typescript
     :linenos:

           // Illustrating S<:T
           // S is interface, T is interface
           interface T {}
           interface S extends T {}
           function foo(t: T) {}
           let t: T
           let s: S

           // Using T
           class A implements T {}
           t = new A
           foo(t)

           // Using S (S<:T)
           class B implements S {}
           s = new B
           foo(s)

- 如果接口 ``S`` 没有 ``extends`` 子句（参见 :ref:`Superinterfaces and Subinterfaces`），
  那么接口 ``S`` 是类 ``Object`` 的直接子类型（``S<:Object``）：

  .. code-block:: typescript
     :linenos:

           // Illustrating subinterface of Object
           interface S {}
           function foo(o: Object) {}

           // Using Object
           foo(new Object)

           // Using subinterface of Object
           class A implements S {}
           let s: S = new A;
           foo(s)

这些规则意味着：只有通过 ``extends`` 或 ``implements`` 明确建立的关系，才会形成
非泛型类与接口之间的直接子类型关系；单纯“结构相似”不会自动引入子类型关系。

.. index::
   subtype
   subclass
   subinterface
   supertype
   superclass
   superinterface
   interface type
   implementation
   non-generic class
   extension clause
   implementation clause
   Object
   class extension

|

.. _Subtyping for Generic Classes and Interfaces:

泛型类与接口的子类型关系
===============================================================================

.. meta:
    frontend_status: Partly

泛型类或泛型接口 ``C`` <``F``:sub:`1` ``,..., F``:sub:`n`>（其中 ``n>0``）
在下列任一情况下是另一个泛型类或接口 ``T`` 的 *直接子类型*：

- ``T`` 是 ``C`` 的 ``extends`` 子句中列出的 *直接超类*：

  .. code-block:: typescript
     :linenos:

           // T<U, V>  is direct superclass of C<U,V>
           // T<U, V> >: C<U, V>

           class T<U, V> {
              foo(p: U|V): U|V { return p }
           }

           class C<U, V> extends T<U, V> {
              bar(u: U): U { return u }
           }


           // OK, exact match
           let t: T<int, boolean>  = new T<int, boolean>
           let c: C<int, boolean> = new C<int, boolean>


           // OK, assigning to direct superclass
           t =  new C<int, boolean>

           // Compile-time error, cannot assign to subclass
           c =  new T<int, boolean>

- ``T`` 是 ``C`` 的某个 *直接超接口*
  （参见 :ref:`Superinterfaces and Subinterfaces`）：

  .. code-block:: typescript
     :linenos:

           // Interface I<U, V>  is direct superinterface
           // of J<U,V>, X<U, V>

           interface I<U, V> {
              foo(u: U): U;
              bar(v: V): V;
           }

           // J<U, V> <: I<U, V>
           // since J extends I
           interface J<U, V> extends I<U, V>
           {
              foo(u: U): U
              bar(v: V): V

              foo1(p: U|V): U|V
           }

           // X<U, V> <: I<U, V>
           // since X implements I
           class X<U, V> implements I<U,V> {
             foo(u: U): U { return u }
             bar(v: V): V { return v }
           }

           // Y<U,V> <: J<U, V>  (directly)
           // Also Y<U, V> <: I<U, V> (transitively)
           class Y<U, V> implements J<U,V> {
             foo(u: U): U { return u }
             bar(v: V): V { return v }

             foo1(p: U|V): U|V { return p }
           }

           let i: I<int, boolean>
           let j: J<int, boolean>
           let x = new X<int, boolean>
           let y = new Y<int, boolean>

           // OK, assigning to direct supertypes
           i = x
           j = y

           // OK, assigning subinterface (J<:I)
           i = j

           // Compile-time error, cannot assign superinterface (I>:JJ
           j = i

- 如果 ``C`` 是没有 *直接超类* 的泛型类类型，或没有直接超接口的泛型接口类型，
  则 ``T`` 是 ``Object``：

  .. code-block:: typescript
     :linenos:

           // Object is direct superclass and for C<U,V>
           // and direct superinterface for I<U,V>
           //
           class C<U, V> {
              foo(u: U): U { return u }
              bar(v: V): V { return v }
           }
           interface I<U, V> {
              foo(u: U): U { return u }
              bar(v: V): V { return v }
           }

           let o: Object = new Object
           let c: C<int, boolean> = new C<int, boolean>
           let i: I<int, boolean>

           // // example1 - C<U,V> <: Object
           function example1(o: Object) {}

           // OK, example(Object)
           example1(o)
           // OK, C<int, boolean> <: Object
           example1(c)

           // // example2 - I<U,V> <: Object
           function example2(o: Object) {}
           class D<U, V> implements I<U, V> {}
           i = new D<int, boolean>

           // OK, example2(Object)
           example2(o)
           // OK, I<int, boolean> <: Object
           example2(i)

类型参数的直接超类型，就是该类型参数约束中指定的类型。
换句话说，若类型参数显式写出了约束，那么该约束类型本身就参与对应的直接子类型关系判断。

如果泛型类或接口的类型参数带有变型（见 :ref:`Type Parameter Variance`），
那么该类或接口各个实例化之间的子类型关系，要按照相应类型参数的变型来确定。
例如，若定义了泛型类 ``G<in T1,out T2>``，并且 ``S>:U``、``T<:V``，
则有 ``G<S,T> <: G<U, V>``。

这意味着泛型子类型关系既可能来自 ``extends`` / ``implements``，也可能来自同一泛型类型
不同实例化之间按变型传播出来的关系。

.. code-block:: typescript
   :linenos:

      // Subtyping illustration for a generic with parameter variance

      // U1 <: U0
      class U0 {}
      class U1 extends U0 {}

      // Generic with contravariant parameter
      class E<in T> {}

      let e0: E<U0> = new E<U1> // Compile-time error, E<U0> is subtype of E<U1>
      let e1: E<U1> = new E<U0> // OK, E<U1> is supertype for E<U0>

      // Generic with covariant parameter
      class F<out T> {}

      let f0: F<U0> = new F<U1> // OK, F<U0> is supertype for F<U1>
      let f1: F<U1> = new F<U0> // Compile-time error, F<U1> is subtype of F<U0>


.. index::
   direct supertype
   generic type
   generic class
   generic interface
   interface type declaration
   direct superinterface
   type parameter
   superclass
   supertype
   type
   constraint
   type parameter
   superinterface
   variance
   subtyping
   instantiation
   class
   interface
   bound
   Object

|

.. _Subtyping for Literal Types:

字面量类型的子类型关系
===============================================================================

.. meta:
    frontend_status: Done

任意 ``string`` 字面量类型（见 :ref:`String Literal Types`）都是 ``string`` 类型的
*子类型*。这一点会影响重写，如下例所示：

.. code-block:: typescript
   :linenos:

       class Base {
           foo(p: "1"): string { return "42" }
       }
       class Derived extends Base {
           override foo(p: string): "1" { return "1" }
       }
       // Type "1" <: string

       let base: Base = new Derived
       let result: string = base.foo("1")
       /* Argument "1" (value) is compatible to type "1" and to type string in
          the overridden method
          Function result of type string accepts "1" (value) of literal type "1"
       */

字面量类型 ``null``（见 :ref:`Literal Types`）既是它自身的子类型，也是它自身的超类型。
类似地，字面量类型 ``undefined`` 也既是它自身的子类型，也是它自身的超类型。

.. index::
   literal type
   subtype
   subtyping
   string type
   overriding
   supertype
   string literal
   null type
   undefined type
   literal type
   supertype

|

.. _Subtyping for Tuple Types:

元组类型的子类型关系
===============================================================================

.. meta:
    frontend_status: None

任意元组类型都是 ``Tuple`` 类型（见 :ref:`Type Tuple`）的子类型。

元组类型 ``T``（``P``:sub:`1` ``, ... , P``:sub:`n`）是类型 ``S``
（``P``:sub:`1` ``, ... , P``:sub:`m`）的子类型，其中 ``n`` :math:`\geq` ``m``。
也就是说，某个元组类型是“具有较少且相同 constituent types 的元组类型”
的子类型（见 :ref:`Type Identity`）。

.. code-block:: typescript
   :linenos:

       function foo(t1: [number], t2: [string, number]) {
           let a: [] = t1       // OK
           let b: [string] = t2 // OK

           t1 = t2 // Compile-time error
           t2 = t1 // Compile-time error

           let d: [string, number, boolean] = ["a", 1, true]
           let t2 = d // OK
           let d = t2 // Compile-time error
       }

|

.. _Subtyping for Union Types:

联合类型的子类型关系
===============================================================================

.. meta:
    frontend_status: Done

联合类型 ``U`` 参与子类型关系（见 :ref:`Subtyping`）的情况如下：

1. 如果联合类型 ``U``（``U``:sub:`1` ``| ... | U``:sub:`n`）中的每个
   ``U``:sub:`i` 都是 ``T`` 的子类型，那么 ``U`` 是类型 ``T`` 的子类型。

.. code-block:: typescript
   :linenos:

       let s1: "1" | "2" = "1"
       let s2: string = s1 // OK

       let a: string | number | boolean = "abc"
       let b: string | number = 42
       a = b // OK
       b = a // Compile-time error, boolean is absent is 'b'

       class Base {}
       class Derived1 extends Base {}
       class Derived2 extends Base {}

       let x: Base = ...
       let y: Derived1 | Derived2 = ...

       x = y // OK, both Derived1 and Derived2 are subtypes of Base
       y = x // Compile-time error

       let x: Base | string = ...
       let y: Derived1 | string ...
       x = y // OK, Derived1 is subtype of Base
       y = x // Compile-time error

.. index::
   union type
   subtyping
   subtype
   type
   string
   boolean

2. 如果对某个 ``i``，``T`` 是 ``U``:sub:`i` 的子类型，那么类型 ``T`` 是
   联合类型 ``U``（``U``:sub:`1` ``| ... | U``:sub:`n`）的子类型。

.. code-block:: typescript
   :linenos:

       let u: number | string = 1 // OK
       u = "aa" // OK
       u = 1.0  // OK, 1.0 is of type 'number' (double)
       u = 1    // Compile-time error, type 'int' is not a subtype of 'number'
       u = true // Compile-time error

.. index::
   union type
   union type normalization
   subtype
   number type

.. note::
   若联合类型归一化后只剩一个类型，则直接使用该单一类型代替原联合：

   .. code-block:: typescript
      :linenos:

             let u: "abc" | "cde" | string // type of 'u' is string

.. note::
   两个联合类型之间的子类型关系：联合类型 ``U2``
   （``U2``:sub:`1` ``| ... | U2``:sub:`n`）是联合类型 ``U1``
   （``U1``:sub:`1` ``| ... | U1``:sub:`m`）的子类型，当且仅当先应用规则 1，
   再对 ``U2`` 的每个成员类型应用规则 2。

   .. code-block:: typescript
      :linenos:

             class T1 {}
             class T2 {}
             class T3 extends T1 {}  // T3 <: T1
             class T4 extends T2 {}  // T4 <: T2
             class T5 {}

             type U1 = T1 | T2
             type U2 = T3 | T4 | T5

             function foo (u1: U1, u2: U2) {
                 u1 = u2
                 // step 1. U2 to be a subtype of U1 iff T3 <: U1 and T4 <: U1 and T5 <: U1
                 // step 2.
                 //         T3 to be a subtype of T1 or T2 - T1 true
                 //         T4 to be a subtype of T1 or T2 - T2 true
                 //         T5 to be a subtype of T1 or T2 - false for both
                 // Compile-time error as a result
             }


.. index::
   union type
   subtype
   supertype

|

.. _Subtyping for Function Types:

函数类型的子类型关系
===============================================================================

.. meta:
    frontend_status: Done

若函数类型 ``F`` 具有参数 ``FP``:sub:`1` ``, ... , FP``:sub:`m`、
rest 参数 ``FPrest``（若存在）以及返回类型 ``FR``，而函数类型 ``S`` 具有参数
``SP``:sub:`1` ``, ... , SP``:sub:`n`、rest 参数 ``SPrest``（若存在）
以及返回类型 ``SR``，那么当且仅当以下所有条件都满足时，``F`` 才是 ``S`` 的
*子类型*：

- ``F`` 的必选与可选参数总数不大于 ``S`` 的；
- ``F`` 的必选参数数目不大于 ``S`` 的；
- 如果 ``FPrest`` 存在，则 ``SPrest`` 也存在；
- 对每个 ``i <= m``，``SP``:sub:`i` 的类型是 ``FP``:sub:`i` 类型的子类型；
- 如果 ``FPrest`` 存在：
  - 对每个 ``i > m``，``SP`` 的参数类型必须是 ``FPrest`` 元素类型的子类型；
  - ``SPrest`` 的类型必须是 ``FPrest`` 类型的子类型；
- 返回类型 ``FR`` 是 ``SR`` 的子类型。

这体现出函数类型子类型关系的三个核心约束：

- 参数类型逆变；
- 返回类型协变；
- 子类型函数可以拥有更少的参数，但不能引入比目标类型更多的必选参数。

.. code-block:: typescript
   :linenos:

       class Base {}
       class Derived extends Base {}

       function check(
          bb: (p: Base) => Base,
          bd: (p: Base) => Derived,
          db: (p: Derived) => Base,
          dd: (p: Derived) => Derived
       ) {
          bb = bd
          /* OK: identical parameter types, and covariant return type */
          bb = dd
          /* compile-time error, parameter types are not contravariant */
          db = bd
          /* OK: contravariant parameter types, and covariant  return type */

          let f: (p: Base, n: number) => Base = bb
          /* OK: subtype has less parameters */

          let g: () => Base = bb
          /* compile-time error, less parameters than expected */
       }

       let foo: (x?: number, y?: string) => void = (): void => {} // OK: ``m <= n``
       foo = (p?: number): void => {}                             // OK:  ``m <= n``
       foo = (p1?: number, p2?: string): void => {}               // OK: Identical types
       foo = (p: number): void => {}
             // Compile-time error, 1st parameter in type is optional but mandatory in lambda
       foo = (p1: number, p2?: string): void => {}
             // Compile-time error,  1st parameter in type is optional but mandatory in lambda

.. index::
   type
   parameter type
   covariance
   contravariance
   covariant return type
   contravariant return type
   supertype
   parameter
   lambda

.. index::
   function type
   subtype
   subtyping
   parameter type
   contravariance
   rest parameter
   parameter

|

.. _Subtyping for Fixed-Size Array Types:

固定大小数组类型的子类型关系
===============================================================================

如果固定大小数组包含的是引用类型元素（见 :ref:`Reference Types`），
那么元素之间的子类型关系建立在元素类型的子类型关系之上。形式化地说：

``FixedSize<B> <: FixedSize<A>`` 当且仅当 ``B <: A``。

.. code-block:: typescript
   :linenos:

       let x: FixedArray<string> = ["aa", "bb", "cc"]
       let y: FixedArray<Object> = x // OK, as string <: Object
       x = y // Compile-time error

这种子类型关系允许数组赋值，但如果通过某个数组元素类型的子类型向另一个数组中
写入了并非该数组元素类型子类型的值，那么在运行时可能导致 ``ArrayStoreError``。
运行时系统会执行检查以保证类型安全：

.. code-block:: typescript
   :linenos:

       class C {}
       class D extends C {}

       function foo (ca: FixedArray<C>) {
         ca[0] = new C() // ArrayStoreError if ca refers to FixedArray<D>
       }

       let da: FixedArray<D> = [new D()]

       foo(da) // leads to runtime error in 'foo'

.. index::
   type
   subtype
   subtyping
   fixed-size array
   fixed-size array type
   array element
   parameter type
   runtime check
   array
   array element type
   type safety
   runtime system

|

.. _Difference Types:
.. _Subtyping for Difference Types:

差集类型的子类型关系
===============================================================================

差集类型 ``A - B`` 是 ``T`` 的子类型，当且仅当 ``A`` 是 ``T`` 的子类型。

如果 ``T`` 是 ``A`` 的子类型，且没有任何值同时属于 ``T`` 和 ``B``
（即 ``T & B = never``），那么 ``T`` 是差集类型 ``A - B`` 的子类型。

.. index::
   subtype
   subtyping
   difference type

|

.. _Type Identity:

类型同一性
*******************************************************************************

.. meta:
    frontend_status: Done

两个类型之间的 *同一性* 关系表示它们不可区分。
同一性关系是对称的，也是传递的。对类型 ``A`` 与 ``B`` 而言，
同一性按如下规则定义：

- 若数组类型 ``A = T1[]`` 与 ``B = Array<T2>`` 中的 ``T1`` 与 ``T2`` 同一，
  则 ``A`` 与 ``B`` 同一；
- 若元组类型 ``A`` 与 ``B`` 拥有相同的元素个数，且每个对应位置上的类型彼此同一，
  则它们同一；
- 若联合类型 ``A`` 与 ``B`` 拥有相同的成员个数，且 ``B`` 中成员经过某种排列后，
  每个对应位置上的成员类型都与 ``A`` 中对应成员同一，则它们同一；
- 如果 ``A`` 是 ``B`` 的子类型，且 ``B`` 同时也是 ``A`` 的子类型，
  那么 ``A`` 与 ``B`` 同一。

.. note::
   :ref:`Type Alias Declaration` 不会创建新类型，而只是给现有类型引入一个新名称。
   别名与其基类型不可区分。

.. note::
   如果某个泛型类或接口有类型参数 ``T``，而其方法又定义了自己的类型参数 ``T``，
   那么这两个 ``T`` 指代的是不同且互不相关的类型。

   .. code-block:: typescript
      :linenos:

            class A<T> {
               data: T
               constructor (p: T) { this.data = p } // OK, as here 'T' is a class type parameter
               method <T>(p: T) {
                   this.data = p // Compile-time error as 'T' of the class is different from 'T' of the method
               }
            }


.. index::
   type identity
   identity
   indistinguishable type
   permutation
   array type
   tuple type
   union type
   subtype
   type
   type alias
   type alias declaration
   declaration
   base type

|

.. _Assignability:

可赋值性
*******************************************************************************

.. meta:
    frontend_status: Done

如果满足以下任一条件，则类型 ``T``:sub:`1` 可赋值给类型 ``T``:sub:`2`：

- ``T``:sub:`1` 是 ``T``:sub:`2` 的子类型（见 :ref:`Subtyping`）；或
- 存在隐式转换（见 :ref:`Implicit Conversions`），允许把 ``T``:sub:`1`
  类型的值转换成 ``T``:sub:`2` 类型。

*可赋值性* 关系是不对称的。也就是说，``T``:sub:`1` 可赋值给 ``T``:sub:`2`，
并不意味着 ``T``:sub:`2` 也可赋值给 ``T``:sub:`1`。

.. index::
   assignability
   assignment
   type
   type identity
   subtyping
   conversion
   implicit conversion
   asymmetric relationship
   value

|

.. _Invariance, Covariance and Contravariance:

不变、协变与逆变
*******************************************************************************

.. meta:
    frontend_status: Done

变型描述“类型之间的子类型关系”如何映射到“派生类型之间的子类型关系”，
包括泛型类型（见 :ref:`Generics`）、泛型类型的成员签名（参数类型、返回类型），
以及重写实体（见 :ref:`Override-Compatible Signatures`）。
变型分为三种：

- 协变（Covariance）；
- 逆变（Contravariance）；
- 不变（Invariance）。

.. index::
   variance
   subtyping
   type
   derived type
   generic type
   signature
   type parameter
   overriding entity
   override-compatible signature
   parameter
   invariance
   covariance
   contravariance

下面的示例展示了合法的变型使用方式：

.. code-block:: typescript
   :linenos:

      class Base {
         method_one(p: Base): Base {}
         method_two(p: Derived): Base {}
         method_three(p: Derived): Derived {}
      }

则下面的代码是合法的：

.. code-block:: typescript
   :linenos:

      class Derived extends Base {
         // invariance: parameter type and return type are unchanged
         override method_one(p: Base): Base {}

         // covariance for the return type: Derived is a subtype of Base
         override method_two(p: Derived): Derived {}

         // contravariance for parameter types: Base is a supertype for Derived
         override method_three(p: Base): Derived {}
      }

相反，下面的代码会产生编译时错误：

.. code-block-meta:
   expect-cte

.. code-block:: typescript
   :linenos:

      class Derived extends Base {

         // covariance for parameter types is prohibited
         override method_one(p: Derived): Base {}

         // contravariance for the return type is prohibited
         override method_three(p: Derived): Base {}
      }

.. index::
   covariance
   type

.. index::
   contravariance
   type

.. index::
   invariance
   type
   subtyping
   derived type

.. index::
   variance
   base class

.. index::
   variance
   parameter type
   invariance
   covariance
   contravariance
   subtype
   supertype
   overriding

|

.. _Compatibility of Call Arguments:

调用实参的兼容性
*******************************************************************************

.. meta:
    frontend_status: Done

函数、方法、构造函数和 lambda 调用都必须对实参与形参执行兼容性检查。
检查按从左到右顺序进行，并处理普通参数、可选参数、rest 参数和展开表达式。

语义上需要满足：

- 所有必需参数都有对应实参；
- 不存在无法匹配的多余实参；
- 每个实参都可赋值给相应形参类型，或可赋值给 rest 参数的元素类型。

展开数组字面量时，规范会先把这类展开线性化，再继续检查参数兼容性。

更形式化地说，检查按以下步骤进行：

**步骤 1**：把所有“展开数组字面量”的 spread 表达式（见 :ref:`Spread Expression`、
:ref:`Array Literal`）递归线性化，直到调用点不再保留这种形式的 spread 表达式。

**步骤 2**：从左到右检查所有实参，初始时 ``arg_pos = 1`` 且 ``par_pos = 1``：

- 如果 ``par_pos`` 位置的形参不是 rest 参数：

  - 若 ``Targ_pos <: Tpar_pos``，则同时递增 ``arg_pos`` 和 ``par_pos``；
  - 否则发生 :index:`compile-time error`，并结束步骤 2。

- 否则，该形参是 rest 参数（见 :ref:`Rest Parameter`）：

  - 若该形参是 rest-array 形式：

    - 如果当前实参是某个数组的 spread 表达式，且其元素类型 ``U`` 满足
      ``U <: Trest_array_type``，则递增 ``arg_pos``；
    - 否则，若 ``Targ_pos <: Trest_array_type``，则递增 ``arg_pos``；
    - 否则递增 ``par_pos``。

  - 若该形参是 rest-tuple 形式：

    - 依次检查元组中的各元素类型；若某一位置不满足子类型关系，且这还不是元组最后一个元素，
      就继续检查下一个元组元素；
    - 若已经检查到元组最后一个元素仍不匹配，则发生 :index:`compile-time error`，并结束步骤 2；
    - 整个 rest tuple 检查完成后递增 ``par_pos``。

.. index::
   assignability
   call argument
   compatibility
   semantic check
   function call
   method call
   constructor call
   function
   method
   constructor
   rest parameter
   parameter
   spread operator
   spread expression
   array
   tuple
   argument type
   expression
   operator
   assignable type
   increment
   array type
   rest parameter
   check

下面的例子展示了这些检查规则：

.. code-block:: typescript
   :linenos:

       function call (...p: Object[]) {}
       call (...[1, "str", true], ...[ 123])  // Initial call form
       call (1, "str", true, 123)             // To be unfolded into the form with
                                              // no spread expressions

      let arr: number[] = [1, 2, 3]
      tricky_call (...arr)  // Type 'number' is assignable to 'Object', and a new
                            // array of type 'Object[]' is created from elements
                            // of the array of numbers

      function tricky_call (...p: Object[]) {
         p[0] = true
         console.log ("Initial array: ", arr)
         console.log ("Parameter array: ", p)
      }

       function foo1 (p: Object) {}
       foo1 (1)  // Type of '1' must be assignable to 'Object'
                 // p becomes 1

       function foo2 (...p: Object[]) {}
       foo2 (1, "111")  // Types of '1' and "111" must be assignable to 'Object'
                 // p becomes array [1, "111"]

       function foo31 (...p: (number|string)[]) {}
       foo31 (...[1, "111"])  // Type of array literal [1, "111"] must be assignable to (number|string)[]
                 // p becomes array [1, "111"]

       function foo32 (...p: [number, string]) {}
       foo32 (...[1, "111"])  // Types of '1' and "111" must be assignable to 'number' and 'string' accordingly
                 // p becomes tuple [1, "111"]

       function foo4 (...p: number[]) {}
       foo4 (1, ...[2, 3])  //
                 // p becomes array [1, 2, 3]

.. index::
   assignability
   call argument
   compatibility
   semantic check
   function call
   method call
   const

.. index::
   check
   assignable type
   Object
   string
   array

|

.. _Type Inference:

类型推断
*******************************************************************************

.. meta:
    frontend_status: Done

|LANG| 支持强类型，同时允许在若干上下文中省略显式类型注解。
编译器可以根据周围上下文推断部分实体或表达式的类型。
这种称为 *类型推断* 的技术在保证类型安全和代码可读性的同时减少了手动类型注解。
编译器在以下上下文中应用类型推断：

- :ref:`Type Inference for Constant Expressions`；
- 变量与常量声明（见 :ref:`Type Inference from Initializer`）；
- 隐式泛型实例化（见 :ref:`Implicit Generic Instantiations`）；
- 函数、方法或 lambda 的返回类型（见 :ref:`Return Type Inference`）；
- lambda 表达式形参类型（见 :ref:`Lambda Signature`）；
- 数组字面量类型推断（见 :ref:`Array Literal Type Inference from Context`
  及 :ref:`Array Type Inference from Types of Elements`）；
- 对象字面量类型推断（见 :ref:`Object Literal`）。

.. index::
   strong typing
   type annotation
   annotation
   smart compiler
   type inference
   inferred type
   expression
   entity
   surrounding context
   code readability
   type safety
   context
   numeric constant expression
   initializer
   variable declaration
   constant declaration
   generic instantiation
   function return type
   function
   method return type
   method
   lambda
   lambda return type
   return type
   lambda expression
   parameter type
   array literal
   Object literal

|

.. _Type Inference for Constant Expressions:

常量表达式的类型推断
===============================================================================

.. meta:
    frontend_status: Partly

常量表达式（见 :ref:`Constant Expressions`）的类型首先从上下文推断（如果上下文允许）。
允许推断的上下文包括：

- :ref:`Assignment-like Contexts`；
- :ref:`Cast Expression` 上下文；
- :ref:`Numeric Operator Contexts`。

若上下文不允许推断类型，则按如下规则设置 *值默认类型*：

- 若常量表达式为大整数类型（见 :ref:`Specifics of Constant Expressions Evaluation`）
  且其值可用 32 位表示，则默认类型为 ``int``；
- 若常量表达式为大整数类型（见 :ref:`Specifics of Constant Expressions Evaluation`）
  且其值可用 64 位表示，则默认类型为 ``long``；
- 浮点常量表达式（见 :ref:`Specifics of Constant Expressions Evaluation`）的默认类型
  为 ``double`` 或 ``float``。

仅当 *目标类型* 为以下之一时，才进行类型推断：

- 情况 1：数值类型（见 :ref:`Numeric Types`）；或
- 情况 2：包含至少一个数值类型的联合类型（见 :ref:`Union Types`）。

**情况 1：目标类型为数值类型**

当 *目标类型* 为数值类型时，若满足以下条件之一，则从 *目标类型* 和常量表达式的值推断类型：

#. *目标类型* 等于或大于值默认类型；或

#. 值为 *整数值* 且能落入 *目标类型* 的范围内。

.. note::
   若存在 *浮点后缀*，则该字面量已显式声明为 ``floating-point literal``，不再做类型推断。

若上述条件均不满足，则目标类型对该常量表达式无效，发生 :index:`compile-time error`。

数值目标类型上的推断示例如下：

.. code-block-meta:
   expect-cte:

.. code-block:: typescript
   :linenos:

       let l: long = 1     // OK, target type is larger than literal default type
       let b: byte = 127   // OK, integer value fits type 'byte'
       let f: float = 123f // OK, target type is the same as literal default type
       let g: double = 11  // OK, target type is larger than literal default type

       b = 63 + 64           // OK, integer value fits type 'byte'
       b = 1 as short        // OK, integer value fits type 'byte'
       let s: short = -32767 // OK, integer value fits type 'short'

       l = 1.0    // Compile-time error, 'double' cannot be assigned to 'long'
       b = 128    // Compile-time error, value is out of range
       f = 3.14   // Compile-time error, 'double' cannot be assigned to 'float'

当目标类型是包含至少一个数值类型的联合类型时，推断规则如下：

- 若没有任何数值成员适合该值，则使用默认类型；
- 若只有一个数值成员适合该值，则该成员类型就是推断结果；
- 若有多个数值成员都适合该值，则：

  - 若该值是整数值，且适合的整数数值类型只有一个，则推断为该整数类型；
  - 否则，会因类型推断歧义而产生 :index:`compile-time error`。

联合目标类型上的推断示例如下：

.. code-block:: typescript
   :linenos:

       let b: byte | Object = 127 // inferred type for 127 is 'byte'
       b = 128 /* inferred type for 128 is 'int' (default literal type),
                  which is a subtype of Object
               */

       let c: byte | string = 127 // inferred type for 127 is 'byte'
       c = 128 /* inferred type for 128 is 'int' (default literal type),
                  compile-time error as no type in a union is a supertype for
                  'int'
               */

       let d: int | double = 1.0 // inferred type for 1.0 is 'double'
       d = 1 // inferred type for 1 is 'int'

       let e: byte | long = 128 // inferred type for 128 is 'long'
       e = 127 // Compile-time error, type inference ambiguity for 127

       let f: float | double = 3.14 // inferred type is 'double'
       f = 2f                       // inferred type is 'float'

       let g: float | double = 3.4e39 // inferred type is 'double'
       g = 1 // Compile-time error, type inference ambiguity for 1

.. note::

    若目标类型是不包含任何数值类型的联合类型，则使用字面量的默认类型。
    以下代码合法（数值类型是 ``Object`` 的子类型）：

    .. code-block:: typescript
        :linenos:

               let x: Object | string = 1 // OK, instance of type 'int' is assigned to 'x'

               x = 1.0 // OK, instance of type 'double' is assigned to 'x'

.. index::
   numeric type
   inferred type
   target type
   integer type
   context
   type
   union type
   value

|

.. _Return Type Inference:

返回类型推断
===============================================================================

.. meta:
    frontend_status: Done

函数、方法、getter 或 lambda 若未显式写出返回类型，编译器可从函数、方法、getter 或
lambda 体推断。若原生函数（见 :ref:`Native Functions`）缺少返回类型，则发生
:index:`compile-time error`。

当前版本的 |LANG| 至少在以下条件下支持返回类型推断：

- 若没有 ``return`` 语句，或所有 ``return`` 语句均不带表达式，则返回类型为 ``void``
  （见 :ref:`Type undefined or void`），即函数、方法或 lambda 的调用结果为 ``undefined``；
- 若存在 *k* 个（*k* >= 1）返回语句且所有语句均具有相同的类型表达式 *R*，则 ``R`` 为返回类型；
- 若存在 *k* 个（*k* >= 2）类型各异的返回语句，则返回类型为这些类型的 *联合类型*
  （见 :ref:`Union Types`）及其归一化形式（见 :ref:`Union Types Normalization`）；
  若至少一个返回语句不带表达式，则 ``undefined`` 也纳入联合类型；
- 若 lambda 体不含 return 语句但至少含一个 throw 语句
  （见 :ref:`Throw Statements`），则 lambda 返回类型为 never（见 :ref:`Type never`）；
- 若函数、方法或 lambda 为 ``async``（见 :ref:`Asynchronous execution`），
  经上述规则推断出返回类型 ``T`` 后，若 ``T`` 不是 ``Promise``，则最终返回类型为
  ``Promise<T>``。

也就是说，返回类型推断并不是“取第一个 return 的类型”，而是要综合整个函数、方法、getter
或 lambda 体中的所有返回路径，再依次应用 ``void``、同类型返回、联合类型归一化、
``never`` 以及 ``async`` 包装这些规则。

返回类型推断示例如下：

.. code-block:: typescript
   :linenos:

       // Explicit return type
       function foo(): string { return "foo" }

       // Implicit return type inferred as string
       function goo() { return "goo" }

       class Base {}
       class Derived1 extends Base {}
       class Derived2 extends Base {}

       function bar (condition: boolean) {
           if (condition)
               return new Derived1()
           else
               return new Derived2()
       }
       // Return type of bar will be Derived1|Derived2 union type

       function boo (condition: boolean) {
           if (condition) return 1
       }
       // That is a compile-time error as there is an execution path with no return


.. index::
   return type
   function
   method
   getter
   lambda
   value
   getter return type
   lambda return type
   function return type
   method return type
   return type inference
   native function
   void type
   type inference
   inferred type
   union type normalization
   union type
   never type
   async type
   async
   Promise
   method body
   return statement
   normalization
   expression type
   expression
   function implementation
   compiler

|

.. _Overriding:

重写
*******************************************************************************

方法重写与继承紧密相关。它允许子类或子接口为超类型中已有的方法提供具体实现，
并在允许的范围内修改签名。

运行时最终调用哪个方法，取决于对象的运行时类型，因此重写体现的是运行时多态。
构造函数不参与重写。

|LANG| 使用 ``override-compatible`` 规则检查重写是否合法。
只有当子类型中的方法签名与超类型方法 *override-compatible*
（见 :ref:`Override-Compatible Signatures`）时，重写才是合法的。
在某些情况下，实现还需要生成桥接方法（见 :ref:`Make a Bridge Method for Overriding Method`）。

.. index::
   overriding
   method overriding
   subclass
   subinterface
   supertype
   signature
   method signature
   runtime polymorphism
   inheritance
   parent class
   object type
   runtime
   override-compatibility
   override-compatible signature
   implementation
   bridge method

|

.. _Overriding in Classes:

类中的重写
===============================================================================

.. meta:
    frontend_status: Partly

只有可访问的方法与访问器才参与重写。
重写方法可以保持超类或超接口方法的访问修饰符，也可以把 ``protected`` 放宽为
``public``；否则报错。

如果子类中定义或继承了与超类/超接口同名的实例方法，则需按以下规则判断：

- 若签名不是 override-compatible，但擦除后又兼容，则报错；
- 若签名是 override-compatible，则子类方法重写超类型方法；
- 否则按隐式方法重载处理。

多个子类方法若同时重写同一个超类方法，则会报错。

.. note::
   只有 :ref:`Accessible` 的方法才参与重写；访问器在重写时也遵循同样规则。

重写方法可以保留超类或超接口方法的访问修饰符，也可以把 protected 放宽为 public（见 :ref:`Access Modifiers`）；否则发生 :index:`compile-time error`。

.. index::
   overloading
   class
   inheritance
   overriding
   constructor
   accessibility
   access
   private method
   method
   subclass
   accessor
   superclass
   access modifier
   implementation
   superinterface

.. code-block:: typescript
   :linenos:

      class Base {
         public public_member() {}
         protected protected_member() {}
         private private_member() {}
      }

      interface Interface {
         public_member()             // All members are public in interfaces
         private private_member() {} // Except private methods with default implementation
      }

      class Derived extends Base implements Interface {
         public override public_member() {}
            // Public member can be overridden and/or implemented by the public one
         public override protected_member() {}
            // Protected member can be overridden by the protected or public one
         override private_member() {}
            // A compile-time error occurs as private methods of a superclass or
            // a superinterface are not accessible in the derived class, and such
            // a declaration attempt has nothing to override. 
         private_member() {}
            // Will be a correct method declaration which is not related to 
            // private methods with the same name and signature from a supoer class
            // or superinterfaces
      }

若子类中定义或继承了与超类或超接口中实例方法同名的方法，则按以下语义规则处理：

- 若签名本身不 :ref:`Override-Compatible Signatures`，但把原始签名替换成
  类型擦除后的*有效签名类型*后又变得 override-compatible，则发生
  :index:`compile-time error`；
- 若签名 override-compatible，则子类方法重写超类型方法；
- 否则使用 :ref:`Implicit Method Overloading`。

.. index::
   context
   semantic check
   instance method
   subclass
   superclass
   override-compatible signature
   overriding
   override-compatibility

.. code-block:: typescript
   :linenos:

      class Base {
         method_1() {}
         method_2(p: number) {}
      }
      class Derived extends Base {
         override method_1() {} // overriding
         override method_2(p: string) {} // Compile-time error, not override-compatible
      }

超接口中的方法也可能通过超类中的实现被满足：

.. code-block:: typescript
   :linenos:

      interface I {
         m(): void
      }

      class Base {
         m() { }
      }
      class Derived extends Base implements I {
         // method 'm' inherited from 'Base' overrides 'm' defined in 'I'
      }

子类中的单个方法还可能同时重写超类中的多个方法：

.. code-block:: typescript
   :linenos:

       class B {}
       class C {
           foo(a: A) {}
           foo(b: B) {}
       }
       class D extends C {
           foo(o: Object) { console.log("foo in D")}
       }

       let c: C = new D()
       c.foo(new A()) // output: foo in D
       c.foo(new B()) // output: foo in D


若子类中不止一个方法同时重写了超类中的同一个方法，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

        class I{
           foo (p: C) {}
        }
        class C extends I { // More than one method of class C overrides the same method
           foo (p: C) {}
           foo (p: I) {}
        }

|

.. _Overriding in Interfaces:

接口中的重写
===============================================================================

.. meta:
    frontend_status: Done

如果子接口中定义了与超接口可访问方法同名的方法，则适用以下语义规则：

- 若签名不是 *override-compatible*（见 :ref:`Override-Compatible Signatures`），
  且使用原始签名的 *有效签名类型* 构成的签名在类型擦除后是 *override-compatible*，
  则发生 :index:`compile-time error`；

- 若签名是 override-compatible（见 :ref:`Override-Compatible Signatures`），
  则子接口方法在该子接口中重写超接口方法；

- 否则，使用 :ref:`Implicit Method Overloading`。

一个子接口方法可以重写多个超接口方法，但多个子接口方法不能同时重写同一个
超接口方法。

.. code-block:: typescript
   :linenos:

      interface Base {
         m(p: string): void
         m(p: number): void
      }
      interface Derived extends Base {
         m(p: object): void  // m overrides both Base.m(string) and Base.m(number)
      }

.. note::
   一个子接口方法可以重写多个超接口方法。

.. index::
   overriding
   interface
   subinterface
   name
   method
   superinterface
   signature
   override-compatible signature
   override-compatibility
   private method

.. code-block:: typescript
   :linenos:

      interface Base {
         method_1()
         method_2(p: number)
         method_3(): string
         method_4(a: Array<string>)
         private foo() {} // private method with implementation body
      }
      interface Derived extends Base {
         method_1()           // overriding
         method_2(p: string)  // overloading
         method_3(): number // Compile-time error, bad overriding, return type mismatch
         method_4(a: Array<number>)  // Compile-time error, original signatures are
                                     // not override-compatible, but effective
                                     // signatures after type erasure are compatible.
         foo(p: number): void // it is just a new method declaration
                              // Base.foo() is not accessible here at all
      }


若子接口中的多个方法同时重写了超接口中的同一个方法，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

        interface I{
           foo (p: C): void
        }
        interface C extends I { // More than one method of interface C overrides the same method
           foo (p: C): void
           foo (p: I): void
        }


|

.. _Override-Compatible Signatures:

可重写兼容签名
===============================================================================

.. meta:
    frontend_status: Partly

若派生类型中的签名 ``S2`` 要重写基类型中的签名 ``S1``，则至少要满足：

- 参数个数相同；
- 派生方法的每个参数类型都是基方法对应参数类型的超类型；
- 返回类型要么保持 ``this`` 语义兼容，要么是基方法返回类型的子类型；
- 类型参数数量相同；
- 派生方法类型参数约束需对基方法类型参数约束满足逆变兼容。

这一规则适用于类/接口类型、函数类型、联合类型以及带类型参数约束的场景。

可把这个规则具体化为：设 ``Derived`` 是 ``Base`` 的子类，且二者都声明了 ``foo``。
若在 ``Base`` 中 ``foo`` 的签名为 ``S1``，在 ``Derived`` 中 ``foo`` 的签名为 ``S2``，
则 ``S2`` 只有在以下条件全部满足时才与 ``S1`` override-compatible：

.. code-block:: typescript
   :linenos:

       class Base {
          foo <V1, ... Vk> (p1: U1, ... pn: Un): Un+1
       }
       class Derived extends Base {
          override foo <W1, ... Wj> (p1: T1, ... pm: Tm): Tm+1
       }

1. 两个方法的参数个数相同；
2. 派生方法中每个参数类型，都是基方法对应参数类型的超类型（逆变）；
3. 若派生方法返回类型是 ``this``，则基方法返回类型必须是 ``this``，或是当前类型的某个超接口或超类；
4. 若派生方法返回类型不是 ``this``，则该返回类型必须是基方法返回类型的子类型（协变）；
5. 两个方法的类型参数个数相同；
6. 派生方法类型参数约束要对基方法对应约束满足逆变兼容
   （见 :ref:`Invariance, Covariance and Contravariance`）。

.. index::
   signature
   override-compatible signature
   override compatibility
   class
   method
   parameter
   superinterface
   superclass
   return type
   contravariant
   covariance
   invariance
   constraint
   type parameter

对于泛型，还要额外满足：派生类的类型参数约束必须是基类型相应约束的子类型
（见 :ref:`Subtyping`）；否则会发生 :index:`compile-time error`。

.. index::
   generic
   derived class
   subtyping
   subtype
   type parameter
   base type

.. code-block:: typescript
   :linenos:

      class Base {}
      class Derived extends Base {}
      class A1 <CovariantTypeParameter extends Base> {}
      class B1 <CovariantTypeParameter extends Derived> extends A1<CovariantTypeParameter> {}
          // OK, derived class may have type compatible constraint of type parameters

      class A2 <ContravariantTypeParameter extends Derived> {}
      class B2 <ContravariantTypeParameter extends Base> extends A2<ContravariantTypeParameter> {}
          // Compile-time error, derived class cannot have non-compatible constraints of type parameters

这个语义可在下列场景中观察到：

1. **类/接口类型**

.. code-block:: typescript
   :linenos:


       interface Base {
           param(p: Derived): void
           ret(): Base
       }

       interface Derived extends Base {
           param(p: Base): void    // Contravariant parameter
           ret(): Derived          // Covariant return type
       }

2. **函数类型**

.. code-block:: typescript
   :linenos:


       interface Base {
           param(p: (q: Base)=>Derived): void
           ret(): (q: Derived)=> Base
       }

       interface Derived extends Base {
           param(p: (q: Derived)=>Base): void  // Covariant parameter type, contravariant return type
           ret(): (q: Base)=> Derived          // Contravariant parameter type, covariant return type
       }

3. **联合类型**

.. code-block:: typescript
   :linenos:

       interface BaseSuperType {}
       interface Base extends BaseSuperType {
          // Overriding for parameters
          param<T extends Derived, U extends Base>(p: T | U): void

          // Overriding for return type
          ret<T extends Derived, U extends Base>(): T | U
       }

       interface Derived extends Base {
          // Overriding kinds for parameters, Derived <: Base
          param<T extends Base, U extends Object>(
             p: Base | BaseSuperType // contravariant parameter type:  Derived | Base <: Base | BaseSuperType
          ): void
          // Overriding kinds for return type
          ret<T extends Base, U extends BaseSuperType>(): T | U
       }

4. **类型参数约束**

.. code-block:: typescript
   :linenos:


       interface Base {
           param<T extends Derived>(p: T): void
           ret<T extends Derived>(): T
       }

       interface Derived extends Base {
           param<T extends Base>(p: T): void       // Contravariance for constraints of type parameters
           ret<T extends Base>(): T                // Contravariance for constraints of the return type
       }

``Object`` 覆盖兼容性示例如下：

.. index::
   contravariance
   constraint
   return type
   type parameter
   override compatibility

.. code-block:: typescript
   :linenos:

       enum E  { One, Two }
       interface Base {
          kinds_of_parameters<T extends Derived, U extends Base>(
             // It represents all possible kinds of parameter type
             p01: Derived,
             p02: (q: Base)=>Derived,
             p03: number,
             p04: T | U,
             p05: E,
             p06: Base[],
             p07: [Base, Base]
          ): void
          kinds_of_return_type(): Object // It can be overridden by all subtypes of Object
       }
       interface Derived extends Base {
          kinds_of_parameters( // Object is a supertype for all class types
             p1: Object,
             p2: Object,
             p3: Object,
             p4: Object,
             p5: Object,
             p6: Object,
             p7: Object
          ): void
       }

       interface Derived1 extends Base {
          kinds_of_return_type(): Base // Valid overriding
       }
       interface Derived2 extends Base {
          kinds_of_return_type(): (q: Derived)=> Base // Valid overriding
       }
       interface Derived3 extends Base {
          kinds_of_return_type(): number // Valid overriding
       }
       interface Derived4 extends Base {
          kinds_of_return_type(): number | string // Valid overriding
       }
       interface Derived5 extends Base {
          kinds_of_return_type(): E // Valid overriding
       }
       interface Derived6 extends Base {
          kinds_of_return_type(): Base[] // Valid overriding
       }
       interface Derived7 extends Base {
          kinds_of_return_type(): [Base, Base] // Valid overriding
       }

.. index::
   override-compatible signature
   override-compatibility
   class
   base class
   derived class
   signature
   method
   field
   override

.. index::
   class type
   semantics
   interface type
   contravariant parameter
   covariant return type

.. index::
   function type
   covariant parameter type
   contravariant return type
   contravariant parameter type

.. index::
   union type
   return type
   parameter
   overriding

.. index::
   contravariance
   constraint
   return type
   type parameter
   override compatibility

.. index::
   parameter type
   overriding
   subtype
   supertype
   overriding
   compatibility

|

.. _Overloading:

重载
*******************************************************************************

重载允许使用同一名称调用多个不同签名的函数、方法或构造函数。
最终调用目标在编译期确定，因此它体现的是按名称的编译期多态。

|LANG| 支持以下两种 *重载* 机制：

- *隐式重载*：对函数和方法而言，由名称确定重载实体集合；对构造函数而言，
  则包含该类的全部构造函数；
- 显式重载（见 :ref:`Explicit Overload Declarations`）：允许开发者显式指定
  一个重载实体集合。

.. index::
   overloading
   name
   context
   entity
   function
   constructor
   method
   signature
   compile-time polymorphism
   polymorphism by name
   explicit overload declaration

无论采用哪种机制，编译期都会构造一个有序重载实体集合，并通过
:ref:`Overload Resolution` 从集合中选出一个要调用的实体。*重载决议*
采用 first-match 算法来简化选择过程，也就是说，只要找到第一个适用实体，
就调用该实体，其余实体不再继续考虑。

若 *隐式重载实体* 的签名是 *overload-equivalent*，则发生 :index:`compile-time error`
（见 :ref:`Overload-Equivalent Signatures`）；
显式重载实体不进行这项检查，具有独立名称的显式重载实体不会因此触发 :index:`compile-time error`。

模块作用域名称 ``main`` 不允许重载，若尝试重载则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       function main() {}
       function main(p: string[]) {}
       // Such declarations lead to a compile-time error


|

.. _Implicit Function Overloading:

隐式函数重载
===============================================================================

.. meta:
    frontend_status: None

同一声明作用域中两个或多个同名函数会形成 *隐式函数重载*。

.. note::
   声明在不同作用域中的同名函数不能形成 *隐式函数重载*
   （见 :ref:`Declaration Distinguishable by Signatures`）。
   若导入函数与当前模块中声明的函数同名，则发生 :index:`compile-time error`。

调用重载函数时（见 :ref:`Function Call Expression`），
通过 :ref:`Overload Resolution` 确定最终被调用的函数。

.. index::
   implicit function overloading
   declaration scope
   signature
   name
   overload-equivalence
   overload-equivalent signature
   overloaded function
   function call

.. code-block:: typescript
   :linenos:

       function foo(p: number) {} // #1
       function foo(p: string) {} // #2

       foo(5)   // function marked #1 is called
       foo("5") // function marked #2 is called

.. index::
   overload set
   call

若两个隐式重载的非泛型函数签名是 overload-equivalent
（见 :ref:`Overload-Equivalent Signatures`），则发生 :index:`compile-time error`。
但若某次调用中，泛型函数实例化后与普通函数形成等价签名，则不会因此报错，
而是使用文本顺序更靠前的函数：

.. code-block:: typescript
   :linenos:

        function goo(x: int): void {}
        function goo(x: int): void {}  // Compile-time error, overload-equivalent
                                       // non-generic functions

        function foo<T>(x: T) {}
        function foo(x: number) {}

        foo(1)   // OK, instance of foo<T>() with T=number called

        function bar(x: number) {}
        function bar<T>(x: T) {}

        bar(1)   // OK, plain bar() called

.. _Implicit Method Overloading:

隐式方法重载
===============================================================================

.. meta:
    frontend_status: None

类或接口中，满足以下条件的两个或多个方法构成 *隐式方法重载*：

- 所有方法同名；
- 所有方法同为 ``static`` 或同为非 ``static``。

若两个隐式重载方法的签名是 *overload-equivalent*
（见 :ref:`Overload-Equivalent Signatures`），则发生 :index:`compile-time error`。

.. index::
   implicit method overloading
   class
   signature
   overload-equivalent signature
   overload equivalence
   overloaded method
   method
   instantiation
   interface

隐式方法重载既可能由本类型中的方法声明产生，也可能由继承带来：

.. code-block:: typescript
   :linenos:

       class Base{
           foo(p: number) {} // #1
       }

       class Derived extends Base {
           foo(p: string) {} // #2
       }

       let d = new Derived()

       d.foo(5)   // method marked #1 is called
       d.foo("5") // method marked #2 is called

调用重载方法时（见 :ref:`Method Call Expression`），
通过 :ref:`Overload Resolution` 确定最终被调用的方法：

.. code-block:: typescript
   :linenos:

       class C{
           foo(p: number) {} // #1
           foo(p: string) {} // #2
       }

       let c = new C()

       c.foo(5)   // method marked #1 is called
       c.foo("5") // method marked #2 is called

若泛型类或泛型接口的某个实例化导致方法变成 overload-equivalent，
则按文本顺序调用第一个方法：

.. code-block:: typescript
   :linenos:

        class Template<T> {
           foo (p: T) { console.log("generic foo") }
           foo (p: number) { console.log("plain foo") }
           bar (p: number) { console.log("plain bar") }
           bar (p: T) { console.log("generic bar") }
        }

        // This instantiation leads to two *overload-equivalent* methods
        let instantiation: Template<number> = new Template<number>

        instantiation.foo(1)  // prints 'generic foo'
        instantiation.bar(1)  // prints 'plain bar'

.. index::
   implicit method overloading
   overload-equivalence
   overload-equivalent signature
   overloaded function

.. _Implicit Constructor Overloading:

隐式构造函数重载
===============================================================================

.. meta:
    frontend_status: None

一个类中的两个或多个构造函数天然构成 *隐式构造函数重载*。
若两个重载构造函数的签名是 *overload-equivalent*
（见 :ref:`Overload-Equivalent Signatures`），则发生 :index:`compile-time error`。

在类实例创建表达式（见 :ref:`New Expressions`）中，
通过 :ref:`Overload Resolution` 确定最终被调用的构造函数。

.. index::
   constructor overloading
   constructor
   class instance
   instance creation expression
   compile time

.. code-block:: typescript
   :linenos:

       class BigFloat {
           /*other code*/
           constructor (n: number) {/*body1*/} // #1
           constructor (s: string) {/*body2*/} // #2
       }

       new BigFloat(1)      // constructor, marked #1,  is used
       new BigFloat("3.14") // constructor, marked #2,  is used

.. _Overload-Equivalent Signatures:

重载等价签名
===============================================================================

.. meta:
    frontend_status: None

若两个签名满足以下条件，则它们是 *overload-equivalent*：

- 参数个数相同；
- 每个对应参数的有效签名类型（见 :ref:`Type Erasure`）相等。

返回类型不影响 *overload-equivalence* 的判断。

.. index::
   overload-equivalent signature
   signature
   overload equivalence

下面的代码会产生 :index:`compile-time error`，因为两个函数签名 overload-equivalent：

.. code-block:: typescript
   :linenos:

       function foo(x: Array<string>): string {}
       function foo(x: Array<number>): number {} // Compile-time error

.. _Overload Resolution:

重载决议
*******************************************************************************

.. meta:
    frontend_status: None

*重载决议* 用于在有序 *overload set* 中选出最终被调用的实体，
适用于函数调用、方法调用，以及 ``new`` 表达式（见 :ref:`New Expressions`）中的构造函数调用。

在声明处理阶段，会为每个重载名称形成唯一的 *overload set*
（见 :ref:`Forming an Overload Set`）。该过程考虑：

- 隐式重载实体的文本顺序；
- 显式重载声明中的列举顺序；
- 继承关系。

重载决议本身是直接的 first-match 过程：从 *overload set* 中按顺序逐个取出候选，
检查对该候选的调用在当前实参下是否有效。第一个有效候选就是最终选择的实体，
在找到第一个有效候选之后，其余候选不再继续检查。

若对所有候选都不成立，则发生 :index:`compile-time error`。

一个候选调用有效，当且仅当：

- 每个必需形参都有对应实参；
- 不存在无法对应到普通参数、可选参数或 rest 参数的多余实参；
- 每个实参都可赋给对应形参类型，或者在对应形参是 rest 参数时，
  可赋给该 rest 参数的元素类型。

重载决议不考虑重载实体的返回类型。这意味着，被选中的实体后续仍可能导致
:index:`compile-time error`，如下例所示：

.. code-block:: typescript
   :linenos:

       function foo(n: number): number {}
       function foo(s: string): string {}

       let x: number = foo("1") // Compile-time error

|

.. _Forming an Overload Set:

形成重载集合
===============================================================================

.. meta:
    frontend_status: None

每个重载名称在声明处理阶段都会形成唯一的重载集合。
其形成只能通过以下两种方式之一：

1. 若调用使用的名称指向显式重载声明，则重载集合就是该声明中列出的实体，且顺序与列举顺序一致；
2. 否则，若调用使用的名称对应隐式重载，则重载集合由同名实体按文本顺序组成。

若显式重载声明中的标识符指向的是“隐式重载名称”而不是单个可访问实体，则发生
:index:`compile-time error`。

显式重载声明在源码中的文本位置不会改变集合中的实体顺序；
在语义上，它等效于在作用域末尾统一处理。

接口方法和类实例方法形成集合时，本地声明的方法会排在任何继承方法之前，
也就是说，更具体的实体具有更高优先级。
详见 :ref:`Overload Set for Interface Methods` 和
:ref:`Overload Set for Class Instance Methods`。

.. index::
   overload set
   explicit overload
   implicit overloading
   textual order
   compile-time error

.. _Overload Set for Functions:

函数的重载集合
===============================================================================

.. meta:
    frontend_status: None

对给定函数名，重载集合由当前作用域中相关函数与显式重载声明按规则组合而成。
函数的显式重载与隐式重载不会任意混合。

若名称对应显式函数重载，则集合为显式重载列出的有序列表；否则若名称对应隐式函数重载，
则集合为同名函数按文本顺序排列的列表。

.. code-block:: typescript
   :linenos:

       function foo() {}           // implicitly overloaded foo#1
       function foo(b: boolean) {} // implicitly overloaded foo#2
       // The overload set for 'foo' is {foo#1, foo#2}

       function foo0() {}
       function foo1(b: boolean) {}
       overload bar {foo0, foo1}
       // The overload set for 'bar' is {foo0, foo1}

       foo()     // foo#1 is called
       foo(true) // foo#2 is called
       bar()     // foo0 is called
       bar(true) // foo1 is called

.. index::
   overload set for functions
   explicit function overload
   implicit function overloading

.. _Overload Set for Interface Methods:

接口方法的重载集合
===============================================================================

.. meta:
    frontend_status: None

接口方法的重载集合由以下内容组成：

- 接口中的隐式重载方法（见 :ref:`Implicit Method Overloading`），
  或显式重载方法（见 :ref:`Explicit Interface Method Overload`）；
- 各超接口的重载集合（如有）。

形成步骤如下：

1. 先在当前接口中按 :ref:`Forming an Overload Set` 形成本地集合；
2. 再按 ``extends`` 列表顺序，把各直接超接口的重载集合依次追加到末尾；
3. 已经加入过的同一方法不会再次加入。

.. note::
   非直接超接口不单独处理，因为它们已经被直接超接口的集合覆盖。

无超接口时的集合形成示例：

.. code-block:: typescript
   :linenos:

       interface I {
           foo(x: number) // #1
           foo(s: string) // #2
           // The overload set for 'foo' is {foo#1, foo#2}
       }

       interface J {
           fooNum(x: number)
           fooStr(s: string)
           overload foo { fooNum, fooStr }
           // The overload set for 'foo' is {fooNum, fooStr}
       }

有超接口时的集合形成示例：

.. code-block:: typescript
   :linenos:

       interface I1 {
           foo(x: number)  // #1
       }
       interface I2 {
           foo(s: string)  // #2
           foo(b: boolean) // #3
       }
       interface J extends I1, I2 { // the order in extends list is used
           foo() // #4
           /* The overload set is {foo#4, foo#1, foo#2, foo#3}
              Formed as: set(J)={foo#4} append set(I1)={foo#1} append set(I2)={foo#2, foo#3}
           */
       }

被覆盖的方法在集合中只出现一次：

.. code-block:: typescript
   :linenos:

       interface K extends I1, I2 {
           foo(s: string) // #2 as it overrides I2.foo
           /* The overload set is {foo#2, foo#1, foo#3}
              Formed as: set(K)={foo#2} append set(I1)={foo#1} append set(I2)={foo#2, foo#3}
              Second occurrence of foo#2 is skipped.
           */
       }

.. index::
   overload set for interface methods
   superinterface
   extends clause
   duplicate elimination

.. _Overload Set for Class Static Methods:

类静态方法的重载集合
===============================================================================

.. meta:
    frontend_status: None

静态方法的重载集合按如下规则形成（见 :ref:`Forming an Overload Set`）：

- 若名称对应显式类方法重载，则集合为显式声明列出的实体；
- 否则若名称对应隐式方法重载，则集合为当前类中同名静态方法按文本顺序组成的列表。

静态方法不参与继承，因此超类静态方法不会进入该集合。
显式静态重载与隐式静态重载不会混合；同一个名称不会把两类来源合并到一个集合中。

.. code-block:: typescript
   :linenos:

       class C {

           static foo0() {}
           static foo1(b: boolean) {}
           static overload foo {foo0, foo1}
           // The overload set for 'foo' is {foo0, foo1}
       }

       C.foo()     // foo0 is called
       C.foo(true) // foo1 is called

.. index::
   overload set for class static methods
   static method
   explicit class method overload

.. _Overload Set for Class Instance Methods:

类实例方法的重载集合
===============================================================================

.. meta:
    frontend_status: None

类实例方法的重载集合由以下内容组成：

- 类中的隐式重载方法（见 :ref:`Implicit Method Overloading`），
  或显式重载方法（见 :ref:`Explicit Class Method Overload`）；
- 直接超类的方法（如有）。

形成步骤如下：

1. 先在当前类中按 :ref:`Forming an Overload Set` 形成本地集合，
   包括覆盖或实现超类型方法的方法；
2. 将直接超类中的重载集合追加到末尾，重复方法跳过；
3. 再按 ``implements`` 列表顺序追加各直接超接口中的重载集合，重复方法同样跳过。

.. note::
   非直接超类型不单独处理。

下面的例子说明了没有超类型时的集合形成方式：

.. code-block:: typescript
   :linenos:

       class C {
           foo(x: number) {} // #1
           foo(s: string) {} // #2
           // The overload set for 'foo' is {foo#1, foo#2}
       }

       class D {
           fooNum(x: number) {}
           fooStr(s: string) {}
           overload foo { fooNum, fooStr }
           // The overload set for 'foo' is {fooNum, fooStr}
       }

当类同时拥有直接超类和直接超接口时，集合形成如下：

.. code-block:: typescript
   :linenos:

       interface I {
           foo(x: number)  // #1
       }

       class C {
           foo(s: string) {} // #2
       }

       class D extends C implements I{
           foo(x: number) {}  // overrides #1
           foo(x: boolean) {} // #3
           /* The overload set is {foo#1, foo#3, foo#2}
              Formed as: set(D)={foo#1, foo#3} append set(C)={foo#2} append set(I)={foo#1}
              Second occurrence of foo#1 is skipped.
           */
       }

只考虑直接超类型这一点，也可由下面的例子说明：

.. code-block:: typescript
   :linenos:

       interface I{
           foo(x: number) {} // #1
       }
       class C implement I{
           foo(x: A)      // #2
           // Note: foo in I has default body, no need to implement it in C
           // The overload set is {foo#2, foo#1}
       }
       class D extends C {
           foo(s: string) {}   // #3
           foo(x: A | undefined) {} // overrides #2
           /* The overload set is {foo#3, foo#2, foo#1}
              Formed as: set(D)={foo#3, foo#2} append set(C)={foo#2, foo#1}
              Second occurrence of foo#2 from set(C) is skipped.
           */
       }
       class E extends D {
           foo(x: number) {} // overrides #1
           /* The overload set is {foo#1, foo#3, foo#2}
              Formed as: set(E)={foo#1} append set(D)={foo#3, foo#2, foo#1}
              Second occurrence of foo#1 from set(D) is skipped.
           */
       }

.. note::
   更完整的运行时影响见 :ref:`Overloading and Overriding`。

.. index::
   overload set for class instance methods
   direct superclass
   direct superinterface
   overriding
   overload resolution

.. _Overload Set for Constructors:

构造函数的重载集合
===============================================================================

.. meta:
    frontend_status: None

对给定类，构造函数的重载集合由隐式重载构造函数
（见 :ref:`Implicit Constructor Overloading`）组成，
其顺序等于构造函数文本出现顺序。

下面的例子同时说明了重载集合的形成以及重载解析如何从中选择构造函数：

.. code-block:: typescript
   :linenos:

       class C {
           constructor () {}       // ctor#1
           constructor (s: string) {} // ctor#2
       }
       /* The overload set of constructors for class 'C' is
          {ctor#1, ctor#2} */

       new C()     // ctor#1 is used
       new C("aa") // ctor#2 is used

.. index::
   overload set for constructors
   constructor
   textual order

.. _Overload Set Warning:

重载集合告警
===============================================================================

.. meta:
    frontend_status: None

如果重载集合中的顺序导致某些候选永远不可能被选中，则会发出 :index:`compile-time warning`。

.. code-block:: typescript
   :linenos:

       function f1 (p: number) {}
       function f2 (p: number|string) {}
       function f3 (p: string) {}
       overload foo {f1, f2, f3}  // warning: f3 will never be called as foo()

       foo (5)                    // f1() is called
       foo ("5")                  // f2() is called

.. index::
   overload set warning
   compile-time warning
   unreachable overload

.. _Overload Set at Method Call:

方法调用处的重载集合
===============================================================================

方法调用（:ref:`Method Call Expression`）时需要对 *重载集合* 进行额外处理，
因为调用点的标识符可能同时表示 *实例方法* 与 :ref:`Functions with Receiver`。
若标识符同时表示实例方法与带接收者函数，则优先使用方法重载集合；
若因此屏蔽了接收者函数，可能会给出告警。

更具体地说：

- 若调用点名称只表示 *functions with receiver*，则使用 :ref:`Overload Set for Functions`，
  但候选仅限带接收者函数：

  .. code-block:: typescript
     :linenos:

         class C {}

         function foo(this: C) {}            // #1
         function foo(this: C, s: string) {} // #2
         function foo(c: C, n: number) {}    // #3

         let c = new C()
         c.foo() // only function #1, #2, but not #3 are considered here

         foo(c) // all three functions are considered here

- 若调用点名称同时表示实例方法和带接收者函数，则重载决议只使用方法重载集合；
- 在后一种场景中，若接收者函数因此被遮蔽，则可能发出 :index:`compile-time warning`：

  .. code-block:: typescript
     :linenos:

         // File1
         export class C {
             bar() {}
         }
         export function foo(this: C) {}

         // File2
         import {C, foo as bar} from "File1"

         new C().bar() // C.bar is called, warning is issued

.. note::
   若声明了带接收者函数，而该函数名又与接收者类型中可访问的实例方法或字段同名，
   大多数情况下会发生 :index:`compile-time error`（参见 :ref:`Functions with Receiver`）。
   只有未触发该错误而仍发生遮蔽时，才会退化为 warning。

.. index::
   overload set at method call
   function with receiver
   instance method
   compile-time warning

.. _Overloading and Overriding:

重载与重写
===============================================================================

调用重载方法（类实例方法或接口方法）时，:ref:`Overloading` 和 :ref:`Overriding`
会共同影响最终调用的方法。*重载解析* 作为 *编译期* 多态，从编译期已知的类类型或接口类型中
选择要调用的方法。但该方法可能在子类型中被重写，最终调用的方法取决于调用对象的 *运行时类型*。

.. note::
   重写本身不会影响重载集合的形成——类中声明的任何方法（无论是新声明还是重写）
   都会在任何继承方法之前加入重载集合。

*重载* 与 *重写* 如何共同影响方法调用，见以下示例：

.. code-block:: typescript
   :linenos:

       class C1 {}
       class C2 extends C1 {}

       class A {
           foo(c: C2) { console.log("A.C2") }
           foo(c: C1) { console.log("A.C1") }
       }

       class B extends A {
           override foo(c: C2) { console.log("B.C2") }
       }

       let a: A = new B()
       a.foo(new C2()) // 1st call output: B.C2
       a.foo(new C1()) // 2nd call output: A.C1

第一次调用的关键点是：

- ``a`` 的静态类型是 ``A``，因此重载解析时只考虑 ``A`` 中可见的方法；
- 编译期会先选择 ``foo(c: C2)`` 这个重载；
- 运行时 ``a`` 的实际类型是 ``B``，而 ``B`` 重写了该方法，因此最终调用 ``B.C2``。

第二次调用的关键点是：

- 编译期选择的是 ``foo(c: C1)``；
- 该方法没有在 ``B`` 中被重写；
- 因此运行时仍调用从 ``A`` 继承下来的原始实现。

子类中单个方法重写超类多个方法的情形，示例如下：

.. code-block:: typescript
   :linenos:

       class C1 {}
       class C2 extends C1 {}

       class A {
           foo(c: C2) { console.log("A.C2") }
           foo(c: C1) { console.log("A.C1") }
       }

       class D extends A {
           foo(c: C1) { console.log("D.C1") }
       }

       let a: A = new D()
       a.foo(new C2()) // 1st call output: D.C1
       a.foo(new C1()) // 2nd call output: D.C1

在上例中，``D`` 中的 ``foo`` 重写了 ``A`` 中的两个方法（见
:ref:`Override-Compatible Signatures`）。因此，尽管编译期为第一次和第二次调用
选择了不同的方法，运行时调用的却是同一个方法。

|

.. _Dynamic resolution of method calls:

方法调用的动态解析
===============================================================================

.. meta:
    frontend_status: Done

在 :ref:`Method Call Expression` 求值时，运行时会以编译期静态解析结果（见
:ref:`Overload Set at Method Call`）为基础，再结合 ``objectReference`` 的实际运行时类型
解析最终方法。

解析结果取决于调用表达式的形式：

- 对 *静态方法调用*，不使用重写分派，直接调用编译期确定的方法；
- 对 ``super`` 调用，不使用重写分派，直接调用编译期静态解析到的超类方法；
- 对 *实例方法调用*，要结合 ``objectReference`` 的实际运行时类型继续查找最合适的实现。

对编译期静态解析到、定义于类型 ``C`` 中的方法 ``M``，运行时以对象的实际类型 ``T`` 为起点：

- 若 ``T`` 就是 ``C``，则直接调用 ``M``；
- 若 ``T`` 有超类，且在对其超类递归解析 ``M`` 时得到某个类方法 ``Ms``，则：

  - 若 ``T`` 自身声明了恰好一个覆盖 ``Ms`` 的方法，则该方法成为解析结果；
  - 若 ``T`` 自身声明了多个覆盖 ``Ms`` 的方法，则对 ``T`` 的解析失败；
  - 否则沿用 ``Ms``。

- 否则在 ``T`` 的超接口中查找满足覆盖条件且最具体的默认方法：

  - 每个候选方法都必须声明在 ``T`` 的某个超接口中；
  - 每个候选方法都必须覆盖 ``C`` 中的 ``M``；
  - 对每个候选 ``Mi``，不能存在另一个同样满足条件且更具体的候选 ``Mio`` 覆盖 ``Mi``。

- 若候选默认方法不唯一，或找不到唯一候选，则动态解析失败。

若动态解析失败，则会发生 :index:`runtime error`。

若接口默认方法在运行期形成多个同等具体的候选，或者根本找不到唯一候选，
则动态解析会失败并抛出运行时错误。

.. note::
   对一组在编译后未发生源代码更新的程序而言，类型 ``C`` 通常就等于编译期静态解析时
   定义方法 ``M`` 的那个类型；这时动态解析不会因为签名漂移而失败。

.. index::
   dynamic resolution of method calls
   runtime type
   overriding
   default method
   runtime error
   static method
   super call
   superclass
   superinterface

|

.. _Type Erasure:

类型擦除
*******************************************************************************

.. meta:
    frontend_status: Done

*类型擦除* 描述的是：某些语言类型在特定语义场景下，需要被映射到一个更小的
“有效类型”子系统中。

有效类型映射满足以下性质：

- 对任意类型 ``A`` 和 ``B``，若 ``A`` 是 ``B`` 的子类型，则 ``EffectiveType(A)``
  也是 ``EffectiveType(B)`` 的子类型；
- ``EffectiveType(A | B)`` 等于 ``EffectiveType(A) | EffectiveType(B)``；
- 对任意类型 ``A``，``A | undefined`` 是 ``EffectiveType(A | undefined)`` 的子类型；

  - 特别地，对任意类型 ``A``，``undefined`` 也是 ``EffectiveType(A)`` 的子类型。

原始类型与其 *有效类型* 之间有两种关系：

- 若 ``T`` 的有效类型与 ``T`` 本身相同，则称 ``T`` *在类型擦除下被保留*；
- 否则，称 ``T`` *受类型擦除影响*。

若 ``T | undefined`` 在类型擦除下被保留，则称 ``T`` *在 undefined 意义下被保留*。

这一性质会限制以下依赖运行时类型信息的表达式：

- :ref:`instanceof Expression`
- :ref:`Cast Expression`

.. index::
   type erasure
   instanceof expression
   cast expression
   execution
   operation
   type
   generic
   subtype
   supertype
   effective type

类型映射按如下方式确定有效类型（为简洁起见，右侧列中的联合类型默认省略 ``undefined`` 成员）：

.. list-table::
   :width: 100%
   :widths: 45 55
   :header-rows: 1

   * - **原始类型**
     - **有效类型（省略 undefined 成员）**
   * - ``Any``
     - ``Any``
   * - ``never``
     - ``never``
   * - ``undefined``
     - ``undefined``
   * - ``null``
     - ``null``
   * - 泛型类型（见 :ref:`Generics`）
     - 按 :ref:`Type Parameter Variance` 选择类型实参后，对同一泛型类型执行 :ref:`Explicit Generic Instantiations`：

       - *协变* 类型参数以约束类型实例化；
       - *逆变* 类型参数以 ``never`` 实例化。
   * - :ref:`Type Parameters`
     - :ref:`Type Parameter Constraint`
   * - :ref:`Union Types`，形式为 ``T1 | T2 ... Tn``
     - ``T1 ... Tn`` 各成员类型有效类型的联合。
   * - :ref:`Array Types`，形式为 ``T[]``
     - 与泛型类型 ``Array<T>`` 的规则相同。
   * - :ref:`Fixed-Size Array Types` （``FixedArray<T>``）
     - ``FixedArray<T>`` 的实例化结果，也就是保留类型实参 ``T`` 的有效类型。
   * - :ref:`Function Types`，形式为 ``(P1, P2, Pn) => R``
     - 依据参数个数 ``n`` 实例化内部泛型 *函数* 类型。参数类型 ``P1 ... Pn`` 以 ``Any`` 实例化，返回类型 ``R`` 以 ``never`` 实例化。
   * - :ref:`Function Types`，形式为 ``(P1, P2, Pn, ...PR) => R``
     - 依据参数个数 ``n`` 实例化内部泛型 *rest 参数函数* 类型。参数类型 ``P1 ... Pn`` 与 rest 参数类型 ``PR`` 以 ``Any`` 实例化，返回类型 ``R`` 以 ``never`` 实例化。
   * - :ref:`Tuple Types`，形式为 ``[T1, T2 ..., Tn]``
     - 依据元素个数 ``n`` 实例化内部泛型元组类型。
   * - :ref:`String Literal Types`
     - ``string``
   * - ``Awaited<T>``
     - - 如果 ``T`` 既不是类型参数也不是 ``Promise`` 的子类型，则 ``Awaited<T>`` 的有效类型等于 ``T`` 的有效类型；
       - 如果 ``T`` 是 ``Promise<U>``，则 ``Awaited<T>`` 的有效类型等于 ``U`` 的有效类型；
       - 如果 ``T`` 是带 ``in`` 变型的类型参数，则 ``Awaited<T>`` 的有效类型为 ``never``；
       - 如果 ``T`` 是带 ``out`` 变型或未显式声明变型的类型参数，则 ``Awaited<T>`` 的有效类型等于 ``upper-bound(T)`` 的有效类型。
   * - ``NonNullable<T>``
     - ``Effective type (T) - null``
   * - ``Partial<T>``
     - 特殊的运行时 partial 类
   * - ``Required<T>``
     - ``Effective type (T)``
   * - ``Readonly<T>``
     - ``Effective type (T)``
   * - ``Record<K, V>``
     - ``Map <Effective type (K), Effective type (V)>``
   * - ``ReturnType<F>``
     - ``Effective type (return type of F)``

附加类型映射还定义了 *有效签名类型*，也就是对应类型的有效类型，但存在如下例外：

.. list-table::
   :width: 100%
   :widths: 45 55
   :header-rows: 1

   * - **原始类型**
     - **有效签名类型**
   * - 返回类型 ``never``
     - ``never``

除此之外，原始类型都会被保留。

泛型、类型参数、联合类型、数组类型、函数类型、元组类型、字符串字面量类型、
以及若干 utility types 都有各自的有效类型映射规则。

若程序使用了在擦除后 *不被保留* 的类型，并依赖某些强制转换表达式（见 :ref:`Cast Expression`）
将值收窄为 *在 undefined 意义下不被保留* 的类型，则在求值
:ref:`Field Access Expression`、:ref:`Method Call Expression` 或
:ref:`Function Call Expression` 时可能抛出 ``ClassCastError``。
上述运行时检查的目的，就是在这些场景下维持程序执行的类型安全。

.. index::
   type erasure
   type mapping
   generic type
   type parameter
   effective type
   instantiation
   type argument
   covariance
   contravariance
   tuple type
   type preservation
   runtime error

.. code-block:: typescript
   :linenos:

       class A<T> {
         field: T

         constructor(value: T) {
           this.field = value
         }
       }

       function unsafe(p: Object): A<number> {
         return p as A<number>  // OK, but check is performed against type A<>, but not against A<number>
                                // thus it can cause exception later during execution
       }

       let a: A<number> = unsafe(new A<string>("a")) // A<string> resides in A<number>

       let n = a.field // An exception is raised as the expected type is number but the actual type is string

同理，如果某个经擦除后仍能通过外层检查的值被继续当作更具体的参数化类型使用，
那么真正访问字段、调用方法或函数时，运行时检查才会在需要的位置失败。

.. index::
   access
   type erasure
   field access
   function call
   method call
   target type
   cast expression
   ClassCastError

|

.. _Static Initialization:

静态初始化
*******************************************************************************

.. meta:
    frontend_status: Done

*静态初始化* 会对类（见 :ref:`Classes`）、命名空间（见 :ref:`Namespace Declarations`）
或模块（见 :ref:`Namespaces and Modules`）执行一次性的初始化流程。
它包括：

- 变量或静态字段的 *初始化器*；
- 模块与命名空间的顶层语句；
- 类中静态块内的代码。

静态初始化会在以下任一操作首次执行之前发生：

- 首次调用类静态方法（见 :ref:`Method Call Expression`）、访问类静态字段
  （见 :ref:`Accessing Static Fields`）、创建类实例（见 :ref:`New Expressions`），
  或触发其直接子类的静态初始化；
- 首次调用命名空间或模块中的函数，或访问其中的变量。

.. note::
   若同一实体的静态初始化尚未完成，上述操作不会递归再次触发该实体自身的
   静态初始化。

   命名空间中的静态块仅在程序实际使用到该命名空间成员时才执行
   （示例见 :ref:`Namespace Declarations`）。

若静态初始化因为抛出异常而中止，则该次初始化不算完成；后续再次触发时，
仍可能再次抛出异常。

若静态初始化触发了并发执行（见 :ref:`Execution model`），所有试图触发它的
|C_JOBS| 都必须同步，确保初始化只执行一次，并确保依赖该初始化的操作在其完成后
再继续。

若两个静态初始化存在循环依赖，则可能导致死锁。

.. index::
   static initialization
   routine
   class
   namespace
   namespace declaration
   module
   initializer
   variable
   static field
   top-level statement
   static block

.. index::
   static initialization
   entity
   scope
   static field
   variable
   access
   direct subclass
   subclass
   class
   interface
   operation
   exception
   invocation
   concurrent execution
   synchronization
   deadlock

.. _Static Initialization Safety:

静态初始化安全
===============================================================================

.. meta:
    frontend_status: Done

若命名引用指向尚未初始化的实体（包括以下情形），则发生
:index:`compile-time error`：

- 模块或命名空间（见 :ref:`Namespace Declarations`）的变量（见
  :ref:`Variable and Constant Declarations`）；

- 类的静态字段（见 :ref:`Static and Instance Fields`）。

若编译器无法静态检测对尚未初始化 *实体* 的访问，则运行时处理如下：

- 先尝试产生默认值；
- 若无默认值，则抛出 ``NullPointerError``。

.. index::
   static initialization
   safety
   named reference
   initialization
   entity
   variable
   module
   namespace
   static field
   class
   access
   runtime evaluation
   default value
   value
   type

|

.. _Compatibility Features:

兼容性特性
*******************************************************************************

.. meta:
    frontend_status: Done

部分语言特性是为了平滑支持 |TS| 兼容性而加入的。
在纯 |LANG| 开发中，通常并不推荐依赖这些兼容性特性。

.. index::
   compatibility

.. _Extended Conditional Expressions:

扩展条件表达式
===============================================================================

.. meta:
    frontend_status: Done

|LANG| 为了更好地与 |TS| 对齐，为若干条件表达式提供了扩展语义。
这些扩展语义影响：

- :ref:`Ternary Conditional Expressions`；

- :ref:`Conditional-And Expression`；

- :ref:`Conditional-Or Expression`；

- :ref:`Logical Complement`；

- :ref:`While Statements and Do Statements`；

- :ref:`For Statements`；

- :ref:`if Statements`。

.. note::
   这套扩展语义将在 |LANG| 未来的某个版本中废弃。

这套扩展语义建立在 *truthiness* 概念之上，也就是允许某些非布尔值被当作
真或假来处理。

下表给出了主要值类型在扩展条件表达式中的真假判定规则：

.. index::
   extended conditional expression
   ternary conditional expression
   expression
   alignment
   semantics
   conditional-and expression
   conditional-or expression
   logical complement
   while statement
   do statement
   for statement
   if statement
   truthiness
   non-boolean type
   expression type

.. list-table::
   :width: 100%
   :widths: 25 25 25 25
   :header-rows: 1

   * - 值类型类别
     - 判定为 ``false`` 时
     - 判定为 ``true`` 时
     - 对应的 |LANG| 检查示例
   * - ``string``
     - 空字符串
     - 非空字符串
     - ``s.length == 0``
   * - ``boolean``
     - ``false``
     - ``true``
     - ``x``
   * - ``char``
     - ``c'\\u0000'``
     - 除 ``c'\\u0000'`` 外的任意值
     - ``x``
   * - ``enum``
     - 被当作 ``false`` 处理的枚举常量
     - 被当作 ``true`` 处理的枚举常量
     - ``x.valueOf()``
   * - ``number``（``double`` / ``float``）
     - ``0`` 或 ``NaN``
     - 其他任意数字
     - ``n != 0 && !isNaN(n)``
   * - 任意整数类型
     - ``== 0``
     - ``!= 0``
     - ``i != 0``
   * - ``bigint``
     - ``== 0n``
     - ``!= 0n``
     - ``i != 0n``
   * - ``null`` 或 ``undefined``
     - 始终为假
     - 不适用
     - ``x != null`` 或 ``x != undefined``
   * - 联合类型
     - 运行时值按其实际成员对应的假值规则判定
     - 运行时值按其实际成员对应的真值规则判定
     - 对带 nullish 成员的联合，常见检查是 ``x != null`` 或 ``x != undefined``
   * - 其他任意非 nullish 类型
     - 从不为假
     - 始终为真
     - ``new SomeType != null``

.. index::
   value type
   integer type
   union type
   nullish type
   empty string
   non-empty string
   string
   number
   nonzero

扩展语义还会影响 ``&&`` 与 ``||`` 的结果类型：

- 若左操作数的真值在编译期可知，则结果类型更接近被实际返回的分支；
- 若左操作数真值不可知，则结果类型通常是左右操作数的联合类型。

它也会直接影响以下表达式或语句的静态可接受性与运行时求值：

- :ref:`Ternary Conditional Expressions`
- :ref:`Conditional-And Expression`
- :ref:`Conditional-Or Expression`
- :ref:`Logical Complement`
- ``while`` / ``do`` / ``for`` / ``if`` 条件

.. note::
   这套扩展语义计划在未来某个版本中弃用，因此不应继续扩大新代码对它的依赖。

这种实现方式在实践中的效果如下例所示。任意非零数值被当作 ``true``，循环持续
执行直到数值变为 ``zero``（被视为 ``false``）：

.. code-block-meta:

.. code-block:: typescript
   :linenos:

       console.log(typeof (false || 1) )
       console.log(typeof (true || 1) )
       for (let i = 10; i; i--) {
          console.log (i)
       }
       /* And the output will be
            int
            boolean
            10
            9
            8
            7
            6
            5
            4
            3
            2
            1
        */

.. index::
   NaN
   nullish expression
   numeric expression
   semantics
   conditional-and expression
   conditional-or expression
   loop

.. index::
   extended conditional expressions
   truthiness
   union type
   nullish type
   if statement
   while statement
   conditional-and expression

.. raw:: pdf

   PageBreak
