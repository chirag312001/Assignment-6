#include <stdio.h>
#include "object.h"
#include "value.h"

int main() {
    /* Create A and B */
    ObjPair* A = new_pair(make_int(1), make_int(2));
    ObjPair* B = new_pair(make_int(3), make_int(4));

    /* Make a cycle:
         A → B
         B → A
    */
    A->left = make_obj((Obj*)B);
    B->left = make_obj((Obj*)A);

    printf("Before GC: heap_objects = %p\n", (void*)heap_objects);

    /* No roots added → both unreachable */
    gc_collect();

    printf("After GC: heap_objects = %p\n", (void*)heap_objects);

    if (heap_objects == NULL)
        printf("Cycle correctly collected!\n");
    else
        printf("ERROR: cycle not collected\n");

    return 0;
}
