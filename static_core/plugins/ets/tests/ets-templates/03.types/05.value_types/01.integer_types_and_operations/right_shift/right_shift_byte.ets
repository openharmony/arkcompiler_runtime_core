{% for v in values %}
/*---
desc: check right shift operation for byte
---*/

function main(): void {
    const a: byte = {{v.left}}
    const b: int = {{v.right}}
    assert (a >> b) == {{v.result}}
}

{% endfor %}
