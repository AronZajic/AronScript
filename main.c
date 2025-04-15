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

	/*if(node->nodeType == DEFINE_NODE || node->nodeType == ASIGN_NODE){

		if(node->nodeType == DEFINE_NODE && g_hash_table_contains(contextVariables, node->name)){
			fprintf(stderr, "Variable %s already defined.\n", node->name);
			return 0;
		}

		struct EvalNode *tmp = malloc(sizeof(struct EvalNode));
		*tmp = eval(node->expression, contextVariables, contextFunctions);

		if(tmp->valueType != node->valueType){
			fprintf(stderr, "Left and right side of asignment are not the same type. Left is %c. Right is %c.\n", node->valueType, tmp->evalType);
			return 0;
		}

		g_hash_table_insert(contextVariables, g_strdup(node->name), (tmp));
		tmp = g_hash_table_lookup(contextVariables, node->name);
		
		if(tmp->valueType == INTEGER){
			printf("Integer %s set to %d.\n", node->name, tmp->value.integerValue);
		}
		if(tmp->valueType == DECIMAL){
			printf("Decimal %s set to %f.\n", node->name, tmp->value.decimalValue);
		}
		if(tmp->valueType == BOOLEAN){
			if(tmp->value.integerValue){
				printf("Boolean %s set to True.\n", node->name);
			} else {
				printf("Boolean %s set to False.\n", node->name);
			}
		}

		return 0;
	}*/

	if(node->nodeType == DEFINE_NODE){

		if(g_hash_table_contains(context->variables, node->nodeUnion.asignDefineNode.name)){
			fprintf(stderr, "Variable %s already defined.\n", node->nodeUnion.asignDefineNode.name);
			return 0;
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->nodeUnion.asignDefineNode.expression, context);

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

		//printf("%s", line);

		line[strcspn(line, "\n")] = 0;
		node = parse(line);

	placeNode:
		if(node != NULL){

			if(node->indentation == parent->indentation + 1){
				parent->nodeUnion.statementsNode.body = g_list_append(parent->nodeUnion.statementsNode.body, node);
			}

			if(node->indentation > parent->indentation + 1){
				fprintf(stderr, "Wrong indentation!\n");
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
				node->nodeUnion.ifNode.left = malloc(sizeof(struct Node));
				node->nodeUnion.ifNode.left->indentation = node->indentation;
				node->nodeUnion.ifNode.left->nodeType = STATEMENTS_NODE;
				node->nodeUnion.ifNode.left->nodeUnion.statementsNode.body = NULL;
				node->nodeUnion.ifNode.right = NULL;

				struct Node* tmp = read_file(file, node->nodeUnion.ifNode.left);
				struct Node* tmpDescend = node;

				while(tmp != NULL && tmp->nodeType == ELSE_IF_NODE){
					tmpDescend->nodeUnion.ifNode.right = tmp;
					tmpDescend->nodeUnion.ifNode.right->nodeType = IF_NODE;

					tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left = malloc(sizeof(struct Node));
					tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left->indentation = tmpDescend->indentation;
					tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left->nodeType = STATEMENTS_NODE;
					tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left->nodeUnion.statementsNode.body = NULL;

					tmp = read_file(file, tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left);
					tmpDescend = tmpDescend->nodeUnion.ifNode.right;
				}

				if(tmp != NULL && tmp->nodeType == ELSE_NODE){
					tmpDescend->nodeUnion.ifNode.right = malloc(sizeof(struct Node));
					tmpDescend->nodeUnion.ifNode.right->indentation = node->indentation;
					tmpDescend->nodeUnion.ifNode.right->nodeType = STATEMENTS_NODE;
					tmpDescend->nodeUnion.ifNode.right->nodeUnion.statementsNode.body = NULL;
					free(tmp);
					tmp = read_file(file, tmpDescend->nodeUnion.ifNode.right);
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
				node->nodeUnion.ifNode.left = malloc(sizeof(struct Node));
				node->nodeUnion.ifNode.left->indentation = node->indentation;
				node->nodeUnion.ifNode.left->nodeType = STATEMENTS_NODE;
				node->nodeUnion.ifNode.left->nodeUnion.statementsNode.body = NULL;
				node->nodeUnion.ifNode.right = NULL;

				struct Node* tmp = replRead(node->nodeUnion.ifNode.left, ";");
				struct Node* tmpDescend = node;

				while(tmp != NULL && tmp->nodeType == ELSE_IF_NODE){
					tmpDescend->nodeUnion.ifNode.right = tmp;
					tmpDescend->nodeUnion.ifNode.right->nodeType = IF_NODE;

					tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left = malloc(sizeof(struct Node));
					tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left->indentation = tmpDescend->indentation;
					tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left->nodeType = STATEMENTS_NODE;
					tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left->nodeUnion.statementsNode.body = NULL;

					tmp = replRead(tmpDescend->nodeUnion.ifNode.right->nodeUnion.ifNode.left, ";");
					tmpDescend = tmpDescend->nodeUnion.ifNode.right;
				}

				if(tmp != NULL && tmp->nodeType == ELSE_NODE){
					tmpDescend->nodeUnion.ifNode.right = malloc(sizeof(struct Node));
					tmpDescend->nodeUnion.ifNode.right->indentation = node->indentation;
					tmpDescend->nodeUnion.ifNode.right->nodeType = STATEMENTS_NODE;
					tmpDescend->nodeUnion.ifNode.right->nodeUnion.statementsNode.body = NULL;
					free(tmp);
					tmp = replRead(tmpDescend->nodeUnion.ifNode.right, ";");
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
	} else {

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
	}

	freeNode(program);
		
	g_hash_table_destroy(variables);
	g_hash_table_destroy(functions);
	free(context);
    exit(EXIT_SUCCESS);
}
