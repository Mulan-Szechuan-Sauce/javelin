# These should all have the same type signature, since we have implicit typing

def identity0(a : int) -> int:
    return a
def identity1(a : int):
    return a
def identity2(a) -> int:
    return a
def identity3(a):
    return 0 + a
def identity4(a):
    return a + 0
def identity5(a):
    b = 0 + a
    return b
def identity6(a):
  if 0:
    # Silly implication example
    return 0
  return a
