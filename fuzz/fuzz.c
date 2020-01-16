#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include <tomjson.h>


int main(int argc,char **argv){
	if(argc!=2){
		printf("Pass file to parse as argument\n");
		return 1;
	}

	const char *fname=argv[1];

	__AFL_INIT();

	size_t bufsz=4096;
	char *buf=malloc(bufsz);
	assert(buf);

	while(__AFL_LOOP(1000)){
		FILE *f=fopen(fname,"r");
		assert(f);

		size_t cursor=0;
		while(true){
			if(bufsz-cursor<bufsz/4){
				bufsz*=2;
				buf=realloc(buf,bufsz);
				assert(buf);
			}
			size_t nr=fread(buf+cursor,1,bufsz-cursor,f);
			if(nr==0){
				assert(!ferror(f));
				assert(feof(f));
				break;
			}
			cursor+=nr;
		}

		fclose(f);

		assert((size_t)(int)cursor==cursor);

		Jsonnode *node=json_parse(buf,cursor);
		if(node){
			printf("Valid JSON\n");
			json_free(node);
		} else {
			printf("Invalid JSON\n");
		}
	}
}
