#!/bin/bash

ip link add vxlan0 type vxlan id 100 dstport 4789 group 239.1.1.1 dev eth1
ip addr add 20.0.0.1/24 dev vxlan0
ip link set vxlan0 up
