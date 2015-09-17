#include <gtk/gtk.h>
#include <sstream>
#ifdef DEBUG_MODE
	#include "./include/debug.h"
	#include <iostream>
#else
	#define DEB(a)
	#define VER(a)
#endif
using namespace std ;

// Da save_load.cpp
bool nuovo_esame (const char [], int , int [], int [], int , int) ;

// Da time_module.cpp
bool data_odierna_uint (unsigned int &, unsigned int &, unsigned int &) ;
int diff_giorni_da_attuale (int g, int m, int a) ;
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
	case 3: {
		GtkAdjustment *g_rip = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "giorni_ripasso")) ;
		int giorni_ripasso = static_cast<int>(gtk_adjustment_get_value(g_rip)) ;
		unsigned int g_sel, m_sel, a_sel ;
		gtk_calendar_get_date(GTK_CALENDAR(gtk_builder_get_object(builder, "giorno_esame")), &a_sel, &m_sel, &g_sel) ;
		if (giorni_ripasso >= diff_giorni_da_attuale(static_cast<int>(g_sel), static_cast<int>(m_sel), static_cast<int>(a_sel))) {
			messaggio_errore("Il numero di giorni di ripasso deve essere minore di quelli che mancano all'esame", GTK_WINDOW(assistant)) ;
			corretto = false ;
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
	GtkCalendar *giorno_esame = GTK_CALENDAR(gtk_builder_get_object(builder, "giorno_esame")) ;
	GtkAdjustment *num_pagine = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "num_pagine")) ,
				  *giorni_studio = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "sett")),
				  *giorni_ripasso = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "giorni_ripasso")) ;
	unsigned int giorno_att, mese_att, anno_att,
				 giorno_sel, mese_sel, anno_sel ;
	data_odierna_uint(giorno_att, mese_att, anno_att) ;
	gtk_calendar_get_date(giorno_esame, &anno_sel, &mese_sel, &giorno_sel) ;
	int pag = static_cast<int>(gtk_adjustment_get_value(num_pagine)),
		studio_per_sett = static_cast<int>(gtk_adjustment_get_value(giorni_studio)),
		gg_ripasso = static_cast<int>(gtk_adjustment_get_value(giorni_ripasso)),
		gg_studio = diff_giorni_da_attuale (static_cast<int>(giorno_sel), static_cast<int>(mese_sel), static_cast<int>(anno_sel)) ;
	gg_studio -= gg_ripasso ;
	int pagine_al_giorno = (pag / (gg_studio/studio_per_sett)) ;
	// La divisione potrebbe dare 0 pagine da studiare come parte intera della divisione, visto che e' impossibile
	// si incrementa di uno
	if (pagine_al_giorno == 0)
		pagine_al_giorno ++ ;
	VER(cout << "Pagine: " << pag << endl
			 << "Giorni di studio: " << gg_studio << endl
			 << "Studio alla settimana: " << studio_per_sett << endl
			 << "Pagine al giorno = (" << pag << " / (" << gg_studio << '/' << studio_per_sett << ")) = " << pagine_al_giorno << endl ) ;
	
	GtkMessageDialog *conferma = GTK_MESSAGE_DIALOG(user_data) ;
	stringstream buff ;
	buff << "Si devono studiare (almeno) " << pagine_al_giorno << " pagine al giorno\nContinuare?" ;
	gtk_message_dialog_format_secondary_text(conferma, buff.str().c_str()) ;
	int conf = gtk_dialog_run(GTK_DIALOG(conferma)) ;
	if (conf == GTK_RESPONSE_YES) {
		const int DATA = 3 ;
		int inizio[DATA] = {giorno_att, mese_att, anno_att},
			esame[DATA] = {giorno_sel, mese_sel, anno_sel} ;
		GtkEntryBuffer *nome_esame = GTK_ENTRY_BUFFER(gtk_builder_get_object(builder, "Nome_esame")) ;
		if (!nuovo_esame(gtk_entry_buffer_get_text(nome_esame), pagine_al_giorno, inizio, esame, gg_studio, gg_ripasso))
			messaggio_errore("C'e' stato un errore, impossibile creare il nuovo esame", GTK_WINDOW(assistant)) ;
	}
	else
		nascondi_finestra(reinterpret_cast<gpointer>(conferma)) ;
}







