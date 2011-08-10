#coding: utf-8

def sort_items(input_filename, output_filename):
	input_file = open(input_filename, "r")
	dic = {}
	
	for line in input_file:
		if len(line) == 0:
			continue
		key, value = line.split("\t")
		while value[-1] == "\n" or value[-1] == "\r":
			value = value[:-1]
		dic[key] = value
	
	input_file.close()
	
	output_file = open(output_filename, "w")
	
	for key in sorted(dic.iterkeys()):
		output_file.write(key + "\t" + dic[key] + "\n")
		
	output_file.close()

