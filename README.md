# rabbitmq_priority_topic_c
rabbitmq with priority and topic functions, you can send items with priorities and received with topics

make sure plugin rabbitmq_priority_queue has installed and make it enable.
and this demo is base on rabbimq-c library.

see the situation here: 
you want to use queue to save items with different priorities, for example from 0 - 10, and each time when you insert the higher priority item, it must queue out than some lower ones.
and what's more, the output of priority queue have more selections which call 'exchange-topic' in rabbitmq，just see the offcial docs. 

+---- how to use ----+
+rabbitmq version: 3.5.3
+rabbitmq-c version: 0.6.0 
+compile:
g++ test.cpp utils.cpp rabbitmq_mgr.cpp rabbitmq_log.cpp -lrabbitmq -lpthread

