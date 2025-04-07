function fibo (Integer number) -> Integer:

	if number < 2:
		return number
	else:
		return fibo(number - 1) + fibo(number - 2)

fibo(10)
