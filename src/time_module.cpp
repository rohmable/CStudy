#include <sstream>
#include <ctime>
#include <cctype>
#ifdef DEBUG_MODE
	#include "debug.h"
	#include <iostream>
#else
	#define DEB(a)
	#define VER(a)
#endif
using namespace std ;

// Da main.cpp
extern unsigned int MASK ;

/** Converte una stringa contenente caratteri numerici nell' intero senza segno che rappresenta
	@param[in] stringa la stringa contenente il numero
	@param[in] size la grandezza della stringa (per controllare se la stringa contiene solo numeri)
	@param[out] num variabile in cui verra' inserito il numero
*/
static bool converti_stringa_in_numero (const char stringa[], const int size, unsigned int &num)
{
	stringstream buff ;
	for (int i = 0 ; i < size && stringa[i] != '\0' ; i++) {
		if (!isdigit(stringa[i])) {
			num = -1 ;
			return false ;
		}
	}
	buff << stringa ;
	buff >> num ;
	return true ;
}

/** Fornisce la data odierna nel formato mm/gg/aaaa
	@param[in,out] g giorno
	@param[in,out] m mese
	@param[in,out] a anno
*/
extern bool data_odierna_uint (unsigned int &g, unsigned int &m, unsigned int &a)
{
	time_t t ;
	time (&t) ;
	tm *tempo = localtime (&t);
	const int DIGITS = 3, ANNO = 5 ;
	char giorno[DIGITS], mese[DIGITS], anno[ANNO] ;
	strftime(giorno, DIGITS, "%d", tempo) ;
	strftime(mese, DIGITS, "%m", tempo) ;
	strftime(anno, ANNO, "%Y", tempo) ;
	VER(cout << "Giorno di oggi: " << giorno << "/" << mese << "/" << anno << endl ) ;
	
	if (!converti_stringa_in_numero(giorno, DIGITS, g)) {
		DEB(cout << "Errore, la stringa \"giorno\" non contiene solo numeri!" << endl
				 << "\tgiorno: " << giorno << endl ) ;
		return false ;
	}
	if (!converti_stringa_in_numero(mese, DIGITS, m)) {
		DEB(cout << "Errore, la stringa \"mese\" non contiene solo numeri!" << endl
				 << "\tmese: " << mese << endl ) ;
		return false ;
	}
	if (!converti_stringa_in_numero(anno, ANNO, a)) {
		DEB(cout << "Errore, la stringa \"mese\" non contiene solo numeri!" << endl
				 << "\tmese: " << mese << endl ) ;
		return false ;
	}
	
	return true ;
}
