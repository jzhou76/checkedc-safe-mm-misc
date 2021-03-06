# A list of functionalities that the new safe pointers should support or forbid

Note: the **ptr** appeared in the following text means the new safe pointer to
struct objects.

## assignment
- ~~assign the address of newly allocated heap object~~
- ~~assign NULL to ptr~~
- ~~assign NULL to an array of pointers statically~~
- ~~assign one safe pointer to another~~
- ~~forbid directly assigning an address to a ptr~~

## dereference
- ~~dereference a valid ptr pointing to an allocated object~~
- ~~dereference a null ptr~~
- ~~dereference a ptr pointing to a freed object~~

## function calls
- pass 1 ptr as function argument
- pass n ptr as function arguments
- return ptr from a function

- pointer arithmetic, ptr++/--, ptr + const, ptr1 + ptr2

## cast
- disallow cast raw pointer to ptr

- have ptr inside a struct; check whether `sizeof` can determine the size correctly
