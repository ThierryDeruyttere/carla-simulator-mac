import sys
import math
import time
import os
import math
sys.path.append(
    'PythonAPI/dist/carla-0.9.0-py2.7-macosx-10.13-intel.egg')
from macchina.macchina import Macchina
from macchina.meshtag import MeshTag
from macchina.image_converter import labels_to_cityscapes_palette
from carla import *

client = Client("localhost", 2000)
world = client.get_world()
spectator = world.get_spectator()
HALF_PI = math.pi/2
PI = math.pi

def get_transform(angle, d=6.5):
    a = math.radians(angle)
    location = Location(d * math.cos(a), d * math.sin(a), 2.0) + LOC
    return Transform(location, Rotation(yaw=0, pitch=-15))

def createRoad():
    x = 0
    y = 0
    macchina = Macchina(client, x=0, y=0, z=2, angle=0)

    # First road
    macchina.addObject(MeshTag.RoadTwoLanes_LaneLeft,          x, y, HALF_PI)
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkLeft,      x, y, HALF_PI)
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkRight,     -400, y, HALF_PI)

    # second road
    y = 810.0
    macchina.addObject(MeshTag.RoadTwoLanes_LaneLeft,          x, y, HALF_PI)
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkLeft,      x, y, HALF_PI)
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkRight,     -400, y, HALF_PI)

    # Horizontal
    y = 1202.0
    macchina.addObject(MeshTag.RoadTwoLanes_LaneLeft,          -200, y)
    macchina.addObject(MeshTag.RoadTwoLanes_LaneLeft,          610, y)
    macchina.addObject(MeshTag.RoadTwoLanes_LaneLeft,          -1010, y)

    y = 1623.0
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkRight,     430, y)
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkRight,     -810, y)

    # second road
    y = 2020.0
    macchina.addObject(MeshTag.RoadTwoLanes_LaneLeft,          x, y, HALF_PI)
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkLeft,      x, y, HALF_PI)
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkRight,     -410, y, HALF_PI)


    y = 1203.0
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkLeft,      400, y, 0)
    macchina.addObject(MeshTag.RoadTwoLanes_SidewalkRight,     -815, y, PI)

    # props
    macchina.addObject(MeshTag.Props_Trashcan,     -615, 810, PI)
    macchina.addObject(MeshTag.Props_FireHydrant,  200, 810, PI)

    # part 2
    #macchina.addObject(MeshTag.Props_Bench,  -615, 500, 3 * HALF_PI)
    #macchina.addObject(MeshTag.Props_Bench,  100, 500, HALF_PI)

    #macchina.addObject(MeshTag.Trashcan,     -400, y, PI)

    # House
    #macchina.addObject(MeshTag.House_AmerSuburb002_N2, 700, 400, HALF_PI)
    #macchina.addObject(MeshTag.House_AmerSuburb003_N2, -1100, 400, 3 * HALF_PI)
    #macchina.addObject(MeshTag.House_AmerSuburb005_N5, -1100, 2300, 3 * HALF_PI)
    #macchina.addObject(MeshTag.House_AmerSuburb006_N2, -170, 3030, PI)
    #macchina.addObject(MeshTag.House_AmerSuburb006_N2, 600, 2300,  HALF_PI)

    # Vehicle
    #macchina.addObject(MeshTag.Vehicle_Toyota_prius, 300, 1300, HALF_PI)
    #macchina.addObject(MeshTag.Vehicle_mini, -300, 800, 0)
    return macchina

def createHouses():
    macchina = Macchina(client, x=0, y=0, z=2, angle=0)

    macchina.addObject(MeshTag.House_AmerSuburb002_N2, 700, 400, HALF_PI)
    macchina.addObject(MeshTag.House_AmerSuburb003_N2, -1100, 400, 3 * HALF_PI)
    macchina.addObject(MeshTag.House_AmerSuburb005_N5, -1100, 2300, 3 * HALF_PI)
    #macchina.addObject(MeshTag.House_AmerSuburb006_N2, -170, 3030, PI)
    macchina.addObject(MeshTag.House_AmerSuburb006_N2, 600, 2300,  HALF_PI)
    return macchina

def createBenches():
    macchina = Macchina(client, x=0, y=0, z=2, angle=0)
    macchina.addObject(MeshTag.Props_Bench,  -615, 500, 3 * HALF_PI)
    macchina.addObject(MeshTag.Props_Bench,  100, 500, HALF_PI)
    return macchina

def createVehicles():
    macchina = Macchina(client, x=0, y=0, z=2, angle=0)
    macchina.addObject(MeshTag.Vehicle_Toyota_prius, 300, 1300, HALF_PI)
    macchina.addObject(MeshTag.Vehicle_mini, -300, 800, 0)
    return macchina


def save_to_disk(image, name):
    """Save this image to disk (requires PIL installed)."""

    filename = '{}/{:0>6d}_{:s}.png'.format(name, image.frame_number, image.type)

    try:
        from PIL import Image as PImage
    except ImportError:
        raise RuntimeError(
            'cannot import PIL, make sure pillow package is installed')

    image = PImage.frombytes(
        mode="RGBA",
        size=(image.width, image.height),
        data=image.raw_data,
        decoder_name='raw')

    color = image.split()
    image = PImage.merge("RGB", color[2::-1])

    folder = os.path.dirname(filename)
    if not os.path.isdir(folder):
        os.makedirs(folder)
    image.save(filename)

i = 1
def save_semantic_img(image, name):
    global i
    res = labels_to_cityscapes_palette(image)

    filename = '{}/{}.png'.format(name,i)
    i+=1
    try:
        from PIL import Image as PImage
    except ImportError:
        raise RuntimeError(
            'cannot import PIL, make sure pillow package is installed')

    image = PImage.frombytes(
        mode="RGB",
        size=(res.shape[0], res.shape[1]),
        data=res,
        decoder_name='raw')

    #color = image.split()
    #image = PImage.merge("RGB", color[2::-1])

    folder = os.path.dirname(filename)
    if not os.path.isdir(folder):
        os.makedirs(folder)
    image.save(filename)


# Set
world = client.get_world()
bp = world.get_blueprint_library()
blueprint = bp.find('sensor.camera')
blueprint.set_attribute('post_processing', 'SceneFinal')
LOC = Location(x=-2, y=-4, z=3)
transform = Transform(LOC, Rotation(yaw=90))
cam1 = world.spawn_actor(blueprint, transform)
cam1.listen(lambda image: save_to_disk(image, "scene"))

blueprint2 = bp.find('sensor.camera')
blueprint.set_attribute('post_processing', 'SemanticSegmentation')
transform = Transform(LOC, Rotation(yaw=90))
cam2 = world.spawn_actor(blueprint, transform)
cam2.listen(lambda image: save_semantic_img(image, "semantic"))

time.sleep(4)
m1 = createRoad()
m1.spawn()

LOC = Location(x=-2, y=-4, z=3)
transform = Transform(LOC, Rotation(yaw=90))
spectator.set_transform(transform)

time.sleep(4)
m2 = createHouses()
m2.spawn()

time.sleep(4)
m3 = createBenches()
m3.spawn()

time.sleep(4)
m4 = createVehicles()
m4.spawn()
