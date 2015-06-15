+rabbitmq version: 3.5.3
+rabbitmq-c version: 0.6.0 
+compile:
gcc test.c -lrabbitmq utils.c rabbitmq_mgr.c rabbitmq_log.c

