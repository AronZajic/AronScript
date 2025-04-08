#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

enum EvalType {
	VALUE_TYPE,
	NULL_TYPE,
	RETURN_TYPE
};

struct EvalNode {
	enum EvalType evalType;
	enum ValueType valueType;
	union Value value;
};


struct EvalNode eval(struct Node* node, GHashTable *contextVariables, GHashTable *contextFunctions){

	if(node->nodeType == VALUE_NODE){
		return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=node->valueType, .value.intValue=node->value.intValue};
	}

	if(node->nodeType == VARIABLE_NODE){

		if(!g_hash_table_contains(contextVariables, node->name)){
			fprintf(stderr, "Variable with the name \"%s\" does not exist.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}
		
		struct EvalNode *tmp = g_hash_table_lookup(contextVariables, node->name);
		return *tmp;
	}

	if(node->nodeType == FUNCTION_DECLARATION_NODE){
		if(g_hash_table_contains(contextFunctions, node->name)){
			fprintf(stderr, "Function %s already defined.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		g_hash_table_insert(contextFunctions, g_strdup(node->name), node);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == FUNCTION_CALL_NODE){

		if(strcmp(node->name, "printline") == 0){
			printf("%d\n", eval(node->argument, contextVariables, contextFunctions).value.intValue);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}
		if(strcmp(node->name, "print") == 0){
			printf("%d", eval(node->argument, contextVariables, contextFunctions).value.intValue);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		if(!g_hash_table_contains(contextFunctions, node->name)){
			fprintf(stderr, "Function with the name \"%s\" does not exist.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		struct Node* functionDefinition = g_hash_table_lookup(contextFunctions, node->name);

		GHashTable *contextVariablesInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);

		if(node->argument != NULL){

			int *tmp = malloc(sizeof(int));
			*tmp = eval(node->argument, contextVariables, contextFunctions).value.intValue;

			g_hash_table_insert(contextVariablesInFunction, g_strdup(functionDefinition->argument->name), tmp);

		}

		for (GList *listIterator = functionDefinition->body; listIterator != NULL; listIterator = listIterator->next) {
			
			struct Node* statement = listIterator->data;
			
			struct EvalNode evalTypeTpm = eval(statement, contextVariablesInFunction, contextFunctions);
			if(evalTypeTpm.evalType == RETURN_TYPE){
				g_hash_table_destroy(contextVariablesInFunction);
				return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=evalTypeTpm.valueType, .value.intValue=evalTypeTpm.value.intValue};
			}
		}

		g_hash_table_destroy(contextVariablesInFunction);

		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == RETURN_NODE){
		int tmp = eval(node->expression, contextVariables, contextFunctions).value.intValue;
		return (struct EvalNode){.evalType=RETURN_TYPE, .value.intValue=tmp};
	}

	if(node->nodeType == BINARY_OPERATION_NODE){

        struct EvalNode left = eval(node->left, contextVariables, contextFunctions);
        struct EvalNode right = eval(node->right, contextVariables, contextFunctions);

        /*if(left.evalType != right.evalType){
            fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.evalType, right.evalType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
        }*/

		if(left.evalType == NULL_TYPE || right.evalType == NULL_TYPE){
            fprintf(stderr, "Left or right side of binary operation is Null. Left is %c. Right is %c.\n", left.evalType, right.evalType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
        }

		if(left.evalType != right.evalType){
            fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.evalType, right.evalType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
        }

		if(left.valueType != right.valueType){
            fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.valueType, right.valueType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
        }

        struct EvalNode result;

		result.evalType = VALUE_TYPE;

		if(left.valueType == INTEGER){
			result.valueType = INTEGER;
			switch (node->binaryOperation)
			{
			case '+':
				result.value.intValue = left.value.intValue + right.value.intValue;
				break;
			case '-':
				result.value.intValue = left.value.intValue - right.value.intValue;
				break;
			case '*':
				result.value.intValue = left.value.intValue * right.value.intValue;
				break;
			case '/':
				result.value.intValue = left.value.intValue / right.value.intValue;
				break;
			case '<':
				result.value.intValue = left.value.intValue < right.value.intValue;
				result.valueType = BOOLEAN;
				break;
			case '>':
				result.value.intValue = left.value.intValue > right.value.intValue;
				result.valueType = BOOLEAN;
				break;
			case 'Q':
				result.value.intValue = left.value.intValue == right.value.intValue;
				result.valueType = BOOLEAN;
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0}; // TODO dopisat sem typ
			}
		}

		if(left.valueType == DECIMAL){
			result.valueType = DECIMAL;
			switch (node->binaryOperation)
			{
			case '+':
				result.value.decimalValue = left.value.decimalValue + right.value.decimalValue;
				break;
			case '-':
				result.value.decimalValue = left.value.decimalValue - right.value.decimalValue;
				break;
			case '*':
				result.value.decimalValue = left.value.decimalValue * right.value.decimalValue;
				break;
			case '/':
				result.value.decimalValue = left.value.decimalValue / right.value.decimalValue;
				break;
			case '<':
				result.value.intValue = left.value.decimalValue < right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case '>':
				result.value.intValue = left.value.decimalValue > right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case 'Q':
				result.value.intValue = left.value.decimalValue == right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
			}
		}

		if(left.valueType == BOOLEAN){
			result.valueType = BOOLEAN;
			switch (node->binaryOperation)
			{
			case '<':
				result.value.intValue = left.value.decimalValue < right.value.decimalValue;
				break;
			case '>':
				result.value.intValue = left.value.decimalValue > right.value.decimalValue;
				break;
			case 'Q':
				result.value.intValue = left.value.intValue == right.value.intValue;
				break;
			case '+':
			case '-':
			case '*':
			case '/':
				fprintf(stderr, "BOOLEAN types do not support eritmetic operations.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
			}
		}
        return result;
	}

	if(node->nodeType == DEFINE_NODE || node->nodeType == ASIGN_NODE){

		if(node->nodeType == DEFINE_NODE && g_hash_table_contains(contextVariables, node->name)){
			fprintf(stderr, "Variable %s already defined.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		struct EvalNode *tmp = malloc(sizeof(struct EvalNode));
		*tmp = eval(node->expression, contextVariables, contextFunctions);

		if(tmp->valueType != node->valueType){
			fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", node->valueType, tmp->evalType);
		}

		g_hash_table_insert(contextVariables, g_strdup(node->name), tmp);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == WHILE_NODE){
		while(eval(node->condition, contextVariables, contextFunctions).value.intValue){
			for (GList *listIterator = node->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalNode evalTypeTpm = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalTypeTpm.evalType == RETURN_TYPE){
					return evalTypeTpm;
				}
			}
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == IF_NODE){

		if (!node) {
			printf("node is NULL\n");
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}
		if (!node->left) {
			printf("node->left is NULL\n");
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}
		if (!node->left->body) {
			printf("node->left->body is NULL\n");
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		if(eval(node->condition, contextVariables, contextFunctions).value.intValue){
			for (GList *listIterator = node->left->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalNode evalTypeTpm = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalTypeTpm.evalType == RETURN_TYPE){
					return evalTypeTpm;
				}
			}
		} else if(node->right != NULL) {
			for (GList *listIterator = node->right->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalNode evalTypeTpm = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalTypeTpm.evalType == RETURN_TYPE){
					return evalTypeTpm;
				}
			}
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	fprintf(stderr, "ERROR! NODE TYPE %c NOT IMPLEMENTED!\n", node->nodeType);
	return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
}