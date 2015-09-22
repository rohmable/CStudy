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

#include "include/esame.h"
#include "include/time_module.h"
#include "include/main.h"
#include "include/save_load.h"

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
	gtk_window_set_transient_for(GTK_WINDOW(user_data), GTK_WINDOW(widget)) ;
	gtk_window_set_attached_to(GTK_WINDOW(user_data), GTK_WIDGET(widget)) ;
	int risposta = gtk_dialog_run(GTK_DIALOG(user_data)) ;
	if (risposta == GTK_RESPONSE_YES) {
		gtk_main_quit() ;
		return false ;
	}
	else {
		nascondi_finestra(reinterpret_cast<gpointer>(GTK_DIALOG(user_data))) ;
		return true ;
	}
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

/** Controlla se la data cliccata e' in un tempo passato, in tal caso rende la pagina dell'assistant incompleta e impedisce di continuare
	altrimenti permette di proseguire */
extern "C" void giorno_cliccato (GtkCalendar *calendar, gpointer user_data)
{
	GtkAssistant *wizard = GTK_ASSISTANT(gtk_widget_get_toplevel(GTK_WIDGET(calendar))) ;
	guint giorno_att, mese_att, anno_att,
		  giorno_sel, mese_sel, anno_sel ;
	gtk_calendar_get_date(calendar, &anno_sel, &mese_sel, &giorno_sel) ;
	if (!data_odierna_uint(giorno_att, mese_att, anno_att)) {
		messaggio_errore("Impossibile recuperare la data odierna", GTK_WINDOW(wizard)) ;
		return ;
	}
	mese_sel ++ ; // Viene portato da una data di tipo 0-11 a 1-12
	VER(cout << "Data selezionata: " << giorno_sel << '/' << mese_sel << '/' << anno_sel << endl ) ;
	if (anno_att > anno_sel || (anno_att == anno_sel && mese_att > mese_sel) || 
		(anno_att == anno_sel && mese_att == mese_sel && giorno_att > giorno_sel))
			gtk_assistant_set_page_complete(wizard, gtk_assistant_get_nth_page(wizard, gtk_assistant_get_current_page(wizard)), FALSE) ;
	else
		gtk_assistant_set_page_complete(wizard, gtk_assistant_get_nth_page(wizard, gtk_assistant_get_current_page(wizard)), TRUE) ; 
}

/** Controlla se il numero di giorni di ripasso e' minore del numero di giorni che separano la data attuale da quella dell'esame
	Se il numero di giorni di ripasso e' troppo grande impedisce di continuare e mostra un errore*/
extern "C" void giorni_ripasso_modificato (GtkSpinButton *spin_button, gpointer user_data)
{
	guint giorno_sel, mese_sel, anno_sel ;
	gtk_calendar_get_date(GTK_CALENDAR(user_data), &anno_sel, &mese_sel, &giorno_sel) ;
	int diff_gg = diff_giorni_da_attuale(static_cast<int>(giorno_sel), static_cast<int>(mese_sel), static_cast<int>(anno_sel)),
		gg_ripasso = gtk_spin_button_get_value_as_int(spin_button) ;
	gboolean pagina_completa = FALSE ;
	VER(cout << "Differenza giorni: " << diff_gg - gg_ripasso << endl ) ;
	if ( diff_gg <= gg_ripasso )
		gtk_revealer_set_reveal_child(GTK_REVEALER(gtk_builder_get_object(builder, "revealer1")), TRUE) ;
	else {
		gtk_revealer_set_reveal_child(GTK_REVEALER(gtk_builder_get_object(builder, "revealer1")), FALSE) ;
		pagina_completa = TRUE ;
	}
	GtkAssistant *wizard = GTK_ASSISTANT(gtk_widget_get_toplevel(GTK_WIDGET(spin_button))) ;
	int pag_wizard = gtk_assistant_get_current_page(wizard) ;
	gtk_assistant_set_page_complete(wizard, gtk_assistant_get_nth_page(wizard, pag_wizard), pagina_completa) ;
}

static void messaggio_pagine_al_giorno (int pagine)
{
	GtkMessageDialog *diag = GTK_MESSAGE_DIALOG(gtk_builder_get_object(builder, "messagedialog2")) ;
	gtk_message_dialog_format_secondary_text(diag, "Dovrai studiare %d pagine al giorno", pagine) ;
	int risposta = gtk_dialog_run(GTK_DIALOG(diag)) ;
	if (risposta == GTK_RESPONSE_OK || risposta == GTK_RESPONSE_DELETE_EVENT)
		gtk_widget_hide(GTK_WIDGET(diag)) ;
}

/** Crea un nuovo esame, non controlla la validita' delle informazioni dal momento che il precedente handler si e'
	assicurato che ad ogni passo tutte le informazioni fossero corrette */
extern "C" void applica_wizard (GtkAssistant *assistant, gpointer user_data)
{
	GtkCalendar *calendario = GTK_CALENDAR(gtk_builder_get_object(builder, "calendar1")) ;
	int pagine = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spinbutton1"))),
		studio_per_settimana = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spinbutton2"))),
		giorni_ripasso = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "spinbutton3"))) ;
	percorso = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(gtk_builder_get_object(builder, "filechooserwidget1"))) ;
	if (percorso == NULL) {
		gtk_revealer_set_reveal_child(GTK_REVEALER(user_data), TRUE) ;
		return ;
	}
	else {
		guint g, m, a ;
		gtk_calendar_get_date(calendario, &a, &m, &g) ;
		m ++ ;
		if (!nuovo_esame(g, m, a, pagine, studio_per_settimana, giorni_ripasso))
			messaggio_errore("Impossibile creare l'esame, hai per caso scelto un percorso protetto da scrittura?", GTK_WINDOW(assistant)) ;
		else {
			messaggio_pagine_al_giorno(calcola_pagine_al_giorno(g, m, a, pagine, studio_per_settimana, giorni_ripasso)) ;
			gtk_widget_hide(GTK_WIDGET(assistant)) ;
			gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "Menu"))) ;
			gtk_widget_show_all(GTK_WIDGET(gtk_builder_get_object(builder, "Main_Menu"))) ;
		}
	}
}

extern "C" void carica (GtkButton *button, gpointer user_data)
{
	percorso = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(user_data)) ;
	if (!carica_base())
		messaggio_errore("Impossibile caricare l'esame, hai per caso sbagliato cartella?", GTK_WINDOW(user_data)) ;
	else {
		gtk_widget_hide(GTK_WIDGET(user_data)) ;
		gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder, "Menu"))) ;
		gtk_widget_show_all(GTK_WIDGET(gtk_builder_get_object(builder, "Main_Menu"))) ;
	}
}

extern "C" void main_menu_shown (GtkWidget *widget, gpointer user_data)
{
	stringstream buff ;
	int giorni_all_esame = diff_giorni_da_attuale(static_cast<int>(esame.g_esame), static_cast<int>(esame.m_esame - 1), static_cast<int>(esame.a_esame)) ;
	if (giorni_all_esame > 1)
		buff << "Mancano " << giorni_all_esame << " giorni all'esame" ;
	else
		buff << "Manca " << giorni_all_esame << " giorno all'esame" ;
	gtk_label_set_text(GTK_LABEL(user_data), buff.str().c_str()) ;
}




