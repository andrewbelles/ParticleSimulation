import ctypes
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

libc = ctypes.CDLL('./python_integration/benchmark.so')

libc.partition_benchmark.restype = ctypes.POINTER(ctypes.c_double)
libc.partition_benchmark.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int]

libc.free_memory.argtypes = [ctypes.c_void_p]

sizes = np.array(list(range(1, 101)), dtype=np.int32)

TimeArray = ctypes.c_double * len(sizes)
time = libc.partition_benchmark(sizes.ctypes.data_as(ctypes.POINTER(ctypes.c_int)), len(sizes), 25)
time = ctypes.cast(time, ctypes.POINTER(TimeArray))

for i in range(len(sizes)):
  print(time.contents[i])

libc.free_memory(ctypes.cast(time, ctypes.c_void_p))

