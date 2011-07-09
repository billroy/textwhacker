
def finish():
	pass



f = open('tiny.font', 'r')

def getline():
	l = f.readline()
	if not l:
		finish()
		sys.exit(0)


// skip comments
while True:
	l = getline()
	print l
while 