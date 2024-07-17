# Direct-Memory-Access-Controller-DMA-DMAC-in-SystemC
## Abstract
Develop a Direct Memory Access Controller (DMA/DMAC) in SystemC, wrapped by TLM 2.0.
Please read carefully. All outputs required are described in the text. Five (5) points will be taken for each bug, missing required output and behavior.

## Description

### 1.The DMA has:
- i.One (1) 32-bit master port developed with TLM 2.0 blocking initiator port.
- ii.One (1) 32-bit slave port developed with TLM 2.0 blocking target port.
- iii.One (1) 1-bit interrupt pin developed with sc_out.
- iv.One (1) 1-bit reset pin developed with sc_in.
- v.One static address register to store its base address.
### 2.There are also four (4) control registers in the DMA:
- i.SOURCE: 32-bit, the starting source address, at 0x0
- ii.TARGET: 32-bit, the starting target address, at 0x4
- iii.SIZE: 32-bit, the data size, at 0x8
- iv.START/CLEAR: 1-bit, the start and clear signal, at 0xC
### 3.The DMA behavior is illustrated as:
![image](https://github.com/user-attachments/assets/30a48f94-edab-4a78-8500-71d9e0454dae)
### 4.Implement the DMA module with a SC_CTHREAD process and it:
- Through the slave port and according to the addresses received, passively write control data, i.e. source address, target address, size, and start/clear, to four (4) control registers.
- Once the value 1 is stored in the START/CLEAR control register (0xC = 1), via the master port actively start moving data from the SOURCE address to the TARGET address until the SIZE is reached.
- When the data movement is completed, first pull the interrupt signal (interrupt) to high and wait for the interrupt clear control (START/CLEAR, 0X0C = 0). Only after the CLEAR arrives then the DMA resets the interrupt to low and resume the original state.
- The reset pin (reset) is synchronous and active low. At reset, all 4 control registers are cleared as 0 and the interrupt signal is pulled to low.
### 5.This DMA is verified using the single-core system developed in Part 2. 
