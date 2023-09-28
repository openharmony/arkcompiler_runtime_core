{% for rn in right_shift_negative %}
/*---
desc: Corner case for right shift
---*/

function main(): void {
    let n: {{rn.type}} = {{rn.value}};
    let s = 1;

    let a = n >>> s;
    let b = (n >> s) + (2 << ~s);
    assert a == b;
}
{% endfor %}
