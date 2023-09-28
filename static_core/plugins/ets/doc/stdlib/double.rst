
Double
========

Constants
------------

**NaN: double = 0.0 / 0.0**

**EPSILON: double = 2.7755575615628914e-17**

**MAX_SAFE_INTEGER: long = 9007199254740991**

**MIN_SAFE_INTEGER: long = -9007199254740991**

**POSITIVE_INFINITY: double = 1.0 / 0.0**

**NEGATIVE_INFINITY: double = -1.0 / 0.0;**

**MIN_VALUE: double = 4.9e-324**

**MAX_VALUE: double = 1.7976931348623157e+308**

Construction
------------

Signature:

* constructor()
* constructor(value: double)
* constructor(value: Double)

.. code-block:: typescript

        let doub: Double = new Double();
        let doub2: Double = new Double(2.0);
        let doub3: Double = new Double(doub2);

Static Methods
--------------

``isFinite``

Checks if double is a floating-point value (not a NaN, i.e., not a number, or
infinity).

Signature: **isFinite(v: double): boolean**

.. code-block:: typescript

        console.println(Double.isFinite(1.0));
        // expected output true

``isInteger``

Checks if double is similar to an integer value.

Signature: **isInteger(v: double): boolean**

.. code-block:: typescript

        console.println(Double.isInteger(1.0));
        // expected output true

``isNaN``

Checks if double is NaN.

Signature: **isNaN(v: double): boolean**

.. code-block:: typescript

        console.println(Double.isNaN(1.0));
        // expected output false

Instance Methods
----------------

``isFinite``

Checks if double is a floating-point value (not a NaN or infinity).

Signature: **isFinite(): boolean**

.. code-block:: typescript

        let doub : Double = new Double(1.0);
        console.println(doub.isFinite());
        // expected output true

``isInteger``

Checks if the underlying double is similar to an integer value.

Signature: **isInteger(): boolean**

.. code-block:: typescript

        let doub : Double = new Double(1.0);
        console.println(doub.isInteger());
        // expected output true

``isNaN``

Checks if the underlying double is NaN (not a number).

Signature: **public isNaN(): boolean**

.. code-block:: typescript

        let doub : Double = new Double(1.0);
        console.println(doub.isNaN());
        // expected output false


``isSafeInteger``

The ``Number.isSafeInteger()`` static method determines whether the provided
value is a number that is a safe integer.

.. code-block:: typescript

        function warn(x) {
          if (Number.isSafeInteger(x)) {
            return 'Precision safe.';
          }
          return 'Precision may be lost!';
        }
        console.log(warn(Math.pow(2, 53)));
        // Expected output: "Precision may be lost!"
        console.log(warn(Math.pow(2, 53) - 1));
        // Expected output: "Precision safe."

``parseFloat``

The ``Number.parseFloat()`` static method parses an argument and returns a
floating-point number. The method returns NaN if a number cannot be parsed
from the argument.

.. code-block:: typescript

        function circumference(r) {
          if (Number.isNaN(Number.parseFloat(r))) {
            return 0;
          }
          return parseFloat(r) * 2.0 * Math.PI ;
        }
        console.log(circumference('4.567abcdefgh'));
        // Expected output: 28.695307297889173
        console.log(circumference('abcdefgh'));
        // Expected output: 0

``parseInt``

The ``Number.parseInt()`` static method parses a string argument and returns
an integer of the specified radix or base. 

.. code-block:: typescript

        function roughScale(x, base) {
          const parsed = Number.parseInt(x, base);
          if (Number.isNaN(parsed)) {
            return 0;
          }
          return parsed * 100;
        }
        console.log(roughScale(' 0xF', 16));
        // Expected output: 1500
        console.log(roughScale('321', 2));
        // Expected output: 0

``toPrecision``

The ``toPrecision()`` method returns a string representing the Number object
to the specified precision. 

.. code-block:: typescript

        function precise(x) {
          return x.toPrecision(4);
        }
        console.log(precise(123.456));
        // Expected output: "123.5"
        console.log(precise(0.004));
        // Expected output: "0.004000"
        console.log(precise(1.23e5));
        // Expected output: "1.230e+5"


``toFixed``

The ``toFixed()`` method formats a number using fixed-point notation.

.. code-block:: typescript

        function financial(x) {
          return Number.parseFloat(x).toFixed(2);
        }
        console.log(financial(123.456));
        // Expected output: "123.46"
        console.log(financial(0.004));
        // Expected output: "0.00"
        console.log(financial('1.23e+5'));
        // Expected output: "123000.00"

``toExponential``

The ``toExponential()`` method returns a string representing the Double object
in exponential notation. 

.. code-block:: typescript

        function expo(x, f) {
          return Number.parseFloat(x).toExponential(f);
        }
        console.log(expo(123456, 2));
        // Expected output: "1.23e+5"
        console.log(expo('123456'));
        // Expected output: "1.23456e+5"
        console.log(expo('oink'));
        // Expected output: "NaN"
 
