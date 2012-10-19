
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

#pragma warning(disable : 4146 4018 4244 )
@z

@x
 no_const_found: next_char=s;@+return -1;
@y
                 next_char=s;@+return -1;
@z

@x
 make_it_zero: exp=-99999;@+ goto packit;
@y
               exp=-99999;@+ goto packit;
@z