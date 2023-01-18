#include "HashTable.h"

table_node* hash_table[TABLE_SIZE];

//Private hash function
unsigned int hash(int id) {
	return id % TABLE_SIZE;
}

void init_hash_table() {
	int i = 0;
	for (i = 0; i < TABLE_SIZE; i++) {
		hash_table[i] = NULL;
	}
}

void print_table() {
	int i = 0;
	for (i = 0; i < TABLE_SIZE; i++) {
		printf_s("\n %d : ", i);
		if (hash_table[i] == NULL)
			printf_s(" ----- ");
		else {
			table_node* temp = hash_table[i];

			while (temp != NULL) {
				printf_s(" {%d} ", temp->value.ID);
				temp = temp->next;
			}
		}
	}

	printf_s("\n");
}

bool hash_table_insert(process p) {
	int index = hash(p.ID);

	table_node* new_node = (table_node*)malloc(sizeof(table_node));
	if (new_node == NULL)
		return false;
	new_node->value = p;
	new_node->next = NULL;

	if (hash_table[index] == NULL) {
		hash_table[index] = new_node;
	}
	else {
		table_node* temp = hash_table[index];
		if (temp->value.ID == p.ID)
			return false;

		while (temp->next != NULL) {
			temp = temp->next;
			if (temp->value.ID == p.ID)
				return false;
		}

		temp->next = new_node;
	}

	printf_s("HASH TABLE: insert successfull, id : %d\n", p.ID);
	return true;
}

bool hash_table_lookup(int id, process *p) {
	if (p == NULL)
		return false;

	int index = hash(id);
	table_node* tmp = hash_table[index];

	if (tmp == NULL)
		return false;

	while ((tmp != NULL) && (tmp->value.ID != id)) {
		tmp = tmp->next;
	}

	if (tmp == NULL) {
		return false;
	}
	*p = tmp->value;

	return true;
}

bool hash_table_delete(int id) {
	int index = hash(id);

	table_node* tmp = hash_table[index];
	table_node* prev = NULL;

	while ((tmp != NULL) && (tmp->value.ID != id)) {
		prev = tmp;
		tmp = tmp->next;
	}

	if (tmp == NULL) 
		return false;

	if (prev == NULL) {
		//deleting the head
		hash_table[index] = tmp->next;
	}
	else
	{
		prev->next = tmp->next;
	}

	free(tmp);

	printf_s("HASH TABLE: delete successfull, id : %d\n", id);
	return true;
}