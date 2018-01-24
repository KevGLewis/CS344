import uuid
import string
import random
import sys

def create_rand_file():
	"""Creates a file and saves 10 random characters to it"""
	"""tempFile = str(uuid.uuid4()) + ".tmp"""
	tempFile = "test.txt"
	file = open(tempFile,"w+")
	
	for i in range (10):
		q = str(random.choice(string.ascii_lowercase))	
				
		if i == 9:
			q += '\n'

		sys.stdout.write(q)
		file.write(q)
		
	file.close

for i in range (3):
	create_rand_file()

x = random.randint(1, 43)
y = random.randint(1, 43)
z = x * y
sys.stdout.write(str(x) + '\n' + str(y) + '\n' + str(z) + '\n')
