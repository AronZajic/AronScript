#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include "interpreter.h"

struct EvalNode lintWithFreshContext(struct Node* node, struct Context *context);

struct EvalNode lint(struct Node* node, struct Context *context){

	if(node == NULL){
		return (struct EvalNode){.evalType=NULL_TYPE};
	}

	if(node->nodeType == VALUE_NODE){
		return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=node->nodeUnion.valueNode.valueType};
	}

	if(node->nodeType == BREAK_NODE){
		return (struct EvalNode){.evalType=BREAK_TYPE};
	}

	if(node->nodeType == CONTINUE_NODE){
		return (struct EvalNode){.evalType=CONTINUE_TYPE};
	}

	if(node->nodeType == VARIABLE_NODE){

		//if(!g_hash_table_contains(context->variables, node->name)){
		if(!contextVariablesContains(context, node->nodeUnion.variableNode.name)){
			fprintf(stderr, "Variable with the name \"%s\" does not exist.\n", node->nodeUnion.variableNode.name);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}
		
		//struct EvalNode *tmp = g_hash_table_lookup(context->variables, node->name);
		//return *tmp;
		struct EvalNode *tmp = contextGetVariable(context, node->nodeUnion.variableNode.name);
		return *tmp;
	}

	if(node->nodeType == NOT_NODE){
		struct EvalNode tmp = lint(node->nodeUnion.notNode.expression, context);

		if(tmp.evalType == NULL_TYPE){
			fprintf(stderr, "Got NULL value as input to not.\n");
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		if(tmp.valueType != BOOLEAN){
			fprintf(stderr, "Got %c type as input to not. Input to not has to be Boolean.\n", tmp.valueType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=BOOLEAN};
	}

	if(node->nodeType == FUNCTION_DECLARATION_NODE){
		if(g_hash_table_contains(context->functions, node->nodeUnion.functionDeclarationNode.name)){
		//if(contextFunctionsContains(context, node->name)){
			fprintf(stderr, "Function %s already defined.\n", node->nodeUnion.functionDeclarationNode.name);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		g_hash_table_insert(context->functions, g_strdup(node->nodeUnion.functionDeclarationNode.name), node);

		GHashTable *contextVariablesInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
		GHashTable *contextFunctionsInFunction = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, freeFunctionDeclaration);

		struct Context contextInFunction;
		contextInFunction.parent = context;
		contextInFunction.variables = contextVariablesInFunction;
		contextInFunction.functions = contextFunctionsInFunction;

		GList *listIteratorDefinition = node->nodeUnion.functionDeclarationNode.arguments;

		while(listIteratorDefinition != NULL){

			struct Node *argumentFunction = (struct Node*)listIteratorDefinition->data;

			struct EvalNode *tmp = malloc(sizeof(struct EvalNode));
			tmp->evalType = VALUE_TYPE;
			tmp->valueType = argumentFunction->nodeUnion.valueNode.valueType;

			g_hash_table_insert(contextInFunction.variables, g_strdup(argumentFunction->nodeUnion.asignDefineNode.name), tmp);

			listIteratorDefinition = listIteratorDefinition->next;
		}

		struct EvalNode tmp = lint(node->nodeUnion.functionDeclarationNode.statements, &contextInFunction);

		g_hash_table_destroy(contextInFunction.variables);
		g_hash_table_destroy(contextInFunction.functions);

		if(tmp.evalType == NULL_TYPE && node->nodeUnion.functionDeclarationNode.retutnType != NULL_TYPE_VALUE){
			fprintf(stderr, "Function %s is returning NULL but has return type of %c.\n", node->nodeUnion.functionDeclarationNode.name, node->nodeUnion.functionDeclarationNode.retutnType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
		}

		if(tmp.evalType == RETURN_TYPE && node->nodeUnion.functionDeclarationNode.retutnType == NULL_TYPE_VALUE){
			fprintf(stderr, "Function %s is returning not NULL but has return type of %c.\n", node->nodeUnion.functionDeclarationNode.name, node->nodeUnion.functionDeclarationNode.retutnType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
		}

		if(tmp.evalType != NULL_TYPE && tmp.valueType != node->nodeUnion.functionDeclarationNode.retutnType){
			fprintf(stderr, "Function %s is returning %c but has return type of %c.\n", node->nodeUnion.functionDeclarationNode.name, tmp.valueType, node->nodeUnion.functionDeclarationNode.retutnType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
		}

		return (struct EvalNode){.evalType=NULL_TYPE};
	}

	if(node->nodeType == FUNCTION_CALL_NODE){

		//if(!g_hash_table_contains(context->functions, node->name)){
		if(!contextFunctionsContains(context, node->nodeUnion.functionCallNode.name)){
			fprintf(stderr, "Function with the name \"%s\" does not exist.\n", node->nodeUnion.functionCallNode.name);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		//struct Node* functionDefinition = g_hash_table_lookup(context->functions, node->name);
		struct Node* functionDefinition = contextGetFunction(context, node->nodeUnion.functionCallNode.name);

		int nodeArgsLen = g_list_length(node->nodeUnion.functionCallNode.arguments);
		int functionDefinitionArgsLen = g_list_length(functionDefinition->nodeUnion.functionDeclarationNode.arguments);

		if(nodeArgsLen != functionDefinitionArgsLen){
			fprintf(stderr, "Wrong number of arguments. Function \"%s\" takes %d argumet(s).\n", node->nodeUnion.functionCallNode.name, functionDefinitionArgsLen);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
		}

		GList *listIteratorNode = node->nodeUnion.functionCallNode.arguments;
		GList *listIteratorDefinition = functionDefinition->nodeUnion.functionDeclarationNode.arguments;

		int i = 0;

		while(listIteratorNode != NULL){

			struct Node *argumentCall = (struct Node*)listIteratorNode->data;
			struct Node *argumentFunction = (struct Node*)listIteratorDefinition->data;

			struct EvalNode *evalValue = malloc(sizeof(struct EvalNode));
			*evalValue = lint(argumentCall, context);

			if(evalValue->evalType == NULL_TYPE){
				fprintf(stderr, "Error, got NULL type as function call argument at position %d.\n", i);
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
				return (struct EvalNode){.evalType=NULL_TYPE};
			}

			if(argumentFunction->nodeUnion.asignDefineNode.valueType != evalValue->valueType){
				fprintf(stderr, "Wrong type of argument, got type %c. Function \"%s\" takes argumet of type %c at postion %d.\n", evalValue->valueType, node->nodeUnion.functionCallNode.name, argumentFunction->nodeUnion.asignDefineNode.valueType, i);
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
				return (struct EvalNode){.evalType=NULL_TYPE};
			}

			listIteratorNode = listIteratorNode->next;
			listIteratorDefinition = listIteratorDefinition->next;
			i++;
		}

		return (struct EvalNode){.evalType=VALUE_TYPE, .valueType=functionDefinition->nodeUnion.functionDeclarationNode.retutnType};
	}

	if(node->nodeType == RETURN_NODE){
		struct EvalNode tmp = lint(node->nodeUnion.returnNode.expression, context);
		return (struct EvalNode){.evalType=RETURN_TYPE, .valueType=tmp.valueType};
	}

	if(node->nodeType == BINARY_OPERATION_NODE){

        struct EvalNode left = lint(node->nodeUnion.binaryOperationNode.left, context);
        struct EvalNode right = lint(node->nodeUnion.binaryOperationNode.right, context);

		if(left.evalType == NULL_TYPE || right.evalType == NULL_TYPE){
            fprintf(stderr, "Left or right side of binary operation is Null. Left is %c. Right is %c.\n", left.evalType, right.evalType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
            return (struct EvalNode){.evalType=NULL_TYPE};
        }

		if(left.evalType != VALUE_TYPE ||  right.evalType != VALUE_TYPE){
            fprintf(stderr, "Left or right side of binary operation is not VALUE type. Left is %c. Right is %c.\n", left.evalType, right.evalType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
            return (struct EvalNode){.evalType=NULL_TYPE};
        }

		if(left.valueType != ZERO && left.valueType != right.valueType){
            fprintf(stderr, "Left and right side of binary operation are not the same type. Left is %c. Right is %c.\n", left.valueType, right.valueType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
            return (struct EvalNode){.evalType=NULL_TYPE};
        }

        struct EvalNode result;

		result.evalType = VALUE_TYPE;

		if(right.valueType == INTEGER){
			result.valueType = INTEGER;
			switch (node->nodeUnion.binaryOperationNode.binaryOperation)
			{
			case PLUS:
			case MINUS:
			case MULTIPLY:
			case DIVIDE:
			case REMAINDER:
				break;
			case LESS_THAN:
			case GREATER_THAN:
			case GREATER_THAN_EQUAL:
			case LESS_THAN_EQUAL:
			case EQUALS:
			case NOT_EQUALS:
				result.valueType = BOOLEAN;
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
				result = (struct EvalNode){.evalType=NULL_TYPE};
			}
		}

		if(right.valueType == DECIMAL){
			result.valueType = DECIMAL;
			switch (node->nodeUnion.binaryOperationNode.binaryOperation)
			{
			case PLUS:
			case MINUS:
			case MULTIPLY:
			case DIVIDE:
				break;
			case LESS_THAN:
			case GREATER_THAN:
			case GREATER_THAN_EQUAL:
			case LESS_THAN_EQUAL:
			case EQUALS:
			case NOT_EQUALS:
				result.valueType = BOOLEAN;
				break;
			case REMAINDER:
				fprintf(stderr, "Decimal types do not support %% operation.\n");
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
				result = (struct EvalNode){.evalType=NULL_TYPE};
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
				result = (struct EvalNode){.evalType=NULL_TYPE};
			}
		}

		if(right.valueType == BOOLEAN){
			result.valueType = BOOLEAN;
			switch (node->nodeUnion.binaryOperationNode.binaryOperation)
			{
			case EQUALS:
			case NOT_EQUALS:
			case AND:
			case OR:
				break;
			case LESS_THAN:
			case GREATER_THAN:
			case GREATER_THAN_EQUAL:
			case LESS_THAN_EQUAL:
			case PLUS:
			case MINUS:
			case MULTIPLY:
			case DIVIDE:
			case REMAINDER:
				fprintf(stderr, "BOOLEAN types do not support <, >, <=, >= and aritmetic operations.\n");
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
				result = (struct EvalNode){.evalType=NULL_TYPE};
				break;
			default:
				fprintf(stderr, "Wrong Binary operation.\n");
				fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
				result = (struct EvalNode){.evalType=NULL_TYPE};
			}
		}
        return result;
	}

	if(node->nodeType == DEFINE_NODE){

		if(g_hash_table_contains(context->variables, node->nodeUnion.asignDefineNode.name)){
		//if(contextVariablesContains(context, node->name)){
			fprintf(stderr, "Variable %s already defined.\n", node->nodeUnion.asignDefineNode.name);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = lint(node->nodeUnion.asignDefineNode.expression, context);

		if(tmpExpression->evalType == NULL_TYPE){
			fprintf(stderr, "Right side of declaration is NULL.\n");
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		if(tmpExpression->valueType != node->nodeUnion.asignDefineNode.valueType){
			fprintf(stderr, "Left and right side of declaration are not the same type. Left is %c. Right is %c.\n", node->nodeUnion.asignDefineNode.valueType, tmpExpression->valueType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		g_hash_table_insert(context->variables, g_strdup(node->nodeUnion.asignDefineNode.name), tmpExpression);
		return (struct EvalNode){.evalType=NULL_TYPE};
	}

	if(node->nodeType == ASIGN_NODE){

		//if(!g_hash_table_contains(context->variables, node->name)){
		if(!contextVariablesContains(context, node->nodeUnion.asignDefineNode.name)){
			fprintf(stderr, "Variable %s not defined.\n", node->nodeUnion.asignDefineNode.name);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		struct EvalNode *tmpExpression = malloc(sizeof(struct EvalNode));
		*tmpExpression = lint(node->nodeUnion.asignDefineNode.expression, context);

		if(tmpExpression->evalType == NULL_TYPE){
			fprintf(stderr, "Right side of asignment is NULL.\n");
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		//struct EvalNode *tmpVariable = g_hash_table_lookup(context->variables, node->name);
		struct EvalNode *tmpVariable = contextGetVariable(context, node->nodeUnion.asignDefineNode.name);

		if(tmpExpression->valueType != tmpVariable->valueType){
			fprintf(stderr, "Left and right side of asignment are not the same type. Left is %c. Right is %c.\n", tmpVariable->valueType, tmpExpression->valueType);
			fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
			return (struct EvalNode){.evalType=NULL_TYPE};
		}

		//g_hash_table_insert(context->variables, g_strdup(node->name), tmpExpression);
		contextInsertVariable(context, node->nodeUnion.asignDefineNode.name, tmpExpression);
		return (struct EvalNode){.evalType=NULL_TYPE};
	}

	if(node->nodeType == WHILE_NODE){
		lint(node->nodeUnion.whileNode.condition, context);
		struct EvalNode evalNodeTmp = lintWithFreshContext(node->nodeUnion.whileNode.statements, context);

		if(evalNodeTmp.evalType == RETURN_TYPE){
			return evalNodeTmp;
		}

		return (struct EvalNode){.evalType=NULL_TYPE};
	}

	if(node->nodeType == IF_NODE){

		lint(node->nodeUnion.ifNode.condition, context);
		struct EvalNode evalNodeTmp = lintWithFreshContext(node->nodeUnion.ifNode.ifBody, context);

		if(evalNodeTmp.evalType == RETURN_TYPE){
			return evalNodeTmp;
		}

		evalNodeTmp = lintWithFreshContext(node->nodeUnion.ifNode.elseBody, context);

		if(evalNodeTmp.evalType == RETURN_TYPE){
			return evalNodeTmp;
		}

		return (struct EvalNode){.evalType=NULL_TYPE};
	}

	if(node->nodeType == STATEMENTS_NODE){
		for (GList *listIterator = node->nodeUnion.statementsNode.body; listIterator != NULL; listIterator = listIterator->next) {
			struct EvalNode evalNodeTmp = lint(listIterator->data, context);
			if(evalNodeTmp.evalType == RETURN_TYPE){
				return evalNodeTmp;
			}
		}
		return (struct EvalNode){.evalType=NULL_TYPE};
	}

	fprintf(stderr, "ERROR! NODE TYPE %c %d NOT IMPLEMENTED!\n", node->nodeType, node->nodeType);
	fprintf(stderr, "At line \"%s\" column %d.\n", node->line, node->column);
	return (struct EvalNode){.evalType=NULL_TYPE};
}

struct EvalNode lintWithFreshContext(struct Node* node, struct Context *context){
	GHashTable *contextVariablesNew = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, free);
	GHashTable *contextFunctionsNew = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, freeFunctionDeclaration);

	struct Context *contextNew = malloc(sizeof(struct Context));
	contextNew->parent = context;
	contextNew->variables = contextVariablesNew;
	contextNew->functions = contextFunctionsNew;

	struct EvalNode evalNodeTmp = lint(node, contextNew);

	g_hash_table_destroy(contextVariablesNew);
	g_hash_table_destroy(contextFunctionsNew);
	free(contextNew);

	return evalNodeTmp;
}