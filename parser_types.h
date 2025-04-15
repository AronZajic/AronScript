union Value
{
	int integerValue;
	float decimalValue;
};

enum BinaryOperation {
    PLUS = '+',
    MINUS = '-',
    MULTIPLY = '*',
    DIVIDE = '/',
    LESS_THAN = '<',
    GREATER_THAN = '>',
    EQUALS = 'Q',
    NOT_EQUALS = '!',
    AND = '&',
    OR = '|',
    LESS_THAN_EQUAL = '4',
    GREATER_THAN_EQUAL = '7',
};

enum NodeType {
    VALUE_NODE = 'N',
    VARIABLE_NODE = 'V',
    BINARY_OPERATION_NODE = 'B',
    DEFINE_NODE = 'D',
    ASIGN_NODE = 'A',
    WHILE_NODE = 'W',
    FOR_NODE = 'F',
    STATEMENTS_NODE = 'S',
    FUNCTION_DECLARATION_NODE = 'U',
    END_NODE = 'E',
    RETURN_NODE = 'R',
    FUNCTION_CALL_NODE = 'C',
    IF_NODE = 'I',
    ELSE_NODE = 'e',
    ELSE_IF_NODE = 'i',
    NOT_NODE = 'O',
    BREAK_NODE = 'b',
    CONTINUE_NODE = 'c'
};

enum ValueType {
    INTEGER = 'I',
    DECIMAL = 'D',
    BOOLEAN = 'B',
    NULL_TYPE_VALUE = 'N'
};

/*struct Parameter {
    enum ValueType type;
    char* name;
};*/

struct Node {
    enum NodeType nodeType;
    int indentation;

    // Type VALUE
    enum ValueType valueType;
    union Value value;

    // Type BINARYOP
    enum BinaryOperation binaryOperation;
    struct Node* left;
    struct Node* right;
    int presedence;

    // Type VARIABLE
    char* name;
    // enum ValueType valueType;
    // union Value value;

    // Type WHILE
    struct Node* condition;
    struct Node* statements;

    // Type FUNCTION_DECLARATION
    // char* name;
    enum ValueType retutnType;
    GList* arguments;

    // Type RETURN
    struct Node* expression;

    // Type FUNCTION_CALL
    // GList* arguments;

    // Type ASIGN and DEFINE
    // enum ValueType valueType;

    // Type STATEMENTS
    GList* body;
};