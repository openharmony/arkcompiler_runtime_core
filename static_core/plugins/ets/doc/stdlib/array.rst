
Array
=====

Array class used to store collection of objects of the same type.

Construction
------------

You can create an array by providing elements in the array constructor.

.. code-block:: typescript

       let array = new Array(1, 2, 3)

Static Methods
--------------

``Array.from``

Creates a new Array from an array of Object or string.

.. code-block:: typescript

       const array1 = Array.from([1, 2, 3])
       const array2 = Array.from("abc")
       const array3 = Array.from([3.2, 4.3, 9.1])

       console.log(array1)
       // expected output: [1, 2, 3]
       console.log(array2)
       // expected output: ["a", "b", "c"]
       console.log(array3)
       // expected output: [3.2, 4.3, 9.1]

``Array.of``

Creates a new Array from variable number of arguments.

.. code-block:: typescript

       const array1 = Array.of(1, 2, 3)
       const array2 = Array.of("foo", "bar")

       console.log(array1)
       // expected output: [1, 2, 3]
       console.log(array2)
       // expected output: ["foo", "bar"]

Instance Properties
-------------------

``Array.length``

Specifies the length of the array.

.. code-block:: typescript

       const array = new Array(1, 2, 3)
       console.log(array.length)
       // expected output: 3

Instance Methods
----------------

``Array.at``

Returns the object at the given index of the array.

.. code-block:: typescript

       const array = new Array(8, -2, 12)
       console.log(array.at(0))
       // expected output: 8
       console.log(array.at(2))
       // expected output: 12

``Array.concat``

Returns the concatenation of two arrays as a new array. Does not modify
the original array.

.. code-block:: typescript

       const array1 = new Array("hello", "this")
       const array2 = new Array("beautiful", "world")
       const array3 = array1.concat(array2)
       console.log(array3)
       // expected output: ["hello", "this", "beautiful", "world"]

``Array.copyWithin``

Copies a sequence of elements from an array and pastes them to a new location
within the same array.
This method modifies the original array and returns it. 

.. code-block:: typescript

       let array1 = new Array(1, 2, 3, 4, 5)
       array1.copyWithin(0, 3)
       console.log(array1) 
       // expected output: [4, 5, 3, 4, 5]

``Array.entries``

Returns an iterator that contains pairs (index, value) of the array elements.

.. code-block:: typescript

       let array = new Array("one", "two", "three", "four")
       const i = array.entries()

       console.log(i.next().value)
       // expected output: [0, "one"]

       console.log(i.next().value)
       // expected output: [1, "two"]

``Array.every``

Returns a boolean value indicating whether every element in the array satisfies
passed condition.

.. code-block:: typescript

       function lessThan10(n : int): boolean { return n < 10 }

       const array = new Array(1, 2, 3, 4, 5)
       console.log(array.every(lessThan10))
       // expected output: true

``Array.fill``

Changes values of an array to specified value according to the specified start
and end positions.
Modifies the array and returns it.

.. code-block:: typescript

       let array = new Array(-2, -1, 0, 1, 2)
       array.fill(100, 1, 3)
       console.log(array)
       // expected output: [-2, 100, 100, 1, 2]
       array.fill(0)
       // expected output: [0, 0, 0, 0, 0]

``Array.filter``

Filters an array using the specified predicate function. Does not modify the
array.

.. code-block:: typescript

       function notRandom(x : string): boolean { return x != "random" }

       const array = new Array("some", "random", "words")
       const result = array.filter(notRandom)
       console.log(result)
       // expected output: ["some", "words"]

``Array.find``

Finds the first element of the array matching the specified predicate. Returns
``null`` if such element is not found.

.. code-block:: typescript

       function is80(x : int): boolean { return x == 80 }
       function is200(x : int): boolean { return x == 200 }

       const array = new Array(-42, 80, 94)

       const found = array.find(is80)
       console.log(found)
       // expected output: 80

       const not_found = array.find(is200)
       console.log(not_found)
       // expected output: null

``Array.findIndex``

Finds an index of the first element of the array matching the specified
predicate.
Returns -1 if no such element present.

.. code-block:: typescript

       function isExclamation(x : string): boolean { return x == "!" }

       const array = new Array("hello", "world", "!")
       const result = array.findIndex(isExclamation)
       console.log(result)
       // expected output: 2

``Array.flat``

Returns a new array as a result of concatenating all nested arrays. A specified
parameter used to indicate the depth to which concatenation is performed.

.. code-block:: typescript

       const array1 = new Array(0, 2, [3, 5, [0, 12]])
       const result1 = array1.flat()
       console.log(result1)
       // expected output: [0, 2, 3, 5, 0, 12]

       const array2 = new Array("def", new Array("sq", "x"), new Array("body", new Array("*", "x", "x")))
       const result2 = array.flat(2)
       console.log(result2)
       // expected output: ["def", "sq", "x", "body", "*", "x", "x"]

``Array.flatMap``

Behaves similarly to ``Array.map``\ but flattens the resultant array by one
level.

.. code-block:: typescript

       function id(x: Object): Object { return x }

       const array = new Array(0, 2, new Array(3, 5, new Array(0, 12)))
       const result = array.flatMap(id)
       console.log(result)
       // expected output: [0, 2, 3, 5, [0, 12]]

``Array.forEach``

Applies a specified function to each element of the array. Does not return
anything and does not modify the array.
Does not prohibit the callback function to modify the array elements.

.. code-block:: typescript

       function log(x: Object): void { console.log(x) }

       const array = new Array(1, 2, 3, 4, 5)
       numbers.forEach(log)
       // expected output: 1 2 3 4 5

``Array.includes``

Returns true if the provided value exists in the array, otherwise false.

.. code-block:: typescript

       const array = new Array("some", "strings")
       console.log(array.includes("hello"))
       // expected output: false

``Array.indexOf``

Similar to ``Array.findIndex``, but uses equality to test elements.

.. code-block:: typescript

       const array = new Array("here", "are", "some", "words")
       const index = array.indexOf("are")
       console.log(index)
       // expected output: 1

       const neg_index = array.indexOf("hello")
       console.log(neg_index)
       // expected output: -1

``Array.join``

Joins array elements with a specified separator using ``toString`` method on every element of array.
The default separator "," (comma) is used if no separator is provided.

.. code-block:: typescript

       const array1 = new Array("some", "body")
       const joined1 = array1.join("...")
       console.log(joined1)
       // expected output: "some...body"

       const array2 = new Array(1, 2, 3, 4)
       const joined2 = array2.join("")
       console.log(joined2)
       // expected output: 1234

``Array.keys``

Returns keys of the array (sequence from 0 to array.length - 1) as an iterator.

.. code-block:: typescript

       const array = new Array(-5, 2, 10, 8)
       const i = array.keys()

       console.log(i.next().value)
       // expected output: 0

       console.log(i.next().value)
       // expected output: 1

       console.log(i.next().value)
       // expected output: 2

``Array.lastIndexOf``

The array is searched backwards starting from the provided ``fromIndex``, and
the index of found elements is returned.
Returns -1 if no element is found.

.. code-block:: typescript

       const array = new Array("a", "b", "c", "c", "d")
       const index = array.indexOf("c")
       console.log(index)
       // expected output: 3

       const neg_index = array.indexOf("z")
       console.log(neg_index)
       // expected output: -1

``Array.map``

Creates a new array containing the results of the provided function application
to each element of array.

.. code-block:: typescript

       function twoMul(x: int): int { return 2 * x }

       const array = new Array(1, 2, 3, 4)
       const powers_of_two = array.map(twoMul)
       console.log(powers_of_two)
       // expected output: [2, 4, 6, 8]

``Array.pop``

Removes the last element from an array and returns it. Changes the array length.

.. code-block:: typescript

       let array = new Array(1, 2, 3, 4)
       const last = array.pop()

       console.log(last)
       // expected output: 4
       console.log(array)
       // expected output: [1, 2, 3]

``Array.push``

Adds an element to the end of the array. Returns the new length of the array.

.. code-block:: typescript

       let array = new Array(0, 0, 2, 2)
       const element = 8
       const new_len = array.push(element)

       console.log(new_len) 
       // expected output: 5
       console.log(array)
       // expected output: [0, 0, 2, 2, 8]

``Array.reduce``

Iterates over the elements of the array (from the start) and uses the provided
function to accumulate the result.
The function takes two arguments: accumulator and current element.
The first invocation of the provided function gets the provided initial value
as the accumulator.
For example, ``Array.reduce`` can be used to sum up or concatenate the elements.

.. code-block:: typescript

       function concat(acc: string, cur: string): string { return acc + cur }

       const array = new Array("h", "e", "l", "l", "o")
       const initial = ""
       const hello = array.reduce(concat, initial)

       console.log(hello)
       // expected output: "hello"

``Array.reduceRight``

Same as ``Array.reduce``\ , except iteration starts from the end of the array.

.. code-block:: typescript

       function concat(acc: string, cur: string): string { return acc + cur }

       const array = new Array("d", "l", "r", "o", "w")
       const initial = "hello "
       const hello_world = array.reduceRight(concat, initial)

       console.log(hello_world)
       // expected output: "hello world"

``Array.reverse``

Changes the order of elements in the array so that the last element becomes the
first and vice versa.
Modifies the array in place and returns the reference to the same array.

.. code-block:: typescript

       let array = new Array("d", "l", "r", "o", "w")
       const world = array.reverse()

       console.log(array)
       // expected output: ["w", "o", "r", "l", "d"]
       console.log(world)
       // expected output: ["w", "o", "r", "l", "d"]

``Array.shift``

Removes the first element from the array and returns it. Changes the array
length.

.. code-block:: typescript

       let array = new Array(1, 2, 3)
       const one = array.shift()

       console.log(array)
       // expected output: [2, 3]
       console.log(one)
       // expected output: 1

``Array.slice``

Returns a view to a portion of the array (from start index to end index, end
index not included).
Starts from the beginning if no start index is provided.
The end becomes index of the last element if no end index is provided.
The method accepts negative indices, in which case the index counts backwards
starting from the end of the array.

.. code-block:: typescript

       const array = new Array(1, 2, 3, 4, 5)
       console.log(array.slice(2))
       // expected output: [3, 4, 5]
       console.log(array.slice(-2))
       // expected output: [4, 5]
       console.log(array.slice())
       // expected output: [1, 2, 3, 4, 5]

``Array.some``

Tests whether any element of the array satisfies the provided predicate
function. If so, returns true, otherwise false.

.. code-block:: typescript

       function even(x: int): boolean { return x % 2 == 0 }
       function greater500(x: int): boolean { return x > 500 }

       const array = new Array(1, 2, 3, 5, 7)
       console.log(array.some(even))
       // expected output: true
       console.log(array.some(greater500))
       // expected output: false

``Array.sort``

Sorts the elements of the array in place and returns the sorted reference to
the same array. The default order is ascending.
The elements are sorted as their string representations if no function is
provided.

.. code-block:: typescript

       function compare(a: string, b: string): boolean { return lhs.length < rhs.length }

       const array = new Array(1, 50, 2, 3, 11)
       console.log(array.sort())
       // expected output: [1, 11, 2, 3, 50]

       const array2 = new Array("this", "strings", "are", "sorted")
       console.log(array2.sort(compare))
       // expected output: ["are", "this", "sorted", "strings"]

``Array.splice``

Changes the contents of the array replacing/removing elements from a specified
index.
Modifies the array in place and returns the reference to the same array.

.. code-block:: typescript

       let array = new Array("one", "two", "four", "five")

       // add "three" after the second element
       array.splice(1, 0, "three")
       console.log(array)
       // expected output: ["one", "two", "three", "four", "five"]

       // replace first two elements with "zero"
       array.splice(0, 2, "zero")
       console.log(array)
       // expected output: ["zero", "three", "four", "five"]

``Array.unshift``

Adds elements to the beginning of the array.

.. code-block:: typescript

       let array = new Array(4, 5, 6)
       console.log(array.unshift(0, 1, 2, 3))
       // expected output: [0, 1, 2, 3, 4, 5, 6]

``Array.values``

Returns an iterator to array elements.

.. code-block:: typescript

       const array = new Array("next", "element")
       const i = array.values()

       console.log(i.next().value)
       // expected output: "next"

       console.log(i.next().value)
       // expected output: "element"

