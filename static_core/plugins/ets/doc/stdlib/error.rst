
Error
================

Construction
------------

Signature:

* constructor()
* constructor(str: string)

Properties
----------------

``message``

The ``message`` data property of an Error instance is a human-readable
description of the error.

.. code-block:: typescript

        let e = new Error('Could not parse input');
        // e.message is 'Could not parse input'
        throw e;

``name``

The ``name`` data property of ``Error.prototype`` is shared by all Error
instances.
Represents the name for the type of error. For ``Error.prototype.name``, the
initial value is "Error".
Subclasses like ``TypeError`` and ``SyntaxError`` provide their own name
properties.

.. code-block:: typescript

        let e = new Error("Malformed input"); // e.name is 'Error'

        e.name = "ParseError";
        throw e;
        // e.toString() would return 'ParseError: Malformed input'

``cause``

The ``cause`` data property of an Error instance indicates the specific
original cause of the error.

.. code-block:: typescript

        try {
          connectToDatabase();
        } catch (err) {
          throw new Error('Connecting to database failed.', { cause: err });
        }

``columnNumber``

The ``columnNumber`` data property of an Error instance contains the column 
number in the line of the file that raised the error.

.. code-block:: typescript

        try {
          throw new Error("Could not parse input");
        } catch (err) {
          console.log(err.columnNumber); // 9
        }

``fileName``

The ``fileName`` data property of an Error instance contains the path to the
file that raised the error.

.. code-block:: typescript

        let e = new Error("Could not parse input");
        throw e;
        // e.fileName could look like "file:///C:/example.html"


``stack``

The non-standard ``stack`` property of an Error instance offers a trace of
which functions were called, in what order, from which line and file, and with
what arguments.
The stack string proceeds from the most recent calls to earlier ones, leading
back to the original global scope call.

.. code-block:: typescript

        try {
          new Function("throw new Error()")();
        } catch (e) {
          console.log(e.stack);
        }
        // anonymous@file:///C:/example.html line 7 > Function:1:1
        // @file:///C:/example.html:7:6


