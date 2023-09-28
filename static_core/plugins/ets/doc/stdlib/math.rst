Math API
================

Constants
--------------
**PI: double = 3.141592653589793**

**e: double = 2.718281828459045**

Static Methods
--------------

``abs``

Returns an absolute value.

.. code-block:: typescript

        console.println(abs(-4))
        // expected output 4

* If the argument is NaN, returns NaN.
* If the argument is inf or -inf, returns inf.

``acosh``

Hyperbolic arccosine of an angle.

.. code-block:: typescript

        console.println(acosh(1))
        // expected output 0

* If the argument is NaN or -inf, returns NaN.
* If the argument is inf, returns inf.

``acos``

Arccosine of an angle.

.. code-block:: typescript

        console.println(acos(1))
        // expected output 0

* If the argument is NaN, inf or -inf, returns NaN.

``asinh``

Hyperbolic arcsine of angle.

.. code-block:: typescript

        console.println(asinh(0))
        // expected output 0

* If the argument is NaN, returns NaN.
* If the argument is inf, returns inf.
* If the argument is -inf, returns -inf.

``asin``

Arcsine of an angle.

.. code-block:: typescript

        console.println(asin(0))
        // expected output 0

**atan2**

The angle in radians (between -π and π, inclusive) between the positive
x-axis and the ray from (0, 0) to the point (x, y).
The atan2() method measures the counterclockwise angle θ in radians between
the positive x-axis and the point (x, y).
Note that the arguments to this function pass the y-coordinate first and the
x-coordinate second.

.. code-block:: typescript

        console.println(abs(-4))
        // expected output 4

``atanh``

Hyperbolic arctangent of an angle.

.. code-block:: typescript

        console.println(atanh(0))
        // expected output 0

* If the argument is NaN, inf or -inf returns NaN.

``atan``

Arctangent of an angle.

.. code-block:: typescript

        console.println(atan(0))
        // expected output 0

* If the argument is NaN, returns NaN.
* If the argument is inf, returns Pi / 2.
* If the argument is -inf, returns -Pi / 2.

``cbrt``

Returns the cube root of a number.

.. code-block:: typescript

        console.println(cbrt(1))
        // expected output 1

``ceil``

Smallest integer greater or equal to argument.

.. code-block:: typescript

        console.println(ceil(2.5))
        // expected output 3

* If the argument is NaN, returns NaN.
* If the argument is inf, returns inf.
* If the argument is -inf, returns -inf.

``clz64``

Leading zero bits count in 64-bit representation of an argument.

.. code-block:: typescript

        console.println(clz64(0xFFFFFFFFFFFFFFFF))
        // expected output 0
        console.println(clz64(0x0000FFFFFFFFFFFF))
        // expected output 16
        console.println(clz64(0x0))
        // expected output 64

``clz32``

Leading zero bits count in 32-bit representation of an argument.

.. code-block:: typescript

        console.println(clz32(0xFFFFFFFF))
        // expected output 0
        console.println(clz32(0x0000FFFF))
        // expected output 16
        console.println(clz32(0x0))
        // expected output 32

``cosh``

Hyperbolic cosine of an angle.

.. code-block:: typescript

        console.println(cosh(0))
        // expected output 1

* If the argument is NaN, returns NaN.
* If the argument is inf, returns inf.
* If the argument is -inf, returns inf.

``cos``

Cosine of an argument.

        console.println(cos(0))
        // expected output 1

* If the argument is NaN, returns NaN.
* If the argument is inf, returns NaN.
* If the argument is -inf, returns NaN.

``power2``

2 raised to power argument.

.. code-block:: typescript

        console.println(power2(4))
        // expected output 16

``expm1``

(e raised to power `v`) - 1.

.. code-block:: typescript

        console.println(expm1(0))
        // expected output 0

``exp``

e raised to power argument.

.. code-block:: typescript

        console.println(exp(0))
        // expected output 1

``floor``

The largest integer less or equal to an argument.

Signature: **hypot(u: double, v: double): double**

.. code-block:: typescript

        console.println(floor(2.5))
        // expected output 2

* If the argument is NaN, returns NaN.
* If the argument is inf, returns inf.
* If the argument is -inf, returns -inf.

``log10``

Base 10 logarithm of an argument.

Signature: **log10(v: double): double**

.. code-block:: typescript

        console.println(log10(100))
        // expected output 2

* If the argument is NaN, returns NaN.
* If the argument is inf, returns inf.
* If the argument is -inf, returns NaN.

``log1p``

Natural logarithm of (1 + argument).

Signature: **log1p(v: double): double**

.. code-block:: typescript

        console.println(log10(100))
        // expected output 2

* If the argument is NaN, returns NaN.
* If the argument is inf, returns inf.
* If the argument is -inf, returns NaN.

``max``

The largest value of arg1 and arg2.

Signature:
* max(v: double, u: double): double
* max(v: int, u: int): int

.. code-block:: typescript

        console.println(max(2, 4))
        // expected output 4

``min``

The smallest value of arg1 and arg2.

Signature: **min(v: double, u: double): double**

.. code-block:: typescript

        console.println(min(2, 4))
        // expected output 2

``power``

arg1 raised to power of arg2.

Signature: **function power(v: double, u: double): double**

.. code-block:: typescript

        console.println(power(2, 4))
        // expected output 16
        console.println(power(doubleNaN, doubleInf))
        // expected output nan
        console.println(power(doubleInf, doubleInf))
        // expected output inf

``sign``

Sign of an argument.

Signature: **sign(v: double): int**

.. code-block:: typescript

        console.println(sign(1.5))
        // expected output 1

``sinh``

Hyperbolic sine of an argument.

Signature: **sinh(v: double): double**

.. code-block:: typescript

        console.println(sinh(0))
        // expected output 0

* If the argument is NaN, returns NaN.
* If the argument is inf, returns NaN.
* If the argument is -inf, returns NaN.

``sin``

Sine of an argument.

Signature: **sin(v: double): double**

.. code-block:: typescript

        console.println(sin(PI / 2));
        // expected output 1

* If the argument is NaN, returns NaN.
* If the argument is inf, returns NaN.
* If the argument is -inf, returns NaN.

``sqrt``

Square root of an argument.

Signature: **sqrt(v: double): double**

.. code-block:: typescript

        console.println(sqrt(4));
        // expected output 2

* If the argument is NaN, returns NaN.
* If the argument is inf, returns inf.
* If the argument is less than zero, returns NaN.

``tanh``

Hyperbolic tangent of an argument.

Signature: **tanh(v: double): double**

.. code-block:: typescript

        console.println(tanh(0));
        // expected output 0

``tan``

Tangent of an argument.

Signature: **tan(v: double): double**

.. code-block:: typescript

        console.println(tan(PI / 4));
        // expected output 1

``trunc``

Integer part of an argument.

Signature: **trunc(v: double): double**

.. code-block:: typescript

        console.println(trunc(PI));
        // expected output 3

``random``

Pseudo-random number in the range [0.0, 1.0).

Signature: **random(): double**

.. code-block:: typescript

        console.println(random(PI));
        // expected output random number in the range [0.0, 1.0)

``log2``

Base 2 logarithm of an argument.

Signature: **log2(v: double): double**

.. code-block:: typescript

        console.println(log2(16));
        // expected output 4

``log10``

Base 10 logarithm of an argument.

Signature: 
* log10(v: double): double
* log10(i: int): int

.. code-block:: typescript

        console.println(log10(100));
        // expected output 2

