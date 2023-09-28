/*---
desc: For each Ti (1 ≤ i ≤ n), let Ci be the most specific class or array type such that Ti < Ci . Then there must be some Ck such that Ck < Ci for any i (1 ≤ i ≤ n), or a compile-time error occurs.
---*/

class A {}
final class B extends A {}

interface C {}
interface D extends C {}

final class F<T extends A, G extends C> {}

function main(): void {
  let instance = new F<B, D>();
}
