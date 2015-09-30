#include <cstring>
#ifdef DEBUG_MODE
	#include "./include/debug.h"
	#include <iostream>
#else
	#define DEB(a)
	#define VER(a)
#endif
#include <gtk/gtk.h>
#include <string>
#include "../lib/notify.h"
using namespace std;

#include "include/esame.h"

unsigned int MASK = 0 ;
GtkBuilder *builder ;
extern char *percorso ;

void messaggio_errore (const char messaggio[], GtkWindow *parent)
{
	GObject *finestra = gtk_builder_get_object (builder,"Errore");
	gtk_window_set_transient_for(GTK_WINDOW(finestra), parent) ;
	gtk_window_set_attached_to(GTK_WINDOW(finestra), GTK_WIDGET(parent)) ;
	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(finestra), messaggio) ;
	int risposta = gtk_dialog_run(GTK_DIALOG(finestra)) ;
	if (risposta == GTK_RESPONSE_OK || risposta == GTK_RESPONSE_DELETE_EVENT)
		 gtk_widget_hide(GTK_WIDGET(finestra)) ;
}

int main (int argc, char *argv[]) {
	#ifdef DEBUG_MODE
	if (argc < 2) {
		cout << "Utilizzo in debug mode: ./CStudy <opzione>\n"
				"Opzioni disponibili:\n"
				"\tDEB: per la modalita' di debug semplice\n"
				"\tVER: per la modalita' di debug verbosa" << endl ;
		return 1 ;
	}
	else {
		if (strcmp(argv[1], "DEB") == 0) {
			cout << "Esecuzione in modalita' debug" << endl ;
			MASK = 1 ;
		} else if (strcmp (argv[1], "VER") == 0) {
			cout << "Esecuzione in modalita' verbosa" << endl ;
			MASK = 3 ;
		} else {
			cout << "Opzioni disponibili:\n"
					"\tDEB: per la modalita' di debug semplice\n"
					"\tVER: per la modalita' di debug verbosa" << endl ;
			return 2 ;
		}
	}
	#endif
	
	VER(cout << "Preparazione dell'interfaccia grafica" << endl ) ;
	gtk_init(&argc, &argv) ;
	builder = gtk_builder_new() ;
	gtk_builder_add_from_file(builder, "gui.glade", NULL) ;
	gtk_builder_connect_signals(builder, NULL) ;
	notify_init ("CStudy");
	gtk_main() ;
	g_free(percorso) ;
	notify_uninit();
	return 0 ;
}
