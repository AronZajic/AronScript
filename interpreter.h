#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

enum EvalType {
	NULL_TYPE,
	INTEGER_TYPE,
	RETURN_TYPE
};

struct EvalValue {
	enum EvalType evalType;
	int value;
};

struct EvalValue eval(struct Node* node, GHashTable *contextVariables, GHashTable *contextFunctions){

	if(node->nodeType == NUMBER_NODE){
		return (struct EvalValue){INTEGER_TYPE, node->number};
	}

	if(node->nodeType == VARIABLE_NODE){

		if(!g_hash_table_contains(contextVariables, node->name)){
			fprintf(stderr, "Variable with the name \"%s\" does not exist.\n", node->name);
			return (struct EvalValue){NULL_TYPE, 0};
		}
		
		int *tmp = (g_hash_table_lookup(contextVariables, node->name));
		return (struct EvalValue){INTEGER_TYPE, *tmp};
	}

	if(node->nodeType == FUNCTION_DECLARATION_NODE){
		if(g_hash_table_contains(contextFunctions, node->name)){
			fprintf(stderr, "Function %s already defined.\n", node->name);
			return (struct EvalValue){NULL_TYPE, 0};
		}

		g_hash_table_insert(contextFunctions, g_strdup(node->name), node);
		return (struct EvalValue){NULL_TYPE, 0};
	}

	if(node->nodeType == FUNCTION_CALL_NODE){

		if(strcmp(node->name, "printline") == 0){
			printf("%d\n", eval(node->argument, contextVariables, contextFunctions).value);
			return (struct EvalValue){NULL_TYPE, 0};
		}
		if(strcmp(node->name, "print") == 0){
			printf("%d", eval(node->argument, contextVariables, contextFunctions).value);
			return (struct EvalValue){NULL_TYPE, 0};
		}

		if(!g_hash_table_contains(contextFunctions, node->name)){
			fprintf(stderr, "Function with the name \"%s\" does not exist.\n", node->name);
			return (struct EvalValue){NULL_TYPE, 0};
		}

		struct Node* functionDefinition = g_hash_table_lookup(contextFunctions, node->name);

		GHashTable *contextVariablesInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);

		if(node->argument != NULL){

			int *tmp = malloc(sizeof(int));
			*tmp = eval(node->argument, contextVariables, contextFunctions).value;

			g_hash_table_insert(contextVariablesInFunction, g_strdup(functionDefinition->argument->name), tmp);

		}

		for (GList *listIterator = functionDefinition->body; listIterator != NULL; listIterator = listIterator->next) {
			
			struct Node* statement = listIterator->data;
			
			struct EvalValue evalTypeTpm = eval(statement, contextVariablesInFunction, contextFunctions);
			if(evalTypeTpm.evalType == RETURN_TYPE){
				g_hash_table_destroy(contextVariablesInFunction);
				return (struct EvalValue){INTEGER_TYPE, evalTypeTpm.value};
			}
		}

		g_hash_table_destroy(contextVariablesInFunction);

		return (struct EvalValue){NULL_TYPE, 0};
	}

	if(node->nodeType == RETURN_NODE){
		int tmp = eval(node->expression, contextVariables, contextFunctions).value;
		return (struct EvalValue){RETURN_TYPE, tmp};
	}

	if(node->nodeType == BINARY_OPERATION_NODE){

        struct EvalValue left = eval(node->left, contextVariables, contextFunctions);
        struct EvalValue right = eval(node->right, contextVariables, contextFunctions);

        if(left.evalType != right.evalType){
            fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.evalType, right.evalType);
            return (struct EvalValue){NULL_TYPE, 0};
        }

        struct EvalValue result;

		switch (node->binaryOperation)
		{
		case '+':
			result = (struct EvalValue){INTEGER_TYPE, left.value + right.value};
            break;
		case '-':
            result = (struct EvalValue){INTEGER_TYPE, left.value - right.value};
            break;
		case '*':
            result = (struct EvalValue){INTEGER_TYPE, left.value * right.value};
            break;
		case '/':
            result = (struct EvalValue){INTEGER_TYPE, left.value / right.value};
            break;
		case '<':
            result = (struct EvalValue){INTEGER_TYPE, left.value < right.value};
            break;
		case '>':
            result = (struct EvalValue){INTEGER_TYPE, left.value > right.value};
            break;
		default:
			fprintf(stderr, "Wrong Binary operation.\n");
			result = (struct EvalValue){NULL_TYPE, 0};
		}

        return result;
	}

	if(node->nodeType == DEFINE_NODE || node->nodeType == ASIGN_NODE){

		if(node->nodeType == DEFINE_NODE && g_hash_table_contains(contextVariables, node->name)){
			fprintf(stderr, "Variable %s already defined.\n", node->name);
			return (struct EvalValue){NULL_TYPE, 0};
		}

		int *tmp = malloc(sizeof(int));
		*tmp = eval(node->expression, contextVariables, contextFunctions).value;
		g_hash_table_insert(contextVariables, g_strdup(node->name), tmp);
		return (struct EvalValue){NULL_TYPE, 0};
	}

	if(node->nodeType == WHILE_NODE){
		while(eval(node->condition, contextVariables, contextFunctions).value){
			for (GList *listIterator = node->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalValue evalTypeTpm = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalTypeTpm.evalType == RETURN_TYPE){
					return evalTypeTpm;
				}
			}
		}
		return (struct EvalValue){NULL_TYPE, 0};
	}

	if(node->nodeType == IF_NODE){
		if(eval(node->condition, contextVariables, contextFunctions).value){
			for (GList *listIterator = node->left->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalValue evalTypeTpm = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalTypeTpm.evalType == RETURN_TYPE){
					return evalTypeTpm;
				}
			}
		} else if(node->right != NULL) {
			for (GList *listIterator = node->right->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalValue evalTypeTpm = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalTypeTpm.evalType == RETURN_TYPE){
					return evalTypeTpm;
				}
			}
		}
		return (struct EvalValue){NULL_TYPE, 0};
	}

	fprintf(stderr, "ERROR! NODE TYPE %c NOT IMPLEMENTED!\n", node->nodeType);
}