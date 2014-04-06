#include <gtk/gtk.h>
#include <math.h>




#define TABLES_COUNT    3
#define POINT_RADIUS    4




struct calc_data_;
struct table_;
struct window_main_;

typedef struct calc_data_ calc_data;
typedef struct table_ table;
typedef struct window_main_ window_main;




struct calc_data_ {
	double y1[3], y2[3], x1[3], x2[3];
	double a[3], b[3];
	int sectors_count;
};

struct table_ {
	GtkAllocation alloc;
	double color[3];
	double y1, y2;
	double gx0, gy0, gx1, gy1;
	double cpy0, cpy1;
	guint16* color1;
	guint16* color2;
};

struct window_main_ {
	GtkWidget* window_main;
	GtkWidget* drawing_area;
	GdkColor color1;
	GdkColor color2;
	table tables[3];
	table* table_change;
	guint16* color_change;
};




void calc_circle(window_main* wm, calc_data* cd);
void draw_circle(window_main* wm, cairo_t* c, GtkAllocation* allocation, calc_data* cd);
void draw_circle_and_tables(window_main* wm, cairo_t* c, GtkAllocation* allocation);
void draw_point(cairo_t* c, double x, double y);
void draw_table(table* tbl, window_main* wm, cairo_t* c, calc_data* cd);
void draw_tables(window_main* wm, cairo_t* c, GtkAllocation* alloc_tables, calc_data* cd);
gboolean drawing_area_button_press_event(GtkWidget *widget,
               GdkEventButton *event,
               window_main* wm);
gboolean drawing_area_button_release_event(GtkWidget *widget,
               GdkEventButton *event,
               window_main* wm);
gboolean drawing_area_motion_notify_event(GtkWidget *widget,
               GdkEventButton *event,
               window_main* wm);
static gboolean expose_event(GtkWidget* widget, GdkEventExpose* event, window_main* wm);
void tables_press(window_main* wm, table* tbl, double x, double y, int state);
void window_main_create(window_main* wm);




void window_main_create(window_main* wm)
{
	wm->tables[0].color1 = &wm->color1.red;
	wm->tables[0].color2 = &wm->color2.red;
	wm->tables[1].color1 = &wm->color1.green;
	wm->tables[1].color2 = &wm->color2.green;
	wm->tables[2].color1 = &wm->color1.blue;
	wm->tables[2].color2 = &wm->color2.blue;

	wm->table_change = NULL;
	wm->color_change = NULL;

	wm->window_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	wm->drawing_area = gtk_drawing_area_new();
	gtk_widget_add_events(GTK_WIDGET(wm->drawing_area),
			GDK_BUTTON_PRESS_MASK |
			GDK_BUTTON_RELEASE_MASK |
			GDK_POINTER_MOTION_MASK);
	gtk_container_add(GTK_CONTAINER(wm->window_main), wm->drawing_area);
	g_signal_connect (GTK_WIDGET(wm->drawing_area), "button-press-event", G_CALLBACK (drawing_area_button_press_event), wm);
	g_signal_connect (GTK_WIDGET(wm->drawing_area), "button-release-event", G_CALLBACK (drawing_area_button_release_event), wm);
	g_signal_connect (GTK_WIDGET(wm->drawing_area), "motion-notify-event", G_CALLBACK (drawing_area_motion_notify_event), wm);
	g_signal_connect(wm->drawing_area, "expose-event", G_CALLBACK(expose_event), (gpointer) wm);
	g_signal_connect_swapped(wm->window_main, "destroy", G_CALLBACK(gtk_main_quit), NULL);
	gtk_window_resize(GTK_WINDOW(wm->window_main), 1000, 800);
	gtk_widget_show_all(wm->window_main);
}

static gboolean expose_event(GtkWidget* widget, GdkEventExpose* event, window_main* wm)
{
	cairo_t* c;
	GtkAllocation allocation;
	
	
	gtk_widget_get_allocation(wm->drawing_area, &allocation);
	c = gdk_cairo_create(wm->drawing_area->window);

	int offset = 15;
	
	
	cairo_set_line_width(c, 1);

	allocation.x = offset;
	allocation.y = offset;
	allocation.width -= (offset * 2);
	allocation.height -= (offset * 2);

	cairo_rectangle(c, allocation.x, allocation.y, allocation.width, allocation.height);

	cairo_set_source_rgb(c, 1, 1, 1);
	cairo_fill_preserve(c);
	cairo_set_source_rgb(c, 0, 0, 0);
	cairo_stroke(c);

	allocation.x += offset;
	allocation.y += offset;
	allocation.width -= (offset * 2);
	allocation.height -= (offset * 2);

	draw_circle_and_tables(wm, c, &allocation);

	cairo_destroy(c);
}

void draw_circle_and_tables(window_main* wm, cairo_t* c, GtkAllocation* allocation)
{
	GtkAllocation alloc_tables;
	calc_data cd;

	
	alloc_tables.width = allocation->width*40/100;

	alloc_tables.x = allocation->x + allocation->width - alloc_tables.width;
	alloc_tables.y = allocation->y;
	alloc_tables.height = allocation->height;

	allocation->width = allocation->width - 15 - alloc_tables.width;

	calc_circle(wm, &cd);
	draw_circle(wm, c, allocation, &cd);
	draw_tables(wm, c, &alloc_tables, &cd);
}

void calc_circle(window_main* wm, calc_data* cd)
{
	double sectors_count = 16;
	double sector_max;
	int i;


	cd->sectors_count = sectors_count;
	sector_max = sectors_count - 1;
	
	for (i = 0; i < TABLES_COUNT; ++i)
	{
		cd->x1[i] = 0;
		cd->x2[i] = sector_max;
	}
	
	cd->y1[0] = ((double) wm->color1.red) / 65535.0;
	cd->y1[1] = ((double) wm->color1.green) / 65535.0;
	cd->y1[2] = ((double) wm->color1.blue) / 65535.0;
	
	cd->y2[0] = ((double) wm->color2.red) / 65535.0;
	cd->y2[1] = ((double) wm->color2.green) / 65535.0;
	cd->y2[2] = ((double) wm->color2.blue) / 65535.0;

	for (i = 0; i < TABLES_COUNT; ++i)
	{
		cd->a[i] = (cd->y2[i] - cd->y1[i]) / cd->x2[i];
		cd->b[i] = cd->y1[i];
	}
}

void draw_circle(window_main* wm, cairo_t* c, GtkAllocation* allocation, calc_data* cd)
{
	double y[3];
	double cur_sector;
	double angle1, angle2;
	int i;
	const int percent0 = 40;
	const int percent1 = 85;
	
	
	cairo_set_line_width(c, 1);
	
	for (cur_sector = 0, angle1 = 0; cur_sector < cd->sectors_count; ++cur_sector)
	{
		for (i = 0; i < TABLES_COUNT; ++i)
			y[i] = cd->a[i]*cur_sector + cd->b[i];

		angle2 = 2 * (cur_sector+1) * M_PI / cd->sectors_count;
		
		cairo_arc(c,
			allocation->width / 2 + allocation->x,
			allocation->height / 2 + allocation->y,
			allocation->width < allocation->height ?
			allocation->width*percent1 / 200:
			allocation->height*percent1 / 200,
			angle1, angle2);
		cairo_arc_negative(c,
			allocation->width / 2 + allocation->x,
			allocation->height / 2 + allocation->y,
			allocation->width < allocation->height ?
			allocation->width*percent0 / 200:
			allocation->height*percent0 / 200,
			angle2, angle1);
		cairo_close_path(c);
		
		cairo_set_source_rgb(c, y[0], y[1], y[2]); 
		cairo_fill_preserve(c);
		cairo_set_source_rgb(c, 0, 0, 0);
		cairo_stroke(c);
		
		angle1 = angle2;
	}
}

void draw_tables(window_main* wm, cairo_t* c, GtkAllocation* alloc_tables, calc_data* cd)
{
	int interval = 15;
	int y, i;
	GtkAllocation alloc;


	alloc.x = alloc_tables->x;
	alloc.y = alloc_tables->y;
	alloc.width = alloc_tables->width;
	alloc.height = (alloc_tables->height - interval*2) / TABLES_COUNT;

	for (i = 0; i < TABLES_COUNT; ++i) {
		wm->tables[i].alloc.x = alloc.x;
		wm->tables[i].alloc.y = alloc.y;
		wm->tables[i].alloc.width = alloc.width;
		wm->tables[i].alloc.height = alloc.height;

		switch (i) {
		case 0:
			wm->tables[i].color[0] = 1;
			wm->tables[i].color[1] = 0;
			wm->tables[i].color[2] = 0;
			break;
		case 1:
			wm->tables[i].color[0] = 0;
			wm->tables[i].color[1] = 0.8;
			wm->tables[i].color[2] = 0;
			break;
		case 2:
			wm->tables[i].color[0] = 0;
			wm->tables[i].color[1] = 0;
			wm->tables[i].color[2] = 1;
			break;
		}

		wm->tables[i].y1 = cd->y1[i];
		wm->tables[i].y2 = cd->y2[i];

		draw_table(wm->tables + i, wm, c, cd);

		alloc.y += alloc.height + interval;
	}
}

void draw_table(table* tbl, window_main* wm, cairo_t* c, calc_data* cd)
{
	int i, x, y0, y1;
	int text_offset = 3;
	const int fig_offset_x1 = 25;
	const int fig_offset_x2 = 10;
	const int fig_offset_y1 = 15;
	const int fig_offset_y2 = 25;
	char buf[50];


	cairo_set_line_width(c, 1);

	tbl->gx0 = tbl->alloc.x + fig_offset_x1;
	tbl->gx1 = tbl->alloc.x + tbl->alloc.width - fig_offset_x2;
	tbl->gy0 = tbl->alloc.y + fig_offset_y1;
	tbl->gy1 = tbl->alloc.y + tbl->alloc.height - fig_offset_y2;

	cairo_rectangle(c,
		tbl->alloc.x,
		tbl->alloc.y,
		tbl->alloc.width,
		tbl->alloc.height);

	cairo_rectangle(c,
		tbl->gx0,
		tbl->gy0,
		tbl->gx1 - tbl->gx0,
		tbl->gy1 - tbl->gy0);

	cairo_set_source_rgb(c, 1, 1, 1);
	cairo_fill_preserve(c);
	cairo_set_source_rgb(c, 0, 0, 0);
	cairo_stroke(c);

	cairo_set_source_rgb(c, 0.8, 0.8, 0.8);

	for (i = 1; i < cd->sectors_count - 1; ++i) {
		x = tbl->gx0 + i * (tbl->gx1 - tbl->gx0) / (cd->sectors_count - 1);
		cairo_move_to(c, x, tbl->gy0);
		cairo_line_to(c, x, tbl->gy1);
		cairo_stroke(c);
	}

	cairo_set_source_rgb(c, tbl->color[0], tbl->color[1], tbl->color[2]);

	cairo_move_to(c,
		tbl->gx0,
		tbl->gy0 + (tbl->gy1 - tbl->gy0)*(1-tbl->y1));
	cairo_line_to(c,
		tbl->gx1,
		tbl->gy0 + (tbl->gy1 - tbl->gy0)*(1-tbl->y2));
	cairo_stroke(c);

	cairo_move_to(c,
		tbl->alloc.x + text_offset,
		tbl->gy0 + (tbl->gy1 - tbl->gy0)*(1-tbl->y1));
	sprintf(buf, "%.0f", tbl->y1 * 255);
	cairo_show_text(c, buf);

	cairo_move_to(c,
		tbl->alloc.x + text_offset,
		tbl->gy0 + (tbl->gy1 - tbl->gy0)*(1-tbl->y2));
	sprintf(buf, "%.0f", tbl->y2 * 255);
	cairo_show_text(c, buf);

	cairo_fill_preserve (c);
	cairo_stroke (c);

	tbl->cpy0 = tbl->gy0 + (tbl->gy1 - tbl->gy0)*(1-tbl->y1);
	tbl->cpy1 = tbl->gy0 + (tbl->gy1 - tbl->gy0)*(1-tbl->y2);

	draw_point(c, tbl->gx0, tbl->cpy0);
	draw_point(c, tbl->gx1, tbl->cpy1);
}

gboolean drawing_area_button_press_event(GtkWidget *widget,
               GdkEventButton *event, window_main* wm)
{
	tables_press(wm, wm->tables, event->x, event->y, 1);
}

gboolean drawing_area_button_release_event(GtkWidget *widget,
               GdkEventButton *event, window_main* wm)
{
	tables_press(wm, wm->tables, event->x, event->y, 3);
}

gboolean drawing_area_motion_notify_event(GtkWidget *widget,
               GdkEventButton *event,
               window_main* wm)
{
	tables_press(wm, wm->tables, event->x, event->y, 2);
}

void tables_press(window_main* wm, table* tbl, double x, double y, int state)
{
	#define OVER_THE_POINT(x, y, px, py)    \
		((px) - POINT_RADIUS <= (x) && \
		(x) <= (px) + POINT_RADIUS && \
		(py) - POINT_RADIUS <= (y) && \
		(y) <= (py) + POINT_RADIUS)

	if (state == 1) {
		for (int i = 0; i < TABLES_COUNT; ++i) {
			if (OVER_THE_POINT(x, y, tbl[i].gx0, tbl[i].cpy0)) {
				wm->table_change = tbl + i;
				wm->color_change = tbl[i].color1;
			}
			if (OVER_THE_POINT(x, y, tbl[i].gx1, tbl[i].cpy1)) {
				wm->table_change = tbl + i;
				wm->color_change = tbl[i].color2;
			}
		}
	}

	if (wm->table_change == NULL)
		return ;

	table* tc = wm->table_change;
	guint16* cc = wm->color_change;

	if (y <= tc->gy0)
		*cc = 0xFFFF;
	else if (tc->gy0 < y && y < tc->gy1)
		*cc = (tc->gy1 - y) * 65535 / (tc->gy1 - tc->gy0);
	else
		*cc = 0x0000;

	if (state == 3) {
		wm->table_change = NULL;
		wm->color_change = NULL;
	}

	gtk_widget_queue_draw (wm->drawing_area);
}

void draw_point(cairo_t* c, double x, double y)
{
	cairo_arc(c, x, y, POINT_RADIUS, 0, 2*M_PI);
	cairo_set_source_rgb(c, 1, 1, 1); 
	cairo_fill_preserve(c);
	cairo_set_source_rgb(c, 0, 0, 0);
	cairo_stroke(c);
}

int main(int argc, char* argv[])
{
	window_main wm;
	
	
	gtk_init(&argc, &argv);
	window_main_create(&wm);
	gtk_main();
	
	return 0;
}
