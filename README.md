# lispterp — A Scheme-like Lisp Interpreter in C++

A tree-walking interpreter for a small Scheme/Lisp dialect, written in modern C++17.
Built to demonstrate language-implementation fundamentals: lexing, parsing, tree-walking
evaluation, lexical scoping, closures, and tail-call optimization.

## Features

- **Lexer / Parser**: hand-written recursive-descent parser that turns S-expressions into
  a tree of `Value` nodes (cons cells).
- **Dynamic typing**: numbers, symbols, strings, booleans, pairs/lists, and functions, all
  represented via a tagged `Value` struct with `shared_ptr`-based memory management.
- **Lexical scoping & closures**: environments are chained via parent pointers; lambdas
  capture their defining environment, so closures work correctly.
- **Tail-call optimization**: `eval` is implemented as a loop rather than naive recursion,
  so tail calls (including self-recursive loops) execute in constant C++ stack space —
  verified with a 1,000,000-iteration tail-recursive loop.
- **Special forms**: `define`, `lambda`, `if`, `cond`, `let`, `begin`, `quote`, `set!`,
  `and`, `or`.
- **Builtins**: arithmetic (`+ - * / modulo`), comparisons (`= < > <= >=`), list operations
  (`cons car cdr list null? pair? length append reverse`), `eq?`, `not`, `display`, `newline`.
- **REPL** and **file execution** modes.

## Build

Requires a C++17 compiler. No external dependencies.

```bash
g++ -std=c++17 -O2 -o lispterp src/main.cpp
```

or with CMake:

```bash
mkdir build && cd build
cmake ..
make
```

## Usage

```bash
./lispterp                  # start the REPL
./lispterp examples/demo.lisp   # run a script
```

## Example

```scheme
(define (factorial n)
  (if (= n 0)
      1
      (* n (factorial (- n 1)))))

(display (factorial 10))   ; => 3628800

(define (make-adder n)
  (lambda (x) (+ x n)))

(define add5 (make-adder 5))
(display (add5 10))        ; => 15
```

See `examples/demo.lisp` for more, including a tail-recursive loop and a `map`
implemented entirely in the language itself.

## Architecture

```
src/
  value.hpp        Value type (numbers, symbols, pairs, lambdas...) + Environment
  lexer.hpp         Source text -> tokens
  parser.hpp         Tokens -> S-expression AST
  list_utils.hpp     Helpers for converting between Lisp lists and std::vector, pretty-printing
  interpreter.hpp     eval(): special forms, function application, tail-call loop
  builtins.hpp        Native (C++) functions registered into the global environment
  main.cpp             REPL + file runner
```

### Design notes

- **Why a loop-based `eval` instead of recursive?** A naive tree-walking interpreter
  recurses once per nested call, so a tail-recursive Lisp loop would grow the *C++* call
  stack and eventually overflow, even though the Lisp program itself is iterative. Here,
  tail positions (the last expression in a `begin`/`let`/lambda body, both branches of
  `if`, etc.) update `expr`/`env` and `continue` the same `while` loop instead of calling
  `eval` again, giving proper tail calls.
- **Why `shared_ptr` for `Value`?** Lisp values can be referenced from multiple places at
  once (a list shared between two variables, a closure capturing its environment). Manual
  ownership tracking would be error-prone; `shared_ptr` reference-counting matches Lisp's
  GC'd semantics with minimal code, at the cost of not handling reference cycles (not
  needed for this subset of the language, since closures only capture parent environments,
  never each other in a cycle).

## Possible extensions

- Garbage collector (mark-and-sweep) instead of refcounting, to handle cycles
- `define-syntax` / macros
- Proper numeric tower (integers vs. floats vs. rationals)
- Better error messages with source line/column tracking
- A standard library file (`stdlib.lisp`) loaded at startup, with `map`, `filter`,
  `fold`, etc. written in the language itself
