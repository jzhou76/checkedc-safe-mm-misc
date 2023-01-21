#!/usr/bin/env python3

#
# This script reads in thttpd memory consumption data and calculates the
# memory overhead of Checked C. Specifically, it computes the maximum RSS
# and the maximum WSS.
#
# Transferred file sizes: 4~KB to 32~MB.
#
#

import numpy as np
import os
import csv

#
# Project root directory and data directory for memory overhead.
#
ROOT_DIR = os.path.abspath(os.getcwd() + "/../../..")
DATA_DIR = ROOT_DIR + "/eval/mem_data/thttpd/"

#
# Transferred file size from 2^start KB to 2^end KB
#
FILE_SIZE_START, FILE_SIZE_START_END = 2, 15

#
# Memory consumption data
#
mem_data = {
        "baseline" : {
            "rss" : {}, "rss_max" : {},
            "wss" : {}, "wss_max" : {}
        },
        "checked" : {
            "rss" : {}, "rss_max" : {},
            "wss" : {}, "wss_max" : {}
        },
}

#
# Round a float number to its nearest integer.
#
def Int(num):
    return int(round(num, 0))

#
# Collect data in the output files from wss.pl, and compute the max RSS.
#
# @param setting: "baseline" or "checked"
#
def collect_data(setting):
    data_file = open(DATA_DIR + setting + "/mem.stat")
    # Skip the headings and the last ERROR reporting line.
    data = data_file.readlines()[3:]
    rss, wss = [], []
    size = 4  # Initial size: 4~KB
    for line in data:
        line = line.split()
        if line[0] == "Transferring" or "ERROR" in line[0]:
            # Finished reading in data for a file
            # Compute the maximum RSS and WSS
            mem_data[setting]["rss_max"][size] = round(max(rss), 2)
            mem_data[setting]["wss_max"][size] = round(max(wss), 2)
            # Compute the average RSS and WSS.
            mem_data[setting]["rss"][size] = round(np.mean(rss), 2)
            mem_data[setting]["wss"][size] = round(np.mean(wss), 2)

            # Start to process data for a new file
            size *= 2
            rss, wss = [], []
        else:
            rss += [float(line[1])]
            wss += [float(line[3])]

#
# Write results to a CSV file.
#
def write_result():
    with open(DATA_DIR + "mem.csv", "w") as mem_csv:
        writer = csv.writer(mem_csv)
        header = ["data", "baseline_rss (MB)", "checked_rss (MB)", "checked_rss (x)",\
                "baseline_wss (MB)", "checked_wss (MB)", "checked_wss (x)"]
        writer.writerow(header)

        rss_norm, wss_norm = [], []
        for size in [2**i for i in range(FILE_SIZE_START, FILE_SIZE_START_END + 1)]:
            if size < 1024:
                row = [str(size) + " KB"]
            else:
                row = [str(int(size / 1024)) + " MB"]

            # Add rss data
            baseline_rss_max = mem_data["baseline"]["rss_max"][size]
            checked_rss_max = mem_data["checked"]["rss_max"][size]
            normalized = round(checked_rss_max / baseline_rss_max, 3)
            rss_norm += [normalized]
            # Add baseline RSS (MB)
            row += [Int(baseline_rss_max)]
            # Add checked RSS (MB)
            row += [Int(checked_rss_max)]
            # Add normalized Checked C RSS (X)
            row += [normalized]

            # Add wss data
            baseline_wss_max = mem_data["baseline"]["wss_max"][size]
            checked_wss_max = mem_data["checked"]["wss_max"][size]
            normalized = round(checked_wss_max / baseline_wss_max, 3)
            wss_norm += [normalized]
            # Add baseline WSS (MB)
            row += [Int(baseline_wss_max)]
            # Add checked WSS(MB)
            row += [Int(checked_wss_max)]
            # Add normalized Checked C WSS (X)
            row += [normalized]
            writer.writerow(row)


        # Compute the geomean of Checked C's and CETS' RSS and WSS.
        data_num = len(rss_norm)
        rss_geomean = round(np.array(rss_norm).prod() ** (1.0 / data_num), 3)
        wss_geomean = round(np.array(wss_norm).prod() ** (1.0 / data_num), 3)

        row = ["geomean", "", "", rss_geomean, "", "", wss_geomean]
        writer.writerow(row)
        # Close result file.
        mem_csv.close()

        # Print the summarized data.
        print("Checked C's RSS overhead on thttpd:")
        print("Min = " + str(round((min(rss_norm) - 1) * 100, 2)) + "%")
        print("Max = " + str(round((max(rss_norm) - 1) * 100, 2)) + "%")
        print("Geomean = " + str(round((rss_geomean - 1) * 100, 2)) + "%")

#
# Entrance of this script
#
def main():
    # Collect all the raw data generated by wss.
    collect_data("baseline")
    collect_data("checked")

    # Write results to a csv file
    write_result()

if __name__ == "__main__":
    main()
