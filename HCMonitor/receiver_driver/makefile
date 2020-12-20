.PHONY: all clean load unload

all: memory_dev pcie_driver fpga_config trace_operation
memory_dev::
	make -C memory_dev/
pcie_driver::
	make -C pcie_driver/
fpga_config::
	make -C fpga_config/
trace_operation::
	make -C trace_operation/
clean:
	make -C memory_dev/ clean
	make -C pcie_driver/ clean
	make -C fpga_config/ clean
	make -C trace_operation/ clean
load: pcie_driver_load memory_dev_load fpga_config_load
pcie_driver_load::
	cd pcie_driver && sh hmtt_pcie_driver_load
memory_dev_load::
	cd memory_dev && sh memory_dev_load
fpga_config_load::
	cd fpga_config && ./fpga_config
unload: pcie_driver_unload memory_dev_unload
pcie_driver_unload::
	cd pcie_driver && sh hmtt_pcie_driver_unload
memory_dev_unload::
	cd memory_dev && sh memory_dev_unload
