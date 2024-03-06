i#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PAGES_PER_PROC 8
#define MAX_PROCS 2
#define PHYSICAL_ADDRESS_MASK 0xFFFFFF
#define LOGICAL_ADDRESS_MASK 0x00007fff
#define VALID_BIT_MASK 0x80000000 /* bit 32 */
#define OFFSET_MASK 0x0FFF
#define PAGE_MASK 0x7
#define OFFSET_BIT_WIDTH 0xC

#define GET_PAGE_NUMBER(log_addr) ((log_addr) >> (OFFSET_BIT_WIDTH)) & (PAGE_MASK)
#define GET_OFFSET(log_addr) (log_addr) & (OFFSET_MASK)
#define GET_PHYSICAL_ADDRESS(pte, offset) ((PHYSICAL_ADDRESS_MASK) & ((pte) << (OFFSET_BIT_WIDTH)) | (offset))
#define SET_VALID_BIT(pte) (pte |= (VALID_BIT_MASK))
#define CLR_VALID_BIT(pte) (pte &= (~VALID_BIT_MASK))
#define IS_VALID_BIT_SET(pte) (pte & (VALID_BIT_MASK))

int log_to_phys(int log_addr, const int *page_table, int *fault);

int proc0_page_table[MAX_PAGES_PER_PROC] = {0x3AD, 0xF90, 0x273, 0x11E, 0xF00, 0xBA1, 0x000, 0xFFF};

int proc1_page_table[MAX_PAGES_PER_PROC] = {0x111, 0x222, 0x333, 0x444, 0x555, 0x666, 0x777, 0x888};

const int *page_tables[MAX_PROCS] = {proc0_page_table, proc1_page_table};

int main(int argc, char *argv[])
{
    /* note: int is 32-bits on this architecture */
    unsigned int log_addr, phys_addr;
    int fault;

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <logical address in hex>\n", argv[0]);
        exit(1);
    }

    /* arg represents a logical address in hex */
    sscanf(argv[1], "%x", &log_addr);

    /* perform only basic input checking */
    if ((log_addr & LOGICAL_ADDRESS_MASK) != log_addr)
    {
        fprintf(stderr, "input not in logical address range\n");
        exit(2);
    }

    printf("logical  address:   %04X\n", log_addr);

    for (size_t i = 0; i < MAX_PAGES_PER_PROC - 1; i++)
    {
        if (i < 4)
        {
            SET_VALID_BIT(proc0_page_table[i]);
        }
        SET_VALID_BIT(proc1_page_table[i]);
    }

    for (size_t i = 0; i < MAX_PROCS; i++)
    {
        phys_addr = log_to_phys(log_addr, page_tables[i], &fault);

        if (fault)
        {
            printf("proc%ld fault! Does not map to a physical address\n", i);
        }
        else
        {
            printf("proc%ld physical address: %06X\n", i, phys_addr);
        }
    }
    return 0;
}

/* simulates MMU operation */
int log_to_phys(int log_addr, const int *page_table, int *fault)
{
    uint32_t pte = page_table[GET_PAGE_NUMBER(log_addr)];
    uint32_t offset = GET_OFFSET(log_addr);
    uint32_t phys_addr = GET_PHYSICAL_ADDRESS(pte, offset);

    *fault = !IS_VALID_BIT_SET(pte);
    return phys_addr;
}
