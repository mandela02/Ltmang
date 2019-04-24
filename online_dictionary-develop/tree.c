#include <gtk/gtk.h>


GtkBuilder      *builder; 
GtkWidget       *window;
GtkWidget *tree_name;
GtkListStore *store;
GtkWidget *text_b_des;
GtkTreeSelection *tree_selection_name;
GtkWidget *entry;

void add_to_list(const gchar *str){
    // GtkListStore *list_store;
    GtkTreeIter iter;

    // store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_name)));

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, str, -1);
}

void on_changed(){
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar* value;

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(tree_selection_name), &model, &iter))
    {
        gtk_tree_model_get(model, &iter, 0, &value, -1);
        gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_b_des), value, -1);
        g_free(value);
    }
}

void on_window_main_destroy()
{
    gtk_main_quit();
}

 
int main(int argc, char *argv[])
{
    
    gtk_init(&argc, &argv);
 
    builder = gtk_builder_new();
    gtk_builder_add_from_file (builder, "tree.glade", NULL);
 
    window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
    gtk_builder_connect_signals(builder, NULL);

    tree_name = GTK_WIDGET(gtk_builder_get_object(builder, "tree_name"));
    // list_store = GTK_TREE_MODEL(gtk_builder_get_object(builder, "list_store"));
    store = GTK_LIST_STORE(gtk_builder_get_object(builder, "list_store"));
    text_b_des = GTK_WIDGET(gtk_builder_get_object(builder, "text_b_des"));
    tree_selection_name = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "tree_selection_name"));
    entry = GTK_WIDGET(gtk_builder_get_object(builder,"entry_nd"));
    
    // gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree_name), FALSE);

    add_to_list("Aliens");
    add_to_list("Leo");
    add_to_list("The Verdict");
    add_to_list("North Face");
    add_to_list("Der Untergang");

    g_object_unref(builder);
    
 
    gtk_widget_show(window);                
    gtk_main();
 
    return 0;
}
 
// called when window is closed
