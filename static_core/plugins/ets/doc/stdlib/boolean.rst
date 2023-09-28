
Boolean API
================

Construction
------------
Signature:
* constructor()
* constructor(value: boolean)
* constructor(value: Boolean)

.. code-block:: typescript

        let boo1: Boolean = new Boolean();
        let boo2: Boolean = new Boolean(true);
        let boo3: Boolean = new Boolean(boo2);

Static Methods
--------------

``valueOf``

Returns mangled value of boolean.

Signature: **valueOf(b: boolean): Boolean**

.. code-block:: typescript

        console.println(Boolean.valueOf(true));

