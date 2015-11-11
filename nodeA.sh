
#!/bin/sh
start=$(date +"%T")
echo "Start Time :$start"
sudo python client.py 10.1.1.3 10.1.1.2 eth4 test.txt
sudo python server.py 10.1.1.3 10.1.1.2 eth4 test3.txt
echo "Start Time :$start"
end=$(date +"%T")
echo "End Time :$end"
