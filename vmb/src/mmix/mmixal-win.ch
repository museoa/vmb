
@x
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif
@y
#ifdef __STDC__
#define ARGS(list) list
#else
#define ARGS(list) ()
#endif

#pragma warning(disable : 4146 4244) 
@z
