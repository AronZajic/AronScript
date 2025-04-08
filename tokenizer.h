int src_i;
int token_i = 0;

enum TokenType {
    INTEGER_VALUE_TOKEN = 'N',
    DECIMAL_VALUE_TOKEN = 'd',
    EXPRESSION_TOKEN = 'X',
    LEFT_P_TOKEN = '(',
    RIGHT_P_TOKEN = ')',
    END_TOKEN = 'E',
    BINARY_OPERATION_TOKEN = 'B',
    TAB_TOKEN = 'T',
    FOR_TOKEN = 'F',
    WHILE_TOKEN = 'W',
    INTEGER_TOKEN = 'I',
    DECIMAL_TOKEN = 'D',
    BOOLEAN_TOKEN = 'b',
    LIST_TOKEN = 'L',
    FUNCTION_TOKEN = 'f',
    IF_TOKEN = 'i',
    STRUCT_TOKEN = 'S',
    NAME_TOKEN = 'A',
    ASIGN_TOKEN = '=',
    EQUALS_TOKEN = '"',
    COLON_TOKEN = ':',
    ARROW_TOKEN = '>',
    RETURN_TOKEN = 'R',
    FUNCTION_CALL_TOKEN = 'C',
    COMA_TOKEN = ',',
    DOT_TOKEN = '.',
    ELSE_TOKEN = 'e'
};

struct Token {
    enum TokenType tokenType;
    char value[64];
    int presedence;
};

char* getKeywordTokenTypeString(enum TokenType tokenType){
    switch (tokenType)
    {
    case FOR_TOKEN:
        return "for";
    case WHILE_TOKEN:
        return "while";
    case INTEGER_TOKEN:
        return "Integer";
    case DECIMAL_TOKEN:
        return "Decimal";
    case LIST_TOKEN:
        return "List";
    case FUNCTION_TOKEN:
        return "fun";
    case IF_TOKEN:
        return "if";
    case STRUCT_TOKEN:
        return "Struct";
    default:
        return NULL;
    }
}

int match(char src[], char s[], char separators[], int skip){
    int i;
    for(i = 0; i < strlen(s); i++){
        if(s[i] != src[src_i+i]){
            return 0;
        }
    }

    if(separators == NULL || strlen(separators) == 0){
        src_i += i;
        return 1;
    }

    for(int j = 0; j < strlen(separators); j++){
        if(separators[j] == src[src_i+i]){
            src_i += i + skip;
            return 1;
        }
    }

    return 0;
}

int matchNumber(char src[], char *dest, char separators[]){

    //printf("match number\n");

    int i = 0;
    while (src_i+i < strlen(src) && '0' <= src[src_i+i] && src[src_i+i] <= '9')
    {
        //dest[i] = src[src_i+i];
        i++;
    }

    if(i == 0){
        return 0;
    }

    //dest[i] = 0;
    strncpy(dest, src + src_i, i);
    dest[i] = '\0';

    if(separators == NULL || strlen(separators) == 0){
        src_i += i;
        return 1;
    }

    for(int j = 0; j < strlen(separators); j++){
        if(separators[j] == src[src_i+i]){
            src_i += i;
            return 1;
        }
    }

    return 0;
}

int matchName(char src[], char *dest, char separators[]){

    //printf("match name\n");

    int i = 0;
    while (
        (src_i+i < strlen(src) && 'a' <= src[src_i+i] && src[src_i+i] <= 'z') ||
        (src_i+i < strlen(src) && 'A' <= src[src_i+i] && src[src_i+i] <= 'Z') ||
        (src_i+i < strlen(src) && '0' <= src[src_i+i] && src[src_i+i] <= '9'))
    {
        //dest[i] = src[src_i+i];
        i++;
    }

    if(i == 0){
        return 0;
    }

    strncpy(dest, src + src_i, i);
    dest[i] = '\0';

    //dest[i] = 0;

    if(separators == NULL || strlen(separators) == 0){
        src_i += i;
        return 1;
    }

    for(int j = 0; j < strlen(separators); j++){
        if(separators[j] == src[src_i+i]){
            src_i += i;
            return 1;
        }
    }

    return 0;
}

int tokenize(char src[], struct Token tokens[]){

    //printf("%s\n", src);

    src_i = 0;
    token_i = 0;

    if(match(src, " ", NULL, 0)){

        fprintf(stderr, "Line can not start with space.\n");

        return 0;
    }

    while(match(src, "\t", NULL, 0)){
        tokens[token_i++].tokenType = TAB_TOKEN;
    }

    while(src_i < strlen(src)){

        //printf("%d %d %d\n", strlen(src), src_i, token_i);

        // we will see if we need this, probably will for robustness
        if(match(src, "\t", NULL, 0)){
            continue;
        }

        if(match(src, " ", NULL, 0)){
            continue;
        }

        if(match(src, "Integer", " :", 0)){
            tokens[token_i++].tokenType = INTEGER_TOKEN;
            continue;
        }
        if(match(src, "Decimal", " :", 0)){
            tokens[token_i++].tokenType = DECIMAL_TOKEN;
            continue;
        }
        if(match(src, "Boolean", " :", 0)){
            tokens[token_i++].tokenType = BOOLEAN_TOKEN;
            continue;
        }
        if(match(src, "if", " ", 0)){
            tokens[token_i++].tokenType = IF_TOKEN;
            continue;
        }
        if(match(src, "else", " :", 0)){
            tokens[token_i++].tokenType = ELSE_TOKEN;
            continue;
        }
        if(match(src, "while", " ", 0)){
            tokens[token_i++].tokenType = WHILE_TOKEN;
            continue;
        }
        if(match(src, "function", " ", 0)){
            tokens[token_i++].tokenType = FUNCTION_TOKEN;
            continue;
        }
        if(match(src, "return", " ", 0)){
            tokens[token_i++].tokenType = RETURN_TOKEN;
            continue;
        }
        if(match(src, "->", NULL, 0)){
            tokens[token_i++].tokenType = ARROW_TOKEN;
            continue;
        }

        if(match(src, ":", NULL, 0)){
            tokens[token_i++].tokenType = COLON_TOKEN;
            continue;
        }
        if(match(src, ".", NULL, 0)){
            tokens[token_i++].tokenType = DOT_TOKEN;
            continue;
        }
        if(match(src, "(", NULL, 0)){
            tokens[token_i++].tokenType = LEFT_P_TOKEN;
            continue;
        }
        if(match(src, ")", NULL, 0)){
            tokens[token_i++].tokenType = RIGHT_P_TOKEN;
            continue;
        }
        if(match(src, "==", NULL, 0)){
            tokens[token_i++].tokenType = EQUALS_TOKEN;
            continue;
        }
        if(match(src, "=", NULL, 0)){
            tokens[token_i++].tokenType = ASIGN_TOKEN;
            continue;
        }

        if(match(src, "<", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 1;
            tokens[token_i].value[0] = '<';
            token_i++;
            continue;
        }
        if(match(src, ">", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 1;
            tokens[token_i].value[0] = '>';
            token_i++;
            continue;
        }

        if(match(src, "+", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 2;
            tokens[token_i].value[0] = '+';
            token_i++;
            continue;
        }
        if(match(src, "-", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 2;
            tokens[token_i].value[0] = '-';
            token_i++;
            continue;
        }

        if(match(src, "*", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 3;
            tokens[token_i].value[0] = '*';
            token_i++;
            continue;
        }
        if(match(src, "/", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 3;
            tokens[token_i].value[0] = '/';
            token_i++;
            continue;
        }

        //char *number;

        if(matchNumber(src, tokens[token_i].value, ".")){
            tokens[token_i].tokenType = DECIMAL_VALUE_TOKEN;
            //tokens[token_i].value = number;
            token_i++;
            continue;
        }

        if(matchNumber(src, tokens[token_i].value, NULL)){
            tokens[token_i].tokenType = INTEGER_VALUE_TOKEN;
            //tokens[token_i].value = number;
            token_i++;
            continue;
        }

        if(matchName(src, tokens[token_i].value, "(")){
            tokens[token_i].tokenType = FUNCTION_CALL_TOKEN;
            //tokens[token_i].value = number;
            token_i++;
            continue;
        }

        if(matchName(src, tokens[token_i].value, NULL)){
            tokens[token_i].tokenType = NAME_TOKEN;
            //tokens[token_i].value = number;
            token_i++;
            continue;
        }

        fprintf(stderr, "Input coud not be tokenized.\n");

        /*for(int j = 0; j < 10; j++){

            struct Token token = tokens[j];
    
            fprintf(stderr, "( %c, %c, %d, %d ), ", token.tokenType, token.value, token.value, j);
    
        }
        return 0;*/

    }

    tokens[token_i++].tokenType = END_TOKEN;

    return 1;
}