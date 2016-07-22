#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../tomjson.h"


#define STR_(x) #x
#define STR(x) STR_(x)

#define CHECK(cond) CHECKX(cond,#cond)

#define CHECKX(cond,desc) \
		do { \
			bool _c=(cond); \
			if(!_c){ \
				printf("-- FAIL -- : CHECK " desc "\n"); \
				exit(1); \
			} \
		} while(0)

#define OK() printf("-- OK --\n")

#define CHECKJSONX(str,not) \
		do { \
			Jsonnode *_n=json_parse(str,strlen(str)); \
			if(not false) \
				CHECKX(!_n,"ERR: " str); \
			else \
				CHECKX(_n,str); \
			if(_n)json_free(_n); \
		} while(0)

#define CHECKJSON(str) CHECKJSONX(str,)
#define CHECKJSONERR(str) CHECKJSONX(str,!)


#define CHECKBIDIRX(s1,s2,not) \
		do { \
			Jsonnode *n1=json_parse((s1),strlen((s1))); \
			Jsonnode *n2=json_parse((s2),strlen((s2))); \
			CHECKX(not json_equal(n1,n2),STR(not(s1 == s2))); \
			char *s1s=json_stringify(n1); \
			CHECK(not(strcmp(s1s,s2)==0)); \
		} while(0)

#define CHECKBIDIREQ(s1,s2) CHECKBIDIRX(s1,s2,)
#define CHECKBIDIRNEQ(s1,s2) CHECKBIDIRX(s1,s2,!)

#define CHECKBIDIREQSAME(s) CHECKBIDIREQ(s,s)


int main(void){
	//volatile Jsonnode *n=json_parse("\"kaas\"",6);
	//__asm("int3\n\t");

	CHECKJSON("[\"hello\"]");
	CHECKJSON("[ \"hello\" ]");
	CHECKJSON("[ \"hello\", \"world\" ]");
	CHECKJSON("[ 1, 2 ]");
	CHECKJSON("{ \"kaas\": [ \"is\", \"lekker\" ] }");

	CHECKJSONERR("");
	CHECKJSON("123");
	CHECKJSON("21.4e-3");
	CHECKJSONERR("123a");

	CHECKJSON("\"kaas\"");
	CHECKJSON("\"kaa\\u003cs\\\"\"");
	CHECKJSONERR("\"");
	CHECKJSON("\"\\uacef\"");
	CHECKJSONERR("\"\\uaceg\"");
	CHECKJSONERR("\"dingen\\\"");

	CHECKJSON("true");
	CHECKJSON("false");

	CHECKJSON("null");

	CHECKJSON("[]");
	CHECKJSON("[1]");
	CHECKJSON("[\"aaaaaaaaaaaaaaaaaaaaaaaaaaa\",42]");
	CHECKJSON("[1,2,\"hoi\",\"\\\\\"]");
	CHECKJSONERR("[1,2,\"hoi\",\"\\\"][]");
	CHECKJSON("[[1,2],3]");
	CHECKJSON("[[[[[[[[1],1],1],1],[[1],1],[[[[1],1],1],1],1],1],1],1]");
	CHECKJSONERR("[[[[[[[[1],1],1],1],[[1],1],[[[[[1],1],1],1],1],1],1],1]");
	CHECKJSON("[[[[[1],2],[3],4],[[5],6],[7],8],[[[9],0],[1],2],[[3],4],[5],6]");

	CHECKJSON("{}");
	CHECKJSON("{\"a\":[1,2],\"kaas\":null}");
	CHECKJSONERR("{\"obj\":{\"1\":2.\"3\":4},\"x\":\"y\"}");
	CHECKJSON("{\"obj\":{\"1\":2,\"3\":4},\"x\":\"y\"}");
	CHECKJSONERR("{\"obj\":{\"1\":2.\"3\":4},\"x\":y}");


	CHECKBIDIREQSAME("null");
	CHECKBIDIREQSAME("false");
	CHECKBIDIREQSAME("[1,2,3]");
	printf("%s\n",json_stringify(json_parse("\"iets\"",6)));
	CHECKBIDIREQSAME("\"iets\"");
	CHECKBIDIRNEQ("true","false");
	const char *str="{\"a\":\"\\u003c\t\n\fkaas\\\"\",\"iets\":[]}";
	printf("%s\n",json_stringify(json_parse(str,strlen(str))));
	CHECKBIDIREQSAME("{\"a\":\"\\u003c\t\n\fkaas\\\"\",\"iets\":[]}");


	OK();
}
