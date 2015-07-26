/** @file
*  Plugin library for setting up dynamic PCDs for TerminalDxe, from fw_cfg
*
*  Copyright (C) 2015-2016, Red Hat, Inc.
*  Copyright (c) 2014, Linaro Ltd. All rights reserved.<BR>
*
*  This program and the accompanying materials are licensed and made available
*  under the terms and conditions of the BSD License which accompanies this
*  distribution.  The full text of the license may be found at
*  http://opensource.org/licenses/bsd-license.php
*
*  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
*  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR
*  IMPLIED.
*
**/

#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/QemuFwCfgLib.h>

STATIC
RETURN_STATUS
GetNamedFwCfgBoolean (
  IN  CONST CHAR8 *FwCfgFileName,
  OUT BOOLEAN     *Setting
  )
{
  RETURN_STATUS        Status;
  FIRMWARE_CONFIG_ITEM FwCfgItem;
  UINTN                FwCfgSize;
  UINT8                Value[3];

  Status = QemuFwCfgFindFile (FwCfgFileName, &FwCfgItem, &FwCfgSize);
  if (RETURN_ERROR (Status)) {
    return Status;
  }
  if (FwCfgSize > sizeof Value) {
    return RETURN_BAD_BUFFER_SIZE;
  }
  QemuFwCfgSelectItem (FwCfgItem);
  QemuFwCfgReadBytes (FwCfgSize, Value);

  if ((FwCfgSize == 1) ||
      (FwCfgSize == 2 && Value[1] == '\n') ||
      (FwCfgSize == 3 && Value[1] == '\r' && Value[2] == '\n')) {
    switch (Value[0]) {
      case '0':
      case 'n':
      case 'N':
        *Setting = FALSE;
        return RETURN_SUCCESS;

      case '1':
      case 'y':
      case 'Y':
        *Setting = TRUE;
        return RETURN_SUCCESS;

      default:
        break;
    }
  }
  return RETURN_PROTOCOL_ERROR;
}

#define UPDATE_BOOLEAN_PCD_FROM_FW_CFG(TokenName)                             \
          do {                                                                \
            BOOLEAN       Setting;                                            \
            RETURN_STATUS PcdStatus;                                          \
                                                                              \
            if (!RETURN_ERROR (GetNamedFwCfgBoolean (                         \
                    "opt/org.tianocore.edk2.aavmf/" #TokenName, &Setting))) { \
              PcdStatus = PcdSetBoolS (TokenName, Setting);                   \
              ASSERT_RETURN_ERROR (PcdStatus);                                \
            }                                                                 \
          } while (0)

RETURN_STATUS
EFIAPI
TerminalPcdProducerLibConstructor (
  VOID
  )
{
  UPDATE_BOOLEAN_PCD_FROM_FW_CFG (PcdResizeXterm);
  return RETURN_SUCCESS;
}
