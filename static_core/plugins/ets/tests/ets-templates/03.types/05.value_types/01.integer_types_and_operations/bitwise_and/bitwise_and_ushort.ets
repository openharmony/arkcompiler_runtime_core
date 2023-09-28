{% for v in values %}
/*---
desc: check bitwise AND operation for unsigned short operands
---*/

function main(): void {
    const a: ushort = {{v.left}}
    const b: ushort = {{v.right}}
    assert (a & b) == {{v.result}}
}

{% endfor %}
