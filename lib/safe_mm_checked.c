/** safe_mm_checked.c - Runtime library of the temporal memory safe Checked C.
 *
 * This files defines the runtime library for the Checked C project, including
 * customized memory allocators and deallocators for memory objects pointed
 * by mm_ptr and mm_array_ptr.
 *
 * Note that the current implementation assumes a 32-32 key-offset metadata
 * design for mmsafe ptr. In the paper we say that we support two
 * key-offset schemes: 32-32 and 40-24. Also it is not totally clear to us
 * which is faster key-offset or offset-key. For the 32-32 option it might
 * be the same but the key-offset one might be a little faster for
 * the 40-24 one because the constant used in an "and" instruction can be
 * hardcoded in the instruction instead of loading from another register.
 *
 * */

#include "safe_mm_checked.h"

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <immintrin.h>   /* for _rdrand32_step() */

#define __INLINE __attribute__((always_inline))

/* The real lock size is 4 bytes but we allocate 8 bytes for it for alignment. */
#define LOCK_MEM 8
#define HEAP_PADDING 8
#define KEY_MASK 0x00000000ffffffff

#define GET_KEY(key_offset) ((key_offset >> 32) & KEY_MASK)
#define GET_OFFSET(key_offset) (key_offset & KEY_MASK)

// A helper struct that has the same inner structure as an mmsafe ptr.
typedef struct {
  void *p;
  uint64_t key_offset;
} _MMSafe_ptr_Rep;

#if 1
// Our current implementation uses a 32-bit key. We may need to change it
// to 40-bit key later.
uint32_t key = 3;
#endif

//
// Function: rand_keygen()
//
// Generate a 32-bit random key using Intel's RDRAND instruction.
//
__INLINE
static uint32_t rand_keygen() {
    uint32_t key;
    while (1) {
        int succeeded = _rdrand32_step(&key);
        // 0, 1, and 2 are reserved for invalid key, stack, and global variables.
        if (succeeded && key > 2) break;
    }

    return key;
}

/*
 * Function: mm_initKey()
 *
 * Create the initial key for a program. All subsequent allocations just
 * increase the key by 1. Call to this function is inserted by the compiler
 * at the beginning of the main function.
 *
 * Jie Zhou: For some unknow reason, if we add __INLINE to this function,
 * and use LTO for Olden benchmarks, the cmake configuration procedure
 * would throw a "LLVM ERROR: Cannot select: t9: i32,i32,ch = X86ISD::RDRAND t0"
 * error while using the compiler to compile a temporary test program.
 * */
void mm_init_key() {
    key = rand_keygen();
}

//
// Function: mm_alloc()
//
// This is a customized memory allocator to allocate a struct object
// on the heap. First it creates a helper struct
// _MM_ptr_Rep and initializes its two field accordingly. Second it casts
// the type of an implicit pointer to the type of a pointer to a _MM_ptr.
// Finally it returns the dereference of the pointer to _MM_ptr.
//
// Note that arithmetic on "void *" is undefined behavior by C's specification.
// For GCC or Clang, the "-pedantic-errors" flag would cause a program to fail
// compiling if it has such an undefined behavior.
// Related reading: https://stackoverflow.com/questions/3523145/pointer-arithmetic-for-void-pointer-in-c
__attribute__ ((noinline))
for_any(T) mm_ptr<T> mm_alloc(size_t size) {
    // We need the HEAP_PADDING to ensure that mm_ptr inside a struct
    // is aligned by 16 bytes.
    // See this issue for the reason: https://github.com/jzhou76/checkedc-llvm/issues/2
    void *raw_ptr = malloc(size + HEAP_PADDING + LOCK_MEM);
    if (raw_ptr == NULL) return NULL;

    // The lock is located before the first field of the referent.
    raw_ptr += HEAP_PADDING;
    *((uint32_t *)(raw_ptr)) = key;

    // Create a helper struct to initialize the mm_ptr.
    _MMSafe_ptr_Rep safe_ptr = { .p = raw_ptr + LOCK_MEM, .key_offset = key };
    // Move the key to the highest 32 bits and make the offset 0.
    safe_ptr.key_offset <<= 32;

    key++;

    return *((mm_ptr<T> *)&safe_ptr);
}

//
// Function: mm_array_alloc()
//
// This is a customized memory allocator to allocator an array on the heap.
// The implementation is the same as mm_alloc() because in the current
// prototype mm_ptr and mm_array_ptr have the same structure.
//
__attribute__ ((noinline))
for_any(T) mm_array_ptr<T> mm_array_alloc(size_t array_size) {
  void *raw_ptr = malloc(array_size + LOCK_MEM + HEAP_PADDING);
  if (raw_ptr == NULL) return NULL;

  raw_ptr += HEAP_PADDING;
  *((uint32_t *)(raw_ptr)) = key;

    // Create a helper struct to initialize the mm_array_ptr.
    _MMSafe_ptr_Rep safe_ptr = { .p = raw_ptr + LOCK_MEM, .key_offset = key };
    // Move the key to the highest 32 bits and make the offset 0.
    safe_ptr.key_offset <<= 32;

    key++;

  return *((mm_array_ptr<T> *)&safe_ptr);
}

//
// Function: mm_array_realloc()
//
// This is the mm-safe version of realloc() for arrays.
//
// Note:
// Now we assume that if the re-allocated "new" object starts from the
// same starting address of the "old" object, then they are the same object
// and hence the "new" object uses the same lock as the "old" one.
// If the new object is really re-allocated and starts from somewhere else,
// the old one would be freed and we need create a new lock for the new object.
//
__attribute__ ((noinline))
for_any(T) mm_array_ptr<T> mm_array_realloc(mm_array_ptr<T> p, size_t size) {
    // Get the original raw pointer.
    void * old_raw_ptr = ((_MMSafe_ptr_Rep *)&p)->p;
    old_raw_ptr = old_raw_ptr - LOCK_MEM - HEAP_PADDING;

    void *new_raw_ptr = realloc(old_raw_ptr, size);
    if (new_raw_ptr == old_raw_ptr) {
        return p;
    }

    // The new object is placed in a different location and the old one
    // is freed. The old object's lock needs to be invalidated.
    *((uint32_t *)(old_raw_ptr + HEAP_PADDING)) = 0;

    new_raw_ptr += HEAP_PADDING;
    *((uint32_t *)new_raw_ptr) = key;
    _MMSafe_ptr_Rep safe_ptr = {.p = new_raw_ptr + LOCK_MEM, .key_offset = key};
    safe_ptr.key_offset <<= 32;

    key++;

    mm_array_ptr<T> *mm_array_ptr_ptr = (mm_array_ptr<T> *)&safe_ptr;
    return *mm_array_ptr_ptr;
}

//
// Function: mm_free()
//
// This is a customized memory deallocator for mm_ptr.
// It sets the lock of the singleton memory object to 0
// and calls free() from the stdlib to free the whole memory object.
// Beofore the real free, it also does double-free and invalid-free checking.
//
// @param p - a _MM_ptr whose pointee is going to be freed.
//
// Potential optimization: can we pass p by register?
//
for_any(T) void mm_free(mm_ptr<const T> const p) {
    // free() allows freeing a null ptr, in which case nothing is performed.
    if (p == NULL) return;

    // Without the "volatile" keyword, Clang may optimize away the next
    // statement.
    volatile _MMSafe_ptr_Rep *mm_ptr_ptr = (_MMSafe_ptr_Rep *)&p;

    // Do two temporal memory safety checks.
    // First, check if the offset is zero. A non-zero offset means an invalid free.
    uint64_t key_offset = mm_ptr_ptr->key_offset;
    if (GET_OFFSET(key_offset) != 0) {
        // An invalid free
        fprintf(stderr, "Invalid Free (non-zero offset in an mmptr).\n");
        abort();
    }

    // Second, do a key checking. This would catch double free or UAF errors.
    void *lock_ptr = mm_ptr_ptr->p - LOCK_MEM;
    if (GET_KEY(key_offset) != *(uint32_t *)lock_ptr) {
        fprintf(stderr, "Double Free or UAF\n");
        abort();
    }

    // Invalidate the lock.
    // This step may not be necessary in some cases. In some implementation,
    // free() zeros out all bytes of the memory region of the freed object.
    *(uint32_t *)lock_ptr = 0;

    free(lock_ptr - HEAP_PADDING);
}

//
// Function: mm_array_free()
//
// This is a customized memory deallocator to free heap arrays pointed by
// mm_array_ptr<T>.
//
// @param p - a _MM_array_ptr whose pointee is going to be freed.
//
for_any(T) void mm_array_free(mm_array_ptr<const T> const p) {
    if (p == NULL) return;

    volatile _MMSafe_ptr_Rep *mm_array_ptr_ptr = (_MMSafe_ptr_Rep *)&p;

    // Do two temporal memory safety checks.
    // First, check if the offset is zero. A non-zero offset means an invalid free.
    uint64_t key_offset = mm_array_ptr_ptr->key_offset;
    if (GET_OFFSET(key_offset) != 0) {
        // An invalid free
        fprintf(stderr, "Invalid Free (non-zero offset in an mmptr).\n");
        abort();
    }

    // Second, do a key checking. This would catch double free or UAF errors.
    void *lock_ptr = mm_array_ptr_ptr->p - LOCK_MEM;
    if (GET_KEY(key_offset) != *(uint32_t *)lock_ptr) {
        fprintf(stderr, "Double Free or UAF\n");
        abort();
    }

    // Invalidate the lock.
    // This step may not be necessary in some cases. In some implementation,
    // free() zeros out all bytes of the memory region of the freed object.
    *(uint32_t *)lock_ptr = 0;

    free(lock_ptr - HEAP_PADDING);
}


//
// Function: _getptr_mm()
//
// This function extracts the inner raw pointer to the start of a struct
// from an mm_ptr<T>.
// The motivation of creating this function is that some code tries to
// cast an mm_ptr<T> to a raw pointer to another type of struct,
// e.g., the loadtree() function in the bh benchmark of the Olden
// benchmark suite.
//
// However, this is a danger operation. Unless a programmer is completely
// sure that doing so is safe, this function should be avoided.
//
// Maybe we should implement this as a compiler intrinsic?
//
// @param p - the safe pointer whose inner raw pointer to be extracted.
//
for_any(T) void *_getptr_mm(mm_ptr<const T> const p) {
    return ((_MMSafe_ptr_Rep *)&p)->p;
}

//
// This function extracts the inner raw pointer to the start of a struct
// from an mm_array_ptr<T>.
//
for_any(T) void *_getptr_mm_array(mm_array_ptr<const T> const p) {
    return ((_MMSafe_ptr_Rep *)&p)->p;
}


/*
 * Function: create_invalid_mm_ptr()
 *
 * This function creates an invalid mm_ptr with the raw pointer value
 * set to the argument ptr_val.
 *
 * The motivation of having such a function is that sometimes a function
 * with a return type of pointer may want to return different
 * invalid pointers for different erroneous situations. One example is the
 * fdwatch_get_next_client_data() function of thttpd. It returns "(void *)0"
 * and "(void *)-1" for different errors.
 * */
for_any(T) mm_ptr<T> create_invalid_mm_ptr(uint64_t ptr_val) {
  _MMSafe_ptr_Rep mmptr;
  mmptr.p = (void *)ptr_val;
  mm_ptr<T> *mm_ptr_ptr = (mm_ptr<T> *)&mmptr;
  return *mm_ptr_ptr;
}

/**
 * Function: mmptr_to_mmarrayptr()
 *
 * This function converts an mmptr to an mm_array_ptr. Our design generates
 * an mm_ptr for the address-of operator on an item of an array pointed
 * by an mm_array_ptr (https://github.com/jzhou76/checkedc-clang/commit/471fa1e721c8e72640c81c7c21b0c09a74af7346).
 * However, sometimes the result pointer may be used to do pointer arithmetic.
 * For example, function really_clear_connection() of thttpd computes the
 * inteval between the element pointed by an mm_ptr and the beginning of the
 * array that contains the element (https://github.com/jzhou76/checkedc-safe-mm-misc/blob/master/benchmarks/baseline/thttpd-2.29/thttpd.c#L2053)
 *
 * */
for_any(T) mm_array_ptr<T> mmptr_to_mmarrayptr(mm_ptr<T> p) {
    return *((mm_array_ptr<T> *)(&p));
}

/*
 * Function: mmarrayptr_to_mmptr()
 *
 * This function converts an mm_array_ptr to an mm_ptr. This function handles
 * the situation when a program uses the "->" operator to dereference
 * an mm_array_ptr.
 *
 * */
for_any(T) mm_ptr<T> mmarrayptr_to_mmptr(mm_array_ptr<T> p) {
    return *((mm_ptr<T> *)(&p));
}


#if 0
/*
 * Function: _setptr_mm()
 *
 * Do we need this function?
 *
 * */
for_any(T) void _setptr_mm(mm_ptr<const T> *p, char *new_p) {
}
#endif

/*
 * Function: _setptr_mm_array()
 *
 * This function updates an mm_array_ptr with a provided raw C pointer.
 * This is a very dangerous behavior but it is needed for interacting with
 * certain library code. For example, a program may call strtod() and then
 * use the endptr to update an existing pointer. (The parse_number_value()
 * function of parson (https://github.com/kgabis/parson/blob/master/parson.c#L862)
 * shows such an example.)
 * If the to-be-updated pointer is an mm_array_ptr and we would like to keep it
 * as a checked pointer, then we need this function.
 *
 * */
for_any(T) void _setptr_mm_array(mm_array_ptr<const T> *p, char *new_p) {
    _MMSafe_ptr_Rep *safeptr = (_MMSafe_ptr_Rep *)p;
    safeptr->key_offset += (new_p - (char *)(safeptr->p));
    safeptr->p = (void *)new_p;
}
