Integer n = 40

Integer t1 = 0
Integer t2 = 1
Integer next = t1 + t2

Integer i = 3

while i < n + 1:
	t1 = t2
	t2 = next
	next = t1 + t2

	i = i + 1

printLineInteger(next)