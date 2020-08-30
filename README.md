## (jigg lisp puff)

 Compiling:
```
clang data.c env.c eval.c main.c primitive.c print.c read.c stack.c heap.c util.c
```
Running:
```
./a.out
```

A Scheme interpreter in C developed for exercise 5.51 of SICP.

My approach will be to garbage collect on a fixed region of memory for
list-structured data, but strings/symbols will actually be stored in
dynamically-allocated memory via malloc/free. Though we could store them
in lists eventually.

Should we want to explicitly control call stacks, we can either store the
stack within or as a separate structure from the GCed heap. Since stacks
never build up if tail recursion is implemented properly, it doesn't make
sense to keep it with other data that does need to be garbage collected.

I think we should use the agnostic Element type more than the Pair pointer.

### Overview of features added
- Verbose option for tracing evaluation (WIP)
- Lambda expressions
- Quotations
- Converts single quotes in input into quote lists and does the opposite
  conversion on output.
- Definitions
- Conditional expression and booleans.

### Questions
- How do you differentiate between newlines (ignored) and the enter key?
  A: Don't think you can, you need to listen to keyboard's shift.
- Do we need to manually set CDR to null because of GC?
- Should I create an abstraction layer to convert an argument list into
  a va_list for primitive functions to apply?
- Will staticall-declared functions be accessible from the outside?

### Tests
These should be in them:
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
'apple
```
```
   'apple
```
```
(define fact
  (lambda (n)
    (if (= n 1)
        1
        (* (fact (+ n -1)) n))))
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
For tail-call recursion testing:
```
(define y (lambda (x) (if (= x 10) 'woof (y (+ x 1)))))

(y 1)
```

### Todos
- Devise method of unit testing.
- Remove unneeded wrapping of Pair pointers with an Element, since they're
  initialized with PAIR type tags already. (GC might complicate this, we'll
  need to reset the type_tag and content in get_next_free_ptr! Hmm, but that
  approach might mean we'd need to manually set the CDR to the empty pair
  instead on relying on uninitialized Elements to always be empty pairs, or
  would it?)
- Can we convert the functions that take Element pointers into functions that
  take Elements? (read?) Maybe make it Pair pointers so it might be left-right
  expression agnostic?
- Make backend more C-like.
- Make read robust against newlines, live stdin.
- Protect against overly long input words.
- Unify procedures used in both interpreted and backend, or separate them.
- Expand verbose to eval/apply.
- Can we change a lot of our Element functions to Pair pointers without
  breaking their behavior within interpretation?
- Implement manual tail recursion.
- GC!
  - Use case from SICP where two lists are produced to generate a number to
    write a test:
    ```(accumulate + 0 (filter odd? (enumerate-interval 0 n)))```
  - Get primitive `cons` working so I can run:
    ```
    (define enum-interval
      (lambda (start end)
        (if (= start end)
            (list start)
            (cons start
                  (enum-interval (+ 1 start)
                                 end)))))

    (enum-interval 1 3)
    ```
  - I believe my interpreter can have the illusion of infinite memory in
    two ways: I can implement GC in our "virtual" heap and stack (including
    registers?), or implement GC for C. The former will probably mean
    reworking the interpreter into an explicit-control evaluator (which is
    what the exercise is asking for). The latter might mean we don't need the
    "virtual" heap and stack anymore (since it's Lisp-specific). The latter
    is definitely harder; I'm not sure how a GC in C could recognize what's
    deletable and what isn't in the Lisp layer.
- Writeup architecture readme.
- I waiver between relying on default initialization for Elements and
  explicitly setting the values. Choose one. (Also, we might not be able to
  rely on uninitialized values being what we expect on automatic variables.)
- Am I not storing pairs for strings? Makes sense, right, because how would a
  string be kept in list-structured memory?
- I'm not treating single-quotes in quotes literally; it's still converting
  them to a quote list. Not sure if this affects any behavior.
- Add full list of primitives.
- Does empty list work? (i.e. '())
- Support reading boolean.
- Make a list of the built-in functions.
- Tail-optimized recursion

### Lessons learned
- Creating a parser was quite a task on its own. Even for that alone I'm
  proud of my work.
- For mutating an object's pointer member, I can't pass the pointer into
  a function; I must pass the object.
- fseek and fgets don't work with live stdin (as opposed to from a text file).
  For fgets, the program waits until we have _new_ input before getting.
- () and [] has higher precedence than ++ and *, despite some online sources
  stating otherwise.
- Going back to a codebase after almost a year in a language that you spent
  around 3 weeks learning is terrifying.
