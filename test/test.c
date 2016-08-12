#define _GNU_SOURCE //asprintf
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "../tomjson.h"


#define CRED "\x1B[31m"
#define CGREEN "\x1B[32m"
#define CBOLD "\x1B[1m"
#define CNORM "\x1B[0m"

#define COK CGREEN
#define CERROR CRED CBOLD


#define PRINTOK(prefix) \
		if (!quietmode || !ok) { \
			printf("%s%s%s\n" CNORM, ok ? "" : CERROR, prefix, ok ? COK "OK" : "FAIL"); \
		}

#define INCCOUNTS(cond) \
		do { \
			ran++; \
			passed += (ok = (cond)); \
		} while(0)


#define CHECKJSONX(str,not) \
		do { \
			char *printbuf; \
			asprintf(&printbuf, "    %s'%s': ", not false ? "NOT " : "", str); \
			Jsonnode *_n=json_parse(str,strlen(str)); \
			if(not false) \
				INCCOUNTS(!_n); \
			else \
				INCCOUNTS(_n); \
			if(_n)json_free(_n); \
			PRINTOK(printbuf); \
			free(printbuf); \
		} while(0)

#define CHECKJSON(str) CHECKJSONX(str,)
#define CHECKJSONERR(str) CHECKJSONX(str,!)


#define CHECKBIDIRX(s1,s2,not) \
		do { \
			char *printbuf; \
			asprintf(&printbuf, "    '%s' %c= '%s': ", s1, not false ? '!' : '=', s2); \
			Jsonnode *n1=json_parse((s1),strlen((s1))); \
			Jsonnode *n2=json_parse((s2),strlen((s2))); \
			INCCOUNTS(not json_equal(n1,n2)); \
			char *s1s=json_stringify(n1); \
			INCCOUNTS(not(strcmp(s1s,s2)==0)); \
			PRINTOK(printbuf); \
			free(printbuf); \
		} while(0)

#define CHECKBIDIREQ(s1,s2) CHECKBIDIRX(s1,s2,)
#define CHECKBIDIRNEQ(s1,s2) CHECKBIDIRX(s1,s2,!)

#define CHECKBIDIREQSAME(s) CHECKBIDIREQ(s,s)


#define CHECKBIDIRXNODE(n1,n2,not) \
		do { \
			char *printbuf; \
			asprintf(&printbuf, "    '%s' %c~ '%s': ", #n1, not false ? '!' : '~', #n2); \
			char *s1=json_stringify((n1)); \
			char *s2=json_stringify((n2)); \
			INCCOUNTS(not(strcmp(s1,s2)==0)); \
			Jsonnode *n1n=json_parse(s1,strlen(s1)); \
			INCCOUNTS(not json_equal(n1n,n2)); \
			PRINTOK(printbuf); \
			free(printbuf); \
		} while(0)

#define CHECKBIDIREQNODE(n1,n2) CHECKBIDIRXNODE(n1,n2,)
#define CHECKBIDIRNEQNODE(n1,n2) CHECKBIDIRXNODE(n1,n2,!)

#define CHECKBIDIREQSAMENODE(n) CHECKBIDIREQNODE(n,n)

#define CHECKNODEGEN(obj,s) \
	do { \
		char *printbuf; \
		asprintf(&printbuf, "    BUILD '%s': ", s); \
		char *str = json_stringify(obj); \
		INCCOUNTS(strcmp(str,s)==0); \
		PRINTOK(printbuf); \
		json_free(obj); \
		free(printbuf); \
	} while(0)

#define CHECKGETKEY(str,key,cond) \
	do { \
		char *printbuf; \
		asprintf(&printbuf, "    let val = (%s).%s in %s: ", str, key, #cond); \
		Jsonnode *_n=json_parse(str,strlen(str)); \
		Jsonnode *val=json_object_get_item(&_n->objval, key); \
		INCCOUNTS(cond); \
		PRINTOK(printbuf); \
		json_free(_n); \
		if (val) json_free(val); \
		free(printbuf); \
	} while(0)

#define EXPECT(str, block) { \
	bool ispassed = true; \
	char *printbuf; \
	asprintf(&printbuf, "    %s: ", str); \
	block; \
	INCCOUNTS(ispassed); \
	PRINTOK(printbuf); \
	free(printbuf); \
}

#define ASSERT(cond) if (!(cond)) ispassed = false;

#define SECTION(str, block) { \
	int ran = 0, passed = 0; \
	bool ok; \
	if (!quietmode) printf("  %s \n", str); \
	block; \
	ranTotal += ran; \
	passedTotal += passed; \
	if (!quietmode) printf("\n"); \
}


Jsonnode* peanozero(void){
	static const char *basestr="[{\"a\":false,\"b\xff\":[1,null,[]],\"\":{\"kaas\":[{}]}}]";
	return json_parse(basestr,strlen(basestr));
}

void peanosucc(Jsonnode *n){
	Jsonnode *copy=json_copy(n);
	n->arrval.length++;
	n->arrval.elems=realloc(n->arrval.elems,n->arrval.length*sizeof(Jsonnode*));
	n->arrval.elems[n->arrval.length-1]=copy;
}

Jsonnode* peanonumber(int n){
	Jsonnode *node=peanozero();
	while(n-->0)peanosucc(node);
	return node;
}


void perftest(void){
#define TIMEIT(block) \
		do { \
			clock_t start=clock(); assert(start!=-1UL); \
			block \
			clock_t end=clock(); assert(end!=-1UL); \
			clock_t diff=end-start; \
			printf("[%1.6lfs] %s\n",(double)diff/CLOCKS_PER_SEC,#block); \
		} while(0)

	Jsonnode *n;
	TIMEIT(n=peanonumber(20););
	char *s;
	TIMEIT(s=json_stringify(n););
	TIMEIT(json_free(n););
	printf("string length: %lu\n",strlen(s));

#undef TIMEIT
}

void usage(const char *argv0){
	fprintf(stderr,
		"Usage: %s [-pq]\n"
		"    -p  Run performance test instead of regression suite\n"
		"    -q  Don't explain everything\n",
		argv0);
}

int main(int argc,char **argv){
	bool quietmode=false,runperf=false;
	for(int i=1;i<argc;i++){
		if(argv[i][0]!='-'){
			fprintf(stderr,"Unrecognised argument '%s'\n",argv[i]);
			return 1;
		}
		for(int j=1;argv[i][j];j++){
			switch(argv[i][j]){
				case 'q': quietmode=true; break;
				case 'p': runperf=true; break;
				case 'h': usage(argv[0]); return 0; break;
				default:
					fprintf(stderr,"Unrecognised flag '-%c'\n",argv[i][j]);
					return 1;
			}
		}
	}

	if(runperf){
		perftest();
		return 0;
	}

	int ranTotal = 0,
	    passedTotal = 0;

	SECTION("empty", {
		CHECKJSONERR("");
		CHECKJSONERR("    ");
		CHECKJSONERR("\t\t");
	});

	SECTION("numbers", {
		CHECKJSON("123");
		CHECKJSON("21.4e-3");
		CHECKJSONERR("123a");

		// non standard complaint extensions
		CHECKJSON("NaN");
		CHECKJSON("Infinity");
		CHECKJSON("-Infinity");

		Jsonnode *num = json_make_num(100.0);
		CHECKNODEGEN(num, "100");
	});

	SECTION("strings", {
		CHECKJSON("\"kaas\"");
		CHECKJSON("\"kaa\\u003cs\\\"\"");
		CHECKJSONERR("\"");
		CHECKJSON("\"\\uacef\"");
		CHECKJSONERR("\"\\uaceg\"");
		CHECKJSONERR("\"dingen\\\"");

		CHECKBIDIREQSAME("\"iets\"");

		Jsonnode *strval = json_make_str("kaas");
		CHECKNODEGEN(strval, "\"kaas\"");
	});

	SECTION("identifiers", {
		CHECKJSON("true");
		CHECKJSON("false");

		CHECKJSON("null");

		CHECKBIDIREQSAME("null");
		CHECKBIDIREQSAME("false");
		CHECKBIDIRNEQ("true","false");

		Jsonnode *b = json_make_bool(true);
		CHECKNODEGEN(b, "true");
		Jsonnode *null = json_make_null();
		CHECKNODEGEN(null, "null");
	});

	SECTION("arrays", {
		CHECKJSON("[]");
		CHECKJSON("[1]");
		CHECKJSON("[\"aaaaaaaaaaaaaaaaaaaaaaaaaaa\",42]");
		CHECKJSON("[1,2,\"hoi\",\"\\\\\"]");
		CHECKJSONERR("[1,2,\"hoi\",\"\\\"][]");
		CHECKJSON("[[1,2],3]");
		CHECKJSON("[[[[[[[[1],1],1],1],[[1],1],[[[[1],1],1],1],1],1],1],1]");
		CHECKJSONERR("[[[[[[[[1],1],1],1],[[1],1],[[[[[1],1],1],1],1],1],1],1]");
		CHECKJSON("[[[[[1],2],[3],4],[[5],6],[7],8],[[[9],0],[1],2],[[3],4],[5],6]");
		CHECKJSON("[\"hello\"]");
		CHECKJSON("[ \"hello\" ]");
		CHECKJSON("[ \"hello\", \"world\" ]");
		CHECKJSON("[ 1, 2 ]");

		CHECKBIDIREQSAME("[1,2,3]");
		{Jsonnode *n=peanonumber(0); CHECKBIDIREQSAMENODE(n); json_free(n);}
		{Jsonnode *n=peanonumber(1); CHECKBIDIREQSAMENODE(n); json_free(n);}
		{Jsonnode *n=peanonumber(2); CHECKBIDIREQSAMENODE(n); json_free(n);}
		{Jsonnode *n=peanonumber(7); CHECKBIDIREQSAMENODE(n); json_free(n);}

		Jsonnode *arr = json_make_array(1);
		json_array_add_item(&arr->arrval, json_make_str("kaas"));
		CHECKNODEGEN(arr, "[\"kaas\"]");

		EXPECT("resizing", {
			Jsonnode *arrnode = json_make_array(0);
			ASSERT(arrnode->arrval.capacity == 1);
			json_array_add_item(&arrnode->arrval, json_make_str("a"));
			json_array_add_item(&arrnode->arrval, json_make_str("b"));
			ASSERT(arrnode->arrval.capacity == 2);
			json_array_add_item(&arrnode->arrval, json_make_str("c"));
			ASSERT(arrnode->arrval.capacity == 4);

			char *str = json_stringify(arrnode);
			ASSERT(strcmp(str, "[\"a\",\"b\",\"c\"]") == 0);
			free(str);

			json_free(arrnode);
		});

		EXPECT("item removal", {
			const char *str = "[ 1,  2 ]";
			Jsonnode *arrnode = json_parse(str, strlen(str));
			ASSERT(arrnode->arrval.length == 2);
			ASSERT(arrnode->arrval.elems[1]->numval == 2);
			json_array_remove_item(&arrnode->arrval, 0);
			ASSERT(arrnode->arrval.length == 1);
			ASSERT(arrnode->arrval.elems[0]->numval == 2);
			json_free(arrnode);
		});
	});

	SECTION("objects", {
		CHECKJSON("{}");
		CHECKJSONERR("{ \"a\":, }");
		CHECKJSONERR("{ \"a\": 1 \"b\": 2 }");
		CHECKJSONERR("{ \"a\": 1: \"b\": 2 }");
		CHECKJSONERR("{ \"a\": 1, \"b\": 2, }");
		CHECKJSON("{\"a\":[1,2],\"kaas\":null}");
		CHECKJSONERR("{\"obj\":{\"1\":2.\"3\":4},\"x\":\"y\"}");
		CHECKJSON("{\"obj\":{\"1\":2,\"3\":4},\"x\":\"y\"}");
		CHECKJSONERR("{\"obj\":{\"1\":2.\"3\":4},\"x\":y}");
		CHECKJSON("{ \"kaas\": [ \"is\", \"lekker\" ] }");

		CHECKBIDIREQ("{\"a\":\"\\u003c\t\\n\\fkaas\\\"\",\"iets\":[]}",
		             "{\"a\":\"<\\t\\n\\fkaas\\\"\",\"iets\":[]}");

		Jsonnode *obj = json_make_object(1);
		json_object_add_key(&obj->objval, "kaas", json_make_str("lekker"));
		CHECKNODEGEN(obj, "{\"kaas\":\"lekker\"}");

		CHECKGETKEY("{ \"a\": 1, \"b\": 2 }", "b", val->numval == 2);
		CHECKGETKEY("{ \"foo\": 1 }", "bar", val == NULL);

		EXPECT("resizing", {
			Jsonnode *objnode = json_make_object(0);
			ASSERT(objnode->objval.capacity == 1);
			json_object_add_key(&objnode->objval, "0", json_make_str("a"));
			json_object_add_key(&objnode->objval, "1", json_make_str("b"));
			ASSERT(objnode->objval.capacity == 2);
			json_object_add_key(&objnode->objval, "2", json_make_str("c"));
			ASSERT(objnode->objval.capacity == 4);

			char *str = json_stringify(objnode);
			ASSERT(strcmp(str, "{\"0\":\"a\",\"1\":\"b\",\"2\":\"c\"}") == 0);
			free(str);

			json_free(objnode);
		});

		EXPECT("item removal", {
			const char *str = "{ \"a\": 1, \"b\": 2 }";
			Jsonnode *objnode = json_parse(str, strlen(str));
			ASSERT(objnode->objval.numkeys == 2);
			ASSERT(objnode->objval.values[1]->numval == 2);
			json_object_remove_item(&objnode->objval, "a");
			ASSERT(objnode->objval.numkeys == 1);
			ASSERT(objnode->objval.values[0]->numval == 2);
			json_free(objnode);
		});
	});

	bool successful = passedTotal == ranTotal;
	if (quietmode && successful) {
		printf(COK "OK\n" CNORM);
	} else if (successful) {
		printf(COK "All tests OK\n" CNORM);
	} else {
		printf(CERROR "%d of %d failed\n" CNORM, ranTotal - passedTotal, ranTotal);
	}
	return !successful;
}
