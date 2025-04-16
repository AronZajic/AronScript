#include "tokenizer.h"
#include "parser_types.h"

struct Token tokens[128];

int token_i_parser = 0;

int runningAsREPL;

struct Token peek(){
    return tokens[token_i_parser];
}

struct Token eat(){
    struct Token token = tokens[token_i_parser];
    token_i_parser++;
    return token; 
}

char* getValueTypeString(enum ValueType valueType){
    switch (valueType)
    {
    case INTEGER:
        return "INTEGER";
    case DECIMAL:
        return "DECIMAL";
    case BOOLEAN:
        return "BOOLEAN";
    case ZERO:
        return "ZERO";
    case NULL_TYPE_VALUE:
        return "NULL_TYPE";
    default:
        return NULL;
    }
}

void wrongTokenPrint(char expected[], struct Token token){
	fprintf(stderr, "\nWrong token! %s token expected, got %s.\n", expected, getTokenString(token.tokenType));
    fprintf(stderr, "At line \"%s\" column %d.\n", token.line, token.column);
    if(!runningAsREPL){
        exit(EXIT_FAILURE);
    }
}

struct Node* colon(){
    struct Token token = eat();

    if(token.tokenType != COLON_TOKEN){
        wrongTokenPrint("COLON", token);
        return NULL;
    }

    return NULL;
}

struct Node* integerValue() {

    struct Token token = eat();

    if(token.tokenType != INTEGER_VALUE_TOKEN){
        wrongTokenPrint("INTEGER VALUE", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = VALUE_NODE;
    node->nodeUnion.valueNode.valueType = INTEGER;
    node->nodeUnion.valueNode.value.integerValue = atoi(token.value);

    node->line = token.line;
    node->column = token.column;
    return node;
}

struct Node* breakNode() {

    struct Token token = eat();

    if(token.tokenType != BREAK_TOKEN){
        wrongTokenPrint("BREAK", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = BREAK_NODE;

    node->line = token.line;
    node->column = token.column;
    return node;
}

struct Node* continueNode() {

    struct Token token = eat();

    if(token.tokenType != CONTINUE_TOKEN){
        wrongTokenPrint("CONTINUE", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = CONTINUE_NODE;

    node->line = token.line;
    node->column = token.column;
    return node;
}

struct Node* dot(){
    struct Token token = eat();

    if(token.tokenType != DOT_TOKEN){
        wrongTokenPrint("DOT", token);
        return NULL;
    }

    return NULL;
}

struct Node* decimalValue() {

    struct Token token = eat();

    if(token.tokenType != DECIMAL_VALUE_TOKEN){
        wrongTokenPrint("DECIMAL_VALUE", token);
        return NULL;
    }

    dot();

    struct Token token2 = eat();

    if(token2.tokenType != INTEGER_VALUE_TOKEN){
        wrongTokenPrint("INTEGER_VALUE", token2);
        return NULL;
    }

    char tmp[128];
    sprintf(tmp, "%s.%s", token.value, token2.value); 

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = VALUE_NODE;
    node->nodeUnion.valueNode.valueType = DECIMAL;
    node->nodeUnion.valueNode.value.decimalValue = atof(tmp);

    node->line = token.line;
    node->column = token.column;
    return node;
}

struct Node* booleanValue() {

    struct Token token = eat();

    if(token.tokenType != TRUE_TOKEN && token.tokenType != FALSE_TOKEN){
        wrongTokenPrint("BOOLEAN", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = VALUE_NODE;
    node->nodeUnion.valueNode.valueType = BOOLEAN;

    if(token.tokenType == TRUE_TOKEN){
        node->nodeUnion.valueNode.value.integerValue = 1;
    } else {
        node->nodeUnion.valueNode.value.integerValue = 0;
    }

    node->line = token.line;
    node->column = token.column;
    return node;
}

struct Node* variable() {

    struct Token token = eat();

    if(token.tokenType != NAME_TOKEN){
        wrongTokenPrint("NAME", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = VARIABLE_NODE;

    node->nodeUnion.variableNode.name = g_strdup(token.value);

    node->line = token.line;
    node->column = token.column;
    return node;
}

struct Node* binaryOperation() {

    struct Token token = eat();

    if(token.tokenType != BINARY_OPERATION_TOKEN){
        wrongTokenPrint("BINARY_OPERATION", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = BINARY_OPERATION_NODE;
    node->nodeUnion.binaryOperationNode.binaryOperation = token.value[0];
    node->nodeUnion.binaryOperationNode.presedence = token.presedence;

    node->line = token.line;
    node->column = token.column;
    return node;

}

struct Node* leftP() {

    struct Token token = eat();

    if(token.tokenType != LEFT_P_TOKEN){
        wrongTokenPrint("LEFT_P", token);
    }

    return NULL;
}

struct Node* rightP() {

    struct Token token = eat();

    if(token.tokenType != RIGHT_P_TOKEN){
        wrongTokenPrint("RIGHT_P", token);
    }

    return NULL;
}

struct Node* functionCall();
struct Node* expression();

struct Node* negation() {

    struct Token token = eat();

    if(token.tokenType != NOT_TOKEN){
        wrongTokenPrint("NOT", token);
    }

    struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = NOT_NODE;

    switch (peek().tokenType)
    {
    case INTEGER_VALUE_TOKEN:
        fprintf(stderr, "Integer value can not be negated.");
        fprintf(stderr, "At line \"%s\" column %d.\n", peek().line, peek().column);
        if(!runningAsREPL){
            exit(EXIT_FAILURE);
        }
        return NULL;
    case DECIMAL_VALUE_TOKEN:
        fprintf(stderr, "Decimal value can not be negated.");
        fprintf(stderr, "At line \"%s\" column %d.\n", peek().line, peek().column);
        if(!runningAsREPL){
            exit(EXIT_FAILURE);
        }
        return NULL;
    case TRUE_TOKEN:
    case FALSE_TOKEN:
        e->nodeUnion.notNode.expression = booleanValue();
        break;
    case NAME_TOKEN:
        e->nodeUnion.notNode.expression = variable();
        break;
    case FUNCTION_CALL_TOKEN:
        e->nodeUnion.notNode.expression = functionCall();
		break;
    case LEFT_P_TOKEN:
        leftP();
        e->nodeUnion.notNode.expression = expression();
        rightP();
        break;
    case NOT_TOKEN:
        e->nodeUnion.notNode.expression = negation();
        break;
    default:
        fprintf(stderr, "Could not parse expression at line \"%s\" column %d.\n", peek().line, peek().column);
        if(!runningAsREPL){
            exit(EXIT_FAILURE);
        }
        return NULL;
    }

    e->line = token.line;
    e->column = token.column;
    return e;
}

struct Node* handleInfix(struct Node* tmp);

struct Node* expression() {

    struct Node* e;
    struct Node* tmp;

    switch (peek().tokenType)
    {
    case INTEGER_VALUE_TOKEN:
        tmp = integerValue();
        e = handleInfix(tmp);
        break;
    case DECIMAL_VALUE_TOKEN:
        tmp = decimalValue();
        e = handleInfix(tmp);
        break;
    case TRUE_TOKEN:
    case FALSE_TOKEN:
        tmp = booleanValue();
        e = handleInfix(tmp);
        break;
    case NAME_TOKEN:
        tmp = variable();
        e = handleInfix(tmp);
        break;
    case FUNCTION_CALL_TOKEN:
		tmp = functionCall();
		e = handleInfix(tmp);
		break;
    case LEFT_P_TOKEN:
        leftP();
        tmp = expression();
        rightP();

        e = handleInfix(tmp);
        break;
    case BINARY_OPERATION_TOKEN:
        if(!(peek().value[0] == MINUS || peek().value[0] == PLUS)){
            fprintf(stderr, "Expression can only start with BINARY_OPERATION PLUS or MINUS.");
            fprintf(stderr, "At line \"%s\" column %d.\n", peek().line, peek().column);
            if(!runningAsREPL){
                exit(EXIT_FAILURE);
            }
            e = NULL;
            break;
        }

        tmp = malloc(sizeof(struct Node));
        tmp->nodeType = VALUE_NODE;
        tmp->nodeUnion.valueNode.valueType = ZERO;
        tmp->nodeUnion.valueNode.value.integerValue = 0;

        tmp->line = peek().line;
        tmp->column = 0;

        e = handleInfix(tmp);

        break;
    case NOT_TOKEN:
        tmp = negation();
        e = handleInfix(tmp);
        break;
    default:
        fprintf(stderr, "Could not parse expression at line \"%s\" column %d.\n", peek().line, peek().column);
        if(!runningAsREPL){
            exit(EXIT_FAILURE);
        }
        break;
    }

    return e;
}

void infixHelper(struct Node* dest){
    switch(peek().tokenType){
    case INTEGER_VALUE_TOKEN:
        dest->nodeUnion.binaryOperationNode.right = integerValue();
        break;
    case DECIMAL_VALUE_TOKEN:
        dest->nodeUnion.binaryOperationNode.right = decimalValue();
        break;
    case TRUE_TOKEN:
    case FALSE_TOKEN:
        dest->nodeUnion.binaryOperationNode.right = booleanValue();
        break;
    case NAME_TOKEN:
        dest->nodeUnion.binaryOperationNode.right = variable();
        break;
	case FUNCTION_CALL_TOKEN:
		dest->nodeUnion.binaryOperationNode.right = functionCall();
		break;
    case LEFT_P_TOKEN:
        leftP();
        dest->nodeUnion.binaryOperationNode.right = expression();
        rightP();
        break;
    case NOT_TOKEN:
        dest->nodeUnion.binaryOperationNode.right = negation();
        break;
    default:
        fprintf(stderr, "Could not parse expression at line \"%s\" column %d.\n", peek().line, peek().column);
        if(!runningAsREPL){
            exit(EXIT_FAILURE);
        }
        dest = NULL;
    }
}

struct Node* handleInfix(struct Node* tmp){

    struct Node* e = NULL;

    while(peek().tokenType == BINARY_OPERATION_TOKEN){

        if(e != NULL && e->nodeUnion.binaryOperationNode.presedence < peek().presedence){
            tmp = e->nodeUnion.binaryOperationNode.right;
            e->nodeUnion.binaryOperationNode.right = binaryOperation();
            e->nodeUnion.binaryOperationNode.right->nodeUnion.binaryOperationNode.left = tmp;
            infixHelper(e->nodeUnion.binaryOperationNode.right);
            tmp = e;
            continue;
        }

        e = binaryOperation();
        e->nodeUnion.binaryOperationNode.left = tmp;
        infixHelper(e);
        tmp = e;
    }

    if(e == NULL){
        e = tmp;
    }

    return e;
}

struct Node* ifDescend() {
    struct Token token = eat();

    if(token.tokenType != IF_TOKEN){
        wrongTokenPrint("IF", token);
        return NULL;
    }

	struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = IF_NODE;

    e->nodeUnion.ifNode.condition = expression();

	colon();

    e->line = token.line;
    e->column = token.column;
    return e;
}

struct Node* elseIfDescend() {
    struct Token token = eat();

    if(token.tokenType != ELSE_IF_TOKEN){
        wrongTokenPrint("ELSE IF", token);
        return NULL;
    }

	struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = ELSE_IF_NODE;

    e->nodeUnion.ifNode.condition = expression();
	colon();

    e->line = token.line;
    e->column = token.column;
    return e;
}

struct Node* elseDescend() {
    struct Token token = eat();

    if(token.tokenType != ELSE_TOKEN){
        wrongTokenPrint("ELSE", token);
        return NULL;
    }

	struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = ELSE_NODE;

	colon();

    e->line = token.line;
    e->column = token.column;
    return e;
}

int end() {
    struct Token token = eat();

    if(token.tokenType != END_TOKEN){
        wrongTokenPrint("END", token);
        return 0;
    }

    return 1;
}

struct Node* asign(){
    struct Token token = eat();

    if(token.tokenType != ASIGN_TOKEN){
        wrongTokenPrint("ASIGN", token);
        return NULL;
    }

    return NULL;
}

char* name(){
    struct Token token = eat();

    if(token.tokenType != NAME_TOKEN){
        wrongTokenPrint("NAME", token);
        return NULL;
    }

    char *tmp = malloc(sizeof(token.value));

    strcpy(tmp, token.value);

    return tmp;
}

struct Node* integer() {
    struct Token token = eat();

    if(token.tokenType != INTEGER_TOKEN){
        wrongTokenPrint("INTEGER", token);
        return NULL;
    }
    return NULL;
}

struct Node* integerDeclaration() {

    struct Node *e = malloc(sizeof(struct Node));

    e->line = peek().line;
    e->column = peek().column;

    integer();

    e->nodeType = DEFINE_NODE;
    e->nodeUnion.asignDefineNode.valueType = INTEGER;

    e->nodeUnion.asignDefineNode.name = name();

    if(peek().tokenType == ASIGN_TOKEN){
        asign();
        e->nodeUnion.asignDefineNode.expression = expression();
    } else {
        e->nodeUnion.asignDefineNode.expression = malloc(sizeof(struct Node));
        e->nodeUnion.asignDefineNode.expression->nodeType = VALUE_NODE;
        e->nodeUnion.asignDefineNode.expression->nodeUnion.asignDefineNode.value.integerValue = 0;
        e->nodeUnion.asignDefineNode.expression->nodeUnion.asignDefineNode.valueType = INTEGER;
        e->nodeUnion.asignDefineNode.expression->line = e->line;
        e->nodeUnion.asignDefineNode.expression->column = e->column;
    }

    return e;
}

struct Node* decimal() {
    struct Token token = eat();

    if(token.tokenType != DECIMAL_TOKEN){
        wrongTokenPrint("DECIMAL", token);
        return NULL;
    }
    return NULL;
}

struct Node* decimalDeclaration() {

    struct Node *e = malloc(sizeof(struct Node));

    e->line = peek().line;
    e->column = peek().column;

    decimal();

    e->nodeType = DEFINE_NODE;
    e->nodeUnion.asignDefineNode.valueType = DECIMAL;

    e->nodeUnion.asignDefineNode.name = name();

    if(peek().tokenType == ASIGN_TOKEN){
        asign();
        e->nodeUnion.asignDefineNode.expression = expression();
    } else {
        e->nodeUnion.asignDefineNode.expression = malloc(sizeof(struct Node));
        e->nodeUnion.asignDefineNode.expression->nodeType = VALUE_NODE;
        e->nodeUnion.asignDefineNode.expression->nodeUnion.asignDefineNode.value.integerValue = 0;
        e->nodeUnion.asignDefineNode.expression->nodeUnion.asignDefineNode.valueType = DECIMAL;
        e->nodeUnion.asignDefineNode.expression->line = e->line;
        e->nodeUnion.asignDefineNode.expression->column = e->column;
    }

    return e;
}

struct Node* boolean() {
    struct Token token = eat();

    if(token.tokenType != BOOLEAN_TOKEN){
        wrongTokenPrint("BOOLEAN", token);
        return NULL;
    }
    return NULL;
}

struct Node* booleanDeclaration() {

    struct Node *e = malloc(sizeof(struct Node));

    e->line = peek().line;
    e->column = peek().column;

    boolean();

    e->nodeType = DEFINE_NODE;
    e->nodeUnion.asignDefineNode.valueType = BOOLEAN;

    e->nodeUnion.asignDefineNode.name = name();

    if(peek().tokenType == ASIGN_TOKEN){
        asign();
        e->nodeUnion.asignDefineNode.expression = expression();
    } else {
        e->nodeUnion.asignDefineNode.expression = malloc(sizeof(struct Node));
        e->nodeUnion.asignDefineNode.expression->nodeType = VALUE_NODE;
        e->nodeUnion.asignDefineNode.expression->nodeUnion.asignDefineNode.value.integerValue = 0;
        e->nodeUnion.asignDefineNode.expression->nodeUnion.asignDefineNode.valueType = BOOLEAN;
        e->nodeUnion.asignDefineNode.expression->line = e->line;
        e->nodeUnion.asignDefineNode.expression->column = e->column;
    }

    return e;
}

enum ValueType type(){

    switch (peek().tokenType)
    {
    case INTEGER_TOKEN:
        integer();
        return INTEGER;
    case DECIMAL_TOKEN:
        decimal();
        return DECIMAL;
    case BOOLEAN_TOKEN:
        boolean();
        return BOOLEAN;
    default:
        wrongTokenPrint("TYPE", peek());
        return NULL_TYPE_VALUE;
    }
}

struct Node* declaration(){

    struct Node *e;

    switch (peek().tokenType)
    {
    case INTEGER_TOKEN:
        e = integerDeclaration();
        break;
    case DECIMAL_TOKEN:
        e = decimalDeclaration();
        break;
    case BOOLEAN_TOKEN:
        e = booleanDeclaration();
        break;
    default:
        wrongTokenPrint("DECLARATION", peek());
        e = NULL;
        break;
    }

    return e;
}

struct Node* coma(){
    struct Token token = eat();

    if(token.tokenType != COMA_TOKEN){
        wrongTokenPrint("COMA", token);
        return NULL;
    }

    return NULL;
}

struct Node* whileDescend() {
    struct Token token = eat();

    if(token.tokenType != WHILE_TOKEN){
        wrongTokenPrint("WHILE", token);
        return NULL;
    }

    struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = WHILE_NODE;
    e->nodeUnion.whileNode.statements = NULL;

    e->nodeUnion.whileNode.condition = expression();

    colon();

    e->line = token.line;
    e->column = token.column;
    return e;
}

struct Node* returnDescend(){
    struct Token token = eat();

    if(token.tokenType != RETURN_TOKEN){
        wrongTokenPrint("RETURN", token);
        return NULL;
    }

    struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = RETURN_NODE;
    e->nodeUnion.returnNode.expression = expression();

    e->line = token.line;
    e->column = token.column;
    return e;
}

struct Node* functionDeclaration() {

    struct Token token = eat();

    if(token.tokenType != FUNCTION_TOKEN){
        wrongTokenPrint("FUNCTION", token);
        return NULL;
    }

    struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = FUNCTION_DECLARATION_NODE;
    e->nodeUnion.functionDeclarationNode.statements = NULL;
    e->nodeUnion.functionDeclarationNode.arguments = NULL;
    e->nodeUnion.functionDeclarationNode.retutnType = NULL_TYPE_VALUE;

    if(peek().tokenType == NAME_TOKEN){
        e->nodeUnion.functionDeclarationNode.name = name();
    } else {
        token = eat();
        if(token.tokenType != FUNCTION_CALL_TOKEN){
            wrongTokenPrint("FUNCTION CALL", token);
            return NULL;
        }
        e->nodeUnion.functionDeclarationNode.name = g_strdup(token.value);
    }

    leftP();

	if(peek().tokenType != RIGHT_P_TOKEN){
		e->nodeUnion.functionDeclarationNode.arguments = g_list_append(e->nodeUnion.functionDeclarationNode.arguments, declaration());
	}

	while (peek().tokenType != RIGHT_P_TOKEN)
    {	
		coma();
        e->nodeUnion.functionDeclarationNode.arguments = g_list_append(e->nodeUnion.functionDeclarationNode.arguments, declaration());
    }

    rightP();

    if(peek().tokenType == ARROW_TOKEN){
        eat();
        e->nodeUnion.functionDeclarationNode.retutnType = type();
    }

    colon();

    e->line = token.line;
    e->column = token.column;
    return e;
}


struct Node* functionCall() {

    struct Token token = eat();

    if(token.tokenType != FUNCTION_CALL_TOKEN){
        wrongTokenPrint("FUNCTION CALL", token);
        return NULL;
    }

	leftP();

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = FUNCTION_CALL_NODE;
    //node->name = malloc(sizeof(token.value));
    //strncpy(node->name, token.value, strlen(token.value));
    node->nodeUnion.functionCallNode.name = g_strdup(token.value);
    node->nodeUnion.functionCallNode.arguments = NULL;

	if(peek().tokenType != RIGHT_P_TOKEN){
        node->nodeUnion.functionCallNode.arguments = g_list_append(node->nodeUnion.functionCallNode.arguments, expression());
	}

	while (peek().tokenType != RIGHT_P_TOKEN)
    {	
		coma();
        node->nodeUnion.functionCallNode.arguments = g_list_append(node->nodeUnion.functionCallNode.arguments, expression());
    }

    rightP();

    node->line = token.line;
    node->column = token.column;
    return node;
}

struct Node* statement() {

    struct Node* e;

    switch (peek().tokenType)
    {
    case INTEGER_VALUE_TOKEN:
    case DECIMAL_VALUE_TOKEN:
    case TRUE_TOKEN:
    case FALSE_TOKEN:
    case LEFT_P_TOKEN:
    case BINARY_OPERATION_TOKEN:
	case FUNCTION_CALL_TOKEN:
    case NOT_TOKEN:
        e = expression();
        break;
    case INTEGER_TOKEN:
        e = integerDeclaration();
        break;
    case DECIMAL_TOKEN:
        e = decimalDeclaration();
        break;
    case BOOLEAN_TOKEN:
        e = booleanDeclaration();
        break;
    case NAME_TOKEN:
        e = malloc(sizeof(struct Node));
        e->line = peek().line;
        e->column = peek().column;
        //e->name = name();
        char *tmpName = name();
        if(peek().tokenType == ASIGN_TOKEN){
            e->nodeType = ASIGN_NODE;
            e->nodeUnion.asignDefineNode.name = tmpName;
            asign();
            e->nodeUnion.asignDefineNode.expression = expression();
        } else {
            e->nodeType = VARIABLE_NODE;
            e->nodeUnion.variableNode.name = tmpName;
            struct Node* tmp = e;
            e = handleInfix(tmp);
        }
        break;
	case IF_TOKEN:
        e = ifDescend();
        break;
	case ELSE_TOKEN:
        e = elseDescend();
        break;
    case ELSE_IF_TOKEN:
        e = elseIfDescend();
        break;
    case WHILE_TOKEN:
        e = whileDescend();
        break;
    case FUNCTION_TOKEN:
        e = functionDeclaration();
        break;
    case RETURN_TOKEN:
        e = returnDescend();
        break;
    case BREAK_TOKEN:
        e = breakNode();
        break;
    case CONTINUE_TOKEN:
        e = continueNode();
        break;
    default:
        fprintf(stderr, "Could not parse statement at line \"%s\" column %d.\n", peek().line, peek().column);
        if(!runningAsREPL){
            exit(EXIT_FAILURE);
        }
        e = NULL;
        break;
    }

    return e;
}

struct Node* parse(char* src){

    token_i_parser = 0;

    if(!tokenize(src, tokens)){
        return NULL;
    }

    int indentation = 0;

    while(peek().tokenType == TAB_TOKEN){
        indentation++;
        eat();
    }

    if(peek().tokenType == END_TOKEN){
        return NULL;
    }

    struct Node* e = statement();

    if(!(e != NULL && end())){
        fprintf(stderr, "\nParsing not succesfull.\n");
        fprintf(stderr, "At line \"%s\".\n", src);
        if(!runningAsREPL){
            exit(EXIT_FAILURE);
        }
        return NULL;
    }

    e->indentation = indentation;
    return e;
}