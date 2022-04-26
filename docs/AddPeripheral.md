# Adding new peripherals

## Creating the new files

Add your new files inside the [hardware/peripherals folder](/hardware/peripherals/).
This files will contain the peripheral class header and source (.hpp and .cpp files).
It is important to provide the NoC interface inside the new module:
```cpp
/**
 * @file YourPeripheral.hpp
 */
...

SC_MODULE(YourPeripheral){
public:
	...

	sc_in<bool>		clock;
	sc_in<bool>		reset;

	sc_in<bool>		rx;
	sc_in<regflit>	data_in;
	sc_out<bool>	credit_out;

	sc_out<bool>	tx;
	sc_out<regflit>	data_out;
	sc_in<bool>		credit_in;

	...

private:
	...
};

...
```

## Connecting the peripheral to the many-core

In [Memphis.hpp](/hardware/src/include/Memphis.hpp), create signals.
```cpp
...

SC_MODULE(memphis) {
	...

	//IO interface - Create the IO interface for your component here:
	sc_out< bool >			memphis_your_peripheral_tx;
	sc_in< bool >			memphis_your_peripheral_credit_i;
	sc_out< regflit >		memphis_your_peripheral_data_out;

	sc_in< bool >			memphis_your_peripheral_rx;
	sc_out< bool >			memphis_your_peripheral_credit_o;
	sc_in< regflit >		memphis_your_peripheral_data_in;
};
```

In [memphis.cpp](/hardware/src/memphis.cpp), update the signals and add the signals to the sensitive list of `pes_interconnection`:
```cpp
...

...

SC_CTOR(memphis){
	...

	SC_METHOD(pes_interconnection);
	sensitive << memphis_your_peripheral_tx;
	sensitive << memphis_your_peripheral_credit_i;
	sensitive << memphis_your_peripheral_data_out;
	sensitive << memphis_your_peripheral_rx;
	sensitive << memphis_your_peripheral_credit_o;
	sensitive << memphis_your_peripheral_data_in;
	...
}
...

void Memphis::pes_interconnection(){
	...
	for(i=0;i<N_PE;i++){
		...

		//Insert the IO wiring for your component here if it connected to a port:
		if (i == YOURPERIPHERAL && io_port[i] != NPORT) {
 			p = io_port[i];
			memphis_your_peripheral_tx.write(tx[YOURPERIPHERAL][p].read());
			memphis_your_peripheral_data_out.write(data_out[YOURPERIPHERAL][p].read());
			credit_i[YOURPERIPHERAL][p].write(memphis_your_peripheral_credit_i.read());

			rx[YOURPERIPHERAL][p].write(memphis_your_peripheral_rx.read());
			memphis_your_peripheral_credit_o.write(credit_o[YOURPERIPHERAL][p].read());
			data_in[YOURPERIPHERAL][p].write(memphis_your_peripheral_data_in.read());
		}
	}
}

```

## Instatiating the peripheral

In [test_bench.h](/hardware/src/include/test_bench.h), include your peripheral header, add signals, create a pointer, instantiate and connect the signals:

```cpp
...
#include "peripherals/YourPeripheral.hpp"
...
SC_MODULE(test_bench) {
	...
	//IO signals connecting Your Peripheral and Memphis
	sc_signal<bool>		memphis_your_peripheral_tx;
	sc_signal<bool>		memphis_your_peripheral_credit_i;
	sc_signal<regflit> 	memphis_your_peripheral_data_out;
	sc_signal<bool>		memphis_your_peripheral_rx;
	sc_signal<bool>		memphis_your_peripheral_credit_o;
	sc_signal<regflit>	memphis_your_peripheral_data_in;
	...
	YourPeripheral * your_peripheral;
	...
	SC_HAS_PROCESS(test_bench);
	test_bench(sc_module_name name_, char *filename_= "output_master.txt") :
    sc_module(name_), filename(filename_)
    {
		...
		MPSoC = new memphis("Memphis");
		...
		MPSoC->memphis_your_peripheral_tx(memphis_your_peripheral_tx);
		MPSoC->memphis_your_peripheral_credit_i(memphis_your_peripheral_credit_i);
		MPSoC->memphis_your_peripheral_data_out(memphis_your_peripheral_data_out);
		MPSoC->memphis_your_peripheral_rx(memphis_your_peripheral_rx);
		MPSoC->memphis_your_peripheral_credit_o(memphis_your_peripheral_credit_o);
		MPSoC->memphis_your_peripheral_data_in(memphis_your_peripheral_data_in);
		...
		your_peripheral = new YourPeripheral("YourPeripheral");
		your_peripheral->clock(clock);
		your_peripheral->reset(reset);
		your_peripheral->rx(memphis_your_peripheral_tx);
		your_peripheral->data_in(memphis_your_peripheral_data_out);
		your_peripheral->credit_out(memphis_your_peripheral_credit_i);
		your_peripheral->tx(memphis_your_peripheral_rx);
		your_peripheral->data_out(memphis_your_peripheral_data_in);
		your_peripheral->credit_in(memphis_your_peripheral_credit_o);
		...
	}
	...
};
```

## Add your peripheral to the testcase

Remember to add your peripheral to the testcase the same way the macro is used in [memphis.cpp](../hardware/sc/memphis.cpp):
```yaml
...
  Peripherals: 
    - name: YOURPERIPHERAL
      pe: 0, 0
      port: S
```
