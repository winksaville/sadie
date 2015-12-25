/*
 * copyright 2015 wink saville
 *
 * licensed under the apache license, version 2.0 (the "license");
 * you may not use this file except in compliance with the license.
 * you may obtain a copy of the license at
 *
 *     http://www.apache.org/licenses/license-2.0
 *
 * unless required by applicable law or agreed to in writing, software
 * distributed under the license is distributed on an "as is" basis,
 * without warranties or conditions of any kind, either express or implied.
 * see the license for the specific language governing permissions and
 * limitations under the license.
 */

// Ported [from here](http://forum.osdev.org/viewtopic.php?t=16990)

//
// here is the slighlty complicated ACPI poweroff code
//

#include <ac_inttypes.h>
#include <ac_printf.h>
#include <ac_memcmp.h>
#include <ac_io.h>


static ac_u32 *SMI_CMD;
static ac_u8 ACPI_ENABLE;
static ac_u8 ACPI_DISABLE;
static ac_u32 *PM1a_CNT;
static ac_u32 *PM1b_CNT;
static ac_u16 SLP_TYPa;
static ac_u16 SLP_TYPb;
static ac_u16 SLP_EN;
static ac_u16 SCI_EN;
static ac_u8 PM1_CNT_LEN;


struct RSDPtr
{
   ac_u8 Signature[8];
   ac_u8 CheckSum;
   ac_u8 OemID[6];
   ac_u8 Revision;
   ac_u32 *RsdtAddress;
};



struct FACP
{
   ac_u8 Signature[4];
   ac_u32 Length;
   ac_u8 unneded1[40 - 8];
   ac_u32 *DSDT;
   ac_u8 unneded2[48 - 44];
   ac_u32 *SMI_CMD;
   ac_u8 ACPI_ENABLE;
   ac_u8 ACPI_DISABLE;
   ac_u8 unneded3[64 - 54];
   ac_u32 *PM1a_CNT_BLK;
   ac_u32 *PM1b_CNT_BLK;
   ac_u8 unneded4[89 - 72];
   ac_u8 PM1_CNT_LEN;
};

// check if the given address has a valid header
static ac_u32 *acpiCheckRSDPtr(ac_u32 *ptr)
{
   char *sig = "RSD PTR ";
   struct RSDPtr *rsdp = (struct RSDPtr *) ptr;
   ac_u8 *bptr;
   ac_u8 check = 0;
   int i;

   if (ac_memcmp(sig, rsdp, 8) == 0)
   {
      // check checksum rsdpd
      bptr = (ac_u8 *) ptr;
      for (i=0; i<sizeof(struct RSDPtr); i++)
      {
         check += *bptr;
         bptr++;
      }

      // found valid rsdpd   
      if (check == 0) {
         /*
          if (desc->Revision == 0)
            ac_printf("acpi 1");
         else
            ac_printf("acpi 2");
         */
         return (ac_u32 *) rsdp->RsdtAddress;
      }
   }

   return AC_NULL;
}



// finds the acpi header and returns the address of the rsdt
static ac_u32 *acpiGetRSDPtr(void)
{
   ac_u32 *addr;
   ac_u32 *rsdp;

   // search below the 1mb mark for RSDP signature
   for (addr = (ac_u32 *) 0x000E0000; (int) addr<0x00100000; addr += 0x10/sizeof(addr))
   {
      rsdp = acpiCheckRSDPtr(addr);
      if (rsdp != AC_NULL)
         return rsdp;
   }


   // at address 0x40:0x0E is the RM segment of the ebda
   int ebda = *((short *) 0x40E);   // get pointer
   ebda = ebda*0x10 &0x000FFFFF;   // transform segment into linear address

   // search Extended BIOS Data Area for the Root System Description Pointer signature
   for (addr = (ac_u32 *) ebda; (int) addr<ebda+1024; addr+= 0x10/sizeof(addr))
   {
      rsdp = acpiCheckRSDPtr(addr);
      if (rsdp != AC_NULL)
         return rsdp;
   }

   return AC_NULL;
}



// checks for a given header and validates checksum
static int acpiCheckHeader(ac_u32 *ptr, char *sig)
{
   if (ac_memcmp(ptr, sig, 4) == 0)
   {
      char *checkPtr = (char *) ptr;
      int len = *(ptr + 1);
      char check = 0;
      while (0<len--)
      {
         check += *checkPtr;
         checkPtr++;
      }
      if (check == 0)
         return 0;
   }
   return -1;
}

#define DELAY_MILLISEC 1000000
static volatile ac_size_t counter;

void __sleep(ac_size_t millisecs) {
  while (millisecs-- > 0) {
    for (ac_size_t i = 0; i < DELAY_MILLISEC; i++ ) {
      __atomic_add_fetch(&counter, 1, __ATOMIC_ACQUIRE);
    }
  }
}

int acpiEnable(void)
{
   // check if acpi is enabled
   if ( (inw((ac_u32) PM1a_CNT) &SCI_EN) == 0 )
   {
      // check if acpi can be enabled
      if (SMI_CMD != 0 && ACPI_ENABLE != 0)
      {
         outb((ac_u32) SMI_CMD, ACPI_ENABLE); // send acpi enable command
         // give 3 seconds time to enable acpi
         int i;
         for (i=0; i<300; i++ )
         {
            if ( (inw((ac_u32) PM1a_CNT) &SCI_EN) == 1 )
               break;
            __sleep(10);
         }
         if (PM1b_CNT != 0)
            for (; i<300; i++ )
            {
               if ( (inw((ac_u32) PM1b_CNT) &SCI_EN) == 1 )
                  break;
               __sleep(10);
            }
         if (i<300) {
            ac_printf("enabled acpi.\n");
            return 0;
         } else {
            ac_printf("couldn't enable acpi.\n");
            return -1;
         }
      } else {
         ac_printf("no known way to enable acpi.\n");
         return -1;
      }
   } else {
      //ac_printf("acpi was already enabled.\n");
      return 0;
   }
}



//
// bytecode of the \_S5 object
// -----------------------------------------
//        | (optional) |    |    |    |   
// NameOP | \          | _  | S  | 5  | _
// 08     | 5A         | 5F | 53 | 35 | 5F
// 
// -----------------------------------------------------------------------------------------------------------
//           |           |              | ( SLP_TYPa   ) | ( SLP_TYPb   ) | ( Reserved   ) | (Reserved    )
// PackageOP | PkgLength | NumElements  | byteprefix Num | byteprefix Num | byteprefix Num | byteprefix Num
// 12        | 0A        | 04           | 0A         05  | 0A          05 | 0A         05  | 0A         05
//
//----this-structure-was-also-seen----------------------
// PackageOP | PkgLength | NumElements | 
// 12        | 06        | 04          | 00 00 00 00
//
// (Pkglength bit 6-7 encode additional PkgLength bytes [shouldn't be the case here])
//
int initAcpi(void)
{
   ac_u32 *ptr = acpiGetRSDPtr();

   // check if address is correct  ( if acpi is available on this pc )
   if (ptr != AC_NULL && acpiCheckHeader(ptr, "RSDT") == 0)
   {
      // the RSDT contains an unknown number of pointers to acpi tables
      int entrys = *(ptr + 1);
      entrys = (entrys-36) /4;
      ptr += 36/4;   // skip header information

      while (0<entrys--)
      {
         // check if the desired table is reached
         if (acpiCheckHeader((ac_u32 *) *ptr, "FACP") == 0)
         {
            entrys = -2;
            struct FACP *facp = (struct FACP *) *ptr;
            if (acpiCheckHeader((ac_u32 *) facp->DSDT, "DSDT") == 0)
            {
               // search the \_S5 package in the DSDT
               char *S5Addr = (char *) facp->DSDT +36; // skip header
               int dsdtLength = *(facp->DSDT+1) -36;
               while (0 < dsdtLength--)
               {
                  if (ac_memcmp(S5Addr, "_S5_", 4) == 0)
                     break;
                  S5Addr++;
               }
               // check if \_S5 was found
               if (dsdtLength > 0)
               {
                  // check for valid AML structure
                  if ( ( *(S5Addr-1) == 0x08 || ( *(S5Addr-2) == 0x08 && *(S5Addr-1) == '\\') ) && *(S5Addr+4) == 0x12 )
                  {
                     S5Addr += 5;
                     S5Addr += ((*S5Addr &0xC0)>>6) +2;   // calculate PkgLength size

                     if (*S5Addr == 0x0A)
                        S5Addr++;   // skip byteprefix
                     SLP_TYPa = *(S5Addr)<<10;
                     S5Addr++;

                     if (*S5Addr == 0x0A)
                        S5Addr++;   // skip byteprefix
                     SLP_TYPb = *(S5Addr)<<10;

                     SMI_CMD = facp->SMI_CMD;

                     ACPI_ENABLE = facp->ACPI_ENABLE;
                     ACPI_DISABLE = facp->ACPI_DISABLE;

                     PM1a_CNT = facp->PM1a_CNT_BLK;
                     PM1b_CNT = facp->PM1b_CNT_BLK;
                     
                     PM1_CNT_LEN = facp->PM1_CNT_LEN;

                     SLP_EN = 1<<13;
                     SCI_EN = 1;

                     return 0;
                  } else {
                     ac_printf("\\_S5 parse error.\n");
                  }
               } else {
                  ac_printf("\\_S5 not present.\n");
               }
            } else {
               ac_printf("DSDT invalid.\n");
            }
         }
         ptr++;
      }
      ac_printf("no valid FACP present.\n");
   } else {
      ac_printf("no acpi.\n");
   }

   return -1;
}

void acpiPowerOff(void)
{
   // SCI_EN is set to 1 if acpi shutdown is possible
   if (SCI_EN == 0)
      return;

   acpiEnable();

   // send the shutdown command
   outw((ac_u32) PM1a_CNT, SLP_TYPa | SLP_EN );
   if ( PM1b_CNT != 0 )
      outw((ac_u32) PM1b_CNT, SLP_TYPb | SLP_EN );

   ac_printf("acpi poweroff failed.\n");
}
