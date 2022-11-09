#!/bin/bash

CONTAINER_ADDR=${1}

# Usage: ./vxlan_with_auto.sh [container ip]
#   example: ./vxlan_with_bridge_auto.sh 20.0.0.1

# clean
ip link del veth1 2> /dev/null || true
ip netns del ns1 2> /dev/null || true
ip link del vxlan0 2> /dev/null || true
ip link del br0 2> /dev/null || true

# add vxlan
ip link add vxlan0 type vxlan id 100 dstport 4789 dev eth1 nolearning proxy l2miss l3miss

# add bridge
ip link add br0 type bridge
ip link set vxlan0 master br0
ip link set vxlan0 up
ip link set br0 up

# add namespace and veth pair
ip netns add ns1
ip link add veth1 type veth peer name veth1-peer
ip link set dev veth1 master br0
ip link set dev veth1 up
ip link set dev veth1-peer netns ns1
ip netns exec ns1 ip link set veth1-peer name eth0
ip netns exec ns1 ip link set lo up
ip netns exec ns1 ip addr add $CONTAINER_ADDR/24 dev eth0
ip netns exec ns1 ip link set eth0 up
