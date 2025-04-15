Integer integer = 123
Decimal decimal = 123.123
Boolean boolean = True

if integer == 123:
	integer = 456
elseIf integer == 789:
	decimal = 111.1
else:
	boolean = False

while integer < 500:
	integer = integer + 1
	if integer == 480:
		continue
	if integer == 499:
		break

function fun():
	decimal = 222.2

function doubleIt(Integer it) -> Integer:
	return 2 * it

function add(Decimal a, Decimal b) -> Decimal:
	return a + b

fun()

printLineInteger(doubleIt(16))

printLineDecimal(add(1.2, 2.4))