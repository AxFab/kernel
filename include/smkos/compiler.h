/*
    This header file has for purpose to identify the compiler and define some
  generic macro that we can used in a portable application.

  Current version identify 73 compilers.
  At start the compiler was on alphabetical order, but for optimization
  pupose, we will try to sort them by order of usage.

  sources: http://sourceforge.net/p/predef/wiki

  Define the macros:
    __CC                          A const c-string this the name of compilor.
    __CXX                         Same as __CC if it support C++.
    __LLONG                       Is defined if the architecture allow
                                  long long integer (32 bits only).
    PACK (struct {...});          Disable structure fields alignement.

  We define also one of:
    __SIP16, __LP32, __ILP32, __LP64, __LLP64, __ILP64, __SILP64

  This represent the data model and will be reused to define:
    WORDSIZE                           Size used as word (pointer)

*/
#pragma once

/* --------------------------------------------------------------------------
    GCC C/C++
-------------------------------------------------------------------------- */
#if defined __GNUC__
#  define __CC  "GCC C/C++"
#  define __CXX "GCC C/C++"

#  if defined __alpha__ /* Alpha */
#  elif defined __amd64__ || defined __amd64 || defined __x86_64__ || defined __x86_64 /* AMD64 */
#    define __LP64
#  elif defined __arm__ /* ARM */
#  elif defined __thumb__ /* ARM thumb mode */
#  elif defined __aarch64__ /* ARM64 */
#  elif defined __bfin || defined __BFIN__ /* Blackfin */
#  elif defined __convex__ /* Convex */
#  elif defined __hppa__ /* HP/PA RISC */
#  elif defined i386 || defined __i386 || defined __i386__ /* Intel x86 */
#    define __ILP32
#    define __LLONG
#  elif defined __ia64__ || defined _IA64 || defined __IA64__ /* Intel Itanium (IA-64) */
#  elif defined __m68k__ /* Motorola 68k */
#  elif defined __mips__ || defined mips /* MIPS */
#  elif defined __powerpc || defined __powerpc__ || defined __POWERPC__ || defined __ppc__ /* PowerPC */
#  elif defined __powerpc64__ || defined __ppc64__ /* PowerPC 64 */
#  elif defined __sparc__ /* SPARC */
#  elif defined __sh__ /* SuperH */
#  elif defined __s390__ /* SystemZ */
#  else
#    error GCC C/C++ use an unrecognized architecture
#  endif

#  define PACK(decl) decl __attribute__((__packed__))

#  undef __THROW
#  define __THROW
#  undef restrict
#  define restrict __restrict


/* --------------------------------------------------------------------------
    Microsoft Visual C++
-------------------------------------------------------------------------- */
#elif defined _MSC_VER
#  define __CC  "Microsoft Visual C++"
#  define __CXX "Microsoft Visual C++"
#  define asm __asm
#  define inline __inline

#  if defined _M_ALPHA
#  elif defined _M_X64 || defined _M_AMD64
#    define __LLP64
#  elif defined _M_ARM
#  elif defined _M_I86 /* Intel 16 bits */
#    define __SIP16
#  elif defined _M_IX86 /* Intel 32 bits */
#    define __ILP32
#    define __LLONG
#  elif defined _M_IA64 /* Itanium 64 bits */
#    define __LLP64
#  elif defined _M_PPC /* Power PC */
#  else
#    error Microsoft Visual C++ use an unrecognized architecture
#  endif

#  undef __THROW
#  define __THROW
/*#  define restrict __restrict*/
#  define PACK(decl) __pragma( pack(push, 1) ) decl __pragma( pack(pop) )


/* --------------------------------------------------------------------------
    Clang
-------------------------------------------------------------------------- */
#elif defined __clang__                       /* Clang */
#  error Unsupported compiler: Clang



/* --------------------------------------------------------------------------
    Intel C/C++
-------------------------------------------------------------------------- */
#elif defined __INTEL_COMPILER || defined __ICL /* Intel C/C++ */
#  error Unsupported compiler: Intel C/C++
#elif defined __ICC || defined __ECC          /* Intel C/C++ (Obsolete) */
#  error Obsolete version of the Intel C/C++ Compiler



/* --------------------------------------------------------------------------
    HP ANSI C / HP aC++
-------------------------------------------------------------------------- */
#elif defined __HP_cc                         /* HP ANSI C */
#  error Unsupported compiler: HP ANSI C
#elif defined __HP_aCC                        /* HP aC++ */
#  error Unsupported compiler: HP aC++



/* --------------------------------------------------------------------------
    MinGW & MinGW-w64
-------------------------------------------------------------------------- */
#elif defined __MINGW32__                     /* MinGW */
#  error Unsupported compiler: MinGW
#elif defined __MINGW64__                     /* MinGW-w64 */
#  error Unsupported compiler: MinGW-w64



/* --------------------------------------------------------------------------
    Unsupported but recognized compilers
-------------------------------------------------------------------------- */
#elif defined __CMB__                            /* Altium MicroBlaze C */
#  define restrict
#  error Unsupported compiler: Altium MicroBlaze C
#elif defined __CHC__                         /* Altium C-to-Hardware */
#  error Unsupported compiler: Altium C-to-Hardware
#elif defined __ACK__                         /* Amsterdam Compiler Kit */
#  error Unsupported compiler: Amsterdam Compiler Kit
#elif defined __CC_ARM                        /* ARM Compiler */
#  error Unsupported compiler: ARM Compiler
#elif defined AZTEC_C || defined __AZTEC_C__  /* Aztec C */
#  error Unsupported compiler: Aztec C
#elif defined __BORLANDC__                    /* Borland C++ */
#  error Unsupported compiler: Borland C++
#elif defined __CODEGEARC__                   /* Borland C++ (>2006) */
#  error Unsupported compiler: Borland C++ (>2006)
#elif defined __CC65__                        /* CC65 */
#  error Unsupported compiler: CC65
#elif defined __COMO__                        /* Comeau C++ */
#  error Unsupported compiler: Comeau C++
#elif defined __DECC || defined __DECCXX      /* Compaq C/C++ */
#  error Unsupported compiler: Compaq C/C++
#elif defined __convexc__                     /* Convex C */
#  error Unsupported compiler: Convex C
#elif defined __COMPCERT__                    /* CompCert */
#  error Unsupported compiler: CompCert
#elif defined __COVERITY__                    /* Coverity C/C++ Static A. */
#  error Unsupported compiler: Coverity C/C++ Static A.
#elif defined __DCC__                         /* Diab C/C++ */
#  error Unsupported compiler: Diab C/C++
#elif defined _DICE                           /* DICE C */
#  error Unsupported compiler: DICE C
#elif defined __DMC__                         /* Digital Mars */
#  error Unsupported compiler: Digital Mars
#elif defined __SYSC__                        /* Dignus Systems/C++ */
#  error Unsupported compiler: Dignus Systems/C++
#elif defined __DJGPP__ || defined __GO32__   /* DJGPP */
#  error Unsupported compiler: DJGPP
#elif defined __EDG__                         /* EDG C++ Frontend */
#  error Unsupported compiler: EDG C++ Frontend
#elif defined __PATHCC__                      /* EKOPath */
#  error Unsupported compiler: EKOPath
#elif defined __FCC_VERSION                   /* Fujitsu C++ */
#  error Unsupported compiler: Fujitsu C++

#elif defined __ghs__                         /* Green Hill C/C++ */
#  error Unsupported compiler: Green Hill C/C++
#elif defined   __IAR_SYSTEMS_ICC__           /* IAR C/C++ */
#  error Unsupported compiler: IAR C/C++
#elif defined __xlc__ || defined    __xlC__   /* IBM XL C/C++ */
#  error Unsupported compiler: IBM XL C/C++
#elif defined __IBMC__ || defined __IBMCPP__  /* IBM z/OS C/C++ */
#  error Unsupported compiler: IBM z/OS C/C++
#elif defined __IMAGECRAFT__                  /* ImageCraft C */
#  error Unsupported compiler: ImageCraft C
#elif defined __KCC                           /* KAI C++ */
#  error Unsupported compiler: KAI C++
#elif defined __CA__ || defined __KEIL__      /* KEIL CARM */
#  error Unsupported compiler: KEIL CARM
#elif defined __C166__                        /* KEIL C166 */
#  error Unsupported compiler: KEIL C166
#elif defined __C51__ || defined __CX51__     /* KEIL C51 */
#  error Unsupported compiler: KEIL C51
#elif defined __LCC__                         /* LCC */
#  error Unsupported compiler: LCC
#elif defined __llvm__                        /* LLVM */
#  error Unsupported compiler: LLVM
#elif defined __HIGHC__                       /* MetaWare High C/C++ */
#  error Unsupported compiler: MetaWare High C/C++
#elif defined __MWERKS__ || defined __CWCC__  /* Metrowerks CodeWarrior */
#  error Unsupported compiler: Metrowerks CodeWarrior

#elif defined _MRI                            /* Microtec C/C++ */
#  error Unsupported compiler: Microtec C/C++
#elif defined __NDPC__ || defined __NDPX__    /* Microway NDP C */
#  error Unsupported compiler: Microway NDP C
#elif defined __sgi || defined sgi            /* MIPSpro */
#  error Unsupported compiler: MIPSpro
#elif defined MIRACLE                         /* Miracle C */
#  error Unsupported compiler: Miracle C
#elif defined __MRC__ || defined MPW_C || defined MPW_CPLUS /* MPW C++ */
#  error Unsupported compiler: MPW C++
#elif defined __CC_NORCROFT                   /* Norcroft C */
#  error Unsupported compiler: Norcroft C
#elif defined __NWCC__                        /* NWCC */
#  error Unsupported compiler: NWCC
#elif defined __OPEN64__ || defined __OPENCC__ /* NWCC */
#  error Unsupported compiler: NWCC
#elif defined ORA_PROC                        /* Oracle Pro*C Precompiler */
#  error Unsupported compiler: Oracle Pro*C Precompiler
#elif defined __SUNPRO_C || defined __SUNPRO_CC /* Oracle Solaris Studio */
#  error Unsupported compiler: Oracle Solaris Studio
#elif defined __PACIFIC__                     /* Pacific C */
#  error Unsupported compiler: Pacific C
#elif defined _PACC_VER                       /* Palm C/C++ */
#  error Unsupported compiler: Palm C/C++
#elif defined __POCC__                        /* Pelles C */
#  error Unsupported compiler: Pelles C
#elif defined __PGI                           /* Portland Group C/C++ */
#  error Unsupported compiler: Portland Group C/C++
#elif defined __RENESAS__ || defined __HITACHI__ /* Renesas C/C++ */
#  error Unsupported compiler: Renesas C/C++
#elif defined SASC || defined __SASC || defined __SASC__ /* SAS/C */
#  error Unsupported compiler: SAS/C
#elif defined _SCO_DS                         /* SCO OpenServer */
#  error Unsupported compiler: SCO OpenServer
#elif defined SDCC                            /* Small Device C Compiler */
#  error Unsupported compiler: Small Device C Compiler
#elif defined __SNC__                         /* SN Compiler */
#  error Unsupported compiler: SN Compiler
#elif defined __VOSC__                        /* Stratus VOS C */
#  error Unsupported compiler: Stratus VOS C
#elif defined __SC__                          /* Symantec C++ */
#  error Unsupported compiler: Symantec C++
#elif defined __TenDRA__                      /* TenDRA C/C++ */
#  error Unsupported compiler: TenDRA C/C++
#elif defined __TI_COMPILER_VERSION__ || defined _TMS320C6X   /* Texas Instruments C/C++ Compiler */
#  error Unsupported compiler: Texas Instruments C/C++ Compiler
#elif defined THINKC3 || defined THINKC4      /* THINK C */
#  error Unsupported compiler: THINK C
#elif defined __TINYC__                       /* Tiny C */
#  error Unsupported compiler: Tiny C
#elif defined __TURBOC__                      /* Turbo C/C++ */
#  error Unsupported compiler: Turbo C/C++
#elif defined _UCC                            /* Ultimate C/C++ */
#  error Unsupported compiler: Ultimate C/C++
#elif defined __USLC__                        /* USL C */
#  error Unsupported compiler: USL C
#elif defined __VBCC__                        /* VBCC */
#  error Unsupported compiler: VBCC
#elif defined __WATCOMC__                     /* Watcom C++ */
#  error Unsupported compiler: Watcom C++
#elif defined __ZTC__                         /* Zortech C++ */
#  error Unsupported compiler: Zortech C++
#else
#  error "Is that you're own personal compiler or what ?"
#  error Right one, I did't recognize this one, but if it's used propose your changes.
#  error For now you will have to make it work by yourself.
#endif



/* --------------------------------------------------------------------------
-------------------------------------------------------------------------- */
