/* Global */
typedef enum boolean Boolean;
typedef enum type Type;
typedef union value Value;
typedef struct element Element;
typedef struct pair Pair;
typedef struct binding Binding;

enum error_codes {
  BAD_IDENTIFIER,
  BAD_EXPRESSION,
  UNBOUND_VARIABLE,
  UNEXPECTED_ARG
};

// Be explicit that false should be 0.
enum boolean {
  FALSE = 0,
  TRUE = 1
};

// Members of enum and union must be in same corresponding order for initial
// memory values to match their types.
// Be careful, though, since "initial values" means nothing after garbage
// collection.
struct element {
  enum {
    PAIR,
    NUMBER,
    SYMBOL,
    // The following are never read from input, and are never exposed to
    // users:
    PRIMITIVE_PROCEDURE,
    COMPOUND_PROCEDURE
  } type_tag;
  union {
    Pair *pair_ptr;
    int number;
    char *symbol;
    // PRIMITIVE_PROCEDURE uses a Pair pointer like PAIR.
    Element (*func_ptr)(const Pair *);
    // Need to store string too.
    // And boolean?
  } contents;
};

struct pair {
  Element car;
  Element cdr;
};

// Just a single-use object to hold variables to load into memory in
// proper list-structure.
struct binding {
  char *variable;
  Element value;
};

/* main.c */

/* read.c */
extern void read_input(Element *, const Boolean);

/* eval.c */
extern Element eval_dispatch(const Element, const Element);

/* print.c */
extern void print_element(const Element);

/* memory.c */
extern Pair *get_next_free_ptr(void);
extern char *string_alloc(int);

/* data.c */
extern Element make_list(const Pair *);
extern Element make_cons(const Element, const Element);
extern Element car(const Element);
extern Element cdr(const Element);

/* env.c */
extern Element extend_environment(const Element, const Element);
extern Element setup_environment(void);
extern Binding find_binding(char *, Element);
extern Element lookup_variable_value(char *, Element);
extern Boolean is_empty_environment(const Element);

/* primitive.c */
extern Binding initial_frame[];

/* util.c */
extern Boolean is_integer(char *);