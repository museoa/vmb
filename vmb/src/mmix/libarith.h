/*15:*/
#line 106 "vmb.ch"

extern octa zero_octa;
#line 593 "mmix-sim.w"
extern octa neg_one;
extern octa aux,val;
extern bool overflow;
extern int exceptions;
extern int cur_round;
extern char*next_char;
extern octa oplus ARGS((octa y,octa z));

extern octa ominus ARGS((octa y,octa z));

extern octa incr ARGS((octa y,int delta));

extern octa oand ARGS((octa y,octa z));

extern octa shift_left ARGS((octa y,int s));

extern octa shift_right ARGS((octa y,int s,int u));

extern octa omult ARGS((octa y,octa z));

extern octa signed_omult ARGS((octa y,octa z));

extern octa odiv ARGS((octa x,octa y,octa z));

extern octa signed_odiv ARGS((octa y,octa z));

extern int count_bits ARGS((tetra z));

extern tetra byte_diff ARGS((tetra y,tetra z));

extern tetra wyde_diff ARGS((tetra y,tetra z));

extern octa bool_mult ARGS((octa y,octa z,bool xor));

extern octa load_sf ARGS((tetra z));

extern tetra store_sf ARGS((octa x));

extern octa fplus ARGS((octa y,octa z));

extern octa fmult ARGS((octa y,octa z));

extern octa fdivide ARGS((octa y,octa z));

extern octa froot ARGS((octa,int));

extern octa fremstep ARGS((octa y,octa z,int delta));

extern octa fintegerize ARGS((octa z,int mode));

extern int fcomp ARGS((octa y,octa z));

extern int fepscomp ARGS((octa y,octa z,octa eps,int sim));

extern octa floatit ARGS((octa z,int mode,int unsgnd,int shrt));

extern octa fixit ARGS((octa z,int mode));

extern void print_float ARGS((octa z));

extern int scan_const ARGS((char*buf));


/*:15*/
