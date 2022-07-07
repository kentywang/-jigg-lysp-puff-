## (jigg lisp puff)

 Compiling:
```
clang data.c memory.c env.c eval.c main.c primitive.c print.c read.c
```
Running:
```
./a.out
```

A Scheme interpreter developed from scratch in C.

 Overview of features
- Lambda expressions
- Quotations
- Definitions
- Conditional expression and booleans
- Tail call recursion
- Garbage collection (mark-and-sweep)
- Virtual stack and two virtual heaps
- Verbose option for tracing evaluation (WIP)

### Lessons learned
- Creating a parser was quite a task on its own. Even for that alone I'm
  proud of my work.
- Creating a garbage collector was even more challenging, but I learned a lot
  about memory allocation and the expected lifecycle of data during a REPL
  cycle.
- There's three things to consider: not freeing everything (memory leak),
  freeing things twice (easy to notice), and freeing too much (like what I
  experienced when I freed intermediate values used during eval).
- Freeing too much happens when I don't capture all the roots to mark to keep.
- Every malloc needs an accompanying free, ideally in the same function.
- But that only solves the memory leak issue; it doesn't address the
  freeing too much issue.
- To solve that, I need to make sure we preserve all intermediate values so
  they don't get freed during GC. One way to do that is before every
  string_alloc or get_next_free_ptr, we make sure to push our current work
  to the stack.
- The pattern I've found useful is for each function to be responsible for
  preserving its own intermediate values to prevent GC from taking them, 
  but they needn't have any obligation to preserve those values once it
  passes them to another function. It's up to the next function to
  preserve them (if it needs them). Simultaneously, calling functions must
  assume its called functions that involve allocating new memory will destroy
  any data in unsaved addresses during the function execution.
- Using calloc is a good idea if I don't want to use memory addresses with
  random initial data.
- For mutating an object's pointer member, I can't pass the pointer into
  a function; I must pass the object.
- fseek and fgets don't work with live stdin (as opposed to from a text file).
  For fgets, the program waits until we have _new_ input before getting.
- () and [] has higher precedence than ++ and *, despite some online sources
  stating otherwise.
- Going back to a codebase after almost a year in a language that you spent
  around 3 weeks learning is terrifying.

### Todos
- Improve time complexity of GC from O(n^4) to O(n).
- Writeup architecture readme.
- Add more primitives and other usual language features.
- Flesh out verbose mode.

### Tests
Parsing/printing tests:
```
  '(1(  2  3  )4)
```
```
   '(1 2 3)
```
```
    '(1 2 (3 4 (5)) 6 7)
```
```
    '(1(  -2  3a  )4)
```
```
(list 'n 2 3)
```
```
(list '- -2)
```
```
'apple
```
```
   'apple
```
Evaluation tests:
```
(define fact
  (lambda (n)
    (if (= n 1)
        1
        (* (fact (+ n -1)) n))))
```
```
(define id (lambda (x) x))
```
```
(define cons (lambda (x y) (lambda (m) (m x y))))
```
```
(define car (lambda (z) (z (lambda (p q) p))))
```
```
((lambda (x) (x x)) (lambda (x) (x x)))
```
```
(((lambda (x) (x x))
  (lambda (fact-gen)
    (lambda (n)
      (if (= 0 n)
          1
          (* n ((fact-gen fact-gen) (+ n -1)))))))
 5)
```
```
(define enum-interval
  (lambda (start end)
    (if (= start end)
        (list start)
        (cons start
              (enum-interval (+ 1 start)
                             end)))))
(enum-interval 1 8)
```
For tail-call recursion testing:
```
(define y (lambda (x) (if (= x 10000) 'woof (y (+ x 1)))))

(y 1)
```
