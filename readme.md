

毕业论文第四章 一个自动化的ARM指令集架构模拟器的测试方法（Chapter 4: A General Approach on Automatic ARM ISA Emulator Debugging） 的源代码。


/src


源码


/schematic


电路图


GBA卡带：

类似ARM CPU，GBA卡带也分为Non-Sequential和Sequential，第一次读写一定为Non-Sequential。


卡带读：

1：Address on AD0-AD15 & A16-A23, ~CS lo

2: ~RD lo

3: ~RD hi (address increment, rising edge read)



大致原理及步骤：


1：stm32f4开发板接gba游戏机卡带接口，stm32f4开发板接收gba对卡带的读写访问。
（读写触发DMA?）

gba卡带3.3v 16.78MHz，stm32f4引脚3.3v APB2 168MHz APB1 84MHz。


卡带A0-A23, 24位地址+1最低位默认0 = 25位 寻址32MBytes(1FFFFFF)。
卡带A0-A15共用D0-D15，2Bytes每访问，


（都是3.3v，不需要上下拉电阻）


2：启动gba实机，实机初始化完成后会访问卡带0x08000000，此时开发板返回一条跳转到0x08000000指令（使实机在卡带地址0x08000000处死循环）。


3：开发板半随机生成待测试的指令Ins，并配合Ins指令再附加一段指令（寄存器dump并将dump放入卡带0x08100000后跳转到卡带地址0x08300000并进行一次读访问），指令放入卡带地址0x08200000。


5：完成后再次检测到访问0x08000000时，返回给gba实机跳转到0x08200000的指令。


6：开发板接受到gba实机0x08100000的dump和对地址0x08200000的读访问，意味着该条测试指令及寄存器dump执行完成，返回跳转到0x08000000指令。


7：开发板将测试指令传入模拟器中执行。


8：验证模拟器的寄存器组和gba实机产生的寄存器dump的一致性，并将结果打log。


9：验证完毕，回到第3步生成下条测试指令，直至测试完成。


BOM:


STM32F40  x1


74AC191PC x4


74AC08PC  x1


74AC00PC  x1




其它：


面包板    x2


32p排线   x1


公对公杜邦线  若干