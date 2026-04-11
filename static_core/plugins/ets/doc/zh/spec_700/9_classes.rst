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


.. _Classes:

类
################################################################################

.. meta:
    frontend_status: Done

类声明用于引入新的引用类型，并描述该类型的实现方式。

类体中可以包含类成员声明（见 :ref:`Class Members`）或类构造函数
（见 :ref:`Constructor Declaration`）以及初始化块。

成员声明的声明体构成该声明的作用域（见 :ref:`Scopes`）。

类成员包括：

- 字段；
- 方法；以及
- 访问器。

字段、方法、访问器和构造函数都可以带访问修饰符（见 :ref:`Access Modifiers`）。

类成员既可以在当前类中声明，也可以从超类或超接口中继承而来。

每个成员都与声明它的那个类声明相关联。

每个类都定义两类类级作用域（见 :ref:`Scopes`）：一类用于实例成员，另一类用于静态成员。
这意味着一个类中两个成员可以同名，只要一个是静态成员而另一个不是。

.. index::
   class declaration
   class constructor
   reference type
   implementation
   class body
   field
   method
   accessor
   class member
   constructor
   initializer block
   scope
   declaration scope

.. index::
   declared class member
   inherited class member
   access modifier
   accessor
   method
   field
   implementing
   overriding
   superclass
   superinterface
   class-level scope
   instance member
   static member

|

.. _Class Declarations:

类声明
*******************************************************************************

.. meta:
    frontend_status: Done

每个类声明都会定义一个 *类类型*，也就是一个具名的引用类型。
类名通过类声明中的标识符给出。若类声明带有 ``typeParameters``，
则该类是泛型类（见 :ref:`Generics`）。

类声明的语法如下：

.. code-block:: abnf

       classDeclaration:
           classModifier? 'class' identifier typeParameters?
           classExtendsClause? implementsClause?
           classMembers
           ;

       classModifier:
           'abstract' | 'final'
           ;

带 ``final`` 修饰符的类属于实验性特性（见 :ref:`Final Classes`）。

.. 带 ``sealed`` 修饰符的类属于实验性特性（见 :ref:`Sealed Classes`）。

类声明的作用域由
:ref:`Scopes` 统一规定。

类声明至少需要明确：

- 类名；
- 可选的类型参数；
- 可选的 ``extends`` 子句；
- 可选的 ``implements`` 子句；
- 类体中的成员与初始化内容。

.. index::
   class declaration
   class type
   generic class
   identifier
   class name

.. index::
   class
   final modifier
   modifier
   final class
   class declaration
   class type
   reference type
   class name
   identifier
   generic class

类声明示例如下：

.. code-block:: typescript
   :linenos:

       class Point {
         public x: number
         public y: number
         public constructor(x : number, y : number) {
           this.x = x
           this.y = y
         }
         public distanceBetween(other: Point): number {
           return Math.sqrt(
             (this.x - other.x) * (this.x - other.x) +
             (this.y - other.y) * (this.y - other.y)
           )
         }
         static origin = new Point(0, 0)
       }

|

.. _Abstract Classes:

抽象类
===============================================================================

.. meta:
    frontend_status: Done

带有 ``abstract`` 修饰符的类称为抽象类。抽象类不能直接实例化。
抽象类允许声明抽象方法，而非抽象类不允许包含抽象方法。

抽象类的子类既可以继续保持抽象，也可以成为可实例化的具体类。
如果某个非抽象子类继承了抽象超类，那么它必须实现所有需要实现的抽象成员。

若非抽象类包含抽象方法，则发生 :index:`compile-time error`。
若抽象方法声明包含 ``private``、``static``、``final``、``native`` 或 ``async`` 修饰符，也会发生 :index:`compile-time error`。

若试图创建抽象类实例，则会产生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

      abstract class X {
         field: number
         constructor (p: number) { this.field = p }
      }
      let x = new X(42)
        // Compile-time error, Cannot create an instance of an abstract class.

非抽象的抽象类子类仍然可以被实例化；此时抽象超类的构造函数和其中非静态字段的初始化器都会执行：

.. index::
   abstract class
   abstract modifier
   subclass
   superclass
   instantiation
   non-abstract class
   field initializer
   constructor
   non-static field
   class

.. code-block:: typescript
   :linenos:

      abstract class Base {
         field: number
         constructor (p: number) { this.field = p }
      }

      class Derived extends Base {
         constructor (p: number) { super(p) }
      }

带有 ``abstract`` 修饰符的方法是 :ref:`Abstract Methods`。抽象方法没有方法体，只能声明，不能在声明处实现。

只有抽象类才能包含抽象方法。若非抽象类中声明了抽象方法，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

      class Y {
        abstract method (p: string)
        /* compile-time error, Abstract methods can only
           be within an abstract class. */
      }

如果抽象方法声明同时带有 ``final`` 或 ``override`` 修饰符，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

      abstract class Y {
        final abstract method (p: string)
        // Compile-time error, Abstract methods cannot be final
      }


.. index::
   abstract modifier
   modifier
   abstract method
   method body
   non-abstract class
   class
   method declaration
   implementation
   abstract class
   final modifier
   override modifier

|

.. _Class Extension Clause:

类扩展子句
*******************************************************************************

.. meta:
    frontend_status: Done

除 ``Object`` 类外，所有类都可以包含 ``extends`` 子句，用来指定当前类的*基类*，
也就是当前类的*直接超类*。此时，当前类就是*派生类*，也是该基类的*直接子类*。
除 ``Object`` 外，任何未显式写出 ``extends`` 子句的类，都视为带有
``extends Object`` 子句。

其语法如下：

.. code-block:: abnf

       classExtendsClause:
           'extends' typeReference
           ;

下列情况会导致 :index:`compile-time error`：

- ``typeReference`` 直接引用或别名化了非类类型，例如接口、枚举、联合、函数、
  utility type，或任意 ``FixedArray`` 实例化类型；
- ``typeReference`` 所命名的类类型不可访问（参见 :ref:`Accessible`）；
- 在 ``Object`` 类声明中出现 ``extends`` 子句；
- ``extends`` 图中存在环。

类扩展意味着类会继承其直接超类的所有成员。

.. note::
   ``private`` 成员会从超类继承下来，但在子类内部不可访问（见 :ref:`Accessible`）：

   .. code-block:: typescript
      :linenos:

             class Base {
               /* All methods are accessible in the class where
                   they were declared */
               public publicMethod () {
                 this.protectedMethod()
                 this.privateMethod()
               }
               protected protectedMethod () {
                 this.publicMethod()
                 this.privateMethod()
               }
               private privateMethod () {
                 this.publicMethod();
                 this.protectedMethod()
               }
             }
             class Derived extends Base {
               foo () {
                 this.publicMethod()    // OK
                 this.protectedMethod() // OK
                 this.privateMethod()   // Compile-time error,
                                        // the private method is inaccessible
               }
             }


直接子类关系的传递闭包构成子类关系。换言之，若满足以下任一条件，
则类 ``A`` 是类 ``C`` 的子类：

- 类 ``A`` 是 ``C`` 的直接子类；
- 类 ``A`` 是某个类 ``B`` 的子类，而 ``B`` 又是 ``C`` 的子类
  （即该定义递归适用）。

相应地，只要 ``A`` 是 ``C`` 的子类，则 ``C`` 就是 ``A`` 的超类。

.. index::
   class
   Object
   extends clause
   base class
   derived class
   direct subclass
   direct superclass
   superclass

.. index::
   syntax
   class
   class extension clause
   non-class type
   interface
   enumeration
   union
   function
   utility type
   accessibility

.. index::
   extends clause
   base class
   derived class
   direct superclass
   subclass

|

.. _Class Implementation Clause:

类实现子句
*******************************************************************************

.. meta:
    frontend_status: Done

类可以通过 ``implements`` 子句实现一个或多个接口。
实现的接口必须是可访问的接口类型。若同一接口在直接超接口列表中重复出现，
则属于错误情形。

其语法如下：

.. code-block:: abnf

       implementsClause:
           'implements' interfaceTypeList
           ;

       interfaceTypeList:
           typeReference (',' typeReference)*
           ;

若 ``typeReference`` 不能命名可访问的接口类型（见 :ref:`Accessible`），则发生
:index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       // File1
       interface I { } // Not exported

       // File2
       class C implements I {} // Compile-time error I is not accessible

对于泛型类与泛型接口，接口实现关系需要同时考虑类型参数、替换后的接口实例化、
以及接口属性与方法的完整实现。

.. index::
   class declaration
   class implementation clause
   implements clause
   accessible interface type
   accessibility
   interface type
   type argument
   interface
   syntax
   direct superinterface

对类声明 ``C<F1, ..., Fn>``（其中 ``n >= 0``，且 ``C != Object``）而言，
类类型 ``C<F1, ..., Fn>`` 的*直接超接口*，就是 ``C`` 声明中 ``implements`` 子句列出的类型。

对于泛型类声明 ``C<F1, ..., Fn>``（其中 ``n > 0``），参数化类类型 ``C<T1, ..., Tn>`` 的
直接超接口，是把替换 ``[F1 := T1, ..., Fn := Tn]`` 应用于原始 ``implements`` 子句后得到的接口实例化类型。

若接口类型 ``I`` 满足以下任一条件，则 ``I`` 是类类型 ``C`` 的超接口：

- ``I`` 是 ``C`` 的直接超接口；
- ``I`` 是 ``J`` 的超接口，而 ``J`` 是 ``C`` 的直接超接口
  （见 :ref:`Superinterfaces and Subinterfaces`，该章节定义了接口的超接口）；
- ``I`` 是 ``C`` 的直接超类的某个超接口。

类会实现其所有超接口。

可选接口属性既可以显式实现，也可以依赖语言规则进行隐式实现；
但必需接口属性必须明确满足。

若类不是抽象类，则必须满足以下条件：

- 超接口中的方法必须拥有可用的实现体；这些实现可以定义在类本身、其超类或其超接口中
  （详见 :ref:`Overriding in Classes`）；
- 超接口中的必需属性必须被实现（详见 :ref:`Implementing Required Interface Properties`）。

否则，会产生 :index:`compile-time error`。

若类实现了同一泛型接口（见 :ref:`Generics`）的两个不同实例化版本，或类字段
与其某个超接口继承而来的方法同名（除非一方为静态成员、另一方为实例成员），
也会产生 :index:`compile-time error`。

超接口中的可选属性可以显式实现，也可以依赖隐式定义
（见 :ref:`Implementing Optional Interface Properties`）。

.. index::
   class declaration
   class implementation clause
   implements clause
   interface type
   type argument
   interface
   direct superinterface
   syntax

.. index::
   class declaration
   direct superinterface
   implements clause
   substitution
   generic class declaration

.. index::
   class type
   direct superinterface
   superinterface
   interface
   superclass
   class
   interface type
   instantiation

|

.. _Implementing Required Interface Properties:

实现必需接口属性
===============================================================================

.. meta:
    frontend_status: Partly

类必须实现其所有超接口中的必需属性（见 :ref:`Interface Properties`）。
实现方式可以是字段、getter/setter，或访问器组合，但必须满足以下核心要求：

- 属性名匹配；
- 类型兼容；
- 只读/可写语义匹配；
- 访问性满足接口契约。

下表总结了类中实现接口属性的所有合法组合；其他任意组合都会导致 :index:`compile-time error`：

   =========================== ======================================================
   接口属性形式                类中的实现形式
   =========================== ======================================================
   字段                        字段，或 getter 与 setter
   只读字段                    只读字段、字段、getter，或 getter 与 setter
   仅 getter                   只读字段、字段、getter，或 getter 与 setter
   仅 setter                   字段、setter，或 setter 与 getter
   getter 与 setter            字段，或 getter 与 setter
   =========================== ======================================================

如果类字段用于实现接口属性，那么字段类型、只读属性和覆盖行为都必须一致。
如果接口属性以访问器形式定义，那么类中的访问器实现也必须满足相应约束。

当类字段用于实现任意形式的接口属性时：

- 该字段本身仍表示实例数据成员（见 :ref:`Field Declarations`）；
- 通过类实例访问该字段时，始终表示访问普通类字段；
- 通过接口类型引用访问同名属性时，始终表示调用 getter 或 setter；
- 若接口属性本身以字段形式声明，则编译器会在类中隐式生成 getter，
  并在属性非 ``readonly`` 时隐式生成 setter。

下面的示例展示了字段实现接口属性时，类实例访问与接口引用访问之间的差异：

.. code-block:: typescript
   :linenos:

       interface I {
           n: number
       }
       class C implements I {
           n: number = 1 // property is implemented as a field
       }
       let c = new C()
       console.log(c.n) // class field read
       c.n = 1 // class field write

       let i: I = c
       console.log(c.n) // implicitly generated getter is called
       i.n = 2          // implicitly generated setter is called

隐式生成的 getter 和 setter 在伪代码上等价于：

.. code-block:: typescript

       get n(): number  { return this.n }
       set n(x: number) { this.n = x }

.. note::

   - 上述伪代码中的 ``this.n`` 表示直接访问类字段本身，而不是再次调用 getter 或 setter；
   - 若显式声明与字段同名的 getter 或 setter，则会产生 :index:`compile-time error`，
     因为类成员必须满足 :ref:`Declarations` 中定义的可区分规则。

如果接口属性通过访问器实现，则其行为如下例所示：

.. code-block:: typescript
   :linenos:

       interface I {
           n: number
           readonly r: string
       }
       class C implements I {
           // 'n' is explicitly implemented as getter and setter
           get n(): number  { return 1 }
           set n(x: number) { /*some body*/ }
           // 'r' is explicitly implemented as getter
           get r(): string { return "abc" }
           // a setter can be defined for 'r', but it is not mandatory
           set r(x: string) { /*some body*/ }
       }

在这类实现中，若出现以下情形，则发生 :index:`compile-time error`：

- 接口属性为 ``readonly``，但未定义 getter；
- 接口属性不是 ``readonly``，但 getter 或 setter 缺少其一。

此外，若 getter 返回类型或 setter 参数类型与接口契约不一致，也会发生
:index:`compile-time error`。

下面的示例表示这些错误情形：

.. code-block:: typescript
   :linenos:

       interface I {
           n: number
       }
       class A implements I { // Compile-time error, setter is not defined
           get n(): number  { return 1 }
       }
       class B implements I { // Compile-time error, getter is not defined
           set n(x: number)  {}
       }
       class C implements I {
           get n(): string { return "aa" } // Compile-time error, wrong type
       }

       interface J {
           readonly r: string
       }
       class D implements J { // Compile-time error, getter is not defined
           set r(x: string) {}
       }

如果接口属性本身以访问器形式声明，而类选择用字段来实现，那么编译器只会为接口中
实际声明过的访问器隐式生成方法体。也就是说，接口若只声明 getter，则类字段实现后
也只会得到 getter，不会自动得到 setter：

.. code-block:: typescript
   :linenos:

       interface I {
         get n(): number
       }
       class C implements I {
           n: number = 1 // property is implemented as a field
           // getter body is implicitly generated
           // setter is not defined and not generated
       }

       function foo(i: I) {
         console.log(i.n) // OK, getter is used
         i.n = 1 // Compile-time error, setter is not defined
       }

       interface J {
         get n(): number
         set n(x: number)
       }
       class D implements J {
           n: number = 1 // property is implemented as a field
           // getter body is implicitly generated
           // setter body is implicitly generated
       }

       function bar(j: J) {
         console.log(j.n) // OK, getter is used
         j.n = 1          // OK, setter is used
       }

若以下所有条件同时满足，则发生 :index:`compile-time error`：

- 接口同时声明同名的 getter 和 setter；
- 类用字段来实现该属性；
- getter 的返回类型与 setter 的参数类型不同。

示例如下：

.. code-block:: typescript
   :linenos:

       interface J {
         get n(): number
         set n(x: string)
       }
       class D implements J {
           n: number = 1 // Compile-time error, types mismatch
       }

这类属性只能通过访问器来显式实现：

.. code-block:: typescript
   :linenos:

       interface J {
         get n(): number
         set n(x: string)
       }

       class E implements J {
         value: string = ""
         get n(): number  { return Number.parseFloat(this.value) }
         set n(x: string) { this.value = x }
       }

       let e = new E
       e.n = "123"
       console.log(e.n)

如果超类已经以某种形式实现了接口属性，则所有派生类都会以相同形式继承该实现。
因此，下列行为都会触发 :index:`compile-time error`：

- 用访问器覆盖超类字段；
- 用字段覆盖超类访问器。

.. code-block:: typescript
   :linenos:

       interface I {
           n: number
       }
       class C implements I {
           n: number = 1
       }
       class C1 extends C {
           get n(): number { return 1 } // Compile-time error, 'n' cannot be overridden by an accessor
       }

       class D implements I {
           get n(): number { return 1 }
           set n(x: number) {}
       }
       class D1 extends D {
           n: number = 2 // Compile-time error, 'n' cannot be overridden by a field
       }

字段实现接口属性时，可以覆盖超类中同类型的字段：

.. code-block:: typescript
   :linenos:

       interface I {
           n: number
       }
       class A {
           n: number = 1
       }
       class C extends A implements I {
           n: number = 2 // OK, 'n' overrides 'n' from A and implements 'n' from I
       }

但若接口之间、或接口与超类字段之间的类型不一致，则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       interface I {
           n: number
       }
       interface J {
           n: string
       }
       class A implements I, J { // Compile-time error, types mismatch
       }

       class B {
           n: string = "aa"
       }
       class C extends B implements I { // Compile-time error, types mismatch
       }

       class C implements I {
           get n(): string { return "aa" } // Compile-time error, types mismatch
       }

如果属性在接口中定义为 ``readonly``，那么实现它的字段既可以是 ``readonly``，
也可以是可写字段：

.. code-block:: typescript
   :linenos:

       interface I {
           readonly r: number
       }
       class A implements I {
           r: number = 0 // OK, the field is writeable
       }
       class B implements I {
           readonly r: number = 1  // OK, the field is readonly
       }

       function foo(i: I) {
           i.r = 42 // Compile-time error, 'r' is readonly
           if (i instanceof A) {
               i.r = 42 // OK, here 'i' is of type A, and 'r' is writeable
           }
           if (i instanceof B) {
            i.r = 42 // Compile-time error, here 'i' is of type B, and 'r' is readonly
           }
       }

若可写接口属性被实现为 ``readonly``，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       interface I {
           r: number
       }
       class C implements I {
           readonly r: number = 1 // Compile-time error
       }

       interface J {
           readonly r: number
       }
       class D implements I, J {
           readonly r: number = 1 // Compile-time error
       }
       class E implements I, J {
           r: number = 1 // OK
       }

若类根本没有提供接口属性的实现，也会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       interface I {
           property: number
       }
       class C implements I { // Compile-time error, no implementation at all
       }


.. index::
   accessor
   getter
   setter
   readonly field
   required interface property
   field implementation
   accessor implementation
   type mismatch
   compile-time error

|

.. _Implementing Optional Interface Properties:

实现可选接口属性
===============================================================================

.. meta:
    frontend_status: None

类可以实现超接口中的可选接口属性（见 :ref:`Optional Interface Properties`），
也可以使用接口中隐式定义的访问器。

可选属性实现为字段时，编译器可能引入隐藏字段与访问器辅助逻辑。
如果以访问器实现，则 getter/setter 规则同样适用。

下面的示例展示了直接使用接口中隐式定义访问器的情况：

.. code-block:: typescript
   :linenos:

       interface I {
         n?: number
       }
       class C implements I {}

       let c = new C()
       console.log(c.n) // Output: undefined
       c.n = 1 // runtime error is thrown

.. index::
   property
   interface
   implementation
   class
   superinterface
   accessor

若可选接口属性由字段实现，例如：

.. code-block:: typescript
   :linenos:

       interface I {
         num?: number
       }
       class C implements I {
         num?: number = 42
       }

则编译器会为类 ``C`` 隐式定义私有隐藏字段及所需访问器，用于覆盖接口中的访问器：

.. code-block:: typescript
   :linenos:

       class C implements I {
         private $$_num: number = 42 // the exact name of the field is implementation specific
         get num(): number | undefined { return this.$$_num }
         set num(n: number | undefined) { this.$$_num = n }
       }


若属性通过访问器实现（见 :ref:`Class Accessor Declarations`），
则可选字段允许只显式提供一个访问器，而把另一个交给默认实现：

.. index::
   interface
   implementation
   property
   field
   private field
   hidden field
   accessor
   class
   class accessor declaration

.. code-block:: typescript
   :linenos:

       interface I {
         num?: number
       }

       class C1 implements I { // OK, both default implementations
       }

       class C2 implements I { // OK, default implementation used for get
         set num(n: number | undefined) { this.$$_num = n }
       }

       class C3 implements I { // OK, both explicit implementations
         get num(): number | undefined { return this.$$_num }
         set num(n: number | undefined) { this.$$_num = n }
       }

若把接口中的可选属性实现为非可选字段，则会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       interface I {
         num?: number
       }
       class C implements I {
         num: number = 42 // Compile-time error, must be optional
       }

.. index::
   interface
   implementation
   property
   non-optional field
   optional field

|

.. _Class Members:

类成员
*******************************************************************************

.. meta:
    frontend_status: Done

类中可以声明字段、方法、访问器、构造函数、显式方法重载
（见 :ref:`Explicit Class Method Overload`）和单个静态初始化块
（见 :ref:`Static Initialization`）等内容。
其中构造函数与静态块不被视为“成员”，也不会被继承；字段、方法和访问器才参与
成员继承、覆盖与可访问性规则。

成员既可以是静态的，也可以是实例级的。类声明中所有可访问的静态与实例实体名称
都参与相应的作用域规则。注解的使用由 :ref:`Using Annotations` 统一规定。

类成员的语法如下：

.. index::
   class member
   declaration
   field
   method
   accessor
   constructor
   method overload
   class method
   static block
   initialization
   syntax

.. code-block:: abnf

       classMembers:
           '{'
              classMember* staticBlock? classMember*
           '}'
           ;

       classMember:
           annotationUsage?
           accessModifier?
           ( constructorDeclaration
           | explicitConstructorOverload
           | classFieldDeclaration
           | classMethodDeclaration
           | explicitClassMethodOverload
           | classAccessorDeclaration
           )
           ;

       staticBlock: 'static' Block;

类中的声明既可能是直接声明，也可能是继承而来。任何类体内声明都处于类作用域中，
其完整规则见 :ref:`Scopes`。

成员分为两类：

- 静态成员，不属于类实例，可在类名可访问的位置（见 :ref:`Accessible`）
  通过限定名访问（见 :ref:`Names`）；
- 非静态成员，也就是实例成员，属于类的每个实例。

.. note::
   超类的静态成员不会被继承。子类若要使用超类静态成员，必须显式用超类名限定。
   这条规则适用于静态字段、静态方法和静态访问器。

类声明作用域（见 :ref:`Scopes`）中所有可访问的静态和实例实体名称都必须唯一。
也就是说，静态性相同的字段、方法和重载不能重名。

.. index::
   annotation
   static block
   class body
   class
   static member
   non-static member
   static entity
   non-static entity
   declaration
   member
   class instance
   access
   accessibility
   qualified name
   inheritance
   declaration scope
   overload
   scope

类成员来源如下：

- 直接超类继承而来的成员（见 :ref:`Inheritance`）；
- 直接超接口（见 :ref:`Superinterfaces and Subinterfaces`）声明的成员；
- 当前类体中直接声明的成员（见 :ref:`Class Members`）。

``private`` 成员对所有子类都不可访问（见 :ref:`Accessible`）；
``protected`` 或 ``public`` 成员会被所有子类继承，
并在相应规则下保持可访问（见 :ref:`Accessible`）。

构造函数和静态块不是成员，因此不会被继承。

类成员类型包括：

- 字段（见 :ref:`Field Declarations`）；
- 方法（见 :ref:`Method Declarations`）；
- 访问器（见 :ref:`Class Accessor Declarations`）。

.. index::
   inheritance
   class member
   inherited member
   direct superclass
   private
   subclass
   accessibility

.. index::
   class
   protected
   public
   subclass
   access
   constructor
   initializer block
   inheritance

.. index::
   class field
   method
   accessor
   type parameter
   return type
   declaration
   method member
   static member

|

.. _Access Modifiers:

访问修饰符
*******************************************************************************

.. meta:
    frontend_status: Done

访问修饰符决定类成员和构造函数如何被访问。
|LANG| 中的可访问性主要分为 ``private``、``protected`` 和 ``public``。
如果未显式指定修饰符，则采用语言定义的默认可访问性。

访问修饰符同样会影响继承、实现、覆盖与外部访问。

其语法如下：

.. code-block:: abnf

       accessModifier:
           'private'
           | 'protected'
           | 'public'
           ;

若未显式给出修饰符，则类成员或构造函数默认视为 ``public``。

.. index::
   access modifier
   class member
   access
   member
   constructor
   private
   public
   protected

.. _Private Access Modifier:

``private`` 访问修饰符
===============================================================================

.. meta:
    frontend_status: Done

``private`` 表示该成员或构造函数只能在当前类内部访问（见 :ref:`Accessible`），
即声明在类 ``C`` 中的 private 成员或构造函数 *m* 只能在 ``C`` 的类体内访问。
使用示例如下：

.. code-block:: typescript
   :linenos:

       class C {
         private count: number = 1
         private hello() {}
         getCount(): number {
           return this.count // OK
         }
         sayHello() {
           this.hello()  // OK
         }
       }

       function increment(c: C) {
         c.sayHello()  // OK
         c.hello()     // Compile-time error - 'hello()' is private
         c.count++     // Compile-time error - 'count' is private
       }

       class D1 extends C {
         foo() {
            this.getCount() // OK
            this.sayHello() // OK
            this.hello()    // Compile-time error, private base method
            this.count++    // Compile-time error, private base field
         }
       }

       class D2 extends C {
         count = 1  // OK to reuse name since `count` in base inaccessible
         hello() {} // OK to reuse name since `hello` in base inaccessible
         foo() {
            this.count++    // OK
            this.hello()    // OK
         }
       }

       function bar(p: D2) {
         p.count++ // OK
         p.hello() // OK
      }


.. index::
   access modifier
   private
   private member
   class member
   constructor
   access
   accessibility
   declaring class
   class body
   method
   derived class

.. _Protected Access Modifier:

``protected`` 访问修饰符
===============================================================================

.. meta:
    frontend_status: Done

``protected`` 表示该成员或构造函数可以在当前类及其派生类中访问（见 :ref:`Accessible`）。
声明在类 ``C`` 中的 protected 成员 ``M`` 只能在 ``C`` 的类体或从 ``C`` 派生的类中访问：

.. code-block:: typescript
   :linenos:

       class C {
         protected count: number
          getCount(): number {
            return this.count // OK
          }
       }

       class D extends C {
         increment() {
           this.count++ // OK, D is derived from C
         }
       }

       function increment(c: C) {
         c.count++ // Compile-time error - 'count' is not accessible
       }


.. index::
   protected modifier
   access modifier
   accessible constructor
   method
   protected
   constructor
   accessibility
   class
   class body
   class member
   derived class
   declaring class

.. _Public Access Modifier:

``public`` 访问修饰符
===============================================================================

.. meta:
    frontend_status: Done

``public`` 表示该成员或构造函数可以在所有允许该类被访问（见 :ref:`Accessible`）
的位置使用。换句话说，只要该成员或构造函数所属的类型本身可访问，``public`` 成员就可在任何地方访问。
接口成员默认表现为公开可访问的契约。

.. index::
   access modifier
   public modifier
   public
   class member
   constructor
   protected
   access
   accessibility
   accessible type

|

.. _Field Declarations:

字段声明
*******************************************************************************

.. meta:
    frontend_status: Partly

字段声明在语法上类似变量声明，但其生命周期、初始化时机和继承语义由类机制控制。
字段可以是静态或实例的（见 :ref:`Static and Instance Fields`），可以是普通字段、
只读字段、可选字段（见 :ref:`Optional Fields`），也可以带延迟初始化语义
（见 :ref:`Fields with Late Initialization`）。

其语法如下：

.. code-block:: abnf

       classFieldDeclaration:
           fieldModifier*
           identifier
           ( '?'? ':' type initializer?
           | '?'? initializer
           | '!' ':' type
           )
           ;

       fieldModifier:
           'static' | 'readonly' | 'override'
           ;

标识符带 ``?`` 的字段称为 *可选字段*；标识符带 ``!`` 的字段称为
*延迟初始化字段*。

下列情况会导致 :index:`compile-time error`：

- 同一字段修饰符在字段声明中重复出现；
- 某字段名与同一类体中静态性相同的方法同名；
- 某字段名与同一类体中另一静态性相同的字段同名。

如果字段声明同时实现了从超接口继承（见 :ref:`Interface Inheritance`）的一个或多个
属性，那么字段类型与所有属性类型必须一致。否则发生 :index:`compile-time error`。

静态字段只能通过类名限定来访问，实例字段则通过对象引用访问（见
:ref:`Field Access Expression`）。

.. code-block:: typescript
   :linenos:

       // Two unrelated interfaces
       interface B1 {}
       interface B2 {}
       // Interface which extends both of them
       interface B3 extends B1, B2 {}
       // Class which implements B3
       class BB3 implements B3 {}

       interface II1 { f: B1 }
       interface II2 { f: B2 } // Different property 'f' type as in II1
       interface II3 { f: B1 } // The same property 'f' type as in II1

       class CC1 implements II1, II2 {
           f: B1  = new BB3 /* compile-time error, field and all inherited properties
                               must be of the same type */
       }
       class CC2 implements II1, II3 {
           f: B3 = new BB3 /* compile-time error, field and all inherited properties
                              must be of the same type */
       }
       class CC3 implements II1, II3 {
           f: B1 = new BB3 // OK, correct properties implementation
       }



.. index::
   field declaration
   data member
   class instance
   instance field
   own field
   inheritance
   syntax
   variable declaration

.. index::
   field
   identifier
   optional field
   field with late initialization
   field modifier
   field declaration
   method
   class
   static field
   non-static field

.. index::
   static field
   qualified name
   access
   superinterface
   superclass
   field
   property
   class body
   field declaration
   inheritance

.. _Static and Instance Fields:

静态字段与实例字段
===============================================================================

.. meta:
    frontend_status: Done

类字段分为以下两类：

- 静态字段

  静态字段使用 ``static`` 修饰符声明，不属于任何类实例。无论最终创建了多少个该类实例
  （即使一个都没有），静态字段都只有一份副本。

  只要类名可访问（见 :ref:`Accessible`），静态字段就总是通过限定名形式访问。

- 实例字段（即非静态字段）

  实例字段属于类的每个实例。创建类实例时，相应实例字段会随该实例一同创建并与之关联；
  继承体系中的超类实例字段也按同样方式成为该对象状态的一部分。实例字段通过实例名访问。

若外围声明的某个类型形参名称被用作静态字段的类型，则发生
:index:`compile-time error`。

.. index::
   class fields
   modifier static
   static
   static field
   instantiation
   instance
   initialization
   class
   class instance
   superclass
   non-static field
   accessibility
   access
   instance field
   qualified name
   notation
   instance name

.. _Readonly Constant Fields:

只读字段
===============================================================================

.. meta:
    frontend_status: Done

带有 ``readonly`` 修饰符的字段是只读字段。
这类字段在初始化后不能再被任意修改，只能在语言允许的位置完成赋值。

.. index::
   readonly field
   readonly modifier
   readonly
   constant field
   initialization
   modifier
   static field
   non-static field

.. _Optional Fields:

可选字段
===============================================================================

.. meta:
    frontend_status: Partly

可选字段 ``f?: T = expr`` 实际上等价于类型为 ``T | undefined`` 的字段。
若字段声明中省略初始化器，则字段的初始值为默认值 ``undefined``
（见 :ref:`Default Values for Types`）。

.. index::
   undefined
   initializer
   field declaration
   value
   default value
   optional field

例如，下面两个字段的定义方式实际上完全相同：

.. code-block:: typescript
   :linenos:

       class C {
           f?: string
           g: string | undefined = undefined
       }

.. _Field Initialization:

字段初始化
===============================================================================

.. meta:
    frontend_status: Done

除 :ref:`Fields with Late Initialization` 之外，所有字段都会使用默认值
（见 :ref:`Default Values for Types`）或字段初始化器（若存在，见下文）完成初始化。
否则，字段必须以下列方式之一完成初始化：

若字段既没有可接受的默认值，也没有初始化器，则必须：

- 对静态字段，在 :ref:`Static Initialization` 中完成初始化；
- 对实例字段，在 :ref:`Constructor Declaration` 中完成初始化。

*字段初始化器* 是一个可在编译期或运行期求值的表达式。若求值成功，其结果会被赋给
对应字段。因此，字段初始化器的语义与赋值（见 :ref:`Assignment`）类似：
每个初始化器表达式的求值，以及随后的赋值，都只执行一次。

非静态字段声明中的初始化器会在运行期求值。每次创建类实例
（见 :ref:`New Expressions`）时，都会执行该赋值。

若初始化器中以任何形式使用 ``this`` 或 ``super``，则会产生 :index:`compile-time warning`，
因为这可能在对象尚未完全初始化时访问未就绪状态。

.. code-block:: typescript
   :linenos:

       class C {
           f0 = this // Compile-time warning as 'this' is used

           f1 = this.init_f1() // Compile-time warning as method of 'this' method is invoked

           init_f1 (): string {
              console.log (this.f1) // this.f1 was not yet initialized, a runtime error occurs
              return "a string field"
           }
       }

       class B {}
       function foo (f: () => B) { return f() }
       class A {
           field1 = foo(() => this.field2) 
               // Compile-time warning as 'this' is used in the initializer code
               // At runtime the lambda call will lead to a runtime error as
               // this.field2 was not yet initialized

           field2 = new B
       }


.. index::
   field initialization
   field initializer
   instance field
   static field
   default value
   compile-time warning
   this
   super

.. index::
   field initializer
   evaluation
   expression
   compile time
   compile time
   runtime
   access
   field
   semantics
   assignment
   this keyword
   super keyword
   method

.. index::
   non-static field declaration
   initializer
   initializer expression
   uninitialized field
   evaluation
   runtime
   assignment
   instance
   class
   instance field initializer
   call method
   this
   super
   restriction
   class instance

.. index::
   compiler
   field initializer
   this method
   access
   non-static field
   initialization
   circular dependency
   initializer expression
   this method

.. _Fields with Late Initialization:

延迟初始化字段
===============================================================================

.. meta:
    frontend_status: Done

延迟初始化字段 ``f!: T`` 表示该字段的初始化会在某个位置完成，而且可能不在类声明内部完成。
在类内部，这样的字段表现为类型 ``T`` 的字段。

如果延迟初始化字段满足以下任一情况，则发生 :index:`compile-time error`：

- 是 *静态字段*；
- 是 nullish 类型（见 :ref:`Nullish Types`）；
- 是 ``readonly`` 字段（见 :ref:`Readonly Constant Fields`）；
- 是 ``optional`` 字段（见 :ref:`Optional Fields`）；
- 带有 *字段初始化器*。

与其他字段一样，延迟初始化字段必须在首次读取前完成初始化。

.. index::
   field with late initialization
   field initializer
   instance field
   initialization
   nullish type
   field

延迟初始化字段必须被显式初始化，即使其类型有默认值，该默认值也永远不会被使用。

每次读取延迟初始化字段时，都会执行初始化检查。若编译器能确认字段未初始化，则发生
:index:`compile-time error`。否则在运行时检查，若字段未被初始化则触发
:index:`runtime error`：

.. code-block:: typescript
   :linenos:

       class C {
           f!: string
       }

       let x = new C()
       x.f = "aa"
       console.log(x.f) // OK

       let y = new C()
       console.log(y.f) // runtime or compile-time error

.. note::
   读取检查会带来额外开销，因此延迟初始化字段不应滥用。

.. index::
   field with late initialization
   initialization check
   runtime error
   compile-time error
   definite assignment assertion

.. index::
   field with late initialization
   field initializer
   optional field
   initialization
   default value
   check
   runtime
   field value

.. _Override Fields:

字段覆盖
===============================================================================

.. meta:
    frontend_status: None

当类扩展另一个类时，超类中的实例字段可以被子类中同名同类型字段覆盖。
覆盖字段必须满足类型、只读性、初始化和访问器兼容规则。
字段也不能随意覆盖 getter/setter 等不同实体种类。

.. note::
   通过字段实现接口属性的详情，见 :ref:`Implementing Required Interface Properties`
   和 :ref:`Implementing Optional Interface Properties`。

若覆盖字段（带 ``override`` 修饰符的字段）不覆盖超类中的字段、字段同时带
``static`` 和 ``override`` 修饰符、覆盖字段类型与被覆盖字段类型不同，或二者
的 :ref:`Access Modifiers` 不同，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class C {
           field: number = 1
           protected ff = 25
       }
       class D extends C {
           field: string = "aa"     // Compile-time error, type is not the same
           override no_field = 1224 // Compile-time error, no overridden field in the base class
           static override field: string = "aa" // Compile-time error, static cannot override
           ff = 66                  // Compile-time error, as access modifers are different 
       }

被覆盖字段必须通过初始化器或构造函数显式初始化；否则发生 :index:`compile-time error`。
即使字段类型有默认值（见 :ref:`Default Values for Types`），隐式初始化也不适用：

.. code-block:: typescript
   :linenos:

       class C {
           f1: number = 1
           f2: Object = 2
       }
       class D extends C {
           f1: number = 7 // OK
           f2: Object     // OK, initialized in the constructor
           constructor () {
               super()
               this.f2 = "abc"
           }
       }
       class E extends C {
           f1: number  // Compile-time error, must be initialized explicitly
           f2: Object  // Compile-time error, must be initialized explicitly
       }

被覆盖字段的初始化器会被保留执行，通常在超类构造函数上下文中完成：

.. code-block:: typescript
   :linenos:

       class C {
           field: number = C.init()
           private static init() {
              console.log ("Field initialization in C")
              return 123
           }
       }
       class D1 extends C {
           override field: number = 321 // field can be explicitly marked as overridden
       }

       class D2 extends D1 {
           field: number = D2.init_in_derived()
           private static init_in_derived() {
              console.log ("Field initialization in Derived")
              return 42
           }
       }
       console.log ((new D2()).field)
       /* Output:
           Field initialization in C
           Field initialization in Derived
           42
       */

对于泛型类，"同类型"意味着派生类字段类型必须与基类实例化后的字段类型相同：

.. code-block:: typescript
   :linenos:

       class B<T> {
          f1: T
          f2: T
          constructor (v: T) { this.f1 = v; this.f2 = v }
       }
       class D<U, V> extends B<U>  {
          f1: U // Valid overriding as D extends B<U>, and type of f1 in B<U> is U
          f2: V // Compile-time error, wrong overriding
          constructor (v: U) {
              super (v)
          }
       }


若超类字段未声明为 ``readonly``，而覆盖字段标记了 ``readonly``，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class C {
           field = 1
       }
       class D extends C {
           readonly field = 2 // Compile-time error, wrong overriding
       }

若字段覆盖了超类中的 getter、setter 或两者，则发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class C {
           get num(): number { return 42 }
           set num(x: number) {}
       }
       class D extends C {
           num: number = 2 // Compile-time error, wrong overriding
       }

用单个访问器或同时用 getter/setter 覆盖字段，也会发生 :index:`compile-time error`：

.. code-block:: typescript
   :linenos:

       class C {
           num: number = 1
       }
       class D extends C {
           get num(): number  { return this.shadow } // Compile-time error, wrong overriding
           set num(x: number) { this.shadow = p }    // Compile-time error, wrong overriding
           private shadow: number = 123
       }

.. index::
   overriding field
   class
   interface
   field
   declaration
   superclass
   superinterface
   overriding

.. index::
   overriding
   field overriding
   overridden field
   base class
   initialization
   initializer
   instance field

.. index::
   field
   override
   overriding field
   superclass
   implementation
   superinterface
   inheritance
   accessor

|

.. _Method Declarations:

方法声明
*******************************************************************************

.. meta:
    frontend_status: Done

方法由名称、签名、可选修饰符以及方法体共同定义。
类方法既可以是静态方法（见 :ref:`Static Methods`），也可以是实例方法
（见 :ref:`Instance Methods`）；还可以是抽象方法（见 :ref:`Abstract Methods`）、
异步方法（见 :ref:`Async Methods`）、原生方法（见 :ref:`Native Methods`）
或参与覆盖的方法（见 :ref:`Overriding Methods`）。

一个方法由以下部分共同定义：

1. 类型参数；
2. 形参类型列表；
3. 返回类型。

其语法如下：

.. code-block:: abnf

       classMethodDeclaration:
           methodModifier* identifier typeParameters? signature block?
           ;

       methodModifier:
           'abstract'
           | 'static'
           | 'final'
           | 'override'
           | 'native'
           | 'async'
           ;

下列情况会导致 :index:`compile-time error`：

- 方法修饰符重复出现；
- 类体中某方法的名字已被同一类体中的字段占用。

非静态方法在类中可以承担三种角色：

- 实现来自某个超接口的同名方法；
- 覆盖来自超类的某个实例方法；
- 声明一个全新的实例方法。

静态类方法不会实现或覆盖超类或超接口中的实例成员。

.. index::
   class method
   method declaration
   abstract method
   static method
   native method
   async method
   override modifier
   final modifier
   method signature
   method body

.. index::
   method declaration
   class method declaration
   method name
   method
   declaration
   executable code
   overriding
   inheritance
   superclass

.. _Static Methods:

静态方法
===============================================================================

.. meta:
    frontend_status: Done

静态方法使用 ``static`` 修饰符声明，属于类本身而非对象实例。
它们总是通过类类型调用，并且不会像实例方法那样参与动态分派。

若出现以下情况，则发生 :index:`compile-time error`：

- ``static`` 与 ``abstract``、``final`` 或 ``override`` 同时出现；
- 方法头或方法体中引用了外围声明的类型形参；
- 在静态方法内部使用 ``this`` 或 ``super``。

在静态方法内部使用 ``this`` 或 ``super`` 会导致 :index:`compile-time error`。

静态方法不会从超类继承：

.. code-block:: typescript
   :linenos:

       class Base {
           static foo() { console.log ("static foo() from Base") }
           static bar() { console.log ("static foo() from Base") }
       }

       class Derived extends Base {
           static foo(p: string) { console.log ("static foo() from Derived") }
       }

       Base.foo() // Output: static foo() from Base
       Base.bar() // Output: static foo() from Base
       Derived.bar()           // Compile-time error, there is no bar() in Derived
       Derived.foo("a string") // Output: static foo() from Derived
       Derived.foo()           // Compile-time error, there is no foo(p:string) in Derived


.. note::
   静态方法仍然可以访问参数或局部变量所表示对象上的 ``protected`` 或 ``private``
   成员，只要这些访问本身满足可访问性规则：

   .. code-block:: typescript
      :linenos:

             class C {
               protected count1: number
               private   count2: number
               static getCount(c: C): number {
                 const local_c = new C
                 return c.count1 + c.count2 + local_c.count1 + local_c.count2 // OK
               }
               static handleDerived (b: B) {
                   b.count1 + b.count2 // OK
               }
             }
             class B extends C {
               static dealWithProtected (b: B) {
                   b.count1 // OK
                   b.count2 // Compile-time error
               }
             }

             C.getCount (new C)      // will return the sum of counts
             C.handleDerived (new B) // will work with protected and private fields



.. index::
   static method
   static modifier
   inheritance
   this
   super
   compile-time error

.. _Instance Methods:

实例方法
===============================================================================

.. meta:
    frontend_status: Done

未声明为 ``static`` 的方法就是实例方法。实例方法总是相对于某个对象调用，
该对象在运行时成为 ``this`` 的绑定对象。

.. index::
   static method
   instance method
   non-static method
   declaration
   this keyword
   object
   method body
   execution
   instance

.. _Abstract Methods:

抽象方法
===============================================================================

.. meta:
    frontend_status: Done

抽象方法只把该方法作为成员连同其签名一起引入，但不提供实现。抽象方法在声明中使用
``abstract`` 修饰符。

非抽象方法也可以称为具体方法。

只有抽象类（见 :ref:`Abstract Classes`）才能声明抽象方法。具体类必须为继承到的抽象方法
提供可用实现。

以下情况会导致 :index:`compile-time error`：

- 抽象方法被声明为 ``private``；
- ``abstract`` 与 ``static``、``final``、``native`` 或 ``async`` 同时出现；
- 抽象方法没有直接声明在抽象类中；
- 非抽象子类没有为继承到的抽象方法提供实现。

抽象子类中声明的抽象方法可以覆盖另一个抽象方法。抽象方法也可以覆盖从基类或基接口继承而来的
非抽象方法：

.. code-block:: typescript
   :linenos:

       class C {
           foo() {}
       }
       interface I {
           foo() {} // default implementation
       }
       abstract class X extends C implements I {
           abstract foo(): void /* Here abstract foo() overrides both foo()
                                   coming from class C and interface I */
       }


也就是说，抽象方法不仅能延迟实现，还能显式重新声明一个继承而来的默认实现，
要求更下层的具体子类重新给出实现。

.. index::
   abstract method
   abstract modifier
   abstract class
   implementation
   overriding
   concrete method

.. index::
   method declaration
   abstract method
   private modifier
   static modifier
   final modifier
   native modifier
   async modifier
   declaration
   non-abstract method
   method
   member
   signature
   implementation
   abstract
   non-abstract subclass
   method signature
   inheritance
   interface

.. _Async Methods:

异步方法
===============================================================================

.. meta:
    frontend_status: Done

异步方法的语义在并发章节中定义（见 :ref:`Concurrency Async Methods`）。本章只强调：
异步方法仍然遵循类方法的声明、访问与覆盖框架。

.. index::
   async method

.. _Overriding Methods:

方法覆盖
===============================================================================

.. meta:
    frontend_status: Done

``override`` 修饰符表明该实例方法旨在覆盖超类中的某个实例方法
（见 :ref:`Overriding`）。

``override`` 修饰符的使用是可选的，但强烈建议显式写出，因为这样可以把覆盖意图明确表达出来。

以下情况会导致 :index:`compile-time error`：

- 带 ``override`` 的方法实际上没有覆盖任何超类方法；
- 声明同时包含 ``static`` 与 ``override``。

如果被覆盖方法的签名中带有默认参数值（见 :ref:`Optional Parameters`），
则覆盖方法必须始终为相应参数使用完全相同的默认值。否则发生
:index:`compile-time error`。

更详细的覆盖规则见 :ref:`Overriding in Classes` 与 :ref:`Overriding in Interfaces`。
这些章节会进一步给出类与接口中覆盖关系的语义检查细节。

.. index::
   override modifier
   abstract modifier
   static modifier
   final method
   modifier
   signature
   overriding
   method
   overriding method
   default value
   superclass
   class
   instance
   interface
   subclass
   compile-time error

.. _Native Methods:

原生方法
===============================================================================

.. meta:
    frontend_status: Done

原生方法的具体语义参见实验性特性章节（见 :ref:`Native Methods Experimental`）。
在类章节中，只需要遵守方法声明与方法体合法性规则。

.. index::
   native method

.. _Method Body:

方法体
===============================================================================

.. meta:
    frontend_status: Done

方法体是实现方法的代码块。分号或空体（即完全没有方法体）都表示该方法没有实现。

抽象方法与原生方法必须使用空体形式。

更具体地说：

- 抽象方法和原生方法必须使用空体或分号形式；
- 非抽象且非原生的方法必须带有实际代码块；
- 若方法的返回类型不是 ``void``，则所有正常完成的执行路径都必须返回值。

若抽象方法或原生方法声明了非空方法体，则发生 :index:`compile-time error`。
返回语句在方法体中的约束见 :ref:`Return Statements`。
若方法声明了非 ``void`` 返回类型，但某条执行路径没有返回值，则发生 :index:`compile-time error`。

.. index::
   method body
   semicolon
   empty body
   block
   implementation
   implementation method
   abstract method
   native method
   empty body
   method body
   method declaration
   return statement
   return type
   normal completion

.. _Methods Returning this:

返回 ``this`` 的方法
===============================================================================

.. meta:
    frontend_status: Done

实例方法的返回类型可以声明为 ``this``。
这表示该返回类型就是该方法所属类的类类型。
这是规范中允许把关键字 ``this`` 作为类型注解使用的特殊位置之一
（见 :ref:`Signatures` 和 :ref:`Return Type`）。

实例方法中唯一允许返回的结果是 ``this``。有两种方式可以返回 ``this``：

- 字面量 ``return this``；或
- 其他同样返回 ``this`` 的方法的调用结果。

方法可以调用另一个返回 ``this`` 的方法来实现链式调用：

.. code-block:: typescript
   :linenos:

       class C {
           foo(): this {
               return this
           }
           bar(): this {
               return this.foo()
           }
       }

.. index::
    return type
    instance method
    type
    class
    method
    method signature
    signature
    this keyword
    this statement
    subclass
    annotation

子类中覆盖方法的返回类型也必须是 ``this``：

.. code-block:: typescript
   :linenos:

       class C {
           foo(): this {
               return this
           }
       }

       class D extends C {
           foo(): this {
               return this
           }
       }

       let x = new C().foo() // type of 'x' is 'C'
       let y = new D().foo() // type of 'y' is 'D'

否则，发生 :index:`compile-time error`。

.. index::
    return type
    overriding
    overridden method
    subclass

|

.. _Class Accessor Declarations:

类访问器声明
*******************************************************************************

.. meta:
    frontend_status: Done

类访问器以 getter 或 setter 的形式声明，本质上是带特殊约束的方法。
访问器常用于替代直接字段访问，以增加读写控制、隐藏存储实现或实现接口属性。

其语法如下：

.. code-block:: abnf

       classAccessorDeclaration:
           classAccessorModifier*
           ( 'get' identifier '(' ')' returnType? block?
           | 'set' identifier '(' parameter ')' block?
           )
           ;

       classAccessorModifier:
           'abstract'
           | 'static'
           | 'final'
           | 'override'
           | 'native'
           ;

访问器修饰符是方法修饰符的一个子集，含义与对应方法修饰符相同
（``abstract`` 参见 :ref:`Abstract Methods`，``static`` 参见 :ref:`Static Methods`，
``final`` 参见 :ref:`Final Methods`，``override`` 参见 :ref:`Overriding Methods`，
``native`` 参见 :ref:`Native Methods`）。
getter 必须有返回类型且不接收参数；setter 必须恰好接收一个参数且没有返回类型。

在继承与覆盖过程中，访问器也要遵守与名称、访问修饰符、类型兼容性相关的规则。

.. code-block:: typescript
   :linenos:

       class Person {
         private _age: number = 0
         get age(): number { return this._age }
         set age(a: number) {
           if (a < 0) { throw new Error("wrong age") }
           this._age = a
         }
       }

get-accessor 必须显式声明返回类型，或其返回类型能从 getter 体中推断出来；
set-accessor 必须只有一个参数，且不能声明返回类型。把 getter/setter 当方法调用，
或把 setter 的唯一参数声明为可选参数（见 :ref:`Optional Parameters`），都会导致
:index:`compile-time error`。

.. code-block:: typescript
   :linenos:

       class Person {
         private _age: number = 0
         get age(): number { return this._age }
         set age(a: number) {
           if (a < 0) { throw new Error("wrong age") }
           this._age = a
         }
       }

       let p = new Person()
       p.age = 25        // setter is called
       if (p.age > 30) { // getter is called
         // do something
       }
       p.age(17) // Compile-time error, setter is used as a method
       let x = p.age() // Compile-time error, getter is used as a method

       class X {
           set x (p?: Object) {} // Compile-time error, setter has optional parameter
       }

如果 getter 没有显式返回类型，则按 :ref:`Return Type Inference` 推断：

.. code-block:: typescript
   :linenos:

       class Person {
         private _age: number = 0
         get age() { return this._age } // return type is inferred as number
       }


类可以只声明 getter、只声明 setter，或同时声明同名 getter 与 setter。
如果同名 getter 与 setter 同时存在，则两者的访问器修饰符必须一致；
否则会发生 :index:`compile-time error`。

访问器实现可以借助私有字段或普通字段保存数据：

.. code-block:: typescript
   :linenos:

       class Person {
         forename: string = ""
         surname: string = ""
         get fullName(): string {
           return this.surname + " " + this.forename
         }
       }
       console.log (new Person().fullName)

访问器名称不能与非静态字段名冲突，也不能与类或接口中的方法名冲突：

.. code-block:: typescript
   :linenos:

       class Person {
         name: string = ""
         get name(): string { // Compile-time error, getter name clashes with the field name
             return this.name
         }
         set name(a_name: string) { // Compile-time error, setter name clashes with the field name
             this.name = a_name
         }
       }

在继承与覆盖（见 :ref:`Overriding`）中，访问器按方法处理。
getter 返回类型遵循协变模式，setter 参数类型遵循逆变模式
（见 :ref:`Override-Compatible Signatures`）：

.. code-block:: typescript
   :linenos:

       class Base {
         get field(): Base { return new Base }
         set field(a_field: Derived) {}
       }
       class Derived extends Base {
         override get field(): Derived { return new Derived }
         override set field(a_field: Base) {}
       }
       function foo (base: Base) {
          base.field = new Derived // setter is called
          let b: Base = base.field // getter is called
       }
       foo (new Derived)

.. index::
   class accessor declaration
   getter
   setter
   inferred return type
   optional parameter
   accessor modifier
   overriding
   covariance
   contravariance

.. index::
   accessor modifier
   access modifier
   method modifier
   subset
   abstract modifier
   native modifier
   static method

.. index::
   get-accessor
   getter
   getter body
   inferred type
   type inference
   return type
   parameter
   set-accessor
   setter
   field
   method

.. index::
   getter
   return type
   inferred type
   setter
   accessor
   private field
   accessor modifier
   implementation
   non-static

若访问器在类或接口之外使用，则发生 :index:`compile-time error`。

.. index::
   overriding
   inheritance
   accessor
   method
   getter parameter
   setter parameter
   parameter type
   covariance pattern
   contravariance pattern

|

.. _Constructor Declaration:

构造函数声明
*******************************************************************************

.. meta:
    frontend_status: Partly

构造函数用于初始化作为类实例的对象。构造函数声明以关键字 ``constructor`` 开头，
并且可以带可选名称。从其他语法角度看，构造函数声明类似于没有返回类型的方法声明：

其语法如下：

.. code-block:: abnf

       constructorDeclaration:
           'native'? 'constructor' identifier? parameters constructorBody?
           ;

构造函数形式参数的语法和语义规则与方法参数完全一致。

超类型中定义的构造函数不能直接用于其子类型的 ``new`` 表达式（见
:ref:`New Expressions`），但可以在派生类构造函数中通过 ``super`` 调用
（见 :ref:`Explicit Constructor Call`）使用：

.. code-block:: typescript
   :linenos:

       class C {
           constructor (s: string) {}
       }
       class D extends C {
       }

       new D("aa") // Compile-time error, 'D' has default constructor only

       class D1 extends C {
           constructor (n: number) {
               super("" + n) // OK
           }
       }

构造函数会以下列方式被调用：

- 类实例创建表达式（见 :ref:`New Expressions`）；
- 其他构造函数中的显式构造函数调用（见 :ref:`Constructor Body`）。

构造函数的可访问性受访问修饰符（见 :ref:`Access Modifiers` 和 :ref:`Scopes`）控制。
若构造函数被声明为不可访问，则类实例化不能使用该构造函数。若类中唯一的构造函数不可访问，
则该类实例无法被创建。

``native`` 构造函数（见 :ref:`Native Constructors`）不得带 ``constructorBody``；
非 ``native`` 构造函数必须带 ``constructorBody``。否则发生 :index:`compile-time error`。

.. index::
   constructor
   initialization
   class instance
   instance
   constructor declaration
   constructor keyword
   optional name
   syntax
   method declaration
   return type
   optional identifier
   identifier
   accessibility
   native constructor

.. index::
   class instance
   class instantiation
   expression
   constructor
   instance creation expression
   constructor keyword
   constructor declaration
   constructor call
   access modifier
   access
   non-native constructor

.. _Constructor Body:

构造函数体
===============================================================================

.. meta:
    frontend_status: Done

*构造函数体* 是实现构造函数的代码块。

其语法如下：

.. code-block:: abnf

       constructorBody:
           '{' statement* '}'
           ;

构造函数体必须保证新实例被正确初始化。构造函数分为主构造函数与次构造函数两类，
它们在调用超类构造函数、执行字段初始化和自定义逻辑的顺序上存在差异。

如果构造函数体中未正确调用必要的 ``super`` 或未满足字段初始化要求，
则可能导致编译时错误或运行时问题。

主构造函数负责为新实例完成超类初始化、字段初始化以及自身构造逻辑；
次构造函数则需要通过显式构造函数调用（见 :ref:`Explicit Constructor Call`）把
控制流转交给主构造函数或超类构造函数。

更具体地说，主构造函数的高层执行顺序包括：

1. 可选的预备代码，但不得使用 ``this`` 或 ``super``；

2. 如有扩展子句（见 :ref:`Class Extension Clause`），则调用超类构造函数
   （见 :ref:`Explicit Constructor Call`）。若出现以下情形，则发生
   :index:`compile-time error`：

   - 该调用不是构造函数中的根级语句；
   - 调用参数使用了 ``this`` 或 ``super``。

3. 编译器按类体顺序插入字段初始化器；

4. 在字段全部完成初始化前，不得通过 ``this`` 读取未初始化字段；

5. 所有字段初始化完成后，才允许执行任意实例逻辑。

如果构造函数体没有显式写出 ``super(...)``，编译器会隐式插入无参 ``super()``。
若超类不存在可访问的无参构造函数，则发生 :index:`compile-time error`。

.. code-block:: typescript
   :linenos:

       class C1 {
           constructor() {}
       }
       class D1 extends C1 {
           constructor () {
               // OK, call 'super()' is implicitly added
               /* other code here */
           } 
       }

       class C2 {
           constructor(n: number) {}
       }
       class D2 extends C2 {
           constructor () {
               // Compile-time error, call 'super()' cannot be implicitly added
               /*other code here*/
           }
       }
       
       class C3 {
           constructor(n?: number) {}
       }
       class D3 extends C3 {
           constructor () {
               // OK, call 'super()' is implicitly added
               /*other code here*/
           } 
       }
       
若编译器能静态发现读取了未初始化字段，则发生 :index:`compile-time error`；
若只能确认存在风险但无法完全证明，则发出 :index:`compile-time warning`：

.. code-block:: typescript
   :linenos:

       class Base {
         x: Object
         constructor() {
             this.x = new Object // Base object is fully initialized
             crash_this (this)   // A compiler may issue a compile-time error
         }
       }
       class Derived extends Base {
         y: Object
         constructor () {
             super() // mandatory call to base class constructor
             crash_this(this) // Compile-time warning as this.y is not initialized yet
                              // is guaranteed, however a compiler can issue a
                              // Compile-time error
             this.y = new Object
         }
       }
       function crash_this (b: Base) {
             if (b instanceof Derived) { // If b is of type Derived, then
                   console.log ((b as Derived).y) // Access y field of Derived object
                   // Depending on the compilation context, either the compiler reports
                   // a compile-time error, or the runtime system is to detect the case
             }
       }

主构造函数示例如下：

.. code-block:: typescript
   :linenos:

    class Point {
        x: number
        y: number
        constructor(x: number, y: number) {
            this.x = x
            this.y = y
        }
    }

    class ColoredPoint extends Point {
        static readonly WHITE = 0
        static readonly BLACK = 1
        color: number
        constructor(x: number, y: number, color: number) {
            super(x, y)
            this.color = color
        }
    }

    class BWPoint extends ColoredPoint {
        constructor(x: number, y: number, black: boolean) {
            console.log("Code that uses neither 'this' nor 'super'")
            super(x, y, black ? ColoredPoint.BLACK : ColoredPoint.WHITE)
            console.log("Any code after that point")
        }
    }

下例演示了 new 表达式（见 :ref:`New Expressions`）与构造函数体执行顺序的协作方式：

.. code-block:: typescript
   :linenos:

      let count = 0
      function trace (msg: string) {
         count++
         console.log (count + ": " + msg)
         return count
      }

      class C {
         C_field = trace("C fields initialization performed")
         constructor() {
            trace ("C constructor called")
         }
      }
      class B extends C {
         B_field = trace("B fields initialization performed")
         constructor() {
            super ()
            trace ("B constructor called")
         }
      }
      class A extends B {
         A_field = trace("A fields initialization performed")
         constructor(p: number) {
            super()
            trace ("A constructor called")
         }
      }
      new A (trace("constructor arguments evaluated"))

      // The output
      // "1: constructor arguments evaluated"
      // "2: C fields initialization performed"
      // "3: C constructor called "
      // "4: B fields initialization performed"
      // "5: B constructor called "
      // "6: A fields initialization performed"
      // "7: A constructor called "

次构造函数体的高层执行顺序包括：

1. 可选的预备代码，但不得使用 ``this`` 或 ``super``；

2. 对同一类中另一构造函数的强制调用（使用关键字 ``this``，
   见 :ref:`Explicit Constructor Call`）。若出现以下情形，则发生
   :index:`compile-time error`：

   - 该调用不是构造函数中的根级语句；
   - 调用参数使用了 ``this`` 或 ``super``。

3. 可选的任意代码。

若构造函数直接或间接通过 ``this`` 调用自身，则发生 :index:`compile-time error`。

主构造函数与次构造函数综合示例：

.. code-block:: typescript
   :linenos:

       class Point {
         x: number
         y: number
         constructor(x: number, y: number) {
           this.x = x
           this.y = y
         }
       }

      class ColoredPoint extends Point {
        static readonly WHITE = 0
        static readonly BLACK = 1
        color: number
        // calls base class constructor
        // Primary constructor:
        constructor(x: number, y: number, color: number) {
          super(x, y) // Calls base class constructor as class has 'extends'
          this.color = color
        }
        // Secondary constructor:
        constructor (color: number) {
          this(0, 0, color) // Calls same class primary constructor
        }
      }

      class BWPoint extends ColoredPoint {
         constructor(x: number, y: number, black: boolean) {
           console.log ("Code that uses neither 'this' nor 'super'")
           super (x, y, black ? ColoredPoint.BLACK : ColoredPoint.WHITE)
           console.log ("Any code after that point")
         }
       }

构造函数体类似方法体（见 :ref:`Method Body`），但明确禁止使用带值的 return 语句
（见 :ref:`Return Statements`）；允许使用无表达式的 return 语句。

构造函数体中对当前类或直接超类构造函数的调用次数不得超过一次。否则，发生
:index:`compile-time error`。

.. index::
   constructor body
   primary constructor
   secondary constructor
   super call
   field initialization
   compile-time warning
   compile-time error

.. index::
   constructor body
   initialization
   class instance
   primary constructor
   instance own field
   secondary constructor
   constructor
   instance

.. index::
   primary constructor
   constructor body
   this
   super
   mandatory call
   field initializer

.. index::
   primary constructor
   secondary constructor
   readonly
   constructor
   class

.. index::
   constructor
   constructor call
   constructor body
   method body
   semantics
   value
   return statement
   expression
   class
   superclass

.. _Explicit Constructor Call:

显式构造函数调用
===============================================================================

.. meta:
    frontend_status: Done

显式构造函数调用分为两类：

- 以关键字 ``super`` 开头的*超类构造函数调用*，用于调用直接超类中的构造函数；
- 以关键字 ``this`` 开头的*其他构造函数调用*，用于调用同一个类中的另一个构造函数。

若出现以下情况，则发生 :index:`compile-time error`：

- ``super`` 调用指向不可访问的直接超类构造函数；
- 显式构造函数调用被当作表达式使用。

如果显式构造函数调用的实参引用了以下任一内容，则发生
:index:`compile-time error`：

- 任意非静态字段或实例方法；
- ``this`` 或 ``super``。

.. code-block:: typescript
   :linenos:

    class Base {
        constructor () {}
    }
    class Derived extends Base {
        constructor () {
            super()        // Call Base class constructor
        }
    }
    class Derived2 extends Base {
        constructor (x: number) {
            super()        // Call Base class constructor
        }
    }

对 ``super`` 调用的语义检查按 :ref:`Compatibility of Call Arguments`
进行。

.. index::
   explicit constructor call
   constructor call
   superclass constructor call
   this keyword
   super keyword
   constructor
   superclass
   call
   superclass constructor call
   constructor call
   non-static field
   instance method
   base class

.. _Default Constructor:

默认构造函数
===============================================================================

.. meta:
    frontend_status: Done

如果某个类没有显式声明构造函数，则编译器会为其合成默认构造函数。
这样可以保证每个类实际上都至少拥有一个构造函数。默认构造函数的形式如下：

- 默认构造函数带有 ``public`` 修饰符（见 :ref:`Access Modifiers`）；
- 默认构造函数不带任何参数。

对非 Object 类（见 :ref:`Type Object`），默认构造函数体包含：

- 对超类无参构造函数的调用；
- 按类体声明顺序执行字段初始化器（如有）。

对 Object 类（见 :ref:`Type Object`），默认构造函数体为空。

若超类没有可访问的（见 :ref:`Accessible`）无参构造函数，则发生
:index:`compile-time error`。

.. code-block:: typescript
   :linenos:

      // Class declarations with default constructors declared implicitly
      class Base_no_ctor_declared {}
      class Derived_no_ctor_declared extends Base_no_ctor_declared {}

      // Example of an error case
      class A {
          private constructor () {}
      }
      class B0 extends A {} // Compile-time error as default constructor for B0
                            // cannot call super() as it is private
      class B1 extends A {
           constructor () {
               super ()   // Compile-time error as super() is private
           }
      }

.. index::
   class
   constructor declaration
   constructor
   default constructor
   public modifier
   access modifier
   call
   constructor body
   superclass constructor
   argument
   accessible constructor
   class Object
   accessibility
   parameter
   execution
   field initializer
   class body

.. index::
   class declaration
   constructor
   default constructor
   superclass
   compile-time error
   constructor call
   super
   private constructor
   access

|

.. _Inheritance:

继承
*******************************************************************************

.. meta:
    frontend_status: Done

类 ``C`` 会继承其直接超类中的所有非静态且可访问成员。
它也会继承直接超接口中的所有非静态且可访问成员，并且可以选择覆盖或实现其中一部分成员。

如果 ``C`` 不是抽象类，那么它必须实现所有继承而来的抽象方法。

每个继承而来的抽象方法都必须由一个具有 :ref:`Override-Compatible Signatures`
的方法来实现。

继承方法和访问器时所执行的覆盖语义检查见 :ref:`Overriding in Classes`。

覆盖字段以及实现属性时所执行的语义检查见：

- :ref:`Override Fields`
- :ref:`Implementing Required Interface Properties`
- :ref:`Implementing Optional Interface Properties`

超类构造函数不会被覆盖，因为它们在子类中并不可直接访问；
它们只能通过构造函数体中的显式构造函数调用参与实例初始化。

非抽象类必须为所有继承而来的抽象方法提供实现，而且这些实现的方法签名必须满足
:ref:`Override-Compatible Signatures`。方法和访问器的覆盖语义由
:ref:`Overriding in Classes` 约束；字段与属性实现则分别受
:ref:`Override Fields`、:ref:`Implementing Required Interface Properties`、
:ref:`Implementing Optional Interface Properties` 约束。

.. index::
   class
   inheritance
   inherited member
   accessibility
   accessible member
   abstract method
   override-compatible signature
   superclass
   superinterface
   direct superclass
   direct superinterface
   overriding
   semantic check
   constructor
   constructor body
   accessor

|
