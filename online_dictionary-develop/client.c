// gcc -o gladewin main.c -Wall `pkg-config --cflags --libs gtk+-3.0` -export-dynamic

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <gtk/gtk.h>

#define MAXLINE 4096 /*max text line length*/
#define SERV_PORT 3000 /*port*/

GtkBuilder *builder; 
GtkWidget *window;
GtkWidget *entry_word;
GtkWidget *entry_edit_word;
GtkWidget *entry_add_word;
GtkWidget	*entry_login;
GtkTextBuffer *text_mean;
GtkTextBuffer *text_add_mean;
GtkTextBuffer *text_edit_mean;
GtkWidget *tree_word;
GtkListStore *list_word;
GtkTreeSelection *tree_selection_word;
GtkWidget *button_add;
GtkWidget *button_edit;
GtkWidget *button_add_accept;
GtkWidget *button_cancel;
GtkWidget *button_login;
GtkWidget *button_guest;
GtkWidget *dialog_add;
GtkWidget *dialog_login;
GtkWidget *dialog_edit;
GtkWidget *button_edit_accept;
GtkWidget *button_edit_cancel;

int sockfd;
struct sockaddr_in servaddr;
char sendline[MAXLINE], recvline[MAXLINE];

void add_to_list(const gchar *str){
  // GtkListStore *list_store;
  GtkTreeIter iter;

  // store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tree_name)));

  gtk_list_store_append(list_word, &iter);
  gtk_list_store_set(list_word, &iter, 0, str, -1);
}

void on_treeview_selection_word_changed(){
  GtkTreeIter iter;
  GtkTreeModel *model;
  gchar* value;
  char intro[MAXLINE];

  if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(tree_selection_word), &model, &iter))
  {
    gtk_tree_model_get(model, &iter, 0, &value, -1);
    gtk_entry_set_text(GTK_ENTRY(entry_edit_word), value);

    strcpy(sendline, "VIEW:");
    strcat(sendline, value);
    send(sockfd, sendline, strlen(sendline)+1, 0);

    if (recv(sockfd, recvline, MAXLINE,0) == 0){
  //error: server terminated prematurely
      perror("The server terminated prematurely"); 
      exit(4);
    }
    strcpy(intro, "");
    strcat(intro,recvline);

    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_mean), intro, -1);
    g_free(value);
  }
}

void on_entry_word_activate(){
  const gchar *word = gtk_entry_get_text(GTK_ENTRY(entry_word));
  strcpy(sendline,"\0");
  strcpy(sendline, "SEARCH:");
  strcat(sendline, word);
  gtk_entry_set_text(GTK_ENTRY(entry_word),"");
  send(sockfd, sendline, strlen(sendline)+1, 0);

  if (recv(sockfd, recvline, MAXLINE,0) == 0){
  //error: server terminated prematurely
    perror("The server terminated prematurely"); 
    exit(4);
  }
  char intro[MAXLINE];
  strcpy(intro, "String received from the server: ");
  strcat(intro,recvline);
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_mean), intro, -1);
  //add_to_list(recvline);
}

void on_entry_word_insert_text(){
  GtkTreeIter iter;
  GtkTreeModel *model;
  if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(tree_selection_word), &model, &iter)){
    gtk_tree_selection_unselect_iter(GTK_TREE_SELECTION(tree_selection_word), &iter);
  }
  // gtk_tree_selection_unselect_all(tree_selection_word);
  gtk_list_store_clear(list_word);
  const gchar *word = gtk_entry_get_text(GTK_ENTRY(entry_word));
  strcpy(sendline,"\0");
  strcpy(sendline, "SEARCH:");
  strcat(sendline, word);
  send(sockfd, sendline, strlen(sendline)+1, 0);

  if (recv(sockfd, recvline, MAXLINE,0) == 0){
  //error: server terminated prematurely
    perror("The server terminated prematurely"); 
    exit(4);
  }
  char* p = strtok(recvline, " ");
  while(p!=NULL){
    add_to_list(p);
    p = strtok(NULL, " ");
  }
}

void on_button_add_clicked(){
  gtk_widget_show(dialog_add);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog_add));
  switch(result){
    case GTK_RESPONSE_ACCEPT:
      printf("OK\n");
      break;
    default:
      printf("Canceled\n");
      break;
  }
  gtk_widget_hide(dialog_add);
}

void on_button_add_accept_clicked(){
  GtkTextIter startPos;
  gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(text_add_mean), &startPos);
  GtkTextIter endPos;
  gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(text_add_mean), &endPos);
  const gchar *des = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(text_add_mean), &startPos, &endPos, FALSE);
  const gchar *word = gtk_entry_get_text(GTK_ENTRY(entry_add_word));
  strcpy(sendline,"\0");
  strcpy(sendline, "ADD:");
  strcat(sendline, word);
  strcat(sendline, ";");
  strcat(sendline, des);
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_add_mean),"",-1);
  gtk_entry_set_text(GTK_ENTRY(entry_add_word),"");
  send(sockfd, sendline, strlen(sendline)+1, 0);

  if (recv(sockfd, recvline, MAXLINE,0) == 0){
  //error: server terminated prematurely
    perror("The server terminated prematurely"); 
    exit(4);
  }
  char intro[MAXLINE];
  strcpy(intro, "String received from the server: ");
  strcat(intro,recvline);
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_mean), intro, -1);
  gtk_dialog_response(GTK_DIALOG(dialog_add), GTK_RESPONSE_ACCEPT);
}

void on_button_cancel_clicked(){
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_add_mean),"",-1);
  gtk_entry_set_text(GTK_ENTRY(entry_add_word),"");
  gtk_dialog_response(GTK_DIALOG(dialog_add), GTK_RESPONSE_NONE);
}

void on_button_edit_clicked(){
  gtk_widget_show(dialog_edit);
  gint result = gtk_dialog_run(GTK_DIALOG(dialog_edit));
  switch(result){
    case GTK_RESPONSE_ACCEPT:
      printf("OK\n");
      break;
    default:
      printf("Canceled\n");
      break;
  }
  gtk_widget_hide(dialog_edit);
}

void on_button_edit_accept_clicked(){
  GtkTextIter startPos;
  gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(text_edit_mean), &startPos);
  GtkTextIter endPos;
  gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(text_edit_mean), &endPos);
  const gchar *des = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(text_edit_mean), &startPos, &endPos, FALSE);
  const gchar *word = gtk_entry_get_text(GTK_ENTRY(entry_edit_word));
  strcpy(sendline,"\0");
  strcpy(sendline, "EDIT:");
  strcat(sendline, word);
  strcat(sendline, ";");
  strcat(sendline, des);
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_edit_mean),"",-1);
  gtk_entry_set_text(GTK_ENTRY(entry_edit_word),"");
  send(sockfd, sendline, strlen(sendline)+1, 0);

  if (recv(sockfd, recvline, MAXLINE,0) == 0){
  //error: server terminated prematurely
    perror("The server terminated prematurely"); 
    exit(4);
  }
  char intro[MAXLINE];
  strcpy(intro, "String received from the server: ");
  strcat(intro,recvline);
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_mean), intro, -1);
  gtk_dialog_response(GTK_DIALOG(dialog_edit), GTK_RESPONSE_ACCEPT);
}

void on_button_edit_cancel_clicked(){
  gtk_text_buffer_set_text(GTK_TEXT_BUFFER(text_edit_mean),"",-1);
  gtk_entry_set_text(GTK_ENTRY(entry_edit_word),"");
  gtk_dialog_response(GTK_DIALOG(dialog_edit), GTK_RESPONSE_NONE);
}

void on_button_login_clicked(){
  const gchar* user = gtk_entry_get_text(GTK_ENTRY(entry_login));
  if (strcmp(user, "admin") == 0) {
    gtk_entry_set_text(GTK_ENTRY(entry_login), "");
    gtk_dialog_response(GTK_DIALOG(dialog_login), GTK_RESPONSE_ACCEPT);
  } else {
    gtk_entry_set_text(GTK_ENTRY(entry_login), "");
    gtk_dialog_response(GTK_DIALOG(dialog_login), GTK_RESPONSE_NONE);
  }
}

void on_button_guest_clicked(){
  gtk_widget_hide(button_add);  
  gtk_dialog_response(GTK_DIALOG(dialog_login), GTK_RESPONSE_DELETE_EVENT);
}

void on_window_main_destroy()
{
  gtk_main_quit();
}

int main(int argc, char **argv) {
  gint result;	
  //basic check of the arguments
  //additional checks can be inserted
  if (argc !=2) {
    perror("Usage: TCPClient <IP address of the server"); 
    exit(1);
  }

  //Create a socket for the client
  //If sockfd<0 there was an error in the creation of the socket
  if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) <0) {
    perror("Problem in creating the socket");
    exit(2);
  }

  //Creation of the socket
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr= inet_addr(argv[1]);
  servaddr.sin_port =  htons(SERV_PORT); //convert to big-endian order

  //Connection of the client to the socket 
  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0) {
    perror("Problem in connecting to the server");
    exit(3);
  }

  gtk_init(&argc, &argv);
  builder = gtk_builder_new();
  gtk_builder_add_from_file (builder, "Window.glade", NULL);
  window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  gtk_builder_connect_signals(builder, NULL);


  entry_word = GTK_WIDGET(gtk_builder_get_object(builder, "entry_word"));
  text_mean = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "text_mean"));
  text_edit_mean = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "text_edit_mean"));
  tree_word = GTK_WIDGET(gtk_builder_get_object(builder,"tree_word"));
  list_word = GTK_LIST_STORE(gtk_builder_get_object(builder,"list_word"));
  tree_selection_word = GTK_TREE_SELECTION(gtk_builder_get_object(builder,"treeview_selection_word"));
  button_add = GTK_WIDGET(gtk_builder_get_object(builder, "button_add"));
  button_add_accept = GTK_WIDGET(gtk_builder_get_object(builder, "button_add_accept"));
  button_cancel = GTK_WIDGET(gtk_builder_get_object(builder,"button_cancel"));
  dialog_add = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_add"));
  text_add_mean = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "text_add_mean"));
  entry_add_word = GTK_WIDGET(gtk_builder_get_object(builder, "entry_add_word"));
  dialog_login = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_login"));
  entry_login = GTK_WIDGET(gtk_builder_get_object(builder, "entry_login"));
  button_login = GTK_WIDGET(gtk_builder_get_object(builder, "button_login"));
  dialog_edit = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_edit"));
  button_guest = GTK_WIDGET(gtk_builder_get_object(builder, "button_guest"));
  button_edit = GTK_WIDGET(gtk_builder_get_object(builder, "button_edit"));
  button_edit_accept = GTK_WIDGET(gtk_builder_get_object(builder, "button_edit_accept"));
  button_edit_cancel = GTK_WIDGET(gtk_builder_get_object(builder, "button_edit_cancel"));
  entry_edit_word = GTK_WIDGET(gtk_builder_get_object(builder, "entry_edit_word"));

  g_object_unref(builder);

  do{
    gtk_widget_show(dialog_login);
    result = gtk_dialog_run(GTK_DIALOG(dialog_login));
    switch(result){
      case GTK_RESPONSE_ACCEPT:
        printf("You are admin\n");
        break;
      case GTK_RESPONSE_NONE:
        printf("You aren't admin\n");
        break;
      default:
        printf("Welcome guest\n");
        break;
    }
    gtk_widget_hide(dialog_login);
  } while(result == GTK_RESPONSE_NONE);

  gtk_widget_show(window);             
  gtk_main();


  return 0;	

  exit(0);
}
