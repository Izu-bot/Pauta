#include <adwaita.h>
#include <stdio.h>
#include <string.h>

#define TASK_FILE "tarefas.txt"

// Estrutura para armazenar widgets importantes
typedef struct {
    GtkWidget *entry;
    GtkWidget *list_box;
} AppWidgets;

// Declaração antecipada para uso mútuo
static void add_task_to_ui(AppWidgets *w, const gchar *text, gboolean completed);

// Reescreve o arquivo com todas as tarefas atuais da lista
static void rewrite_all_tasks(GtkListBox *list_box) {
    FILE *file = fopen(TASK_FILE, "w");
    GtkListBoxRow *row;
    int i = 0;

    if (!file) return;

    while ((row = gtk_list_box_get_row_at_index(list_box, i))) {
        GtkWidget *row_box = gtk_list_box_row_get_child(row);
        GtkWidget *check = gtk_widget_get_first_child(row_box);
        GtkWidget *label = gtk_widget_get_next_sibling(check);

        if (GTK_IS_LABEL(label)) {
            const gchar *text = gtk_label_get_text(GTK_LABEL(label));
            gboolean active = gtk_check_button_get_active(GTK_CHECK_BUTTON(check));
            fprintf(file, "%d|%s\n", active ? 1 : 0, text);
        }
        i++;
    }

    fclose(file);
}

static void on_remove_button_clicked(GtkButton *btn, gpointer user_data) {
    GtkWidget *row = GTK_WIDGET(user_data);
    GtkWidget *list_box_widget = gtk_widget_get_ancestor(row, GTK_TYPE_LIST_BOX);

    if (list_box_widget) {
        gtk_list_box_remove(GTK_LIST_BOX(list_box_widget), row);
        rewrite_all_tasks(GTK_LIST_BOX(list_box_widget));
    }
}

// Marcar tarefas como concluídas/inconclusas
static void on_check_toggled(GtkCheckButton *check, gpointer user_data) {
    GtkWidget *label = GTK_WIDGET(user_data);
    gboolean active = gtk_check_button_get_active(check);
    GtkWidget *list_box_widget = gtk_widget_get_ancestor(GTK_WIDGET(check), GTK_TYPE_LIST_BOX);
    PangoAttrList *attrs;

    attrs = pango_attr_list_new(); // Cria uma lista de atributos nova
    if (active) {
        // Se estiver ativo, adiciona o atributo de riscado
        PangoAttribute *strike = pango_attr_strikethrough_new(TRUE);
        pango_attr_list_insert(attrs, strike);
    }
    // Define os novos atributos (com ou sem o risco)
    gtk_label_set_attributes(GTK_LABEL(label), attrs);
    pango_attr_list_unref(attrs);

    if (list_box_widget) {
        rewrite_all_tasks(GTK_LIST_BOX(list_box_widget));
    }
}

// Adicionar tarefa à lista visual (UI)
static void add_task_to_ui(AppWidgets *w, const gchar *text, gboolean completed) {
    // Declarações no início do bloco
    GtkWidget *row;
    GtkWidget *row_box;
    GtkWidget *check;
    GtkWidget *label;
    GtkWidget *remove_button;
    PangoAttrList *attrs;

    if (g_strcmp0(text, "") == 0) return;

    row = gtk_list_box_row_new();
    row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    check = gtk_check_button_new();
    label = gtk_label_new(text);
    remove_button = gtk_button_new_from_icon_name("user-trash-symbolic");
    gtk_widget_set_margin_top (remove_button, 6);
    gtk_widget_set_margin_end (remove_button, 6);
    gtk_widget_set_margin_bottom(remove_button, 6);
    gtk_widget_add_css_class (remove_button, "flat");

    gtk_widget_set_hexpand(label, TRUE);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);

    gtk_box_append(GTK_BOX(row_box), check);
    gtk_box_append(GTK_BOX(row_box), label);
    gtk_box_append(GTK_BOX(row_box), remove_button);

    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), row_box);
    gtk_list_box_append(GTK_LIST_BOX(w->list_box), row);

    gtk_check_button_set_active(GTK_CHECK_BUTTON(check), completed);

    // Aplica o estilo inicial (riscado ou não)
    attrs = pango_attr_list_new();
    if (completed) {
      PangoAttribute *strike = pango_attr_strikethrough_new(TRUE);
      pango_attr_list_insert(attrs, strike);
    }
    gtk_label_set_attributes(GTK_LABEL(label), attrs);
    pango_attr_list_unref(attrs);

    // Conectar sinais
    g_signal_connect(remove_button, "clicked", G_CALLBACK(on_remove_button_clicked), row);
    g_signal_connect(check, "toggled", G_CALLBACK(on_check_toggled), label);
}

// Manipulador para o clique do botão "Adicionar"
static void on_add_button_clicked(GtkButton *btn, gpointer data) {
    AppWidgets *w = data;
    const gchar *text = gtk_editable_get_text(GTK_EDITABLE(w->entry));

    if (g_strcmp0(text, "") != 0) {
        add_task_to_ui(w, text, FALSE);
        rewrite_all_tasks(GTK_LIST_BOX(w->list_box));
        gtk_editable_set_text(GTK_EDITABLE(w->entry), "");
    }
}

// Carrega tarefas do arquivo ao iniciar o app
static void load_tasks_from_file(AppWidgets *w) {
    FILE *file = fopen(TASK_FILE, "r");
    char line[512];

    if (!file) return;

    while (fgets(line, sizeof(line), file)) {
        char *sep;
        gboolean completed;
        const gchar *task_text;

        line[strcspn(line, "\n")] = '\0';
        if (g_strcmp0(line, "") == 0) continue;

        sep = strchr(line, '|');
        if (sep) {
            *sep = '\0';
            completed = (strcmp(line, "1") == 0);
            task_text = sep + 1;
            add_task_to_ui(w, task_text, completed);
        }
    }

    fclose(file);
}

static void on_activate(GtkApplication *app, gpointer user_data) {
    // Declaração de todos os widgets no início
    GtkWidget *window;
    GtkWidget *main_view;
    GtkWidget *header;
    GtkWidget *content_box;
    GtkWidget *entry_box;
    GtkWidget *entry;
    GtkWidget *add_button;
    GtkWidget *scrolled_window;
    GtkWidget *list_box;
    AppWidgets *widgets = g_new(AppWidgets, 1);

    window = adw_application_window_new(app);
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    gtk_window_set_title(GTK_WINDOW(window), "Pauta");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);

    main_view = adw_toolbar_view_new();
    adw_application_window_set_content(ADW_APPLICATION_WINDOW(window), main_view);

    header = adw_header_bar_new();
    adw_toolbar_view_add_top_bar(ADW_TOOLBAR_VIEW(main_view), header);

    content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(content_box, 12);
    gtk_widget_set_margin_end(content_box, 12);
    gtk_widget_set_margin_top(content_box, 12);
    gtk_widget_set_margin_bottom(content_box, 12);
    adw_toolbar_view_set_content(ADW_TOOLBAR_VIEW(main_view), content_box);

    entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    gtk_box_append(GTK_BOX(content_box), entry_box);

    entry = gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "Escreva uma nova tarefa...");
    gtk_box_append(GTK_BOX(entry_box), entry);

    add_button = gtk_button_new_with_label("Adicionar");
    gtk_box_append(GTK_BOX(entry_box), add_button);

    scrolled_window = gtk_scrolled_window_new();
    gtk_widget_set_vexpand(scrolled_window, TRUE);
    gtk_box_append(GTK_BOX(content_box), scrolled_window);

    list_box = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(list_box), GTK_SELECTION_NONE);
    gtk_widget_add_css_class(list_box, "boxed-list");
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), list_box);

    widgets->entry = entry;
    widgets->list_box = list_box;

    g_signal_connect(add_button, "clicked", G_CALLBACK(on_add_button_clicked), widgets);
    g_signal_connect(entry, "activate", G_CALLBACK(on_add_button_clicked), widgets);

    load_tasks_from_file(widgets);

    g_signal_connect_swapped(window, "destroy", G_CALLBACK(g_free), widgets);

    gtk_window_present(GTK_WINDOW(window)); // <-- CORREÇÃO
}

int main(int argc, char *argv[]) {
    g_autoptr(AdwApplication) app = adw_application_new("org.gnome.Pauta", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(on_activate), NULL);
    return g_application_run(G_APPLICATION(app), argc, argv);
}
