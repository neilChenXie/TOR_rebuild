#!/bin/bash
TRAFFIC=10.5.51.2/24
ETH0=192.168.96.141
#setup tunnel and revise routing table
sudo ip tuntap add dev tun1 mode tun
sudo ifconfig tun1 $TRAFFIC up
sudo ip rule add from $ETH0 table 9 priority 8
sudo ip route add table 9 to 18/8 dev tun1
sudo ip route add table 9 to 128/8 dev tun1
sudo iptables -A OUTPUT -p tcp --tcp-flags RST RST -j DROP
#config router ip
sudo ifconfig eth1 192.168.201.2/24 up
sudo ifconfig eth2 192.168.202.2/24 up
sudo ifconfig eth3 192.168.203.2/24 up
sudo ifconfig eth4 192.168.204.2/24 up
sudo ifconfig eth5 192.168.205.2/24 up
sudo ifconfig eth6 192.168.206.2/24 up
sudo ifconfig eth7 192.168.207.2/24 up
