#pragma once

#include <stddef.h>
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

Jsonnode *json_make_num(double val);
Jsonnode *json_make_str(char *val);
Jsonnode *json_make_bool(bool val);
Jsonnode *json_make_null(void);

typedef struct Jsonarray{
	int length;
	size_t capacity;
	Jsonnode **elems;  // array
} Jsonarray;
Jsonnode *json_make_array(size_t capacity);
void json_array_add_item(Jsonarray *arr, const Jsonnode *item);
void json_array_remove_item(Jsonarray *arr, int index);

typedef struct Jsonobject{
	int numkeys;
	size_t capacity;
	char **keys;  // array
	Jsonnode **values;  // array
} Jsonobject;
Jsonnode *json_make_object(size_t capacity);
void json_object_add_key(Jsonobject *obj, const char *key, const Jsonnode *val);
Jsonnode *json_object_get_item(const Jsonobject *obj, const char *key);

struct Jsonnode{
	Jsontype type;
	double numval;
	char *strval;
	bool boolval;
	Jsonarray arrval;
	Jsonobject objval;
};


Jsonnode* json_parse(const char *str,int length); // returns NULL on parse error

// the following functions assume a sane structure
void json_free(Jsonnode *node);
char* json_stringify(const Jsonnode *node);
bool json_equal(const Jsonnode *a,const Jsonnode *b);
Jsonnode* json_copy(const Jsonnode *node);
