# ifj21

Compiler for subset of TEAL language named "ifj21". Code in "ifj21" language is compiled to an intermediate
representation called "ifjcode21"

### Compilation

- To compile a compiler itself:

```shell
  cd cmake-build-debug && make ifj21
```

-To compile & generate tests:

```shell
  cd cmake-build-debug && make testgen && ./testgen
```

### Running

```shell
./ifj21 < "inputfile.tl"
```

### Testing

```shell
cd tests && ./tests_shch all
```

An intermediate code is written to the stdout.
