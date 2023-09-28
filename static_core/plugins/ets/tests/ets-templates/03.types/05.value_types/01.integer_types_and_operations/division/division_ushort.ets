{% for v in values %}
/*---
desc: check division for unsigned short integer operands
---*/

function main(): void {
    const a: ushort = {{v.left}}
    const b: ushort = {{v.right}}
    assert (a / b) == {{v.result}}
}

{% endfor %}
