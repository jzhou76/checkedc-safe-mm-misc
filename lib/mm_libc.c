/**
 * MM safe version of common libc functions that needs special handling to
 * return an mmsafe pointer.
 * */

#include "safe_mm_checked.h"
#include <string.h>

// A helper struct that has the same inner structure as an mmsafe ptr.
typedef struct {
  void *p;
  uint64_t key_offset;
} _MMSafe_ptr_Rep;

/* A helper function  for mm_strchr, mm_strchr, etc.
 * See the comment of  _create_mm_array_ptr() in safe_mm_checked.c for details.
 *
 * */
static mm_array_ptr<char>
_create_mm_array_ptr_char(mm_array_ptr<const char> p, char *new_p) {
    _MMSafe_ptr_Rep *base_safeptr_ptr = (_MMSafe_ptr_Rep *)&p;
    _MMSafe_ptr_Rep new_safeptr = {
        new_p,
        base_safeptr_ptr->key_offset + (new_p - (char *)(base_safeptr_ptr->p))
  };

  return *((mm_array_ptr<char> *)&new_safeptr);
}


/* strchr */
mm_array_ptr<char> mm_strchr(mm_array_ptr<const char> p, int c) {
    char *new_p = strchr(_GETCHARPTR(p), c);
    return _create_mm_array_ptr_char(p, new_p);
}

/* strpbrk */
mm_array_ptr<char> mm_strpbrk(mm_array_ptr<const char> p, const char *accept) {
    char *new_p = strpbrk(_GETCHARPTR(p), accept);
    return _create_mm_array_ptr_char(p, new_p);
}

/* strstr() */
mm_array_ptr<char> mm_strstr(mm_array_ptr<const char> p, const char *needle) {
  char *new_p = strstr(_GETCHARPTR(p), needle);
  return _create_mm_array_ptr_char(p, new_p);
}

/* memchr() */
mm_array_ptr<char> mm_memchr(mm_array_ptr<const char> s, int c, size_t n) {
  char *ret_p = memchr(_GETPTR(char, s), c, n);
  return _create_mm_array_ptr_char(s, ret_p);
}

/* memrchr() */
mm_array_ptr<char> mm_memrchr(mm_array_ptr<const char> s, int c, size_t n) {
  char *ret_p = memrchr(_GETPTR(char, s), c, n);
  return _create_mm_array_ptr_char(s, ret_p);
}


/* strtok_r() */
mm_array_ptr<char> mm_strtok_r(mm_array_ptr<char> str, const char *delim, char **saveptr) {
  char *ret_p = strtok_r(_GETCHARPTR(str), delim, saveptr);
  return _create_mm_array_ptr_char(str, ret_p);
}
