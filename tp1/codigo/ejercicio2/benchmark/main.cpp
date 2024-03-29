#include <iostream>
#include <assert.h>
#include <string>
#include <fstream>

#include "SolucionPosible.h"

unsigned long long int operaciones = 0LL;

using namespace std;

void camionAux(SolucionPosible& candActual, Cosa* cosas, unsigned capacidad,unsigned indice, unsigned cant,SolucionPosible& mejorSol);


void camion(Camion& c,SolucionPosible& mejorSol) {
    operaciones = 0LL;
    operaciones = 4*c.cantCosas + 4;
    SolucionPosible * s = new SolucionPosible(c.cantCosas);
    camionAux(*s,c.cosas,c.capacidad,0,c.cantCosas,mejorSol);
    delete s;
    cout << operaciones << endl;
}


// Variables utilizadas:
// capacidad: capacidad del camion
// cosas: arreglo con las cosas que hay para llevar
// indice: posicion del arreglo que estoy tratando de meter
// cant: tama�o del arreglo cosas
// candActual: la solucion que voy construyendo
// mejorSol: la mejor solucion encontrada hasta ahora

void camionAux(SolucionPosible& candActual, Cosa* cosas, unsigned capacidad,unsigned indice, unsigned cant,SolucionPosible& mejorSol){
    // cuando ya llegue hasta el ultimo elemento paro, pero me fijo si consegui una mejor solucion
    operaciones = operaciones + 3;
    if(indice == cant && candActual.valor > mejorSol.valor) {
        operaciones = operaciones + candActual.cantCosas;
        mejorSol = candActual;
    }
    else {
        // caso base: indice == cant
        operaciones++;
        if(indice != cant) {
            // caso recursivo
            operaciones = operaciones + 2;
            if(candActual.peso + cosas[indice].peso <= capacidad) {
                operaciones = operaciones + 5;
                candActual.agregar(indice,cosas[indice].peso,cosas[indice].valor);
                camionAux(candActual,cosas, capacidad, indice+1, cant, mejorSol);

                //sacamos el elemento agregado para hacer backtracking
                operaciones = operaciones + 5;
                candActual.sacar(indice,cosas[indice].peso, cosas[indice].valor);
            }
            operaciones++;
            camionAux(candActual,cosas, capacidad, indice+1, cant, mejorSol);
            operaciones++;
            if(candActual.valor > mejorSol.valor) {
                operaciones++;
                mejorSol = candActual;
            }
        }
    }
}


int main(int argc, char* argv[]) {

    // leo los datos de entrada
    string ruta;
    if(argc >= 2) {
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
    if(argc > 2) {
        salida = argv[2];
    } else {
        salida = "Tp1Ej2.out";
    }
    ofstream o (salida.c_str());

    // leo la secuencia de cosas
    while(caso != "Fin"){
        unsigned cantElem, capacidad;
        f >> cantElem;
        f >> capacidad;
        Cosa* cs = new Cosa[cantElem];
        for(unsigned i = 0; i < cantElem; i++) {
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
        for(unsigned int i = 0; i < cam.cantCosas; i++) {
            if(s->guardo[i]) {
                contador++;
            }
        }

        o << contador << " ";
        for(unsigned int i = 0; i < cam.cantCosas; i++) {
            if(s->guardo[i]) {
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
