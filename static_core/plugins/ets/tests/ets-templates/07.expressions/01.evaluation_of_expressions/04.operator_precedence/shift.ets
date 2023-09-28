/*---
desc: shift operators with other lower precedence operators
---*/

function main(): int {
    let res: int, t2 = 2, t3 = 3, t4 = 4, t8 = 8, t255  = 255

    // shifts
    res = t255 << t3 >> t2 >>> t4
    if (res != 31) return 1;

    // relational
    if (t8 < t2 << t2) return 1;
    if (t255 <= t2 << t2) return 1;
    if (t8 > t2 << t2) return 1;
    if (t4 >= t2 << t2) return 1;
    if (t4 == t2 << t2) return 1;
    if (t8 != t2 << t2) return 1;

    // bitwise
    res = t4 & t4 << t2
    if (res != 0) return 1;

    res = t3 | t4 << t2
    if (res != 19) return 1;

    res = t255 ^ t8 << t2
    if (res != 223) return 1;

    // ternary
    res = t3 <= t4 ? t4 : t3 << t2
    if (res != 4) return 1;

    res = t4 << t2 > t4 ? t4 : t3
    if (res != 4) return 1;

    // assignments
    res = t2 += t4 << t2
    if (res != 18) return 1; else t2 = 2;

    res = t2 -= t4 << t2
    if (res != -14) return 1; else t2 = 2;

    res = t8 *= t2 << t3
    if (res != 128) return 1; else t8 = 8;

    res = t255 /= t2 << t3
    if (res != 15) return 1; else t255 = 255;

    res = t255 %= t2 << t3
    if (res != 15) return 1; else t255 = 255;

    res = t255 &= t255 << t3
    if (res != 248) return 1; else t255 = 255;

    res = t255 |= t255 << t3
    if (res != 2047) return 1; else t255 = 255;

    res = t255 ^= t4 << t8
    if (res != 1279) return 1; else t255 = 255;

    res = t4 <<= t3 << t2
    if (res != 16384) return 1; else t4 = 4;

    res = t255 >>= t2 << t3
    if (res != 0) return 1; else t255 = 255;

    res = t255 >>>= t3 << t2
    if (res != 0) return 1; else t255 = 255;

    return 0;
}
