union Value
{
	int intValue;
	float decimalValue;
};

enum BinaryOperation {
    PLUS = '+',
    MINUS = '-',
    MULTIPLY = '*',
    DIVIDE = '/',
    LESS_THAN = '<',
    GREATER_THAN = '>',
    EQUALS = 'Q'
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
    ELSE_NODE = 'e'
};

enum ValueType {
    INTEGER = 'I',
    DECIMAL = 'D',
    BOOLEAN = 'B',
    LIST = 'L',
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
    struct Node* expression;
    //enum ValueType valueType;

    // Type WHILE
    struct Node* condition;
    //struct Node* body[1024];
    GList* body;

    // Type FUNCTION_DECLARATION
    // char* name;
    //struct Parameter parameters[1024];
    enum ValueType retutnType;
    // struct Node* body[1024];

    // Type RETURN
    // struct Node* expression;

    // Type FUNCTION_CALL
    //struct Node* argument;
    GList* arguments;

    // Type ASIGN and DEFINE
    //enum ValueType valueType;
};