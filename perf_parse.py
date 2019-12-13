import pandas
import fileinput, sys
from io import StringIO

filename = sys.argv[1]
# parse file and break it down into frames for each run
for line in fileinput.input(filename, inplace=1, backup='.orig'):
    if line.startswith("#"):
        print("NewDataFrame\n")
    else:
        print(line[:-1])
fileinput.close()

with open(filename, "r") as f:
    dataframes = []
    stringy = f.read()
    stringy = stringy.split("NewDataFrame")
    for frames in stringy:
        if frames != "":
            #  parse text into frame for panda
            temp = StringIO(frames)
            csvdata = pandas.read_csv(temp, "|", header=None)
            updated = csvdata.drop(columns=[1, 3, 4, 5, 6])
            updated = updated.reindex(columns=[2, 0])

            updated = updated.transpose()
            updated.columns = updated.iloc[0]
            updated = updated.drop([2])
            dataframes.append(updated)
    #  concat all frames together
    final = dataframes[0]
    for frame in dataframes[1:]:
        final = final.append(frame, ignore_index=True)
    cols = final.columns
    final[cols] = final[cols].apply(pandas.to_numeric, errors='coerce')
    print(final.mean())
