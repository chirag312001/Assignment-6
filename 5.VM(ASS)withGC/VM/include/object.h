#ifndef OBJECT_H
#define OBJECT_H

#include "value.h"

extern int no_of_object_freed;
extern int total_objects_created;

/* All heap object types */
typedef enum {
    OBJ_PAIR,
    OBJ_INT,
    OBJ_FUNCTION,
    OBJ_CLOSURE
} ObjType;

/* Base header for every heap object */
typedef struct Obj {
    ObjType type;
    int marked;      /* for garbage collection (GC mark bit)*/
    struct Obj* next;   /* linked list of all heap objects */
} Obj;

/* Pair object */
typedef struct {
    Obj base;
    Value left;
    Value right;
} ObjPair;

typedef struct {
    Obj base;
    int value;
} ObjInt;

/* Dummy definitions for new types */
typedef struct {
    Obj base;
    /* In a real VM, this would point to bytecode or a C function */
} ObjFunction;

typedef struct {
    Obj base;
    Obj* function; /* Reference to the ObjFunction */
    Obj* env;      /* Reference to an ObjPair or other scope object */
} ObjClosure;

/*  Heap tracking  */
extern Obj* heap_objects;

void heap_register(Obj* o);


/* Allocation */
ObjPair* new_pair(Value l, Value r);
ObjInt  *new_int(int value);


/* graph creation from list(heap) .*/
void obj_visit_children(Obj* o, void (*visit)(Obj*));

/* Additional object types for functions and closures(dummy part that is created to test the part) */
Obj* new_function();
Obj* new_closure(Obj* fn, Obj* env);
/* GC roots & mark phase */
//garbage collector functions would go here
void gc_mark_from_roots();

// void gc_add_root(Obj* o);

//This is where memory is actually freed.
void gc_sweep();

// Complete garbage collection cycle
void gc_collect(int show_debug);






#endif
