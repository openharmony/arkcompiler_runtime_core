/*---
desc: Multiple entrypoints with different names
---*/

package test;

function main(): void {

}

{% for ident in identifiers %}

function main${{ident}}(): void {
    
}

{%- endfor %}