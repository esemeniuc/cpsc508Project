# Calculates averages and standard deviations for all the statistics output by perf
import sys 
import math as m

# Add a value to the dict entry 
def add_to_dict(field, line, dict):
    stats = [x.replace(",", "") for x in line.split()]

    if (field in dict):
        dict[field] += int(stats[0])
    else:
        dict[field] = 0
        dict[field] += int(stats[0])

def add_sd_to_dict(field, line, stat_map, stdev_map):
    stats = [x.replace(",", "") for x in line.split()]

    if (field in stdev_map):
        stdev_map[field] += ((int(stats[0]) - stat_map[field])**2)
    else:
        stdev_map[field] = 0
        stdev_map[field] += ((int(stats[0]) - stat_map[field])**2)

def truncate(number, digits):
    stepper = 10.0 ** digits
    return m.trunc(stepper * number) / stepper

filename = sys.argv[1]
stat_map = dict() # dictionary to hold the <K, V> stat pairs
stdev_map = dict() # dictionary to hold standard deviations
num_samples = 0 # used to compute averages 

# Open the file and read it line by line, extracting relevant stats 
with open(filename, "r") as f: 
    for line in f:
        if "stats" in line:
            num_samples += 1
        elif "DTLBWALKS" in line:
            add_to_dict("DTLBWALKS", line, stat_map)
        elif "context-switches" in line:
            add_to_dict("context-switches", line, stat_map)
        elif "cpu-migrations" in line:
            add_to_dict("cpu-migrations", line, stat_map)
        elif "page-faults" in line:
            add_to_dict("page-faults", line, stat_map)
        elif "cycles" in line:
            add_to_dict("cycles", line, stat_map)
        elif "instructions" in line:
            add_to_dict("instructions", line, stat_map)
        elif "branches" in line:
            add_to_dict("branches", line, stat_map)
        elif "branch-misses" in line:
            add_to_dict("branch-misses", line, stat_map)
        elif "L1-dcache-loads" in line:
            add_to_dict("L1-dcache-loads", line, stat_map)
        elif "L1-dcache-load-misses" in line:
            add_to_dict("L1-d-cache-load-misses", line, stat_map)
        elif "LLC-loads" in line:
            add_to_dict("LCC-loads", line, stat_map)
        elif "LLC-load-misses" in line:
            add_to_dict("LLC-load-misses", line, stat_map)
        # elif "L1-icache-loads" in line:
        #     add_to_dict("L1-icache", line, stat_map)
        elif "L1-icache-load-misses" in line:
            add_to_dict("L1-icache-load-misses", line, stat_map)
        elif "dTLB-loads" in line:
            add_to_dict("dTLB-loads", line, stat_map)
        elif "dTLB-load-misses" in line:
            add_to_dict("dTLB-load-misses", line, stat_map)
        elif "iTLB-loads" in line:
            add_to_dict("iTLB-loads", line, stat_map)
        elif "iTLB-load-misses" in line:
            add_to_dict("iTLB-load-misses", line, stat_map)
    
    for key in stat_map:
        stat_map[key] /= num_samples

with open(filename, "r") as f:
    for line in f:
        if "context-switches" in line:
            add_sd_to_dict("context-switches", line, stat_map, stdev_map)
        elif "DTLBWALKS" in line:
            add_to_dict("DTLBWALKS", line, stat_map, stdev_map)
        elif "cpu-migrations" in line:
            add_sd_to_dict("cpu-migrations", line, stat_map, stdev_map)
        elif "page-faults" in line:
            add_sd_to_dict("page-faults", line, stat_map, stdev_map)
        elif "cycles" in line:
            add_sd_to_dict("cycles", line, stat_map, stdev_map)
        elif "instructions" in line:
            add_sd_to_dict("instructions", line, stat_map, stdev_map)
        elif "branches" in line:
            add_sd_to_dict("branches", line, stat_map, stdev_map)
        elif "branch-misses" in line:
            add_sd_to_dict("branch-misses", line, stat_map, stdev_map)
        elif "L1-dcache-loads" in line:
            add_sd_to_dict("L1-dcache-loads", line, stat_map, stdev_map)
        elif "L1-dcache-load-misses" in line:
            add_sd_to_dict("L1-d-cache-load-misses", line, stat_map, stdev_map)
        elif "LLC-loads" in line:
            add_sd_to_dict("LCC-loads", line, stat_map, stdev_map)
        elif "LLC-load-misses" in line:
            add_sd_to_dict("LLC-load-misses", line, stat_map, stdev_map)
        # elif "L1-icache-loads" in line:
        #     add_to_dict("L1-icache", line, stdev_map)
        elif "L1-icache-load-misses" in line:
            add_sd_to_dict("L1-icache-load-misses", line, stat_map, stdev_map)
        elif "dTLB-loads" in line:
            add_sd_to_dict("dTLB-loads", line, stat_map, stdev_map)
        elif "dTLB-load-misses" in line:
            add_sd_to_dict("dTLB-load-misses", line, stat_map, stdev_map)
        elif "iTLB-loads" in line:
            add_sd_to_dict("iTLB-loads", line, stat_map, stdev_map)
        elif "iTLB-load-misses" in line:
            add_sd_to_dict("iTLB-load-misses", line, stat_map, stdev_map)


    for res in stdev_map:
        stdev_map[res] /= num_samples
        stdev_map[res] = m.sqrt(stdev_map[res])
    
    print("SUMMARY -- STATS FOR " + str(filename) + ":")
    for key in stat_map:
        print(key + ": " + str(stat_map[key]) + " (+/-) " + str(truncate(stdev_map[key], 2)))