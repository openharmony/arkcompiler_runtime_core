How to Use the Cookbook
=======================

The main goal of this cookbook is to provide recipes for all partially
supported features and explicitly list all unsupported features.

The document is built on the feature-by-feature basis, and if you do not
find some feature, then you can safely consider it **supported**. Otherwise,
a recipe will give you a suggestion on how to rewrite your code and work
around an unsupported case.

|

Recipe Explained
----------------

The original |TS| code containing the keyword ``var``:

.. code-block:: typescript

    function addTen(x: number): number {
        var ten = 10
        return x + ten
    }

must be rewritten as follows:

.. code:: typescript

    // Important! This is still valid TypeScript.
    function addTen(x: number): number {
        let ten = 10
        return x + ten
    }

Status of Unsupported Features
------------------------------

Currently unsupported are mainly the features which:

- relate to dynamic typing that degrades runtime performance, or
- require extra support in the compiler, thus degrading project build time.

However, the |LANG| team reserves the right to reconsider the list and
**shrink** it in the future releases based on the feedback from the developers,
and on more real-world data experiments.

|

|


.. raw:: pdf

   PageBreak


