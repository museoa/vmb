
extern void create_symtab(void);
extern void symtab_reset(void);
extern void symtab_add(char *symbol, sym_node *sym);
extern void symtab_mark(char *symbol);
extern int symtab_measureitem(LPMEASUREITEMSTRUCT mi);
extern int symtab_drawitem(LPDRAWITEMSTRUCT di);
extern sym_node *find_symbol(char *symbol, trie_node *t);