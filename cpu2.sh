#!/bin/bash

BAUD_RATE=115200

# Helper functions
get_network_interface() { ip route show default | awk '/default/ {print $5}'; }
get_ip_address() { ip -4 addr show "$(get_network_interface)" | grep -oP '(?<=inet\s)\d+(\.\d+){3}'; }
get_device_path() { echo $(ls /dev/ttyUSB* /dev/ttyACM* 2>/dev/null); }

get_cpu_usage() { 
    read cpu_stat1 < /proc/stat; sleep 1; read cpu_stat2 < /proc/stat
    cpu1=(${cpu_stat1// / }); cpu2=(${cpu_stat2// / })
    idle1=${cpu1[4]}; idle2=${cpu2[4]}
    total1=0; total2=0
    for i in "${cpu1[@]:1}"; do total1=$((total1 + i)); done
    for i in "${cpu2[@]:1}"; do total2=$((total2 + i)); done
    echo $((100 * ((total2 - total1) - (idle2 - idle1)) / (total2 - total1)))
}

get_network_usage() { 
    rx1=$(< "/sys/class/net/$(get_network_interface)/statistics/rx_bytes")
    tx1=$(< "/sys/class/net/$(get_network_interface)/statistics/tx_bytes")
    sleep 1
    rx2=$(< "/sys/class/net/$(get_network_interface)/statistics/rx_bytes")
    tx2=$(< "/sys/class/net/$(get_network_interface)/statistics/tx_bytes")
    echo "$(((rx2 - rx1) / 1024)) KB/s $(((tx2 - tx1) / 1024)) KB/s"
}

get_battery_level() { 
    upower -i $(upower -e | grep '/battery') | grep -E "percentage" | grep -oP '[0-9]+'
}

echo "IP Address: $(get_ip_address)"

# Main loop
while true; do
    device=$(get_device_path)
    if [ -n "$device" ]; then
        echo "ESP32 detected at $device"
        stty -F "$device" "$BAUD_RATE" cs8 -cstopb -parenb -ixon

        # Collect and send system stats
        message="$(get_cpu_usage)% - $(get_ip_address) - $(get_network_usage) - $(get_battery_level)%"
        echo "$message"
        echo "$message" > "$device"
        sleep 1
    else
        echo "ESP32 not connected. Retrying..."
        sleep 3
    fi
    sleep 1  # Adjust the sleep interval as needed
done