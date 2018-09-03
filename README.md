PEG debug
=========

A debug viewer for Parsing Expression Grammars using [cpp-peglib](https://github.com/yhirose/cpp-peglib).

When writing grammars, it somehow often happens that they don't do what they were intended to do. This recursive explorer lets you investigate, why certain rules matched certain parts of text and why other rules did not. It does not only show the AST of the completed parsement, it illustrates the complete recursive parsing process, including not matching attempts.

Building
--------

```
mkdir build
cd build
cmake ..
make
```

Running
-------

```
./pegdebug ../example/pl0.peg ../example/gcd.pl0 ../example/output.html
```

Example
-------

See /example/output.html for exemplary output.

The left side displays the recursion tree of the parsing process. When a rule matches, it is green and the matching part of the string is highlighted. If it doesn't match, it is read. Both matching and not matching rules can be unfolded and investigated by clicking them.

The right side shows, what part of the text was matched by what rule. Hovering over the text makes an info window appear that shows the stack of rules that apply for the piece of text. Clicking freezes the info window and the matches for the rules in the rulestack can be highlighted by hovering over them.

