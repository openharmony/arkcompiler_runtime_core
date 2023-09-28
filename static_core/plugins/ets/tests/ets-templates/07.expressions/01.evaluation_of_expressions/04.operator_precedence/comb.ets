/*---
desc: combined multiple operations
---*/

function main(): int {
    let t1 = 1, t2 = 2, t3 = 1, t4 = 2, t5 = 1, t6 = 2;

    if (++t1 + t2++ > t3 % t1 * t6 + t4 | ++t2 / t6-- + ~t1 > t3-- | t3 <= t5 && t3 << t6 / t4 > -t1) ; else return 1;
    if (t6 + t5 + t4 * ~t3 % ++t1 - t6 != t1) ; else return 1;

    return 0;
}
