union Value
{
	int integerValue;
	float decimalValue;
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
    ZERO = 'Z',
    BOOLEAN = 'B',
    NULL_TYPE_VALUE = 'N'
};

/*struct Parameter {
    enum ValueType type;
    char* name;
};*/

struct ValueNode {
    enum ValueType valueType;
    union Value value;
};

struct VariableNode {
    //enum ValueType valueType;
    char* name;
};

struct AsignDefineNode {
    enum ValueType valueType;
    union Value value;
    char* name;
    struct Node* expression;
};

struct FunctionDeclarationNode {
    char* name;
    enum ValueType retutnType;
    GList* arguments;
    struct Node* statements;
};

struct FunctionCallNode {
    char* name;
    GList* arguments;
};

struct BinaryOperationNode {
    enum BinaryOperation binaryOperation;
    struct Node* left;
    struct Node* right;
    int presedence;
};

struct WhileNode {
    struct Node* condition;
    struct Node* statements;
};

struct ReturnNode {
    struct Node* expression;
};

struct StatementsNode {
    GList* body;
};

struct IfNode {
    struct Node* condition;
    struct Node* ifBody;
    struct Node* elseBody;
};

struct NotNode {
    struct Node* expression;
};

union NodeUnion
{
    struct ValueNode valueNode;
    struct VariableNode variableNode;
    struct AsignDefineNode asignDefineNode;
    struct FunctionDeclarationNode functionDeclarationNode;
    struct FunctionCallNode functionCallNode;
    struct BinaryOperationNode binaryOperationNode;
    struct WhileNode whileNode;
    struct ReturnNode returnNode;
    struct StatementsNode statementsNode;
    struct IfNode ifNode;
    struct NotNode notNode;
};


struct Node {
    enum NodeType nodeType;
    int indentation;

    char *line;
    int column;

    union NodeUnion nodeUnion;

    // Type VALUE
    // enum ValueType valueType;
    // union Value value;

    // Type BINARYOP
    // enum BinaryOperation binaryOperation;
    // struct Node* left;
    // struct Node* right;
    // int presedence;

    // Type VARIABLE
    // char* name;
    // enum ValueType valueType;
    // union Value value;

    // Type WHILE
    // struct Node* condition;
    // struct Node* statements;

    // Type FUNCTION_DECLARATION
    // char* name;
    // enum ValueType retutnType;
    // GList* arguments;

    // Type RETURN
    // struct Node* expression;

    // Type FUNCTION_CALL
    // GList* arguments;

    // Type ASIGN and DEFINE
    // enum ValueType valueType;

    // Type STATEMENTS
    // GList* body;
};