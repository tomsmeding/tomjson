#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "../tomjson.h"


#define CHECK(cond) CHECKX(cond,cond)

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

int main(void){
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

	OK();
}
