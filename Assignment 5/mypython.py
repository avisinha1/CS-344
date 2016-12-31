#Aviral Sinha
#CS 344
#Program 5: Python Exploration
#Program creates 3 new files that 
#have 10 random lowercase letters and when executed, 
#the values of the file will be printed.



import sys
import random
import string

content_array = [] #variable declaration
lower_letters = ""

print("\n")
print("3 files with 10 random lower letters each: \n")

for i in range(0, 3): #creates 3 strings
	for j in range(0,10): #fills each string with random letters
		lower_letters = lower_letters + str(random.choice(string.ascii_lowercase))

	content_array.append(lower_letters)

	f = "file" + str(i+1) + ".txt" #create a file and the elements are filled
	f2 = open(f, 'w')
	f2.write(content_array[i] + "\n")
	f2.close()

	print("File" + str(i+1) + "contents: " + content_array[i]) #output to console

	lower_letters = "" #reset lower letters

print("\n")

num1 = 0  #declare variables
num2 = 0
product = 0

num1 = random.randint(1, 42)
num2 = random.randint(1, 42)

product = num1 * num2

print("--------")
print("Numbers multiplied")
print("Number 1: " + str(num1))
print("Number 2: " + str(num2))
print(str(num1) + " X " + str(num2) + " = " + str(product))
print("\n")
