#define DBG(A, B) {if ((A) & MASK) {B ;}}
#define DEB(a) DBG(1, a)
#define VER(a) DBG(2, a)
