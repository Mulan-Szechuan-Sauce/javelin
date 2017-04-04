def fib(n):
  if n == 0 or n == 1:
    return 1
  return fib(n - 1) + fib(n - 2)

i = 0
while i < 10:
  print("Fibonacci number " + str(i) + ": " + str(fib(i)))
  i = i + 1

