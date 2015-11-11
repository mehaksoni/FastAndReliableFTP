#!/bin/sh
sudo python server.py 10.1.1.2 10.1.1.3 eth6 test2.txt
sudo python client.py 10.1.1.2 10.1.1.3 eth6 test2.txt
