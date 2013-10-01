#include <stdlib.h>
#include "mmix-internals.h"
#include "vmb.h"
#include "mmixlib.h"

static void free_sym(sym_node *sym)
{ if (((unsigned long long int)sym&~0x3)==0) return;
  free_sym(sym->link);
  sym->link=NULL;
  free(sym);
}
  

void free_tree(trie_node *root)
{ if (root==NULL) return;
  if (root->left!=NULL) free_tree(root->left);
  root->left=NULL;
  if (root->mid!=NULL) free_tree(root->mid);
  root->mid=NULL;
  if (root->right!=NULL) free_tree(root->right);
  root->right=NULL;
  free_sym(root->sym);
  free(root);
}

void free_file_info(void)
{ int i;
  for (i=0; i<256; i++)
  { if (file_info[i].name!=NULL)
    {  free(file_info[i].name);
       file_info[i].name=NULL;
    }
    file_info[i].line_count=0;
    if (file_info[i].map!=NULL)
    {  free(file_info[i].map);
       file_info[i].map=NULL;
    }
	ybyte2file_no[i]=-1;
  }
}