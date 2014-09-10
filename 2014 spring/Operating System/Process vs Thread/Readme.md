Process vs Thread
=================

This is a sample server and client program determine which is better performanc
e. Server forks several process or thread.

Process and Thread model server contains same structure. Each folder have Makef
ile to make executable objects.

Test folder contains test shell script. Script execute server first, and later 
execute clients. Finally, script execute special client - shell client.

This program uses customized linux system call which returns system uptime in n
anosecond.