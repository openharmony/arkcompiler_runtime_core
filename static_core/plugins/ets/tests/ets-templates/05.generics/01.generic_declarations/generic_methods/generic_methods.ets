/*---
desc: declaration generic fumction
tags: [compile-only]
---*/

class A {
  foo<T, E>(x: T, y: E): E {
    return y;
  };
}