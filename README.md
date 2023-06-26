[![License: CC BY-NC 4.0](https://img.shields.io/badge/License-CC%20BY--NC%204.0-lightgrey.svg)](http://creativecommons.org/licenses/by-nc/4.0/) 

# Spectre Implementation on rpi3

Simple Spectre implementation attack PoC for Raspberry Pi 3B+

## Resources
* `Spectre attack` contains a single- and multi-process implementation of Spectre.
* `enable_arm_pmu.mod` kernel module to facilitate access to PMU from thread mode
* `cacheAttack6bit `The code in this folder is just a naive implementation of a Flush+Reload attack. For the reasons presented in the report, it was tested on a single L1 cache of an ARM Cortex-a53, and it can read up to 6 bits at a time.
This result was not significant enough for our purposes, so this version of the attack was not embedded in the final code of Spectre. Nevertheless, it is provided as supporting material.
* `CPUACTRL_EL1`a simple kernel module to print CPU and L1 cache related information.

## Supporting material

* **Lecture - Spectre.pdf:** an introductory lecture on Spectre* **Lecture - an introduction to spectre and meltdown.pdf:** An extensive overview of the Spectre attack and its proposed implementation


## Authors

This material was initially developed as part of an assignment for the Operating Systems for embedded systems course delivered at Politecnico di Torino by Prof. Stefano Di Carlo during the academic year 2022/20023. 

Credits for the preparation of this material go to:

* [Davide Giuffrida](https://www.linkedin.com/in/davide-giuffrida-55959a267/)
* [Matteo Fragassi](https://www.linkedin.com/in/matteo-fragassi-06a19b241/)
* [Umberto Toppino](https://www.linkedin.com/in/elena-roncolino-177908159/)
* [{Elena Roncolino](https://www.linkedin.com/in/elena-roncolino-177908159/)

# References

All the files named "armpmu_lib.h" and all the contents in the folder "enable_arm_pmu.mod/enable_arm_pmu" have been written by Austin Seipp for ARMv7 (https://github.com/thoughtpolice/enable_arm_pmu) and they have been adapted for ARMv8 by Zhiyi Sun. The code used for the Raspberry Pi 3-B (ARMv8) is available in the following GitHub repository: [https://github.com/zhiyisun/enable_arm_pmu/tree/dev]().

