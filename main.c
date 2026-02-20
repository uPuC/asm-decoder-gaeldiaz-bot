/******************************************************************************
Prac 2 - AVR ASM OpCode Decoder
*******************************************************************************/

#include <stdio.h>
#include <inttypes.h>

// Mantenemos el nombre y arreglo original del profesor
const uint8_t flash_mem[] ={ 
    0x00, 0x24, 0xA0, 0xE0, 0xB2, 0xE0, 0x0D, 0x91, 0x00, 0x30, 0xE9, 0xF7, 0x11, 0x97, 0xC0, 0xE0, 0xC4, 
    0xD2, 0xE0, 0x09, 0x91, 0x1E, 0x91, 0x01, 0x17, 0x51, 0xF4, 0x0A, 0x2F, 0x0A, 0x95, 0x1C, 0x2F, 0x65, 
    0x01, 0x17, 0xB9, 0xF7, 0x0B, 0x2F, 0x1D, 0x2F, 0x01, 0x17, 0x99, 0xF7, 0x03, 0x94, 0x00, 0x00 
};

const uint16_t inst16_table[] = {
  {0x0}, //NOP
};

enum{
    e_NOP,
};

// Op Code struct original con los tipos faltantes agregados
typedef union {
    uint16_t op16; 
    
    struct {
        uint16_t op4:4;
        uint16_t d5:5;
        uint16_t op7:7;
    } type1; // LD, INC, DEC
    
    struct {
        uint16_t r4:4;
        uint16_t d5:5;
        uint16_t r1:1;
        uint16_t op6:6;
    } type2; // MOV, CP, ADC, SBC, CLR
    
    // Tipo agregado para operaciones inmediatas (LDI, CPI, SUBI, ORI)
    struct {
        uint16_t imm_lo:4;
        uint16_t reg_d:4;
        uint16_t imm_hi:4;
        uint16_t opcode:4;
    } type_imm; 
    
    // Tipo agregado para Word Ops (SBIW)
    struct {
        uint16_t imm_lo:4;
        uint16_t reg_pair:2;
        uint16_t imm_hi:2;
        uint16_t opcode:8;
    } type_word; 
    
    // Tipo agregado para Saltos relativos (BRNE)
    struct {
        uint16_t op_bot:3;
        uint16_t offset:7;
        uint16_t op_top:6;
    } type_branch; 

} Op_Code_t;


int main()
{
    Op_Code_t *instruction;
    printf("- Practica 2: AVR OpCode Decoder -\n");
    printf("----------------------------------\n");
    
    // Decode the instructions by cycling through the array
    for (uint8_t idx = 0; idx < sizeof(flash_mem); idx += 2)
    {
        instruction = (Op_Code_t*) &flash_mem[idx];
        uint16_t current_op = instruction->op16; // Respaldamos el valor de 16 bits

        if (current_op == inst16_table[e_NOP]) {
            printf("NOP\n");
        }
        // Identificar LDI (1110)
        else if ((current_op & 0xF000) == 0xE000) {
            uint8_t dest = 16 + instruction->type_imm.reg_d;
            uint8_t val  = (instruction->type_imm.imm_hi << 4) | instruction->type_imm.imm_lo;
            printf("LDI R%d, 0x%02X\n", dest, val);
        }
        // Identificar CPI (0011)
        else if ((current_op & 0xF000) == 0x3000) {
            uint8_t dest = 16 + instruction->type_imm.reg_d;
            uint8_t val  = (instruction->type_imm.imm_hi << 4) | instruction->type_imm.imm_lo;
            printf("CPI R%d, 0x%02X\n", dest, val);
        }
        // Identificar SUBI (0101)
        else if ((current_op & 0xF000) == 0x5000) {
            uint8_t dest = 16 + instruction->type_imm.reg_d;
            uint8_t val  = (instruction->type_imm.imm_hi << 4) | instruction->type_imm.imm_lo;
            printf("SUBI R%d, 0x%02X\n", dest, val);
        }
        // Identificar ORI (0110)
        else if ((current_op & 0xF000) == 0x6000) {
            uint8_t dest = 16 + instruction->type_imm.reg_d;
            uint8_t val  = (instruction->type_imm.imm_hi << 4) | instruction->type_imm.imm_lo;
            printf("ORI R%d, 0x%02X\n", dest, val);
        }
        // Identificar CLR (0010 01rd dddd rrrr) donde Rd = Rr
        else if ((current_op & 0xFC00) == 0x2400) {
            uint8_t src = (instruction->type2.r1 << 4) | instruction->type2.r4;
            if (instruction->type2.d5 == src) {
                printf("CLR R%d\n", instruction->type2.d5);
            }
        }
        // Identificar MOV (0010 11rd dddd rrrr)
        else if ((current_op & 0xFC00) == 0x2C00) {
            uint8_t src = (instruction->type2.r1 << 4) | instruction->type2.r4;
            printf("MOV R%d, R%d\n", instruction->type2.d5, src);
        }
        // Identificar CP (0001 01rd dddd rrrr)
        else if ((current_op & 0xFC00) == 0x1400) {
            uint8_t src = (instruction->type2.r1 << 4) | instruction->type2.r4;
            printf("CP R%d, R%d\n", instruction->type2.d5, src);
        }
        // Identificar SBC (0000 10rd dddd rrrr)
        else if ((current_op & 0xFC00) == 0x0800) {
            uint8_t src = (instruction->type2.r1 << 4) | instruction->type2.r4;
            printf("SBC R%d, R%d\n", instruction->type2.d5, src);
        }
        // Identificar ADC (0001 11rd dddd rrrr)
        else if ((current_op & 0xFC00) == 0x1C00) {
            uint8_t src = (instruction->type2.r1 << 4) | instruction->type2.r4;
            printf("ADC R%d, R%d\n", instruction->type2.d5, src);
        }
        // Identificar MOVW (0000 0001 dddd rrrr)
        else if ((current_op & 0xFF00) == 0x0100) {
            uint8_t dest = ((current_op >> 4) & 0x0F) * 2;
            uint8_t src  = (current_op & 0x0F) * 2;
            printf("MOVW R%d, R%d\n", dest, src);
        }
        // Identificar LD X+ (1001 000d dddd 1101)
        else if ((current_op & 0xFE0F) == 0x900D) {
            printf("LD R%d, X+\n", instruction->type1.d5);
        }
        // Identificar INC (1001 010d dddd 0011)
        else if ((current_op & 0xFE0F) == 0x9403) {
            printf("INC R%d\n", instruction->type1.d5);
        }
        // Identificar DEC (1001 010d dddd 1010)
        else if ((current_op & 0xFE0F) == 0x940A) {
            printf("DEC R%d\n", instruction->type1.d5);
        }
        // Identificar SBIW (1001 0111 KKKK ddKK)
        else if ((current_op & 0xFF00) == 0x9700) {
            uint8_t pair_base = 24 + (instruction->type_word.reg_pair * 2);
            uint8_t val = (instruction->type_word.imm_hi << 4) | instruction->type_word.imm_lo;
            printf("SBIW R%d:R%d, 0x%02X\n", pair_base + 1, pair_base, val);
        }
        // Identificar BRNE (1111 01kk kkkk k001)
        else if ((current_op & 0xFC07) == 0xF401) {
            int8_t offset = instruction->type_branch.offset;
            if (offset & 0x40) offset |= 0x80; // Ajuste para saltos hacia atras (complemento a 2)
            printf("BRNE PC%+d\n", offset + 1);
        }
        // Identificar RCALL (1101 kkkk kkkk kkkk)
        else if ((current_op & 0xF000) == 0xD000) {
            int16_t offset = current_op & 0x0FFF;
            if (offset & 0x0800) offset |= 0xF000;
            printf("RCALL PC%+d\n", offset + 1);
        }
        else {
            printf("unknown\n");
        }
    }
    return 0;
}
