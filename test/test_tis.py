import tis 

program = tis.Program()
with open('./test/programs/divide.sys',"r") as f:
 program.LoadSystem(f)

with open('./test/programs/move.tis',"r") as d:
 program.LoadCode(d)

print str(program.GetNode(0))