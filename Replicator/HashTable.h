#pragma once
#include "../Common/Includes.h"
#include "../Common/RequestInterface.h"

#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define TABLE_SIZE 10		//Height of HashTable

typedef struct table_node {
	process value;
	struct table_node* next;
} table_node;

/*
	Initializes a fresh hash table
*/
void init_hash_table();

/*
	Inserts a new entry into the hash table.

	@param p:	Process to be inserted
	@returns	TRUE if insert is successfull, FALSE if not
*/
BOOL hash_table_insert(process p);

/*
	Searches the hash table for a process with a specified id.

	@param id:		ID of process
	@param out p:	structure for process to be writen to if found
	@returns TRUE if process is found, FALSE if not
*/
BOOL hash_table_lookup(int id, process* p);

/*
	Deletes the process entry if it exists.

	@param id: ID of process
	@returns TRUE if successfull, FALSE if not

*/
BOOL hash_table_delete(int id);

/*
	Frees memmory allocated by hash table.
*/
void delete_hash_table();

/*
	Prints the whole HashTable
*/
void print_table();
#endif // !HASH_TABLE_H

