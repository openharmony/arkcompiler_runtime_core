{% for c in cases %}
/*---
desc: An if statement can be conditionally executed, or one of the two two statements can be selected conditionally.
---*/

let res = ""

function ct(): boolean {
    res += "T"
    return true;
}

function cf(): boolean {
    res += "F"
    return false;
}

function retcode(s: string): int {
    if (res == s) {
        return 0;
    } else {
        return 1;
    }
}

function main(): int {
    res = ""; // reset global res variable because of JIT testing
    let i = 1;
    let j = 1;
    {{c.use|indent}}
}
{% endfor %}
