#!/bin/bash

# run as root
# sudo su
echo 1024 > /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
# exit
