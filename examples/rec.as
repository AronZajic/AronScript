function rec (Integer x):
	printline(x)
	if x > 0:
		rec(x - 1)
	else:
		0

rec(5)