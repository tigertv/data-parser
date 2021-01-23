#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "parser.h"


struct record {
	char* name;
	int param[4];
};

struct record* precord = NULL;
struct record records[256];
int records_last_index = 0;
int current_param_index = 0;

void param0(char* s, int size) {
	char* name = malloc(size + 1);
	char* n = name;
	while(size != 0) {
		*n++ = *s;
		++s;
		--size;
	}
	*n = '\0';
	records[records_last_index].name = name;
}

void param1(char* s, int size) {
	(void)size; // unused
	int r = atoi(s);
	records[records_last_index].param[current_param_index++] = r;
}

#define HANDLERS_SIZE 2
void (*handlers[HANDLERS_SIZE])(char *s , int size);

void init_handlers() {
	handlers[0] = param0;
	handlers[1] = param1;
}

void handler_func(int index, char *s , int size) {
	handlers[index](s, size);
}

void parse_data(myparser_t* parser, myparser_node_t* gram, const char* s) {
	parser->cur = s;
	while(*parser->cur != '\0') {
		bool result = myparser_visit(parser, gram);
		current_param_index = 0;	
		if (result) {
			// make record
			++records_last_index;
		} else {
			break;
		}
	}
}

int main() {
	FILE *f = fopen("grammar.txt", "r");
	if(!f) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

	fseek(f, 0, SEEK_END);
	unsigned long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);

	char *grammar = malloc(sizeof(char) * (fsize + 1));
	fread(grammar, sizeof(grammar[0]), fsize, f);
	fclose(f);

	printf("The size of the file is equal to %ld\n", fsize);
	printf("grammar:\n%s\n", grammar);

	// "digit" and "letter" are by default
	myparser_t* parser = myparser_new(handler_func);
	myparser_node_t* gram = myparser_parse_grammar(parser, grammar);	

	printf("\nentries:\n");
	for (int i = 0; i < parser->st_index; ++i) {
		printf("entry: %s\n", parser->st[i].name);
	}
	printf("\n");

	init_handlers();

	f = fopen("data.txt", "r");
	if(!f) {
        perror("File opening failed");
        return EXIT_FAILURE;
    }

	char sp[256];
	while(fgets(sp, 256, f) != NULL) {
		parse_data(parser, gram, (char*)&sp);
	}

	fclose(f);
	myparser_delete(parser);

	for(int i = 0; i < records_last_index; ++i) {
		printf("str %d = %s %d %d %d %d\n", i, records[i].name, records[i].param[0], records[i].param[1], records[i].param[2], records[i].param[3]);
	}

	return 0;
}
