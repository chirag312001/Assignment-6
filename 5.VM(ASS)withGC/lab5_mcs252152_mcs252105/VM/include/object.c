#include <stdlib.h>
#include <stdio.h>
#include "object.h"
#include "vm.h"

/*    MARK STACK (ITERATIVE GC)   */

#define MARK_STACK_MAX 1024
static Obj* mark_stack[MARK_STACK_MAX];
static int mark_stack_top = 0;

static void mark_push(Obj* o) {
    if (!o) return;
    if (mark_stack_top < MARK_STACK_MAX) {
        mark_stack[mark_stack_top++] = o;
    }
}

/*    HEAP LIST   */

Obj* heap_objects = NULL;

int no_of_object_freed = 0;

void heap_register(Obj* o) {
    o->next = heap_objects;
    heap_objects = o;
}

/*   ALLOCATION    */

ObjPair* new_pair(Value l, Value r) {
    ObjPair* pair = malloc(sizeof(ObjPair));
    if (!pair) return NULL;

    pair->base.type = OBJ_PAIR;
    pair->base.marked = 0;
    heap_register((Obj*)pair);

    pair->left = l;
    pair->right = r;
    return pair;
}

ObjInt* new_int(int value) {
    ObjInt* i = malloc(sizeof(ObjInt));
    if (!i) return NULL;

    i->base.type = OBJ_INT;
    i->base.marked = 0;
    heap_register((Obj*)i);

    i->value = value;
    return i;
}

Obj* new_function() {
    ObjFunction* f = malloc(sizeof(ObjFunction));
    if (!f) return NULL;

    f->base.type = OBJ_FUNCTION;
    f->base.marked = 0;

    heap_register((Obj*)f);
    return (Obj*)f;
}

Obj* new_closure(Obj* fn, Obj* env) {
    ObjClosure* cl = malloc(sizeof(ObjClosure));
    if (!cl) return NULL;

    cl->base.type = OBJ_CLOSURE;
    cl->base.marked = 0;
    cl->function = fn;
    cl->env = env;

    heap_register((Obj*)cl);
    return (Obj*)cl;
}


/*    OBJECT GRAPH TRAVERSAL    */

void obj_visit_children(Obj* o, void (*visit)(Obj*)) {
    if (!o) return;

    switch (o->type) {
        case OBJ_PAIR: {
            ObjPair* p = (ObjPair*)o;
            visit(p->left.obj);
            visit(p->right.obj);
            break;
        }
        case OBJ_INT:
            /* boxed integer has no children */
            break;
        case OBJ_FUNCTION:
            /* function has no children in this simple VM */
            break;
        case OBJ_CLOSURE:
            {
                ObjClosure* cl = (ObjClosure*)o;
                visit(cl->function);
                visit(cl->env);
                break;
            }
        default:
            break;
    }
}

/*    DEBUG: HEAP PRINTER   */

static void gc_print_heap(const char* phase) {
    printf("\n[GC] HEAP DUMP (%s)\n", phase);

    if (!heap_objects) {
        printf("  <empty>\n");
        return;
    }

    for (Obj* o = heap_objects; o; o = o->next) {
        if (o->type == OBJ_INT) {
            ObjInt* i = (ObjInt*)o;
            printf("  ObjInt  %p | value=%d | marked=%d\n",
                   (void*)i, i->value, i->base.marked);
        } else if (o->type == OBJ_PAIR) {
            ObjPair* p = (ObjPair*)o;
            printf("  ObjPair %p | marked=%d | left=%p | right=%p\n",
                   (void*)p, p->base.marked,
                   (void*)p->left.obj, (void*)p->right.obj);
        }
    }
}

/*    MARK PHASE    */

static void gc_mark(Obj* root) {
    if (!root) return;

    mark_stack_top = 0;
    mark_push(root);

    while (mark_stack_top > 0) {
        Obj* o = mark_stack[--mark_stack_top];
        if (o->marked) continue;

        o->marked = 1;
        obj_visit_children(o, mark_push);
    }
}

void gc_mark_from_roots(void) {
    vm_visit_roots(gc_mark);
}

/*    SWEEP PHASE    */

void gc_sweep(void) {
    Obj** o = &heap_objects;

    while (*o) {
        if (!(*o)->marked) {
            Obj* dead = *o;
            *o = dead->next;
            no_of_object_freed++;
            printf("[GC] freeing %p\n", (void*)dead);
            free(dead);
        } else {
            (*o)->marked = 0; /* reset for next GC */
            o = &(*o)->next;
        }
    }
}

/*    FULL GC    */

void gc_collect(void) {
   
    gc_print_heap("before mark");
    gc_mark_from_roots();
    gc_print_heap("after mark (before sweep)");
    gc_sweep();
    gc_print_heap("after sweep");
}
