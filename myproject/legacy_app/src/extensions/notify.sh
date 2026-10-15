
#!/bin/bash

VOLATILE_DIR="/opt/appdata/data"
MASTER_FILE="${VOLATILE_DIR}/keepalived_master"
BACKUP_FILE="${VOLATILE_DIR}/keepalived_backup"

STATE=$1

if [ -f "$MASTER_FILE" ]; then
    rm -f "$MASTER_FILE"
fi

if [ -f "$BACKUP_FILE" ]; then
    rm -f "$BACKUP_FILE"
fi

if [ "$STATE" == "MASTER" ]; then
    touch "$MASTER_FILE"  
elif [ "$STATE" == "BACKUP" ]; then
    touch "$BACKUP_FILE"  
fi


