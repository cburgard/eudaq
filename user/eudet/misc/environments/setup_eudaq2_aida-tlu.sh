# IP variables for start scripts
export RUNCONTROLIP=192.168.22.101
export TLUIP=192.168.22.101
export NIIP=192.168.22.101

echo "Check pathes in telescope.ini and in the conf-files"

# eudaq2 binaries
export EUDAQ=/home/itkstrip/TB2106/eudaq2/
export PATH=$PATH:$EUDAQ/bin

## for AIDA TLU
# for Ubuntu 18.04
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/cactus/lib
# start controlhub for stable AIDA TLU TCP/IP communication
/home/itkstrip/TB2106/cactus/bin/controlhub_start
/home/itkstrip/TB2106/cactus/bin/controlhub_status
