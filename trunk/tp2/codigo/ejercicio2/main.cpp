#include <cstdlib>
#include <iostream>
#include <cassert>
#include "Grafo.h"
#include "BuscadorCaminoTCI.h"

using namespace std;

void test_grafo() {
    pair<unsigned, unsigned> p1 = make_pair(0,1);
    pair<unsigned, unsigned> p2 = make_pair(0,2);
    pair<unsigned, unsigned> p3 = make_pair(3,4);

    list<pair<unsigned, unsigned> > l;
    l.push_back(p1);
    l.push_back(p2);
    l.push_back(p3);
    l.push_back(p3);

    Grafo g = Grafo(5, l);
    assert(!(g.estanConectados(2,4)));
    assert(!(g.estanConectados(0,3)));
    assert(g.estanConectados(0,1));
    assert(g.estanConectados(0,2));
    assert(g.estanConectados(3,4));
    assert(g.estanConectados(4,3));
    assert(g.n == 5);
    assert(g.m == 3);
    cout << "Grafo: todos los tests pasaron satisfactoriamente!" << endl;
}

void test_buscadorcamino() {
    list<pair<unsigned,unsigned> > l;
    l.push_back(make_pair(1,2));
    l.push_back(make_pair(2,3));
    l.push_back(make_pair(2,4));
    l.push_back(make_pair(2,5));
    l.push_back(make_pair(3,0));
    l.push_back(make_pair(4,0));
    l.push_back(make_pair(4,5));

    Grafo g = Grafo(6,l);
    BuscadorCaminoTCI b = BuscadorCaminoTCI(g);
    
    b.buscarCamino();
    cout << "BuscadorCaminoTCI: todos los tests pasaron satisfactoriamente!" << endl;
}

int main() {
    test_grafo();
    test_buscadorcamino();
    return 0;
}