import math as mt

#convert values


#velocity up convert
min_phys_val = -999.99
max_phys_val = 999.99

min_tx_val = -32768
max_tx_val = 32767

phys_val = 50.0

tx_val = (max_tx_val - min_tx_val)/(max_phys_val - min_phys_val)*(phys_val - min_phys_val)  + min_tx_val

print("velocity up: " + str(mt.ceil(tx_val)))

#velocity left convert
min_phys_val = -999.99
max_phys_val = 999.99

min_tx_val = -32768
max_tx_val = 32767

phys_val = -50.0

tx_val = (max_tx_val - min_tx_val)/(max_phys_val - min_phys_val)*(phys_val - min_phys_val)  + min_tx_val

print("velocity left: " + str(mt.ceil(tx_val)))

#velocity forward convert
min_phys_val = -999.99
max_phys_val = 999.99

min_tx_val = -32768
max_tx_val = 32767

phys_val = 0.0

tx_val = (max_tx_val - min_tx_val)/(max_phys_val - min_phys_val)*(phys_val - min_phys_val)  + min_tx_val

print("velocity forward: " +str(mt.ceil(tx_val)))

#heading convert
min_phys_val = -180.0
max_phys_val = 180.0

min_tx_val = -32768
max_tx_val = 32767

phys_val = 100.0

tx_val = (max_tx_val - min_tx_val)/(max_phys_val - min_phys_val)*(phys_val - min_phys_val)  + min_tx_val

print("heading: " + str(mt.ceil(tx_val)))

#altitude convert
min_phys_val = -1000
max_phys_val = 1000

min_tx_val = -32768
max_tx_val = 32767

phys_val = 500.5

tx_val = (max_tx_val - min_tx_val)/(max_phys_val - min_phys_val)*(phys_val - min_phys_val)  + min_tx_val

print("altitude: " + str(mt.ceil(tx_val)))

#pitch convert
min_phys_val = -180.0
max_phys_val = 180.0

min_tx_val = -32768
max_tx_val = 32767

phys_val = -100.0

tx_val = (max_tx_val - min_tx_val)/(max_phys_val - min_phys_val)*(phys_val - min_phys_val)  + min_tx_val

print("pitch: " + str(mt.ceil(tx_val)))

#roll convert
min_phys_val = -180.0
max_phys_val = 180.0

min_tx_val = -32768
max_tx_val = 32767

phys_val = 21.0

tx_val = (max_tx_val - min_tx_val)/(max_phys_val - min_phys_val)*(phys_val - min_phys_val)  + min_tx_val

print("roll: " + str(mt.ceil(tx_val)))

