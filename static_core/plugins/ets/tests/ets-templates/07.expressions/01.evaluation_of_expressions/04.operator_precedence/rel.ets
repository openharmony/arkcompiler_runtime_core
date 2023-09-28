/*---
desc: relational and logical operations
---*/

function main(): int {
    let t1 = 1, t2 = 2, t3 = 3, t4 = 4;

    if (t1 > t2 || t2 >= t3 && t3 < t4 || t4 <= t1 && t4 == t2 || t4 != t1) {} else return 1;
    if (t1 > t2 && t2 >= t3 || t3 < t4 && t4 <= t1 || t4 == t2 && t4 != t1) return 1;

    // prefix and postfix added
    if (++t1 <= t2-- && --t2 == t3++ && ++t3 != t4-- || --t4 <= t1++ && ++t4 == t2-- || --t4 != t1++) {} else return 1;
    if (++t1 <= t2-- || --t2 == t3++ && ++t3 != t4-- && --t4 <= t1++ || ++t4 == t2-- && --t4 != t1++) return 1;

    // bitwise xor, unary + and - added
    if (++t1 <= ~t2-- && --t2 == -t3++ && ++t3 != -t4-- || --t4 <= ~t1++ && ++t4 == +t2-- || --t4 != -t1++) {} else return 1;
    if (++t1 <= -~++t2 && ~++t2 == -t3++ && ++t3 != -t4-- || ~+t4++ <= ~t1++ && -t4 == +t2-- || --t4 == -t1++) return 1;

    return 0;
}
