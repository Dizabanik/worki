#include "worki.h"
#include <cstddef>

// Global variables

static int next_idx = 0;
static Container *desk_container = nullptr;

static void init_if_needed() {
	if (!desk_container) {
		desk_container = new Container{nullptr, nullptr, nullptr, 0};
	}
}

static void unlink_przedmiot(przedmiot *p) {
	if (p->prev)
		p->prev->next = p->next;
	else if (p->loc)
		p->loc->first_przedmiot = p->next;

	if (p->next)
		p->next->prev = p->prev;

	p->prev = nullptr;
	p->next = nullptr;
}

static void unlink_worek(worek *w) {
	if (w->prev)
		w->prev->next = w->next;
	else if (w->loc)
		w->loc->first_worek = w->next;

	if (w->next)
		w->next->prev = w->prev;

	w->prev = nullptr;
	w->next = nullptr;
}

static void link_przedmiot(przedmiot *p, Container *c) {
	p->loc = c;
	p->next = c->first_przedmiot;
	p->prev = nullptr;
	if (c->first_przedmiot)
		c->first_przedmiot->prev = p;
	c->first_przedmiot = p;
}

static void link_worek(worek *w, Container *c) {
	w->loc = c;
	w->next = c->first_worek;
	w->prev = nullptr;
	if (c->first_worek)
		c->first_worek->prev = w;
	c->first_worek = w;
}

// Helper to update a bag's metadata
static void update_bag_counts(worek *w) {
	// Sync legacy int with actual total
	w->liczba_przedmiotow = w->inner_loc->total_items;

	// Calculate the change in this bag's size since last update
	int current_total = w->inner_loc->total_items;
	int diff = current_total - w->contribution;

	// Propagate difference to the parent container
	if (diff != 0) {
		if (w->loc) {
			w->loc->total_items += diff;
		}
		w->contribution = current_total;
	}
}

// Implementation

przedmiot *nowy_przedmiot() {
	init_if_needed();
	przedmiot *p = new przedmiot;
	p->parent = nullptr;

	link_przedmiot(p, desk_container);
	desk_container->total_items++;

	return p;
}

worek *nowy_worek() {
	init_if_needed();
	worek *w = new worek;
	w->idx = next_idx++;
	w->parent = nullptr;
	w->liczba_przedmiotow = 0;
	w->contribution = 0;

	w->inner_loc = new Container{w, nullptr, nullptr, 0};

	link_worek(w, desk_container);

	return w;
}

void wloz(przedmiot *co, worek *gdzie) {
	// Remove from Desk
	unlink_przedmiot(co);
	desk_container->total_items--;

	// Add to Bag
	link_przedmiot(co, gdzie->inner_loc);
	gdzie->inner_loc->total_items++;
	co->parent = gdzie;

	// Update Bag counts and propagate growth to Desk
	update_bag_counts(gdzie);
}

void wloz(worek *co, worek *gdzie) {
	// Remove 'co' from Desk
	unlink_worek(co);
	desk_container->total_items -= co->contribution;

	// Add 'co' to 'gdzie'
	link_worek(co, gdzie->inner_loc);
	gdzie->inner_loc->total_items += co->inner_loc->total_items;
	co->parent = gdzie;

	// 'co' is no longer on Desk, so its 'contribution' field is now relative to
	// 'gdzie'. We sync it so 'co' is valid in its new home.
	co->contribution = co->inner_loc->total_items;

	// Update 'gdzie' counts and propagate growth to Desk
	update_bag_counts(gdzie);
}

void wyjmij(przedmiot *p) {
	worek *old_bag = p->loc->owner;

	// Remove from Bag
	unlink_przedmiot(p);
	p->loc->total_items--;

	// Add to Desk
	link_przedmiot(p, desk_container);
	desk_container->total_items++;
	p->parent = nullptr;

	// Update old Bag
	if (old_bag) {
		update_bag_counts(old_bag);
	}
}

void wyjmij(worek *w) {
	worek *old_bag = w->loc->owner;

	// Remove from Bag
	unlink_worek(w);
	w->loc->total_items -= w->contribution;

	// Add to Desk
	link_worek(w, desk_container);
	// Adding w to desk adds its contents to desk count
	desk_container->total_items += w->inner_loc->total_items;
	w->parent = nullptr;

	// Reset w's contribution logic now that it's on Desk
	w->contribution = w->inner_loc->total_items;

	// Update old Bag
	if (old_bag) {
		update_bag_counts(old_bag);
	}
}

int w_ktorym_worku(przedmiot *p) {
	if (!p->loc || !p->loc->owner)
		return -1;
	return p->loc->owner->idx;
}

int w_ktorym_worku(worek *w) {
	if (!w->loc || !w->loc->owner)
		return -1;
	return w->loc->owner->idx;
}

int ile_przedmiotow(worek *w) { return w->liczba_przedmiotow; }

void na_odwrot(worek *w) {
	// Prepare
	Container *desk = desk_container;
	Container *bag_inner = w->inner_loc;

	// Remove w from Desk so it isn't part of the swap
	unlink_worek(w);
	desk->total_items -= w->contribution;

	// Swap Identies
	desk_container = bag_inner;
	w->inner_loc = desk;

	// Update Owners
	desk_container->owner = nullptr;
	w->inner_loc->owner = w;

	// Put w back on the New Desk
	link_worek(w, desk_container);

	// Recalculate w's contribution to the new desk
	// w now contains the OLD desk items.
	// w is sitting on the NEW desk (which has OLD w items).

	// w's contribution is its size
	w->contribution = w->inner_loc->total_items;
	// New desk total increases by w's size
	desk_container->total_items += w->contribution;

	// Sync legacy count
	w->liczba_przedmiotow = w->inner_loc->total_items;
}

// Cleanup
void free_container(Container *c) {
	if (!c)
		return;
	przedmiot *p = c->first_przedmiot;
	while (p) {
		przedmiot *next = p->next;
		delete p;
		p = next;
	}
	worek *w = c->first_worek;
	while (w) {
		worek *next = w->next;
		free_container(w->inner_loc);
		delete w;
		w = next;
	}
	delete c;
}

void gotowe() {
	if (desk_container) {
		free_container(desk_container);
		desk_container = nullptr;
	}
	next_idx = 0;
}
