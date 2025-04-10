function funName () -> Integer:
	return 15

function funName2 ( Integer x ) -> Integer:
	return x * 2

function funName3 () -> Integer :
	return funName2(3) + funName2(2)

printline(funName() + funName2(2) + funName3() + funName2(3))

function testRet () -> Integer:
	if 1 > 2:
		return 5
	return 6

printline(testRet())