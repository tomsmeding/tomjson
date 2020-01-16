#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include <tomjson.h>


int main(int argc,char **argv){
	(void)argv;
	if(argc!=1){
		printf("Does not expect to be called with arguments\n");
		return 1;
	}

#ifdef __AFL_HAVE_MANUAL_CONTROL
	__AFL_INIT();
#endif

	size_t bufsz=100;
	char *buf=malloc(bufsz);
	assert(buf);
	size_t cursor=0;

	while(true){
		if(bufsz-cursor<bufsz/4){
			bufsz*=2;
			buf=realloc(buf,bufsz);
			assert(buf);
		}
		size_t nr=fread(buf+cursor,1,bufsz-cursor,stdin);
		if(nr==0){
			assert(!ferror(stdin));
			assert(feof(stdin));
			break;
		}
		cursor+=nr;
	}

	assert((size_t)(int)cursor==cursor);

	Jsonnode *node=json_parse(buf,cursor);
	if(!node){
		printf("Invalid JSON\n");
		return 1;
	}
	printf("Valid JSON\n");
	json_free(node);
}
