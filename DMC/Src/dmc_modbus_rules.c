#include "dmc_modbus_rules.h"

struct Rules RulesListEntries[MAX_NUMBER_OF_RULE_LISTS];
uint8_t RulesListActive = RULES_LIST_NUMBER_1;

void dmc_init_ruleslists(void)
{
  dmc_init_ruleslist(RULES_LIST_NUMBER_1);
  dmc_init_ruleslist(RULES_LIST_NUMBER_2);
  // number, FunctionCode, StartRegister, NoOfRegisters
  // If NoOfRegisters is 0, we allow the function code regardless of register

  // Allow FC03 (all registers)
  dmc_add_rule(RULES_LIST_NUMBER_1, FC03, 0, 0);
  // Allow FC16 Registers 2, 3, 4, 5
  dmc_add_rule(RULES_LIST_NUMBER_1, FC16, 2, 4);

  // Allow FC03 Registers 0, 1, 2, 3
  dmc_add_rule(RULES_LIST_NUMBER_2, FC03, 0, 4);
  // Allow FC03 Registers 8, 9, 10, 11
  dmc_add_rule(RULES_LIST_NUMBER_2, FC03, 8, 4);
  // Allow FC16 Registers 10 and 11
  dmc_add_rule(RULES_LIST_NUMBER_2, FC16, 10, 2);

  dmc_set_ruleslist_active(RULES_LIST_NUMBER_1);
}

void dmc_init_ruleslist(uint8_t number)
{
  RulesListEntries[number].NumberOfRulesInList = 0;
}

void dmc_set_ruleslist_active(uint8_t number)
{
  RulesListActive = number;
}

void dmc_add_rule(uint8_t number, uint8_t FunctionCode, uint16_t StartRegister, uint16_t NoOfRegisters)
{
  RulesListEntries[number].RulesListEntries[RulesListEntries[number].NumberOfRulesInList].FunctionCode = FunctionCode;
  RulesListEntries[number].RulesListEntries[RulesListEntries[number].NumberOfRulesInList].StartRegister = StartRegister;
  RulesListEntries[number].RulesListEntries[RulesListEntries[number].NumberOfRulesInList].NoOfRegisters = NoOfRegisters;
  RulesListEntries[number].NumberOfRulesInList++;
}

uint8_t dmc_rule_function_code_allowed(uint8_t FunctionCode, uint16_t StartRegister, uint16_t NoOfRegisters)
{
  uint16_t LastRegisterRequest = StartRegister + NoOfRegisters - 1;

  for (uint8_t i = 0; i < RulesListEntries[RulesListActive].NumberOfRulesInList; i++)
  {
    uint16_t LastRegisterRule = RulesListEntries[RulesListActive].RulesListEntries[i].StartRegister +
        RulesListEntries[RulesListActive].RulesListEntries[i].NoOfRegisters - 1;

    // For each entry in the white list first assume "Forbidden"
    if (RulesListEntries[RulesListActive].RulesListEntries[i].FunctionCode == FunctionCode)
    {
      // Function Code match, now check if registers are in range with rules too

      // If NoOfRegisters is 0, we assume any register for this FC valid
      if (RulesListEntries[RulesListActive].RulesListEntries[i].NoOfRegisters == 0)
      {
        // Matching entry found: any register is allowed, we are done
        return ALLOWED;
      }

      // We compare if first and last registers are within range
      if ((StartRegister >= RulesListEntries[RulesListActive].RulesListEntries[i].StartRegister) &&
          (LastRegisterRequest <= LastRegisterRule))
      {
        // Matching entry found: registers are in allowed range, we are done
        return ALLOWED;
      }
    }
  }

  // If we didn't return a valid entry, there wasn't one.
  return FORFIDDEN;
}
