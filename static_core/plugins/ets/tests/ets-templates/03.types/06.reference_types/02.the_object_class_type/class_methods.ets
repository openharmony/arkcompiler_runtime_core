{% for method in methods %}

/*---
desc: Inheritance methods from Object class.
params: {{method}}
---*/

function main(): void {
    let s: String = new String("test");
    s.{{method}}
}

{% endfor %}