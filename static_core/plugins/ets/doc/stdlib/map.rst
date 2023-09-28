
Map API
================

Construction
------------

Signature: **constructor()**

.. code-block:: typescript

        let m1 = new Map<string, string>()

Instance Methods
----------------

``length``

Returns length of map.

Signature: **length(): int**

.. code-block:: typescript

        let m1 = new Map<string, string>()
        console.println(m1.length())

``put``

Puts key with value to map.

Signature: **put(k: Char, v: Double): void**

.. code-block:: typescript

        let m1 = new Map<string, string>()
        m1.length('a', 1.2)

``clear``

Clears map.

Signature: **clear(): void**

.. code-block:: typescript

        let m1 = new Map<string, string>()
        m1.clear()

``remove``

Removes the value with a specified key.

Signature: **remove(k: Char): void**

.. code-block:: typescript

        let m1 = new Map<string, string>()
        m1.remove('a')

``entries``

Returns an array of entries.

Signature: **override entries(): Entry[]**

.. code-block:: typescript

        let m1 = new Map<string, string>()
        let arr = m1.entries()

``get``

Returns the value of a specified key.

Signature: 

* get(k: Char): Double
* get(k: Char, def: Double): Double

.. code-block:: typescript

        let m1 = new Map<string, string>()
        let arr = m1.entries()

``hasKey``

Сhecks if a specified value is in the set.

Signature: **hasKey(k: Char): boolean**

.. code-block:: typescript

        let m1 = new Map<string, string>()
        let arr = m1.hasKey()

``keys``

Returns an array of keys.

Signature: **keys(): Char[]**

.. code-block:: typescript

        let m1 = new Map<string, string>()
        let arr = m1.keys()

``set``

The set() method adds or updates an entry in a Map object with a specified
key and a value.

.. code-block:: typescript

        const map1 = new Map()
        map1.set('bar', 'foo')
        console.log(map1.get('bar'))
        // Expected output: "foo"

``values``

The values() method returns a new iterator object that contains the values
for each element in the Map object in insertion order. 

.. code-block:: typescript

        const map1 = new Map()
        map1.set('0', 'foo')
        map1.set(1, 'bar')
        const iterator1 = map1.values()
        console.log(iterator1.next().value)
        // Expected output: "foo"

