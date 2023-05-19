# Cefunge
[Befunge-93](https://catseye.tc/view/Befunge-93/doc/Befunge-93.markdown) interpreter written in C.
---
## Usage
#### Compile:
`make`
#### Execute HelloWorld.befunge:
`./cefunge examples/HelloWorld.befunge`
#### Execute Quine.befunge on 35x2 field with max stack size of 512 elements:
`./cefunge -w 35 -h 2 -s 512 examples/Quine.befunge`