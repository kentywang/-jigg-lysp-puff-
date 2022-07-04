## (jigg lisp puff)

 Compiling:
```
clang data.c memory.c env.c eval.c main.c primitive.c print.c read.c util.c
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
For tail-call recursion testing:
```
(define y (lambda (x) (if (= x 10) 'woof (y (+ x 1)))))

(y 1)
```
Doesn't work yet, need to figure out why:
```
λ 》((lambda (x) (x x)) +)
45800720

λ 》(((lambda (x) (x x))
  (lambda (fact-gen)
    (lambda (n)
      (if (= 0 n)
          1
          (* n ((fact-gen fact-gen) (- n 1)))))))
 5)
Not a procedure.
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
  - Also try spamming `'1`, mix with other commands
  - Make sure define_variable mutates global env. I suspect it doesn't.
    - Actually, it's working fine since we're modifying the car.
  - Maybe prob was when there was no stack, global env gets deleted.
    - Yep! That was it.
  - Still need to turn back on recursive cleaning. 

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
- Why this returns `(. 2)` instead of `2`?
```(cdr '(1 . 2))```
- So I think for first pass, we don't need virtual stack. C's stack and C's
  heap will support our Lisp layer's 2 heaps (which are statically
  allocated C memory).
  - Actually, I think we do need virtual stack for GC.
- C's stack will obviously clean up after itself, but when GCing Lisp heap,
  that's when we free symbol memory off C heap.
- For values to mark during GC to keep, mark current stack of env frames,
  along with current input (remember parsed input is also on Lisp heap) at
  least until evaluation of the input fully finishes.
- How will Lisp heap know what's the current env stack or current input? I
  guess it needs to maintain two elements that the REPL instantiates for it.
- I think we need to have a statically allocated array of pair addresses, not
  pairs themselves (and so get_next_free_ptr returns an C heap address rather
  than an address in the statically allocated pair array) because we don't
  want to change the underlying addresses that Lisp uses for its data whenever
  we GC. We just want to cull the addresses themselves, and have C free the
  memory for the culled addresses.
- Wait, can I recursively clean up (i.e. freeing) without accidentally
  freeing necessary memory? I.e. deleting a used env stack will eventually
  recurse to the global env, which means the global bindings get deleted. How
  can I avoid this?
- Not working (but `nil` does, why?):
```
()
```
  - Fixed. We need to evaluate `()` as a self-evaluating expression.
- Now infinite loop when GCing, and also if delete symbol, corrupted data.
  Why?
    - A: we have cycles in env! Because defined functions have env in it. Need
      to mark seen nodes.
  - Why unexpected type tag? Corrupted data?
- Okay, so if I leave out cleanup_element, I still see unexpected type tag
  after GC. If I leave it in, I also get corrupted symbols.
  
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
- After struggling with how to build the GC in my interpreter for a while, I
  think I realize now that explicit control of GC and tail-call optimization
  (as opposed to relying on the implementation language's own facilities for
  those features) requires the modeling of their channels of control,
  registers and stacks.
- GC needs more than just current Lisp stack's environment as a root node. It
  needs all stack environments as root nodes. This is cause the current stack
  frame may have a partial environment, while the frame underneath has
  another, different partial environment that also needs to be preserved so
  that the full stack can return something.
  - Example:
```
(define y (lambda (x) (+ x 1)))
(define z (lambda (w) (+ w (y 30))))
(z 10)
```
      While within the y call of the z call, w is nowhere in the env, but we
      still need to remember w for the stack frame beneath.