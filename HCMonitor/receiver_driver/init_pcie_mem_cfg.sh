mount /dev/sda /mnt/ssd
cd pcie_driver
sh hmtt_pcie_driver_load
cd ../memory_dev
sh memory_dev_load
cd ../fpga_config
./fpga_config
