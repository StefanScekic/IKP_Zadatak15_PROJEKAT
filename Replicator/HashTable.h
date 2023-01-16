#pragma once
#include "../Common/Includes.h"
#include "../Common/RequestInterface.h"

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define TABLE_SIZE 10

typedef struct table_node {
	process value;
	struct table_node* next;
} table_node;

/*
	Initializes a fresh hash table
*/
void init_hash_table();

/*
	Creates a new entry in the hash table.

	Returns True if succesfull, False if not	
*/
bool hash_table_insert(process p);

/*
	Searches the hash table for a process with a specified id.

	If successfull returns true and changes passed pointer to the process.

	Othervise returns false.
*/
bool hash_table_lookup(int id, process* p);

/*
	Deletes the process entry if able to.

	Returns:
		True if successfull.

		False if not.

*/
bool hash_table_delete(int id);
#endif // !HASH_TABLE_H

