# 深入理解FreeRTOS内核实现
> 实现FreeRTOS内核来进行深入理解rtos
>
> [https://aeneag.xyz](https://aeneag.xyz)
>
> 艾恩凝
>
> 2021/12/12

## 序

按照例程已经梳理了一遍内核，主要过程思维导图已展现了全部过程，为了加深理解，亲自上手实现一遍，梳理例程用的是野火的，编译版本为5，野火代码并不严谨，使用6版本编译器进行编译。

<img src="https://pic.aeneag.xyz/FreeRTOS%20%E5%86%85%E6%A0%B8.png" alt="内核图" style="zoom:50%;" />

## 一、链表实现

