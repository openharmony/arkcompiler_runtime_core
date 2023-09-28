/*---
desc: operators on boolean values
---*/

function main(): int {
    let v = false, vt = true, vf = false;

    // & |
    v = vt | vf & vf
    if (!v) return 1;

    v = vt | vt & vf
    if (!v) return 1;

    // & ^
    v = vt ^ vf & vf
    if (!v) return 1;

    v = vt ^ vt & vf
    if (!v) return 1;

    // ^ |
    v = vt | vf ^ vt
    if (!v) return 1;

    v = vt | vt ^ vt
    if (!v) return 1;

    // & ==
    v = vf & vf == vf
    if (v) return 1;

    v = vf & vt == vf
    if (v) return 1;

    // & !=
    v = vf & vf != vt
    if (v) return 1;

    v = vf & vt != vt
    if (v) return 1;

    // | ==
    v = vt | vf == vf
    if (!v) return 1;

    v = vt | vt == vf
    if (!v) return 1;

    // | !=
    v = vt | vf != vt
    if (!v) return 1;

    v = vt | vt != vt
    if (!v) return 1;

    return 0;
}
