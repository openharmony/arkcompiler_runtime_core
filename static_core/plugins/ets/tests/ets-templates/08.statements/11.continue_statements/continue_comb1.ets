/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int;
    for (let y: int = 0; y < 10; y++) {
        let x = 0;
        while (x++ < 10) {
            let z = 0;
            do {
                if (z % 2 == 0) {
                    continue;
                }
                sum += z;
            } while (z++ < 10);

            if (x % 2 == 0) {
                continue;
            }
        }

        if (y % 2 == 0) {
            continue;
        }
    }

    if (sum == 2500) return 0;
    return 1;
}
