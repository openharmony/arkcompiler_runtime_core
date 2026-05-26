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


.. _Generics:

泛型
################################################################################

.. meta:
    frontend_status: Partly

类、接口、类型别名、方法和函数都可以在 |LANG| 中用一个或多个类型进行参数化。经过这种参数化的实体会引入 *泛型声明*（简称 *泛型*）。

在泛型中用作参数的类型称为类型参数（参见 :ref:`Type Parameters`）。

泛型必须先实例化才能使用。*泛型实例化* 是把一个泛型转换为真实程序实体的动作，这个真实程序实体可以是非泛型类、接口、联合、数组、方法或函数，也可以是另一个泛型实例化。实例化（参见 :ref:`Generic Instantiations`）既可以显式进行，也可以隐式进行。

|LANG| 还提供一类在语法上看起来类似泛型的特殊类型，它们允许在编译期间创建新类型，也就是 :ref:`Utility Types`。

.. index::
   class
   array
   interface
   type alias
   method
   function
   entity
   parameterization
   generic
   generic declaration
   generic instantiation
   explicit instantiation
   instantiation
   program entity
   generic parameter
   type parameter
   generic instantiation
   utility type

|

.. _Type Parameters:

类型参数
********************************************************************************

.. meta:
    frontend_status: Done

*类型参数* 声明在类型参数部分中。它可以在泛型内部像普通类型一样使用。

从语法上看，*类型参数* 是具有正确作用域的非限定标识符。每个类型参数都可以带 *约束*（参见 :ref:`Type Parameter Constraint`），也可以带默认类型（参见 :ref:`Type Parameter Default`），并可指定其 *in-* 或 *out-* 变型（参见 :ref:`Type Parameter Variance`）。类型参数的作用域见 :ref:`Scopes`。

.. index::
   generic parameter
   generic
   class
   interface
   function
   parameterization
   type parameter
   unqualified identifier
   scope
   constraint
   default type
   type parameter
   variance
   out-variance
   in-variance
   syntax

类型参数的语法如下：

.. code-block:: abnf

    typeParameters:
        '<' typeParameterList '>'
        ;

    typeParameterList:
        typeParameter (',' typeParameter)*
        ;

    typeParameter:
        ('in' | 'out')? identifier constraint? typeParameterDefault?
        ;

    constraint:
        'extends' type
        ;

    typeParameterDefault:
        '=' typeReference ('[]')?
        ;

泛型类、接口、类型别名、方法或函数分别定义了一组参数化后的类、接口、联合、数组、方法或函数（参见 :ref:`Generic Instantiations`）。对于同一套类型参数，每组类型实参只定义一个对应实例化集合。

.. index::
   generic declaration
   generic class
   generic interface
   generic function
   generic instantiation
   class
   interface
   function
   type parameter
   parameterization
   array
   type alias
   method
   syntax

|

.. _Type Parameter Constraint:

类型参数约束
================================================================================

.. meta:
    frontend_status: Done

如果需要限制可用实例化，就可以在每个类型参数后通过关键字 ``extends`` 为其设置 *约束*。约束可以采用任意类型的形式。

如果没有显式给出约束，则默认约束为 :ref:`Type Any`，即缺省约束在效果上等价于 ``extends Any``。因此，该类型参数与 :ref:`Type Object` 不兼容，也没有可用的方法或字段。

如果类型参数 *T* 的约束为 *S*，那么泛型实例化中的实际类型必须是 *S* 的子类型（参见 :ref:`Subtyping`）。如果约束 *S* 是非空值类型（参见 :ref:`Nullish Types`），则 *T* 也同样是非空值的。

.. index::
   constraint
   instantiation
   type parameter
   extends keyword
   type reference
   union type normalization
   object
   compatibility
   assignability
   nullish-type
   non-nullish-type
   any type
   type argument
   generic instantiation
   instantiation
   subtyping
   subtype


.. code-block:: typescript
   :linenos:

    class Base {}
    class Derived extends Base { }
    class SomeType { }

    class G<T extends Base> { }

    let x = new G<Base>      // OK
    let y = new G<Derived>   // OK
    let z = new G<SomeType>  // Compile-time : SomeType is not compatible with Base

    class H<T extends Base|SomeType> {}
    let h1 = new H<Base>     // OK
    let h2 = new H<Derived>  // OK
    let h3 = new H<SomeType> // OK
    let h4 = new H<Object>   // Compile-time : Object is not compatible with Base|SomeType

    class Exotic<T extends "aa"| "bb"> {}
    let e1 = new Exotic<"aa">   // OK
    let e2 = new Exotic<"cc">  // Compile-time : "cc" is not compatible with "aa"| "bb"

    class A {
      f1: number = 0
      f2: string = ""
      f3: boolean = false
    }
    class B <T extends keyof A> {}
    let b1 = new B<'f1'>    // OK
    let b2 = new B<'f0'>    // Compile-time error as 'f0' does not fit the constraint
    let b3 = new B<keyof A> // OK

同一个泛型中的某个类型参数也可以依赖于更早声明的类型参数。

.. index::
   type parameter
   generic

.. code-block:: typescript
   :linenos:

    class G<T, S extends T> {}

    class Base {}
    class Derived extends Base { }
    class SomeType { }

    let x: G<Base, Derived>  // OK, the second argument directly
                             // depends on the first one
    let y: G<Base, SomeType> // error, SomeType does not depend on Base

如果类型参数部分中的某个类型参数直接或间接依赖于自己，则会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

    class C<T extends T> {} // circular dependency
    class D<T extends R, R extends T> {} // circular dependency
    class E<T extends R, R extends T | undefined> {} // circular dependency

|

.. _Type Parameter Default:

类型参数默认值
================================================================================

.. meta:
    frontend_status: Done

泛型类型的类型参数可以带默认值。这样在使用某种具体实例化时，就允许省略对应的类型实参。不过，在以下情况下会产生 :index:`compile-time error`：

- 在泛型类型声明中，没有默认类型的类型参数出现在带默认类型的类型参数之后；
- 类型参数默认值引用了定义在当前类型参数之后的类型参数。

这一概念同时适用于类和函数，如下所示：

.. index::
   type parameter
   default
   generic type
   type argument
   default type
   instantiation
   type instantiation
   class
   function

.. code-block-meta:
    expect-cte:

.. code-block:: typescript
   :linenos:

    class SomeType {}
    interface Interface <T1 = SomeType> { }
    class Base <T2 = SomeType> { }
    class Derived1 extends Base implements Interface { }
    // Derived1 is semantically equivalent to Derived2
    class Derived2 extends Base<SomeType> implements Interface<SomeType> { }

    function foo<T = number>(input: T): T { return input}
    foo(1) // This call is semantically equivalent to next one
    foo<number>(1)

    class C1 <T1, T2 = number, T3> {}
    // That is a compile-time error, as T2 has default but T3 does not

    class C2 <T1, T2 = number, T3 = string> {}
    let c1 = new C2<number>          // Equal to C2<number, number, string>
    let c2 = new C2<number, string>  // Equal to C2<number, string, string>
    let c3 = new C2<number, Object, number> // All 3 type arguments provided

    function foo <T1 = T2, T2 = T1> () {}
    // Compile-time error,
    // as T1's default refers to T2, which is defined after T1
    // T2's default is valid as it refers to type parameter T1 already defined

|

.. _Type Parameter Variance:

类型参数变型
================================================================================

.. meta:
    frontend_status: Done

通常，同一个泛型类或接口的两个不同实例化会被视为互不相关的不同类型。|LANG| 支持类型参数变型，从而可根据实参类型之间的 :ref:`Subtyping` 关系，在这些实例化之间建立子类型关系。

.. index::
   type parameter
   variance
   class
   interface
   generic interface
   generic class
   subtyping
   argument type
   instantiation

在声明泛型类型的类型参数时，可以使用关键字 in 或 out（称为变型修饰符）来指定类型参数的变型
（见 :ref:`Invariance, Covariance and Contravariance`）。

带有关键字 ``out`` 的类型参数是 *协变* 的。协变类型参数只能出现在输出位置：

- 构造器可以把 ``out`` 类型参数用作参数；
- 方法可以把 ``out`` 类型参数用作返回类型；
- 类型为 ``out`` 类型参数的字段必须是 ``readonly``；
- 否则会产生 :index:`compile-time error`。

.. index::
   type parameter
   generic type
   in keyword
   out keyword
   variance modifier
   variance
   invariance
   covariance
   covariant
   readonly

带有关键字 ``in`` 的类型参数是 *逆变* 的。逆变类型参数只能出现在输入位置：

- 方法可以把 ``in`` 类型参数用作参数类型；
- 否则会产生 :index:`compile-time error`。

没有变型修饰符的类型参数默认是 *不变* 的，可以出现在任意位置；其含义见 :ref:`Invariance, Covariance and Contravariance`。

.. index::
   contravariance
   type parameter
   in keyword
   contravariant
   in-position
   invariant
   variance modifier

.. code-block:: typescript
   :linenos:

    class X<in T1, out T2, T3> {

       // T1 can be used in in-position only
       foo (p: T1) {}  // OK
       foo1(p: T1): T1 { return p } // error, T1 in out-position
       fldT1: T1 // error, T1 in invariant position

      constructor (x: T2) { this.fldT2 = x } // OK
      bar(x: T2) : T2 { return x }           // Compile-time error, x in in-position
      readonly fldT2: T2                     // OK
      bar1() : T2 { return this.fldT2 }      // OK

       // T3 can be used in any position (in-out, write-read)
       fldT3: T3
       method (p: T3): T3 { this.fldT3 = p; return p}  // OK
    }

在 :ref:`Function Types` 中会发生变型交错。

.. code-block:: typescript
   :linenos:

    class X<in T1, out T2> {
       foo (p: T1): T2 {...}                           // in - out
       foo (p: (p: T2)=> T1) {...}                     // out - in
       foo (p: (p: (p: T1)=>T2)=> T1) {...}            // in - out - in
       foo (p: (p: (p: (p: T2)=> T1)=>T2)=> T1) {...}  // out - in - out - in
       // and further more
    }


.. index::
   function type
   variance interleaving

如果函数或方法的类型参数显式指定了变型修饰符，则会产生 :index:`compile-time error`。

.. index::
   function
   method
   type parameter
   variance modifier
   variance

|

.. _Generic Instantiations:

泛型实例化
********************************************************************************

.. meta:
    frontend_status: Done

如前所述，泛型声明定义了一组对应的泛型或非泛型实体。实例化过程的目标是：

- 生成新的泛型实体或非泛型实体；
- 为每个类型参数提供一个类型实参，而该类型实参本身也可以是任意类型。

实例化结果可以是新的类、接口、联合、数组、方法或函数。

.. code-block:: typescript
   :linenos:

    class A <T> {}
    class B <U, V> extends A<U> { // A<U> is a new generic type here
        field: A<V>               // A<V> is a new generic type here
        method (p: A<Object>) {}  // A<Object> is a new non-generic type here
    }

.. index::
   generic class
   generic instantiation
   interface
   type alias
   method
   function
   instantiation
   generic entity
   non-generic entity
   type parameter
   type argument
   class
   union
   array
   interface

|

.. _Type Arguments:

类型实参
================================================================================

.. meta:
    frontend_status: Done

*类型实参* 是用于实例化的、非空的类型列表。

*类型实参* 的语法如下：

.. code-block:: abnf

    typeArguments:
        '<' type (',' type)* '>'
        ;

下面的示例展示了不同形式的类型实参实例化：

.. code-block:: typescript
   :linenos:

    Array<number>                     // Instantiated with type number
    Array<number|string>              // Instantiated with union type
    Array<number[]>                   // Instantiated with array type
    Array<[number, string, boolean]>  // Instantiated with tuple type
    Array<()=>void>                   // Instantiated with function type

.. index::
   type argument
   instantiation
   union type
   array type
   tuple type
   function type

|

.. _Explicit Generic Instantiations:

显式泛型实例化
================================================================================

.. meta:
    frontend_status: Done

显式泛型实例化是一种语言构造，它提供 :ref:`Type Arguments` 列表，用来替换泛型中对应的类型参数：

.. code-block:: typescript
   :linenos:

    class G<T> {}    // Generic class declaration
    let x: G<number> // Explicit class instantiation, type argument provided

    class A {
       method <T> () {}  // Generic method declaration
    }
    let a = new A()
    a.method<string> () // Explicit method instantiation, type argument provided

    function foo <T> () {} // Generic function declaration
    foo <string> () // Explicit function instantiation, type argument provided

    type MyArray<T> = T[] // Generic type declaration
    let array: MyArray<boolean> = [true, false] // Explicit array instantiation, type argument provided

    class X <T1, T2> {}
    // Different forms of explicit instantiations of class X producing new generic entities
    class Y<T> extends X<number, T> { // class Y extends X instantiated with number and T
       f1: X<Object, T> // X instantiated with Object and T
       f2: X<T, string> // X instantiated with T and string
       constructor() {
         this.f1 = new X<Object,T>
         this.f2 = new X<T,string>
       }
    }

.. index::
   instantiation
   generic
   generic instantiation
   type
   type argument
   type parameter
   array
   function
   method
   string

如果为非泛型类、接口、类型别名、方法或函数提供类型实参，则会产生 :index:`compile-time error`。

在显式泛型实例化 *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`> 中，*G* 是泛型声明，而 <``T``:sub:`1`, ``...``, ``T``:sub:`n`> 是其类型实参列表。

如果泛型声明的类型参数 *T*:sub:`1`, ``...``, *T*:sub:`n` 分别受对应约束 ``C``:sub:`1`, ``...``, ``C``:sub:`n` 限制，那么 *T*:sub:`i` 必须 :ref:`可赋值 <Assignability>` 给每个约束类型 *C*:sub:`i`。参数化声明中，对应约束类型的所有子类型都属于相应类型实参 *T*:sub:`i` 的取值范围。

.. index::
   type argument
   non-generic class
   non-generic interface
   non-generic type alias
   non-generic method
   non-generic function
   generic declaration
   class
   interface
   type alias
   method
   function
   generic
   instantiation
   assignability
   assignable type
   constraint
   subtype
   parameterized declaration

如果泛型实例化 *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`> 同时满足以下条件，则称其 *形式良好*：

- 泛型声明名为 *G*；
- 类型实参数量等于 *G* 的类型参数数量；
- 所有类型实参都 :ref:`可赋值 <Assignability>` 给对应的类型参数约束。

若实例化不是形式良好的，则会产生 :index:`compile-time error`。

除非在相关小节中特别说明，本规范中讨论的泛型版本均指类类型、接口类型或函数。

若满足以下任一条件，则任意两个泛型实例化都被视为 *可证明不同*：

- 二者分别参数化自不同的泛型声明；或
- 二者的任意一个类型实参可证明不同。

.. index::
   generic instantiation
   generic declaration
   type parameter
   type argument
   assignability
   constraint
   instantiation
   well-formed instantiation
   class type
   generic type
   interface type
   function
   type argument
   type parameter
   provably distinct instantiation
   parameterization
   distinct generic declaration
   distinct argument

|

.. _Implicit Generic Instantiations:

隐式泛型实例化
================================================================================

.. meta:
    frontend_status: Done

在 *隐式* 实例化中，类型实参不会显式给出，而是从引用泛型时所处的上下文中通过 :ref:`Type Inference` 推断得出。如果类型实参无法推断，则会产生 :index:`compile-time error`。

下面的示例展示了不同的类型实参推断情况：

.. code-block:: typescript
   :linenos:

    function foo <G> (x: G, y: G) {} // Generic function declaration
    foo (new Object, new Object)     // Implicit generic function instantiation
      // based on argument types: the type argument is inferred

    function process <P, R> (arg: P, cb?: (p: P) => R): P | R {
       // Return the data itself or if the processing function provided the
       // result of processing
       return cb != undefined ? cb (arg): arg
    }
    process (123, () => {}) // P is inferred as 'int', while R is 'void'

    function bar <T> (p: number) {}
    bar (1) // Compile-time error, type argument cannot be inferred

隐式实例化只适用于泛型函数和泛型方法。

如果泛型类或接口 *G* <``T``:sub:`1`, ``...``, ``T``:sub:`n`> 的某个方法拥有自己的类型参数 ``U``，且 ``U`` 的默认类型等于 ``T``:sub:`i`（见 :ref:`Type Parameter Default`），并且该方法的隐式泛型实例化没有提供足够信息来推断类型实参，那么就使用对应的 ``T``:sub:`i` 作为 ``U`` 的类型实参。

如下例所示：

.. code-block:: typescript
   :linenos:

    class A <T> {  // T is the class type parameter
        foo<U = T> (p: U) {} // U is own type parameter with default T
        bar<V = T> () {}     // V is own type parameter with default T
    }

    // Assume that X1 and X2 are two distinct types
    let a = new A<X1>

    // Implicit instantiation:
    a.foo(new X2) // Type argument is inferred from ``new X2``
    a.bar()       // Class type argument X1 is used as no other information is provided

    // explicit instantiation:
    a.foo<X2> (new X2) // Explicit type argument is used
    a.bar<X2> ()       // Explicit type argument is used

.. index::
   instantiation
   type argument
   type inference
   inferred type
   generic
   context
   generic method
   generic function
   method
   function

.. raw:: pdf

   PageBreak
