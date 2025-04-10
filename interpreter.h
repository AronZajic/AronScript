#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "parser.h"

enum EvalType {
	VALUE_TYPE = 'V',
	NULL_TYPE = 'N',
	BREAK_TYPE = 'B',
	CONTINUE_TYPE = 'C',
	RETURN_TYPE = 'R'
};

struct EvalNode {
	enum EvalType evalType;
	enum ValueType valueType;
	union Value value;
};

struct Context {
	struct Context *parent;
	GHashTable *variables;
	GHashTable *functions;
};

int contextVariablesContains(struct Context *context, char *name){
	if(g_hash_table_contains(context->variables, name)){
		return 1;
	}
	if(context->parent != NULL){
		return contextVariablesContains(context->parent, name);
	}
	return 0;
}

int contextFunctionsContains(struct Context *context, char *name){
	if(g_hash_table_contains(context->functions, name)){
		return 1;
	}
	if(context->parent != NULL){
		return contextFunctionsContains(context->parent, name);
	}
	return 0;
}

struct EvalNode* contextGetVariable(struct Context *context, char *name){
	if(g_hash_table_contains(context->variables, name)){
		struct EvalNode *tmp = g_hash_table_lookup(context->variables, name);
		return tmp;
	}
	if(context->parent != NULL){
		return contextGetVariable(context->parent, name);
	}
	return NULL;
}

struct Node* contextGetFunction(struct Context *context, char *name){
	if(g_hash_table_contains(context->functions, name)){
		return g_hash_table_lookup(context->functions, name);
	}
	if(context->parent != NULL){
		return contextGetFunction(context->parent, name);
	}
	return NULL;
}

int contextInsertVariable(struct Context *context, char *name, struct EvalNode *node){
	if(g_hash_table_contains(context->variables, name)){
		g_hash_table_insert(context->variables, g_strdup(name), node);
		return 1;
	}
	if(context->parent != NULL){
		return contextInsertVariable(context->parent, name, node);
	}
	return 0;
}

int contextInsertFunction(struct Context *context, char *name, struct Node *node){
	if(g_hash_table_contains(context->functions, name)){
		g_hash_table_insert(context->functions, g_strdup(name), node);
		return 1;
	}
	if(context->parent != NULL){
		return contextInsertFunction(context->parent, name, node);
	}
	return 0;
}

struct EvalNode evalWithFreshContext(struct Node* node, struct Context *context);

struct EvalNode eval(struct Node* node, struct Context *context){

	/*if(node == NULL){
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}*/

	if(node->nodeType == VALUE_NODE){
		return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=node->valueType, .value.intValue=node->value.intValue};
	}

	if(node->nodeType == BREAK_NODE){
		return (struct EvalNode){.evalType=BREAK_TYPE, .value.intValue=0};
	}

	if(node->nodeType == CONTINUE_NODE){
		return (struct EvalNode){.evalType=CONTINUE_TYPE, .value.intValue=0};
	}

	if(node->nodeType == VARIABLE_NODE){

		//if(!g_hash_table_contains(context->variables, node->name)){
		if(!contextVariablesContains(context, node->name)){
			fprintf(stderr, "Variable with the name \"%s\" does not exist.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}
		
		//struct EvalNode *tmp = g_hash_table_lookup(context->variables, node->name);
		//return *tmp;
		struct EvalNode *tmp = contextGetVariable(context, node->name);
		return *tmp;
	}

	if(node->nodeType == NOT_NODE){
		struct EvalNode tmp = eval(node->expression, context);

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
		if(g_hash_table_contains(context->functions, node->name)){
		//if(contextFunctionsContains(context, node->name)){
			fprintf(stderr, "Function %s already defined.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		g_hash_table_insert(context->functions, g_strdup(node->name), node);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == FUNCTION_CALL_NODE){

		//if(!g_hash_table_contains(context->functions, node->name)){
		if(!contextFunctionsContains(context, node->name)){
			fprintf(stderr, "Function with the name \"%s\" does not exist.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		//struct Node* functionDefinition = g_hash_table_lookup(context->functions, node->name);
		struct Node* functionDefinition = contextGetFunction(context, node->name);

		int nodeArgsLen = g_list_length(node->arguments);
		int functionDefinitionArgsLen = g_list_length(functionDefinition->arguments);

		GHashTable *contextVariablesInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
		GHashTable *contextFunctionsInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);

		struct Context contextInFunction;
		contextInFunction.parent = context;
		contextInFunction.variables = contextVariablesInFunction;
		contextInFunction.functions = contextFunctionsInFunction;

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
			*evalValue = eval(argumentCall, context);

			if(evalValue->evalType == NULL_TYPE){
				fprintf(stderr, "Error, got NULL type as function call argument at position %d.\n", i);
			}

			if(argumentFunction->valueType != evalValue->valueType){
				fprintf(stderr, "Wrong type of argument, got type %c. Function \"%s\" takes argumet of type %c at postion %d.\n", evalValue->valueType, node->name, argumentFunction->valueType, i);
			}

			//struct EvalNode *tmp = malloc(sizeof(struct EvalNode));
			//*tmp->evalType = evalValue.evalType;

			g_hash_table_insert(contextInFunction.variables, g_strdup(argumentFunction->name), evalValue);

			listIteratorNode = listIteratorNode->next;
			listIteratorDefinition = listIteratorDefinition->next;
			i++;
		}

		// Todo check for null eval type
		if(strcmp(node->name, "printLine") == 0){
			printf("%d\n", eval((struct Node*)node->arguments->data, context).value.intValue);
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}
		if(strcmp(node->name, "print") == 0){
			printf("%d", eval((struct Node*)node->arguments->data, context).value.intValue);
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		/*for (GList *listIterator = functionDefinition->body; listIterator != NULL; listIterator = listIterator->next) {
			
			struct Node* statement = listIterator->data;
			
			struct EvalNode evalNodeTmp = eval(statement, &contextInFunction);
			if(evalNodeTmp.evalType == RETURN_TYPE){
				g_hash_table_destroy(contextInFunction.variables);
				g_hash_table_destroy(contextInFunction.functions);

				if(evalNodeTmp.valueType != functionDefinition->retutnType){
					fprintf(stderr, "Wrong return type %c. Function \"%s\" return type %c.\n", evalNodeTmp.valueType, node->name, functionDefinition->retutnType);
				}

				return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=functionDefinition->retutnType, .value.intValue=evalNodeTmp.value.intValue};
			}
		}*/

		struct EvalNode evalNodeTmp = eval(functionDefinition->statements, &contextInFunction);
		if(evalNodeTmp.evalType == RETURN_TYPE){
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);

			if(evalNodeTmp.valueType != functionDefinition->retutnType){
				fprintf(stderr, "Wrong return type %c. Function \"%s\" return type %c.\n", evalNodeTmp.valueType, node->name, functionDefinition->retutnType);
			}

			return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=functionDefinition->retutnType, .value.intValue=evalNodeTmp.value.intValue};
		}

		g_hash_table_destroy(contextInFunction.variables);
		g_hash_table_destroy(contextInFunction.functions);

		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == RETURN_NODE){
		struct EvalNode tmp = eval(node->expression, context);
		return (struct EvalNode){.evalType=RETURN_TYPE, .valueType=tmp.valueType, .value.intValue=tmp.value.intValue};
	}

	if(node->nodeType == BINARY_OPERATION_NODE){

        struct EvalNode left = eval(node->left, context);
        struct EvalNode right = eval(node->right, context);

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

		if(g_hash_table_contains(context->variables, node->name)){
		//if(contextVariablesContains(context, node->name)){
			fprintf(stderr, "Variable %s already defined.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->expression, context);

		// TODO check for null eval type

		if(tmpExpression->valueType != node->valueType){
			fprintf(stderr, "Left and right side of declaration are not the same type. Left is %c. Right is %c.\n", node->valueType, tmpExpression->valueType);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		g_hash_table_insert(context->variables, g_strdup(node->name), tmpExpression);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == ASIGN_NODE){

		//if(!g_hash_table_contains(context->variables, node->name)){
		if(!contextVariablesContains(context, node->name)){
			fprintf(stderr, "Variable %s not defined.\n", node->name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->expression, context);

		// TODO check for null eval type

		//struct EvalNode *tmpVariable = g_hash_table_lookup(context->variables, node->name);
		struct EvalNode *tmpVariable = contextGetVariable(context, node->name);

		if(tmpExpression->valueType != tmpVariable->valueType){
			fprintf(stderr, "Left and right side of asignment are not the same type. Left is %c. Right is %c.\n", tmpVariable->valueType, tmpExpression->valueType);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
		}

		//g_hash_table_insert(context->variables, g_strdup(node->name), tmpExpression);
		contextInsertVariable(context, node->name, tmpExpression);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == WHILE_NODE){
		while(eval(node->condition, context).value.intValue){
			struct EvalNode evalNodeTmp;
			/*for (GList *listIterator = node->body; listIterator != NULL; listIterator = listIterator->next) {
				evalNodeTmp = eval(listIterator->data, context);
				if(evalNodeTmp.evalType == BREAK_TYPE){
					break;
				}
				if(evalNodeTmp.evalType == CONTINUE_TYPE){
					break;
				}
				if(evalNodeTmp.evalType == RETURN_TYPE){
					return evalNodeTmp;
				}
			}
			if(evalNodeTmp.evalType == BREAK_TYPE){
				break;
			}
			if(evalNodeTmp.evalType == CONTINUE_TYPE){
				continue;
			}*/

			evalNodeTmp = evalWithFreshContext(node->statements, context);
			//evalNodeTmp = eval(node->statements, context);

			if(evalNodeTmp.evalType == BREAK_TYPE){
				break;
			}
			if(evalNodeTmp.evalType == CONTINUE_TYPE){
				continue;
			}

			if(evalNodeTmp.evalType == RETURN_TYPE){
				return evalNodeTmp;
			}
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == IF_NODE){

		if(eval(node->condition, context).value.intValue){
			/*for (GList *listIterator = node->left->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalNode evalNodeTmp = eval(listIterator->data, context);
				if(evalNodeTmp.evalType == BREAK_TYPE || evalNodeTmp.evalType == CONTINUE_TYPE || evalNodeTmp.evalType == RETURN_TYPE){
					return evalNodeTmp;
				}
			}*/
			//struct EvalNode evalNodeTmp = evalWithFreshContext(node->left, context);
			//return evalNodeTmp;
			return evalWithFreshContext(node->left, context);
		} else if(node->right != NULL) {
			/*for (GList *listIterator = node->right->body; listIterator != NULL; listIterator = listIterator->next) {
				struct EvalNode evalNodeTmp = eval(listIterator->data, context);
				if(evalNodeTmp.evalType == BREAK_TYPE || evalNodeTmp.evalType == CONTINUE_TYPE || evalNodeTmp.evalType == RETURN_TYPE){
					return evalNodeTmp;
				}
			}*/
			//struct EvalNode evalNodeTmp = evalWithFreshContext(node->right, context);
			//return evalNodeTmp;
			return evalWithFreshContext(node->right, context);
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	if(node->nodeType == STATEMENTS_NODE){
		for (GList *listIterator = node->body; listIterator != NULL; listIterator = listIterator->next) {
			struct EvalNode evalNodeTmp = eval(listIterator->data, context);
			if(evalNodeTmp.evalType == BREAK_TYPE || evalNodeTmp.evalType == CONTINUE_TYPE || evalNodeTmp.evalType == RETURN_TYPE){
				return evalNodeTmp;
			}
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
	}

	fprintf(stderr, "ERROR! NODE TYPE %c %d NOT IMPLEMENTED!\n", node->nodeType, node->nodeType);
	return (struct EvalNode){.evalType=NULL_TYPE, .value.intValue=0};
}

struct EvalNode evalWithFreshContext(struct Node* node, struct Context *context){
	GHashTable *contextVariablesInWhile = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
	GHashTable *contextFunctionsInWhile = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);

	struct Context *contextInWhile = malloc(sizeof(struct Context));
	contextInWhile->parent = context;
	contextInWhile->variables = contextVariablesInWhile;
	contextInWhile->functions = contextFunctionsInWhile;

	struct EvalNode evalNodeTmp = eval(node, contextInWhile);

	g_hash_table_destroy(contextVariablesInWhile);
	g_hash_table_destroy(contextFunctionsInWhile);
	free(contextInWhile);

	return evalNodeTmp;
}