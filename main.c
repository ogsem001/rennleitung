#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

int work = 1;
char PATH[]="/proc/carrera";
float ruzeit, bestzeit;
char limiter[5]="= \n\r";
//RasPis
char kb1[4] = "kb1";
char kb2[4] = "kb2";
char bb1[4] = "bb1";
char bb2[4] = "bb2";

char kb1n[] = "Kreuzungsbahn 1";
char kb2n[] = "Kreuzungsbahn 2";
char bb1n[] = "Brückenbahn 1";
char bb2n[] = "Brückenbahn 2";
//GTK Labelpointer
GtkWidget *g_lbl_kb1;
GtkWidget *g_lbl_kb2;
GtkWidget *g_lbl_bb1;
GtkWidget *g_lbl_bb2;
typedef struct commitargs{
		char *dest;
		GtkWidget *label;	
		}labelargs;

pthread_t thread1;
pthread_t thread2;
pthread_t thread3;
pthread_t thread4;

void *run (void *destargs){
	GtkStyleContext *context;
	labelargs *newargs = (labelargs *)destargs;
	context = gtk_widget_get_style_context(newargs->label);
	GtkCssProvider *provider = gtk_css_provider_new();
	//GdkDisplay *display = gdk_display_get_default();
	//GdkScreen *screen = gdk_display_get_default_screen(display);
	gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),GTK_STYLE_PROVIDER(provider),800);
	gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider),"src/style.css",NULL);	
	//gtk_style_context_add_class(context,"green");
	g_object_unref(provider);

	while (work==1){
		char *destpointer;		
		int accesscount, pid,clear;
		accesscount =-1;
		char *saveptr;
		char *ptr;
		char carrerafile[45];
		char ownerof[45];
		char buffer[31];
		buffer[30] = '\0';
		pid =0;
		clear=1;
			//	printf("%s",newargs->dest);

		
		
		//char *dest2 = (char *) dest;
		//GtkWidget *label2 = (GtkWidget *) label;
		char labeltext[50];
		snprintf(carrerafile, sizeof(carrerafile), "ssh local@%s 2>&1 cat %s",newargs->dest,PATH);
		FILE *stream = popen(carrerafile, "r");
		if(stream == NULL)
			printf("SSH");
		
		fread(buffer, 30,1,stream);
		
		if((strncmp(buffer,"ssh: connect to host kb2 por",25))==0){
			printf("\n *** ERROR - CHECK NET CONNECTION ***\n\n");
			gtk_main_quit();
			work = 0;
		

			}
		
		if ((strncmp(buffer,"cat: /proc/carrera: No such",25))==0){
			printf("\n *** ERROR - CHECK CARRERA DRIVER ***\n\n");
			gtk_main_quit();
			work = 0;
				
		}


		//printf("\n\n%s: Read Buffer: %s \n",newargs->dest, buffer);
		//printf("%s: Workcount: %d\n",newargs->dest,work); 
		ptr = strtok_r(buffer, limiter,&saveptr); //tokenize input
		//printf("%s: Tokenized: %s \n",newargs->dest,ptr);
		
		while (ptr != NULL)	{
			//printf("opening loop with '%s' \n",ptr);
			if ((strcmp(ptr,"AccessCountWrite")==0) && accesscount == -1)
			{	
				//printf("%s: First: %s\n",newargs->dest,ptr);	
				accesscount = atoi(strtok_r(NULL,limiter,&saveptr));
				//printf("%s Writing Acesscount: %d \n",newargs->dest,accesscount);
			}	
			else if ((strcmp(ptr, "PID")==0) && pid == 0)
			{	
			//	printf("%s: Second: %s\n",newargs->dest,ptr);	
				pid = atoi(strtok_r(NULL,limiter,&saveptr));	
			//	printf("%s: Writing PID: %d \n", newargs->dest,pid);
			}
			ptr = strtok_r(NULL, limiter, &saveptr);

		}
		
	//	printf("%s Zugriffscode: %d PID: %d\n",newargs->dest,accesscount, pid);
		pclose(stream); 
		
		if (strcmp(newargs->dest,kb1)==0)
			destpointer = kb1n;	
		else if (strcmp(newargs->dest, kb2)==0)
			destpointer = kb2n;
		else if (strcmp(newargs->dest,bb1)==0)
			destpointer = bb1n;
		else if (strcmp(newargs->dest,bb2)==0)
			destpointer = bb2n;

		if (accesscount == 0 && pid!= 0)
		{
			snprintf(ownerof, sizeof(ownerof), "ssh local@%s ps -o user= -p %d", newargs->dest, pid);
			//printf("OWNERSTRING: %s",ownerof);
			FILE *stream2 = popen(ownerof,"r");
			char buffer2[16];
			buffer2[15]='\0';
			fread(buffer2,15,1,stream2);
			//printf("OWNER: %s",buffer);
			snprintf(labeltext,(sizeof(labeltext)-1),"%s blockiert\nvon %s",destpointer,buffer2);
			//printf("%s\n",labeltext);
			gtk_label_set_text(GTK_LABEL(newargs->label),labeltext);
			//gtk_widget_override_background_color((newargs->label),0,&red);
			gtk_style_context_remove_class(context,"green");
			gtk_style_context_add_class(context,"red");
			clear = 1;
			pclose(stream2);	
		}
		else	
		{		
				snprintf(labeltext,sizeof(labeltext),"%s frei",destpointer);
				gtk_label_set_text(GTK_LABEL(newargs->label),labeltext);
				//gtk_widget_override_background_color(/*GTK_LABEL*/(newargs->label),0,&green);
				gtk_style_context_remove_class(context,"red");
				gtk_style_context_add_class(context,"green");
				clear = 0;
		}
		sleep(2);
	}
	
	return 0;
}
/*
void HideCursor(GtkWidget *window)
{
	GdkDisplay      *display;
	GdkCursor       *hideCursor = NULL;
	GdkWindow       *gdk_window;

	// hide cursor
	display = gdk_display_get_default();
	hideCursor = gdk_cursor_new_for_display(display, GDK_BLANK_CURSOR);
	gdk_window = gtk_widget_get_window(window);
	gdk_window_set_cursor(gdk_window, hideCursor);
}

void MoveCursor(int x, int y)
{
	Display *dpy;
	Window root_window;

	dpy = XOpenDisplay(0);
	root_window = XRootWindow(dpy, 0);
	XSelectInput(dpy, root_window, KeyReleaseMask);
	XWarpPointer(dpy, None, root_window, 0, 0, 0, 0, x, y);
	XFlush(dpy);
}
*/
void argumentify (labelargs *largs, char *dest, GtkWidget *label){
	largs->dest = dest;
	largs->label = label;
	
	}

void on_window1_destroy()
{
	    work = 0;
		pthread_join(thread1,NULL);
		pthread_join(thread4,NULL);
		pthread_join(thread3,NULL);
		pthread_join(thread2,NULL);
		gtk_main_quit ();
}
int main (int argc, char **argv, char **envp){
//run();
	GtkBuilder      *builder; 
	GtkWidget       *window;

	gtk_init(&argc, &argv);
	
		
	builder = gtk_builder_new();
	gtk_builder_add_from_file (builder, "glade/new2.glade", NULL);
	window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
	g_lbl_kb1 =	GTK_WIDGET(gtk_builder_get_object(builder,"lbl_kb1"));
	g_lbl_kb2 = GTK_WIDGET(gtk_builder_get_object(builder,"lbl_kb2"));
	g_lbl_bb1 = GTK_WIDGET(gtk_builder_get_object(builder,"lbl_bb1"));
	g_lbl_bb2 = GTK_WIDGET(gtk_builder_get_object(builder,"lbl_bb2"));
	gtk_builder_connect_signals(builder, NULL);

	g_object_unref(builder);

	gtk_window_fullscreen(GTK_WINDOW(window));

	//MoveCursor(1920, 1080);   // move cursor off screen
	gtk_widget_show(window);                

	//HideCursor(window);

	labelargs *Akb1 = malloc(sizeof (labelargs));
	labelargs *Akb2 = malloc(sizeof (labelargs));
	labelargs *Abb1 = malloc(sizeof (labelargs));
	labelargs *Abb2 = malloc(sizeof (labelargs));
	
	argumentify(Akb1,kb1, g_lbl_kb1);
	argumentify(Akb2,kb2, g_lbl_kb2);
	argumentify(Abb1,bb1, g_lbl_bb1);
	argumentify(Abb2,bb2, g_lbl_bb2);
	printf("Starting Threads\n\n");
	

	if ( pthread_create(&thread1, NULL, run,(void *) Akb1) !=0)
	{
		fprintf(stderr, "creation of thread 1 failed\n");
		return -1;
	}     

	if ( pthread_create(&thread2, NULL, run, (void *) Akb2) !=0)
	{
		fprintf(stderr, "creation of thread 2 failed\n");
		return -1;
	}

	if ( pthread_create(&thread3, NULL, run, (void *) Abb1) !=0)
	{
		fprintf(stderr, "creation of thread 2 failed\n");
		return -1;
	}
	
	if ( pthread_create(&thread4, NULL, run, (void *) Abb2) !=0)
	{
		fprintf(stderr, "creation of thread 2 failed\n");
		return -1;
	}

	gtk_main();
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
	pthread_join(thread4, NULL);
	return 0;

}
