# Indexing C projects with clang

Indexing means cross referencing, find definition and all usages of any functions.
Writing indexing tool is like writing linker, main job is resolving symbols.

## Features implemented
* Go to definition
  * Macro
  * Function

## Known bugs
* weak symbols
  Two global functions defined, one is weak, the strong version should take precedence and mark the weak one as declaration.
  eg. `skb_copy_bits` in `kernel/bpf/core.c`.

## TODO
* Go to definition
  * Type
  * Field
* Inline
  * macro expansion, with formatting
  * Enum member
  * Field type
* Find all reference
  * Function
  * Type
  * Field

## No plan to support C++, yet
Use other tools to index your C++ projects:
* [kythe](http://kythe.io)
* [woboq](http://code.woboq.org)
* [dxr](https://wiki.mozilla.org/DXR)

## First enemy of indexing C code: Preprocessing

Almost all difficiulities come from preprocessing especially macro.
* `#include` `MACRO`, file included depends on definition of `MACRO`, hard to insert a link.
* Easy to index macro name, difficiult or impossible to index the content of macro. 
* A header may provide different symbols if macros defined before #include change.
* A function may be defined in macro expansion, hard to find a location.

