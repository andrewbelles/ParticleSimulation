# Imports
import ctypes
import numpy as np
import matplotlib.pyplot as plt



# Declared shared library to pull c functions from 
c = ctypes.CDLL('./python_integration/benchmark.so')

# Declared structure with matching types to the results of collision_benchmark
class collision_results(ctypes.Structure):
  _fields_ = [('time', ctypes.POINTER(ctypes.c_double)),
              ('f_size', ctypes.POINTER(ctypes.c_int)),
              ('n_maps', ctypes.POINTER(ctypes.c_int))]

# Declared return and argument types of partition benchmark
c.partition_benchmark.restype = ctypes.POINTER(ctypes.c_double)
c.partition_benchmark.argtypes = [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int]

# Template function to free memory made by c functions called by py code
c.free_memory.argtypes = [ctypes.c_void_p]

# Declare arrays to pass through to benchmarks
sizes = np.array(list(range(1, 101)), dtype=np.int32)
iteration = np.array(list(range(1, 11)), dtype=np.int32)
particle_ct = 50
dt = 1e-4

times = np.zeros((len(iteration), len(sizes)))

print('Processing Partition Data')
for i in range(len(iteration)):
  time_ptr = c.partition_benchmark(sizes.ctypes.data_as(ctypes.POINTER(ctypes.c_int)), len(sizes), particle_ct)
  time_array = np.ctypeslib.as_array(time_ptr, shape=(len(sizes),))

  times[i] = time_array
  
  del time_array
  c.free_memory(ctypes.cast(time_ptr, ctypes.c_void_p))

# Stat calculation variables
tmean1 = np.mean(times, axis=0)

print('Collected Collision Data')

# Declared ret. and arg types of collision benchmark
c.collision_benchmark.restype = ctypes.POINTER(collision_results)
c.collision_benchmark.argtypes = [ctypes.c_int, ctypes.c_int]

run_time = np.array(list(range(1,10001)), dtype=np.int32)
col_times = np.zeros((len(iteration), len(run_time)))
col_sizes = np.zeros((len(iteration), len(run_time)))
col_nmaps = np.zeros((len(iteration), len(run_time)))

print('Processing Collision Data')
# Collect Results from collision benchmark test

for i in range(len(iteration)):
  # Collect results for iteration
  result_ptr = c.collision_benchmark(len(run_time), particle_ct)
  result = result_ptr.contents

  # Store in ith iteration of each array
  col_times[i] = np.ctypeslib.as_array(result.time, shape=(len(run_time),))
  col_sizes[i] = np.ctypeslib.as_array(result.f_size, shape=(len(run_time),))
  col_nmaps[i] = np.ctypeslib.as_array(result.n_maps, shape=(len(run_time),))

  # Free src
  c.free_memory(result.time)
  c.free_memory(result.f_size)
  c.free_memory(result_ptr)

print('Collected Collision Data')

tmean2 = np.mean(col_times, axis=0)
smean = np.mean(col_times, axis=0)
mmean = np.mean(col_nmaps, axis=0)

# Plotting for two benchmarks
plt.figure(figsize=(12, 8))
plt.subplot(2, 2, 1)
plt.plot(sizes, tmean1)

plt.title('Ave Time to Create Map of n^3 size')
plt.xlabel('Partition Size')
plt.ylabel('Time (ms)')

plt.subplot(2, 2, 2)
plt.hist(tmean1, bins=50, edgecolor='black')
plt.xlabel('Average Time')
plt.ylabel('Count')

plt.subplot(2, 2, 3)

plt.hist(tmean2, bins=200, edgecolor='black')
plt.xlim(0, 1)
sim_time = len(run_time) * dt
plt.title(f'Average Time to simulate {sim_time} seconds')
plt.xlabel('Average Time')
plt.ylabel('Count')

plt.subplot(2, 2, 4)
for i in range(len(iteration)):
  plt.scatter(col_sizes[i], col_nmaps[i], alpha=0.5)

plt.title('Size of Final Map vs. Number of Map Creation Attempts')
plt.xlabel('Final Size (n^3)')
plt.ylabel('Number of Attempts')

plt.tight_layout()
plt.show()