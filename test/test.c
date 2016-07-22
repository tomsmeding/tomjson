#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../tomjson.h"

const char *colorCode(const char *color) {
	if      (strcmp(color, "black"  ) == 0) return "\x1B[30m";
	else if (strcmp(color, "red"    ) == 0) return "\x1B[31m";
	else if (strcmp(color, "green"  ) == 0) return "\x1B[32m";
	else if (strcmp(color, "yellow" ) == 0) return "\x1B[33m";
	else if (strcmp(color, "blue"   ) == 0) return "\x1B[34m";
	else if (strcmp(color, "magenta") == 0) return "\x1B[35m";
	else if (strcmp(color, "cyan"   ) == 0) return "\x1B[36m";
	else if (strcmp(color, "white"  ) == 0) return "\x1B[37m";
	return "\033[0m";
}

char *color(const char *str, const char *color) {
	char *x;
	asprintf(&x, "%s%s%s", colorCode(color), str, colorCode(""));
	return x;
}

#define PRINTOK() printf("%s\n", ok ? color("OK", "green") : color("FAIL", "red"))

#define CHECK(cond) \
		do { \
			bool _c=(cond); \
			ran++; \
			passed += _c; \
			if (!_c) ok = false; \
		} while(0)


#define CHECKJSONX(str,not) \
		do { \
			printf("    %s'%s': ", not false ? "NOT " : "", str); \
			Jsonnode *_n=json_parse(str,strlen(str)); \
			if(not false) \
				CHECK(!_n); \
			else \
				CHECK(_n); \
			if(_n)json_free(_n); \
			PRINTOK(); \
		} while(0)

#define CHECKJSON(str) CHECKJSONX(str,)
#define CHECKJSONERR(str) CHECKJSONX(str,!)


#define CHECKBIDIRX(s1,s2,not) \
		do { \
			printf("    '%s' %c= '%s': ", s1, not false ? '!' : '=', s2); \
			Jsonnode *n1=json_parse((s1),strlen((s1))); \
			Jsonnode *n2=json_parse((s2),strlen((s2))); \
			CHECK(not json_equal(n1,n2)); \
			char *s1s=json_stringify(n1); \
			CHECK(not(strcmp(s1s,s2)==0)); \
			PRINTOK(); \
		} while(0)

#define CHECKBIDIREQ(s1,s2) CHECKBIDIRX(s1,s2,)
#define CHECKBIDIRNEQ(s1,s2) CHECKBIDIRX(s1,s2,!)

#define CHECKBIDIREQSAME(s) CHECKBIDIREQ(s,s)


#define SECTION(str, block) { \
	int ran = 0, passed = 0; \
	bool ok = true; \
	printf("  %s \n", str); \
	block; \
	ranTotal += ran; \
	passedTotal += passed; \
	printf("\n"); \
}


int main(void){
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
	});

	SECTION("strings", {
		CHECKJSON("\"kaas\"");
		CHECKJSON("\"kaa\\u003cs\\\"\"");
		CHECKJSONERR("\"");
		CHECKJSON("\"\\uacef\"");
		CHECKJSONERR("\"\\uaceg\"");
		CHECKJSONERR("\"dingen\\\"");

		CHECKBIDIREQSAME("\"iets\"");
	});

	SECTION("identifiers", {
		CHECKJSON("true");
		CHECKJSON("false");

		CHECKJSON("null");

		CHECKBIDIREQSAME("null");
		CHECKBIDIREQSAME("false");
		CHECKBIDIRNEQ("true","false");
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
	});

	SECTION("objects", {
		CHECKJSON("{}");
		CHECKJSON("{\"a\":[1,2],\"kaas\":null}");
		CHECKJSONERR("{\"obj\":{\"1\":2.\"3\":4},\"x\":\"y\"}");
		CHECKJSON("{\"obj\":{\"1\":2,\"3\":4},\"x\":\"y\"}");
		CHECKJSONERR("{\"obj\":{\"1\":2.\"3\":4},\"x\":y}");
		CHECKJSON("{ \"kaas\": [ \"is\", \"lekker\" ] }");

		CHECKBIDIREQ("{\"a\":\"\\u003c\t\n\fkaas\\\"\",\"iets\":[]}",
		             "{\"a\":\"<\\t\\n\\fkaas\\\"\",\"iets\":[]}");
	});

	bool successful = passedTotal == ranTotal;
	printf(color("%d failed out of %d\n", successful ? "green" : "red"), ranTotal - passedTotal, ranTotal);
	return successful ? 0 : 1;
}
