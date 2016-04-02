#pragma once

#ifndef _HARDWARE_ID_H_
#define _HARDWARE_ID_H_

#define  FILE_DEVICE_SCSI              0x0000001b
#define  IOCTL_SCSI_MINIPORT_IDENTIFY  ( ( FILE_DEVICE_SCSI << 16 ) + 0x0501 )

#define  IOCTL_SCSI_MINIPORT 0x0004D008  //  see NTDDSCSI.H for definition

#define  IDENTIFY_BUFFER_SIZE  512
#define  SENDIDLENGTH  ( sizeof( SENDCMDOUTPARAMS ) + IDENTIFY_BUFFER_SIZE )

#define  IDE_ATAPI_IDENTIFY  0xA1  //  Returns ID sector for ATAPI.
#define  IDE_ATA_IDENTIFY    0xEC  //  Returns ID sector for ATA.
#define  DFP_RECEIVE_DRIVE_DATA   0x0007c088

#include "Md5.h"

typedef struct _IDSECTOR
{
	USHORT  wGenConfig;
	USHORT  wNumCyls;
	USHORT  wReserved;
	USHORT  wNumHeads;
	USHORT  wBytesPerTrack;
	USHORT  wBytesPerSector;
	USHORT  wSectorsPerTrack;
	USHORT  wVendorUnique[3];
	CHAR    sSerialNumber[20];
	USHORT  wBufferType;
	USHORT  wBufferSize;
	USHORT  wECCSize;
	CHAR    sFirmwareRev[8];
	CHAR    sModelNumber[40];
	USHORT  wMoreVendorUnique;
	USHORT  wDoubleWordIO;
	USHORT  wCapabilities;
	USHORT  wReserved1;
	USHORT  wPIOTiming;
	USHORT  wDMATiming;
	USHORT  wBS;
	USHORT  wNumCurrentCyls;
	USHORT  wNumCurrentHeads;
	USHORT  wNumCurrentSectorsPerTrack;
	ULONG   ulCurrentSectorCapacity;
	USHORT  wMultSectorStuff;
	ULONG   ulTotalAddressableSectors;
	USHORT  wSingleWordDMA;
	USHORT  wMultiWordDMA;
	BYTE    bReserved[128];
} IDSECTOR, *PIDSECTOR;

typedef struct _DRIVERSTATUS

{
	BYTE  bDriverError;  //  Error code from driver, or 0 if no error.
	BYTE  bIDEStatus;    //  Contents of IDE Error register.
	//  Only valid when bDriverError is SMART_IDE_ERROR.
	BYTE  bReserved[2];  //  Reserved for future expansion.
	DWORD  dwReserved[2];  //  Reserved for future expansion.
} DRIVERSTATUS, *PDRIVERSTATUS, *LPDRIVERSTATUS;

typedef struct _SENDCMDOUTPARAMS
{
	DWORD         cBufferSize;   //  Size of bBuffer in bytes
	DRIVERSTATUS  DriverStatus;  //  Driver status structure.
	BYTE          bBuffer[1];    //  Buffer of arbitrary length in which to store the data read from the                                                       // drive.
} SENDCMDOUTPARAMS, *PSENDCMDOUTPARAMS, *LPSENDCMDOUTPARAMS;

typedef struct _SRB_IO_CONTROL
{
	ULONG HeaderLength;
	UCHAR Signature[8];
	ULONG Timeout;
	ULONG ControlCode;
	ULONG ReturnCode;
	ULONG Length;
} SRB_IO_CONTROL, *PSRB_IO_CONTROL;

typedef struct _IDEREGS
{
	BYTE bFeaturesReg;       // Used for specifying SMART "commands".
	BYTE bSectorCountReg;    // IDE sector count register
	BYTE bSectorNumberReg;   // IDE sector number register
	BYTE bCylLowReg;         // IDE low order cylinder value
	BYTE bCylHighReg;        // IDE high order cylinder value
	BYTE bDriveHeadReg;      // IDE drive/head register
	BYTE bCommandReg;        // Actual IDE command.
	BYTE bReserved;          // reserved for future use.  Must be zero.
} IDEREGS, *PIDEREGS, *LPIDEREGS;

typedef struct _SENDCMDINPARAMS
{
	DWORD     cBufferSize;   //  Buffer size in bytes
	IDEREGS   irDriveRegs;   //  Structure with drive register values.
	BYTE bDriveNumber;       //  Physical drive number to send 
	//  command to (0,1,2,3).
	BYTE bReserved[3];       //  Reserved for future expansion.
	DWORD     dwReserved[4]; //  For future use.
	BYTE      bBuffer[1];    //  Input buffer.
} SENDCMDINPARAMS, *PSENDCMDINPARAMS, *LPSENDCMDINPARAMS;

typedef struct _GETVERSIONOUTPARAMS
{
	BYTE bVersion;      // Binary driver version.
	BYTE bRevision;     // Binary driver revision.
	BYTE bReserved;     // Not used.
	BYTE bIDEDeviceMap; // Bit map of IDE devices.
	DWORD fCapabilities; // Bit mask of driver capabilities.
	DWORD dwReserved[4]; // For future use.
} GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

class CHwID
{
public:
	bool GetHwID(CStringA &AHash);
	//bool GetHwID( MD5VAL* MD5Val);
protected:
	BOOL GetCpuId();
	BOOL GetHdSn();
	BOOL GetMAC();
	BOOL WinNTHDSerialNumAsPhysicalRead( char* dwSerial, UINT* puSerialLen, UINT uMaxSerialLen );
	BOOL DoIdentify( HANDLE hPhysicalDriveIOCTL, PSENDCMDINPARAMS pSCIP, PSENDCMDOUTPARAMS pSCOP, BYTE bIDCmd, BYTE bDriveNum, PDWORD lpcbBytesReturned );
	BOOL WinNTHDSerialNumAsScsiRead( char* dwSerial, UINT* puSerialLen, UINT uMaxSerialLen );
protected:
	UINT m_str_len;
	char	m_buff[4096];
};
#define GetCompouterHash(AHash) {\
    CHwID obj; obj.GetHwID(AHash); }

#define STORE_CLASSICAL_NAME(x)		sprintf (ChipID.ProcessorName, x)
#define STORE_TLBCACHE_INFO(x,y)	x = (x < y) ? y : x
#define VENDOR_STRING_LENGTH		(12 + 1)
#define CHIPNAME_STRING_LENGTH		(48 + 1)
#define SERIALNUMBER_STRING_LENGTH	(29 + 1)
#define TLBCACHE_INFO_UNITS			(15)
#define CLASSICAL_CPU_FREQ_LOOP		10000000
#define RDTSC_INSTRUCTION			_asm _emit 0x0f _asm _emit 0x31
#define	CPUSPEED_I32TO64(x, y)		(((__int64) x << 32) + y)

#define CPUID_AWARE_COMPILER
#ifdef CPUID_AWARE_COMPILER
#define CPUID_INSTRUCTION		cpuid
#else
#define CPUID_INSTRUCTION		_asm _emit 0x0f _asm _emit 0xa2
#endif

#define MMX_FEATURE					0x00000001
#define MMX_PLUS_FEATURE			0x00000002
#define SSE_FEATURE					0x00000004
#define SSE2_FEATURE				0x00000008
#define AMD_3DNOW_FEATURE			0x00000010
#define AMD_3DNOW_PLUS_FEATURE		0x00000020
#define IA64_FEATURE				0x00000040
#define MP_CAPABLE					0x00000080
#define HYPERTHREAD_FEATURE			0x00000100
#define SERIALNUMBER_FEATURE		0x00000200
#define APIC_FEATURE				0x00000400
#define SSE_FP_FEATURE				0x00000800
#define SSE_MMX_FEATURE				0x00001000
#define CMOV_FEATURE				0x00002000
#define MTRR_FEATURE				0x00004000
#define L1CACHE_FEATURE				0x00008000
#define L2CACHE_FEATURE				0x00010000
#define L3CACHE_FEATURE				0x00020000
#define ACPI_FEATURE				0x00040000
#define THERMALMONITOR_FEATURE		0x00080000
#define TEMPSENSEDIODE_FEATURE		0x00100000
#define FREQUENCYID_FEATURE			0x00200000
#define VOLTAGEID_FREQUENCY			0x00400000

typedef	void (*DELAY_FUNC)(unsigned int uiMS);

class CPUSpeed {
public:
	CPUSpeed ();
	~CPUSpeed ();

	// Variables.
	int CPUSpeedInMHz;

	// Functions.
	__int64 __cdecl GetCyclesDifference (DELAY_FUNC, unsigned int);

private:
	// Functions.
	static void Delay (unsigned int);
	static void DelayOverhead (unsigned int);

protected:

};

class CPUInfo {
public:
	CPUInfo ();
	~CPUInfo ();

	char * GetVendorString ();
	char * GetVendorID ();
	char * GetTypeID ();
	char * GetFamilyID ();
	char * GetModelID ();
	char * GetSteppingCode ();
	char * GetExtendedProcessorName ();
	char * GetProcessorSerialNumber ();
	int GetLogicalProcessorsPerPhysical ();
	int GetProcessorClockFrequency ();
	int GetProcessorAPICID ();
	int GetProcessorCacheXSize (DWORD);
	bool DoesCPUSupportFeature (DWORD);

	bool __cdecl DoesCPUSupportCPUID ();

private:
	typedef struct tagID {
		int Type;
		int Family;
		int Model;
		int Revision;
		int ExtendedFamily;
		int ExtendedModel;
		char ProcessorName[CHIPNAME_STRING_LENGTH];
		char Vendor[VENDOR_STRING_LENGTH];
		char SerialNumber[SERIALNUMBER_STRING_LENGTH];
	} ID;

	typedef struct tagCPUPowerManagement {
		bool HasVoltageID;
		bool HasFrequencyID;
		bool HasTempSenseDiode;
	} CPUPowerManagement;

	typedef struct tagCPUExtendedFeatures {
		bool Has3DNow;
		bool Has3DNowPlus;
		bool SupportsMP;
		bool HasMMXPlus;
		bool HasSSEMMX;
		bool SupportsHyperthreading;
		int LogicalProcessorsPerPhysical;
		int APIC_ID;
		CPUPowerManagement PowerManagement;
	} CPUExtendedFeatures;	

	typedef struct CPUtagFeatures {
		bool HasFPU;
		bool HasTSC;
		bool HasMMX;
		bool HasSSE;
		bool HasSSEFP;
		bool HasSSE2;
		bool HasIA64;
		bool HasAPIC;
		bool HasCMOV;
		bool HasMTRR;
		bool HasACPI;
		bool HasSerial;
		bool HasThermal;
		int CPUSpeed;
		int L1CacheSize;
		int L2CacheSize;
		int L3CacheSize;
		CPUExtendedFeatures ExtendedFeatures;
	} CPUFeatures;

	enum Manufacturer {
		AMD, Intel, NSC, UMC, Cyrix, NexGen, IDT, Rise, Transmeta, UnknownManufacturer
	};

	// Functions.
	bool __cdecl RetrieveCPUFeatures ();
	bool __cdecl RetrieveCPUIdentity ();
	bool __cdecl RetrieveCPUCacheDetails ();
	bool __cdecl RetrieveClassicalCPUCacheDetails ();
	bool __cdecl RetrieveCPUClockSpeed ();
	bool __cdecl RetrieveClassicalCPUClockSpeed ();
	bool __cdecl RetrieveCPUExtendedLevelSupport (int);
	bool __cdecl RetrieveExtendedCPUFeatures ();
	bool __cdecl RetrieveProcessorSerialNumber ();
	bool __cdecl RetrieveCPUPowerManagement ();
	bool __cdecl RetrieveClassicalCPUIdentity ();
	bool __cdecl RetrieveExtendedCPUIdentity ();

	// Variables.
	Manufacturer ChipManufacturer;
	CPUFeatures Features;
	CPUSpeed * Speed;
	ID ChipID;

protected:

};

#endif //_HARDWARE_ID_H_