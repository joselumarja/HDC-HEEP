// System library headers
#include <stdio.h>

// Custom library headers
#include "gr_heep.h"
#include "dma.h"
#include "timer_sdk.h"
#include "input_data.h"

#define HDC_ACCELERATOR_ADDR_A (HDC_ACCELERATOR_START_ADDRESS + 0x0000)
#define HDC_ACCELERATOR_ADDR_B (HDC_ACCELERATOR_START_ADDRESS + 0x0004)
#define HDC_ACCELERATOR_ADDR_C (HDC_ACCELERATOR_START_ADDRESS + 0x0008)
#define HDC_ACCELERATOR_A_SIZE (HDC_ACCELERATOR_START_ADDRESS + 0x000C)
#define HDC_ACCELERATOR_B_SIZE (HDC_ACCELERATOR_START_ADDRESS + 0x0010)
#define HDC_ACCELERATOR_C_SIZE (HDC_ACCELERATOR_START_ADDRESS + 0x0014)
#define HDC_ACCELERATOR_SEL_OP (HDC_ACCELERATOR_START_ADDRESS + 0x0018)
#define HDC_ACCELERATOR_START (HDC_ACCELERATOR_START_ADDRESS + 0x001C)
#define HDC_ACCELERATOR_DONE (HDC_ACCELERATOR_START_ADDRESS + 0x0020)

#define HDC_SEL_OP_BINDING      0
#define HDC_VECTOR_SIZE_BITS    (INPUT_SIZE*32u)
#define HDC_TIMEOUT             1000000u

#define REG32(addr) (*(volatile uint32_t *)(addr))

static void hdc_write_reg(uint32_t addr, uint32_t value)
{
    REG32(addr) = value;
}

static uint32_t hdc_read_reg(uint32_t addr)
{
    return REG32(addr);
}

static void print_vector(const char *name, uint32_t *data, uint32_t size)
{
    printf("%s = [", name);

    for (uint32_t i = 0; i < size; i++) {
        printf("0x%08x", data[i]);

        if (i != size - 1) {
            printf(", ");
        }
    }

    printf("]\n");
}

int main(void)
{
    printf("\n--- HDC accelerator test ---\n");

    printf("HDC base address: 0x%08x\n", (uint32_t)HDC_ACCELERATOR_START_ADDRESS);

    // Inicializar vector de salida
    for (uint32_t i = 0; i < INPUT_SIZE; i++) {
        input_data_c[i] = 0x00000000;
    }

    print_vector("Input A", input_data_a, INPUT_SIZE);
    print_vector("Input B", input_data_b, INPUT_SIZE);
    print_vector("Output C before", input_data_c, INPUT_SIZE);

    printf("Configuring HDC accelerator...\n");

    hdc_write_reg(HDC_ACCELERATOR_ADDR_A, (uint32_t)input_data_a);
    hdc_write_reg(HDC_ACCELERATOR_ADDR_B, (uint32_t)input_data_b);
    hdc_write_reg(HDC_ACCELERATOR_ADDR_C, (uint32_t)input_data_c);

    hdc_write_reg(HDC_ACCELERATOR_A_SIZE, HDC_VECTOR_SIZE_BITS);
    hdc_write_reg(HDC_ACCELERATOR_B_SIZE, HDC_VECTOR_SIZE_BITS);
    hdc_write_reg(HDC_ACCELERATOR_C_SIZE, HDC_VECTOR_SIZE_BITS);

    hdc_write_reg(HDC_ACCELERATOR_SEL_OP, HDC_SEL_OP_BINDING);

    printf("ADDR_A  = 0x%08x\n", hdc_read_reg(HDC_ACCELERATOR_ADDR_A));
    printf("ADDR_B  = 0x%08x\n", hdc_read_reg(HDC_ACCELERATOR_ADDR_B));
    printf("ADDR_C  = 0x%08x\n", hdc_read_reg(HDC_ACCELERATOR_ADDR_C));
    printf("A_SIZE  = %u bits\n", hdc_read_reg(HDC_ACCELERATOR_A_SIZE));
    printf("B_SIZE  = %u bits\n", hdc_read_reg(HDC_ACCELERATOR_B_SIZE));
    printf("C_SIZE  = %u bits\n", hdc_read_reg(HDC_ACCELERATOR_C_SIZE));
    printf("SEL_OP  = %u\n", hdc_read_reg(HDC_ACCELERATOR_SEL_OP));

    printf("Starting accelerator...\n");

    hdc_write_reg(HDC_ACCELERATOR_START, 1u);

    uint32_t timeout = HDC_TIMEOUT;

    while ((hdc_read_reg(HDC_ACCELERATOR_DONE) & 0x1u) == 0u) {
        timeout--;

        if (timeout == 0u) {
            printf("ERROR: HDC accelerator timeout\n");
            print_vector("Output C after timeout", input_data_c, INPUT_SIZE);
            return 1;
        }
    }

    printf("Accelerator finished correctly.\n");

    print_vector("Output C after", input_data_c, INPUT_SIZE);

    printf("--- HDC accelerator test finished ---\n");

    return 0;
}