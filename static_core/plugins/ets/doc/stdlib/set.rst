
Set API
================

Construction
-------------

Signature: **constructor()**

.. code-block:: typescript

        let m1 = new Set<string>()

Instance Methods
----------------

``length``

Returns length of set.

Signature: **length(): int**

.. code-block:: typescript

        let m1 = new Set<string>()
        console.println(m1.length())

``clear``

Clears set.

Signature: **clear(): void**

.. code-block:: typescript

        let m1 = new Set<string>()
        m1.clear()

``remove``

Removes value in set.

Signature: **remove(k: Char): void**

.. code-block:: typescript

        let m1 = new Set<string>()
        m1.remove('a')

``put``

Puts value to set.

Signature: **put(v: Char): void**

.. code-block:: typescript

        let m1 = new Set<string>()
        m1.remove('a')

``hasValue``

Сhecks if a specified value is in the set.

Signature: **hasValue(v: Char): boolean**

.. code-block:: typescript

        let m1 = new Set<string>()
        console.println(m1.hasValue('a'))
        // expected false

``values``

Returns an array of values.

Signature: **values(): Char[]**

.. code-block:: typescript

        let m1 = new Set<string>()
        m1.values()

``entries``

The ``entries()`` method returns a new Iterator object that contains an
array of [value, value] for each element in the set object in the order
of insertion.

.. code-block:: typescript

        const set1 = new Set<string>()
        set1.add('forty two')
        const iterator1 = set1.entries()

``keys``

The ``keys()`` method is an alias for the ``values()`` method.

.. code-block:: typescript

        const set1 = new Set<string>()
        set1.add('forty two')
        const iterator1 = set1.keys()
