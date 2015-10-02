/**
	@mainpage CStudy
	
	Suite per facilitare lo studio. Permette di calcolare tramite un wizard quante pagine si
	dovranno studiare per un esame in una certa data, fornisce un timer che segue la
	<a href="https://en.wikipedia.org/wiki/Pomodoro_Technique" >pomodoro technique</a>
	e un sistema per il ripasso tramite flashcards.
	
	Per la compilazione entrare nell cartella /src e digitare da terminale: @n
	
		make
	
	Se si vuole ottenere la modalita' di debug (normale e verboso): @n
	
		make debug
	
	E poi avviare il programma tramite l'icona se si e' scelta la compilazione normale, altrimenti
	per la modalita' debug avviare il programma da terminale: @n
		./CStudy DEB per la modalita' di debug semplice @n
		./CStudy VER per la modalita' di debug verbosa
	
	@author Mirco Romagnoli
*/
/** @file
	File che si occupa di avviare il gtk_main e di fornire una funzione che avvia un messaggio di errore,
	inoltre contiene la ::MASK per il debug e il ::builder.
*/

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

/** Maschera di bit per decidere tra la modalita' di debug normale o verbosa.*/
unsigned int MASK = 0 ;
/** GtkBuilder per l'interfaccia grafica costruita con Glade.*/
GtkBuilder *builder ;
extern char *percorso ;

/** Mostra un messaggio di errore.

	Mostra il messaggio di errore, la finestra e' collegata a parent, in questo modo puo' apparire centrata
	il modello della GtkDialog e' stato creato su glade.
	@param[in] messaggio il messaggio di errore da mostrare
	@param[in] parent finestra che chiama la funzione a cui collegare il dialogo
*/
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

/** Funzione principale.

	Si occupa di preparare l'interfaccia grafica e di controllare se il programma e' stato chiamato con gli argomenti corretti
	(in modalita' di debug).@n
	Inizializza inoltre il sistema di notifica per il timer
*/
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
