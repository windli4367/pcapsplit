//  Copyright (C) 2010 Lothar Braun <lothar@lobraun.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "list.h"

#include <stdlib.h>

list_t* list_create()
{
	list_t* ret = (list_t*)malloc(sizeof(list_t));

	ret->head = NULL;
	ret->tail = NULL;

	return NULL;
}

inline int list_destroy(list_t* list)
{
	free(list);
	return 0;
}

inline struct list_element_t* list_front(list_t* list)
{
	return list->head;
}

inline struct list_element_t* list_back(list_t* list)
{
	return list->tail;
}

inline int list_push_front(list_t* list, struct list_element_t* element)
{
	if (list->head) {
		struct list_element_t* i = list->head;
		list->head = element;
		element->prev = NULL;
		element->next = i;
		i->prev = element;
	} else {
		list->head = element;
		list->tail = element;
		element->prev = NULL;
		element->next = NULL;
	}
	return 0;
}

inline int list_push_back(list_t* list, struct list_element_t* element)
{
	if (list->tail) {
		struct list_element_t* i = list->tail;
		list->tail = element;
		element->prev = i;
		element->next = NULL;
		i->next = element;
	} else {
		list->head = element;
		list->tail = element;
		element->prev = NULL;
		element->next = NULL;
	}
	return 0;
}

inline struct list_element_t* list_pop_front(list_t* list)
{
	if (!list->head)
		return NULL;

	struct list_element_t* ret = list->head;
	if (ret->next) {
		list->head = ret->next;
		ret->next->prev = NULL;
	} else {
		list->head = list->tail = NULL;
	}
	ret->next = ret->prev = NULL;
	return ret;
}

inline struct list_element_t* list_pop_back(list_t* list)
{
	if (!list->tail)
		return NULL;

	struct list_element_t* ret = list->tail;
	if (ret->prev) {
		list->tail = ret->prev;
		ret->prev->next = NULL;
	} else {
		// poping the last element of the list
		list->head = list->tail = NULL;
	}
	ret->next = ret->prev = NULL;
	return ret;
}

