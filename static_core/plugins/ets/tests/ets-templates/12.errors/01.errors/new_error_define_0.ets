/*---
desc: Define new error from Error class.
assert: Error is the base class of all the errors. Usually there is no need to define a new error class, as error classes for various cases are defined in a standard library, like DivideByZeroError. However, a developer may define a new error using Error or any derived class as the base of new class.
tags: [compile-only]
---*/

class TestError extends Error {}