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
	//printf("param0 executed\nstr=");
	char* name = malloc(size + 1);
	char* n = name;
	while(size != 0) {
		*n++ = *s;
		//printf("%c", *s);
		++s;
		--size;
	}
	//printf("\n");
	*n = '\0';
	records[records_last_index].name = name;
}

void param1(char* s, int size) {
	(void)size; // unused
	int r = atoi(s);
	records[records_last_index].param[current_param_index++] = r;
}

void dummy_func(char* s, int size) {
	printf("DUMMY FUNCTION executed\nstr=");
	while(size != 0) {
		printf("%c", *s++);
		--size;
	}
	printf("\n");
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
		//printf("PARSE_DATA result = %d\n", result);
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
	const char *grammar = 
		"record = '{' ''' %0 ''' ';' two_param ';' two_param '}' ;"
		"%0 = {char} ;"
		"char = letter | ' ' | digit ;"
		"two_param = '[' %1 '&' %1 ']' ;"
		"%1 = {digit} ;"
	;
	// "digit" and "letter" are by default
	myparser_t* parser = myparser_new(handler_func);
	myparser_node_t* gram = myparser_parse_grammar(parser, grammar);	

	printf("\nentries:\n");
	for (int i = 0; i < parser->st_index; ++i) {
		printf("entry: %s\n", parser->st[i].name);
	}
	printf("\n");

	init_handlers();

	const char * s[] = {
		" { 'Parametr1 2001' ; [ 5 & 2 ] ; [ 3 & 2 ] }{'ParametrLoooooooong 2021';[10&5];[ 6 &4]}",
		"{'Parametr 2002' ; [ 5&2 ] ; [ 3 & 4 ] }",
		"{ 'Parametr 2003';[ 5 & 2 ];[ 3 &10 ] }"
	};

	char ** sp = (char**) s;
	char ** end = sp + sizeof(s) / sizeof(s[0]);
	for(; sp != end; ++sp) {
		parse_data(parser, gram, *sp);
	}

	myparser_delete(parser);

	for(int i = 0; i < records_last_index; ++i) {
		printf("str %d = %s %d %d %d %d\n", i, records[i].name, records[i].param[0], records[i].param[1], records[i].param[2], records[i].param[3]);
	}
}

