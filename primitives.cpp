
typedef void PRIMITIVE_TYPE;

/*
 * The primitives are as follows (non-exhaustive):
 *
 * push, pop
 * define
 * move
 * +, *, /
 * concatenate
 *
 * This is a dynamically typed language but it strongly enforces type safety
 * both dynamically and statically.  Static compilation passes can find any
 * type inconsistencies, but if a cell changes at runtime then it is flagged to
 * have a runtime type check included [TODO: Look at handling the runtime check
 * safely, perhaps in a kind of wrapper. The caller of that method may have to
 * expect a null answer or an Maybe type].  This kind of construction ensures
 * that a high-level language targetting these primitives can be either
 * statically or dynamically typed without concessions. 
 *
 */

PRIMITIVE_TYPE push (void* data)
{
}

PRIMITIVE_TYPE pop (void* data)
{
}

PRIMITIVE_TYPE add (void* data)
{
}

/*
PRIMITIVE_TYPE move (void* data)
{
}

PRIMITIVE_TYPE define (void* data)
{
}
*/
