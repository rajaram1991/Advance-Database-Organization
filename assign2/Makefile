#Targets to be execute
all:test_assign2_1 clean
#Creates binary file
test_assign2_1: test_assign2_1.o dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o buffer_node_ll.o
	cc -o test_assign2_1 test_assign2_1.o dberror.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o buffer_node_ll.o -pthread
test_assign2_1.o: test_assign2_1.c storage_mgr.h buffer_mgr_stat.h buffer_mgr.h dberror.h test_helper.h
	cc -c test_assign2_1.c
dberror.o: dberror.c dberror.h
	cc -c dberror.c
storage_mgr.o: storage_mgr.c storage_mgr.h dberror.h
	cc -c storage_mgr.c
buffer_mgr.o: buffer_mgr.c buffer_mgr.h buffer_node_ll.h storage_mgr.h dt.h
	cc -c buffer_mgr.c
buffer_mgr_stat.o: buffer_mgr_stat.c buffer_mgr_stat.h buffer_mgr.h
	cc -c buffer_mgr_stat.c
buffer_node_ll.o: buffer_node_ll.c buffer_node_ll.h
	cc -c buffer_node_ll.c
#Remove object files and keep directory clean
clean:
	-rm *.o
