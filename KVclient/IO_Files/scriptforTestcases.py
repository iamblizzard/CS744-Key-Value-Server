import sys, os, random
f = open("batchRun.txt", "w")
r = open("expectedbatchResponses.txt", "w")

seneca  = "All cruelty springs from weakness."
epictetus = "Men are disturbed not by things but by the view which they take of them."
aurellis = "Everything we hear is an opinion and not a fact. Everything we see is a perspective and not the truth."
socrates = "When the debate is over slander becomes the tool of the loser."
plato  = "Be kind for everyone you meet is fighting a hard battle."
nietzsche = "To live is to suffer and to survive is to find some meaning in the suffering."
request_types = ["PUT,", "GET,","DEL,"]

keys = [seneca,epictetus,socrates, aurellis,plato]

values =["Seneca", "Epictetus","Marcus","Socrates", "Plato"]
request_types = ["PUT,", "GET,","DEL,"]

maxvaluesize = 262144
maxkeysize = 256

num_of_requests = 300
initital_puts = 30

data_dict = {}

for i in range(initital_puts):
	value = "a"*int(maxvaluesize * random.uniform(0.5, 1))
	data_dict["key" + str(i)] = value
	f.write("PUT," + "key" + str(i) + "," + value + "\n")
	r.write("Success\n")

for i in range(initital_puts, num_of_requests, 1):
	request_type_idx = random.randint(0, len(request_types) - 1)
	request = request_types[request_type_idx]

	choice = random.uniform(0, 1)
	
	if request == "PUT,":
		# new put
		if choice < 0.5 or len(data_dict) == 0:
			key = "key"+str(i)
			value = "a"*int(maxvaluesize * min((choice+0.2), 1))
		# overwrite put
		else:
			key, value = random.choice(list(data_dict.items()))

		f.write("PUT," + key + "," + value + "\n")
		r.write("Success\n")

	else:
		# unsuccessful get or unsuccessful del
		if choice < 0.5 or len(data_dict) == 0:
			key = "key"+str(i)
			r.write("Does not exist\n")
		# successful get or successful del
		else:
			key, value = random.choice(list(data_dict.items()))
			if request == "DEL,":
				del data_dict[key]
				r.write("Success\n")
			else:
				r.write(key + "," + value+"\n")
		f.write(request + key + "\n")

f.close()
r.close()