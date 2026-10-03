#include <stdio.h>
#include <stdlib.h>

#define MAX     20                 /* maximo de posiciones (tabla de 20 pasos) */
#define MIN_VAL -9999              /* rango permitido para los datos           */
#define MAX_VAL  9999
#define ES_FIN(p) ((p) <= 0 || (p) > n)   /* -1, 0 o fuera de rango = fin      */

/* ---- Vectores: version 0 = tabla ACTUAL, version 1 = tabla AUXILIAR ------- */
/* La version 1 es la copia de la tabla ORIGINAL guardada antes de modificar.  */
int D[2][MAX + 1];                 /* dato[]                                   */
int E[2][MAX + 1];                 /* sig[] (enlace)                           */
int V[2][4];                       /* [0]=NUEVO [1]=SW2 [2]=EDO [3]=DISP       */
int n = 0;                         /* tamano de los vectores (lo pide al user) */

/* Nombres de los codigos de clase (siempre trabajan con la version actual)    */
#define dato   D[0]
#define sig    E[0]
#define nuevo  V[0][0]
#define SW2    V[0][1]
#define EDO    V[0][2]
#define disp   V[0][3]             /* "libre" en el codigo de clase            */

int inicio = -1;                   /* inicio de la lista en uso (SW2 o EDO)    */
int datosCargados = 0, hayBase = 0;

/* ============================ UTILIDADES ================================== */
void limpiarPantalla()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void esperarTecla()
{
    char b[100];
    printf("\n  Presione ENTER para continuar...");
    fgets(b, sizeof(b), stdin);
}

/* Lee un entero validado en [min,max] sin punteros (fgets + conversion manual) */
int leerEntero(char msg[], int min, int max)
{
    char b[100];
    int i, s, v, d;
    while (1)
    {
        printf("%s", msg);
        if (fgets(b, sizeof(b), stdin) == NULL) exit(0);
        i = 0; s = 1; v = 0; d = 0;
        while (b[i] == ' ') i++;
        if (b[i] == '-') { s = -1; i++; }
        while (b[i] >= '0' && b[i] <= '9') { if (v < 100000) v = v * 10 + b[i] - '0'; d = 1; i++; }
        while (b[i] == ' ') i++;
        if (d && (b[i] == '\n' || b[i] == '\r' || b[i] == '\0'))
        {
            v *= s;
            if (v >= min && v <= max) return v;
            printf("  Fuera de rango (%d a %d).\n", min, max);
        }
        else printf("  Entrada invalida. Escriba un numero entero.\n");
    }
}

void imprimirNombre(int lista)     /* 1 = Software II, 2 = EDO */
{
    printf("%s", lista == 1 ? "Software II (SW2)" : "Estructura de Datos (EDO)");
}

/* Trabajar con una lista: "inicio" toma el valor de SW2 o EDO y luego se guarda */
void usarLista(int lista)     { inicio = (lista == 1) ? SW2 : EDO; }
void guardarLista(int lista)  { if (lista == 1) SW2 = inicio; else EDO = inicio; }

/* ======================= FORMATO TABULAR Y COMPARATIVA ===================== */

/* Imprime la tabla Paso | Dato( ) | Enlace [ ] (20 pasos) de la version v
   (0 = actual, 1 = original). Si comparar = 1 marca con "<<" las filas que
   cambiaron respecto a la tabla original. */
void mostrarTabla(int v, int comparar)
{
    int i, l, p, k, est[MAX + 1];
    char t1[12], t2[12];

    /* A que lista pertenece cada posicion: 1 = SW2, 2 = EDO, 3 = DISP */
    for (i = 1; i <= MAX; i++) est[i] = 0;
    for (l = 1; l <= 3; l++)
        for (p = V[v][l], k = 0; !ES_FIN(p) && k < n; k++, p = E[v][p])
            est[p] = l;

    printf("\n  %s\n", v == 1 ? "TABLA ORIGINAL (ANTES)" :
                        comparar ? "TABLA MODIFICADA (DESPUES)" : "TABLA ACTUAL");
    printf("  +------+------------+------------+\n");
    printf("  | Paso |  Dato( )   | Enlace [ ] |\n");
    printf("  +------+------------+------------+\n");
    for (i = 1; i <= MAX; i++)
    {
        t1[0] = t2[0] = '\0';                      /* celdas en blanco por defecto */
        if (i <= n)
        {
            sprintf(t2, "%d", E[v][i]);
            if (est[i] != 3) sprintf(t1, "%d", D[v][i]);   /* celda libre: sin dato */
        }
        printf("  | %4d | %10s | %10s | %s\n", i, t1, t2,
               (comparar && i <= n && (D[0][i] != D[1][i] || E[0][i] != E[1][i])) ? "<<" : "");
    }
    printf("  +------+------------+------------+\n");
    printf("  N = %d | SW2 = %d | EDO = %d | DISP = %d | NUEVO = %d\n",
           n, V[v][1], V[v][2], V[v][3], V[v][0]);
}

/* Comparativa: 2 tablas separadas, la ORIGINAL y la MODIFICADA */
void mostrarComparativa()
{
    if (!hayBase) { printf("\n  Aun no hay tabla original (realice una insercion).\n"); return; }
    mostrarTabla(1, 0);                /* tabla original (copia auxiliar) */
    printf("\n  ----------------------------------------\n");
    mostrarTabla(0, 1);                /* tabla modificada (actual)       */
}

/* Variable auxiliar: guarda la tabla actual ANTES de modificarla */
void guardarBase()
{
    int i;
    for (i = 1; i <= n; i++) { D[1][i] = D[0][i]; E[1][i] = E[0][i]; }
    for (i = 0; i < 4; i++) V[1][i] = V[0][i];
    hayBase = 1;
}

/* ================== CODIGOS DE CLASE (adaptados) ========================== */

/* Crear nodo (clase). Modificado: en vez de "libre++" se extrae el primer nodo
   de la lista de disponibles: NUEVO = DISP ; DISP = SIG[DISP]. */
int crearNodo(int valor)
{
    int posicion;

    if (disp <= 0 || disp > n)                     /* Paso 1: Overflow */
    {
        printf("No hay espacio disponible (Overflow). DISP = %d\n", disp);
        return -1;
    }

    posicion = disp;                               /* NUEVO = DISP          */
    disp = sig[posicion];                          /* DISP = ENLACE[DISP]   */
    dato[posicion] = valor;                        /* DATO[NUEVO] = ITEM    */
    sig[posicion] = -1;                            /* ENLACE[NUEVO] = NULL  */
    return posicion;
}

/* Insertar al inicio (clase: pasos de insertarInicio) */
void insertarInicio(int valor)
{
    if (disp <= 0 || disp > n)
    {
        printf("Error: Lista llena. No se puede insertar %d.\n", valor);
        return;
    }
    nuevo = disp;                                  /* nuevoNodo = disponible */
    disp = sig[nuevo];                             /* disponible = enlace[nuevoNodo] */
    dato[nuevo] = valor;                           /* dato[nuevoNodo] = valor */
    sig[nuevo] = inicio;                           /* enlace[nuevoNodo] = cabeza */
    inicio = nuevo;                                /* cabeza = nuevoNodo */
}

/* Insertar al final (clase) */
void insertarFinal(int valor)
{
    int aux;

    nuevo = crearNodo(valor);
    if (nuevo == -1)
        return;

    if (ES_FIN(inicio))                            /* Escenario 1: lista vacia */
    {
        inicio = nuevo;
    }
    else                                           /* Escenario 2: recorrido   */
    {
        aux = inicio;
        while (!ES_FIN(sig[aux]))                  /* clase: sig[aux] != -1 */
        {
            aux = sig[aux];
        }
        sig[aux] = nuevo;
    }
}

/* Buscar elemento (clase): devuelve la posicion o -1 si no existe */
int buscar(int valor)
{
    int aux;

    aux = inicio;
    while (!ES_FIN(aux))                           /* clase: aux != -1 */
    {
        if (dato[aux] == valor)
            return aux;
        aux = sig[aux];
    }
    return -1;
}

/* Insertar antes del nodo con valor de referencia (clase) */
void insertarAntes(int valor, int referencia)
{
    int actual, anterior;

    if (ES_FIN(inicio))
    {
        printf("La lista esta vacia.\n");
        return;
    }

    actual = buscar(referencia);
    if (actual == -1)
    {
        printf("Nodo de referencia no encontrado.\n");
        return;
    }

    nuevo = crearNodo(valor);
    if (nuevo == -1)
        return;

    if (actual == inicio)                          /* Si la referencia esta al inicio */
    {
        sig[nuevo] = inicio;
        inicio = nuevo;
    }
    else
    {
        anterior = inicio;
        while (sig[anterior] != actual)
        {
            anterior = sig[anterior];
        }
        sig[nuevo] = actual;
        sig[anterior] = nuevo;
    }
    printf("Elemento %d insertado antes de %d.\n", valor, referencia);
}

/* Insertar despues del nodo con info X (los 7 pasos de la diapositiva) */
void insertarDespues(int valor, int referencia)
{
    int ptr, loc = 0;

    /* 1. Hay memoria disponible? */
    if (disp <= 0 || disp > n)
    {
        printf("Error: Desbordamiento de Memoria (Overflow).\n");
        return;
    }
    /* 2. Buscar la posicion del nodo con X: PTR = INICIO y LOC = 0 */
    ptr = inicio;
    while (!ES_FIN(ptr))
    {
        if (dato[ptr] == referencia) { loc = ptr; break; }
        ptr = sig[ptr];
    }
    /* 3. Verificar si X existe en la lista */
    if (loc == 0)
    {
        printf("%d no se encuentra en la lista. No se puede insertar.\n", referencia);
        return;
    }
    /* 4. Extraer el primer nodo libre y 5. escribir la informacion */
    nuevo = disp;                                  /* NUEVO = DISP */
    disp = sig[disp];                              /* DISP = enlace[DISP] */
    dato[nuevo] = valor;                           /* Dato[NUEVO] = ITEM */
    /* 6. Intercambio de enlaces */
    sig[nuevo] = sig[loc];                         /* enlace NUEVO = enlace LOC */
    sig[loc] = nuevo;                              /* enlace LOC = NUEVO */
    printf("Elemento %d insertado despues de %d.\n", valor, referencia);
}

/* Recorrer/mostrar (clase). Se agrega la lista de posiciones y el promedio */
void mostrarLista()
{
    int aux, suma = 0, cant = 0;

    if (ES_FIN(inicio)) { printf("    La lista esta vacia.\n"); return; }

    printf("    Pos : ");
    for (aux = inicio; !ES_FIN(aux); aux = sig[aux]) printf("%d -> ", aux);
    printf("NULL\n    Dato: ");

    aux = inicio;
    while (!ES_FIN(aux))                           /* clase: aux != -1 */
    {
        printf("%d -> ", dato[aux]);
        suma += dato[aux]; cant++;
        aux = sig[aux];
    }
    printf("NULL\n    Elementos: %d | Promedio: %.2f\n", cant, (float)suma / cant);
}

/* Muestra las dos listas y la lista de disponibles */
void mostrarTodo()
{
    int aux;
    printf("\n  Lista "); imprimirNombre(1); printf(" (inicio = %d)\n", SW2);
    usarLista(1); mostrarLista();
    printf("\n  Lista "); imprimirNombre(2); printf(" (inicio = %d)\n", EDO);
    usarLista(2); mostrarLista();
    printf("\n  Nodos disponibles (DISP = %d): ", disp);
    if (ES_FIN(disp)) printf("ninguno");
    for (aux = disp; !ES_FIN(aux); aux = sig[aux]) printf("%d -> ", aux);
    printf("0 (Null)\n");
}

/* ============================ OPCIONES DEL MENU ============================ */

int verificarDatos()
{
    if (!datosCargados) printf("\n  Primero debe cargar los datos (opcion 1).\n");
    return datosCargados;
}

/* Cuenta cuantos nodos recorre una cadena (para detectar ciclos al cargar) */
int cadenaValida(int ini)
{
    int p, k = 0;
    for (p = ini; !ES_FIN(p); p = sig[p]) if (++k > n) return 0;
    return 1;
}

/* Opcion 1: solicitar la data de entrada inicial (todo lo escribe el usuario) */
void opConfigurar()
{
    int i;
    char msg[60];

    limpiarPantalla();
    printf("\n  ===== 1. SOLICITAR DATA INICIAL DE DATO[] Y ENLACE[] =====\n");
    n = leerEntero("\n  Tamano de dato[] y sig[] (1 a 20): ", 1, MAX);
    printf("  (Celda libre: dato 0 y su enlace dentro de la cadena DISP)\n");
    sprintf(msg, "    Enlace[] (-1 a %d): ", n);
    do
    {
        for (i = 1; i <= n; i++)
        {
            printf("\n  Posicion %d\n", i);
            dato[i] = leerEntero("    Dato[]  : ", MIN_VAL, MAX_VAL);
            sig[i]  = leerEntero(msg, -1, n);
        }
        /* Variables dinamicas: inicio de cada lista, disponible y nuevo */
        printf("\n  --- Variables de control (-1 o 0 = lista vacia) ---\n");
        SW2   = leerEntero("  Inicio de Software II (SW2): ", -1, n);
        EDO   = leerEntero("  Inicio de Estructura de Datos (EDO): ", -1, n);
        disp  = leerEntero("  Inicio de nodos disponibles (DISP): ", -1, n);
        nuevo = leerEntero("  Valor inicial de NUEVO (0 = ninguno): ", 0, n);
        if (!cadenaValida(SW2) || !cadenaValida(EDO) || !cadenaValida(disp))
            printf("\n  ERROR: algun enlace forma un ciclo. Ingrese los datos de nuevo.\n");
    } while (!cadenaValida(SW2) || !cadenaValida(EDO) || !cadenaValida(disp));
    datosCargados = 1; hayBase = 0;

    limpiarPantalla();
    printf("\n  ===== TABLA BASE CARGADA =====\n");
    mostrarTabla(0, 0);
    mostrarTodo();
    esperarTecla();
}

/* Opciones 2 a 5: insertar (modo 1 inicio, 2 final, 3 antes de X, 4 despues de X) */
void opInsertar(int modo)
{
    int lista, valor, ref = 0;

    limpiarPantalla();
    printf("\n  ===== %s =====\n", modo == 1 ? "2. INSERTAR AL INICIO" :
                                   modo == 2 ? "3. INSERTAR AL FINAL" :
                                   modo == 3 ? "4. INSERTAR ANTES DE UN DATO X" :
                                               "5. INSERTAR DESPUES DE UN DATO X");
    if (!verificarDatos()) { esperarTecla(); return; }

    mostrarTabla(0, 0);
    printf("\n  Lista:  1) Software II (SW2)   2) Estructura de Datos (EDO)\n");
    lista = leerEntero("  Seleccione la lista: ", 1, 2);
    valor = leerEntero("  Valor a insertar: ", MIN_VAL, MAX_VAL);
    if (modo >= 3) ref = leerEntero("  Dato de referencia (X): ", MIN_VAL, MAX_VAL);

    guardarBase();                                 /* foto de la tabla original */
    usarLista(lista);                              /* inicio = SW2 o EDO        */
    printf("\n");
    if      (modo == 1) insertarInicio(valor);
    else if (modo == 2) insertarFinal(valor);
    else if (modo == 3) insertarAntes(valor, ref);
    else                insertarDespues(valor, ref);
    guardarLista(lista);                           /* SW2 o EDO = inicio        */

    mostrarComparativa();
    printf("\n  Lista resultante: "); imprimirNombre(lista); printf("\n");
    usarLista(lista); mostrarLista();
    esperarTecla();
}

/* Opcion 6: recorrer la lista en formato tabular con sus variables */
void opRecorrer()
{
    limpiarPantalla();
    printf("\n  ===== 6. RECORRER LA LISTA (FORMATO TABULAR) =====\n");
    if (verificarDatos())
    {
        mostrarTabla(0, 0);
        mostrarTodo();
        if (hayBase) { printf("\n  --- Comparativa de la ultima insercion ---\n"); mostrarComparativa(); }
    }
    esperarTecla();
}

/* Opciones 7 y 8: buscar un valor (se sugiere 100 en SW2 y 25 en EDO) */
void opBuscar(int lista, int sugerido)
{
    int valor, pos;
    char msg[60];

    limpiarPantalla();
    printf("\n  ===== BUSCAR EN "); imprimirNombre(lista); printf(" =====\n");
    if (!verificarDatos()) { esperarTecla(); return; }

    mostrarTabla(0, 0);
    sprintf(msg, "\n  Valor a buscar (sugerido: %d): ", sugerido);
    valor = leerEntero(msg, MIN_VAL, MAX_VAL);
    usarLista(lista);
    pos = buscar(valor);

    if (pos != -1) printf("\n  Resultado: el valor %d SI EXISTE (posicion %d).\n", valor, pos);
    else           printf("\n  Resultado: el valor %d NO EXISTE en la lista.\n", valor);
    mostrarLista();
    esperarTecla();
}

/* ================================ MENU ==================================== */
int main()
{
    int op;
    do
    {
        limpiarPantalla();
        printf("\n  ==============================================================\n");
        printf("     UTP - Estructura de Datos (1261) - Listas Simples Enlazadas\n");
        printf("  ==============================================================\n");
        if (datosCargados) printf("   Estado: N=%d | SW2=%d | EDO=%d | DISP=%d\n", n, SW2, EDO, disp);
        else               printf("   Estado: sin datos cargados (use la opcion 1)\n");
        printf("  --------------------------------------------------------------\n");
        printf("   1. Solicitar data inicial de DATO[] y ENLACE[]\n");
        printf("   2. Insertar al inicio\n");
        printf("   3. Insertar al final\n");
        printf("   4. Insertar antes de un dato X\n");
        printf("   5. Insertar despues de un dato X\n");
        printf("   6. Recorrer la lista (formato tabular)\n");
        printf("   7. Buscar un 100 en Software II\n");
        printf("   8. Buscar un 25 en Estructura de Datos\n");
        printf("   0. Salir\n");
        printf("  --------------------------------------------------------------\n");
        op = leerEntero("  Seleccione una opcion: ", 0, 8);

        if (op == 1)                   opConfigurar();
        else if (op >= 2 && op <= 5)   opInsertar(op - 1);   /* 1 inicio, 2 final, 3 antes, 4 despues */
        else if (op == 6)              opRecorrer();
        else if (op == 7)              opBuscar(1, 100);     /* Software II          */
        else if (op == 8)              opBuscar(2, 25);      /* Estructura de Datos  */
    } while (op != 0);
    limpiarPantalla();
    printf("\n  Programa finalizado. Hasta pronto!\n\n");
    return 0;
}