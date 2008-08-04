#include "SolucionExacta.h"
//TODO: agregar poda por propiedad del paper modificada
//TODO: agregar optimizacion de conteo de cruces usando la matriz
//TODO: agregar optimizacion de llenado de la matriz usando formulita loca
//TODO: hacer q la suma no se re calcule
//TODO: hacer q los valores de la matriz se mantengan (Podria ser necesario tener mas matrices).


unsigned cuantasCombinaciones(unsigned f, unsigned m) {
    unsigned c = 1;
    for (unsigned i = 0; i < m; i++) {
        c = c * (f + i + 1);
    }
    return c;
}


SolucionExacta :: SolucionExacta(Dibujo& original) {
    d = &original;
}

Dibujo SolucionExacta :: resolver() {
    inicializar();

    mejorDibujo = HeuristicaConstructiva(*d).construirSolucion();

    mejorar();

    return mejorDibujo;
}

void SolucionExacta :: inicializar() {
    // Obtengo las listas de nodos fijos y moviles con las que voy a operar
    l1.assign(d->grafo()->nodosEnP1().begin(), d->grafo()->nodosEnP1().end());
    l2.assign(d->grafo()->nodosEnP2().begin(), d->grafo()->nodosEnP2().end());

    fijo1.assign(d->nodosEnP1().begin(), d->nodosEnP1().end());
    fijo2.assign(d->nodosEnP2().begin(), d->nodosEnP2().end());
    fijosOriginales1=fijo1.size();
	fijosOriginales2=fijo2.size();
    vector<nodo>::const_iterator itv;

    movil1 = list<nodo>();
    movil2 = list<nodo>();

    itv = d->grafo()->nodosEnP1().begin();
    while (itv != d->grafo()->nodosEnP1().end()) {
        if (!d->perteneceAP1(*itv)) {
            movil1.push_back(*itv);
        }
        itv++;
    }
    itv = d->grafo()->nodosEnP2().begin();
    while (itv != d->grafo()->nodosEnP2().end()) {
        if (!d->perteneceAP2(*itv)) {
            movil2.push_back(*itv);
        }
        itv++;
    }

    // Determino cual de las 2 mitades tiene menos permutaciones
    // para llenarla primero en el arbol de combinaciones (como se
    // optimiza fuertemente la segunda mitad, resulta más eficiente
    // que ésta sea la más grande).
    int c1 = cuantasCombinaciones(fijo1.size(), movil1.size());
    int c2 = cuantasCombinaciones(fijo2.size(), movil2.size());
    if (c1 > c2) {
        invertirLados = true;
        swap(fijo1, fijo2);
        swap(movil1, movil2);
        swap(l1, l2);
    } else {
        invertirLados = false;
    }

    // Armo las listas de adyacencia que me van a servir para calcular
    // de forma eficiente los cruces de una cierta permutación de p1.

    vector<bool> estaEnFijo2 = vector<bool>(d->grafo()->cantNodos(), false);
	
    list<nodo>::const_iterator it = fijo2.begin();
    while (it != fijo2.end()) {
        estaEnFijo2[*it] = true;
        it++;
    }
	/* vector que voy a usar para armar tabla 2 al vuelo */
	estaEnFijo1 = vector<bool>(d->grafo()->cantNodos(), false);
	it = fijo1.begin();
    while (it != fijo1.end()) {
        estaEnFijo1[*it] = true;
        it++;
    }
    vector<bool> estaEnAdyP1 = vector<bool>(d->grafo()->cantNodos(), false);
    adyp1 = vector< list<nodo> >(d->grafo()->cantNodos());
    it = l1.begin();
    while (it != l1.end()) {
        adyp1[*it] = list<nodo>();
        estaEnAdyP1[*it] = true;
        it++;
    }

    list<eje>::const_iterator ite = d->grafo()->listaEjes().begin();
    while (ite != d->grafo()->listaEjes().end()) {
        if (estaEnAdyP1[ite->primero] && estaEnFijo2[ite->segundo]) {
            adyp1[ite->primero].push_back(ite->segundo);
        } else if (estaEnAdyP1[ite->segundo] && estaEnFijo2[ite->primero]) {
            adyp1[ite->segundo].push_back(ite->primero);
        }
        ite++;
    }

    // Armo un cache de posiciones de los nodos dentro de los fijos,
    // para evitar busquedas y agilizar conteos de cruces.
    unsigned i;

    posiciones1 = vector<unsigned>(d->grafo()->cantNodos());
    i = 0;
    it = fijo1.begin();
    while (it != fijo1.end()) {
        posiciones1[*it] = i;
        i++;
        it++;
    }

    posiciones2 = vector<unsigned>(d->grafo()->cantNodos());
    i = 0;
    it = fijo2.begin();
    while (it != fijo2.end()) {
        posiciones2[*it] = i;
        i++;
        it++;
    }


    // Armo un cache de cruces para reutilizar calculos
    unsigned n = d->grafo()->cantNodos();
    tabla1 = vector< vector<unsigned> >(n, vector<unsigned>(n));
    tabla2 = vector< vector<unsigned> >(n, vector<unsigned>(n));

    tabular1();
	minimosFijos =vector<nodo>(d->grafo()->cantNodos(),0);
	minimosMoviles = vector<nodo>(d->grafo()->cantNodos(),0);
    if (movil1.empty()) {
        tabular2();
		minimoPosible2 = 0;
				for(list<nodo>::iterator each = movil2.begin(); each != movil2.end(); each++){
					minimosMoviles[*each] = minimoDelNodoMoviles(*each);
					minimosFijos[*each] = minimoDelNodoFijos(*each);
					minimoPosible2+=minimosFijos[*each]+minimosMoviles[*each];
				}
    }

    // Cargo los cruces del dibujo original
    cruces = d->contarCruces();
	// cargo un candidato inicial

	list<nodo>::const_iterator it1, it2,it3;
    nodosEnComun = vector< vector<unsigned> >(n, vector<unsigned>(n));
	vector<unsigned> posAux(d->grafo()->cantNodos());
	i=0;
	for(it1 = l1.begin(); it1 != l1.end(); it1++){
       posAux[*it1]=i;
	   i++;
	}
    /*negrada para obtener nodos en comun, se puede hacer interseccion de forma mas rapida  */
    for (it1 = l2.begin(); it1 != l2.end(); it1++) {
		it3=it1;
		it3++;
        for (it2 = it3; it2 != l2.end(); it2++) {
                nodosEnComun[*it1][*it2] =(d->grafo()->ejes()[*it2]).size()*(d->grafo()->ejes()[*it1]).size()- (crucesEntre(*it1, *it2, l1, d->grafo()->ejes(), posAux)+crucesEntre(*it2, *it1, l1, d->grafo()->ejes(), posAux));
                nodosEnComun[*it2][*it1] = nodosEnComun[*it1][*it2];
        }
    }
    movilesOriginales2=movil2.size();
	movilesActuales2=movilesOriginales2;
	

}

unsigned SolucionExacta ::  minimoDelNodoMoviles(nodo v){
	unsigned c = 0;
    list<nodo>::const_iterator it1, it2;
        for (it2 = movil2.begin(); it2 != movil2.end(); it2++) {
            if (v < *it2) {
                c = c + min(tabla2[v][*it2], tabla2[*it2][v]);
            }
        }
    
    return c;
}

unsigned SolucionExacta ::  minimoDelNodoFijos(nodo v){
	unsigned c = 0;
    list<nodo>::const_iterator it2;
    for (it2 = fijo2.begin(); it2 != fijo2.end(); it2++) {
            c = c + min(tabla2[v][*it2], tabla2[*it2][v]);
        }
    
    return c;
}
bool SolucionExacta :: fijoOriginal(nodo v){
	return v < fijosOriginales2 + fijosOriginales1;
}
bool SolucionExacta :: esViable(){
	if(movilesActuales2 > movilesOriginales2 / 2) return true;
	unsigned inversiones = 0;
	list<nodo>:: iterator other;
	for(list<nodo>:: iterator each = fijo2.begin(); each != fijo2.end(); each ++){
        other = each;
	    other++;
		if(other == fijo2.end())break;
		if(!fijoOriginal(*each) || !fijoOriginal(*other)){
			if(tabla2[*each][*other] > tabla2[*other][*each]){
				inversiones +=1;
			}
		}
	}
	return inversiones <= movilesActuales2;
}

void SolucionExacta :: mejorar() {

    // Esto corresponde al caso base, donde chequeo si obtuve una
    // solución mejor a la previamente máxima, y de ser así la
    // actualizo con el nuevo valor.
    if(mejorDibujo.contarCruces() == 0){
		return;
    }
    if (movil1.empty() && movil2.empty()) {
        if (cruces < mejorDibujo.contarCruces()) {
            if (invertirLados) {
                mejorDibujo = Dibujo(d->grafo(), list<nodo>(fijo2), list<nodo>(fijo1), cruces);
            } else {
                mejorDibujo = Dibujo(d->grafo(), list<nodo>(fijo1), list<nodo>(fijo2), cruces);
            }
        }

    }

    // Entro en este caso cuando ya complete una permutacion
    // de fijo1, y ahora tengo que elegir la mejor permutacion
    // para la particion 2.
    else if (movil1.empty()) {
		minimoPosible2 = minimoPosible2 -minimosFijos[*movil2.begin()];
                agregarAtras2();
		movilesActuales2--;
		list<nodo>::iterator last = fijo2.end();
		last--;
		for(list<nodo>::iterator each = movil2.begin(); each != movil2.end(); each++){
			minimosFijos[*each] += min(tabla2[*each][*last],tabla2[*last][*each]);
		}
        list<nodo>::iterator it = fijo2.end();
        unsigned pos = fijo2.size() - 1;
        while (it != fijo2.begin()) {
            if (it != fijo2.end()) {
                retrasar2(it, pos);
                it--;
                pos--;
            } else {
                it--;
            }
            if (minimoPosible2 + cruces < mejorDibujo.contarCruces()&& esViable()) {
                mejorar();
            }
        }
		minimoPosible2+=minimosFijos[*fijo2.begin()];
		for(list<nodo>::iterator each = movil2.begin(); each != movil2.end(); each++){
			minimosFijos[*each] -= min(tabla2[*each][*fijo2.begin()],tabla2[*fijo2.begin()][*each]);
		}
        sacarPrincipio2();
		movilesActuales2++;
    }

    // Entro en este caso cuando lleno la permutacion de fijo1
    // (siempre se hace esto antes de verificar la otra particion,
    // ya que elegimos fijo1 para que tenga menos permutaciones).
    else {
        agregarAtras1();
        list<nodo>::iterator it = fijo1.end();
        unsigned pos = fijo1.size() - 1;
        while (it != fijo1.begin()) {
            if (it != fijo1.end()) {
                retrasar1(it, pos);
                it--;
                pos--;
            } else {
                it--;
            }
            if (movil1.empty()) {
                tabular2();
				minimoPosible2 = 0;
				for(list<nodo>::iterator each = movil2.begin(); each != movil2.end(); each++){
					minimosMoviles[*each] = minimoDelNodoMoviles(*each);
					minimosFijos[*each] = minimoDelNodoFijos(*each);
					minimoPosible2+=minimosFijos[*each]+minimosMoviles[*each];
				}
            }
            //FIXME: minimosCrucesRestantes1 se puede tener precalculado y actualizarlo cada vez q se saca un nodo
            if (minimosCrucesRestantes1() + cruces < mejorDibujo.contarCruces()) {
                mejorar();
            }
        }

        sacarPrincipio1();
    }
}

void SolucionExacta :: tabular1() {
    list<nodo>::const_iterator it1, it2;
    for (it1 = l1.begin(); it1 != l1.end(); it1++) {
        for (it2 = l1.begin(); it2 != l1.end(); it2++) {
            if (*it1 < *it2) {
                tabla1[*it1][*it2] = crucesEntre(*it1, *it2, fijo2, adyp1, posiciones2);
                tabla1[*it2][*it1] = crucesEntre(*it2, *it1, fijo2, adyp1, posiciones2);
            }
        }
    }
}

void SolucionExacta :: tabular2() {
    list<nodo>::const_iterator it1, it2,it3;
    for (it1 = l2.begin(); it1 != l2.end(); it1++) {
		it3=it1;
		it3++;
        for (it2 = it3; it2 != l2.end(); it2++) {
                tabla2[*it1][*it2] = crucesEntre(*it1, *it2, fijo1, d->grafo()->ejes(), posiciones1);
                tabla2[*it2][*it1] = (d->grafo()->ejes()[*it2]).size()*(d->grafo()->ejes()[*it1]).size()- tabla2[*it1][*it2]-nodosEnComun[*it1][*it2];
        }
    }
}


unsigned SolucionExacta :: minimosCrucesRestantes1() {
    unsigned c = 0;
    list<nodo>::const_iterator it1, it2;
    for (it1 = movil1.begin(); it1 != movil1.end(); it1++) {
        for (it2 = movil1.begin(); it2 != movil1.end(); it2++) {
            if (*it1 < *it2) {
                c = c + min(tabla1[*it1][*it2], tabla1[*it2][*it1]);
            }
        }
        for (it2 = fijo1.begin(); it2 != fijo1.end(); it2++) {
            c = c + min(tabla1[*it1][*it2], tabla1[*it2][*it1]);
        }
    }
    return c;
}


unsigned SolucionExacta :: minimosCrucesRestantes2() {
    unsigned c = 0;
    list<nodo>::const_iterator it1, it2;
    for (it1 = movil2.begin(); it1 != movil2.end(); it1++) {
        for (it2 = movil2.begin(); it2 != movil2.end(); it2++) {
            if (*it1 < *it2) {
                c = c + min(tabla2[*it1][*it2], tabla2[*it2][*it1]);
            }
        }
        for (it2 = fijo2.begin(); it2 != fijo2.end(); it2++) {
            c = c + min(tabla2[*it1][*it2], tabla2[*it2][*it1]);
        }
    }
    return c;
}


void SolucionExacta :: sacarPrincipio1() {
	nodo n = fijo1.front();
    unsigned crucesAux = 0;
	list<nodo> :: iterator it;
	it = fijo1.begin();
	/* cuento cruces usando la tabla */
	while(it != fijo1.end()){
		if(*it != n)crucesAux+=tabla1[n][*it];
		it++;
	}
    cruces = cruces - crucesAux;
    
    fijo1.pop_front();
    movil1.push_back(n);

    // Actualizo las posiciones.
    it = fijo1.begin();
    unsigned i = 0;
    while (it != fijo1.end()) {
        posiciones1[*it] = i;
        it++;
        i++;
    }
	estaEnFijo1[n] = false;
}

void SolucionExacta :: sacarPrincipio2() {
	nodo n = fijo2.front();
	unsigned crucesAux = 0;
	list<nodo> :: iterator it;
	it = fijo2.begin();
	/* cuento cruces usando la tabla */
	while(it != fijo2.end()){
		if(*it != n)crucesAux+=tabla2[n][*it];
		it++;
	}
    cruces -=crucesAux;
    fijo2.pop_front();
    movil2.push_back(n);
}

void SolucionExacta :: agregarAtras1() {
    nodo n = movil1.front();
    movil1.pop_front();
    fijo1.push_back(n);
    posiciones1[n] = fijo1.size() - 1;
	unsigned crucesAux = 0;
	list<nodo> :: iterator it;
	it = fijo1.begin();
	/* cuento cruces usando la tabla */
	while(it != fijo1.end()){
		if(*it != n)crucesAux+=tabla1[*it][n];
		it++;
	}
	cruces+=crucesAux;
	estaEnFijo1[n] = true;
    
}

void SolucionExacta :: agregarAtras2() {
    nodo n = movil2.front();
    movil2.pop_front();
    fijo2.push_back(n);
    unsigned crucesAux = 0;
	list<nodo> :: iterator it;
	it = fijo2.begin();
	/* cuento cruces usando la tabla */
	while(it != fijo2.end()){
		if(*it != n)crucesAux+=tabla2[*it][n];
		it++;
	}
	cruces+=crucesAux;
}


void SolucionExacta :: retrasar1(list<nodo>::iterator& it, unsigned pos) {
    list<nodo>::iterator itAux = it;
    itAux--;

    unsigned cAntes = tabla1[*itAux][*it];
    unsigned cDespues = tabla1[*it][*itAux];

    posiciones1[*itAux] = pos;
    posiciones1[*it] = pos - 1;

    nodo val = *it;
    fijo1.erase(it);
    fijo1.insert(itAux,val);
    it = itAux;

    cruces = cruces - cAntes + cDespues;
}

void SolucionExacta :: retrasar2(list<nodo>::iterator& it, unsigned pos) {
    list<nodo>::iterator itAux = it;
    itAux--;

    unsigned cAntes = tabla2[*itAux][*it];
    unsigned cDespues = tabla2[*it][*itAux];

    nodo val = *it;
    fijo2.erase(it);
    fijo2.insert(itAux,val);
    it = itAux;

    cruces = cruces - cAntes + cDespues;
}

