#CC:=arm-linux-gcc
CC:=gcc

TARGET_SRV=iot_server
OBJECT_SRV=$(TARGET_SRV).o 

TARGET_CLN=iot_client
OBJECT_CLN=$(TARGET_CLN).o 

#TARGET_CLN_SQL=iot_client_sql
#OBJECT_CLN_SQL=$(TARGET_CLN_SQL).o 

LDFLAGS=-D_REENTRANT -pthread
LDFLAGS_SQL=-D_REENTRANT -pthread -lmysqlclient

all : $(TARGET_SRV) $(TARGET_CLN) $(TARGET_CLN_SQL)

$(TARGET_SRV):$(OBJECT_SRV)
	$(CC) -o $@ $(OBJECT_SRV) $(LDFLAGS)
$(TARGET_CLN):$(OBJECT_CLN)
	$(CC) -o $@ $(OBJECT_CLN) $(LDFLAGS)
$(TARGET_CLN_SQL):$(OBJECT_CLN_SQL)
	$(CC) -o $@ $(OBJECT_CLN_SQL) $(LDFLAGS_SQL)
%.o:%.c
	$(CC) -c -o $@ $<
clean:
	rm -f *.o 
clean_all:
	rm -f *.o $(TARGET_SRV) $(TARGET_CLN) $(TARGET_CLN_SQL)
	rm -f *.o $(TARGET_SRV) $(TARGET_CLN) $(TARGET_CLN_SQL)
