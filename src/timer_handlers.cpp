/** @file
	File che si occupa di gestire gli handler del ::pomodoro_t per l'interfaccia grafica
*/
#include <gtk/gtk.h>
#include <sstream>
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
#include "../lib/notify.h"

/** Tipo per identificare il tipo di timer che e' in corso.*/
enum tipo_timer_t {LAVORO, PAUSA_CORTA, PAUSA_LUNGA} ;

/** Mantiene in memoria il tag per la GSource per eliminarla in un secondo momento.*/
static unsigned int timer_tag, 
/** Variabile che memorizza il tempo mancante al termine del timer.*/
					timer_tempo ;
static bool timer_funzionante = false ;
/** Variabile che indica il tipo del timer.*/
static tipo_timer_t timer_tipo ;

/** Scrive nella label del timer il tempo mancante al termine del timer.*/
static void imposta_label_timer (unsigned int sec, GtkLabel *label)
{
	VER(cout << "Aggiorno label con " << sec << " secondi" << endl ) ;
	stringstream buff ;
	buff << (sec/60)/10 << (sec/60)%10 << ':' << (sec%60)/10 << (sec%60)%10 ;
	gtk_label_set_text(label, buff.str().c_str()) ;
}

/** Carica e imposta la label del timer.*/
extern "C" void timer_show (GtkWidget *widget, gpointer user_data)
{
	if (!carica_timer()) {
		DEB(cout << "Impossibile caricare il timer" << endl ) ;
		messaggio_errore("Impossibile caricare i tempi del timer, si useranno quelli standard (25/5/15)",
						  GTK_WINDOW(widget)) ;
		timer.lavoro = 25 ; timer.pausa_corta = 5 ; timer.pausa_lunga = 15 ;
	}
	imposta_label_timer(timer.lavoro*60, GTK_LABEL(gtk_builder_get_object(builder, "timer"))) ;
}

/** Funzione per il countdown.

	E' un puntatore ad una funzione di tipo GSourceFunc per poter essere chiamata ad ogni intervallo di tempo
	(in questo caso ogni secondo).
	Il countdown e' implementato tramite una GSource.
	Quando il timer arriva a 0 la funzione invia una notifica per avvertire l'utente che il timer e' concluso
	@return G_SOURCE_CONTINUE se la funzione dovra' essere chiamata un'altra volta
	@return G_SOURCE_REMOVE se la funzione dovra' cessare
*/
static gboolean countdown (gpointer user_data)
{
	timer_tempo -- ;
	imposta_label_timer(timer_tempo, GTK_LABEL(gtk_builder_get_object(builder, "timer"))) ;
	if ( timer_tempo == 0 ) {
		timer_funzionante = false ;
		NotifyNotification * stop = notify_notification_new ("CStudy", "Il timer e' terminato", "dialog-information") ;
		notify_notification_show (stop, NULL) ;
		g_object_unref(G_OBJECT(stop)) ;
		GtkAdjustment *value ;
		switch (timer_tipo) {
		case LAVORO:
			value = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "num_lavoro")) ;
			break ;
		case PAUSA_CORTA:
			value = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "num_p_corte")) ;
			break ;
		case PAUSA_LUNGA:
			value = GTK_ADJUSTMENT(gtk_builder_get_object(builder, "num_p_lunghe")) ;
			break ;
		default:
			break ;
		}
		gtk_adjustment_set_value(value, gtk_adjustment_get_value(value) + 1) ;
		return G_SOURCE_REMOVE ;
	}
	else
		return G_SOURCE_CONTINUE ;
}

/** Imposta il timer per il tempo di lavoro.

	Imposta il timer come funzionante ed il tipo di lavoro, poi avvia il countdown
	
*/
extern "C" void lavoro_timer (GtkButton *button, gpointer user_data)
{
	timer_tempo = timer.lavoro * 60 ;
	VER(cout << "Timer lavoro = " << timer_tempo << endl ) ;
	timer_funzionante = true ;
	timer_tipo = LAVORO ;
	imposta_label_timer(timer_tempo, GTK_LABEL(user_data)) ;
	timer_tag = g_timeout_add_seconds(1, countdown, NULL) ;
}

/** Imposta il timer per il tempo della pausa corta.

	Imposta il timer come funzionante ed il tipo di lavoro, poi avvia il countdown
	
*/
extern "C" void pausa_corta_timer (GtkButton *button, gpointer user_data)
{
	timer_tempo = timer.pausa_corta * 60 ;
	VER(cout << "Timer pausa corta = " << timer_tempo << endl ) ;
	timer_funzionante = true ;
	timer_tipo = PAUSA_CORTA ;
	imposta_label_timer(timer_tempo, GTK_LABEL(user_data)) ;
	timer_tag = g_timeout_add_seconds(1, countdown, NULL) ;
}

/** Imposta il timer per il tempo della pausa lunga.

	Imposta il timer come funzionante ed il tipo di lavoro, poi avvia il countdown
	
*/
extern "C" void pausa_lunga_timer (GtkButton *button, gpointer user_data)
{
	timer_tempo = timer.pausa_lunga * 60 ;
	VER(cout << "Timer pausa lunga = " << timer_tempo << endl ) ;
	timer_funzionante = true ;
	timer_tipo = PAUSA_LUNGA ;
	imposta_label_timer(timer_tempo, GTK_LABEL(user_data)) ;
	timer_tag = g_timeout_add_seconds(1, countdown, NULL) ;
}

/** Ferma il timer

	Se il programma riceve il segnale di delete event (quindi il timer non e' piu' richiesto) interrompe il countdown
	e imposta timer_funzionante a false
*/
extern "C" gboolean timer_delete_event (GtkWidget *widget, GdkEvent  *event, gpointer user_data)
{
	if (timer_funzionante) {
		VER(cout << "Fermo il timer" << endl ) ;
		g_source_remove(timer_tag) ;
		timer_funzionante = false ;
	}
	return gtk_widget_hide_on_delete(widget) ;
}

/** Invoca la finestra di dialogo per modificare timer.

	Alla chiusura della finestra di dialogo vengono recuperati i valori all'interno delle GtkSpinButton nella finestra
	e si aggiornano i valori di ::pomodoro_t::lavoro, ::pomodoro_t::pausa_corta, ::pomodoro_t::pausa_lunga
*/
extern "C" void impostazioni_clicked (GtkButton *button, gpointer user_data)
{
	int risposta = gtk_dialog_run(GTK_DIALOG(user_data)) ;
	if ( risposta == 1 || risposta == GTK_RESPONSE_DELETE_EVENT ) {
		VER(cout << "Modifico i timer" << endl ) ;
		timer.lavoro = static_cast<unsigned int>(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "lavoro")))) ;
		timer.pausa_corta = static_cast<unsigned int>(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pausa_corta")))) ;
		timer.pausa_lunga = static_cast<unsigned int>(gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pausa_lunga")))) ;
		stringstream buff ;
		buff << timer.lavoro << ":00" ;
		gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "timer")), buff.str().c_str()) ;
		if (!salva_timer())
			messaggio_errore("Impossibile scrivere i dati, salvataggio fallito", GTK_WINDOW(user_data)) ;
	}
	gtk_widget_hide(GTK_WIDGET(user_data)) ;
}

/** Imposta le GtkSpinButton al valore corretto al mostrarsi della schermata di impostazioni del timer.*/
extern "C" void impostazioni_show (GtkWidget *widget, gpointer user_data)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "lavoro")), static_cast<double>(timer.lavoro)) ;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pausa_corta")), static_cast<double>(timer.pausa_corta)) ;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pausa_lunga")), static_cast<double>(timer.pausa_lunga)) ;
}














