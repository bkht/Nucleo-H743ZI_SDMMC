#ifndef __DMC_MODBUS_RULES_H
#define __DMC_MODBUS_RULES_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "main.h"
#include <dmc_print.h>

#define MAX_NUMBER_OF_RULE_LISTS          2
#define MAX_NUMBER_OF_RULE_LIST_ENTRIES   256

enum RULES_LIST_NUMBERS {
  RULES_LIST_NUMBER_1 = 0,
  RULES_LIST_NUMBER_2 = 1
};

enum RULES_LIST_ALLOW {
  ALLOWED = 1,
  FORFIDDEN = 0
};

enum RULES_LIST_FUNCTION_CODES {
  FC03 = 3,
  FC16 = 16
};

struct RulesListEntry
{
  uint8_t FunctionCode;
  uint16_t StartRegister;
  uint16_t NoOfRegisters;
};

struct Rules
{
  struct RulesListEntry RulesListEntries[MAX_NUMBER_OF_RULE_LIST_ENTRIES];
  uint16_t NumberOfRulesInList;
};

void dmc_init_ruleslists(void);
void dmc_init_ruleslist(uint8_t number);
void dmc_set_ruleslist_active(uint8_t number);

void dmc_add_rule(uint8_t number, uint8_t FunctionCode, uint16_t StartRegister, uint16_t NoOfRegisters);

uint8_t dmc_rule_function_code_allowed(uint8_t FunctionCode, uint16_t StartRegister, uint16_t NoOfRegisters);


#ifdef __cplusplus
}
#endif
#endif /* __DMC_MODBUS_RULES_H */
