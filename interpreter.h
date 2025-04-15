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

void freeNode(void *n){

	struct Node *node = n;

	if(node == NULL){
		free(node);
		return;
	}

	if(node->nodeType == WHILE_NODE){
		
		if(node->nodeType == WHILE_NODE && node->nodeUnion.whileNode.condition != NULL)
			freeNode(node->nodeUnion.whileNode.condition);

		if(node->nodeUnion.whileNode.statements != NULL){
			freeNode(node->nodeUnion.whileNode.statements);
		}
	}

	if(node->nodeType == STATEMENTS_NODE){
		for (GList *l = node->nodeUnion.statementsNode.body; l != NULL; l = l->next) {
			if(l->data != NULL)
				freeNode(l->data);
		}
		g_list_free(node->nodeUnion.statementsNode.body);
	}

	if(node->nodeType == BINARY_OPERATION_NODE){
		freeNode(node->nodeUnion.binaryOperationNode.left);
		freeNode(node->nodeUnion.binaryOperationNode.right);
	}

	if(node->nodeType == IF_NODE){
		if(node->nodeUnion.ifNode.condition != NULL)
			freeNode(node->nodeUnion.ifNode.condition);
		if(node->nodeUnion.ifNode.left != NULL)
			freeNode(node->nodeUnion.ifNode.left);
		if(node->nodeUnion.ifNode.right != NULL)
			freeNode(node->nodeUnion.ifNode.right);
	}

	if(node->nodeType == DEFINE_NODE || node->nodeType == ASIGN_NODE){
		free(node->nodeUnion.asignDefineNode.name);
	}

	if(node->nodeType == VARIABLE_NODE){
		free(node->nodeUnion.variableNode.name);
	}

	if(node->nodeType == FUNCTION_CALL_NODE){
		free(node->nodeUnion.functionCallNode.name);
	}

	if(node->nodeType == ASIGN_NODE || node->nodeType == DEFINE_NODE){
		freeNode(node->nodeUnion.asignDefineNode.expression);
	}

	if(node->nodeType == FUNCTION_CALL_NODE){
		for (GList *l = node->nodeUnion.functionCallNode.arguments; l != NULL; l = l->next) {
			freeNode(l->data);
		}
		g_list_free(node->nodeUnion.functionCallNode.arguments);
	}

	if(node->nodeType == RETURN_NODE || node->nodeType == NOT_NODE){
		if(node->nodeUnion.returnNode.expression != NULL)
			freeNode(node->nodeUnion.returnNode.expression);
	}

	if(node->nodeType == FUNCTION_DECLARATION_NODE){
		return;
	}

	free(node);
}

void freeFunctionDeclaration(void *n){
	struct Node *node = n;
	if(node->nodeType == FUNCTION_DECLARATION_NODE){
		for (GList *l = node->nodeUnion.functionDeclarationNode.arguments; l != NULL; l = l->next) {
			freeNode(l->data);
		}
		g_list_free(node->nodeUnion.functionDeclarationNode.arguments);
	}
	free(node->nodeUnion.functionDeclarationNode.name);
	free(n);
}

struct EvalNode evalWithFreshContext(struct Node* node, struct Context *context);

struct EvalNode eval(struct Node* node, struct Context *context){

	/*if(node == NULL){
		return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
	}*/

	if(node->nodeType == VALUE_NODE){
		return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=node->nodeUnion.valueNode.valueType, .value.integerValue=node->nodeUnion.valueNode.value.integerValue};
	}

	if(node->nodeType == BREAK_NODE){
		return (struct EvalNode){.evalType=BREAK_TYPE, .value.integerValue=0};
	}

	if(node->nodeType == CONTINUE_NODE){
		return (struct EvalNode){.evalType=CONTINUE_TYPE, .value.integerValue=0};
	}

	if(node->nodeType == VARIABLE_NODE){

		//if(!g_hash_table_contains(context->variables, node->name)){
		if(!contextVariablesContains(context, node->nodeUnion.variableNode.name)){
			fprintf(stderr, "Variable with the name \"%s\" does not exist.\n", node->nodeUnion.variableNode.name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}
		
		//struct EvalNode *tmp = g_hash_table_lookup(context->variables, node->name);
		//return *tmp;
		struct EvalNode *tmp = contextGetVariable(context, node->nodeUnion.variableNode.name);
		return *tmp;
	}

	if(node->nodeType == NOT_NODE){
		struct EvalNode tmp = eval(node->nodeUnion.notNode.expression, context);

		if(tmp.evalType == NULL_TYPE){
			fprintf(stderr, "Got NULL value as input to not.\n");
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		if(tmp.valueType != BOOLEAN){
			fprintf(stderr, "Got %c type as input to not. Input to not has to be Boolean.\n", tmp.valueType);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=BOOLEAN, .value.integerValue=!tmp.value.integerValue};
	}

	if(node->nodeType == FUNCTION_DECLARATION_NODE){
		if(g_hash_table_contains(context->functions, node->nodeUnion.functionDeclarationNode.name)){
		//if(contextFunctionsContains(context, node->name)){
			fprintf(stderr, "Function %s already defined.\n", node->nodeUnion.functionDeclarationNode.name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		g_hash_table_insert(context->functions, g_strdup(node->nodeUnion.functionDeclarationNode.name), node);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
	}

	if(node->nodeType == FUNCTION_CALL_NODE){

		//if(!g_hash_table_contains(context->functions, node->name)){
		if(!contextFunctionsContains(context, node->nodeUnion.functionCallNode.name)){
			fprintf(stderr, "Function with the name \"%s\" does not exist.\n", node->nodeUnion.functionCallNode.name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		//struct Node* functionDefinition = g_hash_table_lookup(context->functions, node->name);
		struct Node* functionDefinition = contextGetFunction(context, node->nodeUnion.functionCallNode.name);

		int nodeArgsLen = g_list_length(node->nodeUnion.functionCallNode.arguments);
		int functionDefinitionArgsLen = g_list_length(functionDefinition->nodeUnion.functionDeclarationNode.arguments);

		GHashTable *contextVariablesInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
		GHashTable *contextFunctionsInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, freeFunctionDeclaration);

		struct Context contextInFunction;
		contextInFunction.parent = context;
		contextInFunction.variables = contextVariablesInFunction;
		contextInFunction.functions = contextFunctionsInFunction;

		if(nodeArgsLen != functionDefinitionArgsLen){
			fprintf(stderr, "Wrong number of arguments. Function \"%s\" takes %d argumet(s).\n", node->nodeUnion.functionCallNode.name, functionDefinitionArgsLen);
		}

		GList *listIteratorNode = node->nodeUnion.functionCallNode.arguments;
		GList *listIteratorDefinition = functionDefinition->nodeUnion.functionDeclarationNode.arguments;

		int i = 0;

		while(listIteratorNode != NULL){

			struct Node *argumentCall = (struct Node*)listIteratorNode->data;
			struct Node *argumentFunction = (struct Node*)listIteratorDefinition->data;

			struct EvalNode *evalValue = malloc(sizeof(struct EvalNode));
			*evalValue = eval(argumentCall, context);

			if(evalValue->evalType == NULL_TYPE){
				fprintf(stderr, "Error, got NULL type as function call argument at position %d.\n", i);
			}

			if(argumentFunction->nodeUnion.asignDefineNode.valueType != evalValue->valueType){
				fprintf(stderr, "Wrong type of argument, got type %c. Function \"%s\" takes argumet of type %c at postion %d.\n", evalValue->valueType, node->nodeUnion.functionCallNode.name, argumentFunction->nodeUnion.asignDefineNode.valueType, i);
			}

			//struct EvalNode *tmp = malloc(sizeof(struct EvalNode));
			//*tmp->evalType = evalValue.evalType;

			g_hash_table_insert(contextInFunction.variables, g_strdup(argumentFunction->nodeUnion.asignDefineNode.name), evalValue);

			listIteratorNode = listIteratorNode->next;
			listIteratorDefinition = listIteratorDefinition->next;
			i++;
		}

		// Todo check for null eval type
		if(strcmp(node->nodeUnion.functionCallNode.name, "printLineInteger") == 0){
			printf("%d\n", eval((struct Node*)node->nodeUnion.functionCallNode.arguments->data, context).value.integerValue);
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}
		if(strcmp(node->nodeUnion.functionCallNode.name, "printInteger") == 0){
			printf("%d", eval((struct Node*)node->nodeUnion.functionCallNode.arguments->data, context).value.integerValue);
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		if(strcmp(node->nodeUnion.functionCallNode.name, "printLineDecimal") == 0){
			printf("%f\n", eval((struct Node*)node->nodeUnion.functionCallNode.arguments->data, context).value.decimalValue);
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}
		if(strcmp(node->nodeUnion.functionCallNode.name, "printDecimal") == 0){
			printf("%f", eval((struct Node*)node->nodeUnion.functionCallNode.arguments->data, context).value.decimalValue);
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		if(strcmp(node->nodeUnion.functionCallNode.name, "printLineBoolean") == 0){
			if(eval((struct Node*)node->nodeUnion.functionCallNode.arguments->data, context).value.integerValue){
				printf("True\n");
			} else {
				printf("False\n");
			}
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}
		if(strcmp(node->nodeUnion.functionCallNode.name, "printBoolean") == 0){
			if(eval((struct Node*)node->nodeUnion.functionCallNode.arguments->data, context).value.integerValue){
				printf("True");
			} else {
				printf("False");
			}
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		struct EvalNode evalNodeTmp = eval(functionDefinition->nodeUnion.functionDeclarationNode.statements, &contextInFunction);
		if(evalNodeTmp.evalType == RETURN_TYPE){
			g_hash_table_destroy(contextInFunction.variables);
			g_hash_table_destroy(contextInFunction.functions);

			if(evalNodeTmp.valueType != functionDefinition->nodeUnion.functionDeclarationNode.retutnType){
				fprintf(stderr, "Wrong return type %c. Function \"%s\" return type %c.\n", evalNodeTmp.valueType, node->nodeUnion.functionCallNode.name, functionDefinition->nodeUnion.functionDeclarationNode.retutnType);
			}

			return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=functionDefinition->nodeUnion.functionDeclarationNode.retutnType, .value.integerValue=evalNodeTmp.value.integerValue};
		}

		g_hash_table_destroy(contextInFunction.variables);
		g_hash_table_destroy(contextInFunction.functions);

		return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
	}

	if(node->nodeType == RETURN_NODE){
		struct EvalNode tmp = eval(node->nodeUnion.returnNode.expression, context);
		return (struct EvalNode){.evalType=RETURN_TYPE, .valueType=tmp.valueType, .value.integerValue=tmp.value.integerValue};
	}

	if(node->nodeType == BINARY_OPERATION_NODE){

        struct EvalNode left = eval(node->nodeUnion.binaryOperationNode.left, context);
        struct EvalNode right = eval(node->nodeUnion.binaryOperationNode.right, context);

        /*if(left.evalType != right.evalType){
            fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.evalType, right.evalType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
        }*/

		if(left.evalType == NULL_TYPE || right.evalType == NULL_TYPE){
            fprintf(stderr, "Left or right side of binary operation is Null. Left is %c. Right is %c.\n", left.evalType, right.evalType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
        }

		if(left.evalType != VALUE_TYPE ||  right.evalType != VALUE_TYPE){
            fprintf(stderr, "Left or right side of binary operation is not VALUE type. Left is %c. Right is %c.\n", left.evalType, right.evalType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
        }

		if(left.valueType != ZERO && left.valueType != right.valueType){
            fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.valueType, right.valueType);
            return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
        }

        struct EvalNode result;

		result.evalType = VALUE_TYPE;

		if(right.valueType == INTEGER){
			result.valueType = INTEGER;
			switch (node->nodeUnion.binaryOperationNode.binaryOperation)
			{
			case '+':
				result.value.integerValue = left.value.integerValue + right.value.integerValue;
				break;
			case '-':
				result.value.integerValue = left.value.integerValue - right.value.integerValue;
				break;
			case '*':
				result.value.integerValue = left.value.integerValue * right.value.integerValue;
				break;
			case '/':
				result.value.integerValue = left.value.integerValue / right.value.integerValue;
				break;
			case '<':
				result.value.integerValue = left.value.integerValue < right.value.integerValue;
				result.valueType = BOOLEAN;
				break;
			case '>':
				result.value.integerValue = left.value.integerValue > right.value.integerValue;
				result.valueType = BOOLEAN;
				break;
			case '7':
				result.value.integerValue = left.value.integerValue >= right.value.integerValue;
				result.valueType = BOOLEAN;
				break;
			case '4':
				result.value.integerValue = left.value.integerValue <= right.value.integerValue;
				result.valueType = BOOLEAN;
				break;
			case 'Q':
				result.value.integerValue = left.value.integerValue == right.value.integerValue;
				result.valueType = BOOLEAN;
				break;
			case '!':
				result.value.integerValue = left.value.integerValue != right.value.integerValue;
				result.valueType = BOOLEAN;
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
			}
		}

		if(right.valueType == DECIMAL){
			result.valueType = DECIMAL;
			switch (node->nodeUnion.binaryOperationNode.binaryOperation)
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
				result.value.integerValue = left.value.decimalValue < right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case '>':
				result.value.integerValue = left.value.decimalValue > right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case '7':
				result.value.integerValue = left.value.decimalValue >= right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case '4':
				result.value.integerValue = left.value.decimalValue <= right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case 'Q':
				result.value.integerValue = left.value.decimalValue == right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			case '!':
				result.value.integerValue = left.value.decimalValue != right.value.decimalValue;
				result.valueType = BOOLEAN;
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
			}
		}

		if(right.valueType == BOOLEAN){
			result.valueType = BOOLEAN;
			switch (node->nodeUnion.binaryOperationNode.binaryOperation)
			{
			case 'Q':
				result.value.integerValue = left.value.integerValue == right.value.integerValue;
				break;
			case '!':
				result.value.integerValue = left.value.integerValue == right.value.integerValue;
				break;
			case '&':
				result.value.integerValue = left.value.integerValue && right.value.integerValue;
				break;
			case '|':
				result.value.integerValue = left.value.integerValue || right.value.integerValue;
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
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				result = (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
			}
		}
        return result;
	}

	if(node->nodeType == DEFINE_NODE){

		if(g_hash_table_contains(context->variables, node->nodeUnion.asignDefineNode.name)){
		//if(contextVariablesContains(context, node->name)){
			fprintf(stderr, "Variable %s already defined.\n", node->nodeUnion.asignDefineNode.name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->nodeUnion.asignDefineNode.expression, context);

		// TODO check for null eval type

		if(tmpExpression->valueType != node->nodeUnion.asignDefineNode.valueType){
			fprintf(stderr, "Left and right side of declaration are not the same type. Left is %c. Right is %c.\n", node->nodeUnion.asignDefineNode.valueType, tmpExpression->valueType);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		g_hash_table_insert(context->variables, g_strdup(node->nodeUnion.asignDefineNode.name), tmpExpression);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
	}

	if(node->nodeType == ASIGN_NODE){

		//if(!g_hash_table_contains(context->variables, node->name)){
		if(!contextVariablesContains(context, node->nodeUnion.asignDefineNode.name)){
			fprintf(stderr, "Variable %s not defined.\n", node->nodeUnion.asignDefineNode.name);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = eval(node->nodeUnion.asignDefineNode.expression, context);

		// TODO check for null eval type

		//struct EvalNode *tmpVariable = g_hash_table_lookup(context->variables, node->name);
		struct EvalNode *tmpVariable = contextGetVariable(context, node->nodeUnion.asignDefineNode.name);

		if(tmpExpression->valueType != tmpVariable->valueType){
			fprintf(stderr, "Left and right side of asignment are not the same type. Left is %c. Right is %c.\n", tmpVariable->valueType, tmpExpression->valueType);
			return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
		}

		//g_hash_table_insert(context->variables, g_strdup(node->name), tmpExpression);
		contextInsertVariable(context, node->nodeUnion.asignDefineNode.name, tmpExpression);
		return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
	}

	if(node->nodeType == WHILE_NODE){
		while(eval(node->nodeUnion.whileNode.condition, context).value.integerValue){
			struct EvalNode evalNodeTmp;

			evalNodeTmp = evalWithFreshContext(node->nodeUnion.whileNode.statements, context);
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
		return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
	}

	if(node->nodeType == IF_NODE){

		if(eval(node->nodeUnion.ifNode.condition, context).value.integerValue){
			//struct EvalNode evalNodeTmp = evalWithFreshContext(node->left, context);
			//return evalNodeTmp;
			return evalWithFreshContext(node->nodeUnion.ifNode.left, context);
		} else if(node->nodeUnion.ifNode.right != NULL) {
			//struct EvalNode evalNodeTmp = evalWithFreshContext(node->right, context);
			//return evalNodeTmp;
			return evalWithFreshContext(node->nodeUnion.ifNode.right, context);
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
	}

	if(node->nodeType == STATEMENTS_NODE){
		for (GList *listIterator = node->nodeUnion.statementsNode.body; listIterator != NULL; listIterator = listIterator->next) {
			struct EvalNode evalNodeTmp = eval(listIterator->data, context);
			if(evalNodeTmp.evalType == BREAK_TYPE || evalNodeTmp.evalType == CONTINUE_TYPE || evalNodeTmp.evalType == RETURN_TYPE){
				return evalNodeTmp;
			}
		}
		return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
	}

	fprintf(stderr, "ERROR! NODE TYPE %c %d NOT IMPLEMENTED!\n", node->nodeType, node->nodeType);
	return (struct EvalNode){.evalType=NULL_TYPE, .value.integerValue=0};
}

struct EvalNode evalWithFreshContext(struct Node* node, struct Context *context){
	GHashTable *contextVariablesNew = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
	GHashTable *contextFunctionsNew = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, freeFunctionDeclaration);

	struct Context *contextNew = malloc(sizeof(struct Context));
	contextNew->parent = context;
	contextNew->variables = contextVariablesNew;
	contextNew->functions = contextFunctionsNew;

	struct EvalNode evalNodeTmp = eval(node, contextNew);

	g_hash_table_destroy(contextVariablesNew);
	g_hash_table_destroy(contextFunctionsNew);
	free(contextNew);

	return evalNodeTmp;
}