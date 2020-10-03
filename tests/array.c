/*
 * Tests of array-specific operations.
 * */

#include "debug.h"

/*
 * f0(): Test basic pointer arithmetic by adding/subtracting a number
 * to/from an mm_array_ptr, and subtracting an mm_array_ptr from another.
 * */
void f0() {
    print_start("pointer adding/subtracting a number");

    signal(SIGILL, ill_handler);
    if (setjmp(resume_context) == 1) goto resume;

    mm_array_ptr<int> p0 = mm_array_alloc<int>(sizeof(int) * 10);
    mm_array_ptr<int> p1 = p0 + 2;
    mm_array_ptr<int> p2 = p1 + 3;
    mm_array_ptr<int> p3 = p2 - 4;

    for (int i = 0; i < 10; i++) {
        p0[i] = i + 1;
    }

    // Test validity of p1.
    for (int i = 0; i < 8; i++) {
        if (p1[i] != i + 3) {
            print_error("array.c::f0(): pointer addition");
        }
    }

    // Test validity of p2.
    for (int i = 0; i < 5; i++) {
        if (p2[i] != i + 6) {
            print_error("array.c::f0(): pointer addition");
        }
    }

    // Test validity of p3.
    for (int i = 0; i < 9; i++) {
        if (p3[i] != i + 2) {
            print_error("array.c::f0(): pointer subtraction");
        }
    }

    // Test subtraction between mm_array_ptr.
    if (p1 - p0 != 2 || p2 - p1 != 3 || p2 - p3 != 4 || p2 - p0 != 5) {
            print_error("array.c::f0(): pointer arithmetic");
    }

    mm_array_free<int>(p0);
    p3[0] = 5;  // UAF; should raise an illegal instruction error.

resume:
    print_end("pointer adding/subtracting a number");
}


int main(int argc, char *argv[]) {
    print_main_start(__FILE__);
    signal(SIGILL, ill_handler);
    signal(SIGSEGV, segv_handler);

    f0();

    print_main_end(__FILE__);
    return 0;
}
