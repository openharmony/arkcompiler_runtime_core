Introduction
============

|

Welcome to the "|TS| to |LANG|" cookbook. This document gives you short
recipes to rewrite your standard |TS| code to |LANG|. Although |LANG| is
designed to be close to |TS|, some limitations were added for the sake of
performance. As a result, all |TS| features can be divided into the following
categories:

#. **Fully supported features**: the original code requires no modification
   at all. According to our measurements, projects that already follow the
   best |TS| practices can keep 90% to 97% of their codebase intact.
#. **Partially supported features**: some minor code refactoring is needed.
   Example: The keyword ``let`` must be used in place of ``var`` to declare
   variables. Please note that your code will still remain a valid |TS| code
   after rewriting by our recipes.
#. **Unsupported features**: a greater code refactoring effort can be required.
   Example: The type ``any`` is unsupported, and you are to introduce explicit
   typing to your code everywhere ``any`` is used.

|

|



.. raw:: pdf

   PageBreak


