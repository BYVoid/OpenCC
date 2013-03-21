#coding: utf-8

def sort_items(input_filename, output_filename):
  input_file = open(input_filename, "r")
  dic = {}
  
  for line in input_file:
    if len(line) == 0:
      continue
    try:
      key, value = line.split("\t")
    except ValueError:
      print line
    while value[-1] == "\n" or value[-1] == "\r":
      value = value[:-1]
    dic[key] = value
  
  input_file.close()
  
  output_file = open(output_filename, "w")
  
  for key in sorted(dic.iterkeys()):
    output_file.write(key + "\t" + dic[key] + "\n")
    
  output_file.close()

def reverse_items(input_filename, output_filename):
  input_file = open(input_filename, "r")
  dic = {}
  
  for line in input_file:
    if len(line) == 0:
      continue
    key, value = line.split("\t")
    while value[-1] == "\n" or value[-1] == "\r":
      value = value[:-1]
    
    value_list = value.split(" ")
    for value in value_list:
      if dic.has_key(value):
        dic[value].append(key)
      else:
        dic[value] = [key]
  
  input_file.close()
  
  output_file = open(output_filename, "w")
  
  for key in sorted(dic.iterkeys()):
    output_file.write(key + "\t" + " ".join(dic[key]) + "\n")
    
  output_file.close()

def find_target_items(input_filename, keyword):
  input_file = open(input_filename, "r")
  for line in input_file:
    if len(line) == 0:
      continue
    key, value = line.split("\t")
    while value[-1] == "\n" or value[-1] == "\r":
      value = value[:-1]
    
    value_list = value.split(" ")
    for value in value_list:
      if keyword in value:
        print line,
  
  input_file.close()
