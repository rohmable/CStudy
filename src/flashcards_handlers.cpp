/** @file
	File che contiene gli handler che gestiscono la parte delle flashcards all'interno del programma
*/

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

static GSList *elem_att = NULL ; /**< L'elemento attualmente stampato nella finestra del ripasso*/
enum {GTK_BUTTON_CHIUDI = 1} ; /**< Usato per dare un significato all'azione del bottone chiudi in ::ripasso_clicked e 
									::elimina_carte_clicked*/

#ifdef DEBUG_MODE

/** Stampa la lista di ::flashcards_t.*/
static void stampa_lista ()
{
	cout << "Stampo la lista" << endl ;
	GSList *tmp = lista ;
	flashcards_t *elem = NULL ;
	while (tmp != NULL) {
		elem = static_cast<flashcards_t *>(tmp->data) ;
		cout << "Scadenza = " << elem->scadenza << endl
			 << "\tDomanda: " << elem->domanda << endl 
			 << "\tRisposta: " << elem->risposta << endl ;
		tmp = tmp->next ;
	}
	VER(cout << "Fine stampa" << endl ) ;
}
#endif

/** Dealloca una ::flashcards_t.

	Funzione di tipo GDestroyNotify chiamata al momento dell'allocazione di una lista, in questo modo si dealloca completamente
	ogni elemento della lista.
*/
static void delete_data (gpointer data)
{
	flashcards_t *carta = static_cast<flashcards_t *>(data) ;
	VER(cout << "Elimino:\n\tDomanda: " << carta->domanda << endl
			 << "\tRisposta: " << carta->risposta << endl ) ;
	delete carta ;
}

/** Controlla se una domanda non supera i 250 caratteri.

	Handler collegato al segnale changed dei buffer di testo dove si scrivono le nuove ::flashcards_t, controlla
	se la lunghezza non supera i 250 caratteri, se l'utente raggiunge questo limite e cerca di scrivere altri caratteri
	gli verra' impedito ed un avviso di errore sara' mostrato
*/
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

/** Svuota i buffer di testo su comando dell'utente.

	Se l'utente decide di annullare l'aggiunta di una carta questo handler provvede a svuotare i buffer di testo.
*/
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

/** Salva una ::flashcards_t nel file <::percorso>/cards.txt.

	Recupera il testo all'interno dei buffer ed invoca ::aggiungi_carta dando alla carta la scadenza di un giorno.
*/
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

/** Permette di scrivere la carta ::elem all'interno dei buffer dedicati al ripasso.*/
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

/** Carica le carte, scrive la prima carta e si occupa di salvarle al termine del ripasso.

	Questo handler si occupa di caricare le carte, se nella lista vi e' una carta con scadenza <= 0 la scrive,
	se la lista e' nulla o non vi sono carte con scadenza <= 0 allora mostra un avviso. @n
	Al termine della sessione di ripasso aggiorna il file contenente le carte.
*/
extern "C" void ripasso_clicked (GtkButton *button, gpointer user_data)
{
	VER(cout << "Apro la finestra di ripasso" << endl) ;
	carica_carte() ;
	VER(stampa_lista()) ;
	if (lista != NULL && static_cast<flashcards_t *>(lista->data)->scadenza <= 0)
		scrivi_carta(lista) ;
	else if (lista == NULL) {
		messaggio_errore("Non e' presente alcuna carta", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
		return ;
	}
	else {
		int risposta = gtk_dialog_run(GTK_DIALOG(gtk_builder_get_object(builder, "ripasso_completato"))) ;
		if (risposta == GTK_RESPONSE_OK || risposta == GTK_RESPONSE_DELETE_EVENT) {
			g_slist_free_full(lista, delete_data) ;
			lista = NULL ;
			gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "ripasso_completato"))) ;
		}
		return ;
	}
	int risposta = gtk_dialog_run(GTK_DIALOG(user_data)) ;
	if (risposta == GTK_BUTTON_CHIUDI || risposta == GTK_RESPONSE_DELETE_EVENT) {
		VER(cout << "risposta = " << risposta << endl
				 << "Cancello la lista" << endl ) ;
		if (!salva_carte())
			messaggio_errore("Impossibile salvare le carte", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
		g_slist_free_full(lista, delete_data) ;
		lista = NULL ;
		gtk_widget_hide(GTK_WIDGET(user_data)) ;
	}
}

/** Apre il GtkRevealer per permettere all'utente di visualizzare la risposta.*/
extern "C" void mostra_risposta (GtkButton *button, gpointer user_data)
{
	gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), TRUE) ;
	gtk_widget_set_sensitive(GTK_WIDGET(button), FALSE) ;
}

/** Aggiorna la carta attuale e stampa la carta successiva.

	Resetta inoltre il bottone per mostrare la risposta e il GtkRevealer, se l'utente poi preme il bottone per chiudere
	la finestra la funzione si occupa di chiamare ::salva_carte per salvare la lista
*/
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
	elem_att = lista ;
	VER(stampa_lista()) ;
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
			g_slist_free_full(lista, delete_data) ;
			lista = NULL ;
		}
	}
}

/** Popola con la GtkTreeView con i valori di ::lista.*/
static void popola_tree_view ( )
{
	GSList *elem = lista ;
	GtkTreeStore *tree = GTK_TREE_STORE(gtk_builder_get_object(builder, "gestione_carte")) ;
	GtkTreeIter iter ;
	flashcards_t *carta = NULL ;
	while (elem != NULL) {
		gtk_tree_store_append (tree, &iter, NULL);
		carta = static_cast<flashcards_t *>(elem->data) ;
		gtk_tree_store_set(tree, &iter, 0, carta->scadenza, 1, carta->domanda.c_str(), 2, carta->risposta.c_str(), -1) ;
		elem = elem->next ;
	}
}

/** Carica le ::flashcards_t e permette all'utente di eliminare le carte.

	Quando l'utente clicca il bottone appare la finestra di dialogo per poter eliminare le carte*/
extern "C" void elimina_carte_clicked (GtkButton *button, gpointer user_data)
{
	carica_carte() ;
	if (lista == NULL)
		messaggio_errore("Nessuna carta da eliminare", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
	else {
		popola_tree_view() ;
		int risposta = gtk_dialog_run(GTK_DIALOG(user_data)) ;
		if (risposta == GTK_BUTTON_CHIUDI || risposta == GTK_RESPONSE_DELETE_EVENT) {
			if (!salva_carte())
				messaggio_errore("Impossibile salvare le carte", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
			g_slist_free_full(lista, delete_data) ;
			lista = NULL ;
			gtk_widget_hide(GTK_WIDGET(user_data)) ;
			gtk_tree_store_clear (GTK_TREE_STORE(gtk_builder_get_object(builder, "gestione_carte")));
		}
	}
}

/** Stabilisce se due ::flashcards_t sono uguali o meno.

	Funzione GCompareFunc per capire se la carta a e la carta b sono uguali, se le stringhe di domanda e risposta sono uguali
	allora la funzione ritorna la differenza tra le scadenze delle due carte (che potrebbero essere diverse), altrimenti ritorna
	1 (valore arbitrario)
*/
static int compara_carte (gconstpointer a, gconstpointer b)
{
	const flashcards_t *a_carta = static_cast<const flashcards_t *>(a),
					  *b_carta = static_cast<const flashcards_t *>(b) ;
	
	if (a_carta->domanda == b_carta->domanda && a_carta->risposta == b_carta->risposta)
		return a_carta->scadenza - b_carta->scadenza ;
	return 1 ;
}

/** Si occupa di eliminare un elemento da ::lista.

	La funzione recupera la selezione dalla GtkTreeView dell'interfaccia grafica e
	@li rintraccia l'elemento all'interno della lista
	@li lo scollega dalla lista
	@li lo elimina
*/
extern "C" void elimina_carta (GtkButton *button, gpointer user_data)
{
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(user_data))) {
		VER(cout << "Eliminazione non confermata" << endl ) ;
		return ;
	}
	GtkTreeSelection *selection = GTK_TREE_SELECTION(gtk_builder_get_object(builder, "treeview-selection")) ;
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW(gtk_builder_get_object(builder, "treeview1"))) ;
	GtkTreeIter iter ;
	
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) {
		DEB(cout << "Impossibile impostare il GtkIter" << endl) ;
		return ;
	}
	int scadenza = 0 ;
	char *domanda = NULL, *risposta = NULL ;
	VER(cout << "Recupero la selezione" << endl ) ;
	gtk_tree_model_get(model, &iter, 0, &scadenza, 1, &domanda, 2, &risposta, -1) ;
	VER(cout << "Elemento selezionato:\n\tScadenza: " << scadenza << endl
			 << "\tDomanda: " << domanda << endl
			 << "\tRisposta: " << risposta << endl ) ;
	flashcards_t *elim = new flashcards_t ;
	elim->scadenza = scadenza ; elim->domanda = domanda ; elim->risposta = risposta ;
	
	GSList *elem_da_eliminare = g_slist_find_custom(lista, static_cast<gpointer>(elim), compara_carte) ;
	if (elem_da_eliminare == NULL) {
		delete elim ;
		DEB(cout << "Elemento non trovato" << endl ) ;
		return ;
	}
	VER(cout << "Estraggo l'elemento" << endl ) ;
	lista = g_slist_remove_link(lista, elem_da_eliminare) ;
	VER(cout << "Dealloco l'elemento" << endl ) ;
	g_slist_free_full(elem_da_eliminare, delete_data) ;
	VER(cout << "Elemento eliminato" << endl ;
		stampa_lista()) ;
	delete elim ;
	gtk_tree_store_remove(GTK_TREE_STORE(gtk_builder_get_object(builder, "gestione_carte")), &iter) ;
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(user_data), FALSE) ;
}








