#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

enum EvalType {
	VALUE_TYPE = 'V',
	NULL_TYPE = 'N',
	RETURN_TYPE = 'R'
};

struct EvalNode {
	enum EvalType evalType;
	enum ValueType valueType;
	union Value value;
};


struct EvalNode eval(struct Node* node, GHashTable *contextVariables, GHashTable *contextFunctions){

	/*if(node == NULL){
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}*/

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

	if(node->nodeType == NOT_NODE){
		struct EvalNode tmp = eval(node->expression, contextVariables, contextFunctions);

		if(tmp.evalType == NULL_TYPE){
			fprintf(stderr, "Got NULL value as input to not.\n");
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		if(tmp.valueType != BOOLEAN){
			fprintf(stderr, "Got %c type as input to not. Input to not has to be Boolean.\n", tmp.valueType);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=BOOLEAN, .value.intValue=!tmp.value.intValue};
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

		if(!g_hash_table_contains(contextFunctions, node->name)){
			fprintf(stderr, "Function with the name \"%s\" does not exist.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		struct Node* functionDefinition = g_hash_table_lookup(contextFunctions, node->name);

		int nodeArgsLen = g_list_length(node->arguments);
		int functionDefinitionArgsLen = g_list_length(functionDefinition->arguments);

		GHashTable *contextVariablesInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);

		if(nodeArgsLen != functionDefinitionArgsLen){
			fprintf(stderr, "Wrong number of arguments. Function \"%s\" takes %d argumet(s).\n", node->name, functionDefinitionArgsLen);
		}

		GList *listIteratorNode = node->arguments;
		GList *listIteratorDefinition = functionDefinition->arguments;

		int i = 0;

		while(listIteratorNode != NULL){

			struct Node *argumentCall = (struct Node*)listIteratorNode->data;
			struct Node *argumentFunction = (struct Node*)listIteratorDefinition->data;

			struct EvalNode *evalValue = malloc(sizeof(struct EvalNode));
			*evalValue = eval(argumentCall, contextVariables, contextFunctions);

			if(evalValue->evalType == NULL_TYPE){
				fprintf(stderr, "Error, got NULL type as function call argument at position %d.\n", i);
			}

			if(argumentFunction->valueType != evalValue->valueType){
				fprintf(stderr, "Wrong type of argument, got type %c. Function \"%s\" takes argumet of type %c at postion %d.\n", evalValue->valueType, node->name, argumentFunction->valueType, i);
			}

			//struct EvalNode *tmp = malloc(sizeof(struct EvalNode));
			//*tmp->evalType = evalValue.evalType;

			g_hash_table_insert(contextVariablesInFunction, g_strdup(argumentFunction->name), evalValue);

			listIteratorNode = listIteratorNode->next;
			listIteratorDefinition = listIteratorDefinition->next;
			i++;
		}

		if(strcmp(node->name, "printLine") == 0){
			printf("%d\n", eval((struct Node*)node->arguments->data, contextVariables, contextFunctions).value.intValue);
			g_hash_table_destroy(contextVariablesInFunction);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}
		if(strcmp(node->name, "print") == 0){
			printf("%d", eval((struct Node*)node->arguments->data, contextVariables, contextFunctions).value.intValue);
			g_hash_table_destroy(contextVariablesInFunction);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		for (GList *listIterator = functionDefinition->body; listIterator != NULL; listIterator = listIterator->next) {
			
			struct Node* statement = listIterator->data;
			
			struct EvalNode evalNodeTmp = eval(statement, contextVariablesInFunction, contextFunctions);
			if(evalNodeTmp.evalType == RETURN_TYPE){
				g_hash_table_destroy(contextVariablesInFunction);

				if(evalNodeTmp.valueType != functionDefinition->retutnType){
					fprintf(stderr, "Wrong return type %c. Function \"%s\" return type %c.\n", evalNodeTmp.valueType, node->name, functionDefinition->retutnType);
				}

				return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=functionDefinition->retutnType, .value.intValue=evalNodeTmp.value.intValue};
			}
		}

		g_hash_table_destroy(contextVariablesInFunction);

		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == RETURN_NODE){
		struct EvalNode tmp = eval(node->expression, contextVariables, contextFunctions);
		return (struct EvalNode){.evalType=RETURN_TYPE, .valueType=tmp.valueType, .value.intValue=tmp.value.intValue};
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
            fprintf(stderr, "ELeft and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.evalType, right.evalType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
        }

		if(left.valueType != right.valueType){
            fprintf(stderr, "VLeft and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.valueType, right.valueType);
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
			case '7':
				result.value.intValue = left.value.intValue >= right.value.intValue;
				result.valueType = BOOLEAN;
				break;
			case '4':
				result.value.intValue = left.value.intValue <= right.value.intValue;
				result.valueType = BOOLEAN;
				break;
			case 'Q':
				result.value.intValue = left.value.intValue == right.value.intValue;
				result.valueType = BOOLEAN;
				break;
			case '!':
				result.value.intValue = left.value.intValue != right.value.intValue;
				result.valueType = BOOLEAN;
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
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
			case '7':
				result.value.intValue = left.value.decimalValue >= right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case '4':
				result.value.intValue = left.value.decimalValue <= right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case 'Q':
				result.value.intValue = left.value.decimalValue == right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case '!':
				result.value.intValue = left.value.decimalValue != right.value.decimalValue;
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
			case 'Q':
				result.value.intValue = left.value.intValue == right.value.intValue;
				break;
			case '!':
				result.value.intValue = left.value.intValue == right.value.intValue;
				break;
			case '&':
				result.value.intValue = left.value.intValue && right.value.intValue;
				break;
			case '|':
				result.value.intValue = left.value.intValue || right.value.intValue;
				break;
			case '<':
			case '>':
			case '7':
			case '4':
			case '+':
			case '-':
			case '*':
			case '/':
				fprintf(stderr, "BOOLEAN types do not support <, >, <=, >= and aritmetic operations.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
			}
		}
        return result;
	}

	if(node->nodeType == DEFINE_NODE){

		if(g_hash_table_contains(contextVariables, node->name)){
			fprintf(stderr, "Variable %s already defined.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->expression, contextVariables, contextFunctions);

		if(tmpExpression->valueType != node->valueType){
			fprintf(stderr, "Left and right side of declaration are not the same type. Left is %c. Right is %c.\n", node->valueType, tmpExpression->valueType);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		g_hash_table_insert(contextVariables, g_strdup(node->name), tmpExpression);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == ASIGN_NODE){

		if(!g_hash_table_contains(contextVariables, node->name)){
			fprintf(stderr, "Variable %s not defined.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->expression, contextVariables, contextFunctions);

		struct EvalNode *tmpVariable = g_hash_table_lookup(contextVariables, node->name);

		if(tmpExpression->valueType != tmpVariable->valueType){
			fprintf(stderr, "Left and right side of asignment are not the same type. Left is %c. Right is %c.\n", tmpVariable->valueType, tmpExpression->valueType);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		g_hash_table_insert(contextVariables, g_strdup(node->name), tmpExpression);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == WHILE_NODE){
		while(eval(node->condition, contextVariables, contextFunctions).value.intValue){
			for (GList *listIterator = node->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalNode evalNodeTmp = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalNodeTmp.evalType == RETURN_TYPE){
					return evalNodeTmp;
				}
			}
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == IF_NODE){

		if(eval(node->condition, contextVariables, contextFunctions).value.intValue){
			for (GList *listIterator = node->left->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalNode evalNodeTmp = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalNodeTmp.evalType == RETURN_TYPE){
					return evalNodeTmp;
				}
			}
		} else if(node->right != NULL) {
			for (GList *listIterator = node->right->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalNode evalNodeTmp = eval(listIterator->data, contextVariables, contextFunctions);
				if(evalNodeTmp.evalType == RETURN_TYPE){
					return evalNodeTmp;
				}
			}
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	fprintf(stderr, "ERROR! NODE TYPE %c %d NOT IMPLEMENTED!\n", node->nodeType, node->nodeType);
	return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
}