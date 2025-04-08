#include <string.h>
#include "interpreter.h"

/*int handleIf(struct Node* node, GHashTable *contextVariables, GHashTable *contextFunctions){
	int tmp;
	for (GList *l = node->body; l != NULL; l = l->next) {

		struct Node* iteratorNode = l->data;

		if(iteratorNode->nodeType == IF_NODE){
			if(eval(iteratorNode->condition, contextVariables, contextFunctions)){
				for (GList *l2 = iteratorNode->body; l2 != NULL; l2 = l2->next) {
					handleIf(l2->data, contextVariables, contextFunctions);
				}
				continue;
			} else {
				l = l->next;
				iteratorNode = l->data;

				if(l != NULL){
					break;
				}

				if(iteratorNode->nodeType == ELSE_NODE){
					for (GList *l2 = iteratorNode->body; l2 != NULL; l2 = l2->next) {
						handleIf(l2->data, contextVariables, contextFunctions);
					}
					continue;
				}
			}
		}
		tmp = eval(l->data, contextVariables, contextFunctions);
	}
	return tmp;
}*/

int replPrint(struct Node* node, GHashTable *contextVariables, GHashTable *contextFunctions){

	int tmp;

	if(node->nodeType == STATEMENTS_NODE){

		for (GList *l = node->body; l != NULL; l = l->next) {
			tmp = eval(l->data, contextVariables, contextFunctions).value.intValue;
		}
		printf("=%d\n", tmp);
		return 0;
	}

	/*if(node->nodeType == WHILE_NODE || node->nodeType == IF_NODE){
		eval(node, contextVariables, contextFunctions);
	}*/

	if(node->nodeType == VALUE_NODE || node->nodeType == BINARY_OPERATION_NODE || node->nodeType == VARIABLE_NODE || node->nodeType == FUNCTION_CALL_NODE){

		struct EvalNode evalNode = eval(node, contextVariables, contextFunctions);

		//tmp = eval(node, contextVariables, contextFunctions).value.intValue;

		if(evalNode.valueType == INTEGER){
			printf("=%d\n", evalNode.value.intValue);
		}
		if(evalNode.valueType == DECIMAL){
			printf("=%f\n", evalNode.value.decimalValue);
		}
		if(evalNode.valueType == BOOLEAN){
			if(evalNode.value.intValue){
				printf("=True\n");
			} else {
				printf("=False\n");
			}
		}
		return 0;
	}

	if(node->nodeType == DEFINE_NODE || node->nodeType == ASIGN_NODE){

		if(node->nodeType == DEFINE_NODE && g_hash_table_contains(contextVariables, node->name)){
			fprintf(stderr, "Variable %s already defined.\n", node->name);
			return 0;
		}

		struct EvalNode *tmp = malloc(sizeof(struct EvalNode));
		*tmp = eval(node->expression, contextVariables, contextFunctions);

		if(tmp->valueType != node->valueType){
			fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", node->valueType, tmp->evalType);
			return 0;
		}

		g_hash_table_insert(contextVariables, g_strdup(node->name), (tmp));
		tmp = g_hash_table_lookup(contextVariables, node->name);
		
		if(tmp->valueType == INTEGER){
			printf("Integer %s set to %d.\n", node->name, tmp->value.intValue);
		}
		if(tmp->valueType == DECIMAL){
			printf("Decimal %s set to %f.\n", node->name, tmp->value.decimalValue);
		}
		if(tmp->valueType == BOOLEAN){
			if(tmp->value.intValue){
				printf("Boolean %s set to True.\n", node->name);
			} else {
				printf("Boolean %s set to False.\n", node->name);
			}
		}

		return 0;
	}

	if(node->nodeType == FUNCTION_DECLARATION_NODE){
		if(g_hash_table_contains(contextFunctions, node->name)){
			fprintf(stderr, "Function %s already defined.\n", node->name);
			return 0;
		}
		g_hash_table_insert(contextFunctions, g_strdup(node->name), node);
		printf("Function %s defined.\n", node->name);
		return 0;
	}

	eval(node, contextVariables, contextFunctions);
	return 0;
}

struct Node* read_file(FILE *file, struct Node *parent){

	char line[1024];

	struct Node *node;

	while(fgets(line, sizeof(line), file)) {

		//printf("%s", line);

		line[strcspn(line, "\n")] = 0;
		node = parse(line);

	placeNode:
		if(node != NULL){

			if(node->indentation == parent->indentation + 1){
				parent->body = g_list_append(parent->body, node);
			}

			if(node->indentation > parent->indentation + 1){
				fprintf(stderr, "Wrong indentation!\n");
                exit(EXIT_FAILURE);
			}

			if(node->indentation < parent->indentation + 1){
				return node;
			}

			if(node->nodeType == WHILE_NODE || node->nodeType == FUNCTION_DECLARATION_NODE){
				struct Node* tmp = read_file(file, node);
				node = tmp;
				goto placeNode;
			}

			if(node->nodeType == IF_NODE){
				node->left = malloc(sizeof(struct Node));
				node->left->indentation = node->indentation;
				node->left->nodeType = STATEMENTS_NODE;
				node->left->body = NULL;

				struct Node* tmp = read_file(file, node->left);

				if(tmp != NULL && tmp->nodeType == ELSE_NODE){
					node->right = malloc(sizeof(struct Node));
					node->right->indentation = node->indentation;
					node->right->nodeType = STATEMENTS_NODE;
					node->right->body = NULL;

					tmp = read_file(file, node->right);
				} else {
					node->right = NULL;
				}
				node = tmp;
				goto placeNode;
			}
		}
	}
	return NULL;   
}

struct Node* replRead(struct Node *parent, char lineStart){

	struct Node *node;

	char input[1024];

	printf("%c", lineStart);
	fgets(input, sizeof(input), stdin);
	input[strcspn(input, "\n")] = 0;

	while (strcmp(input, "") != 0){

		node = parse(input);

	placeNode:
		if(node != NULL){

			if(node->indentation == parent->indentation + 1){
				parent->body = g_list_append(parent->body, node);
			}

			if(node->indentation > parent->indentation + 1){
				fprintf(stderr, "Wrong indentation!");
                // TODO delete already loaded data and not execute further
			}

			if(node->indentation < parent->indentation + 1){
				return node;
			}

			if(node->nodeType == WHILE_NODE || node->nodeType == FUNCTION_DECLARATION_NODE){
				struct Node* tmp = replRead(node, ';');
				node = tmp;
				goto placeNode;
			}

			if(node->nodeType == IF_NODE){
				node->left = malloc(sizeof(struct Node));
				node->left->indentation = node->indentation;
				node->left->nodeType = STATEMENTS_NODE;
				node->left->body = NULL;

				struct Node* tmp = replRead(node->left, ';');

				if(tmp != NULL && tmp->nodeType == ELSE_NODE){
					node->right = malloc(sizeof(struct Node));
					node->right->indentation = node->indentation;
					node->right->nodeType = STATEMENTS_NODE;
					node->right->body = NULL;

					tmp = replRead(node->right, ';');
				} else {
					node->right = NULL;
				}
				node = tmp;
				goto placeNode;
			}
		}
		if(lineStart == '>'){
			return NULL;
		}
		printf("%c", lineStart);
		fgets(input, sizeof(input), stdin);
		input[strcspn(input, "\n")] = 0;
	}
	return NULL;   
}

void treeprint(struct Node *root, int level){
	if (root == NULL)
			return;
	for (int i = 0; i < level; i++)
			printf("  ");
	printf("%c %d %c\n", root->nodeType, root->value.intValue, root->binaryOperation);
	if(root->nodeType == BINARY_OPERATION_NODE){
		treeprint(root->left, level + 1);
		treeprint(root->right, level + 1);
	} else if(root->nodeType == WHILE_NODE || root->nodeType == STATEMENTS_NODE || root->nodeType == FUNCTION_DECLARATION_NODE){
		for (GList *l = root->body; l != NULL; l = l->next) {
			treeprint(l->data, level + 1);
		}
	}
}

int main(int argc, char **argv) {
	
	GHashTable *variables;
	GHashTable *functions;

	variables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
	functions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);

	struct Node *program = malloc(sizeof(struct Node));
	program->nodeType = STATEMENTS_NODE;
	program->indentation = -1;
	program->body = NULL;

	if(argc == 1){

		//char input[1024];

		while (1)
		{
			//printf(">");

			program->body = NULL;

			replRead(program, '>');

			replPrint(program, variables, functions);

			/*fgets(input, sizeof(input), stdin);
			input[strcspn(input, "\n")] = 0;

			//printf("$%s$", input);

			struct Node *node = parse(input);
			if(node != NULL){

				if(
					node->nodeType == FUNCTION_DECLARATION_NODE ||
					node->nodeType == WHILE_NODE ||
					node->nodeType == ELSE_NODE
				) {
					replRead(node);
				}
				if(node->nodeType == IF_NODE){
					node->left = malloc(sizeof(struct Node));
					node->left->indentation = node->indentation;
					node->left->nodeType = STATEMENTS_NODE;
					node->left->body = NULL;
					replRead(node->left);
				}
				replPrint(node, variables, functions);
			}*/
		}
	} else {

		FILE *file;

		file = fopen(argv[1], "r");

        if(file == NULL){
            fprintf(stderr, "File \"%s\" does not exist.\n", argv[1]);
            exit(EXIT_FAILURE);
        }

		//char line[1024];

		read_file(file, program);

		//treeprint(program, 0);

		replPrint(program, variables, functions);

	}
		
	g_hash_table_destroy(variables);
    exit(EXIT_SUCCESS);
}
