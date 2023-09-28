/*---
desc: additive operator with other lower precedence operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255  = 255

    // cast
    res = t2 + t255 as byte
    if (res != 1) return 1;

    res = t255 as byte + t3
    if (res != 2) return 1;

    // shifts
    res = t4 << t3 + t2
    if (res != 128) return 1;

    res = - t255 >> t2 + t3
    if (res != -8) return 1;

    res = - t255 >>> t2 + t3
    if (res != 134217720) return 1;

    // relational
    if (t8 < t4 + t4) return 1;
    if (t255 <= t8 + t4) return 1;
    if (t2 > t2 + t4) return 1;
    if (t4 >= t2 + t4) return 1;
    if (t2 == t2 + t4) return 1;
    if (t8 != t4 + t4) return 1;

    // bitwise
    res = t255 & t4 + t8
    if (res != 12) return 1;

    res = t3 | t4 + t2
    if (res != 7) return 1;

    res = t255 ^ t8 + t2
    if (res != 245) return 1;

    // ternary
    res = t3 <= t4 ? t4 : t3 + t4
    if (res != 4) return 1;

    res = t4 + t2 > t4 ? t4 : t3
    if (res != 4) return 1;

    // assignments
    res = t2 += t4 + t3
    if (res != 9) return 1; else t2 = 2;

    res = t2 -= t4 + t3
    if (res != -5) return 1; else t2 = 2;

    res = t8 *= t2 + t3
    if (res != 40) return 1; else t8 = 8;

    res = t8 /= t2 + t3
    if (res != 1) return 1; else t8 = 8;

    res = t8 %= t2 + t3
    if (res != 3) return 1; else t8 = 8;

    res = t255 &= t2 + t3
    if (res != 5) return 1; else t255 = 255;

    res = t2 |= t3 + t4
    if (res != 7) return 1; else t2 = 2;

    res = t255 ^= t4 + t8
    if (res != 243) return 1; else t255 = 255;

    res = t4 <<= t3 + t2
    if (res != 128) return 1; else t4 = 4;

    res = t255 >>= t2 + t3
    if (res != 7) return 1; else t255 = 255;

    res = t255 >>>= t3 + t2
    if (res != 7) return 1; else t255 = 255;

    return 0;
}
