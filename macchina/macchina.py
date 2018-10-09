import json
import sys
from carla import *

class Macchina:

    def __init__(self, client, x, y, z, angle=0):
        self.scene = {"map": []}
        self.client = client

        # Spawn generator
        world = client.get_world()
        bp = world.get_blueprint_library()
        msh = bp.find("meshholder.mesh")
        msh_loc = Location(x=x, y=y, z=z)
        transform = Transform(msh_loc, Rotation(yaw=angle))
        self.generator = world.spawn_actor(msh, transform)

    def addObject(self, objectTag, x, y, angle=0):
        self.scene["map"].append({"obj": int(objectTag), "x": x, "y": y, "angle": angle})

    def spawn(self):
        self.generator.set_scene(json.dumps(self.scene))
