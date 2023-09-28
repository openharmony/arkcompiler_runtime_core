{% for v in values %}
/*---
desc: check right shift operation for unsigned short integer
---*/

function main(): void {
    const a: ushort = {{v.left}}
    const b: int = {{v.right}}
    assert (a >> b) == {{v.result}}
}

{% endfor %}
