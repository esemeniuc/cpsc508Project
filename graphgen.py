import sys 
# Note: I only need this on my system because mpl has a weird path issue
sys.path.append("/usr/local/lib/python3.7/site-packages")
from mpl_toolkits import mplot3d
import numpy as np
import matplotlib.pyplot as plt

# Wrapper to create a new figure and axis
def plot_figure(title, xlabel, ylabel, xticks=None):
    fig, ax = plt.subplots()
    ax.set_title(title)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    return (ax, fig)

# Wrapper to create a bar chart
def create_bar_chart(ax, fig, xdata, ydata, xticks=None, log=True):
    ax.bar(xdata, ydata, tick_label=xticks, log=log)



ydata_dram = [183897.16, 187320.044, 181281.336]
ydata_pmem = [185642.472, 189318.422, 185432.812]
xticks=["4K", "2M", "1G"]

ax_redis, fig_redis = plot_figure("GETs/second - PMEM vs DRAM", "Page Size", "Throughput (GETs/sec)")

line, = ax_redis.plot(xticks, ydata_dram, label="DRAM")
line.set_dashes([2, 2, 10, 2]) 
ax_redis.plot(xticks, ydata_pmem, label="PMEM")
ax_redis.legend()

# PMEM 1G Sequential Accesses FIO 
ax_bar_fio_pmem_1g, fig_bar_fio_pmem_1g = plot_figure("PMEM vd DRAM dTLB Misses (1GB Sequential Read - FIO)", "Page Size", "dTLB Misses")
ydata_fio_1g_dram_seq = [6502618, ]
ydata_fio_1gp_seq = [8.46E+08, 4.17E+06, 1.73E+06]
index = np.arange(3)
bar_width = 0.35
opacity = 0.9
ax_bar_fio_pmem_1g.bar(index, ydata_fio_1g_dram_seq, bar_width, alpha=opacity, color='r', label ='DRAM', log=True, hatch="//")
ax_bar_fio_pmem_1g.bar(index+bar_width, ydata_fio_1gp_seq, bar_width, alpha=opacity, color='b', label='PMEM', log=True)
ax_bar_fio_pmem_1g.set_xlabel('Page Size')
ax_bar_fio_pmem_1g.set_ylabel('dTLB Misses')
ax_bar_fio_pmem_1g.set_xticks(index+bar_width/2)
ax_bar_fio_pmem_1g.set_xticklabels(('4K', '2M', '1G'))
ax_bar_fio_pmem_1g.legend()

# PMEM 1G Random Accesses FIO 
ax_bar_fio_pmem_1g_random, fig_bar_fio_pmem_1g_random = plot_figure("PMEM dTLB Misses (1GB Random Read - FIO)", "Page Size", "dTLB Misses")
ydata_fio_1gp_random = [3.12E+08, 2.65E+06, 2.10E+06]
create_bar_chart(ax_bar_fio_pmem_1g_random, fig_bar_fio_pmem_1g_random, [1, 2, 3], ydata_fio_1gp_random, xticks)

# TODO: DRAM 1G Accesses FIO

# PMEM 4K Random Read FIO 
ax_fiorandread_pmem_4k, fig_fiorandread_pmem_4k = plot_figure("PMEM - FIO Read Speed (4K Block Accesses)", "Page Size", "Read Speed (GiB/s)")
ydata_fiorandread_pmem_4k = [1.12E+03, 4.71E+03, 3.15E+06]
create_bar_chart(ax_fiorandread_pmem_4k, fig_fiorandread_pmem_4k, [1, 2, 3], ydata_fiorandread_pmem_4k, xticks)

# PMEM 1G Random Read FIO
ax_fiorandread_pmem_1g, fig_fiorandread_pmem_1g = plot_figure("PMEM - FIO Read Speed (1G Block Accesses)", "Page Size", "Read Speed (GiB/s)")
ydata_fiorandread_pmem_1g = [5.04E+03, 5.18E+03, 5.16E+03]
create_bar_chart(ax_fiorandread_pmem_1g, fig_fiorandread_pmem_1g, [1, 2, 3], ydata_fiorandread_pmem_1g, xticks, log=False)


# dTLB Miss vs Page Size for 20GB Apex-MAP
ax_bar_apex, fig_bar_apex = plot_figure("dTLB Misses vs Page Size for 20GB Apex-MAP", "Page Size", "dTLB Misses", xticks)
ydata_apex_map = [3202767, 1862, 198]
create_bar_chart(ax_bar_apex, fig_bar_apex, [1, 2, 3], ydata_apex_map, xticks)

# dTLB Misses for Simple Test
ax_simple_test, fig_simple_test = plot_figure("dTLB Misses For Simple Test", "Page Size", "dTLB Misses that Cause Page Walk")
ydata_simple_test_dram = [129032242, 88692536, 26425]
ydata_simple_test_pmem = [124648518, 82780288, 32855]
index = np.arange(3)
bar_width = 0.35
opacity = 0.9
ax_simple_test.bar(index, ydata_simple_test_dram, bar_width, alpha=opacity, color='r', label ='DRAM', log=True, hatch="//")
ax_simple_test.bar(index+bar_width, ydata_simple_test_pmem, bar_width, alpha=opacity, color='b', label='PMEM', log=True)
ax_simple_test.set_xlabel('Page Size')
ax_simple_test.set_ylabel('dTLB Misses Causing a Page Table Walk')
ax_simple_test.set_xticks(index+bar_width/2)
ax_simple_test.set_xticklabels(('4K', '2M', '1G'))
ax_simple_test.legend()

# Runtime DRAM vs PMEM Polybench 
ax_polybench, fig_polybench = plot_figure("Polybench - DRAM vs PMEM (1G Page Size)", "Polybench Test Name", "Runtime (seconds)")
ydata_polybench_dram = [1.720761, 3.162308, 0.004844, 1.236885, 0.002483, 4.400878, 0.001994, 9.181109]
ydata_polybench_pmem = [1.684509, 2.910898, 0.005201, 1.211314, 0.002804, 2.924414, 0.002201, 1.503683]
polybench_index = np.arange(8)
bar_width = 0.2
opacity = 0.9
ax_polybench.bar(polybench_index, ydata_polybench_dram, bar_width, alpha=opacity, color='r', label='DRAM', log=True, hatch="//")
ax_polybench.bar(polybench_index+bar_width, ydata_polybench_pmem, bar_width, alpha=opacity, color='b', label='PMEM', log=True)
ax_polybench.set_xticks(polybench_index+bar_width/2)
ax_polybench.set_xticklabels(('2mm', '3mm', 'atax', 'cholesky', 'durbin', 'lu', 'trisolv', 'gs'))
ax_polybench.legend()

# Try percent difference too
ax_poly_diff, fig_poly_diff = plot_figure("Polybench - DRAM vs PMEM", "Test Name", "% Difference")
ydata_poly_diff = list()
ydata_poly_diff.append((ydata_polybench_dram[5]/ydata_polybench_pmem[5]) * 100)
ydata_poly_diff.append((ydata_polybench_dram[7]/ydata_polybench_pmem[7]) * 100)
create_bar_chart(ax_poly_diff, fig_poly_diff, [1, 2], ydata_poly_diff, ['lu', 'gs'], log=False)

# Postmark PMEM vs DRAM
ax_postmark, fig_postmark = plot_figure("Postmark - DRAM vs PMEM", "Page Size", "Reads (MB/s")
ydata_postmark_dram = [33.66, 38.47, 29.92]
ydata_postmark_pmem = [33.66, 38.47, 38.47]
postmark_index = np.arange(3)
bar_width = 0.3
opacity = 0.9
ax_postmark.bar(postmark_index, ydata_postmark_dram, bar_width, alpha=opacity, color='r', label='DRAM', hatch="//")
ax_postmark.bar(postmark_index+bar_width, ydata_postmark_pmem, bar_width, alpha=opacity, color='b', label='PMEM')
ax_postmark.set_xticks(postmark_index+bar_width/2)
ax_postmark.set_xticklabels(('4K', '2M', '1G'))
ax_postmark.legend()

ax_postmark, fig_postmark = plot_figure("Postmark - DRAM vs PMEM", "Page Size", "Reads (MB/s")
ydata_postmark_dram = [33.66, 38.47, 29.92]
ydata_postmark_pmem = [33.66, 38.47, 38.47]
postmark_index = np.arange(3)
bar_width = 0.3
opacity = 0.9
ax_postmark.bar(postmark_index, ydata_postmark_dram, bar_width, alpha=opacity, color='r', label='DRAM', hatch="//")
ax_postmark.bar(postmark_index+bar_width, ydata_postmark_pmem, bar_width, alpha=opacity, color='b', label='PMEM')
ax_postmark.set_xticks(postmark_index+bar_width/2)
ax_postmark.set_xticklabels(('4K', '2M', '1G'))
ax_postmark.legend()



plt.show()