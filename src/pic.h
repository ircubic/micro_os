#ifndef KERNEL_PIC_H
#define KERNEL_PIC_H 1

/* PIC PORTS */
#define PIC_MASTER         0x20
#define PIC_SLAVE          0xA0
#define PIC_MASTER_COMMAND PIC_MASTER
#define PIC_MASTER_DATA    (PIC_MASTER+1)
#define PIC_SLAVE_COMMAND  PIC_SLAVE
#define PIC_SLAVE_DATA     (PIC_SLAVE+1)

/* PIC ICWs */
#define PIC_ICW1_ICW4      0x01
#define PIC_ICW1_SINGLE    0x02
#define PIC_ICW1_INTERVAL4 0x04
#define PIC_ICW1_LEVEL     0x08
#define PIC_ICW1_INIT      0x10

#define PIC_ICW4_8086       0x01
#define PIC_ICW4_AUTO       0x02
#define PIC_ICW4_BUF_SLAVE  0x08
#define PIC_ICW4_BUF_MASTER	0x0C
#define PIC_ICW4_SFNM       0x10

/* PIC COMMANDS */
#define PIC_EOI 0x20

void pic_signal_eoi(unsigned int irq);
void pic_remap(unsigned int int_off1, unsigned int int_off2);

#endif
