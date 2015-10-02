/** @file
	File che si occupa del salvataggio e del caricamento di ::esame_t, ::pomodoro_t e ::flashcards_t
*/
#include <fstream>
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
esame_t esame ; /**< Variabile di tipo ::esame_t in cui vengono contenuti tutti i dati di un esame.*/
pomodoro_t timer ; /**< Variabile di tipo ::pomodoro_t in cui vengono contenuti i tempi di ::pomodoro_t::lavoro,
						::pomodoro_t::pausa_corta e ::pomodoro_t::pausa_lunga*/
char *percorso ; /**< Percorso di caricamento/salvataggio dei dati scelto dall'utente */
GSList *lista = NULL ; /**< Lista singola contenente le ::flashcards_t */

/**
	Carica i dati da ::percorso/data.txt .						
																
	I dati vengono inseriti in esame.				
	@return true se la lettura viene effettuata con successo	
	@return false se il file non viene aperto correttamente		
*/																
extern bool carica_base ()
{
	VER(cout << "Carico i dati di base" << endl ) ;
	string file = percorso ;
	file += "/data.txt" ;
	VER(cout << "Percorso: " << file.c_str() << endl ) ;
	ifstream data (file.c_str()) ;
	if (!data) {
		data.close() ;
		DEB(cout << "Errore nel caricamento di \"" << file.c_str() << '\"' << endl ) ;
		return false ;
	}
	data >> esame.g_ultimo_avvio >> esame.m_ultimo_avvio >> esame.a_ultimo_avvio ;
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
	VER(cout << "Data esame: " << esame.g_esame << '/' << esame.m_esame << '/' << esame.a_esame << endl
			 << "Data ultimo avvio: " << esame.g_ultimo_avvio << '/' << esame.m_ultimo_avvio << '/' << esame.a_ultimo_avvio << endl
			 << "Pagine da studiare: " << esame.pag << endl
			 << "Giorni di studio alla settimana: " << esame.gg_studio_sett << endl
			 << "Giorni di ripasso: " << esame.gg_ripasso << endl
			 << "Pagine al giorno: " << esame.pag_per_giorno << endl ) ;
	DEB(cout << "Caricamento dati di base effettuato con successo" << endl ) ;
	data.close() ;
	return true ;
}

/** Carica i tempi del timer da ::percorso/times.txt .

	I dati vengono caricati in ::timer.
	@return true se la lettura e' stata effettuata correttamente
	@return false se l'apertura del file ha fallito
*/
extern bool carica_timer ()
{
	VER(cout << "Carico i dati del timer" << endl ) ;
	string file = percorso ;
	file += "/times.txt" ;
	VER(cout << "Percorso: " << file.c_str() << endl ) ;
	ifstream times(file.c_str()) ;
	if (!times) {
		times.close() ;
		DEB(cout << "Errore nel caricamento di \"" << file.c_str() << '\"' << endl ) ;
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

/** Scrive i tempi del timer in ::percorso/times.txt */
extern bool salva_timer ( )
{
	VER(cout << "Salvo i dati del timer" << endl ) ;
	string file = percorso ;
	file += "/times.txt" ;
	VER(cout << "Percorso: " << file.c_str() << endl ) ;
	ofstream times(file.c_str()) ;
	if (!times) {
		times.close() ;
		DEB(cout << "Impossibile aprire \"" << file.c_str() << "\" in scrittura" << endl ) ;
		return false ;
	}
	times << timer.lavoro << " # Lavoro" << endl
		  << timer.pausa_corta << " # Pausa Corta" << endl
		  << timer.pausa_lunga << " # Pausa Lunga" << endl ;
	times.close() ;
	return true ;
}

static void salva_esame (ofstream &stream)
{
	stream << esame.g_ultimo_avvio << ' ' << esame.m_ultimo_avvio << ' ' << esame.a_ultimo_avvio << " # Data ultimo avvio" << endl
		   << esame.g_esame << ' ' << esame.m_esame << ' ' << esame.a_esame << " # Data dell'esame" << endl
		   << esame.pag << " # Pagine da studiare" << endl
		   << esame.gg_studio_sett << " # Giorni di studio per settimana" << endl 
		   << esame.gg_ripasso << " # Giorni di ripasso prima dell'esame" << endl
		   << esame.pag_per_giorno << " # Pagine da studiare al giorno" << endl ;
	VER(cout << esame.g_ultimo_avvio << ' ' << esame.m_ultimo_avvio << ' ' << esame.a_ultimo_avvio << " # Data ultimo avvio" << endl
			 << esame.g_esame << ' ' << esame.m_esame << ' ' << esame.a_esame << " # Data dell'esame" << endl
		   	 << esame.pag << " # Pagine da studiare" << endl
		   	 << esame.gg_studio_sett << " # Giorni di studio per settimana" << endl 
		   	 << esame.gg_ripasso << " # Giorni di ripasso prima dell'esame" << endl
		   	 << esame.pag_per_giorno << " # Pagine da studiare al giorno" << endl ) ;
}

/** Crea i dati principali per un nuovo esame nel percorso.

	Viene popolato inoltre ::esame.

	@param[in] g,m,a data dell'esame
	@param[in] pag pagine da studiare
	@param[in] giorni_st_sett giorni di studio alla settimana
	@param[in] giorni_rip giorni di ripasso 
*/
extern bool nuovo_esame (unsigned int g, unsigned int m, unsigned int a, int pag, int giorni_st_sett, int giorni_rip)
{
	VER(cout << "Creo nuovo esame" << endl ) ;
	string file = percorso ;
	file += "/data.txt" ;
	VER(cout << "Salvo i dati principali su " << file.c_str() << endl ) ;
	ofstream data(file.c_str()) ;
	if (!data) {
		data.close() ;
		DEB(cout << "Errore nell'apertura del percorso \"" << file.c_str() << "\" in scrittura" << endl
				 << "Si e' per caso scelto un percorso protetto in scrittura?" << endl ) ;
		data.close() ;
		return false ;
	}
	
	VER(cout << "Popolo esame" << endl ) ;
	data_odierna_uint (esame.g_ultimo_avvio, esame.m_ultimo_avvio, esame.a_ultimo_avvio) ;
	esame.g_esame = g ; esame.m_esame = m ; esame.a_esame = a ;
	esame.pag = pag ; esame.gg_ripasso = giorni_rip ; esame.gg_studio_sett = giorni_st_sett ;
	esame.pag_per_giorno = calcola_pagine_al_giorno(g, m, a, pag,giorni_st_sett, giorni_rip) ;
	
	salva_esame(data) ;
	data.close() ;
	
	VER(cout << "Popolo timer" << endl ) ;
	timer.lavoro = 25 ; timer.pausa_corta = 5 ; timer.pausa_lunga = 15 ;
	salva_timer() ;
	DEB(cout << "Nuovo esame creato con successo" << endl ) ;
	return true ;
}

/** Utilizzata dalle funzioni ::aggiungi_carta e ::salva_carte. 

	@param[in] stream file su cui scrivere, all'interno della funzione non vengono fatti controlli.
*/
static void stampa_carta (ofstream &stream, const char *domanda, const char *risposta, int scadenza)
{
	stream << scadenza << endl
		   << "--INIZIO_DOMANDA--" << endl
		   << domanda << endl
		   << "--FINE_DOMANDA--" << endl << "--INIZIO_RISPOSTA--" << endl
		   << risposta << endl 
		   << "--FINE_RISPOSTA--" << endl ;
}

/** Salva in ::percorso/cards.txt una carta assieme alla scadenza. */
extern bool aggiungi_carta (const char *domanda, const char *risposta, int scadenza)
{
	string file = percorso ;
	file += "/cards.txt" ;
	ofstream carta (file.c_str(), ofstream::app) ;
	if (!carta) {
		carta.close() ;
		DEB(cout << "Impossibile aprire \"" << file << '\"' << endl ) ;
		return false ;
	}
	
	VER(cout << "Salvo la carta in \"" << file << endl ) ;
	stampa_carta(carta, domanda, risposta, scadenza) ;
	carta.close() ;
	DEB(cout << "Carta salvata con successo" << endl ) ;
	return true ;
}

/** Legge il testo tra due delimitatori, riga per riga.

	@param[in] stream lo streaming in input da cui leggere
	@param[in,out] stringa la stringa in cui salvare il testo
	@param[in] inizio prima stringa delimitatrice
	@param[in] fine seconda stringa delimitatrice
*/
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

/** Funzione usata per ::carica_carte per inserire gli elementi in una lista in maniera ordinata.

	g_slist_insert_sorted usa una GCompareFunc per determinare se l'elemento a deve essere inserito prima
	dell'elemento b. @n
	In generale:
	@return	<0	se a < b
	@return	0	se a == b
	@return	>0	se a > b
*/
extern gint compara_scadenze (gconstpointer a, gconstpointer b)
{
	const flashcards_t *carta_a = static_cast<const flashcards_t*>(a),
					   *carta_b = static_cast<const flashcards_t*>(b) ;
				 
	return carta_a->scadenza - carta_b->scadenza ;
}

/** Decrementa ::flashcards_t::scadenze.

	Se la data dell'ultimo avvio e' diversa da quella attuale la funzione sottrae il numero di giorni passati dall'ultimo
	avvio a ::flashcards_t::scadenze, in questo modo col passare dei giorni si possono ripassare le ::flashcards_t
*/
static void decrementa_scadenze ( )
{
	int diff_da_ultimo_avvio = diff_giorni_da_attuale(static_cast<int>(esame.g_ultimo_avvio), static_cast<int>(esame.m_ultimo_avvio - 1),
								static_cast<int>(esame.a_ultimo_avvio)) ;
	if (diff_da_ultimo_avvio == 0) 
		return ;
	if (!data_odierna_uint(esame.g_ultimo_avvio, esame.m_ultimo_avvio, esame.a_ultimo_avvio)) {
		DEB(cout << "Errore nell'aggiornamento dell'ultimo avvio" << endl ) ;
		return ;
	}
	
	string file = percorso ;
	file += "/data.txt" ;
	ofstream esame (file.c_str()) ;
	if (!esame) {
		DEB(cout << "Impossibile aprire \"" << file << endl ) ;
		return ;
	}
	salva_esame(esame) ;
	esame.close() ;
	
	VER(cout << "Decremento le scadenze di " << diff_da_ultimo_avvio << " giorni" << endl ) ;
	GSList *elem = lista ;
	while (elem != NULL) {
		static_cast<flashcards_t *>(elem->data)->scadenza -= diff_da_ultimo_avvio ;
		elem = elem->next ;
	}
}

/** Carica le carte da ::percorso/cards.txt e le inserisce in ordine in una lista.

	@return un puntatore alla testa di una lista ordinata se la lettura e' stata effettuata correttamente
	@return NULL se la lettura non e' stata effettuata correttamente
*/
extern void carica_carte ( )
{
	string file = percorso ;
	file += "/cards.txt" ;
	ifstream carte (file.c_str()) ;
	if (!carte) {
		DEB(cout << "Impossibile aprire \"" << file << endl ) ;
		lista = NULL ;
		return ;
	}
	
	VER(cout << "Carico le carte da " << file << endl ) ;
	flashcards_t *elem = NULL ;
	while (true) {
		elem = new flashcards_t ;
		carte >> elem->scadenza ;
		carte.ignore() ;
		if (carte.eof()) {
			VER(cout << "Caricamento terminato" << endl ) ;
			carte.close() ;
			delete elem ;
			decrementa_scadenze() ;
			return ;
		}
		leggi_tra_delim(carte, elem->domanda, "--INIZIO_DOMANDA--", "--FINE_DOMANDA--") ;
		leggi_tra_delim(carte, elem->risposta, "--INIZIO_RISPOSTA--", "--FINE_RISPOSTA--") ;
		gpointer data = static_cast<gpointer>(elem) ;
		lista = g_slist_insert_sorted(lista, data, compara_scadenze) ;
	}
}

/** Salva le carte nella ::lista in ::percorso/cards.txt .
	
	@return true se la scrittura e' stata effettuata correttamente
	@return false se il file non e' stato aperto correttamente
*/
extern bool salva_carte ( )
{
	string file = percorso ;
	file += "/cards.txt" ;
	ofstream carte (file.c_str(), ofstream::trunc) ;
	if (!carte) {
		DEB(cout << "Impossibile aprire \"" << file << endl ) ;
		return false ;
	}
	
	VER(cout << "Salvo le carte in \"" << file << '\"' << endl ) ;
	GSList *elem = lista ;
	while (elem != NULL) {
		stampa_carta(carte, static_cast<flashcards_t *>(elem->data)->domanda.c_str(), static_cast<flashcards_t *>(elem->data)->risposta.c_str(),
					static_cast<flashcards_t *>(elem->data)->scadenza) ;
		elem = elem->next ;
	}
	carte.close() ;
	return true ;
}






















