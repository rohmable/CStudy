#include <gtk/gtk.h>
#include <string>
#ifdef DEBUG_MODE
	#include "./include/debug.h"
	#include <iostream>
#else
	#define DEB(a)
	#define VER(a)
#endif
using namespace std ;

#include "include/esame.h"
#include "include/main.h"
#include "include/save_load.h"

static GSList *elem_att = NULL ;
enum {GTK_BUTTON_CHIUDI = 1} ;

#ifdef DEBUG_MODE
static void stampa_lista ()
{
	cout << "Stampo la lista" << endl ;
	GSList *tmp = lista ;
	flashcards_t *elem = NULL ;
	while (tmp != NULL) {
		elem = static_cast<flashcards_t *>(tmp->data) ;
		cout << "Scadenza = " << elem->scadenza << endl
			 << "Domanda: " << elem->domanda << endl 
			 << "Risposta: " << elem->risposta << endl ;
		tmp = tmp->next ;
	}
	VER(cout << "Fine stampa" << endl ) ;
}
#endif

extern "C" void controlla_lun_testo (GtkTextBuffer *textbuffer, gpointer user_data)
{
	VER(MASK = 1) ;
	int lunghezza = gtk_text_buffer_get_char_count(textbuffer) ;
	DEB(if (lunghezza >= 240) MASK = 3 ) ;
	VER(cout << "Lunghezza testo = " << lunghezza << endl ) ;
	if (lunghezza >= 250) {
		GtkTextIter iter;
		int offset ;
		g_object_get(G_OBJECT(textbuffer), "cursor-position", &offset, NULL) ;
		VER(cout << "Posizione cursore = " << offset << endl ) ;
		gtk_text_buffer_get_iter_at_offset(textbuffer, &iter, offset) ;
		gtk_text_buffer_backspace(textbuffer, &iter, FALSE, TRUE) ;
		if (!gtk_revealer_get_reveal_child(GTK_REVEALER(user_data)))
			gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), TRUE) ;
	}
	else {
		if (gtk_revealer_get_reveal_child(GTK_REVEALER(user_data)))
			gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), FALSE) ;
	}
}

extern "C" void annulla_carta (GtkButton *button, gpointer user_data)
{
	GtkTextBuffer *domanda = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "buffer_domanda")),
				  *risposta = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "buffer_risposta")) ;
	GtkTextIter inizio, fine ;
	gtk_text_buffer_get_start_iter(domanda, &inizio) ;
	gtk_text_buffer_get_end_iter(domanda, &fine) ;
	gtk_text_buffer_delete(domanda, &inizio, &fine) ;
	
	gtk_text_buffer_get_start_iter(risposta, &inizio) ;
	gtk_text_buffer_get_end_iter(risposta, &fine) ;
	gtk_text_buffer_delete(risposta, &inizio, &fine) ;
}

extern "C" void salva_carta (GtkButton *button, gpointer user_data)
{
	GtkTextBuffer *domanda_buff = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "buffer_domanda")),
				  *risposta_buff = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "buffer_risposta")) ;
	GtkTextIter inizio, fine ;
	
	gtk_text_buffer_get_start_iter(domanda_buff, &inizio) ;
	gtk_text_buffer_get_end_iter(domanda_buff, &fine) ;
	char *domanda = gtk_text_buffer_get_text(domanda_buff, &inizio, &fine, FALSE) ;
	
	gtk_text_buffer_get_start_iter(risposta_buff, &inizio) ;
	gtk_text_buffer_get_end_iter(risposta_buff, &fine) ;
	char *risposta = gtk_text_buffer_get_text(risposta_buff, &inizio, &fine, FALSE) ;
	
	if(!aggiungi_carta(domanda, risposta, 1))
		messaggio_errore("Impossibile salvare la carta", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
	annulla_carta(button, user_data) ;
}

static void scrivi_carta (GSList *elem)
{
	VER(cout << "Scrivo carta" << endl ) ;
	elem_att = elem ;
	flashcards_t *carta = static_cast<flashcards_t *>(elem->data) ;
	GtkTextBuffer *domanda_buff = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "domanda_rip")),
				  *risposta_buff = GTK_TEXT_BUFFER(gtk_builder_get_object(builder, "risposta_rip")) ;
	gtk_text_buffer_set_text(domanda_buff, carta->domanda.c_str(), carta->domanda.size()) ;
	gtk_text_buffer_set_text(risposta_buff, carta->risposta.c_str(), carta->risposta.size()) ;
}

extern "C" void ripasso_clicked (GtkButton *button, gpointer user_data)
{
	VER(cout << "Apro la finestra di ripasso" << endl) ;
	carica_carte() ;
	VER(stampa_lista()) ;
	if (lista != NULL)
		scrivi_carta(lista) ;
	else if (static_cast<flashcards_t *>(lista->data)->scadenza > 0) {
		int risposta = gtk_dialog_run(GTK_DIALOG(gtk_builder_get_object(builder, "ripasso_completato"))) ;
		if (risposta == GTK_RESPONSE_OK || risposta == GTK_RESPONSE_DELETE_EVENT)
			gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "ripasso_completato"))) ;
	}
	else {
		messaggio_errore("Non e' presente alcuna carta", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
		return ;
	}
	int risposta = gtk_dialog_run(GTK_DIALOG(user_data)) ;
	if (risposta == GTK_BUTTON_CHIUDI || risposta == GTK_RESPONSE_DELETE_EVENT) {
		VER(cout << "risposta = " << risposta << endl
				 << "Cancello la lista" << endl ) ;
		if (!salva_carte())
			messaggio_errore("Impossibile salvare le carte", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
		g_slist_free(lista) ;
		lista = NULL ;
		gtk_widget_hide(GTK_WIDGET(user_data)) ;
	}
}

extern "C" void mostra_risposta (GtkButton *button, gpointer user_data)
{
	gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), TRUE) ;
	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE) ;
}

extern "C" void risposta_succ (GtkButton *button, gpointer user_data)
{
	GtkToggleButton *risp_difficile = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "radiobutton1")),
					*risp_normale = GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder, "radiobutton2")) ;
					
	flashcards_t *carta = static_cast<flashcards_t *>(elem_att->data) ;
	
	if (gtk_toggle_button_get_active(risp_difficile))
		carta->scadenza = 0 ;
	else if (gtk_toggle_button_get_active(risp_normale))
		carta->scadenza = 3 ;
	else
		carta->scadenza = 7 ;
	lista = g_slist_sort(lista, compara_scadenze) ;
	VER(cout << "Stampo lista riordinata" << endl ;
		stampa_lista()) ;
	gtk_revealer_set_reveal_child(GTK_REVEALER(gtk_builder_get_object(builder, "revealer3")), FALSE) ;
	gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(builder, "button18")), TRUE) ;
	if (static_cast<flashcards_t *>(lista->data)->scadenza <= 0)
		scrivi_carta(lista) ;
	else {
		int risposta = gtk_dialog_run(GTK_DIALOG(user_data)) ;
		if (risposta == GTK_RESPONSE_OK || risposta == GTK_RESPONSE_DELETE_EVENT) {
			gtk_widget_hide (GTK_WIDGET(user_data)) ;
			gtk_widget_hide(gtk_widget_get_toplevel(GTK_WIDGET(button))) ;
			if (!salva_carte())
				messaggio_errore("Impossibile salvare le carte", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
		}
	}
}















