int src_i;
int token_i = 0;

enum BinaryOperation {
    PLUS,
    MINUS,
    MULTIPLY,
    DIVIDE,
    LESS_THAN,
    GREATER_THAN,
    EQUALS,
    NOT_EQUALS,
    AND,
    OR,
    LESS_THAN_EQUAL,
    GREATER_THAN_EQUAL,
    REMAINDER,
    INVALID = 0
};

enum TokenType {

    INTEGER_TOKEN = 'I',
    DECIMAL_TOKEN = 'D',
    BOOLEAN_TOKEN = 'b',
    TRUE_TOKEN = 't',
    FALSE_TOKEN = 'a',
    IF_TOKEN = 'i',
    ELSE_TOKEN = 'e',
    ELSE_IF_TOKEN = 'o',
    NOT_TOKEN = 'O',
    BREAK_TOKEN = 'r',
    CONTINUE_TOKEN = 'c',
    WHILE_TOKEN = 'W',
    FUNCTION_TOKEN = 'f',
    RETURN_TOKEN = 'R',

    INTEGER_VALUE_TOKEN = 'N',
    DECIMAL_VALUE_TOKEN = 'd',

    BINARY_OPERATION_TOKEN = 'B',
    
    TAB_TOKEN = 'T',
    
    NAME_TOKEN = 'A',
    FUNCTION_CALL_TOKEN = 'C',

    LEFT_P_TOKEN = '(',
    RIGHT_P_TOKEN = ')',
    COMA_TOKEN = ',',
    DOT_TOKEN = '.',
    ASIGN_TOKEN = '=',
    EQUALS_TOKEN = '"',
    COLON_TOKEN = ':',
    ARROW_TOKEN = '>',

    END_TOKEN = 'E',
};

char *sourceCode[1024];

int sourceCode_i = -1;

struct Token {
    enum TokenType tokenType;
    char value[64];
    int presedence;
    char *line;
    int column;
};

char* getTokenString(enum TokenType tokenType){
    switch (tokenType)
    {
    case INTEGER_TOKEN:
        return "INTEGER";
    case DECIMAL_TOKEN:
        return "DECIMAL";
    case BOOLEAN_TOKEN:
        return "BOOLEAN";
    case TRUE_TOKEN:
        return "TRUE";
    case FALSE_TOKEN:
        return "FALSE";
    case IF_TOKEN:
        return "IF";
    case ELSE_TOKEN:
        return "ELSE";
    case ELSE_IF_TOKEN:
        return "ELSE_IF";
    case NOT_TOKEN:
        return "NOT";
    case BREAK_TOKEN:
        return "BREAK";
    case CONTINUE_TOKEN:
        return "CONTINUE";
    case WHILE_TOKEN:
        return "WHILE";
    case FUNCTION_TOKEN:
        return "FUNCTION";
    case RETURN_TOKEN:
        return "RETURN";
    case INTEGER_VALUE_TOKEN:
        return "INTEGER_VALUE";
    case DECIMAL_VALUE_TOKEN:
        return "DECIMAL_VALUE";
    case BINARY_OPERATION_TOKEN:
        return "BINARY_OPERATION";
    case TAB_TOKEN:
        return "TAB";
    case NAME_TOKEN:
        return "NAME";
    case FUNCTION_CALL_TOKEN:
        return "FUNCTION_CALL";
    case LEFT_P_TOKEN:
        return "LEFT_P";
    case RIGHT_P_TOKEN:
        return "RIGHT_P";
    case COMA_TOKEN:
        return "COMA";
    case DOT_TOKEN:
        return "DOT";
    case ASIGN_TOKEN:
        return "ASIGN";
    case EQUALS_TOKEN:
        return "EQUALS";
    case COLON_TOKEN:
        return "COLON";
    case ARROW_TOKEN:
        return "ARROW";
    case END_TOKEN:
        return "END";
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

    src_i = 0;
    token_i = 0;

    sourceCode[++sourceCode_i] = g_strdup(src);

    if(match(src, " ", NULL, 0)){

        fprintf(stderr, "Line can not start with space.\n");
        fprintf(stderr, "At line \"%s\".\n", src);

        return 0;
    }

    while(match(src, "\t", NULL, 0)){
        tokens[token_i].line = sourceCode[sourceCode_i];
        tokens[token_i++].tokenType = TAB_TOKEN;
    }

    while(src_i < strlen(src)){

        tokens[token_i].line = sourceCode[sourceCode_i];
        tokens[token_i].column = src_i;

        if(match(src, "\t", NULL, 0)){
            continue;
        }

        if(match(src, " ", NULL, 0)){
            continue;
        }

        if(match(src, "//", NULL, 0)){
            break;
        }

        if(match(src, "True", NULL, 0)){
            tokens[token_i++].tokenType = TRUE_TOKEN;
            continue;
        }
        if(match(src, "False", NULL, 0)){
            tokens[token_i++].tokenType = FALSE_TOKEN;
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
        if(match(src, "elseIf", " ", 0)){
            tokens[token_i++].tokenType = ELSE_IF_TOKEN;
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
        if(match(src, "not", NULL, 0)){
            tokens[token_i++].tokenType = NOT_TOKEN;
            continue;
        }
        if(match(src, "break", NULL, 0)){
            tokens[token_i++].tokenType = BREAK_TOKEN;
            continue;
        }
        if(match(src, "continue", NULL, 0)){
            tokens[token_i++].tokenType = CONTINUE_TOKEN;
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
        if(match(src, ",", NULL, 0)){
            tokens[token_i++].tokenType = COMA_TOKEN;
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

        if(match(src, "or", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = -1;
            tokens[token_i].value[0] = OR;
            token_i++;
            continue;
        }
        if(match(src, "and", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 0;
            tokens[token_i].value[0] = AND;
            token_i++;
            continue;
        }

        if(match(src, "==", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 1;
            tokens[token_i].value[0] = EQUALS;
            token_i++;
            continue;
        }
        if(match(src, "!=", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 1;
            tokens[token_i].value[0] = NOT_EQUALS;
            token_i++;
            continue;
        }
        if(match(src, "<=", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 1;
            tokens[token_i].value[0] = LESS_THAN_EQUAL;
            token_i++;
            continue;
        }
        if(match(src, ">=", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 1;
            tokens[token_i].value[0] = GREATER_THAN_EQUAL;
            token_i++;
            continue;
        }
        if(match(src, "<", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 1;
            tokens[token_i].value[0] = LESS_THAN;
            token_i++;
            continue;
        }
        if(match(src, ">", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 1;
            tokens[token_i].value[0] = GREATER_THAN;
            token_i++;
            continue;
        }

        if(match(src, "+", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 2;
            tokens[token_i].value[0] = PLUS;
            token_i++;
            continue;
        }
        if(match(src, "-", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 2;
            tokens[token_i].value[0] = MINUS;
            token_i++;
            continue;
        }

        if(match(src, "*", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 3;
            tokens[token_i].value[0] = MULTIPLY;
            token_i++;
            continue;
        }
        if(match(src, "/", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 3;
            tokens[token_i].value[0] = DIVIDE;
            token_i++;
            continue;
        }
        if(match(src, "%", NULL, 0)){
            tokens[token_i].tokenType = BINARY_OPERATION_TOKEN;
            tokens[token_i].presedence = 3;
            tokens[token_i].value[0] = REMAINDER;
            token_i++;
            continue;
        }

        if(match(src, "=", NULL, 0)){
            tokens[token_i++].tokenType = ASIGN_TOKEN;
            continue;
        }

        if(matchNumber(src, tokens[token_i].value, ".")){
            tokens[token_i].tokenType = DECIMAL_VALUE_TOKEN;
            token_i++;
            continue;
        }

        if(matchNumber(src, tokens[token_i].value, NULL)){
            tokens[token_i].tokenType = INTEGER_VALUE_TOKEN;
            token_i++;
            continue;
        }

        if(matchName(src, tokens[token_i].value, "(")){
            tokens[token_i].tokenType = FUNCTION_CALL_TOKEN;
            token_i++;
            continue;
        }

        if(matchName(src, tokens[token_i].value, NULL)){
            tokens[token_i].tokenType = NAME_TOKEN;
            token_i++;
            continue;
        }

        fprintf(stderr, "Input coud not be tokenized.\n");
        fprintf(stderr, "At line \"%s\".\n", src);
        return 0;
    }

    tokens[token_i++].tokenType = END_TOKEN;

    return 1;
}