#include "value.h"
#include "object.h"

/* Boxed integer constructor */
Value make_int(int32_t x) {
    Value v;
    v.type = VAL_OBJ;
    v.obj = (Obj*)new_int(x);   /* heap allocation */
    return v;
}

/* Wrap existing object */
Value make_obj(Obj* o) {
    Value v;
    v.type = VAL_OBJ;
    v.obj = o;
    return v;
}
