#ifndef WORKI_H
#define WORKI_H

struct worek;
struct przedmiot;

/*
 * We use a `Container` struct to unify storage logic. Both the global "Desk"
 * and the interior of each "Worek" are treated as Containers. This allows:
 * - Unified management of double-linked lists for items and sub-bags.
 * - O(1) implementation of `na_odwrot` by simply swapping Container pointers
 *   between a specific Worek and the Desk.
 */

// A Container represents a storage location
struct Container {
	// The 'worek' that owns this container. NULL means this container is the desk
	worek *owner;
	// Head of 'przedmiot' list
	przedmiot *first_przedmiot;
	// Head of 'worek' list
	worek *first_worek;
	int total_items;
};

struct przedmiot {
	// Container where this 'przedmiot' currently resides
	Container *loc;
	// Links for the container's list
	przedmiot *prev, *next;
	// 'worek' that directly contains this 'przedmiot', or NULL if on desk
	worek *parent;
};

struct worek {
	// Unique identifier assigned at creation
	int idx;
	// Container where this 'worek' currently resides
	Container *loc;
	// Container holding the contents of this 'worek'
	Container *inner_loc;
	// Links for the container's list
	worek *prev, *next;
	// Cached total number of 'przedmiot's contained in this 'worek' (directly
	// and indirectly)
	int liczba_przedmiotow;
	// Stores the total number of items this bag
	// contributes to its parent Container's count.
	int contribution;
	/*
	 *   When a bag's contents change, we calculate the delta (new total - old
	 *   contribution) and propagate only that difference upward to the parent,
	 *   then update `contribution` to reflect the new total.
	 */

	// Parent 'worek', or NULL if on desk
	worek *parent;
};

// Nowy przedmiot na biurku
przedmiot *nowy_przedmiot();

// Nowy worek na biurku; otrzymuje kolejny numer, począwszy od 0.
worek *nowy_worek();

// Wkłada przedmiot co do worka gdzie.
// Założenie: co i gdzie leżą na biurku.
void wloz(przedmiot *co, worek *gdzie);

// Wkłada worek co do worka gdzie.
// Założenie: co i gdzie leżą na biurku.
void wloz(worek *co, worek *gdzie);

// Wyjmuje przedmiot p z worka i kładzie na biurku.
// Założenie: Przedmiot p był w worku leżącym na biurku.
void wyjmij(przedmiot *p);

// Wyjmuje worek w z worka i kładzie na biurku.
// Założenie: Worek w był w worku leżącym na biurku.
void wyjmij(worek *w);

// Wynik: numer worka, w którym znajduje się przedmiot p (-1 jeśli na biurku).
int w_ktorym_worku(przedmiot *p);

// Wynik: numer worka, w którym znajduje się worek w (-1 jeśli na biurku).
int w_ktorym_worku(worek *w);

// Wynik: liczba przedmiotów zawartych (bezpośrednio i pośrednio) w worku w
int ile_przedmiotow(worek *w);

// Cała zawartość worka w ląduje na biurku, a wszystko, co poza workiem w
// znajdowało się bezpośrednio na biurku, ląduje wewnątrz worka w.
void na_odwrot(worek *w);

// Kończy i zwalnia pamięć
void gotowe();

#endif
