# FastAndReliableFTP
Developed a fast and reliable file transfer application to transfer large files of up to 1GB size over links with high latency and loss ratio by using raw sockets. 
Approach- 
1. Established a UDP connection and trasnferring file by dividing it up into equal sized packets. 
2. Implemented a simplistic version of Negative Acknowledgements (NACK) to transmit the file reliably.

Statistics of the achieved file transfer rates for a 1GB file- 
1. Link capacity: 100Mbps, Latency: 10 ms, Loss ratio: 1% Throughput = 55Mbps 
2. Link capacity: 100Mbps, Latency: 200 ms, Loss ratio: 20% Throughput = 40Mbps
