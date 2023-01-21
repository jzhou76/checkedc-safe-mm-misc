#!/usr/bin/env python3

#
# This script reads in parson memory consumption data and calculates the
# memory overhead of Checked C.
#

import numpy as np
import os
import csv

#
# Project root directory and data directory for memory overhead.
#
ROOT_DIR = os.path.abspath(os.getcwd() + "/../../..")
DATA_DIR = ROOT_DIR + "/eval/mem_data/parson/"

#
# Data files
#
data_files = [
    "countries-small",
    "profiles",
    "covers",
    "books",
    "albums",
    "restaurant",
    "countries-big",
    "zips",
    "images",
    "city_inspections",
    "companies",
    "trades",
    "citylots"
]

#
# RSS threshold: We only process data files whose RSS are greater than a threshold
#
RSS_THRESHOLD = 10

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
# Collect data in the output files from wss.pl, and compute the average and
# max RSS and WSS.
#
# @param setting: "baseline" or "checked"
#
def collect_data(setting):
    for data_name in data_files:
        data_file = open(DATA_DIR + setting + "/" + data_name + ".stat")
        # Skip the headings and the last ERROR reporting line.
        data = data_file.readlines()[2:-1]
        rss, wss = [], []
        for line in data:
            line = line.split()
            rss_one, wss_one = round(float(line[1]), 2),round(float(line[3]), 2)
            if wss_one == 0:
                break
            rss += [rss_one]
            wss += [wss_one]
        # Get the maximum RSS
        if len(rss) == 0:
            continue
        rss_max = round(max(rss), 2)
        if rss_max < RSS_THRESHOLD:
            continue
        mem_data[setting]["rss_max"][data_name] = rss_max
        mem_data[setting]["wss_max"][data_name] = round(max(wss), 2)
        # Compute the average RSS and WSS.
        mem_data[setting]["rss"][data_name] = round(np.mean(rss), 2)
        mem_data[setting]["wss"][data_name] = round(np.mean(wss), 2)

#
# Write results to a CSV file.
#
def write_result():
    with open(DATA_DIR + "mem.csv", "w") as mem_csv:
        writer = csv.writer(mem_csv)
        header = ["data", "baseline_rss (MB)", "checked_rss (x)",\
                "baseline_wss (MB)", "checked_wss (x)"]
        writer.writerow(header)

        checked_rss_norm, checked_wss_norm = [], []
        for data_name in data_files:
            # Skip those lower than RSS_THRESHOLD
            if data_name not in mem_data["baseline"]["rss_max"] or \
               data_name not in mem_data["checked"]["rss_max"]:
                   continue

            row = [data_name]
            # Add rss data
            baseline_rss_max = mem_data["baseline"]["rss_max"][data_name]
            checked_rss_max = mem_data["checked"]["rss_max"][data_name]
            normalized = round(checked_rss_max / baseline_rss_max, 3)
            checked_rss_norm += [normalized]
            # Add baseline memory consumption (MB)
            row += [Int(baseline_rss_max)]
            # Add normalized Checked C memory consumption (X)
            row += [normalized]

            # Add wss data
            baseline_wss= mem_data["baseline"]["wss"][data_name]
            checked_wss= mem_data["checked"]["wss"][data_name]
            if not baseline_wss == 0:
                normalized = round(checked_wss / baseline_wss, 3)
                checked_wss_norm += [normalized]
                # Add baseline memory consumption (MB)
                row += [Int(baseline_wss)]
                # Add normalized Checked C memory consumption (X)
                row += [normalized]
                writer.writerow(row)

        # Compute the geomean of Checked C's RSS and WSS.
        data_num = len(checked_rss_norm)
        checked_rss_geomean = round(np.array(checked_rss_norm).prod() ** (1.0 / data_num), 3)
        checked_wss_geomean = round(np.array(checked_wss_norm).prod() ** (1.0 / data_num), 3)

        row = ["geomean", "", checked_rss_geomean, "", checked_wss_geomean]
        writer.writerow(row)

        # Print the summarized data.
        print("Checked C's RSS overhead on parson:")
        print_max_min(checked_rss_norm, "RSS Min = ", True)
        print_max_min(checked_rss_norm, "RSS Max = ", False)
        print_geomean(checked_rss_geomean)
        print()
        print("Checked C's WSS overhead on parson:")
        print_max_min(checked_wss_norm, "RSS Min = ", True)
        print_max_min(checked_wss_norm, "RSS Max = ", False)
        print_geomean(checked_wss_geomean)

        # Close result file.
        mem_csv.close()

#
# Print the min/max of RSS/WSS overhead.
#
def print_max_min(data, msg, is_min):
    if is_min:
        print(msg + " = " + str(round((min(data) - 1) * 100, 2)) + "%")
    else:
        print(msg + " = " + str(round((max(data) - 1) * 100, 2)) + "%")

#
# Print the geomean of RSS/WSS overhead.
#
def print_geomean(data):
    print("Geomean = " + str(round((data - 1) * 100, 2)) + "%")

#
# Entrance of this script
#
def main():
    # Collect all the raw data generated by wss.
    collect_data("baseline")
    collect_data("checked")

    # print(mem_data["baseline"]["rss_max"])
    # print(mem_data["checked"]["rss_max"])
    # print(mem_data["baseline"]["wss"])
    # print(mem_data["checked"]["wss"])

    # Write results to a csv file
    write_result()

if __name__ == "__main__":
    main()
