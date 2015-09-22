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

#include "include/main.h"
#include "include/esame.h"
#include "include/save_load.h"
#include "../lib/notify.h"

enum tipo_timer_t {LAVORO, PAUSA_CORTA, PAUSA_LUNGA} ;

static unsigned int timer_tag, timer_tempo ;
static unsigned int numero_lavoro = 0, numero_pa_corte = 0, numero_pa_lunghe = 0 ;
static bool timer_funzionante = false ;
static tipo_timer_t timer_tipo ;

extern "C" void timer_show (GtkWidget *widget, gpointer user_data)
{
	carica_timer() ;
	stringstream buff ;
	buff << timer.lavoro << ":00" ;
	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "timer")), buff.str().c_str()) ;
}

static gboolean countdown (gpointer user_data)
{
	timer_tempo -- ;
	VER(cout << "Tempo = " << timer_tempo << endl ) ;
	stringstream buff ;
	buff << timer_tempo/60 << ':' << timer_tempo%60 ;
	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder, "timer")), buff.str().c_str()) ;
	if ( timer_tempo == 0 ) {
		timer_funzionante = false ;
		NotifyNotification * stop = notify_notification_new ("CStudy", "Il timer e' terminato", "dialog-information") ;
		notify_notification_show (stop, NULL) ;
		g_object_unref(G_OBJECT(stop)) ;
		switch (timer_tipo) {
		case LAVORO:
			numero_lavoro ++ ;
			GtkSpinButton *button = GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "lavoro_num")) ;
			gtk_spin_button_set_value (button, gtk_spin_button_get_value(button) + 1) ;
			break ;
		case PAUSA_CORTA:
			numero_pa_corte ++ ;
			break ;
		case PAUSA_LUNGA:
			numero_pa_lunghe ++ ;
			break ;
		}
		return G_SOURCE_REMOVE ;
	}
	else
		return G_SOURCE_CONTINUE ;
}

extern "C" void lavoro_timer (GtkButton *button, gpointer user_data)
{
	timer_tempo = timer.lavoro * 60 ;
	VER(cout << "Timer lavoro = " << timer_tempo << endl ) ;
	timer_funzionante = true ;
	timer_tipo = LAVORO ;
	timer_tag = g_timeout_add_seconds(1, countdown, NULL) ;
}

extern "C" void pausa_corta_timer (GtkButton *button, gpointer user_data)
{
	timer_tempo = timer.pausa_corta * 60 ;
	VER(cout << "Timer pausa corta = " << timer_tempo << endl ) ;
	timer_funzionante = true ;
	timer_tipo = PAUSA_CORTA ;
	timer_tag = g_timeout_add_seconds(1, countdown, NULL) ;
}

extern "C" void pausa_lunga_timer (GtkButton *button, gpointer user_data)
{
	timer_tempo = timer.pausa_lunga * 60 ;
	VER(cout << "Timer pausa lunga = " << timer_tempo << endl ) ;
	timer_funzionante = true ;
	timer_tipo = PAUSA_LUNGA ;
	timer_tag = g_timeout_add_seconds(1, countdown, NULL) ;
}

extern "C" gboolean timer_delete_event (GtkWidget *widget, GdkEvent  *event, gpointer user_data)
{
	if (timer_funzionante) {
		VER(cout << "Fermo il timer" << endl ) ;
		g_source_remove(timer_tag) ;
		timer_funzionante = false ;
	}
	return gtk_widget_hide_on_delete(widget) ;
}

extern "C" void impostazioni_clicked (GtkButton *button, gpointer user_data)
{
	int risposta = gtk_dialog_run(GTK_DIALOG(user_data)) ;
	if ( risposta == 1 ) {
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

extern "C" void impostazioni_show (GtkWidget *widget, gpointer user_data)
{
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "lavoro")), static_cast<double>(timer.lavoro)) ;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pausa_corta")), static_cast<double>(timer.pausa_corta)) ;
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(gtk_builder_get_object(builder, "pausa_lunga")), static_cast<double>(timer.pausa_lunga)) ;
}














