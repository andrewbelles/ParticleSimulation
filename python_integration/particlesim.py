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
    return f'  X: {self.x:.4f}\n  Y: {self.y:.4f}\n  Z: {self.z:.4f}'
  
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
                  ('wall', Short3),
                  ('mass', ct.c_double),
                  ('radius', ct.c_double)]

# Grid array of bounds
class Grid(ct.Structure):
  _fields_ = [('bounds', Vec3 * 2)]

# C Hashtable defn for map which will be updated in update loop
class Map(ct.Structure):
  pass
Map._fields_ = [('object', ct.POINTER(Object)),
                ('count', ct.c_int),
                ('next', ct.POINTER(Map))]

# Definse the vertices and edges for given Cube structure for mesh
def drawSphere(Vec3, radius):
  y = Vec3.x
  x = Vec3.y
  z = Vec3.z
  slices = 40
  stacks = 40

  GL.glPushMatrix()
  GL.glTranslatef(x, y, z)

  GL.glColor3f(0.0, 0.66, 0.33)

  sphere = GLU.gluNewQuadric()
  GLU.gluQuadricNormals(sphere, GLU.GLU_SMOOTH)
  GLU.gluSphere(sphere, radius, slices, stacks)

  GL.glPopMatrix()

  # Check for error
  error = GL.glGetError()
  if error != GL.GL_NO_ERROR:
    print(f'drawSphere error: {error}')

def cube_mesh(Cube):
  min = int(Cube.min.x)
  max = int(Cube.size)
  vertices = (
    (min, min, min),
    (max, min, min),
    (max, max, min),
    (min, max, min),
    (min, min, max),
    (max, min, max),
    (max, max, max),
    (min, max, max),
  )
  edges = (
    (0, 1),
    (1, 2),
    (2, 3),
    (3, 0),
    (4, 5),
    (5, 6),
    (6, 7),
    (7, 4),
    (0, 4),
    (1, 5),
    (2, 6),
    (3, 7)
  )
  return vertices, edges

def drawCube(Cube):
  # Draw Cube by each vertext
  vertices, edges = cube_mesh(Cube)
  GL.glColor3f(1.0, 0.0, 0.0)
  for edge in edges:
    GL.glBegin(GL.GL_LINES)
    for vertex in edge:
      GL.glVertex3fv(vertices[vertex])
    GL.glEnd()

  # Check for error
  error = GL.glGetError()
  if error != GL.GL_NO_ERROR:
    print(f'drawCube error: {error}')


def setLighting():
  GL.glEnable(GL.GL_DEPTH_TEST)
  GL.glEnable(GL.GL_LIGHT0)

  light_position = [10.0, 10.0, 10.0, 1.0]
  light_ambient = [0.2, 0.2, 0.2, 1.0]
  light_diffuse = [0.8, 0.8, 0.8, 1.0]
  light_specular = [1.0, 1.0, 1.0, 1.0]
  material_ambient = [0.0, 0.5, 0.0, 1.0]
  material_diffuse = [0.0, 0.8, 0.0, 1.0]
  material_specular = [1.0, 1.0, 1.0, 1.0]
  shinyness = 50

  # Light inits
  GL.glLightfv(GL.GL_LIGHT0, GL.GL_POSITION, light_position)
  GL.glLightfv(GL.GL_LIGHT0, GL.GL_AMBIENT, light_ambient)
  GL.glLightfv(GL.GL_LIGHT0, GL.GL_DIFFUSE, light_diffuse)
  GL.glLightfv(GL.GL_LIGHT0, GL.GL_SPECULAR, light_specular)

  # Material inits
  GL.glMaterialfv(GL.GL_FRONT, GL.GL_AMBIENT, material_ambient)
  GL.glMaterialfv(GL.GL_FRONT, GL.GL_DIFFUSE, material_diffuse)
  GL.glMaterialfv(GL.GL_FRONT, GL.GL_SPECULAR, material_specular)
  GL.glMaterialf(GL.GL_FRONT, GL.GL_SHININESS, shinyness)

  GL.glShadeModel(GL.GL_SMOOTH)

# Main
def main():
  parser = argparse.ArgumentParser(description="Accepts integer # of particles to simulate.")
  parser.add_argument('particle_ct', type=int, help='n particles')
  parser.add_argument('cube_size', type=int, help='Side length of cube') 

  args = parser.parse_args()

  particle_ct = args.particle_ct
  side_len = args.cube_size
  cube = Cube(Vec3(0, 0, 0), Vec3(0, 0, 0), Vec3(side_len, side_len, side_len), side_len)

  # Initialize head of particle l_list and hashtable
  dt = 1e-3
  sub_steps = 8
  radius = 0.5
  objects = c.initializeObjects(particle_ct, radius)

  axis_ct = c.mapSize(objects, cube.size)
  axis_ct = 8
  partition_ct = axis_ct * axis_ct * axis_ct
  # print(f'Size: {partition_ct}')

  # Renderer initialization
  pygame.init()
  display = (800, 600)
  pygame.display.set_mode(display, DOUBLEBUF|OPENGL)
  GLU.gluPerspective(60, display[0]/display[1], 0.1, 50.0)
  GL.glTranslatef(-10, -5, -25)
  setLighting()

  end = False

# Update Loop
  while True:

    positions = c.read_positions(objects, particle_ct)

    # Clear color and depth buffer 
    GL.glClear(GL.GL_COLOR_BUFFER_BIT|GL.GL_DEPTH_BUFFER_BIT)
    # Rotate Matrix
    GL.glPushMatrix()

    GL.glRotatef(45, 0, 1, 0)
    
    # draw Cube without lighting
    drawCube(cube)

    # draw Sphere with lighting enabled
    GL.glEnable(GL.GL_LIGHTING)
    for i in range(particle_ct):
      drawSphere(positions[i], radius)
    GL.glDisable(GL.GL_LIGHTING)

    GL.glPopMatrix()

    # Handle events
    for event in pygame.event.get():
      # Exit Program Selected
      if event.type == pygame.QUIT:
        pygame.quit()
        quit()
        end = True

      # Key press events
      if event.type == pygame.KEYDOWN:
        if event.key == pygame.K_ESCAPE:
          pygame.quit()
          quit()
          end = True
    
    if end:
      break
    
    # Update positions, check collision map, rectify collisions and oob
    updateStatus = c.updateCall(cube, objects, particle_ct, axis_ct, dt, sub_steps)
    if updateStatus == False:
      print('Error! Aborting')
      break

    # Display concurrent position
    pygame.display.flip()

  # Free allocated memory in C
  c.free_memory(objects)

# Pre-Main Calls
# Definition of C library that will be specifically pulled from
c = ct.CDLL('./python_integration/fast_collisionMath.so')

# Returns the number of partitions on each axis
c.mapSize.restype = ct.c_int
c.mapSize.argtypes = [ct.POINTER(Object), ct.c_double]

# Return and arg types of initializeObjects (c func will be a python representation of the c type)
c.initializeObjects.restype = ct.POINTER(Object)
c.initializeObjects.argtypes = [ct.c_int, ct.c_double]

c.read_positions.restype = ct.POINTER(Vec3)
c.read_positions.argtypes = [ct.POINTER(Object), ct.c_int]

# int updateCall(Map **map[], const Cube cube, Object objects[], const int particle_ct, const double dt, const int sub_steps)
c.updateCall.restype = ct.c_int
c.updateCall.argtypes = [Cube, ct.POINTER(Object), ct.c_int, ct.c_int, ct.c_double, ct.c_int]

# Print object positions since python + ctypes is finicky with trying to print them in loop
c.print_positions.argtypes = [ct.POINTER(Object), ct.c_int]

# Free calls
c.free_memory.argtypes = [ct.c_void_p]
c.destroy_map.argtypes = [ct.POINTER(ct.POINTER(Map)), ct.c_int]

# Call to main
if __name__ == "__main__":
  main()
