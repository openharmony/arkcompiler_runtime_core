/*---
desc: ternary operator on boolean values
---*/

let res = "";

function ff(): boolean {
    res += "F";
    return false;
}

function ft(): boolean {
    res += "T";
    return true;
}

function main(): int {
    let v = false, vt = true, vf = false
    res = "";

    v = ft() ? ff() ? vt: ft() ? ff() ? vt : ff() ? vt : vf : vf : vf;
    if (v || res != "TFTFF") return 1;

    return 0;
}
