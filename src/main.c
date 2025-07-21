#include <adwaita.h>
#include <stdio.h>

#define TASK_FILE "tarefas.txt"

// Estrutura para armazenar widgets importantes

typedef struct {
  GtkWidget *entry;
  GtkWidget *list_box;
} AppWidgets;

// Salvar uma nova tarefa
static void save_task_to_file(const gchar *text) {
  FILE *file = fopen(TASK_FILE, "a");
  if (file) {
    fprintf(file, "%s\n", text);
    fclose(file);
  }
}

// Reescreve o arquivo com todas as tarefas atuais da lista
static void rewrite_all_tasks(GtkListBox *list_box) {
  FILE *file = fopen(TASK_FILE, "w");
  if (!file) return;

  for (GtkListBoxRow *row = gtk_list_box_get_row_at_index (list_box, 0);
       row != NULL;
       row = gtk_list_box_get_row_at_index (list_box, gtk_list_box_row_get_index (row) + 1)) {
         GtkWidget *label = gtk_list_box_row_get_child (row);
         const gchar *text = gtk_label_get_text (GTK_LABEL(label));
         fprintf(file, "%s\n", text);
       }

  fclose(file);
}

static void on_remove_button_clicked(GtkButton *btn, gpointer user_data) {
  GtkWidget *row = GTK_WIDGET (user_data);
  GtkWidget *list_box = gtk_widget_get_parent (row);
  gtk_list_box_remove (GTK_LIST_BOX (list_box), row);
  rewrite_all_tasks (GTK_LIST_BOX (list_box));
}

// Adicionar tarefa à lista visual e remoção
static void add_task(AppWidgets *w, const gchar *text) {
  if (g_strcmp0(text, "") != 0) {
    GtkWidget *row_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
    GtkWidget *label = gtk_label_new (text);
    GtkWidget *remove_button = gtk_button_new_with_label ("Remover");

    gtk_box_append(GTK_BOX (row_box), label);
    gtk_box_append(GTK_BOX (row_box), remove_button);

    GtkWidget *row = gtk_list_box_row_new ();
    gtk_list_box_row_set_child (GTK_LIST_BOX_ROW (row), row_box);
    gtk_list_box_append (GTK_LIST_BOX (w->list_box), row);

    g_signal_connect (remove_button, "clicked", G_CALLBACK (on_remove_button_clicked), row);

    save_task_to_file (text);
  }
}

// Carrega tarefas do arquivo ao iniciar o app
static void load_task_from_file(AppWidgets *w) {
  FILE *file = fopen(TASK_FILE, "r");
  if (!file) return;

  gchar line[512];
  while (fgets(line, sizeof(line), file)) {
    line[strcspn (line, "\n")] = '\0'; // remove o \n
    if (g_strcmp0 (line, "") != 0) {
        add_task (w, line);
    }
  }

  fclose(file);
}

static void on_add_button_clicked(GtkButton *btn, gpointer data) {
  AppWidgets *w = data;
  const gchar *text = gtk_editable_get_text (GTK_EDITABLE (w->entry));

  add_task(w, text);
  gtk_editable_set_text (GTK_EDITABLE (w->entry), "");
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

// Carrega as tarefas do arquivo
  load_task_from_file (widgets);

  gtk_window_present (GTK_WINDOW(window));

}

int main(int argc, char *argv[]) {
    g_autoptr(AdwApplication) app = adw_application_new("dev.pauta.Builder", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}

