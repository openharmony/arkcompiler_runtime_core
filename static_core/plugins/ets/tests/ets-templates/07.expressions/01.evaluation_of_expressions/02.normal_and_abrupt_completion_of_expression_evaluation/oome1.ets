/*---
desc: OutOfMemoryError is thrown by an assignment to an array component of
    reference type, a method call expression, or a prefix / postfix
    increment / decrement operator as a result of boxing conversion.
ark_options: ['--heap-size-limit=33554432', '--init-heap-size-limit=33554432']
timeout: 60
---*/

const ARRAY_SIZE = 1_000_000;

function main(): int {
  try {
    let v: Object[] = new Object[ARRAY_SIZE];

    for (let i: int = 0; i < ARRAY_SIZE; i++) {
        let w = new Double[ARRAY_SIZE];

        for (let j: int = 0; j < ARRAY_SIZE; j++) {
          w[j] = new Double();
        }

        v[i] = w as Object;
    }
    return 1;
  } catch (e: OutOfMemoryError) {
    return 0;
  } catch (e) {
    return 1;
  }
  return 1;
}
