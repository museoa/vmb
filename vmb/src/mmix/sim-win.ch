
@x
@ We declare subroutines twice, once with a prototype and once
with the old-style~\CEE/ conventions. The following hack makes
this work with new compilers as well as the old standbys.

@<Preprocessor macros@>=
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
@y
@ We declare subroutines twice, once with a prototype and once
with the old-style~\CEE/ conventions. The following hack makes
this work with new compilers as well as the old standbys.

For the Windows Visual C Compiler we add some further 
preprocessor definitions to get rid of compiler warnings. 

@<Preprocessor macros@>=
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif

#pragma warning(disable : 4996 4267 4146 4018 4244 4113)
@z

