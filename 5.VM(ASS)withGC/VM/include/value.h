#ifndef VALUE_H
#define VALUE_H

#include <stdint.h>

/* Forward declaration of Obj (defined in object.h later) */
typedef struct Obj Obj;

/* What kind of value is this? */
typedef enum {
    VAL_NIL,
    VAL_OBJ
} ValueType;

/* A VM stack cell */
typedef struct {
    ValueType type;
    Obj* obj;
} Value;

/* Constructors */
Value make_int(int32_t x);
Value make_obj(Obj* o);

#endif
