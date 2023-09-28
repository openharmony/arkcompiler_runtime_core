/*---
desc: declaration generic fumction
tags: [compile-only]
---*/

function foo<T, E>(x: T, y: E): E {
  return y;
};
