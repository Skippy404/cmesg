/* @Auther  -> Stephen Cochrane
 * @Github  -> skippy404
 * @License -> GPL
 */

#include "client.h"

/*
static GtkWidget *n1;
static GtkWidget *n2;
static GtkWidget *res;
*/

static GtkWidget *fixed;
static GtkWidget *send_btn;          /* Send text button */
static GtkWidget *recv_box;          /* Display recv data */
static GtkWidget *text;              /* Input text */
static GtkWidget *name;              /* Name of the program */
static GtkWidget *voip_joined;       /* List of people joined */
static GtkWidget *user_online_label; /* Label for online people */
static GtkWidget *jl_voip;           /* Join/Leave voip button */
static GtkWidget *user_online;       /* List of online people */
static GtkBuilder *builder;          /* The builder for GTK */

void
on_send_clicked(GtkButton *b)
{
	char *input = (char *) gtk_entry_get_text(GTK_ENTRY(text));
	printf("DEBUG: Entered Message: %s\n", input);
	gtk_entry_set_text(GTK_ENTRY(text), "");
}

// Just test code to get used to gtk
int
main(int argc, char *argv[])
{
	GtkWidget *window, *grid, *calculate;
	int i, port;
	char *hostname;

	gtk_init(&argc, &argv);
	
	/* Parse args */
	while ((i = getopt(argc, argv, "p:H:h")) != -1) {
		switch (i) {
			case 'p':
				port = atoi(optarg);
				printf("Port: %d\n", port);
				break;
			case 'H':
				hostname = strdup(optarg);
				printf("hostname: %s\n", hostname);
				break;
			case 'h':
				printf("cmesg client v1.0 (https://github.com/skippy404/cmesg)\n\n");
				printf("Usage: cmesg_client [options].\n");
				printf("-h\tShows this message.\n");
				printf("-p\tSpecify a port to use.\n");
				printf("-H\tSpecify the hostname to connect too.\n");
				return EXIT_SUCCESS;
				break;
		}
	}

	builder = gtk_builder_new_from_file("./assets/client.glade");
	window = GTK_WIDGET(gtk_builder_get_object(builder, "root"));
	g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	/* Map the GTK Widgets from the builder to thier Counterparts */
	fixed = GTK_WIDGET(gtk_builder_get_object(builder, "fixed"));
	send_btn = GTK_WIDGET(gtk_builder_get_object(builder, "send"));
	recv_box = GTK_WIDGET(gtk_builder_get_object(builder, "recv"));
	text = GTK_WIDGET(gtk_builder_get_object(builder, "text"));
	name = GTK_WIDGET(gtk_builder_get_object(builder, "name"));
	voip_joined = GTK_WIDGET(gtk_builder_get_object(builder, "voip_joined"));
	user_online_label = GTK_WIDGET(gtk_builder_get_object(builder, "user_online_label"));
	jl_voip = GTK_WIDGET(gtk_builder_get_object(builder, "jl_voip"));
	user_online = GTK_WIDGET(gtk_builder_get_object(builder, "user_online"));

	/* Map the buttons to thier handlers */
	g_signal_connect(send_btn, "clicked", G_CALLBACK(on_send_clicked), NULL);

	gtk_widget_show(window);
	gtk_main();

	return EXIT_SUCCESS;
}
