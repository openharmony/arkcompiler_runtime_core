{% for v in values %}
/*---
desc: check bitwise AND operation for unsigned long operands
---*/

function main(): void {
    const a: ulong = {{v.left}}
    const b: ulong = {{v.right}}
    assert (a & b) == {{v.result}}
}

{% endfor %}
