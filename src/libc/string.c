#include <string.h>
#include <stdint.h>


#define LINE_MAX 256

// ERROR MESSAGES

static char _strerror_tmp[LINE_MAX];

static char *errMsgs[] = {
  "",
  "Operation not permitted", // EPERM
  "No such file or directory",
  "No such process",
  "Interrupted system call",
  "I/O error",
  "No such device or address",
  "Argument list too long",
  "Exec format error",
  "Bad file number",
  "No child processes",
  "Try again", // EAGAIN
  "Out of memory",
  "Permission denied",
  "Bad address",
  "Block device required",
  "Device or resource busy",
  "File exists",
  "Cross-device link",
  "No such device",
  "Not a directory",
  "Is a directory",
  "Invalid argument",
  "File table overflow",
  "Too many open files",
  "Not a typewriter",
  "Text file busy",
  "File too large",
  "No space left on device",
  "Illegal seek",
  "Read-only file system",
  "Too many links",
  "Broken pipe",
  "Math argument out of domain of func",
  "Math result not representable",
  "Resource deadlock would occur", // EDEADLK
  "File name too long",
  "No record locks available",
  "Function not implemented",
  "Directory not empty",
  "Too many symbolic links encountered",
  "Operation would block", // EWOULDBLOCK => EAGAIN
  "No message of desired type",
  "Identifier removed",
  "Channel number out of range",
  "Level 2 not synchronized",
  "Level 3 halted",
  "Level 3 reset",
  "Link number out of range",
  "Protocol driver not attached",
  "No CSI structure available",
  "Level 2 halted",
  "Invalid exchange",
  "Invalid request descriptor",
  "Exchange full",
  "No anode",
  "Invalid request code",
  "Invalid slot",
  "Resource deadlock would occur", // EDEADLOCK => EDEADLK
  "Bad font file format",
  "Device not a stream",
  "No data available",
  "Timer expired",
  "Out of streams resources",
  "Machine is not on the network",
  "Package not installed",
  "Object is remote",
  "Link has been severed",
  "Advertise error",
  "Srmount error",
  "Communication error on send",
  "Protocol error",
  "Multihop attempted",
  "RFS specific error",
  "Not a data message",
  "Value too large for defined data type",
  "Name not unique on network",
  "File descriptor in bad state",
  "Remote address changed",
  "Can not access a needed shared library",
  "Accessing a corrupted shared library",
  ".lib section in a.out corrupted",
  "Attempting to link in too many shared libraries",
  "Cannot exec a shared library directly",
  "Illegal byte sequence",
  "Interrupted system call should be restarted",
  "Streams pipe error",
  "Too many users",
  "Socket operation on non-socket",
  "Destination address required",
  "Message too long",
  "Protocol wrong type for socket",
  "Protocol not available",
  "Protocol not supported",
  "Socket type not supported",
  "Operation not supported on transport endpoint",
  "Protocol family not supported",
  "Address family not supported by protocol",
  "Address already in use",
  "Cannot assign requested address",
  "Network is down",
  "Network is unreachable",
  "Network dropped connection because of reset",
  "Software caused connection abort",
  "Connection reset by peer",
  "No buffer space available",
  "Transport endpoint is already connected",
  "Transport endpoint is not connected",
  "Cannot send after transport endpoint shutdown",
  "Too many references: cannot splice",
  "Connection timed out",
  "Connection refused",
  "Host is down",
  "No route to host",
  "Operation already in progress",
  "Operation now in progress",
  "Stale NFS file handle",
  "Structure needs cleaning",
  "Not a XENIX named type file",
  "No XENIX semaphores available",
  "Is a named type file",
  "Remote I/O error",
  "Quota exceeded",
  "No medium found",
  "Wrong medium type",
  "Operation Canceled",
  "Required key not available",
  "Key has expired",
  "Key has been revoked",
  "Key was rejected by service",
};



static const char *strerror_p ( int err )
{
  if ( err == 0 ) {
    return "No error";
  }

  int grp = err & 0x1fff00;


  switch ( grp ) {
  case 0:
    // TODO ERANGE, try with sizeof !
    return errMsgs[err];

  default:
    return "Unknown Error";
  }
}

/**
 * @brief Set a suite of bits on a single byte
 */
int bsetbyte (uint8_t *byte, int off, int lg)
{
  uint8_t v = byte[0];
  int mask = (0xFF << off) & 0xFF;

  if (lg + off < 8) {
    mask = (mask & ~(0xFF << (off + lg))) & 0xFF;
  }

  byte[0] = v | mask;
  return v & mask;
}

/**
 * @brief Clear a suite of bits on a single byte
 */
int bclearbyte (uint8_t *byte, int off, int lg)
{
  uint8_t v = byte[0];
  int mask = (0xFF << off) & 0xFF;

  if (lg + off < 8) {
    mask = (mask & ~(0xFF << (off + lg))) & 0xFF;
  }

  byte[0] = v & ~mask;
  return (v ^ mask) & mask;
}

/**
 * @brief Set a suite of bits on a byte map
 */
int bsetbytes (uint8_t *table, int offset, int length)
{
  int ox = offset / 8;
  int oy = offset % 8;
  int r = 0;

  if (oy != 0 || length < 8) {
    if (length + oy < 8) {
      r |= bsetbyte(&table[ox], oy, length);
      length = 0;
    } else {
      r |= bsetbyte(&table[ox], oy, 8 - oy);
      length -= 8 - oy;
    }

    ox++;
  }

  while (length >= 8) {
    r |= table[ox];
    table[ox] = 0xff;
    ox++;
    length -= 8;
  }

  if (length > 0) {
    r |= bsetbyte(&table[ox], 0, length);
  }

  return r;
}


/* ----------------------------------------------------------------------- */
/** Unset a suite of bits on a byte map. */
int bclearbytes (uint8_t *table, int offset, int length)
{
  int ox = offset / 8;
  int oy = offset % 8;
  int r = 0;

  if (oy != 0 || length < 8) {
    if (length + oy < 8) {
      r |= bclearbyte(&table[ox], oy, length);
      length = 0;
    } else {
      r |= bclearbyte(&table[ox], oy, 8 - oy);
      length -= 8 - oy;
    }

    ox++;
  }

  while (length >= 8) {
    r |= ~table[ox];
    table[ox] = 0;
    ox++;
    length -= 8;
  }

  if (length > 0) {
    r |= bclearbyte(&table[ox], 0, length);
  }

  return r;
}


/* ----------------------------------------------------------------------- */
/** Copy block of memory */
void *memcpy ( void *dest, const void *src, size_t length )
{
  register uint8_t *ptr1 = ( uint8_t * ) dest;
  register const uint8_t *ptr2 = ( const uint8_t * ) src;

  while (length--) {
    *ptr1++ = *ptr2++;
  }

  return dest;
}

/* ----------------------------------------------------------------------- */
/** Copy source buffer to destination buffer */
void *memmove ( void *dest, const void *src, size_t length )
{
  register uint8_t *ptr1 = ( uint8_t * ) dest;
  register const uint8_t *ptr2 = ( const uint8_t * ) src;

  if ( ptr1 >= ptr2 || ptr1 >= ptr2 + length ) {
    while (length--) {
      *ptr1++ = *ptr2++;
    }
  } else {
    ptr1 += length - 1;
    ptr2 += length - 1;

    while (length--) {
      *ptr1-- = *ptr2--;
    }
  }

  return dest;
}

/* ----------------------------------------------------------------------- */
/** Compare two blocks of memory */
int memcmp ( const void *dest, const void *src, size_t length )
{
  register const uint8_t *ptr1 = ( const uint8_t * ) dest;
  register const uint8_t *ptr2 = ( const uint8_t * ) src;

  while ( --length && *ptr1 == *ptr2 ) {
    ++ptr1;
    ++ptr2;
  }

  return *ptr1 - *ptr2;
}

/* ----------------------------------------------------------------------- */
/** Search a character into a block of memory */
void *memchr ( const void *ptr, int chr, size_t length )
{
  register const uint8_t *ptr0 = ( const uint8_t * ) ptr;

  while ( length > 0 && ( *ptr0 != ( uint8_t ) chr ) ) {
    ++ptr0;
    --length;
  }

  return ( void * ) ( length ? ptr0 : 0 );
}

/* ----------------------------------------------------------------------- */
/** Set all byte of a block of memory to the same value */
void *memset ( void *ptr, int val, size_t length )
{
  register uint8_t *org = ( uint8_t * ) ptr;

  while (length--) {
    *org++ = ( uint8_t ) val;
  }

  return ptr;
}


/* Copy error string on the buffer */
int strerror_r (int err, char *buffer, size_t length)
{
  const char *str;

  if (buffer == NULL || length == 0)
    return -1;

  str = strerror_p ( err );
  strncpy ( buffer, str, length );
  return strlen(str) <= length ? 0 : -1;
}


/* Copy error string on the buffer */
void strerror_s ( char *str, size_t length, int err )
{
  strncpy ( str, strerror_p ( err ), length );
}


/* Return error string */
char *strerror ( int err )
{
  return strncpy ( _strerror_tmp, strerror_p ( err ), LINE_MAX );
}

/* return length of a null-terminated char string */
size_t strlen ( const char *str )
{
  register const char *end = str;

  while ( *end ) {
    ++end;
  }

  return end - str;
}

/* return length of a null-terminated char string */
size_t strnlen ( const char *str, size_t length )
{
  register size_t count;

  for ( count = 0; count < length && *str; ++str );

  return count;
}



// Copy a null-terminated char string
char *strcpy ( char *dest, const char *src )
{
  register char *ptr1 = ( char * ) dest;
  register const char *ptr2 = ( const char * ) src;

  while ( ( *ptr1++ = *ptr2++ ) );

  return dest;
}

// Copy a char string
char *strncpy ( char *dest, const char *src, size_t length )
{
  register char *ptr1 = ( char * ) dest;
  register const char *ptr2 = ( const char * ) src;

  while ( length-- > 0 && ( *ptr1++ = *ptr2++ ) );

  return dest;
}

// Concat two null-terminated char strings
char *strcat ( char *dest, const char *src )
{
  register char *ptr1 = ( char * ) dest;
  register const char *ptr2 = ( const char * ) src;

  while ( *ptr1 ) {
    ++ptr1;
  }

  while ( ( *ptr1++ = *ptr2++ ) );

  return dest;
}

// Concat two null-terminated char strings
char *strncat ( char *dest, const char *src, size_t length )
{
  register char *ptr1 = ( char * ) dest;
  register const char *ptr2 = ( const char * ) src;

  while ( *ptr1 ) {
    ++ptr1;
  }

  while ( length-- > 0 && ( *ptr1++ = *ptr2++ ) );

  return dest;
}

// Compare two null-terminated char strings
int strcmp ( const char *str1, const char *str2 )
{
  while ( *str1 && ( *str1 == *str2 ) ) {
    ++str1;
    ++str2;
  }

  return *str1 - *str2;
}

// Compare two char strings
int strncmp ( const char *str1, const char *str2, size_t length )
{
  while ( --length && *str1 && *str1 == *str2 ) {
    ++str1;
    ++str2;
  }

  return *str1 - *str2;
}


// ---------------------------------------------------------------------------
// Search a string for a character
char *strchr (const char *string, int ch)
{
  while (*string) {
    if (*string == (char) ch) {
      return (char *) string;
    }

    string++;
  }

  return NULL;
}

// ---------------------------------------------------------------------------
// Search a string for a character
char *strrchr (const char *string, int ch)
{
  int lg = strlen (string) - 1;

  for (; lg >= 0; --lg) {
    if (string[lg] == (char) ch) {
      return (char *) string;
    }
  }

  return NULL;
}


// ---------------------------------------------------------------------------
// Split string into tokens - reentrent (TODO suppress goto and that is not the best way)
char *strtok_r ( register char *s, register const char *delim, char **lasts )
{
  int skip_leading_delim = 1;
  register char *spanp;
  register int c, sc;
  char *tok;

  if ( s == NULL && ( s = *lasts ) == NULL ) {
    return NULL;
  }

  /*
   * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
   */
cont:
  c = *s++;

  for ( spanp = ( char * ) delim; ( sc = *spanp++ ) != 0; ) {
    if ( c == sc ) {
      if ( skip_leading_delim ) {
        goto cont;
      } else {
        *lasts = s;
        s[-1] = 0;
        return ( s - 1 );
      }
    }
  }

  if ( c == 0 ) {      /* no non-delimiter characters */
    *lasts = NULL;
    return NULL;
  }

  tok = s - 1;

  /*
   * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
   * Note that delim must have one NUL; we stop if we see that, too.
   */
  for ( ;; ) {
    c = *s++;
    spanp = ( char * ) delim;

    do {
      if ( ( sc = *spanp++ ) == c ) {
        if ( c == 0 ) {
          s = NULL;
        } else {
          s[-1] = 0;
        }

        *lasts = s;
        return ( tok );
      }
    } while ( sc != 0 );
  }

  /* NOTREACHED */
}

static char *strtok_reent = 0;

// Split string into tokens
char *strtok ( register char *string , register const char *delimitors )
{
  return strtok_r ( string, delimitors, &strtok_reent );
}

