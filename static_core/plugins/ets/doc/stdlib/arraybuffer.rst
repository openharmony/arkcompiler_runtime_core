
ArrayBuffer
===========

Array buffer represents an untyped array of raw bytes.

Construction
------------

You can create an ``ArrayBuffer`` with specified length in bytes

.. code-block:: typescript

       const buffer = new ArrayBuffer(64)

       console.log(buffer.byteLength)
       // expected output: 64

Instance Properties
-------------------

``ArrayBuffer.byteLength``

Returns the length of the array in bytes.

.. code-block:: typescript

       const buffer = new ArrayBuffer(3)

       console.log(buffer.byteLength)
       // expected output: 3

Instance Methods
----------------

``ArrayBuffer.slice``

Returns a new ``ArrayBuffer`` that contains the portion of the original buffer,
from start inclusive up to end exclusive.

.. code-block:: typescript

       const buffer = new ArrayBuffer(16)
       const int16_view = new Int16Array(buffer)

       int16_view[3] = 1024
       console.log(int16_view)
       // expected output: [0, 0, 0, 1024, 0, 0, 0, 0]

       const first_half = buffer.slice(0, 8)
       const new_view = new Int16Array(first_half)
       console.log(new_view)
       // expected output: [0, 0, 0, 1024]


