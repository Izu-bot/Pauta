#include <adwaita.h>

// Estrutura para armazenar widgets importantes

typedef struct {
  GtkWidget *entry;
  GtkWidget *list_box;
} AppWidgets;

static void on_add_button_clicked(GtkButton *btn, gpointer data) {
  AppWidgets *w = data;
  const gchar *text = gtk_editable_get_text (GTK_EDITABLE (w->entry));

  if (g_strcmp0 (text, "") != 0) {
    GtkWidget *row = gtk_label_new (text);
    gtk_list_box_append (GTK_LIST_BOX(w->list_box), row);
    gtk_widget_get_visible (row);

  }
}

static void on_activate(GtkApplication *app, gpointer user_data) {
  GtkWidget *window;
  GtkWidget *main_box;
  GtkWidget *left_box;
  GtkWidget *right_box;
  GtkWidget *entry;
  GtkWidget *add_button;
  GtkWidget *list_box;

  AppWidgets *widgets = g_new(AppWidgets, 1);

// Janela principal
  window = adw_application_window_new(app);
  gtk_window_set_title (GTK_WINDOW (window), "Pauta");
  gtk_window_set_default_size (GTK_WINDOW (window), 600, 400);

// Container horizontal
  main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_widget_set_margin_top (main_box, 12);
  gtk_widget_set_margin_end (main_box, 12);
  gtk_widget_set_margin_bottom (main_box, 12);
  gtk_widget_set_margin_start (main_box, 12);
  gtk_window_set_child (GTK_WINDOW (window), main_box);

// Lado esquerdo: entrada de tarefa + botão
  left_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 12);
  gtk_box_append (GTK_BOX (main_box), left_box);

  entry = gtk_entry_new();
  gtk_editable_set_text (GTK_EDITABLE (entry), "Escreva sua tarefa...");
  gtk_box_append (GTK_BOX (left_box), entry);

  add_button = gtk_button_new_with_label ("Adicionar");
  gtk_box_append (GTK_BOX (left_box), add_button);

// Lado direito: lista as tarefas
  right_box = gtk_scrolled_window_new ();
  gtk_widget_set_vexpand (right_box, TRUE);
  gtk_widget_set_hexpand (right_box, TRUE);

  list_box = gtk_scrolled_window_new ();
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (right_box), list_box);
  gtk_box_append (GTK_BOX (main_box), right_box);

// Salvar ponteiros importantes
  widgets->entry = entry;
  widgets->list_box = list_box;

// Conectar botão
  g_signal_connect (add_button, "clicked", G_CALLBACK (on_add_button_clicked), widgets);

  gtk_window_present (GTK_WINDOW(window));

}

int main(int argc, char *argv[]) {
    g_autoptr(AdwApplication) app = adw_application_new("dev.pauta.Builder", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}

