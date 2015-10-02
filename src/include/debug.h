/** @file
	Header file che fornisce il sistema di debug a due livelli (DEB, VER)
*/
#define DBG(A, B) {if ((A) & MASK) {B ;}}
/** Macro per inserire parti di codice di debug (tipicamente in caso di errori).

	Si attiva se ::MASK e' uguale a 1
*/
#define DEB(a) DBG(1, a)
/** Macro per inserire parti di codice di debug (di tipo piu' verboso, per esempio valori di variabili).

	Si attiva se ::MASK e' uguale a 2
*/
#define VER(a) DBG(2, a)
