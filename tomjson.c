#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "tomjson.h"


static Jsonnode* json_parse_endp(const char *str,const char **endp);

#ifdef DEBUG
# define DBG(...) fprintf(stderr, __VA_ARGS__)
#else
# define DBG(...)
#endif

#define SKIPSPACES(str) \
		do { \
			while(*(str)&&isspace(*(str)))(str)++; \
		} while(0)

#define SKIPSPACESRETNULL(str) \
		do { \
			while(*(str)&&isspace(*(str)))(str)++; \
			if(!*(str))return NULL; \
		} while(0)

static Jsonnode* parsenumber(const char *str,const char **endp){
	SKIPSPACESRETNULL(str);
	double r=strtod(str,(char**)endp);
	if(*endp==str)return NULL;
	Jsonnode *node=malloc(sizeof(Jsonnode));
	assert(node);
	node->type=JSON_NUMBER;
	node->numval=r;
	return node;
}

static bool ishex(char c){
	return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');
}

static char hexdigit(int n){
	assert(n>=0&&n<16);
	if(n<10)return '0'+n;
	return 'a'+n-10;
}

static char readunicodehex(const char *str,int length){
	unsigned char acc=0;
	for(int i=0;i<length;i++){
		unsigned char inc;
		if(str[i]>='0'&&str[i]<='9')inc=str[i]-'0';
		else if(str[i]>='a'&&str[i]<='f')inc=str[i]-'a'+10;
		else if(str[i]>='A'&&str[i]<='F')inc=str[i]-'A'+10;
		else assert(false);
		acc=16*acc+inc;
	}
	return (char)acc;
}

static Jsonnode* parsestring(const char *str,const char **endp){
	SKIPSPACESRETNULL(str);
	if(str[0]!='"')return NULL;
	int i,len=0;
	for(i=1;str[i]&&str[i]!='"';i++){
		len++;
		if(str[i]!='\\')continue;
		i++;
		if(str[i]=='u'){
			if(!ishex(str[i+1])||
			   !ishex(str[i+2])||
			   !ishex(str[i+3])||
			   !ishex(str[i+4])){
				return NULL;
			}
			i+=4;
		}
	}
	if(str[i]=='\0')return NULL;
	*endp=str+i+1;
	char *buf=malloc(len+1);
	assert(buf);
	int j;
	for(i=1,j=0;str[i]!='"';i++){
		if(str[i]=='\\'){
			i++;
			switch(str[i]){
				case 'b': buf[j++]='\b'; break;
				case 'f': buf[j++]='\f'; break;
				case 'n': buf[j++]='\n'; break;
				case 'r': buf[j++]='\r'; break;
				case 't': buf[j++]='\t'; break;
				case 'u':
					buf[j++]=readunicodehex(str+i+1,4);
					i+=4;
					break;
				default:
					buf[j++]=str[i];
					break;
			}
		} else {
			buf[j++]=str[i];
		}
	}
	buf[len]='\0';

	Jsonnode *node=malloc(sizeof(Jsonnode));
	assert(node);
	node->type=JSON_STRING;
	node->strval=buf;
	return node;
}

static Jsonnode* parsebool(const char *str,const char **endp){
	SKIPSPACESRETNULL(str);
	if(memcmp(str,"true",4)==0||memcmp(str,"false",5)==0){
		Jsonnode *node=malloc(sizeof(Jsonnode));
		assert(node);
		node->type=JSON_BOOL;
		node->boolval=str[0]=='t';
		*endp=str+4+(str[0]!='t');
		return node;
	}
	return NULL;
}

static Jsonnode* parsenull(const char *str,const char **endp){
	SKIPSPACESRETNULL(str);
	if(memcmp(str,"null",4)==0){
		Jsonnode *node=malloc(sizeof(Jsonnode));
		assert(node);
		node->type=JSON_NULL;
		*endp=str+4;
		return node;
	}
	return NULL;
}

static Jsonnode* parsearray(const char *str,const char **endp){
#define FAILRETURN \
		do { \
			for(int i=0;i<length;i++)json_free(elems[i]); \
			free(elems); \
			return NULL; \
		} while(0)

	SKIPSPACESRETNULL(str);
	if(str[0]!='[')return NULL;
	const char *cursor=str+1;
	SKIPSPACES(cursor);
	if(*cursor==']'){
		Jsonnode *node=malloc(sizeof(Jsonnode));
		assert(node);
		node->type=JSON_ARRAY;
		node->arrval.capacity=1;
		node->arrval.length=0;
		node->arrval.elems=malloc(1);
		assert(node->arrval.elems);
		*endp=cursor+1;
		return node;
	}

	int sz=16,length=0;
	Jsonnode **elems=malloc(sz*sizeof(Jsonnode*));
	assert(elems);
	//fprintf(stderr,"ARR: start\n");
	while(*cursor){
		const char *elemend;
		Jsonnode *elem=json_parse_endp(cursor,&elemend);
		//fprintf(stderr,"ARR: elem %p, parsing from %s\n",elem,cursor);
		if(!elem)FAILRETURN;
		if(length==sz){
			sz*=2;
			elems=realloc(elems,sz*sizeof(Jsonnode*));
			assert(elems);
		}
		elems[length++]=elem;
		cursor=elemend;
		SKIPSPACES(cursor);
		if(*cursor==']')break;
		if(*cursor!=',')FAILRETURN;
		cursor++;
	}
	if(*cursor!=']')return NULL;

	Jsonnode *node=malloc(sizeof(Jsonnode));
	assert(node);
	node->type=JSON_ARRAY;
	node->arrval.length=length;
	node->arrval.capacity=sz;
	node->arrval.elems=elems;
	*endp=cursor+1;
	return node;

#undef FAILRETURN
}

static Jsonnode* parseobject(const char *str,const char **endp){
	SKIPSPACESRETNULL(str);
	if(*str!='{')return NULL;
	str++;
	SKIPSPACESRETNULL(str);
	if(*str=='}'){
		Jsonnode *node=malloc(sizeof(Jsonnode));
		assert(node);
		node->type=JSON_OBJECT;
		node->objval.capacity=1;
		node->objval.numkeys=0;
		node->objval.keys=malloc(1);
		assert(node->objval.keys);
		node->objval.values=malloc(1);
		assert(node->objval.values);
		*endp=str+1;
		return node;
	}
	int sz=16,numkeys=0;
	char **keys=malloc(sz*sizeof(char*));
	assert(keys);
	Jsonnode **values=malloc(sz*sizeof(Jsonnode*));
	assert(values);

#define FAILRETURN \
		do { \
			for(int i=0;i<numkeys;i++)free(keys[i]); \
			for(int i=0;i<numkeys;i++)json_free(values[i]); \
			free(keys); free(values); \
			return NULL; \
		} while(0)

	const char *cursor=str;
	while(*cursor){
		const char *keyend,*valend;
		Jsonnode *keynode=parsestring(cursor,&keyend);
		if(!keynode)FAILRETURN;
		SKIPSPACES(keyend);
		if(*keyend!=':'){
			json_free(keynode);
			FAILRETURN;
		}
		cursor=keyend+1;
		SKIPSPACES(cursor);
		Jsonnode *valnode=json_parse_endp(cursor,&valend);
		if(!valnode){
			json_free(keynode);
			FAILRETURN;
		}
		if(numkeys==sz){
			sz*=2;
			keys=realloc(keys,sz*sizeof(char*));
			assert(keys);
			values=realloc(values,sz*sizeof(Jsonnode*));
			assert(values);
		}
		keys[numkeys]=keynode->strval;
		free(keynode);
		values[numkeys]=valnode;
		numkeys++;
		cursor=valend;
		SKIPSPACES(cursor);
		if(*cursor=='}')break;
		if(*cursor!=',')FAILRETURN;
		cursor++;
	}
	if(*cursor!='}')return NULL;

	Jsonnode *node=malloc(sizeof(Jsonnode));
	assert(node);
	node->type=JSON_OBJECT;
	node->objval.numkeys=numkeys;
	node->objval.capacity=sz;
	node->objval.keys=keys;
	node->objval.values=values;
	*endp=cursor+1;
	return node;

#undef FAILRETURN
}


static Jsonnode* json_parse_endp(const char *str,const char **endp){
	Jsonnode *node;
	node=parsenumber(str,endp); if(node){SKIPSPACES(*endp); return node;}
	node=parsestring(str,endp); if(node){SKIPSPACES(*endp); return node;}
	node=parsebool  (str,endp); if(node){SKIPSPACES(*endp); return node;}
	node=parsenull  (str,endp); if(node){SKIPSPACES(*endp); return node;}
	node=parsearray (str,endp); if(node){SKIPSPACES(*endp); return node;}
	node=parseobject(str,endp); if(node){SKIPSPACES(*endp); return node;}
	return NULL;
}

Jsonnode* json_parse(const char *str,int length){
	const char *endp;
	Jsonnode *node=json_parse_endp(str,&endp);
	if(!node)return NULL;
	if(endp-str==length)return node;
	json_free(node);
	return NULL;
}


void json_free(Jsonnode *node){
	assert(node);
	switch(node->type){
		case JSON_NUMBER:
			break;

		case JSON_STRING:
			assert(node->strval);
			free(node->strval);
			break;

		case JSON_BOOL:
			break;

		case JSON_NULL:
			break;

		case JSON_ARRAY:
			assert(node->arrval.length>=0);
			assert(node->arrval.elems);
			for(int i=0;i<node->arrval.length;i++){
				json_free(node->arrval.elems[i]);
			}
			free(node->arrval.elems);
			break;

		case JSON_OBJECT:
			assert(node->objval.numkeys>=0);
			assert(node->objval.keys);
			assert(node->objval.values);
			for(int i=0;i<node->objval.numkeys;i++){
				json_free(node->objval.values[i]);
			}
			free(node->objval.keys);
			free(node->objval.values);
			break;

		default:
			assert(false);
	}
	free(node);
}


static void bufextend(char **bufp,int *szp,int *lenp,int targetlen){
	if(*lenp+targetlen<*szp)return;
	*szp+=*szp/2+targetlen;
	*bufp=realloc(*bufp,*szp);
	assert(*bufp);
}

static void bufappend(char **bufp,int *szp,int *lenp,const char *str,int n){
	bufextend(bufp,szp,lenp,*lenp+n);
	memcpy(*bufp+*lenp,str,n+1);
	*lenp+=n;
}

static void stringrepr_inplace(char **bufp,int *szp,int *lenp,const char *str){
	int reslen=2;
	for(int i=0;str[i];i++){
		if(strchr("\"\\/\b\f\n\r\t",str[i])!=NULL)reslen+=2;
		else if(str[i]>=32&&str[i]<=126)reslen++;
		else reslen+=6;
	}
	bufextend(bufp,szp,lenp,*lenp+reslen);

	char *buf=*bufp+*lenp;
	*buf++='"';
	for(int i=0;str[i];i++){
		switch(str[i]){
			case '"': *buf++='\\'; *buf++='"'; break;
			case '\\': *buf++='\\'; *buf++='\\'; break;
			case '\b': *buf++='\\'; *buf++='b'; break;
			case '\f': *buf++='\\'; *buf++='f'; break;
			case '\n': *buf++='\\'; *buf++='n'; break;
			case '\r': *buf++='\\'; *buf++='r'; break;
			case '\t': *buf++='\\'; *buf++='t'; break;
			case '/': *buf++='\\'; *buf++='/'; break;
			default:
				if(str[i]>=32&&str[i]<=126){
					*buf++=str[i];
				} else {
					*buf++='\\';
					*buf++='u';
					*buf++='0';
					*buf++='0';
					*buf++=hexdigit((unsigned int)(unsigned char)str[i]/16);
					*buf++=hexdigit((unsigned int)(unsigned char)str[i]%16);
				}
				break;
		}
	}
	*buf++='"';
	*buf='\0';

	*lenp+=reslen;
}

static void json_stringify_inplace(const Jsonnode *node,char **bufp,int *szp,int *lenp){
	static char tempbuf[64];
	int nwr;
	switch(node->type){
		case JSON_NUMBER:
			nwr=snprintf(tempbuf,sizeof(tempbuf),"%g",node->numval);
			bufappend(bufp,szp,lenp,tempbuf,nwr);
			break;

		case JSON_STRING:
			stringrepr_inplace(bufp,szp,lenp,node->strval);
			break;

		case JSON_BOOL:
			bufappend(bufp,szp,lenp,node->boolval?"true":"false",4+!node->boolval);
			break;

		case JSON_NULL:
			bufappend(bufp,szp,lenp,"null",4);
			break;

		case JSON_ARRAY:
			bufappend(bufp,szp,lenp,"[",1);
			for(int i=0;i<node->arrval.length;i++){
				if(i!=0)bufappend(bufp,szp,lenp,",",1);
				json_stringify_inplace(node->arrval.elems[i],bufp,szp,lenp);
			}
			bufappend(bufp,szp,lenp,"]",1);
			break;

		case JSON_OBJECT:
			bufappend(bufp,szp,lenp,"{",1);
			for(int i=0;i<node->objval.numkeys;i++){
				if(i!=0)bufappend(bufp,szp,lenp,",",1);
				stringrepr_inplace(bufp,szp,lenp,node->objval.keys[i]);
				bufappend(bufp,szp,lenp,":",1);
				json_stringify_inplace(node->objval.values[i],bufp,szp,lenp);
			}
			bufappend(bufp,szp,lenp,"}",1);
			break;
	}
}

char* json_stringify(const Jsonnode *node){
	int sz=128,len=0;
	char *buf=malloc(sz);
	assert(buf);
	buf[0]='\0';
	json_stringify_inplace(node,&buf,&sz,&len);
	return buf;
}


bool json_equal(const Jsonnode *a,const Jsonnode *b){
	assert(a&&b);
	if(a->type!=b->type)return false;
	switch(a->type){
		case JSON_NUMBER:
			return a->numval==b->numval;

		case JSON_STRING:
			return strcmp(a->strval,b->strval)==0;

		case JSON_BOOL:
			return a->boolval==b->boolval;

		case JSON_NULL:
			return true;

		case JSON_ARRAY:
			if(a->arrval.length!=b->arrval.length)return false;
			for(int i=0;i<a->arrval.length;i++){
				if(!json_equal(a->arrval.elems[i],b->arrval.elems[i]))return false;
			}
			return true;

		case JSON_OBJECT:
			if(a->objval.numkeys!=b->objval.numkeys)return false;
			for(int i=0;i<a->objval.numkeys;i++){
				int j;
				for(j=0;j<a->objval.numkeys;j++){
					if(strcmp(a->objval.keys[i],b->objval.keys[j])==0)break;
				}
				if(j==a->objval.numkeys)return false;
				if(!json_equal(a->objval.values[i],b->objval.values[j]))return false;
			}
			return true;

		default:
			assert(false);
	}
}


static char* copyofstring(const char *str){
	assert(str);
	int len=strlen(str);
	char *buf=malloc(len+1);
	assert(buf);
	memcpy(buf,str,len+1);
	return buf;
}

Jsonnode* json_copy(const Jsonnode *node){
	assert(node);
	Jsonnode *dst=malloc(sizeof(Jsonnode));
	assert(dst);
	dst->type=node->type;
	switch(node->type){
		case JSON_NUMBER:
			dst->numval=node->numval;
			break;

		case JSON_STRING:
			dst->strval=copyofstring(node->strval);
			break;

		case JSON_BOOL:
			dst->boolval=node->boolval;
			break;

		case JSON_NULL:
			break;

		case JSON_ARRAY:{
			int len=node->arrval.length;
			size_t cap=node->arrval.capacity;
			dst->arrval.capacity=cap;
			dst->arrval.length=len;
			dst->arrval.elems=malloc(cap*sizeof(Jsonnode*));
			assert(dst->arrval.elems);
			for(int i=0;i<len;i++){
				dst->arrval.elems[i]=json_copy(node->arrval.elems[i]);
			}
			break;
		}

		case JSON_OBJECT:{
			int nk=node->objval.numkeys;
			size_t cap=node->objval.capacity;
			dst->objval.capacity=cap;
			dst->objval.numkeys=nk;
			dst->objval.keys=malloc(cap*sizeof(char*));
			assert(dst->objval.keys);
			dst->objval.values=malloc(cap*sizeof(Jsonnode*));
			assert(dst->objval.values);
			for(int i=0;i<nk;i++){
				dst->objval.keys[i]=copyofstring(node->objval.keys[i]);
				dst->objval.values[i]=json_copy(node->objval.values[i]);
			}
			break;
		}
	}
	return dst;
}


Jsonnode *json_make_num(double val) {
	Jsonnode *node = malloc(sizeof(Jsonnode));
	assert(node);
	node->type = JSON_NUMBER;
	node->numval = val;
	return node;
}

Jsonnode *json_make_str(char *val) {
	Jsonnode *node = malloc(sizeof(Jsonnode));
	assert(node);
	node->type = JSON_STRING;
	node->strval = copyofstring(val);
	return node;
}

Jsonnode *json_make_bool(bool val) {
	Jsonnode *node = malloc(sizeof(Jsonnode));
	assert(node);
	node->type = JSON_BOOL;
	node->boolval = val;
	return node;
}

Jsonnode *json_make_null(void) {
	Jsonnode *node = malloc(sizeof(Jsonnode));
	assert(node);
	node->type = JSON_NULL;
	return node;
}

Jsonnode *json_make_object(size_t capacity) {
	Jsonnode *node = malloc(sizeof(Jsonnode));
	assert(node);

	capacity = capacity > 0 ? capacity : 1;

	node->type = JSON_OBJECT;
	node->objval.numkeys = 0;
	node->objval.capacity = capacity;
	node->objval.keys = malloc(capacity*sizeof(char*));
	assert(node->objval.keys);
	node->objval.values = malloc(capacity*sizeof(Jsonnode*));
	assert(node->objval.values);

	return node;
}

Jsonnode *json_make_array(size_t capacity) {
	Jsonnode *node = malloc(sizeof(Jsonnode));
	assert(node);

	capacity = capacity > 0 ? capacity : 1;

	node->type = JSON_ARRAY;
	node->arrval.length = 0;
	node->arrval.capacity = capacity;
	node->arrval.elems = malloc(capacity*sizeof(Jsonnode*));
	assert(node->arrval.elems);

	return node;
}


static void json_array_ensure_capacity(Jsonarray *arr, size_t capacity) {
	while (arr->capacity < capacity) {
		DBG("resizing arr->capacity to %lu from %lu\n", arr->capacity * 2, arr->capacity);
		arr->capacity *= 2;
	}
	arr->elems = realloc(arr->elems, arr->capacity*sizeof(Jsonnode*));
	assert(arr->elems);
}

void json_array_add_item(Jsonarray *arr, const Jsonnode *item) {
	arr->length++;
	json_array_ensure_capacity(arr, arr->length);
	arr->elems[arr->length - 1] = json_copy(item);
}

void json_array_remove_item(Jsonarray *arr, int index) {
	assert(index >= 0 && index < arr->length);
	memmove(
		arr->elems + index,
		arr->elems + index + 1,
		(arr->length - index - 1) * sizeof(Jsonnode*)
	);
	arr->length--;
}


static void json_object_ensure_capacity(Jsonobject *obj, size_t capacity) {
	while (obj->capacity < capacity) {
		DBG("resizing obj->capacity to %lu from %lu\n", obj->capacity * 2, obj->capacity);
		obj->capacity *= 2;
	}
	obj->keys = realloc(obj->keys, obj->capacity*sizeof(char*));
	assert(obj->keys);
	obj->values = realloc(obj->values, obj->capacity*sizeof(Jsonnode*));
	assert(obj->values);
}

void json_object_add_key(Jsonobject *obj, const char *key, const Jsonnode *val) {
	char *k = copyofstring(key);
	Jsonnode *v = json_copy(val);

	for (int i = 0; i < obj->numkeys; i++) {
		if (strcmp(obj->keys[i], key) == 0) {
			DBG("overwriting previous object key '%s'\n", key);
			free(obj->keys[i]);
			json_free(obj->values[i]);
			obj->keys[i] = k;
			obj->values[i] = v;
			return;
		}
	}

	obj->numkeys++;
	json_object_ensure_capacity(obj, obj->numkeys);
	obj->keys[obj->numkeys - 1] = k;
	obj->values[obj->numkeys - 1] = v;
}

Jsonnode *json_object_get_item(const Jsonobject *obj, const char *key) {
	for (int i = 0; i < obj->numkeys; i++) {
		if (strcmp(obj->keys[i], key) == 0) {
			return json_copy(obj->values[i]);
		}
	}
	return NULL;
}

void json_object_remove_item(Jsonobject *obj, const char *key) {
	int i;
	for (i = 0; i < obj->numkeys; i++) {
		if (strcmp(obj->keys[i], key) == 0) break;
	}
	assert(i != obj->numkeys); // key doesn't exist
	memmove(
		obj->keys + i,
		obj->keys + i + 1,
		(obj->numkeys - i - 1) * sizeof(char*)
	);
	memmove(
		obj->values + i,
		obj->values + i + 1,
		(obj->numkeys - i - 1) * sizeof(Jsonnode*)
	);
	obj->numkeys--;
}
