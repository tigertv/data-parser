#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "parser.h"

//////////////////////////////////////////////////////

static myparser_st_entry_t* myparser_find_entry(myparser_t* self, char *name);
static myparser_st_entry_t* myparser_add_entry(myparser_t* self, char* name, myparser_node_t* node);

static int myparser_st_num_param(char* name);
static void myparser_skip_spaces(myparser_t* self);
static void myparser_ex(myparser_t* self, char c);
static char* myparser_id(myparser_t* self);
static myparser_node_t* myparser_expr(myparser_t* self);

//////////////////////////////////////////////////////

static int myparser_st_num_param(char* name) {
	char *p = "%";
	char *n = name;
	while (*p == *n && *p != '\0') {
		++p;
		++n;
	}
	if (*p != '\0') return -1;

	return atoi(n);
}

//////////////////////////////////////////////////////

myparser_t* myparser_new(void (*handle_func)(int index, char *s , int size)) {
	myparser_t* self = malloc(sizeof(myparser_t));
	self->st_size = ENTRIES;
	self->st = malloc(sizeof(myparser_st_entry_t) * ENTRIES);
	self->st_index = 0;
	self->cur = NULL;
	self->handler = handle_func;
	return self;
}

void myparser_delete(myparser_t* self) {
	free(self->st);
	free(self);
}

static myparser_st_entry_t* myparser_find_entry(myparser_t* self, char *name) {
	for (int j = 0; j < self->st_index; ++j) {
		char* p = self->st[j].name;
		char* ii = name;
		// find entry by name
		while (*p == *ii && *p != '\0')  {
			++p;
			++ii;
		}
		
		if (*p == '\0' && *ii == *p) {
			return &self->st[j];
		}
	}
	return NULL;
}

static myparser_st_entry_t* myparser_add_entry(myparser_t* self, char* name, myparser_node_t* node) {
	int i = self->st_index;
	self->st[i].name = name;
	//printf("name=%s\n", name);
	self->st[i].node = node;
	return &self->st[self->st_index++];
}

static void myparser_skip_spaces(myparser_t* self) {
	while (*self->cur == ' ' || *self->cur == '\n') ++self->cur;
}

static void myparser_ex(myparser_t* self, char c) { // expect char
	myparser_skip_spaces(self);
	
	if (*self->cur != c) {
		printf("\nError: expected '%c' but not '%c'\n", c, *self->cur);
		exit(-1);
	}
	++self->cur;
}

static char* myparser_id(myparser_t* self) { // identificator
	myparser_skip_spaces(self);

	char* p = self->cur;
	while (*self->cur != ' ' && *self->cur != '\0' && *self->cur != '}' && *self->cur != '|' && *self->cur != ';') {
		++self->cur;
	}
	unsigned int size = self->cur - p;
	char* ret = malloc(size + 1);
	char *pret = ret;
	for(unsigned int i = size; i != 0; --i) {
		 *pret++ = *p++;
	}

	*pret = '\0';
	return ret;	
}

static myparser_node_t* myparser_expr(myparser_t* self) {
	myparser_skip_spaces(self);
	myparser_node_t* pnode = NULL;

	while (*self->cur != '\0' && *self->cur != ';') { 
		//printf("operation = %d\n", operation);
		if (*self->cur == '\'') {
			// terminal
			++self->cur;
			char c = *self->cur;
			if (c == '\0') {
				printf("Error: expected a char\n");
				exit(-1);
			}
			++self->cur;

			//printf("terminal = '%c'\n", c);
			myparser_node_t* term = malloc(sizeof(myparser_node_t));
			term->value = c; 
			term->type = TERM;			

			myparser_ex(self, '\'');

			if (pnode) {
				myparser_node_t* anode = malloc(sizeof(myparser_node_t));
				anode->type = AND;
				anode->left = pnode;
				anode->right = term;
				pnode = anode;
			} else {
				pnode = term;
			}
			myparser_skip_spaces(self);

		} else if (*self->cur == '|') {
			++self->cur;
			myparser_node_t* n = myparser_expr(self);
			if (pnode) {
				myparser_node_t* anode = malloc(sizeof(myparser_node_t));
				anode->type = OR;
				anode->left = pnode;
				anode->right = n;
				pnode = anode;
			} else {
				pnode = n;
			}
			myparser_skip_spaces(self);
			
		} else if (*self->cur == '(') {
			 
		} else if (*self->cur == ')') {

		} else if (*self->cur == '{') {
			++self->cur;
			myparser_node_t* n = myparser_expr(self);

			myparser_node_t* rep = malloc(sizeof(myparser_node_t));
			rep->type = REPEAT;
			rep->left = n;
			rep->right = NULL;

			if (pnode) {
				myparser_node_t* anode = malloc(sizeof(myparser_node_t));
				anode->type = AND;
				anode->left = pnode;
				anode->right = rep;
			} else {
				pnode = rep;
			}

		} else if (*self->cur == '}') {
			++self->cur;
			myparser_skip_spaces(self);
			return pnode;
		} else if (*self->cur == ' ') {
			printf("SPACE HERE");
			myparser_skip_spaces(self);
		} else {
			// non-terminal

			char* name = myparser_id(self);
//				printf("non-terminal = %s\n", name);
			// search in symbol_table
			myparser_st_entry_t* entry = myparser_find_entry(self, name);

			if (entry) {
				free(name);
			} else {
				// add in table
//					printf("add in table: %s\n", name);
				myparser_node_t* anode = malloc(sizeof(myparser_node_t));
				anode->type = NON_TERM;
				anode->id = name;
				anode->left = NULL;
				anode->right = NULL;

				entry = myparser_add_entry(self, name, anode);

				int num = myparser_st_num_param(name);

				if (num != -1) {
					entry->node->type = PARAM;
					entry->node->param_value = num;
				}
			}
			
			if (pnode) {
				myparser_node_t* anode = malloc(sizeof(myparser_node_t));
				anode->type = AND;//operation;
				anode->left = pnode;
				anode->right = entry->node;

				pnode = anode;
			} else {
				pnode = entry->node;
			}
			myparser_skip_spaces(self);
		}

	}
	return pnode;
}

myparser_node_t* myparser_parse_grammar(myparser_t* self, const char * grammar) {
	self->cur = grammar;

	while(*self->cur != '\0') {
		char* record = myparser_id(self);
		myparser_st_entry_t* root_entry = myparser_find_entry(self, record);

		if (!root_entry) {
			myparser_node_t* anode = malloc(sizeof(myparser_node_t));
			anode->type = NON_TERM;
			anode->id = record;
			anode->left = NULL;
			anode->right = NULL;

			root_entry = myparser_add_entry(self, record, anode);
		}

		myparser_ex(self, '=');

		myparser_node_t* pnode = myparser_expr(self);
		if(root_entry->node->type == NON_TERM) {
			myparser_node_t* p = root_entry->node;
			p->type = pnode->type;
			p->left = pnode->left;
			p->right = pnode->right;
			p->id = pnode->id;
			p->param_value = pnode->param_value;
			p->value = pnode->value;
			free(pnode);
		} else {
			root_entry->node->left = pnode;
		}
		if (*self->cur == ';') ++self->cur;
		myparser_skip_spaces(self);
	}

	// Note: NON_TERM node shouldn't be presented in the symbol table at the end of creating of grammar AST
	myparser_st_entry_t* entry = myparser_find_entry(self, "digit");
	// set "digit" to default if there is no NON_TERMINAL "digit"
	if (entry && entry->node->type == NON_TERM) {
		myparser_node_t* p = entry->node;
		p->type = DIGIT;
	}

	entry = myparser_find_entry(self, "letter");
	// set "letter" to default if there is no NON_TERMINAL "letter"
	if (entry && entry->node->type == NON_TERM) {
		myparser_node_t* p = entry->node;
		p->type = LETTER;
	}

	return self->st[0].node;
}

bool myparser_visit(myparser_t* self, myparser_node_t* node) {
	if (node == NULL) {
		printf("myparser_node_t is NULL\n");
		return false;
	}

	switch(node->type) {
		case TERM:
			myparser_skip_spaces(self);

			if (node->value == *self->cur) {
				printf("term=%c\n", *self->cur);
				//printf("equal\n");
				++self->cur;
				return true;
			} 
			//printf("Expected '%c' but not '%c'\n", node->value, *cur);
			return false;
			break;

		/*
		case NON_TERM:
			printf("NON TERMINAL: %s\n", node->id);
			return myparser_visit(node->left);
			break;
		//*/

		case LETTER : {
			//printf("visit LETTER: %c\n", *self->cur);
			if ( (*self->cur >= 'a' && *self->cur <= 'z') || (*self->cur >= 'A' && *self->cur <= 'Z') ) {
				printf("LETTER: %c\n", *self->cur);
				++self->cur;
				return true;
			}
			return false;
		}	break;

		case DIGIT : {
			//myparser_skip_spaces();
			//printf("visit DIGIT: %c\n", *cur);
			if (*self->cur >= '0' && *self->cur <= '9') {
				printf("DIGIT: %c\n", *self->cur);
				++self->cur;
				return true;
			}
			return false;
		}
			break;

		case REPEAT: {
			//printf("REPEAT: %s\n", "g");
			char* tmp = self->cur;	
			while(myparser_visit(self, node->left)) {
				tmp = self->cur;
			}
			self->cur = tmp;
			//printf("REPEAT end: %c\n", *cur);
			return true;
		}
			break;

		case PARAM:
			myparser_skip_spaces(self);
			//printf("PARAM: %s\n", node->id);
			{
				// remember string
				char* begin_visit = self->cur;
				bool r = myparser_visit(self, node->left);
				int diff = (int)(self->cur - begin_visit);
				self->handler(node->param_value, begin_visit, diff);
				return r;
			}
			break;

		case AND: {
			myparser_skip_spaces(self);
			//printf("AND\n");
			bool a = myparser_visit(self, node->left);
			if (a == false) return a;
			//printf("AND: a = %d\n", a);
			bool b = myparser_visit(self, node->right);
			//printf("AND: b = %d\n", b);
			return a && b;
		}
			break;

		case OR: {
			myparser_skip_spaces(self);
			//printf("OR\n");
			//printf("OR cur = %c\n", *cur);
			char* tmp = self->cur;	
			bool a = myparser_visit(self, node->left);
			//printf("OR a cur = %c\n", *cur);
			//printf("OR: a = %d\n", a);
			if (a) return true;
			self->cur = tmp;
			bool b = myparser_visit(self, node->right);
			//printf("OR b cur = %c\n", *cur);
			//printf("OR: b = %d\n", b);
			return b;
		}
			break;

		default:
			// error
			printf("Error: wrong node type\n");
			exit(-1);
			break;
	}

}

