/*---
desc: ternary operator with other lower precedence operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255  = 255

    // assignment
    res = t2 += t2 < t3 ? t4 : t8
    if (res != 6) return 1; else t2 = 2;

    res = t2 -= t2 < t3 ? t4 : t8
    if (res != -2) return 1; else t2 = 2;

    res = t2 *= t2 < t3 ? t4 : t3
    if (res != 8) return 1; else t2 = 2;

    res = t2 /= t2 < t3 ? t4 : t3
    if (res != 0) return 1; else t2 = 2;

    res = t2 %= t2 < t3 ? t4 : t3
    if (res != 2) return 1; else t2 = 2;

    res = t2 &= t2 < t3 ? t4 : t3
    if (res != 0) return 1; else t2 = 2;

    res = t2 |= t2 < t3 ? t4 : t3
    if (res != 6) return 1; else t2 = 2;

    res = t2 ^= t2 < t3 ? t4 : t3
    if (res != 6) return 1; else t2 = 2;

    res = t2 <<= t2 < t3 ? t4 : t3
    if (res != 32) return 1; else t2 = 2;

    res = t2 >>= t2 < t3 ? t4 : t3
    if (res != 0) return 1; else t2 = 2;

    res = t2 >>>= t2 < t3 ? t4 : t3
    if (res != 0) return 1; else t2 = 2;

    return 0;
}
