#include <gnome.h>

typedef struct 
{
	char	path_name [2048] ;
	char	file_name [2048] ;
	
	int		size ;
	char	date [2048] ;
	
	char	file_type [2048] ;
	
	GdkPixbuf	*image ;
	GdkPixbuf	*thumbnail ;
}	lats_image_data ;


typedef struct
{
	GtkTreeStore 	*store ;
	char			*dirname ;
}	lats_dir_tree_data ;


int
isdir ( char *filename ) ;

int
islink ( char *filename ) ;

char
*getlink ( char *filename ) ;

int
file_filter ( char *filename ) ;


int
check_for_subdir ( char *dirname ) ;

void
insert_dirname_into_subtree ( 	GtkTreeStore *store, 	// whole tree
								GtkTreeIter iter1, 		// new item
								GtkTreeIter iter2, 		// parent item
								char *dirname, 			// item-name
								char *attrib,			// attribute
								char *realpath ) ;		// the real path

void
insert_dirname_into_tree ( 	GtkTreeStore *store, 		// whole tree
								GtkTreeIter iter, 		// new item
								char *dirname, 			// item-name
								char *attrib,			// attribute
								char *realpath ) ;		// the real path

GtkTreePath
*get_tree_path_by_real_path ( GtkTreeStore *store, GtkTreePath *tree_path, gchar *real_path ) ;

GtkTreePath
*expand_whole_dir_path (	GtkTreeStore *store,
						char *dirname ) ;

void
read_subdir_contents (	GtkTreeStore *store,
						GtkTreeIter parent_iter,
						char *dirname ) ;

void
on_file_tree_expanded ( GtkTreeView *tree_view, GtkTreeIter *iter, 
						GtkTreePath *path, gpointer *user_data ) ;

void
on_file_tree_collapsed ( GtkTreeView *tree_view, GtkTreeIter *iter, GtkTreePath *path ) ;

void
on_file_tree_selection_changed (	GtkTreeSelection *selection,
									GtkTreeModel *model,
									GtkTreePath *path,
									gboolean selected,
									gpointer user_data ) ;

void
create_dir_list ( void ) ;
