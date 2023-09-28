
DataView
========

``DataView`` provides a way to write and read data to a ``ArrayBuffer``
regardless of platform's endianness.

Construction
------------

Can be constructed using ``ArrayBuffer``. Additionally, offset and length
can be specified.

.. code-block:: typescript

       const buffer = new ArrayBuffer(8)
       const view1 = new DataView(buffer)
       const view2 = new DataView(buffer, 4, 4)
       view1.setInt32(0, 42)
       view2.setInt32(0, 101)

       console.log(view1.getInt32(0))
       // expected output: 42
       console.log(view2.getInt32(0))
       // expected output: 101

Instance Properties
-------------------

``DataView.buffer``

Returns a reference to the underlying buffer that was used at the construction
time.

.. code-block:: typescript

       const buffer = new ArrayBuffer(4)
       const view = new DataView(buffer)

       console.log(view.buffer == buffer)
       // expected output: true

``DataView.byteLength``

Returns the length of the view in bytes.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(8))

       console.log(view.byteLength)
       // expected output: 8

``DataView.byteOffset``

Returns the offset from the start of the underlying buffer that was used at
the construction time.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(16), 8, 2)
       console.log(view.byteOffset)
       // expected output: 8

Instance Methods
----------------

``DataView.getBigInt64``

Returns the 64-bit signed integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setBigInt64(0, -86924014951n)
       console.log(view.getBigInt64(0))
       // expected output: -86924014951n

``DataView.getBigUint64``

Returns the 64-bit unsigned integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setBigUint64(0, 59325146234n)
       console.log(view.getBigUint64(0))
       // expected output: 59325146234n

``DataView.getFloat32``

Returns the 32-bit floating point number at the specified offset from the
beginning of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setFloat32(1, 3e6)
       console.log(view.getFloat32(1))
       // expected output: 3000000

``DataView.getFloat64``

Returns the 64-bit floating point number at the specified offset from the
beginning of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setFloat64(2, Math.E)
       console.log(view.getFloat64(2))
       // expected output: 2.718281828459045

``DataView.getInt16``

Returns the 16-bit signed integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setInt16(0, 32700)
       console.log(view.getInt16(0))
       // expected output: 32700

``DataView.getInt32``

Returns the 32-bit signed integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setInt32(0, -500)
       console.log(view.getInt32(0))
       // expected output: -500

``DataView.getInt8``

Returns the 8-bit signed integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setInt8(0, 127)
       console.log(view.getInt8(0))
       // expected output: 127

``DataView.getUint16``

Returns the 16-bit unsigned integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setUint16(0, 65535)
       console.log(view.getUint16(0))
       // expected output: 65535

``DataView.getUint32``

Returns the 32-bit unsigned integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setUint32(0, 123456)
       console.log(view.getUint32(0))
       // expected output: 123456

``DataView.getUint8``

Returns the 8-bit unsigned integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setUint8(0, 255)
       console.log(view.getUint8(0))
       // expected output: 255

``DataView.setBigInt64``

Stores the 64-bit signed integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setBigInt64(0, -86924014951n)
       console.log(view.getBigInt64(0))
       // expected output: -86924014951n

``DataView.setBigUint64``

Stores the 64-bit unsigned integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setBigUint64(0, 59325146234n)
       console.log(view.getBigUint64(0))
       // expected output: 59325146234n

``DataView.setFloat32``

Stores the 32-bit floating-point number at the specified offset from the
beginning of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setFloat32(1, 3e6)
       console.log(view.getFloat32(1))
       // expected output: 3000000

``DataView.setFloat64``

Stores the 64-bit floating-point number at the specified offset from the
beginning of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setFloat64(2, Math.E)
       console.log(view.getFloat64(2))
       // expected output: 2.718281828459045

``DataView.setInt16``

Stores the 16-bit signed integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setInt16(0, 32700)
       console.log(view.getInt16(0))
       // expected output: 32700

``DataView.setInt32``

Stores the 32-bit signed integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setInt32(0, -500)
       console.log(view.getInt32(0))
       // expected output: -500

``DataView.setInt8``

Stores the 8-bit signed integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setInt8(0, 127)
       console.log(view.getInt8(0))
       // expected output: 127

``DataView.setUint16``

Stores the 16-bit unsigned integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setUint16(0, 65535)
       console.log(view.getUint16(0))
       // expected output: 65535

``DataView.setUint32``

Stores the 32-bit unsigned integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setUint32(0, 123456)
       console.log(view.getUint32(0))
       // expected output: 123456

``DataView.setUint8``

Stores the 8-bit unsigned integer at the specified offset from the beginning
of the view.

.. code-block:: typescript

       const view = new DataView(new ArrayBuffer(64))
       view.setUint8(0, 255)
       console.log(view.getUint8(0))
       // expected output: 255

