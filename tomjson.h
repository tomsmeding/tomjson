#pragma once

#include <stdbool.h>


typedef enum Jsontype{
	JSON_NUMBER,
	JSON_STRING,
	JSON_BOOL,
	JSON_NULL,
	JSON_ARRAY,
	JSON_OBJECT,
} Jsontype;

struct Jsonnode;
typedef struct Jsonnode Jsonnode;

typedef struct Jsonarray{
	int length;
	Jsonnode **elems;  // array
} Jsonarray;

typedef struct Jsonobject{
	int numkeys;
	char **keys;  // array
	Jsonnode **values;  // array
} Jsonobject;

struct Jsonnode{
	Jsontype type;
	double numval;
	char *strval;
	bool boolval;
	Jsonarray arrval;
	Jsonobject objval;
};


// returns NULL on parse error
Jsonnode* json_parse(const char *str,int length);
void json_free(Jsonnode *node);
char* json_stringify(const Jsonnode *node); // assumes a sane structure
bool json_equal(const Jsonnode *a,const Jsonnode *b); // assumes sane structures
