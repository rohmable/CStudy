#include <fstream>
#include <sstream>
#include <limits>
#include <string>
#include <glib.h>
#ifdef DEBUG_MODE
	#include "./include/debug.h"
	#include <iostream>
#else
	#define DEB(a)
	#define VER(a)
#endif
using namespace std ;

#include "include/time_module.h"
#include "include/esame.h"

extern unsigned int MASK ;
esame_t esame ;
pomodoro_t timer ;
stat_t stats ;
char *percorso ;
GSList *lista = NULL ;

/** Carica i dati da <percorso>/data.txt */
extern bool carica_base ()
{
	VER(cout << "Carico i dati di base" << endl ) ;
	stringstream file ;
	file << percorso << "/data.txt" ;
	VER(cout << "Percorso: " << file.str().c_str() << endl ) ;
	ifstream data (file.str().c_str()) ;
	if (!data) {
		data.close() ;
		DEB(cout << "Errore nel caricamento di \"" << file.str().c_str() << '\"' << endl ) ;
		return false ;
	}
	data >> esame.g_inizio >> esame.m_inizio >> esame.a_inizio ;
	data.ignore(numeric_limits<streamsize>::max(), '\n') ;
	data >> esame.g_esame >> esame.m_esame >> esame.a_esame ;
	data.ignore(numeric_limits<streamsize>::max(), '\n') ;
	data >> esame.pag ;
	data.ignore(numeric_limits<streamsize>::max(), '\n') ;
	data >> esame.gg_studio_sett ;
	data.ignore(numeric_limits<streamsize>::max(), '\n') ;
	data >> esame.gg_ripasso ;
	data.ignore(numeric_limits<streamsize>::max(), '\n') ;
	data >> esame.pag_per_giorno ;
	VER(cout << "Data inizio: " << esame.g_inizio << '/' << esame.m_inizio << '/' << esame.a_inizio << endl
			 << "Data esame: " << esame.g_esame << '/' << esame.m_esame << '/' << esame.a_esame << endl
			 << "Pagine da studiare: " << esame.pag << endl
			 << "Giorni di studio alla settimana: " << esame.gg_studio_sett << endl
			 << "Giorni di ripasso: " << esame.gg_ripasso << endl
			 << "Pagine al giorno: " << esame.pag_per_giorno << endl ) ;
	DEB(cout << "Caricamento dati di base effettuato con successo" << endl ) ;
	data.close() ;
	return true ;
}

/** Carica i tempi del timer da <percorso>/times.txt */
extern bool carica_timer ()
{
	VER(cout << "Carico i dati del timer" << endl ) ;
	stringstream file ;
	file << percorso << "/times.txt" ;
	VER(cout << "Percorso: " << file.str().c_str() << endl ) ;
	ifstream times(file.str().c_str()) ;
	if (!times) {
		times.close() ;
		DEB(cout << "Errore nel caricamento di \"" << file.str().c_str() << '\"' << endl ) ;
		return false ;
	}
	times >> timer.lavoro ;
	times.ignore(numeric_limits<streamsize>::max(), '\n') ;
	times >> timer.pausa_corta ;
	times.ignore(numeric_limits<streamsize>::max(), '\n') ;
	times >> timer.pausa_lunga ;
	times.close() ;
	VER(cout << "Lavoro: " << timer.lavoro << endl
			 << "Pausa corta: " << timer.pausa_corta << endl 
			 << "Pausa lunga: " << timer.pausa_lunga << endl ) ;
	return true ;
}

/** Scrive i tempi del timer in <percorso>/times.txt */
extern bool salva_timer ( )
{
	VER(cout << "Salvo i dati del timer" << endl ) ;
	stringstream file ;
	file << percorso << "/times.txt" ;
	VER(cout << "Percorso: " << file.str().c_str() << endl ) ;
	ofstream times(file.str().c_str()) ;
	if (!times) {
		times.close() ;
		DEB(cout << "Impossibile aprire \"" << file.str().c_str() << "\" in scrittura" << endl ) ;
		return false ;
	}
	times << timer.lavoro << " # Lavoro" << endl
		  << timer.pausa_corta << " # Pausa Corta" << endl
		  << timer.pausa_lunga << " # Pausa Lunga" << endl ;
	times.close() ;
	return true ;
}

/** Crea i dati principali per un nuovo esame nel ::percorso
	@param[in] percorso il percorso in cui salvare il file data.txt
	@param[in] g,m,a data dell'esame
	@param[in] pag pagine da studiare
	@param[in] giorni_st_sett giorni di studio alla settimana
	@param[in] giorni_rip giorni di ripasso */
extern bool nuovo_esame (unsigned int g, unsigned int m, unsigned int a, int pag, int giorni_st_sett, int giorni_rip)
{
	VER(cout << "Creo nuovo esame" << endl ) ;
	stringstream file ;
	file << percorso << "/data.txt" ;
	VER(cout << "Salvo i dati principali su " << file.str().c_str() << endl ) ;
	ofstream data(file.str().c_str()) ;
	if (!data) {
		data.close() ;
		DEB(cout << "Errore nell'apertura del percorso \"" << file.str().c_str() << "\" in scrittura" << endl
				 << "Si e' per caso scelto un percorso protetto in scrittura?" << endl ) ;
		data.close() ;
		return false ;
	}
	unsigned int g_att, m_att, a_att ;
	data_odierna_uint(g_att, m_att, a_att) ;
	data << g_att << ' ' << m_att << ' ' << a_att << " # Data inizio esame" << endl
		 << g << ' ' << m << ' ' << a << " # Data dell'esame" << endl
		 << pag << " # Pagine da studiare" << endl
		 << giorni_st_sett << " # Giorni di studio per settimana" << endl 
		 << giorni_rip << " # Giorni di ripasso prima dell'esame" << endl
		 << calcola_pagine_al_giorno(g, m, a, pag,giorni_st_sett, giorni_rip) << " # Pagine da studiare al giorno" ;
	VER(cout << "# Data inizio esame" << endl
		 	<< g_att << ' ' << m_att << ' ' << a_att << endl
			<< "# Data dell'esame" << endl
		 	<< g << ' ' << m << ' ' << a << endl
		 	<< "# Pagine da studiare" << endl
		 	<< pag << endl
		 	<< "# Giorni di studio per settimana" << endl 
		 	<< giorni_st_sett << endl
		 	<< "# Giorni di ripasso prima dell'esame" << endl
		 	<< giorni_rip << endl
		 	<< "# Pagine da studiare al giorno" << endl
		 	<< calcola_pagine_al_giorno(g, m, a, pag,giorni_st_sett, giorni_rip) << endl );
	data.close() ;
	VER(cout << "Popolo esame" << endl );
	data_odierna_uint(esame.g_inizio, esame.m_inizio, esame.a_inizio) ;
	esame.g_esame = g ; esame.m_esame = m ; esame.a_esame = a ;
	esame.pag = pag ; esame.gg_ripasso = giorni_rip ; esame.gg_studio_sett = giorni_st_sett ;
	esame.pag_per_giorno = calcola_pagine_al_giorno(g, m, a, pag,giorni_st_sett, giorni_rip) ;
	VER(cout << "Popolo timer" << endl ) ;
	timer.lavoro = 25 ; timer.pausa_corta = 5 ; timer.pausa_lunga = 15 ;
	VER(cout << "Azzero statistiche" << endl ) ;
	data_odierna_uint(stats.g_ultimo_agg, stats.m_ultimo_agg, stats.a_ultimo_agg) ;
	stats.lavoro_giorno = 0 ; stats.lavoro_sett = 0 ;
	salva_timer() ;
	DEB(cout << "Nuovo esame creato con successo" << endl ) ;
	return true ;
}

static void stampa_carta (ofstream &stream, const char *domanda, const char *risposta, int scadenza)
{
	stream << scadenza << endl
		   << "--INIZIO_DOMANDA--" << endl
		   << domanda << endl
		   << "--FINE_DOMANDA--" << endl << "--INIZIO_RISPOSTA--" << endl
		   << risposta << endl 
		   << "--FINE_RISPOSTA--" << endl ;
}

/** Salva in <percorso>/cards.txt una carta assieme alla scadenza */
extern bool aggiungi_carta (const char *domanda, const char *risposta, int scadenza)
{
	stringstream file ;
	file << percorso << "/cards.txt" ;
	ofstream carta (file.str().c_str(), ofstream::app) ;
	if (!carta) {
		carta.close() ;
		DEB(cout << "Impossibile aprire \"" << file.str().c_str() << '\"' << endl ) ;
		return false ;
	}
	VER(cout << "Salvo la carta in \"" << file.str().c_str() << endl ) ;
	stampa_carta(carta, domanda, risposta, scadenza) ;
	carta.close() ;
	DEB(cout << "Carta salvata con successo" << endl ) ;
	return true ;
}

/** Legge il testo da due delimitatori, riga per riga
	@param[in] stream lo streaming in input da cui leggere
	@param[in,out] stringa la stringa in cui salvare il testo
	@param[in] inizio prima stringa delimitatrice
	@param[in] fine seconda stringa delimitatrice */
static void leggi_tra_delim (istream &stream, string &stringa, const string &inizio, const string &fine)
{
	string buff ;
	getline(stream, buff) ;
	if (buff == inizio) {
		while (true) {
			getline(stream, buff) ;
			if (buff == fine)
				return ;
			else
				stringa += buff ;
		}
	}
}

extern gint compara_scadenze (gconstpointer a, gconstpointer b)
{
	const flashcards_t *carta_a = static_cast<const flashcards_t*>(a),
					   *carta_b = static_cast<const flashcards_t*>(b) ;
				 
	return carta_a->scadenza - carta_b->scadenza ;
}

/** Carica le carte da <percorso>/cards.txt e le inserisce in una lista
	Se la lettura del file fallisce o il file e' vuoto ritorna un puntatore NULL */
extern void carica_carte ( )
{
	stringstream file ;
	file << percorso << "/cards.txt" ;
	ifstream carte (file.str().c_str()) ;
	if (!carte) {
		DEB(cout << "Impossibile aprire \"" << file.str().c_str() << endl ) ;
		lista = NULL ;
		return ;
	}
	VER(cout << "Carico le carte da " << file.str().c_str() << endl ) ;
	flashcards_t *elem = NULL ;
	while (true) {
		elem = new flashcards_t ;
		carte >> elem->scadenza ;
		carte.ignore() ;
		if (carte.eof()) {
			VER(cout << "Caricamento terminato" << endl ) ;
			carte.close() ;
			delete elem ;
			return ;
		}
		leggi_tra_delim(carte, elem->domanda, "--INIZIO_DOMANDA--", "--FINE_DOMANDA--") ;
		leggi_tra_delim(carte, elem->risposta, "--INIZIO_RISPOSTA--", "--FINE_RISPOSTA--") ;
		gpointer data = static_cast<gpointer>(elem) ;
		lista = g_slist_insert_sorted(lista, data, compara_scadenze) ;
	}
}

extern bool salva_carte ( )
{
	stringstream file ;
	file << percorso << "/cards.txt" ;
	ofstream carte (file.str().c_str()) ;
	if (!carte) {
		DEB(cout << "Impossibile aprire \"" << file.str().c_str() << endl ) ;
		return false ;
	}
	
	GSList *elem = lista ;
	while (elem != NULL)
		stampa_carta(carte, static_cast<flashcards_t *>(elem->data)->domanda.c_str(), static_cast<flashcards_t *>(elem->data)->risposta.c_str(),
					static_cast<flashcards_t *>(elem->data)->scadenza) ;
	carte.close() ;
	return true ;
}






















