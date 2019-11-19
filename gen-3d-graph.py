import sys 
sys.path.append("/usr/local/lib/python3.7/site-packages")
from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt

fig = plt.figure()
ax = plt.axes(projection="3d")
xs = [1, 1, 2, 2, 3, 3]
ys = [1, 2, 1, 2, 1, 2]
zs = [0, 2340040874, 1035, 222, 1511, 102]
ax.scatter(xs, ys, zs, c=["red", "blue", "red", "blue", "red", "blue"])

plt.show()