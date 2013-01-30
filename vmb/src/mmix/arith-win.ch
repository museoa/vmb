
@x
@<Stuff for \CEE/ preprocessor@>=
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
@y
@<Stuff for \CEE/ preprocessor@>=
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif

#pragma warning(disable : 4146 4018 4244 4267)
@z
