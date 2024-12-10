# Imports
import ctypes as ct
import argparse
# Render Imports
import pygame
from pygame.locals import *
import OpenGL.GL as GL
import OpenGL.GLUT as GLUT
import OpenGL.GLU as GLU

# Structure Definition to communicate positions between update loop and renderer
# Simple vector3 c type structure
class Vec3(ct.Structure):
  _fields_ = [('x', ct.c_double),
              ('y', ct.c_double),
              ('z', ct.c_double)]
  
  def __str__(self):
    return f'  X: {self.x:.2f}\n  Y: {self.y:.2f}\n  Z: {self.z:.2f}'
  
# ^ For 3D boolean
class Short3(ct.Structure):
  _fields_ = [('x', ct.c_short),
              ('y', ct.c_short),
              ('z', ct.c_short)]

# Cube defn to pass as arg 
class Cube(ct.Structure):
  _fields_ = [('origin', Vec3),
              ('min', Vec3),
              ('max', Vec3),
              ('size', ct.c_double)]
  
# C Linked list structure defn for object to pass in update loop
class Object(ct.Structure):
  pass
Object._fields_ = [('next', ct.POINTER(Object)),
                  ('id', ct.c_char_p),
                  ('position', Vec3),
                  ('velocity', Vec3),
                  ('acceleration', Vec3),
                  ('wall', Short3)]

# C Hashtable defn for map which will be updated in update loop
class Map(ct.Structure):
  pass
Map._fields_ = [('object', ct.POINTER(Object)),
                ('count', ct.c_int),
                ('next', ct.POINTER(Map))]


# Main
def main():
  parser = argparse.ArgumentParser(description="Accepts integer # of particles to simulate.")
  parser.add_argument('particle_ct', type=int, help='n particles')
  parser.add_argument('cube_size', type=int, help='Side length of cube') 

  args = parser.parse_args()

  particle_ct = ct.c_int(args.particle_ct)
  side_len = args.cube_size
  cube = Cube(Vec3(0, 0, 0), Vec3(0, 0, 0), Vec3(side_len, side_len, side_len), side_len)

  # Initialize head of particle l_list and hashtable
  n_partitions = ct.c_int(8)
  head = ct.POINTER(Object)()
  map = ct.POINTER(ct.POINTER(Map))()
  head = c.initializeObjects(particle_ct)

  # Renderer initialization
  pygame.init()
  window_dimensions = (800, 600)
  window = pygame.display.set_mode(window_dimensions)

  i = 0
  end = False
  # Update Loop
  while True:

    # Handle events
    for event in pygame.event.get():
      # Exit Program Selected
      if event.type == pygame.QUIT:
        pygame.quit()
        quit()
        end = True

      # Key press events
      if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_z:
          # add object
          c.appendObject(head)
        if event.key == pygame.K_x:
          # delete object
          c.deleteObject(head, ct.byref(particle_ct))
        if event.key == pygame.K_ESCAPE:
          pygame.quit()
          quit()
          end = True
    
    if end:
      break
    
    # Update positions, check collision map, rectify collisions and oob
    updateStatus = c.updateCall(ct.byref(map), cube, ct.byref(head), i, ct.byref(n_partitions))
    if updateStatus == False:
      print('Error! Aborting')
      break

    # Display concurrent position
    c.print_positions(head)
    i += 1

  # Free allocated memory in C
  c.destroy_map(map)
  c.destroy_objects(head)

# Pre-Main Calls
# Definition of C library that will be specifically pulled from
c = ct.CDLL('./python_integration/fast_collisionMath.so')

# Return and arg types of initializeObjects (c func will be a python representation of the c type)
c.initializeObjects.restype = ct.POINTER(Object)
c.initializeObjects.argtypes = [ct.c_int]

c.appendObject.argtypes = [ct.POINTER(Object)]
c.deleteObject.argtypes = [ct.POINTER(Object), ct.POINTER(ct.c_int)]
c.print_positions.argtypes = [ct.POINTER(Object)]

# ret & arg types of updateCall (see above)
c.updateCall.restype = ct.c_int
c.updateCall.argtypes = [ct.POINTER(ct.POINTER(ct.POINTER(Map))), Cube, ct.POINTER(ct.POINTER(Object)), ct.c_int, ct.POINTER(ct.c_int)]

# Free calls
c.free_memory.argtypes = [ct.c_void_p]
c.destroy_map.argtypes = [ct.POINTER(ct.POINTER(Map)), ct.c_int]
c.destroy_objects.argtypes = [ct.POINTER(Object)]

# Call to main
if __name__ == "__main__":
  main()