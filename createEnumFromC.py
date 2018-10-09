test = open("needsConversion", "r")
split = test.readlines()
out = """from enum import Enum

class MeshTag(Enum):
"""
i = 0
for l in split:
    if "*" in l:
        continue

    stripped = [x for x in l.replace("\n", "").split(" ") if x != "" and x != "\n"]
    if len(stripped) == 0:
        continue

    out += "\t" + stripped[0] + " = " + str(i) + "\n"
    i += 1

f = open("macchina/meshtag.py", "w")
f.write(out)
f.close()
