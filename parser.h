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

        printf("( %c, %s, %d ), ", token.tokenType, token.value, j);

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

struct Node* number() {

    //printf("Parsing number\n");

    struct Token token = eat();

    if(token.tokenType != NUMBER_TOKEN){
        wrongTokenPrint("NUMBER", token);
        return NULL;
    }

    struct Node* node = malloc(sizeof(struct Node));

    node->nodeType = NUMBER_NODE;
    node->number = atoi(token.value);

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

    //printf("Parsing leftP\n");

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
struct Node* handle_infix(struct Node* tmp);

struct Node* expression() {

    //printf("Parsing expression\n");

    struct Node* e;
    struct Node* tmp;

    switch (peek().tokenType)
    {
    case NUMBER_TOKEN:
        tmp = number();
        e = handle_infix(tmp);
        break;
    case NAME_TOKEN:
        tmp = variable();
        e = handle_infix(tmp);
        break;
    case FUNCTION_CALL_NODE:
		tmp = functionCall();
		e = handle_infix(tmp);
		break;
    case LEFT_P_TOKEN:
        //printf("Expression LEFT\n");
        leftP();
        tmp = expression();
        rightP();

        if(peek().tokenType == BINARY_OPERATION_TOKEN){
            e = binary_operation();
            e->left = tmp;
            e->right = expression();
        } else {
            e = tmp;
        }
        break;
    case BINARY_OPERATION_TOKEN:
        //printf("Expression BIN\n");
        if(peek().value[0] == '*' || peek().value[0] == '/'){ // TODO prerobiť lebo takto môze cačať aj so zobakom
            fprintf(stderr, "Expression can not start with %c.\n", peek().value[0]);
            e = NULL;
            break;
        }

        tmp = malloc(sizeof(struct Node));
        tmp->nodeType = NUMBER_NODE;
        tmp->number = 0;

        e = handle_infix(tmp);

        break;
    default:
        fprintf(stderr, "Could not parse expression at %c %c.\n", peek().tokenType, peek().value);
        break;
    }

    return e;
}

struct Node* infix_helper(struct Node* dest){
    switch(peek().tokenType){
    case NUMBER_TOKEN:
        dest->right = number();
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
    default:
		fprintf(stderr, "Could not parse expression at %c %c.\n", peek().tokenType, peek().value);
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

    if(token.tokenType != INT_TOKEN){
        wrongTokenPrint("INT", token);
        return NULL;
    }
    return NULL;
}

struct Node* integerDeclaration() {

    integer();

    struct Node *e = malloc(sizeof(struct Node));
    e->nodeType = DEFINE_NODE;

    e->name = name();

    if(peek().tokenType == ASIGN_TOKEN){
        asign();
        e->expression = expression();
    } else {
        e->expression = malloc(sizeof(struct Node));
        e->expression->nodeType = NUMBER_NODE;
        e->expression->number = 0;
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
		e->argument = integerDeclaration();
	}

	while (peek().tokenType != RIGHT_P_TOKEN)
    {	
		coma();
        e->argument = integerDeclaration();
    }

    rightP();

    if(peek().tokenType == ARROW_TOKEN){
        eat();
        integer();
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
    node->argument = NULL;

	if(peek().tokenType != RIGHT_P_TOKEN){
		node->argument = expression();
	}

	while (peek().tokenType != RIGHT_P_TOKEN)
    {	
		coma();
        node->argument = expression();
    }

    rightP();

    return node;
}

struct Node* statement() {

    //printf("Parsing expression\n");

    struct Node* e;

    switch (peek().tokenType)
    {
    case NUMBER_TOKEN:
    case LEFT_P_TOKEN:
    case BINARY_OPERATION_TOKEN:
	case FUNCTION_CALL_TOKEN:
        e = expression();
        break;
    case INT_TOKEN:
        e = integerDeclaration();
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
    default:
        fprintf(stderr, "Could not parse statement at %c %c.\n", peek().tokenType, peek().value);
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