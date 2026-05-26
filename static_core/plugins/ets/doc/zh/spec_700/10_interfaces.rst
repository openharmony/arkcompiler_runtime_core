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


.. _Interfaces:

接口
##########

.. meta:
    frontend_status: Done

接口声明声明一种 *接口类型*，即一种引用类型，该类型：

- 包含属性和方法作为其成员；
- 不包含实例变量（字段）；
- 通常声明一个或多个方法；
- 允许原本互不相关的类为这些方法提供实现，从而实现该接口。

.. index::
   interface declaration
   interface type
   reference type
   instance variable
   field
   property
   method
   member
   implementation
   class
   interface

无法创建接口类型的实例。

一个接口可以被声明为对一个或多个其他接口的 *直接扩展*。若如此，该接口会继承其所扩展接口中的全部成员。
继承而来的成员可以选择性地被重写或隐藏。

一个类可以被声明为 *直接实现* 一个或多个接口。类的任意实例都实现其接口中指定的全部方法。
一个类还会实现其直接超类和直接超接口所实现的全部接口。接口继承使对象能够在不共享超类的前提下支持共同行为。

.. index::
   interface
   interface type
   creation
   instantiation
   direct extension
   inheritance
   inherited member
   extension
   superinterface
   direct implementation
   superclass
   object
   overriding
   hiding
   overridden member
   hidden member

声明为 *接口类型* 的变量，其值可以是对任意实现了该指定接口的类实例的引用。
但是，仅仅让某个类实现接口中的所有方法并不够。该类或其某个超类必须显式声明实现该接口；
否则，该类不会被视为实现了该接口。

子类型化规则将在 :ref:`Subtyping for Non-Generic Classes and Interfaces` 和 :ref:`Subtyping for Generic Classes and Interfaces` 中详细讨论。

.. index::
   value
   variable
   interface type
   interface
   class
   superclass
   declaration
   instance
   reference
   method
   implementation
   assignability
   Object

|

.. _Interface Declarations:

接口声明
********************************************************************************

.. meta:
    frontend_status: Done

*接口声明* 指定一个新的具名引用类型。

*接口声明* 的语法如下：


.. index::
   interface declaration
   reference type
   syntax

.. code-block:: abnf

    interfaceDeclaration:
        'interface' identifier typeParameters?
        interfaceExtendsClause? '{' interfaceMember* '}'
        ;

    interfaceExtendsClause:
        'extends' interfaceTypeList
        ;

    interfaceTypeList:
        typeReference (',' typeReference)*
        ;

接口声明中的 *identifier* 指定接口名。

带有 ``typeParameters`` 的接口声明会引入一个新的泛型接口（见 :ref:`Generics`）。

接口声明的作用域在 :ref:`Scopes` 中定义。

.. The interface declaration shadowing is specified in :ref:`Shadowing by Parameter`.

.. index::
   identifier
   interface declaration
   interface name
   class name
   generic interface
   generic declaration
   scope

|

.. _Superinterfaces and Subinterfaces:

超接口与子接口
********************************************************************************

.. meta:
    frontend_status: Done

带有 ``extends`` 子句声明的接口会扩展所有其他被命名的接口，并因此继承这些接口的全部成员。
这些被扩展的具名接口就是该已声明接口的 *直接超接口*。实现该已声明接口的类，也会实现该接口 *extends* 的所有接口。

.. index::
   superinterface
   subinterface
   extends clause
   direct superinterface
   implementation
   declared interface
   interface
   inheritance

若出现以下情形，则会发生 :index:`compile-time error`：

- ``extends`` 子句中的 ``typeReference`` 直接引用了某个非接口类型，或者是该非接口类型的别名；
- ``typeReference`` 所命名的接口类型不是 :ref:`Accessible` 的；
- ``typeReference`` 的类型实参（见 :ref:`Type Arguments`）所表示的参数化类型不是良构的（见 :ref:`Generic Instantiations`）；
- ``extends`` 图中存在环。

.. index::
   extends clause
   alias
   non-interface type
   interface declaration
   interface type
   access
   accessibility
   scope
   type argument
   parameterized type
   generic instantiation
   extends graph
   well-formed parameterized type

如果某个接口声明（可能是泛型的）``I`` <``F``:sub:`1` ``,...,
F``:sub:`n`>（:math:`n\geq{}0`）包含 ``extends`` 子句，那么接口类型
``I`` <``F``:sub:`1` ``,..., F``:sub:`n`> 的 *直接超接口*，就是 ``I`` 的声明中
``extends`` 子句给出的那些类型。

参数化接口类型 ``I`` <``T``:sub:`1` ``,..., T``:sub:`n`> 的全部 *直接超接口* 为
``J`` <``U``:sub:`1`:math:`\theta{}` ``,..., U``:sub:`k`:math:`\theta{}`>，
当且仅当：

- ``T``:sub:`i`（:math:`1\leq{}i\leq{}n`）是某个泛型接口声明 ``I``
  <``F``:sub:`1` ``,..., F``:sub:`n`>（:math:`n > 0`）的类型；
- ``J`` <``U``:sub:`1` ``,..., U``:sub:`k`> 是 ``I`` <``F``:sub:`1` ``,..., F``:sub:`n`>
  的一个直接超接口；并且
- :math:`\theta{}` 是一个替换
  [``F``:sub:`1` ``:= T``:sub:`1` ``,..., F``:sub:`n` ``:= T``:sub:`n`]。

.. index::
   interface declaration
   generic
   generic declaration
   extends clause
   interface type
   declaration
   direct superinterface
   parameterized interface
   substitution
   superinterface

直接超接口关系的传递闭包形成 *超接口* 关系。

若 *K* 是 *I* 的超接口，则接口 *I* 是 *K* 的 *子接口*。
若满足以下任一条件，则接口 *K* 是 *I* 的超接口：

- *I* 是 *K* 的直接子接口；或者
- *K* 是某个接口 *J* 的超接口，而 *I* 又是 *J* 的子接口。

.. index::
   transitive closure
   direct superinterface
   superinterface
   direct subinterface
   interface
   subinterface

并不存在一个所有接口都扩展自它的单一接口（这一点不同于每个类都扩展自 ``Object`` 类）。

如果接口依赖于其自身，则会发生 :index:`compile-time error`。

.. index::
   interface
   object
   class
   method
   extension
   implementation
   override-compatible signature

|

.. _Interface Members:

接口成员
********************************************************************************

.. meta:
    frontend_status: Done

*接口声明* 包含 *接口成员*，它们要么是属性（见 :ref:`Interface Properties`），要么是方法
（见 :ref:`Interface Method Declarations`）。

*接口成员* 的语法如下：

.. code-block:: abnf

    interfaceMember
        : annotationUsage?
        ( interfaceProperty
        | interfaceMethodDeclaration
        | explicitInterfaceMethodOverload
        )
        ;

接口类型 ``I`` 所声明或继承（见 :ref:`Interface Inheritance`）的成员 ``m`` 的声明作用域，在 :ref:`Scopes` 中规定。

注解的用法见 :ref:`Using Annotations`。

.. index::
   interface
   interface member
   interface type
   property
   method
   syntax
   interface declaration
   method declaration
   scope
   inheritance
   annotation

*接口成员* 包括：

- 在接口声明中显式声明的成员；
- 从直接超接口继承的成员（见 :ref:`Superinterfaces and Subinterfaces`）。

如果接口显式声明的方法与 Object 的某个 public 方法同名，并且其签名与该方法在语义上兼容
（见 :ref:`Override-Compatible Signatures`），且该方法还是接口默认方法（见
:ref:`Default Interface Method Declarations`），
那么这个方法将永远不会被调用。

.. code-block:: typescript
   :linenos:

    interface I {
        toString(): string {
             return "This method will never be called"
        }
    }
    class A implements I {}
    console.log ((new A).toString()) // Object.toString() is called

.. index::
   interface
   interface member
   inheritance
   interface declaration
   direct superinterface
   Object
   public method

接口会继承其所扩展接口的全部成员（见“接口继承”）。

声明作用域中的名称必须唯一。也就是说，接口类型的属性名和方法名不得相同（见
:ref:`Interface Declarations`）。

.. index::
   inheritance
   interface
   property
   method
   declaration scope
   interface type
   interface declaration
   scope

|

.. _Interface Properties:

接口属性
********************************************************************************

.. meta:
    frontend_status: Done

*接口属性* 是一种 *accessor*，它可以声明为字段声明形式，或者 getter / setter 声明形式，
或者同时为 getter 与 setter 声明形式。

*接口属性* 的语法如下：

.. code-block:: abnf

    interfaceProperty:
        'readonly'? identifier '?'? ':' type
        | 'get' identifier '(' ')' returnType
        | 'set' identifier '(' requiredParameter ')'
        ;

.. index::
   interface
   property
   field
   accessor
   getter
   setter
   interface property
   syntax

如果接口属性属于以下任一种情形，则它是必需属性（见 :ref:`Required Interface Properties`）：

- 显式的 *accessor*，即 getter 或 setter；或者
- 不带 ``'?'`` 的字段形式。

否则，它就是可选属性（见 :ref:`Optional Interface Properties`）。

如果在属性名后使用 ``'?'``，则该属性类型在语义上等价于 ``type | undefined``。

.. code-block:: typescript
   :linenos:

    interface I {
        property?: Type
    }
    // is the same as
    interface I {
        property: Type | undefined
    }

.. index::
   interface property
   interface
   property
   required property
   optional property
   accessor
   getter
   setter
   field
   property type
   semantic equivalent

|

.. _Required Interface Properties:

必需接口属性
================================================================================

.. meta:
    frontend_status: Done

以字段形式定义的 *必需属性* 会隐式定义如下内容：

- 如果该属性被标记为 ``readonly``，则定义一个 getter；
- 否则，定义同名的 getter 与 setter。

字段的类型注解定义 getter 的返回类型，以及 setter 参数的类型。

因此，以下声明具有相同效果：

.. index::
   property
   interface
   required property
   interface property
   field
   accessor
   readonly
   getter
   setter
   property
   type annotation
   parameter
   return type

.. code-block:: typescript
   :linenos:

    interface Style {
        color: string
    }
    // is the same as
    interface Style {
        get color(): string
        set color(s: string)
    }

.. note::
   以 accessor 形式定义的 *必需属性*，不会在接口中额外定义任何实体。

实现带属性接口的类，同样可以使用字段形式或 accessor 形式（见 :ref:`Implementing Required Interface Properties` 和 :ref:`Implementing Optional Interface Properties`）。

.. index::
   string
   implementation
   required property
   accessor
   interface
   interface property
   optional property
   field
   notation
   property
   entity
   class

|

.. _Optional Interface Properties:

可选接口属性
================================================================================

.. meta:
    frontend_status: Done

*可选属性* 可以用两种形式定义：

- 简写形式 ``identifier '?' ':' T``；或
- 显式形式 ``identifier ':' T | undefined``。


在这两种情形下，``identifier`` 的有效类型都是 ``T | undefined``。

*可选属性* 会隐式定义如下内容：

.. index::
   optional property
   interface property
   identifier

- 如果该属性被标记为 ``readonly``，则定义一个 getter；
- 否则，定义同名的 getter 与 setter。

这些 accessor 具有隐式定义的函数体；在这一点上，它们类似于 :ref:`Default Interface Method Declarations`。
然而，|LANG| 不支持显式定义带函数体的 accessor。

下列声明：

.. code-block:: typescript
   :linenos:

    interface I {
        num?: number
    }

-- 会隐式声明两个 accessor：

.. code-block:: typescript
   :linenos:

    interface I {
        get num(): number | undefined { return undefined }
        set num(x: number | undefined) { throw new InvalidStoreAccessError }
    }

如果在实现该接口的类中没有重写默认 setter，那么在尝试给可选属性赋值时会抛出
``InvalidStoreAccessError``。另见 :ref:`Implementing Optional Interface Properties`。

.. index::
   getter
   setter
   implementation
   value
   optional property
   readonly
   accessor
   body
   interface property

|

.. _Interface Method Declarations:

接口方法声明
********************************************************************************

.. meta:
    frontend_status: Done

普通 *接口方法声明* 指定方法名和签名，这类声明称为 *抽象* 声明。其隐式可访问性为 ``public``。

作为实验性特性，接口方法也可以带有函数体（见 :ref:`Default Interface Method Declarations`）。

.. index::
   interface
   interface method declaration
   method name
   method signature
   method
   declaration
   signature
   interface method
   method body
   abstract declaration

*接口方法声明* 的语法如下：

.. code-block:: abnf

    interfaceMethodDeclaration:
        identifier signature
        | interfaceDefaultMethodDeclaration
        ;


.. index::
   interface method declaration
   declaration
   syntax

|

.. _Interface Inheritance:

接口继承
********************************************************************************

.. meta:
    frontend_status: Done

接口 *I* 会从其直接超接口继承全部属性和方法。相关语义检查见 :ref:`Overriding in Interfaces` 和 :ref:`Implicit Method Overloading`。

.. note::
   方法的语义规则同样适用于属性，因为任意接口属性都会隐式定义 getter、setter 或两者兼有。

在接口体内，超接口中定义的私有方法不可访问（见 :ref:`Accessible`）。

.. index::
   inheritance
   interface
   interface inheritance
   direct superinterface
   overriding
   method
   superinterface
   semantic check
   private method
   property
   getter
   setter
   access
   accessibility
   interface body

如果接口 *I* 声明了一个 ``private`` 方法 *m*，且其签名（见 :ref:`Override-Compatible Signatures`）与 *I* 的某个超接口中任意可访问修饰符的方法
:math:`m'` 兼容，则会发生 :index:`compile-time error`。

.. index::
   interface
   declaration
   method
   private method
   compatibility
   instance method
   override-compatible signature
   access
   access modifier
   superinterface
   private method
   signature


同样的方案也适用于属性和 accessor：

.. code-block:: typescript
   :linenos:

    interface I1 {
        prop1: number
        set prop2 (p: number)
        get prop3 (): number
    }
    interface I2 {
        prop1: number
        set prop2 (p: number)
        get prop3 (): number
    }
    interface I3 extends I1, I2 {}
    // There is only one property prop1, prop2 and prop3 in I3

    function foo (i3: I3) {
       i3.prop1 = 5 // Setter for prop1 is called
       i3.prop1     // Getter for prop1 is called
       i3.prop2 = 5 // Setter for prop2 is called
       i3.prop2     // Compile-time error as no getter for prop2
       i3.prop3 = 5 // Compile-time error as no getter for prop3
       i3.prop3     // Getter for prop3 is called
    }

.. raw:: pdf

   PageBreak
