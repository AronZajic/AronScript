#include <string.h>
#include "interpreter.h"

int replPrint(struct Node* node, struct Context *context){

	struct EvalNode evalNode;

	if(node->nodeType == STATEMENTS_NODE){

		int didSomething = 0;

		for (GList *l = node->nodeUnion.statementsNode.body; l != NULL; l = l->next) {

			evalNode = eval(l->data, context);

			didSomething = 1;
		}
		if(didSomething){

			if(evalNode.valueType == INTEGER){
				printf("=%d\n", evalNode.value.integerValue);
			}
			if(evalNode.valueType == DECIMAL){
				printf("=%f\n", evalNode.value.decimalValue);
			}
			if(evalNode.valueType == BOOLEAN){
				if(evalNode.value.integerValue){
					printf("=True\n");
				} else {
					printf("=False\n");
				}
			}
		}
		return 0;
	}

	if(node->nodeType == VALUE_NODE || node->nodeType == BINARY_OPERATION_NODE || node->nodeType == VARIABLE_NODE || node->nodeType == FUNCTION_CALL_NODE || node->nodeType == NOT_NODE){

		//tmp = eval(node, contextVariables, contextFunctions).value.integerValue;

		evalNode = eval(node, context);

		if(evalNode.evalType == NULL_TYPE){
			printf("Error, got NULL type as result.\n");
			return 0;
		}

		//printf("got type %c:", evalNode.valueType);

		if(evalNode.valueType == INTEGER){
			printf("=%d\n", evalNode.value.integerValue);
		}
		if(evalNode.valueType == DECIMAL){
			printf("=%f\n", evalNode.value.decimalValue);
		}
		if(evalNode.valueType == BOOLEAN){
			if(evalNode.value.integerValue){
				printf("=True\n");
			} else {
				printf("=False\n");
			}
		}
		return 0;
	}

	if(node->nodeType == DEFINE_NODE){

		if(g_hash_table_contains(context->variables, node->nodeUnion.asignDefineNode.name)){
			fprintf(stderr, "Variable %s already defined.\n", node->nodeUnion.asignDefineNode.name);
			return 0;
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->nodeUnion.asignDefineNode.expression, context);


		if(tmpExpression->evalType == NULL_TYPE){
			fprintf(stderr, "Right side of declaration is NULL.\n");
			return 0;
		}

		if(tmpExpression->valueType != node->nodeUnion.asignDefineNode.valueType){
			fprintf(stderr, "Left and right side of declaration are not the same type. Left is %c. Right is %c.\n", node->nodeUnion.asignDefineNode.valueType, tmpExpression->valueType);
			return 0;
		}

		g_hash_table_insert(context->variables, g_strdup(node->nodeUnion.asignDefineNode.name), tmpExpression);

		if(tmpExpression->valueType == INTEGER){
			printf("Integer %s set to %d.\n", node->nodeUnion.asignDefineNode.name, tmpExpression->value.integerValue);
		}
		if(tmpExpression->valueType == DECIMAL){
			printf("Decimal %s set to %f.\n", node->nodeUnion.asignDefineNode.name, tmpExpression->value.decimalValue);
		}
		if(tmpExpression->valueType == BOOLEAN){
			if(tmpExpression->value.integerValue){
				printf("Boolean %s set to True.\n", node->nodeUnion.asignDefineNode.name);
			} else {
				printf("Boolean %s set to False.\n", node->nodeUnion.asignDefineNode.name);
			}
		}

		return 0;
	}

	if(node->nodeType == ASIGN_NODE){

		if(!g_hash_table_contains(context->variables, node->nodeUnion.asignDefineNode.name)){
			fprintf(stderr, "Variable %s not defined.\n", node->nodeUnion.asignDefineNode.name);
			return 0;
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->nodeUnion.asignDefineNode.expression, context);

		if(tmpExpression->evalType == NULL_TYPE){
			fprintf(stderr, "Right side of asignment is NULL.\n");
			return 0;
		}

		struct EvalNode *tmpVariable = g_hash_table_lookup(context->variables, node->nodeUnion.asignDefineNode.name);

		if(tmpExpression->valueType != tmpVariable->valueType){
			fprintf(stderr, "Left and right side of asignment are not the same type. Left is %c. Right is %c.\n", tmpVariable->valueType, tmpExpression->valueType);
			return 0;
		}

		g_hash_table_insert(context->variables, g_strdup(node->nodeUnion.asignDefineNode.name), tmpExpression);

		if(tmpExpression->valueType == INTEGER){
			printf("Integer %s set to %d.\n", node->nodeUnion.asignDefineNode.name, tmpExpression->value.integerValue);
		}
		if(tmpExpression->valueType == DECIMAL){
			printf("Decimal %s set to %f.\n", node->nodeUnion.asignDefineNode.name, tmpExpression->value.decimalValue);
		}
		if(tmpExpression->valueType == BOOLEAN){
			if(tmpExpression->value.integerValue){
				printf("Boolean %s set to True.\n", node->nodeUnion.asignDefineNode.name);
			} else {
				printf("Boolean %s set to False.\n", node->nodeUnion.asignDefineNode.name);
			}
		}

		return 0;
	}

	if(node->nodeType == FUNCTION_DECLARATION_NODE){
		if(g_hash_table_contains(context->functions, node->nodeUnion.functionDeclarationNode.name)){
			fprintf(stderr, "Function %s already defined.\n", node->nodeUnion.functionDeclarationNode.name);
			return 0;
		}
		g_hash_table_insert(context->functions, g_strdup(node->nodeUnion.functionDeclarationNode.name), node);
		printf("Function %s defined.\n", node->nodeUnion.functionDeclarationNode.name);
		return 0;
	}

	eval(node, context);
	return 0;
}

struct Node* read_file(FILE *file, struct Node *parent){

	char line[1024];

	struct Node *node;

	while(fgets(line, sizeof(line), file)) {

		line[strcspn(line, "\n")] = 0;
		node = parse(line);

	placeNode:
		if(node != NULL){

			if(node->indentation == parent->indentation + 1){
				parent->nodeUnion.statementsNode.body = g_list_append(parent->nodeUnion.statementsNode.body, node);
			}

			if(node->indentation > parent->indentation + 1){
				fprintf(stderr, "Wrong indentation!\n");
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
                exit(EXIT_FAILURE);
			}

			if(node->indentation < parent->indentation + 1){
				return node;
			}

			if(node->nodeType == WHILE_NODE){
				node->nodeUnion.whileNode.statements = malloc(sizeof(struct Node));
				node->nodeUnion.whileNode.statements->indentation = node->indentation;
				node->nodeUnion.whileNode.statements->nodeType = STATEMENTS_NODE;
				node->nodeUnion.whileNode.statements->nodeUnion.statementsNode.body = NULL;
				struct Node* tmp = read_file(file, node->nodeUnion.whileNode.statements);
				node = tmp;
				goto placeNode;
			}

			if(node->nodeType == FUNCTION_DECLARATION_NODE){
				node->nodeUnion.functionDeclarationNode.statements = malloc(sizeof(struct Node));
				node->nodeUnion.functionDeclarationNode.statements->indentation = node->indentation;
				node->nodeUnion.functionDeclarationNode.statements->nodeType = STATEMENTS_NODE;
				node->nodeUnion.functionDeclarationNode.statements->nodeUnion.statementsNode.body = NULL;
				struct Node* tmp = read_file(file, node->nodeUnion.functionDeclarationNode.statements);
				node = tmp;
				goto placeNode;
			}

			if(node->nodeType == IF_NODE){
				node->nodeUnion.ifNode.ifBody = malloc(sizeof(struct Node));
				node->nodeUnion.ifNode.ifBody->indentation = node->indentation;
				node->nodeUnion.ifNode.ifBody->nodeType = STATEMENTS_NODE;
				node->nodeUnion.ifNode.ifBody->nodeUnion.statementsNode.body = NULL;
				node->nodeUnion.ifNode.elseBody = NULL;

				struct Node* tmp = read_file(file, node->nodeUnion.ifNode.ifBody);
				struct Node* tmpDescend = node;

				while(tmp != NULL && tmp->nodeType == ELSE_IF_NODE){
					tmpDescend->nodeUnion.ifNode.elseBody = tmp;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeType = IF_NODE;

					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody = malloc(sizeof(struct Node));
					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody->indentation = tmpDescend->indentation;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody->nodeType = STATEMENTS_NODE;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody->nodeUnion.statementsNode.body = NULL;

					tmp = read_file(file, tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody);
					tmpDescend = tmpDescend->nodeUnion.ifNode.elseBody;
				}

				if(tmp != NULL && tmp->nodeType == ELSE_NODE){
					tmpDescend->nodeUnion.ifNode.elseBody = malloc(sizeof(struct Node));
					tmpDescend->nodeUnion.ifNode.elseBody->indentation = node->indentation;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeType = STATEMENTS_NODE;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.statementsNode.body = NULL;
					free(tmp);
					tmp = read_file(file, tmpDescend->nodeUnion.ifNode.elseBody);
				}
				node = tmp;
				goto placeNode;
			}
		}
	}
	return NULL;   
}

struct Node* replRead(struct Node *parent, char lineStart[]){

	struct Node *node;

	char input[1024];

	printf("%s", lineStart);
	input[0] = 0;
	fgets(input, sizeof(input), stdin);
	input[strcspn(input, "\n")] = 0;

	while (strcmp(input, "") != 0){

		node = parse(input);

	placeNode:
		if(node != NULL){

			if(node->indentation == parent->indentation + 1){
				parent->nodeUnion.statementsNode.body = g_list_append(parent->nodeUnion.statementsNode.body, node);
			}

			if(node->indentation > parent->indentation + 1){
				fprintf(stderr, "Wrong indentation!");
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
                // TODO delete already loaded data and not execute further
			}

			if(node->indentation < parent->indentation + 1){
				return node;
			}

			if(node->nodeType == WHILE_NODE){
				node->nodeUnion.whileNode.statements = malloc(sizeof(struct Node));
				node->nodeUnion.whileNode.statements->indentation = node->indentation;
				node->nodeUnion.whileNode.statements->nodeType = STATEMENTS_NODE;
				node->nodeUnion.whileNode.statements->nodeUnion.statementsNode.body = NULL;
				struct Node* tmp = replRead(node->nodeUnion.whileNode.statements, ";");
				node = tmp;
				goto placeNode;
			}

			if(node->nodeType == FUNCTION_DECLARATION_NODE){
				node->nodeUnion.functionDeclarationNode.statements = malloc(sizeof(struct Node));
				node->nodeUnion.functionDeclarationNode.statements->indentation = node->indentation;
				node->nodeUnion.functionDeclarationNode.statements->nodeType = STATEMENTS_NODE;
				node->nodeUnion.functionDeclarationNode.statements->nodeUnion.statementsNode.body = NULL;
				struct Node* tmp = replRead(node->nodeUnion.functionDeclarationNode.statements, ";");
				node = tmp;
				goto placeNode;
			}

			if(node->nodeType == IF_NODE){
				node->nodeUnion.ifNode.ifBody = malloc(sizeof(struct Node));
				node->nodeUnion.ifNode.ifBody->indentation = node->indentation;
				node->nodeUnion.ifNode.ifBody->nodeType = STATEMENTS_NODE;
				node->nodeUnion.ifNode.ifBody->nodeUnion.statementsNode.body = NULL;
				node->nodeUnion.ifNode.elseBody = NULL;

				struct Node* tmp = replRead(node->nodeUnion.ifNode.ifBody, ";");
				struct Node* tmpDescend = node;

				while(tmp != NULL && tmp->nodeType == ELSE_IF_NODE){
					tmpDescend->nodeUnion.ifNode.elseBody = tmp;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeType = IF_NODE;

					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody = malloc(sizeof(struct Node));
					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody->indentation = tmpDescend->indentation;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody->nodeType = STATEMENTS_NODE;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody->nodeUnion.statementsNode.body = NULL;

					tmp = replRead(tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.ifNode.ifBody, ";");
					tmpDescend = tmpDescend->nodeUnion.ifNode.elseBody;
				}

				if(tmp != NULL && tmp->nodeType == ELSE_NODE){
					tmpDescend->nodeUnion.ifNode.elseBody = malloc(sizeof(struct Node));
					tmpDescend->nodeUnion.ifNode.elseBody->indentation = node->indentation;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeType = STATEMENTS_NODE;
					tmpDescend->nodeUnion.ifNode.elseBody->nodeUnion.statementsNode.body = NULL;
					free(tmp);
					tmp = replRead(tmpDescend->nodeUnion.ifNode.elseBody, ";");
				}
				node = tmp;
				goto placeNode;
			}
		}
		if(strcmp(lineStart, ">") == 0){
			return NULL;
		}
		printf("%s", lineStart);
		input[0] = 0;
		fgets(input, sizeof(input), stdin);
		input[strcspn(input, "\n")] = 0;
	}
	return NULL;   
}

int main(int argc, char **argv) {
	
	GHashTable *variables;
	GHashTable *functions;

	variables = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
	functions = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, freeFunctionDeclaration);

	struct Context *context = malloc(sizeof(struct Context));
	context->parent = NULL;
	context->variables = variables;
	context->functions = functions;

	eval(parse("function printInteger ( Integer i ) :"), context);
	eval(parse("function printLineInteger ( Integer i ) :"), context);

	eval(parse("function printDecimal ( Decimal d ) :"), context);
	eval(parse("function printLineDecimal ( Decimal d ) :"), context);

	eval(parse("function printBoolean ( Boolean b ) :"), context);
	eval(parse("function printLineBoolean ( Boolean b ) :"), context);

	struct Node *program = malloc(sizeof(struct Node));
	program->nodeType = STATEMENTS_NODE;
	program->indentation = -1;
	program->nodeUnion.statementsNode.body = NULL;

	if(argc == 1){

		while (1)
		{

			program->nodeUnion.statementsNode.body = NULL;

			replRead(program, ">");

			if(g_list_length(program->nodeUnion.statementsNode.body) != 0)
				replPrint(program->nodeUnion.statementsNode.body->data, context);
		}
	} else if(argc == 2) {

		FILE *file;

		file = fopen(argv[1], "r");

        if(file == NULL){
            fprintf(stderr, "File \"%s\" does not exist.\n", argv[1]);
            exit(EXIT_FAILURE);
        }

		read_file(file, program);

		replPrint(program, context);
		//eval(program, context);

		fclose(file);
	} else if(argc == 3){

		if(strcmp(argv[1], "check") != 0){
			fprintf(stderr, "Wrong program arguments.\n");
			fprintf(stderr, "Usage:\n");
			fprintf(stderr, "\tAronScript \t\t\t- for running as REPL\n");
			fprintf(stderr, "\tAronScript fileName \t\t- for executing a file\n");
			fprintf(stderr, "\tAronScript check fileName \t- for using the linter on a file\n");
			exit(EXIT_FAILURE);
		}

		FILE *file;

		file = fopen(argv[2], "r");

        if(file == NULL){
            fprintf(stderr, "File \"%s\" does not exist.\n", argv[1]);
            exit(EXIT_FAILURE);
        }

		read_file(file, program);

	} else {
		fprintf(stderr, "Wrong number of program arguments.\n");
		fprintf(stderr, "Usage:\n");
		fprintf(stderr, "\tAronScript \t\t\t- for running as REPL\n");
		fprintf(stderr, "\tAronScript fileName \t\t- for executing a file\n");
		fprintf(stderr, "\tAronScript check fileName \t- for using the linter on a file\n");
        exit(EXIT_FAILURE);
	}

	freeNode(program);
		
	g_hash_table_destroy(variables);
	g_hash_table_destroy(functions);
	free(context);
    exit(EXIT_SUCCESS);
}
