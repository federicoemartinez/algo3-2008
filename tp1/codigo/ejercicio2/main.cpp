#include <iostream>
#include <assert.h>
#include <string>
#include <fstream>

#include "SolucionPosible.h"

using namespace std;

void camionAux(SolucionPosible& candActual, Cosa* cosas, unsigned capacidad,unsigned indice, unsigned cant,SolucionPosible& mejorSol);


void camion(Camion& c,SolucionPosible& mejorSol) {
    SolucionPosible * s = new SolucionPosible(c.cantCosas);
    camionAux(*s,c.cosas,c.capacidad,0,c.cantCosas,mejorSol);
    delete s;
}


// Variables utilizadas:
// candActual: la solucion que voy construyendo
// cosas: arreglo con las cosas que hay para llevar
// capacidad: capacidad del camion
// indice: posicion del arreglo que estoy tratando de meter
// cant: tamaño del arreglo cosas
// mejorSol: la mejor solucion encontrada hasta ahora

void camionAux(SolucionPosible& candActual, 
               Cosa* cosas, 
               unsigned capacidad,
               unsigned indice, 
               unsigned cant,
               SolucionPosible& mejorSol) {

    // caso base: indice == cant
    // cuando llego al final paro y me fijo si consegui una mejor solucion
    if (indice == cant) {
        if (candActual.valor > mejorSol.valor) {
            mejorSol = candActual;
        }
    }
    // caso recursivo: indice != cant
    else {
        
        // si esta rama promete, la examino, en caso contrario
        // se examina directamente la rama que no contiene a este objeto
        if (candActual.peso + cosas[indice].peso <= capacidad) {
            candActual.agregar(indice,cosas[indice].peso,cosas[indice].valor);
            camionAux(candActual,cosas, capacidad, indice+1, cant, mejorSol);
            
            //sacamos el elemento agregado para hacer backtracking
            candActual.sacar(indice,cosas[indice].peso, cosas[indice].valor);
        }

        camionAux(candActual,cosas, capacidad, indice+1, cant, mejorSol);

        if (candActual.valor > mejorSol.valor) {
            mejorSol = candActual;
        }
    }
}


int main(int argc, char* argv[]) {

    // leo los datos de entrada
    string ruta;
    if (argc >= 2) {
        ruta = argv[1];
    } else {
        ruta="Tp1Ej2.in";
    }
    fstream f (ruta.c_str());
    assert(f.is_open());
    string caso;
    f >> caso;

    // preparo el archivo de salida
    string salida;
    if (argc > 2) {
        salida = argv[2];
    } else {
        salida = "Tp1Ej2.out";
    }
    ofstream o (salida.c_str());

    // leo la secuencia de cosas
    while (caso != "Fin") {
        unsigned cantElem, capacidad;
        f >> cantElem;
        f >> capacidad;
        Cosa* cs = new Cosa[cantElem];
        for (unsigned i = 0; i < cantElem; i++) {
            unsigned peso, valor;
            f >> valor;
            f >> peso;
            cs[i] = Cosa(valor,peso);
        }

        // resuelvo este caso
        Camion cam = Camion(cs,capacidad,cantElem);
        SolucionPosible* s = new SolucionPosible(cam.cantCosas);
        camion(cam,*s);

        // almaceno la salida del programa
        o << s->valor << " ";

        unsigned contador = 0;
        for (unsigned int i = 0; i < cam.cantCosas; i++) {
            if (s->guardo[i]) {
                contador++;
            }
        }

        o << contador << " ";
        for (unsigned int i = 0; i < cam.cantCosas; i++) {
            if (s->guardo[i]) {
                o << i+1 << " ";
            }
        }

        o << endl;
        delete [] cs;
        delete s;

        f >> caso;
    }
    return 0;
}
