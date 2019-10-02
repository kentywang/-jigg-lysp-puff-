/* Global */
typedef enum boolean Boolean;
typedef enum type Type;
typedef union value Value;
typedef struct element Element;
typedef struct pair Pair;

enum error_codes {
  BAD_IDENTIFIER
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
  enum type {
    PAIR,
    NUMBER,
    SYMBOL
  } type_tag;
  union value {
    Pair *pair_ptr;
    int number;
    char *symbol;
    // Need to store string too.
    // And boolean?
  } contents;
};

struct pair {
  Element car;
  Element cdr;
};

/* main.c */

/* read.c */
extern void read_input(Element *);

/* print.c */
extern void print_element(Element *);

/* memory.c */
extern Pair *get_next_free_ptr(void);
extern char *string_alloc(int);

/* data.c */
extern Element make_list(const Pair *);
extern Element make_cons(const Element, const Element);

/* env.c */

/* util.c */
extern Boolean is_integer(char *);