# Demonstrating list implications
def plusminus(a):
  # Imply returning an int list
  return [a + 1, a - 1]

counter = 1
# Complex list type iteration example
for i in [plusminus(4), plusminus(42), plusminus(-5402)]:
  print(counter, "+- 1:")
  for j in i:
    # Iterating the return value, since it's just an int list
    print(j)
  counter = counter + 1

