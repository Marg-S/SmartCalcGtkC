#include "smartcalc.h"

#include "gtk/gtk.h"

void draw_clear(cairo_t *cr) {
  cairo_set_source_rgb(cr, 0.98, 0.98, 0.98);
  cairo_paint(cr);
}

double get_step(double max, double min) {
  double step = (max - min) / 20.;
  if (fabs(step) >= 1)
    step = round(step);
  else {
    int i = 0;
    while (fabs(round(step * pow(10, i))) == 0) i++;
    step = round(step * pow(10, i)) / pow(10, i);
  }
  return step;
}

void draw_grid(cairo_t *cr, cairo_matrix_t *values) {
  double scale_x = values->x0;
  double scale_y = values->y0;
  double xmin = values->xx;
  double xmax = values->yx;
  double ymin = values->xy;
  double ymax = values->yy;
  enum size { hfont = 14, vfont = 14, offset_1 = 6, offset_2 = 10 };
  double step = get_step(xmax, xmin);
  double start_x = xmin - fmod(xmin, step);
  cairo_set_font_size(cr, vfont);
  for (double x = start_x; x < xmax; x += step) {
    cairo_move_to(cr, x * scale_x, -ymin * scale_y);
    cairo_line_to(cr, x * scale_x, -ymax * scale_y);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_stroke(cr);
    if (fabs(fmod(x, 5 * step)) < step) {
      char str[LEN] = {0};
      sprintf(str, "%g", x);
      cairo_set_source_rgb(cr, 0, 0, 0);
      double h = x * scale_x - hfont / 2;
      double v = (vfont > -ymin * scale_y) ? -ymin - vfont : vfont;
      cairo_move_to(cr, h, v);
      int in = (h >= xmin * scale_x &&
                h + hfont * 1.5 * (int)strlen(str) < xmax * scale_x);
      if (fabs(x) > 1e-7 && in) cairo_show_text(cr, str);
      cairo_stroke(cr);
    }
  }
  step = get_step(ymax, ymin);
  double start_y = ymin - fmod(ymin, step);
  for (double y = start_y; y < ymax; y += step) {
    cairo_move_to(cr, xmin * scale_x, -y * scale_y);
    cairo_line_to(cr, xmax * scale_x, -y * scale_y);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    cairo_stroke(cr);
    if (fabs(fmod(y, 5 * step)) < step) {
      char str[LEN] = {0};
      sprintf(str, "%g", y);
      cairo_set_source_rgb(cr, 0, 0, 0);
      double h = -hfont * (int)strlen(str);
      h = (h < xmin * scale_x) ? xmin + hfont : h;
      cairo_move_to(cr, h, -y * scale_y + vfont / 2);
      if (fabs(y) > 1e-7) cairo_show_text(cr, str);
      cairo_stroke(cr);
    }
  }
}

void draw_axes(cairo_t *cr, cairo_matrix_t *values) {
  double scale_x = values->x0;
  double scale_y = values->y0;
  double xmin = values->xx * scale_x;
  double xmax = values->yx * scale_x;
  double ymin = values->xy * scale_y;
  double ymax = values->yy * scale_y;

  enum size { hfont = 18, vfont = 14, offset_1 = 6, offset_2 = 10 };
  cairo_set_line_width(cr, 2);
  cairo_set_source_rgb(cr, 0, 0, 0);
  cairo_set_font_size(cr, vfont);

  cairo_move_to(cr, xmin, 0);
  cairo_line_to(cr, xmax, 0);
  cairo_move_to(cr, 0, -ymin);
  cairo_line_to(cr, 0, -ymax);
  cairo_move_to(cr, -hfont, vfont);
  cairo_show_text(cr, "0");

  cairo_move_to(cr, xmax - offset_2, offset_1);
  cairo_line_to(cr, xmax, 0);
  cairo_move_to(cr, xmax - offset_2, -offset_1);
  cairo_line_to(cr, xmax, 0);
  cairo_move_to(cr, xmax - hfont / 2, vfont * 1.5);
  cairo_show_text(cr, "x");

  cairo_move_to(cr, -offset_1, -ymax + offset_2);
  cairo_line_to(cr, 0, -ymax);
  cairo_move_to(cr, offset_1, -ymax + offset_2);
  cairo_line_to(cr, 0, -ymax);
  cairo_move_to(cr, -hfont, -ymax + vfont);
  cairo_show_text(cr, "y");

  cairo_stroke(cr);
}

void line(cairo_t *cr, double last, double last_2, double x, double step_x,
          double y, double step_y, double ymin, double ymax) {
  if (last * y < 0) {
    if ((last_2 < last && last_2 > y) || (last_2 < y && last_2 > last))
      cairo_line_to(cr, x / step_x, -y / step_y);
    else {
      double edge_1 = -ymax, edge_2 = -ymin;
      if (y > last) edge_1 = -ymin, edge_2 = -ymax;
      cairo_line_to(cr, x / step_x, edge_1 / step_y);
      cairo_move_to(cr, x / step_x, edge_2 / step_y);
    }
  } else
    cairo_line_to(cr, x / step_x, -y / step_y);
}

void draw_line(cairo_t *cr, double step_x, double xmin, double xmax,
               double step_y, double ymin, double ymax,
               const char *text_input) {
  double y, last = 0;
  for (double x = xmin; x < xmax; x += step_x) {
    double last_2 = 0;
    int error = calculate(text_input, x, &y);

    if (!error && isfinite(y) && isfinite(last)) {
      if (last * y < 0) error = calculate(text_input, x - step_x / 2, &last_2);
      if (!error) line(cr, last, last_2, x, step_x, y, step_y, ymin, ymax);
    }
    last = y;
  }
}

void show_dialog(char c, GtkWidget *parent) {
  GtkWindow *win = GTK_WINDOW(gtk_widget_get_parent(
      gtk_widget_get_parent(gtk_widget_get_parent(parent))));
  char message[LEN] = "Invalid input";
  if (c == 'x') strcpy(message, "Error: xmin >= xmax");
  if (c == 'y') strcpy(message, "Error: ymin >= ymax");
#if (GTK_MINOR_VERSION >= 10)
  GtkAlertDialog *dialog = gtk_alert_dialog_new("Error");
  gtk_alert_dialog_set_detail(dialog, message);
  gtk_alert_dialog_show(dialog, win);
#else
  GtkDialogFlags flags = GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL;
  GObject *dialog = G_OBJECT(gtk_message_dialog_new(
      win, flags, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "%s", message));
  gtk_window_present(GTK_WINDOW(dialog));
  g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(gtk_window_destroy),
                   NULL);
#endif
}

void draw_graph(GtkDrawingArea *graph_area, cairo_t *cr, int width, int height,
                gpointer data) {
  GtkEntry *entry_input = GTK_ENTRY(data);
  const char *text_input = gtk_editable_get_text(GTK_EDITABLE(entry_input));

  GtkEntry *entry_xmin =
      GTK_ENTRY(g_object_get_data(G_OBJECT(graph_area), "entry-region-xmin"));
  double xmin = atof(gtk_editable_get_text(GTK_EDITABLE(entry_xmin)));
  GtkEntry *entry_xmax =
      GTK_ENTRY(g_object_get_data(G_OBJECT(graph_area), "entry-region-xmax"));
  double xmax = atof(gtk_editable_get_text(GTK_EDITABLE(entry_xmax)));
  GtkEntry *entry_ymin =
      GTK_ENTRY(g_object_get_data(G_OBJECT(graph_area), "entry-region-ymin"));
  double ymin = atof(gtk_editable_get_text(GTK_EDITABLE(entry_ymin)));
  GtkEntry *entry_ymax =
      GTK_ENTRY(g_object_get_data(G_OBJECT(graph_area), "entry-region-ymax"));
  double ymax = atof(gtk_editable_get_text(GTK_EDITABLE(entry_ymax)));
  double y;
  if (xmin >= xmax)
    show_dialog('x', gtk_widget_get_parent(GTK_WIDGET(graph_area)));
  else if (ymin >= ymax)
    show_dialog('y', gtk_widget_get_parent(GTK_WIDGET(graph_area)));
  else if (strlen(text_input) != 0 && calculate(text_input, 0, &y))
    show_dialog('e', gtk_widget_get_parent(GTK_WIDGET(graph_area)));
  else {
    double step_x = (xmax - xmin) / width;
    double step_y = (ymax - ymin) / height;
    double scale_x = width / (xmax - xmin);
    double scale_y = height / (ymax - ymin);
    double start_x = -xmin * scale_x;
    double start_y = ymax * scale_y;
    cairo_matrix_t values = {xmin, xmax, ymin, ymax, scale_x, scale_y};
    draw_clear(cr);
    cairo_translate(cr, start_x, start_y);
    draw_grid(cr, &values);
    draw_axes(cr, &values);
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 1, 0, 0);
    draw_line(cr, step_x, xmin, xmax, step_y, ymin, ymax, text_input);
    cairo_stroke(cr);
  }
}

void clicked_ac(GtkEntry *entry_input, GtkEntry *entry_result,
                GtkEntry *entry_x) {
  gtk_editable_delete_text(GTK_EDITABLE(entry_input), 0, -1);
  if (entry_result) gtk_editable_delete_text(GTK_EDITABLE(entry_result), 0, -1);
  if (entry_x) gtk_editable_set_text(GTK_EDITABLE(entry_x), "0");
}

void clicked_del(GtkEntry *entry_input) {
  int pos = gtk_editable_get_position(GTK_EDITABLE(entry_input));
  gtk_editable_delete_text(GTK_EDITABLE(entry_input), pos - 1, pos);
}

void clicked_plot(GtkButton *button) {
  GtkWidget *graph_area =
      GTK_WIDGET(g_object_get_data(G_OBJECT(button), "graph-area"));
  gtk_widget_queue_draw(graph_area);
}

void clicked_equal(GtkEntry *entry_input, GtkEntry *entry_result,
                   GtkEntry *entry_x) {
  char text_result[LEN], text_input[LEN];
  double x = atof(gtk_editable_get_text(GTK_EDITABLE(entry_x)));
  strcpy(text_input, gtk_editable_get_text(GTK_EDITABLE(entry_input)));
  int pos = (int)gtk_entry_get_text_length(GTK_ENTRY(entry_input));
  if (text_input[pos - 1] != '=') {
    if (!isspace(text_input[pos - 1]))
      gtk_editable_insert_text(GTK_EDITABLE(entry_input), " ", 1, &pos);
    gtk_editable_insert_text(GTK_EDITABLE(entry_input), "=", 1, &pos);
    gtk_editable_set_position(GTK_EDITABLE(entry_input), -1);
  } else
    text_input[pos - 1] = '\0';
  double value;
  int error = calculate(text_input, x, &value);
  if (!isfinite(value)) error = 2;
  if (!error) sprintf(text_result, "%.15g", value);
  if (error == 1) sprintf(text_result, "Division by zero is not possible");
  if (error == 2) sprintf(text_result, "Invalid input");
  gtk_editable_set_text(GTK_EDITABLE(entry_result), text_result);
}

int is_number(int c) { return (isdigit(c) || c == '.' || c == 'x'); }

void output(GtkEntry *entry_input, GtkEntry *entry_result, const char *label) {
  if (entry_result && gtk_entry_get_text_length(entry_result) > 0) {
    gtk_editable_delete_text(GTK_EDITABLE(entry_input), 0, -1);
    gtk_editable_delete_text(GTK_EDITABLE(entry_result), 0, -1);
  }
  char text_input[LEN];
  strcpy(text_input, gtk_editable_get_text(GTK_EDITABLE(entry_input)));
  char *last = NULL;
  int length_input = (int)gtk_entry_get_text_length(GTK_ENTRY(entry_input));
  for (int i = length_input - 1; i >= 0 && (!last || isspace(*last)); i--) {
    last = text_input + i;
  }

  int start = (length_input == 0 || *last == '(');
  int oper = ((is_operator(*label) && *label != '+' && *label != '-') ||
              strcmp(label, "exp") == 0);

  if (!(oper && (start || (!is_number(*last) && *last != ')')))) {
    int pos = gtk_editable_get_position(GTK_EDITABLE(entry_input));

    if (*label == '.' && start)
      gtk_editable_insert_text(GTK_EDITABLE(entry_input), "0", 1, &pos);
    if (!start && *label != ')' &&
        !(is_number(*label) && (is_number(*last) || *last == 'e')))
      gtk_editable_insert_text(GTK_EDITABLE(entry_input), " ", 1, &pos);
    gtk_editable_insert_text(GTK_EDITABLE(entry_input), label, strlen(label),
                             &pos);
    gtk_editable_set_position(GTK_EDITABLE(entry_input), pos);
  }
}

void clicked_exp(GtkEntry *entry_input) {
  const char *text_input = gtk_editable_get_text(GTK_EDITABLE(entry_input));
  int pos = gtk_editable_get_position(GTK_EDITABLE(entry_input));
  if (isdigit(text_input[pos - 1]) || text_input[pos - 1] == '.')
    gtk_editable_insert_text(GTK_EDITABLE(entry_input), "e", 1, &pos);
  gtk_editable_set_position(GTK_EDITABLE(entry_input), pos);
}

void clicked_pi_e(GtkEntry *entry_input, int pi) {
  char *str = pi ? "3.14159265358979" : "2.71828182845904";
  int pos = gtk_editable_get_position(GTK_EDITABLE(entry_input));
  gtk_editable_insert_text(GTK_EDITABLE(entry_input), str, strlen(str), &pos);
  gtk_editable_set_position(GTK_EDITABLE(entry_input), pos);
}

void clicked_sign(GtkEntry *entry_input) {
  char text_input[LEN];
  strcpy(text_input, gtk_editable_get_text(GTK_EDITABLE(entry_input)));
  int pos = gtk_editable_get_position(GTK_EDITABLE(entry_input));
  if (is_number(text_input[pos]) || is_number(text_input[pos - 1])) {
    int pos_backup = pos;
    for (int i = pos; i >= 0 && is_number(text_input[pos - 1]); i--) pos--;
    if (text_input[pos - 1] == '-')
      gtk_editable_delete_text(GTK_EDITABLE(entry_input), pos - 1, pos);
    else {
      if (text_input[pos - 1] == '+') {
        gtk_editable_delete_text(GTK_EDITABLE(entry_input), pos - 1, pos);
        pos--;
        gtk_editable_insert_text(GTK_EDITABLE(entry_input), "-", 1, &pos);
        gtk_editable_set_position(GTK_EDITABLE(entry_input), pos_backup);
      } else {
        gtk_editable_insert_text(GTK_EDITABLE(entry_input), "-", 1, &pos);
        gtk_editable_set_position(GTK_EDITABLE(entry_input), pos_backup + 1);
      }
    }
  }
}

void button_clicked(GtkButton *button, gpointer data) {
  GtkEntry *entry_input = GTK_ENTRY(data);
  gtk_entry_grab_focus_without_selecting(GTK_ENTRY(entry_input));

  GtkEntry *entry_result = NULL, *entry_x = NULL;
  const char *name_entry_input = gtk_widget_get_name(GTK_WIDGET(entry_input));
  if (strcmp(name_entry_input, "entry-calc-input") == 0) {
    entry_result =
        GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry-calc-result"));
    entry_x = GTK_ENTRY(g_object_get_data(G_OBJECT(button), "entry-calc-x"));
  }

  const char *label = gtk_button_get_label(button);

  if (strcmp(label, "AC") == 0) {
    clicked_ac(entry_input, entry_result, entry_x);
  } else if (strcmp(label, "DEL") == 0) {
    clicked_del(entry_input);
  } else if (strcmp(label, "plot") == 0) {
    clicked_plot(button);
  } else if (strcmp(label, "=") == 0) {
    clicked_equal(entry_input, entry_result, entry_x);
  } else if (strcmp(label, "pi") == 0) {
    clicked_pi_e(entry_input, 1);
  } else if (strcmp(label, "e") == 0) {
    clicked_pi_e(entry_input, 0);
  } else if (strcmp(label, "+/-") == 0) {
    clicked_sign(entry_input);
  } else if (strcmp(label, "exp") == 0) {
    clicked_exp(entry_input);
  } else {
    output(entry_input, entry_result, label);
  }
}

static void app_activate(GtkApplication *app) {
  GtkBuilder *builder = gtk_builder_new_from_file("smartcalc_view.ui");
  GObject *window = gtk_builder_get_object(builder, "window");
  gtk_window_set_application(GTK_WINDOW(window), app);
  GObject *entry_input = gtk_builder_get_object(builder, "entry-calc-input");
  gtk_entry_grab_focus_without_selecting(GTK_ENTRY(entry_input));

  GObject *entry_result = gtk_builder_get_object(builder, "entry-calc-result");
  GObject *entry_x = gtk_builder_get_object(builder, "entry-calc-x");
  GObject *entry_graph_input =
      gtk_builder_get_object(builder, "entry-graph-input");
  GObject *graph_area = gtk_builder_get_object(builder, "graph-area");
  gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(graph_area), draw_graph,
                                 entry_graph_input, NULL);

  GSList *widgets = gtk_builder_get_objects(builder);
  for (GSList *lp = widgets; lp != NULL; lp = lp->next) {
    GObject *object = G_OBJECT(lp->data);
    if (g_type_is_a(G_TYPE_FROM_INSTANCE(object), GTK_TYPE_ENTRY)) {
      const char *str = gtk_widget_get_name(GTK_WIDGET(object));
      if (strstr(str, "region")) g_object_set_data(graph_area, str, object);
    }
    if (g_type_is_a(G_TYPE_FROM_INSTANCE(object), GTK_TYPE_BUTTON)) {
      if (strstr(gtk_widget_get_name(GTK_WIDGET(object)), "graph")) {
        g_object_set_data(object, "graph-area", graph_area);
        g_signal_connect(object, "clicked", G_CALLBACK(button_clicked),
                         entry_graph_input);
      } else {
        g_object_set_data(object, "entry-calc-result", entry_result);
        g_object_set_data(object, "entry-calc-x", entry_x);
        g_signal_connect(object, "clicked", G_CALLBACK(button_clicked),
                         entry_input);
      }
    }
  }

  gtk_window_present(GTK_WINDOW(window));
  g_slist_free(widgets);
  g_object_unref(builder);
}

int main() {
  GtkApplication *app = gtk_application_new("my.smart.calc", 0);
  g_signal_connect(app, "activate", G_CALLBACK(app_activate), NULL);
  int status = g_application_run(G_APPLICATION(app), 0, NULL);
  g_object_unref(app);

  return status;
}