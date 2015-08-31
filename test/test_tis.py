import tis 

print "Creating Program"
program = tis.Program()


print "opening divide.sys"
with open('./test/programs/divide.sys',"r") as f:
 program.LoadSystem(f)
print "closed divide.sys"

print "opening divide.tis"
with open('./test/programs/divide.tis',"r") as d:
 program.LoadCode(d)
print "closed divide.tis"

count = 0

'''print str(program.GetNode(0))

print "starting progam"
while program.Tick():
 pass
print "finished program"'''