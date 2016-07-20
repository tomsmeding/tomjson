#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "tomjson.h"


static Jsonnode* parsenumber(const char *str,const char **endp){
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
	if(str[0]!='"')return NULL;
	int i,len=0;
	for(i=0;str[i]&&str[i]!='"';i++){
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
	for(i=0,j=0;str[i]!='"';i++){
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
	(void)str; (void)endp; assert(false);
}

static Jsonnode* parsenull(const char *str,const char **endp){
	(void)str; (void)endp; assert(false);
}

static Jsonnode* parsearray(const char *str,const char **endp){
	(void)str; (void)endp; assert(false);
}

static Jsonnode* parseobject(const char *str,const char **endp){
	(void)str; (void)endp; assert(false);
}


Jsonnode* json_parse(const char *str,int length){
	Jsonnode *node;
	const char *endp;
#define CHECKXX(type) \
		node=parse##type(str,&endp); \
		if(node){ \
			if(endp-str==length)return node; \
			json_free(node); \
		}
	CHECKXX(number)
	CHECKXX(string)
	CHECKXX(bool)
	CHECKXX(null)
	CHECKXX(array)
	CHECKXX(object)
#undef CHECKXX
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
				json_free(node->arrval.elems+i);
			}
			free(node->arrval.elems);
			break;

		case JSON_OBJECT:
			assert(node->objval.numkeys>=0);
			assert(node->objval.keys);
			assert(node->objval.values);
			for(int i=0;i<node->objval.numkeys;i++){
				json_free(node->objval.values+i);
			}
			free(node->objval.keys);
			free(node->objval.values);
			break;

		default:
			assert(false);
	}
	free(node);
}
