
TypedArray
==========

``TypedArray`` object represents an array-like view of an underlying binary
data buffer.
There is no ``TypedArray`` type, but a collection of types named in accordance
with different representations of the underlying binary data:


* ``Int8Array, Uint8Array, Uint8ClampedArray``\ : 8-bit signed and unsigned integers, and unsigned integers clamped to 0-255.
* ``Int16Array, Uint16Array``\ : 16-bit signed and unsigned integers.
* ``Int32Array, Uint32Array``\ : 32-bit signed and unsigned integers.
* ``Float32Array, Float64Array``\ : 32-bit and 64-bit floating-point numbers.
* ``BigInt64Array, BigUint64Array``\ : 64-bit signed and unsigned integers.

All ``TypedArray`` types and their instances have same API.

Construction
------------

Using provided types, you can easily create a ``TypedArray`` from a provided
array of values.

.. code-block:: typescript

       const array1 = new Int8Array([0, 7, 120, -35])
       const array2 = new BigInt64Array([142159609231, 60930124998])

Alternatively, you can create array with a specified length or use an
existing buffer.

.. code-block:: typescript

       const array1 = new Int8Array(8) // size provided in bytes
       const buffer = new ArrayBuffer()
       const array2 = new Uint32Array(buffer)

Static Properties
-----------------

``TypedArray.BYTES_PER_ELEMENT``

Represents the size in bytes of each element in a typed array.

.. code-block:: typescript

       console.log(Int8Array.BYTES_PER_ELEMENT)
       // expected output: 1
       console.log(Float32Array.BYTES_PER_ELEMENT)
       // expected output: 4
       console.log(BigInt64Array.BYTES_PER_ELEMENT)
       // expected output: 8

Static Methods
--------------

``TypedArray.from``

Creates a new TypedArray from an array of Object or string.

.. code-block:: typescript

       const array1 = Int8Array.from([1, 2, 3])
       const array2 = Float32Array.from("135")

       console.log(array1)
       // expected output: [1, 2, 3]
       console.log(array2)
       // expected output: ["1", "3", "5"]

``TypedArray.of``

Create a new TypedArray from a variable number of arguments.

.. code-block:: typescript

       const array1 = new Int8Array.of(1, 2, 3)
       const array2 = new BigInt64Array.of(9843912058125, 0)

       console.log(array1)
       // expected output: [1, 2, 3]
       console.log(array2)
       // expected output: [9843912058125, 0]

Instance Properties
-------------------

``TypedArray.buffer``

Returns a reference to the underlying ``ArrayBuffer``.

.. code-block:: typescript

       const buffer = new ArrayBuffer(64)
       const array = new Uint16Array(buffer)

       console.log(buffer == array.buffer) 
       // expected output: true

``TypedArray.byteLength``

Specifies the length (in bytes) of the array.

.. code-block:: typescript

       const array = new Int16Array([1, 2, 3])
       console.log(array.byteLength)
       // expected output: 6
       const array = new Float64Array([3.5, 6.2])
       console.log(array.byteLength)
       // expected output: 16

``TypedArray.byteOffset``

Returns the offset from the start of the ``ArrayBuffer``\ specified during
creation of the array.

.. code-block:: typescript

       const buffer = new ArrayBuffer(8)
       const array = new Int16Array(buffer)
       console.log(array.byteOffset)
       // expected output: 0
       const array = new Float64Array(buffer, 16)
       console.log(array.byteOffset)
       // expected output: 16

``TypedArray.length``

Specifies the length (in elements) of the array.

.. code-block:: typescript

       const array = new Int8Array([1, 2, 3])
       console.log(array.length)
       // expected output: 3
       const array = new Float64Array([3.5, 6.2])
       console.log(array.length)
       // expected output: 2

Instance Methods
----------------

``TypedArray.at``

Returns the object at the given index of the array.

.. code-block:: typescript

       const array = new Int8Array([8, -2, 127])
       console.log(array.at(0))
       // expected output: 8
       console.log(array.at(2))
       // expected output: 127

``TypedArray.copyWithin``

Copies a sequence of elements from an array and pastes them to a new location
within the same array.
This method modifies the original array and returns it. 

.. code-block:: typescript

       let array1 = new Int16Array([1, 2, 3, 4, 5])
       array1.copyWithin(0, 3)
       console.log(array1) 
       // expected output: [4, 5, 3, 4, 5]

``TypedArray.entries``

Returns an iterator that contains pairs (index, value) of the array elements.

.. code-block:: typescript

       let array = new Uint8Array([0, 127, 5, 4])
       const i = array.entries()

       console.log(i.next().value)
       // expected output: [0, 0]

       console.log(i.next().value)
       // expected output: [1, 127]

``TypedArray.every``

Returns a boolean value indicating whether every element in the array
satisfies a specified predicate.

.. code-block:: typescript

       function positive(n: byte): boolean { return n > 0 }

       const array = new Int8Array([-2, -4, -5])
       console.log(array.every(positive))
       // expected output: false

``TypedArray.fill``

Changes values of an array to a specified value according to the specified
start and end positions.
Modifies the array and returns its copy.

.. code-block:: typescript

       let array = new Int32Array([-2, -1, 0, 1, 2])
       array.fill(100, 1, 3)
       console.log(array)
       // expected output: [-2, 100, 100, 1, 2]
       array.fill(0)
       // expected output: [0, 0, 0, 0, 0]

``TypedArray.filter``

Filters an array using a specified predicate. Does not modify the array.

.. code-block:: typescript

       function positive(n: byte): boolean { return n > 0 }

       const array = new Int8Array([0, -2, 2])
       const result = array.filter(positive)
       console.log(result)
       // expected output: [2]

``TypedArray.find``

Finds the first element of the array matching a specified predicate.

.. code-block:: typescript

       function negative(n: float): boolean { return n < 0 }

       const array = new Float32Array(3.2, 6.4, -2)

       const found = array.find(negative)
       console.log(found)
       // expected output: -2

       const not_found = array.find(isNaN)
       console.log(not_found)
       // expected output: null

``TypedArray.findIndex``

Finds an index of the first element of the array matching a specified predicate.
Returns -1 if no such element present.

.. code-block:: typescript

       function lessThan42(n: float): boolean { return n <= 42 }

       const array = new Uint32Array([0, 12, 42, 101])
       const result = array.findIndex(lessThan42)
       console.log(result)
       // expected output: 0

``TypedArray.forEach``

Applies a specified predicate for each element of the array. Does not return
anything and does not modify the array.

.. code-block:: typescript

       function log(x: Object): void { console.log(x) }

       const array = new Uint32Array([1, 2, 3, 4, 5])
       numbers.forEach(log)
       // expected output: 1 2 3 4 5

``TypedArray.includes``

Returns true if the provided value exists in the array, otherwise false.

.. code-block:: typescript

       const array = new Uint32Array([1, 2, 3, 4])
       console.log(array.includes(3))
       // expected output: true

``TypedArray.indexOf``

Similar to Array.findIndex, but uses equality to test elements.

.. code-block:: typescript

       const array = new Uint8Array([0, 132, 4, 221])
       const index = array.indexOf(4)
       console.log(index)
       // expected output: 2

       const neg_index = array.indexOf(42)
       console.log(neg_index)
       // expected output: -1

``TypedArray.join``

Joins array elements with a specified separator, using toString method on
every element of array.
If no separator is provided, then the default separator "," (comma) is used.

.. code-block:: typescript

       const array1 = new Uint32Array([0, 1, 2, 3])
       const joined1 = array1.join()
       console.log(joined1)
       // expected output: "0,1,2,3"

       const array2 = new Float64Array([12, 7.25, 5.25, 5.255])
       const joined2 = array2.join("")
       console.log(joined2)
       // expected output: "127.255.255.255"

``TypedArray.keys``

Returns keys of the array (sequence from 0 to array.length - 1) as the iterator.

.. code-block:: typescript

       const array = new Int8Array(-5, 2, 10, 8)
       const i = array.keys()

       console.log(i.next().value)
       // expected output: 0

       console.log(i.next().value)
       // expected output: 1

       console.log(i.next().value)
       // expected output: 2

``TypedArray.lastIndexOf``

Searches the array backwards starting from a provided fromIndex, and an index
of found elements is returned.
Returns -1 if no element is found.

.. code-block:: typescript

       const array = new Int8Array([-1, -2, -3, 0, 0, 1])
       const index = array.lastIndexOf(0)
       console.log(index)
       // expected output: 4

       const neg_index = array.lastIndexOf(105)
       console.log(neg_index)
       // expected output: -1

``TypedArray.map``

Creates a new array containing results of application of a provided function
to each element of array.

.. code-block:: typescript

       function twoMul(x: ubyte): ubyte { return 2 * x }

       const array = new Uint8Array([1, 2, 3, 4])
       const powers_of_two = array.map(twoMul)
       console.log(powers_of_two)
       // expected output: [2, 4, 6, 8]

``TypedArray.reduce``

Iterates over the elements of the array and uses the provided function to
accumulate the result.
A function takes two arguments: accumulator and current element.
First invocation of the provided function gets the provided initial value
as the accumulator.
For example, TypedArray.reduce can be used to sum up the elements or apply
another binary operator.

.. code-block:: typescript

       function sum(acc: int, cur: int): int { return acc + cur }

       const array = new Int32Array([100500, 81235, 400, -1502, 0]) // notice the zero in the end
       const initial = 1000
       const result = array.reduce(sum, initial)

       console.log(result)
       // expected output: 0

``TypedArray.reduceRight``

Same as ``TypedArray.reduce``\ , except the iteration starts from the end of
the array.

.. code-block:: typescript

       function sum(acc: ubyte, cur: ubyte): ubyte { return acc + cur }

       const array = new Uint8Array([5, 10, 15, 20, 30])
       const initial = 0
       const result = array.reduceRight(sum, initial)

       console.log(result)
       // expected output: 80

``TypedArray.reverse``

Changes the order of elements in the array so that the last element becomes
the first and vice versa.
Modifies the array in place and returns the reference to the same array.

.. code-block:: typescript

       let array = new Array(1, 2, 3, 4, 5)
       const world = array.reverse()

       console.log(array)
       // expected output: [5, 4, 3, 2, 1]
       console.log(world)
       // expected output: [5, 4, 3, 2, 1]

``TypedArray.set``

Writes (and replaces) the values in ``TypedArray`` using the values from
specified array, starting from the specified index.

.. code-block:: typescript

       const buffer = new ArrayBuffer(8)
       const array = new Uint16Array(buffer)

       array.set([1, 2, 3], 1)
       console.log(array)
       // expected output: [0, 1, 2, 3]

``TypedArray.slice``

Returns new `TypedArray` containing a portion of the array (from start index
to end index, end index not included).
Starts from the beginning if no start index is provided.
End becomes the index of the last element if no end index is provided.
The method accepts negative indices, in which case the index counts backwards
from the end of the array.

.. code-block:: typescript

       const array = new Uint8Array([1, 2, 3, 4, 5])
       console.log(array.slice(2))
       // expected output: [3, 4, 5]
       console.log(array.slice(-2))
       // expected output: [4, 5]
       console.log(array.slice())
       // expected output: [1, 2, 3, 4, 5]

``TypedArray.some``

Tests whether an element of the array satisfies the provided predicate
function. If so, returns true. Otherwise, false.

.. code-block:: typescript

       function even(x: float): boolean { return x % 2 == 0 }

       const array = new Float32Array([0.1, 5.3, NaN, -8.3])
       console.log(array.some(even))
       // expected output: false
       console.log(array.some(isNaN))
       // expected output: true

``TypedArray.sort``

Sorts the elements of the array in place and returns the reference sorted to
the same array.
By default the order is ascending.
The elements are sorted numerically if no function is provided.

.. code-block:: typescript

       const array = new Uint8Array([1, 50, 2, 3, 11])
       console.log(array.sort())
       // expected output: [1, 2, 3, 11, 50]

``TypedArray.subarray``

Returns an array of the same type that references the same underlying
``ArrayBuffer`` as the original array.
Start and end indices can be specified, start being inclusive, and end
exclusive.

.. code-block:: typescript

       const array = new Uint16Array([500, 1000, 10000, 42, 78])

       console.log(array.subarray())
       // expected output: [500, 1000, 10000, 42, 78]
       console.log(array.subarray(1, 2))
       // expected output: [1000]
       console.log(array.subarray(2))
       // expected output: [10000, 42, 78]

``TypedArray.values``

Returns an iterator to the array elements.

.. code-block:: typescript

       const array = new Uint8Array([255, 127, 0])
       const i = array.values()

       console.log(i.next().value)
       // expected output: 255

       console.log(i.next().value)
       // expected output: 127
