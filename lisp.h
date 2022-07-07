/* Global */
#define LAMBDA "lambda"
#define QUOTE "quote"
#define QUOTE_LENGTH 5 // The length of the word "quote"
#define DEFINE "define"
#define IF "if"
#define TRUE_SYMBOL "true"
#define FALSE_SYMBOL "false"
#define print_verbose verbosity && printf

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
  UNEXPECTED_ARG,
  NOT_PROCEDURE,
  ARITY_MISMATCH,
  STACK_OVERFLOW,
  HEAP_OVERFLOW
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
    BOOLEAN,
    // The following are never read from input, and are never exposed to
    // users:
    PRIMITIVE_PROCEDURE,
    COMPOUND_PROCEDURE
  } type;
  union {
    Pair *pair_ptr;
    int number;
    char *symbol;
    Boolean truth;
    // PRIMITIVE_PROCEDURE uses a Pair pointer like PAIR.
    Element (*func_ptr)(const Pair *);
    // Need to store string too.
  } data;
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
extern Boolean verbosity;

/* read.c */
extern void read_input(Element *);

/* eval.c */
extern Element eval_dispatch(Element, Element);

/* print.c */
extern void print_element(Element);
extern void print_pair(const Pair *);

/* memory.c */
extern Pair *get_next_free_ptr(void);
extern char *string_alloc(int);
extern void save(Element);
extern void release(int);
extern void cleanup_element(Element);
static void reset_deleted(void);
extern int idx(void);

/* data.c */
extern Element make_cons(Element, Element);
extern Element car(Element);
extern Element cdr(Element);
extern Boolean is_true(Element);

/* env.c */
extern Element setup_environment(void);
extern Binding find_binding(char *, Element);
extern Element lookup_variable_value(char *, Element);
extern Element make_frame(Element, Element);
extern Element first_frame(Element);
extern Element enclosing_environment(Element);
extern Boolean is_empty_environment(Element);

/* primitive.c */
extern Binding initial_frame[];
extern Element NIL;