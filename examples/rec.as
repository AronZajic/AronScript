function rec (Integer x):
	printLineInteger(x)
	if x > 0:
		rec(x - 1)
	else:
		0

rec(5)