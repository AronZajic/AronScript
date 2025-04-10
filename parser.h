#include "tokenizer.h"
#include "parser_types.h"

struct Token tokens[128];

int token_i_parser = 0;

struct Token peek(){
    return tokens[token_i_parser];
}

struct Token eat(){
    struct Token token = tokens[token_i_parser];
    token_i_parser++;
    return token; 
}

void wrongTokenPrint(char expected[], struct Token token){
    
	/*for(int j = 0; j < 10; j++){

        struct Token token = tokens[j];

        fprintf(stderr, "( %c, %s, %d ), ", token.tokenType, token.value, j);

    }*/
	fprintf(stderr, "\nWrong token! %s token expected, got %c token at (%d).\n", expected, token.tokenType, token_i_parser-1);
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

    //printf("Parsing number\n");

    struct Token token = eat();

    if(token.tokenType != INTEGER_VALUE_TOKEN){
        wrongTokenPrint("INTEGER VALUE", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = VALUE_NODE;
    node->valueType = INTEGER;
    node->value.intValue = atoi(token.value);

    return node;
}

struct Node* breakNode() {

    //printf("Parsing number\n");

    struct Token token = eat();

    if(token.tokenType != BREAK_TOKEN){
        wrongTokenPrint("BREAK", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = BREAK_NODE;

    return node;
}

struct Node* continueNode() {

    //printf("Parsing number\n");

    struct Token token = eat();

    if(token.tokenType != CONTINUE_TOKEN){
        wrongTokenPrint("CONTINUE", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = CONTINUE_NODE;

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
        wrongTokenPrint("DECIMAL VALUE", token);
        return NULL;
    }

    dot();

    struct Token token2 = eat();

    if(token2.tokenType != INTEGER_VALUE_TOKEN){
        wrongTokenPrint("INTEGER VALUE", token2);
        return NULL;
    }

    char tmp[128];
    sprintf(tmp, "%s.%s", token.value, token2.value); 

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = VALUE_NODE;
    node->valueType = DECIMAL;
    node->value.decimalValue = atof(tmp);

    return node;
}

struct Node* booleanValue() {

    //printf("Parsing number\n");

    struct Token token = eat();

    if(token.tokenType != TRUE_TOKEN && token.tokenType != FALSE_TOKEN){
        wrongTokenPrint("BOOLEAN", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = VALUE_NODE;
    node->valueType = BOOLEAN;

    if(token.tokenType == TRUE_TOKEN){
        node->value.intValue = 1;
    } else {
        node->value.intValue = 0;
    }

    return node;
}

struct Node* variable() {

    //printf("Parsing number\n");

    struct Token token = eat();

    if(token.tokenType != NAME_TOKEN){
        wrongTokenPrint("NAME", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = VARIABLE_NODE;
    //node->name = malloc(sizeof(token.value));
    //strncpy(node->name, token.value, strlen(token.value));
    node->name = g_strdup(token.value);

    return node;
}

struct Node* binary_operation() {

    //printf("Parsing binary operation\n");

    struct Token token = eat();

    if(token.tokenType != BINARY_OPERATION_TOKEN){
        wrongTokenPrint("BINARY OPERATION", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = BINARY_OPERATION_NODE;
    node->binaryOperation = token.value[0];
    node->presedence = token.presedence;

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

    //printf("Parsing rightP\n");

    struct Token token = eat();

    if(token.tokenType != RIGHT_P_TOKEN){
        wrongTokenPrint("RIGHT_P", token);
    }

    return NULL;
}

struct Node* functionCall();
struct Node* expression();

struct Node* negation() {

    //printf("Parsing rightP\n");

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
        return NULL;
    case DECIMAL_VALUE_TOKEN:
        fprintf(stderr, "Decimal value can not be negated.");
        return NULL;
    case TRUE_TOKEN:
    case FALSE_TOKEN:
        e->expression = booleanValue();
        break;
    case NAME_TOKEN:
        e->expression = variable();
        break;
    case FUNCTION_CALL_TOKEN:
        e->expression = functionCall();
		break;
    case LEFT_P_TOKEN:
        leftP();
        e->expression = expression();
        rightP();
        break;
    case NOT_TOKEN:
        e->expression = negation();
        break;
    default:
        fprintf(stderr, "Could not parse expression at %c %p.\n", peek().tokenType, peek().value);
        return NULL;
    }

    return e;
}

struct Node* handle_infix(struct Node* tmp);

struct Node* expression() {

    //printf("Parsing expression\n");

    struct Node* e;
    struct Node* tmp;

    switch (peek().tokenType)
    {
    case INTEGER_VALUE_TOKEN:
        tmp = integerValue();
        e = handle_infix(tmp);
        break;
    case DECIMAL_VALUE_TOKEN:
        tmp = decimalValue();
        e = handle_infix(tmp);
        break;
    case TRUE_TOKEN:
    case FALSE_TOKEN:
        tmp = booleanValue();
        e = handle_infix(tmp);
        break;
    case NAME_TOKEN:
        tmp = variable();
        e = handle_infix(tmp);
        break;
    case FUNCTION_CALL_TOKEN:
		tmp = functionCall();
		e = handle_infix(tmp);
		break;
    case LEFT_P_TOKEN:
        //printf("Expression LEFT\n");
        leftP();
        tmp = expression();
        rightP();

        /*if(peek().tokenType == BINARY_OPERATION_TOKEN){
            e = binary_operation();
            e->left = tmp;
            e->right = expression();
        } else {
            e = tmp;
        }*/

        e = handle_infix(tmp);
        break;
    case BINARY_OPERATION_TOKEN:
        //printf("Expression BIN\n");
        if(peek().value[0] == '*' || peek().value[0] == '/'){ // TODO prerobiť lebo takto môze začať aj so zobakom
            fprintf(stderr, "Expression can not start with %c.\n", peek().value[0]);
            e = NULL;
            break;
        }

        tmp = malloc(sizeof(struct Node));
        tmp->nodeType = VALUE_NODE;
        tmp->value.intValue = 0;

        e = handle_infix(tmp);

        break;
    case NOT_TOKEN:
        tmp = negation();
        e = handle_infix(tmp);
        break;
    default:
        fprintf(stderr, "Could not parse expression at %c %p.\n", peek().tokenType, peek().value);
        break;
    }

    return e;
}

void infix_helper(struct Node* dest){
    switch(peek().tokenType){
    case INTEGER_VALUE_TOKEN:
        dest->right = integerValue();
        break;
    case DECIMAL_VALUE_TOKEN:
        dest->right = decimalValue();
        break;
    case TRUE_TOKEN:
    case FALSE_TOKEN:
        dest->right = booleanValue();
        break;
    case NAME_TOKEN:
        dest->right = variable();
        break;
	case FUNCTION_CALL_TOKEN:
		dest->right = functionCall();
		break;
    case LEFT_P_TOKEN:
        leftP();
        dest->right = expression();
        rightP();
        break;
    case NOT_TOKEN:
        dest->right = negation();
        break;
    default:
		fprintf(stderr, "Could not parse expression at %c %p.\n", peek().tokenType, peek().value);
        dest = NULL;
    }
}

struct Node* handle_infix(struct Node* tmp){

    struct Node* e = NULL;

    while(peek().tokenType == BINARY_OPERATION_TOKEN){

        if(e != NULL && e->presedence < peek().presedence){
            tmp = e->right;
            e->right = binary_operation();
            e->right->left = tmp;
            infix_helper(e->right);
            tmp = e;
            continue;
        }

        e = binary_operation();
        e->left = tmp;
        infix_helper(e);
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

    e->condition = expression();

	colon();

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

    return e;
}

struct Node* end() {
    struct Token token = eat();

    if(token.tokenType != END_TOKEN){
        wrongTokenPrint("END", token);
        return NULL;
    }

    return NULL;
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

    integer();

    struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = DEFINE_NODE;
    e->valueType = INTEGER;

    e->name = name();

    if(peek().tokenType == ASIGN_TOKEN){
        asign();
        e->expression = expression();
    } else {
        e->expression = malloc(sizeof(struct Node));
        e->expression->nodeType = VALUE_NODE;
        e->expression->value.intValue = 0;
        e->expression->valueType = INTEGER;
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

    decimal();

    struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = DEFINE_NODE;
    e->valueType = DECIMAL;

    e->name = name();

    if(peek().tokenType == ASIGN_TOKEN){
        asign();
        e->expression = expression();
    } else {
        e->expression = malloc(sizeof(struct Node));
        e->expression->nodeType = VALUE_NODE;
        e->expression->value.intValue = 0;
        e->expression->valueType = DECIMAL;
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

    boolean();

    struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = DEFINE_NODE;
    e->valueType = BOOLEAN;

    e->name = name();

    if(peek().tokenType == ASIGN_TOKEN){
        asign();
        e->expression = expression();
    } else {
        e->expression = malloc(sizeof(struct Node));
        e->expression->nodeType = VALUE_NODE;
        e->expression->value.intValue = 0;
        e->expression->valueType = BOOLEAN;
    }

    return e;
}

enum ValueType type(){

    //struct Node *e = malloc(sizeof(struct Node));

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

    struct Node *e = malloc(sizeof(struct Node));

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
    e->body = NULL;

    e->condition = expression();

    colon();

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
    e->expression = expression();

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
    e->body = NULL;
    e->arguments = NULL;
    e->retutnType = NULL_TYPE_VALUE;

    if(peek().tokenType == NAME_TOKEN){
        e->name = name();
    } else {
        token = eat();
        if(token.tokenType != FUNCTION_CALL_TOKEN){
            wrongTokenPrint("FUNCTION CALL", token);
            return NULL;
        }
        e->name = g_strdup(token.value);
    }

    leftP();

	if(peek().tokenType != RIGHT_P_TOKEN){
		e->arguments = g_list_append(e->arguments, declaration());
	}

	while (peek().tokenType != RIGHT_P_TOKEN)
    {	
		coma();
        e->arguments = g_list_append(e->arguments, declaration());
    }

    rightP();

    if(peek().tokenType == ARROW_TOKEN){
        eat();
        e->retutnType = type();
    }

    colon();

    return e;
}


struct Node* functionCall() {

    //printf("Parsing number\n");

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
    node->name = g_strdup(token.value);
    node->arguments = NULL;

	if(peek().tokenType != RIGHT_P_TOKEN){
        node->arguments = g_list_append(node->arguments, expression());
	}

	while (peek().tokenType != RIGHT_P_TOKEN)
    {	
		coma();
        node->arguments = g_list_append(node->arguments, expression());
    }

    rightP();

    return node;
}

struct Node* statement() {

    //printf("Parsing expression\n");

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
        e->name = name();
        if(peek().tokenType == ASIGN_TOKEN){
            e->nodeType = ASIGN_NODE;
            asign();
            e->expression = expression();
        } else {
            e->nodeType = VARIABLE_NODE;
            struct Node* tmp = e;
            e = handle_infix(tmp);
        }
        break;
	case IF_TOKEN:
        e = ifDescend();
        break;
	case ELSE_TOKEN:
        e = elseDescend();
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
        fprintf(stderr, "Could not parse statement at %c %p.\n", peek().tokenType, peek().value);
        break;
    }

    return e;
}

struct Node* parse(char* src){

    /*token_i_parser = 0;

    tokenize2(src, tokens);

    printf("\n");

    for(int j = 0; j < 10; j++){

        struct Token token = tokens[j];

        printf("( %c, %c, %d, %d ), ", token.tokenType, token.value, token.value, j);

    }*/

    token_i_parser = 0;

    if(!tokenize(src, tokens)){
        return NULL;
    }

    /*for(int j = 0; j < 10; j++){

        struct Token token = tokens[j];

        printf("( %c, %s, %d ), ", token.tokenType, token.value, j);

    }*/

    int indentation = 0;

    while(peek().tokenType == TAB_TOKEN){
        indentation++;
        eat();
    }

    //printf("\n");

    if(peek().tokenType == END_TOKEN){
        return NULL;
    }

    struct Node* e = statement();
    if(e != NULL)
        end();
    else{
        fprintf(stderr, "\nParsing not succesfull.\n");
        return NULL;
    }

    //treeprint(e, 0);

    /*struct Node* p = e;

    printf("NodeK: %c - %d %c\n", p->nodeType, p->number, p->binaryOperation);

    p = e->left;
    printf("NodeL: %c - %d %c\n", p->nodeType, p->number, p->binaryOperation);
    p = e->right;
    printf("NodeR: %c - %d %c\n", p->nodeType, p->number, p->binaryOperation);
    p = e->left->right;
    printf("NodeLR: %c - %d %c\n", p->nodeType, p->number, p->binaryOperation);
    p = e->left->left;
    printf("NodeLL: %c - %d %c\n", p->nodeType, p->number, p->binaryOperation);
    p = e->right->right;
    printf("NodeRR: %c - %d %c\n", p->nodeType, p->number, p->binaryOperation);
    p = e->right->left;
    printf("NodeRL: %c - %d %c\n", p->nodeType, p->number, p->binaryOperation);*/
    e->indentation = indentation;
    return e;
}