#Targets to be execute
all:test_assign3 test_expr  clean
#Creates binary file
test_assign3: test_assign3_1.o expr.o dberror.o record_mgr.o RM_Scan_node_ll.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o buffer_node_ll.o
	cc -o test_assign3 test_assign3_1.o expr.o dberror.o record_mgr.o RM_Scan_node_ll.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o buffer_node_ll.o
test_assign3_1.o: test_assign3_1.c dberror.h expr.h record_mgr.h tables.h test_helper.h
	cc -c test_assign3_1.c
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
expr.o: expr.c dberror.h tables.h
	cc -c expr.c
record_mgr.o: record_mgr.c record_mgr.h dberror.h expr.h tables.h
	cc -c record_mgr.c
RM_Scan_node_ll.o: RM_Scan_node_ll.c record_mgr.h buffer_mgr.h dt.h
	cc -c RM_Scan_node_ll.c
rm_serializer.o: rm_serializer.c dberror.h tables.h record_mgr.h
	cc -c rm_serializer.c

#Creates binary for test expr
test_expr:  test_expr.o expr.o dberror.o record_mgr.o RM_Scan_node_ll.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o buffer_node_ll.o
	cc -o test_expr test_expr.o expr.o dberror.o record_mgr.o RM_Scan_node_ll.o rm_serializer.o storage_mgr.o buffer_mgr.o buffer_mgr_stat.o buffer_node_ll.o
test_expr.o: test_expr.c dberror.h expr.h record_mgr.h tables.h
	cc -c test_expr.c

#Clean up the directory
clean:
	-rm *.o
