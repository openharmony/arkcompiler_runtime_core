Modules
=======

|

Programs are organized as sets of compilation units, or modules.

Each module creates its own scope, i.e., any declarations (variables,
functions, classes, etc.) declared in the module are not visible outside
that module unless exported explicitly.

Conversely, a variable, function, class, interface, etc. exported from
another module must first be imported to a module.

|

Export
------

A top-level declaration can be exported by using the keyword ``export``.

A declared name that is not exported is considered private, and can be used
only in the module it is declared in.

.. code-block:: typescript

    export class Point {
        x: number = 0
        y: number = 0
        constructor(x: number, y: number) {
          this.x = x
          this.y = y
        }
    }
    export let Origin = new Point(0, 0)
    export function Distance(p1: Point, p2: Point): number {
        return Math.sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y))
    }

|

Import
------

Import declarations are used to import entities exported from other modules,
and to provide their bindings in the current module. An import declaration
consists of two parts:

* Import path that determines the module to import from;
* Import bindings that define the set of usable entities in the imported
  module, and the form of use (i.e., qualified or unqualified use).

Import bindings may have several forms.

Assuming that a module has the path '``./utils``', and export entities 'X' and
'Y', an import binding of the form '``* as A``' binds the name '``A``', and
then all entities exported from the module defined by the import path can be
accessed by using the qualified name ``A.name``:

.. code-block:: typescript

    import * as Utils from "./utils"
    Utils.X // denotes X from Utils
    Utils.Y // denotes Y from Utils

An import binding of the form '``{ ident1, ..., identN }``' binds an exported
entity with a specified name, which can be used as a simple name:

.. code-block:: typescript

    import { X, Y } from "./utils"
    X // denotes X from Utils
    Y // denotes Y from Utils

If a list of identifiers contains aliasing of the form '``ident as alias``',
then entity ``ident`` is bound under the name ``alias``:

.. code-block:: typescript

    import { X as Z, Y } from "./utils"
    Z // denotes X from Utils
    Y // denotes Y from Utils
    X // Compile-time error: 'X' is not visible

|

Top-Level Statements
---------------------

At the module level, a module can contain any statements, except ``return``
ones.

If a module contains a ``main`` function (program entry point), then
top-level statements of the module are executed immediately before
the body of that function.
Otherwise, such statements are executed before the execution of any
other function of the module.

|

Program Entry Point
--------------------

The top-level ``main`` function is an entry point of a program (application).
The ``main`` function must have either an empty parameter list, or a single
parameter of ``string[]`` type.

.. code-block:: typescript

    function main() {
        console.log("this is the program entry")
    }

|
|
