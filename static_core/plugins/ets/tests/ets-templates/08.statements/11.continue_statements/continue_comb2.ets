/*---
desc: >-
  A continue statement stops execution of the current loop iteration and passes control
  to the next iteration with proper check of the loop exit condition depending on the loop kind.
---*/

function main(): int {
    let sum: int;
    let arr: int[] = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11]
    for (let y of arr) {
        loop2: for (let x = 0; x < 20; x++) {
            let z = 0
            do {
                let t = 100
                while (t++ < 200) {
                    if (t % 3 == 0) {
                        continue;
                    }
                    sum += t
                }

                if (z % 2 == 0) {
                    continue loop2;
                }
            } while (z++ < 10);

            if (x % 2 == 0) {
                continue;
            }
        }

        if (y % 2 == 0) {
            continue;
        }
    }

    if (sum == 2222000) return 0;
    return 1;
}
