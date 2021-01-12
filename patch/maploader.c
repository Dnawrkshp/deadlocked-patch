#include "stdio.h"
#include "net.h"
#include "rpc.h"
#include "messageid.h"
#include "mc.h"
#include "string.h"

#include <sifcmd.h>
#include <iopheap.h>
#include <iopcontrol.h>
#include <sifrpc.h>
#include <loadfile.h>

#define NEWLIB_PORT_AWARE
#include <io_common.h>

#define MAP_FRAG_PAYLOAD_MAX_SIZE               (1024)
#define LOAD_MODULES_STATE                      (*(u8*)0x000AFF00)
#define HAS_LOADED_MODULES                      (LOAD_MODULES_STATE == 100)

#define USB_FS_PATH                             ("/usbhdfsd.irx")
#define USB_FS_SIZE                             (51245)
#define USB_FS_BUFFER                           ((void*)0x00090000)
#define USB_FS_ID                               (*(u8*)0x000AFF04)

#define USB_SRV_PATH                            ("/usbserv.irx")
#define USB_SRV_SIZE                            (4029)
#define USB_SRV_BUFFER                          ((void*)0x0009D000)
#define USB_SRV_ID                              (*(u8*)0x000AFF08)


void hook(void);

// memory card fd
int fd;

// 
char membuffer[256];

// paths for level specific wad and toc
char * fWad = "dl/%s.wad";
char * fBg = "dl/%s.bg";
char * fMap = "dl/%s.map";

typedef struct MapOverrideMessage
{
    u8 MapId;
    char MapName[128];
} MapOverrideMessage;

struct MapLoaderState
{
    u8 Enabled;
    u8 MapId;
    char MapName[128];
    int LoadingFileSize;
    int LoadingFd;
} State;


//------------------------------------------------------------------------------
int onSetMapOverride(void * connection, void * data)
{
    MapOverrideMessage *payload = (MapOverrideMessage*)data;

    printf("MapId:%d MapName:%s\n", payload->MapId, payload->MapName);

    State.Enabled = 1;
    State.MapId = payload->MapId;
    State.LoadingFd = -1;
    State.LoadingFileSize = -1;
    strncpy(State.MapName, payload->MapName, 128);

    return sizeof(MapOverrideMessage);
}

//------------------------------------------------------------------------------
void loadModules(void)
{
	if (LOAD_MODULES_STATE < 7)
		return;

    // test
    State.Enabled = 1;
    State.MapId = 46;
    State.LoadingFd = -1;
    State.LoadingFileSize = -1;

    strncpy(State.MapName, "level44", 128);

	//
	SifInitRpc(0);

    if (USB_FS_ID > 0)
        SifUnloadModule(USB_FS_ID);
    if (USB_SRV_ID > 0)
        SifUnloadModule(USB_SRV_ID);

	// Load modules
	printf("Loading MASS: %d\n", USB_FS_ID = SifExecModuleBuffer(USB_FS_BUFFER, USB_FS_SIZE, 0, NULL, NULL));
	printf("Loading USBSERV: %d\n", USB_SRV_ID = SifExecModuleBuffer(USB_SRV_BUFFER, USB_SRV_SIZE, 0, NULL, NULL));

	// 
	printf("rpcUSBInit: %d\n", rpcUSBInit());

	LOAD_MODULES_STATE = 100;
}

//------------------------------------------------------------------------------
u64 loadHookFunc(u64 a0, u64 a1)
{
	// Load our usb modules
	loadModules();

	// Loads sound driver
	return ((u64 (*)(u64, u64))0x001518C8)(a0, a1);
}

//--------------------------------------------------------------
int getLevelSizeUsb()
{
	int fd = -1;

	// Generate wad filename
	sprintf(membuffer, fWad, State.MapName);

	// open wad file
	rpcUSBopen(membuffer, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &fd);
	
	// Ensure wad successfully opened
	if (fd < 0)
	{
		printf("error opening file (%s): %d\n", membuffer, fd);
		return 0;									
	}

	// Get length of file
	rpcUSBseek(fd, 0, SEEK_END);
	rpcUSBSync(0, NULL, &State.LoadingFileSize);

	// Check the file has a valid size
	if (State.LoadingFileSize <= 0)
	{
		printf("error seeking file (%s): %d\n", membuffer, State.LoadingFileSize);
		rpcUSBclose(fd);
		rpcUSBSync(0, NULL, NULL);
		fd = -1;
		return 0;
	}

	// Close wad
	rpcUSBclose(fd);
	rpcUSBSync(0, NULL, NULL);

	return State.LoadingFileSize;
}

//--------------------------------------------------------------
int readLevelMapUsb(u8 * buf, int size)
{
	int r, fd, fSize;

	// Generate toc filename
	sprintf(membuffer, fMap, State.MapName);

	// Open
	rpcUSBopen(membuffer, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &fd);

	// Ensure the toc was opened successfully
	if (fd < 0)
	{
		printf("error opening file (%s): %d\n", membuffer, fd);
		return 0;	
	}
	
	// Get length of file
	rpcUSBseek(fd, 0, SEEK_END);
	rpcUSBSync(0, NULL, &fSize);

	// Check the file has a valid size
	if (fSize <= 0)
	{
		printf("error seeking file (%s): %d\n", membuffer, fSize);
		rpcUSBclose(fd);
		rpcUSBSync(0, NULL, NULL);
		return 0;
	}

	// Go back to start of file
	rpcUSBseek(fd, 0, SEEK_SET);
	rpcUSBSync(0, NULL, NULL);

	// clamp size to be no larger than file size
	if (size > fSize)
		size = fSize;

	// Read map
	if (rpcUSBread(fd, buf, size) != 0)
	{
		printf("error reading from file.\n");
		rpcUSBclose(fd);
		rpcUSBSync(0, NULL, NULL);
		return 0;
	}
	rpcUSBSync(0, NULL, &r);

	// Close toc
	rpcUSBclose(fd);
	rpcUSBSync(0, NULL, NULL);

	return 1;
}

//--------------------------------------------------------------
int readLevelBgUsb(u8 * buf, int size)
{
	int r, fd;

	// Ensure a wad isn't already open
	if (State.LoadingFd >= 0)
	{
		printf("readLevelBgUsb() called but a file is already open (%d)!", State.LoadingFd);
		return 0;
	}

	// Generate toc filename
	sprintf(membuffer, fBg, State.MapName);

	// Open
	rpcUSBopen(membuffer, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &fd);

	// Ensure the toc was opened successfully
	if (fd < 0)
	{
		printf("error opening file (%s): %d\n", membuffer, fd);
		return 0;	
	}

	// Read bg
	if (rpcUSBread(fd, buf, size) != 0)
	{
		printf("error reading from file.\n");
		rpcUSBclose(fd);
		rpcUSBSync(0, NULL, NULL);
		return 0;
	}
	rpcUSBSync(0, NULL, &r);

	// Close toc
	rpcUSBclose(fd);
	rpcUSBSync(0, NULL, NULL);

	return 1;
}

//--------------------------------------------------------------
int openLevelUsb()
{
	// Ensure a wad isn't already open
	if (State.LoadingFd >= 0)
	{
		printf("openLevelUsb() called but a file is already open (%d)!", State.LoadingFd);
		return 0;
	}

	// Generate wad filename
	sprintf(membuffer, fWad, State.MapName);

	// open wad file
	rpcUSBopen(membuffer, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &State.LoadingFd);
	
	// Ensure wad successfully opened
	if (State.LoadingFd < 0)
	{
		printf("error opening file (%s): %d\n", membuffer, State.LoadingFd);
		return 0;									
	}

	// Get length of file
	rpcUSBseek(State.LoadingFd, 0, SEEK_END);
	rpcUSBSync(0, NULL, &State.LoadingFileSize);

	// Check the file has a valid size
	if (State.LoadingFileSize <= 0)
	{
		printf("error seeking file (%s): %d\n", membuffer, State.LoadingFileSize);
		rpcUSBclose(State.LoadingFd);
		rpcUSBSync(0, NULL, NULL);
		State.LoadingFd = -1;
        State.Enabled = 0;
		return 0;
	}

	// Go back to start of file
	// The read will be called later
	rpcUSBseek(State.LoadingFd, 0, SEEK_SET);
	rpcUSBSync(0, NULL, NULL);

	printf("%s is %d byte long.\n", membuffer, State.LoadingFileSize);
	
	return State.LoadingFileSize;
}

//--------------------------------------------------------------
int readLevelUsb(u8 * buf)
{
	// Ensure the wad is open
	if (State.LoadingFd < 0 || State.LoadingFileSize <= 0)
	{
		printf("error opening file: %d\n", State.LoadingFd);
		return 0;									
	}

	// Try to read from usb
	if (rpcUSBread(State.LoadingFd, buf, State.LoadingFileSize) != 0)
	{
		printf("error reading from file.\n");
		rpcUSBclose(State.LoadingFd);
		rpcUSBSync(0, NULL, NULL);
		State.LoadingFd = -1;
		return 0;
	}
				
	return 1;
}

//------------------------------------------------------------------------------
void hookedLoad(u64 arg0, void * dest, u32 sectorStart, u32 sectorSize, u64 arg4, u64 arg5, u64 arg6)
{
	void (*cdvdLoad)(u64, void*, u32, u32, u64, u64, u64) = (void (*)(u64, void*, u32, u32, u64, u64, u64))0x00163840;

	// Check if loading MP map
	if (State.Enabled && HAS_LOADED_MODULES)
	{
		int fSize = openLevelUsb();
		if (fSize > 0)
		{
			if (readLevelUsb(dest) > 0)
				return;
		}
	}

	// Default to cdvd load if the usb load failed
	cdvdLoad(arg0, dest, sectorStart, sectorSize, arg4, arg5, arg6);
}

//------------------------------------------------------------------------------
u32 hookedCheck(void)
{
	u32 (*check)(void) = (u32 (*)(void))0x00163928;
	int r, cmd;

	// If the wad is not open then we're loading from cdvd
	if (State.LoadingFd < 0 || !State.Enabled)
		return check();

	// Otherwise check to see if we've finished loading the data from USB
	if (rpcUSBSyncNB(0, &cmd, &r) == 1)
	{
		// If the command is USBREAD close and return
		if (cmd == 0x04)
		{
			printf("finished reading %d bytes from USB\n", r);
			rpcUSBclose(State.LoadingFd);
			rpcUSBSync(0, NULL, NULL);
			State.LoadingFd = -1;
			return check();
		}
	}

	// Set bg color to red
	*((vu32*)0x120000e0) = 0x1010B4;

	// Not sure if this is necessary but it doesn't hurt to call the game's native load check
	check();

	// Indicate that we haven't finished loading
	return 1;
}

//------------------------------------------------------------------------------
void hookedLoadingScreen(u64 a0, void * a1, u64 a2, u64 a3, u64 t0, u64 t1, u64 t2)
{
	if (State.Enabled && HAS_LOADED_MODULES && readLevelBgUsb(a1, a3 * 0x800) > 0)
	{

	}
	else
	{
		((void (*)(u64, void *, u64, u64, u64, u64, u64))0x00163808)(a0, a1, a2, a3, t0, t1, t2);
	}
}

//------------------------------------------------------------------------------
void hookedGetTable(u32 startSector, u32 sectorCount, u8 * dest, u32 levelId)
{
    // load table
    ((void (*)(u32,u32,void*))0x00159A00)(startSector, sectorCount, dest);

    // Disable when loading menu
    if (levelId == 0)
    {
        State.Enabled = 0;
        return;
    }

	// Check if loading MP map
	if (State.Enabled && HAS_LOADED_MODULES)
	{
        // Disable if map doesn't match
        if (levelId != State.MapId)
        {
            State.Enabled = 0;
            return;
        }

        int fSize = getLevelSizeUsb();
        if (fSize > 0)
        {
            ((int*)dest)[7] = (fSize / 0x800) + 1;
        }
        else
        {
            State.Enabled = 0;
            printf("Error reading level wad from usb\n");
        }
	}
}

//------------------------------------------------------------------------------
void hookedGetMap(u64 a0, void * dest, u32 startSector, u32 sectorCount, u64 t0, u64 t1, u64 t2)
{
	// Check if loading MP map
	if (State.Enabled && HAS_LOADED_MODULES)
	{
		// We hardcode the size because that's the max that deadlocked can hold
		if (readLevelMapUsb(dest, 0x27400))
			return;
	}

	((void (*)(u64, void*,u32,u32,u64,u64,u64))0x00163808)(a0, dest, startSector, sectorCount, t0, t1, t2);
}

//------------------------------------------------------------------------------
void hookedGetAudio(u64 a0, void * dest, u32 startSector, u32 sectorCount, u64 t0, u64 t1, u64 t2)
{
	((void (*)(u64, void*,u32,u32,u64,u64,u64))0x00163808)(a0, dest, startSector, sectorCount, t0, t1, t2);
}

//------------------------------------------------------------------------------
u64 hookedLoadCdvd(u64 a0, u64 a1, u64 a2, u64 a3, u64 t0, u64 t1, u64 t2)
{
	u64 result = ((u64 (*)(u64,u64,u64,u64,u64,u64,u64))0x00163840)(a0, a1, a2, a3, t0, t1, t2);

	// We try and hook here to just to make sure that after tha game loads
	// We can still load our custom minimap
	hook();

	return result;
}

//------------------------------------------------------------------------------
void hook(void)
{
	u32 * hookloadAddr = (u32*)0x005CFB48;
	u32 * hookcheckAddr = (u32*)0x005CF9B0;
	u32 * loadModulesAddr = (u32*)0x00161364;
	u32 * hookloadingScreenAddr = (u32*)0x00705554;
	u32 * getTableAddr = (u32*)0x00159B20;
	u32 * getMapAddr = (u32*)0x00557580;
	u32 * getAudioAddr = (u32*)0x0053F970;
	u32 * loadCdvdAddr = (u32*)0x00163814;

	// For some reason we can't load the IRX modules whenever we want
	// So here we hook into when the game uses rpc calls
	// This triggers when entering the online profile select, leaving profile select, and logging out.
	if (*loadModulesAddr == 0x0C054632)
		*loadModulesAddr = 0x0C000000 | ((u32)(&loadHookFunc) / 4);

	if (*hookloadAddr == 0x0C058E10)
	{
		*getTableAddr = 0x0C000000 | ((u32)(&hookedGetTable) / 4);
		*hookloadAddr = 0x0C000000 | ((u32)(&hookedLoad) / 4);
		*hookcheckAddr = 0x0C000000 | ((u32)(&hookedCheck) / 4);
		*hookloadingScreenAddr = 0x0C000000 | ((u32)&hookedLoadingScreen / 4);
		*loadCdvdAddr = 0x0C000000 | ((u32)&hookedLoadCdvd / 4);
	}

	// These get hooked after the map loads but before the game starts
	if (*getMapAddr == 0x0C058E02)
		*getMapAddr = 0x0C000000 | ((u32)(&hookedGetMap) / 4);
}

//------------------------------------------------------------------------------
void runMapLoader(void)
{
    int r, lastResult;

    InstallCustomMsgHandler(CUSTOM_MSG_ID_SET_MAP_OVERRIDE, &onSetMapOverride);

    // hook irx module loading 
    hook();

    // try to read modules from memory card
    int loadModuleState = LOAD_MODULES_STATE;

    // for testing purposes kickstart to loading
    if (loadModuleState == 0)
        loadModuleState = 1;

    if (loadModuleState > 0 && loadModuleState < 100)
    {
        // grab last result
        if ((lastResult = McPollSema()) < 0)
            return;

        // error
        if (lastResult < 0 && loadModuleState > 1)
        {
            printf("error result from %d : %d\n", loadModuleState - 1, lastResult);
            LOAD_MODULES_STATE = -loadModuleState;
            return;
        }

        // 
        switch (loadModuleState)
        {
            case 1: // OPEN USB FS MODULE
            {
                // open
                r = McOpen(0, 0, USB_FS_PATH, 1);
                if (r < 0)
                    break;
                
                ++loadModuleState;
                break;
            }
            case 2: // READ USB FS MODULE
            {
                // set fd to last result
                fd = lastResult;

                // read
                r = McRead(fd, USB_FS_BUFFER, USB_FS_SIZE);
                if (r < 0)
                    break;
                
                ++loadModuleState;
                break;
            }
            case 3: // CLOSE USB FS MODULE
            {
                // 
                if (lastResult != USB_FS_SIZE)
                {
                    printf("error reading %s, %d/%d bytes read\n", USB_FS_PATH, lastResult, USB_FS_SIZE);
                    loadModuleState = -loadModuleState;
                    break;
                }

                // close
                r = McClose(fd);
                if (r < 0)
                    break;
                
                ++loadModuleState;
                break;
            }
            case 4: // OPEN USB SERV MODULE
            {
                // open
                fd = McOpen(0, 0, USB_SRV_PATH, 1);
                if (r < 0)
                    break;
                
                ++loadModuleState;
                break;
            }
            case 5: // READ USB SERV MODULE
            {
                // set fd to last result
                fd = lastResult;

                // read
                r = McRead(fd, USB_SRV_BUFFER, USB_SRV_SIZE);
                if (r < 0)
                    break;
                
                ++loadModuleState;
                break;
            }
            case 6: // CLOSE USB SERV MODULE
            {
                // 
                if (lastResult != USB_SRV_SIZE)
                {
                    printf("error reading %s, %d/%d bytes read\n", USB_SRV_PATH, lastResult, USB_SRV_SIZE);
                    loadModuleState = -loadModuleState;
                    break;
                }

                // close
                r = McClose(fd);
                if (r < 0)
                    break;

                printf("done\n");
                
                ++loadModuleState;
                break;
            }
        }

        // update state
        LOAD_MODULES_STATE = loadModuleState;
    }
}
