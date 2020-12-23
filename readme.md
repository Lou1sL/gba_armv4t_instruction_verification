毕业论文第四章 一个自动化的ARM指令集架构模拟器的测试方法（Chapter 4: A General Approach on Automatic ARM ISA Emulator Debugging） 的源代码及电路图。


GBA卡带访问：

类似ARM CPU，GBA卡带也分为Non-Sequential和Sequential，第一次读写一定为Non-Sequential，地址和数据分时复用一组卡带总线。

1：Address on AD0-AD15 & A16-A23, ~cs1 lo

2: ~rd/~wr lo

3: ~rd/~wr hi, Data on AD0-AD15, address increment, rising edge rd/wr


大致原理及步骤：

0: 焊接排线和gba卡带触点，通过连接地址线和~ cs1到74ac191进行sequencial访问的地址计算（即当~ cs1 lo时latch地址，此后的~rd ~wr信号lo时访问数据，每次数据地址+2）并顺便解开卡带总线复用，具体参见电路图。

gba卡带3.3v 16.78MHz，stm32f4引脚3.3v APB2 168MHz APB1 84MHz，（都是3.3v不需要上下拉电阻）。

卡带A0-A23, 24位地址+1最低位默认0 = 25位 寻址32MBytes(1FFFFFF)。

卡带A0-A15共用D0-D15，2Bytes每访问。


2：启动gba实机，实机初始化完成后会访问卡带0x08000000，此时开发板无限返回跳转offset=0指令，使实机在卡在0x08000000处死循环。


3：开发板半随机生成待验证指令并再附加一段寄存器dump指令（将dump放入卡带0x08200000），这些指令放入卡带地址0x08100000。


4：完成后再次检测到访问0x08000000时，返回给gba实机跳转到0x08100000的指令。


5：开发板接受到gba实机0x08200000的dump，意味待验证指令及寄存器dump执行完成，返回跳转到0x08000000指令。


6：开发板将待验证指令传入模拟器中执行，期间开发板向实机无限返回跳转offset=0指令，使实机再次卡在0x08000000处死循环。


7：验证模拟器的寄存器组和gba实机产生的寄存器dump的一致性，并将结果记录。


8：该条指令验证完毕，回到第3步生成下条验证指令，直至验证完成。


BOM:


STM32F40  x1


74AC191PC x4


74AC08PC  x1


74AC00PC  x1




其它：


面包板    x2


32p排线   x1


杜邦线    若干