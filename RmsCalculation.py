import csv
from math import sqrt
import matplotlib.pyplot as plt


errorValue = []
errorValueInt = []
errorValueSingled = []
errorValueComma = []
errorValueCommaSep = []

expected = 0
sum = 0
meanSquareError = 0
rootMeanSquareError = 0

# read in Errors file that contains the error values as hex values and spaces
with open('Errors.txt', 'r') as eF:
    read = csv.reader(eF)
    for line in read:
        for value in line:
            errorValue.append(value) # read line into variable

for x in range(len(errorValue)):
    errorValueSingled.append(errorValue[:][x][4:]) # strip the 'er: '


for line in errorValueSingled:
    errorValueComma = line.replace(' ', ',') # replace whitespace with comma
    #print(errorValueComma)
    errorValueCommaSep.append(errorValueComma.split(",")) # add the resultant to new variable

for line in errorValueCommaSep:
    for value in line:
        errorValueInt.append(int(value,16)) # convert each value to it's integer value

# write int values to new csv file
with open('ErrorsInt.csv', 'w') as file:
    for value in errorValueInt:
        file.write(str(value))
        file.write(('\n'))

for value in errorValueInt:
    sum = sum + pow((value - expected), 2) # calculate the value minus expected all squared

meanSquareError = sum / len(errorValueInt) # calculate mean square error

rootMeanSquareError = sqrt(meanSquareError) # calculate root mean square error
print("RMS value = " + str(rootMeanSquareError))

plt.plot(errorValueInt)
plt.ylabel('Error Values')
plt.xlabel('Error Number')
plt.show()




#print(errorValueInt)
#print(errorValueComma)
#print(len(errorValue))
#print(errorValue[:][0][4:])
#print(errorValueSingled)
