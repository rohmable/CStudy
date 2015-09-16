#include <gtk/gtk.h>
#include <sstream>
#ifdef DEBUG_MODE
	#include "debug.h"
	#include <iostream>
#else
	#define DEB(a)
	#define VER(a)
#endif
using namespace std ;

// Da time_module.cpp
bool data_odierna_uint (unsigned int &, unsigned int &, unsigned int &) ;
// Da main.cpp
extern unsigned int MASK ;
extern GtkBuilder *builder ;
void messaggio_errore (const char [], GtkWindow *) ;

/** Mostra la finestra ::window */
static void apri_finestra (gpointer window)
{
	gtk_widget_show_all(GTK_WIDGET(window)) ;
}

/** Nasconde la finestra ::window */
static void nascondi_finestra (gpointer window)
{
	gtk_widget_hide(GTK_WIDGET(window)) ;
}

/** Chiama ::apri_finestra con una finestra passata tramite glade */
extern "C" void handler_apri_finestra (GtkButton *button, gpointer user_data)
{
	apri_finestra(user_data) ;
}

/** Chiama ::nascondi_finestra con una finestra passata tramite glade */
extern "C" void handler_nascondi_finestra (GtkButton *button, gpointer user_data)
{
	nascondi_finestra(user_data) ;
}

/** Handler per il delete-event, permette di eseguire altri handler */
extern "C" gboolean esci_dal_programma (GtkWidget *widget, GdkEvent  *event, gpointer user_data)
{
	apri_finestra(user_data) ;
	return true ;
}

extern "C" void dialogo_uscita_risposta (GtkDialog *dialog, gint response_id, gpointer user_data)
{
	if (response_id == GTK_RESPONSE_YES)
		gtk_main_quit() ;
	else
		nascondi_finestra(reinterpret_cast<gpointer>(dialog)) ;
}

extern "C" void annulla_wizard (GtkAssistant *assistant, gpointer user_data)
{
	nascondi_finestra(reinterpret_cast<gpointer>(assistant)) ;
}

/** Aggiorna il calendario del wizard per la creazione di un nuovo esame alla data corrente e apre la finestra del wizard*/
extern "C" void aggiorna_data (GtkButton *button, gpointer user_data)
{
	unsigned int giorno, mese, anno ;
	if (!data_odierna_uint (giorno, mese, anno)) {
		messaggio_errore ("Impossibile inizializzare la data odierna", GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(button)))) ;
		return ;
	}
	
	VER(cout << "Aggiorno il calendario" << endl ) ;
	GtkCalendar *cal = GTK_CALENDAR(user_data) ;
	gtk_calendar_select_month (cal, mese-1, anno) ; //Il mese deve essere decrementato perche' l'intervallo va da 0-11
	gtk_calendar_select_day (cal, giorno) ;
	
	apri_finestra (reinterpret_cast<gpointer>(gtk_widget_get_toplevel(GTK_WIDGET(user_data)))) ;
}

/** Controlla la validita' delle informazioni fornite ad ogni passaggio */
extern "C" void controllo_wizard (GtkAssistant *assistant, GtkWidget *page, gpointer user_data)
{
	int pagina = gtk_assistant_get_current_page (assistant) ;
	pagina -- ;
	VER(cout << "Pagina del wizard numero " << pagina << endl ) ;
	bool corretto = true ;
	switch (pagina) {
	case 1: {
		GtkEntryBuffer *nome_esame = GTK_ENTRY_BUFFER(gtk_builder_get_object(builder, "Nome_esame")) ;
		int lun = gtk_entry_buffer_get_length (nome_esame) ;
		VER(cout << "Lunghezza nome_esame = " << lun << endl ) ;
		if (lun == 0) {
			messaggio_errore("E' necessario inserire il nome dell'esame", GTK_WINDOW(assistant)) ;
			corretto = false ;
			break ;
		}
		const char *nome = gtk_entry_buffer_get_text (nome_esame) ;
		VER(cout << "Nome: " << nome << endl ) ;
		for (int i = 0 ; i < lun ; i++)
			if (nome[i] == '/') {
				messaggio_errore("Il nome non puo' contenere il carattere \" \\ \"", GTK_WINDOW(assistant)) ;
				corretto = false ;
				break ;
			}
		}
		break ;
	case 2: {
		GtkCalendar *calendario = GTK_CALENDAR(gtk_builder_get_object(builder, "giorno_esame")) ;
		unsigned int giorno_att, giorno_sel, mese_att, mese_sel, anno_att, anno_sel ;
		data_odierna_uint(giorno_att, mese_att, anno_att) ;
		gtk_calendar_get_date(calendario, &anno_sel, &mese_sel, &giorno_sel) ;
		mese_sel++ ;
		VER(cout << "Data scelta: " << giorno_sel << '/' << mese_sel << '/' << anno_sel << endl ) ;
		//Controllo sulla validita' della data (deve essere nel futuro)
		if (anno_att > anno_sel || (anno_att == anno_sel && mese_att > mese_sel) || 
		   (anno_att == anno_sel && mese_att == mese_sel && giorno_att > giorno_sel)) {
		   		messaggio_errore ("Scegliere un giorno nel futuro", GTK_WINDOW(assistant)) ;
		   		corretto = false ;
		   		break ;
		} else { // Se la data e' corretta aggiorna la label della data nel recap
			stringstream buff ;
			buff << "Giorno dell'esame: " << giorno_sel << '/' << mese_sel << '/' << anno_sel ;
			gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "giorno_esam")), buff.str().c_str()) ;
		}
		break ;
	}
	default:
		break ;
	}
	if (!corretto)
		gtk_assistant_previous_page (assistant) ;
}

/** Crea un nuovo esame, non controlla la validita' delle informazioni dal momento che il precedente handler si e'
	assicurato che ad ogni passo tutte le informazioni fossero corrette */
extern "C" void applica_wizard (GtkAssistant *assistant, gpointer user_data)
{
	Gtk
}







