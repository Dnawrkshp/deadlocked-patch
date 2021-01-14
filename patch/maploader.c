#include "stdio.h"
#include "net.h"
#include "rpc.h"
#include "messageid.h"
#include "mc.h"
#include "string.h"
#include "ui.h"
#include "graphics.h"
#include "pad.h"
#include "gamesettings.h"
#include "game.h"

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
void loadModules(void);

int readLevelVersion(char * name, int * version);

// memory card fd
int fd;
int initialized = 0;
int installState = 0;

// 
char membuffer[256];

// paths for level specific files
char * fWad = "dl/%s.wad";
char * fBg = "dl/%s.bg";
char * fMap = "dl/%s.map";
char * fVersion = "dl/%s.version";

typedef struct MapOverrideMessage
{
    u8 MapId;
    char MapName[128];
} MapOverrideMessage;

typedef struct MapOverrideResponseMessage
{
	int Version;
} MapOverrideResponseMessage;

struct MapLoaderState
{
    u8 Enabled;
    u8 MapId;
	u8 CheckState;
    char MapName[128];
    int LoadingFileSize;
    int LoadingFd;
} State;


//------------------------------------------------------------------------------
int onSetMapOverride(void * connection, void * data)
{
    MapOverrideMessage *payload = (MapOverrideMessage*)data;

	if (payload->MapId == 0)
	{
		State.Enabled = 0;
		State.CheckState = 0;
	}
	else
	{
		// check for version
		int version = -1;
		if (LOAD_MODULES_STATE != 100)
			version = -1;
		else if (!readLevelVersion(payload->MapName, &version))
			version = -2;

#if DEBUG
    	printf("MapId:%d MapName:%s Version:%d\n", payload->MapId, payload->MapName, version);
#endif

		// send response
		SendCustomAppMessage(connection, CUSTOM_MSG_ID_SET_MAP_OVERRIDE_RESPONSE, 4, &version);

		// enable
		if (version >= 0)
		{
			State.Enabled = 1;
			State.CheckState = 0;
			State.MapId = payload->MapId;
			State.LoadingFd = -1;
			State.LoadingFileSize = -1;
			strncpy(State.MapName, payload->MapName, 128);
		}
	}

    return sizeof(MapOverrideMessage);
}

//------------------------------------------------------------------------------
int onServerSentMapIrxModules(void * connection, void * data)
{
    printf("server sent map irx modules\n");

	// initiate loading
	if (LOAD_MODULES_STATE == 0)
		LOAD_MODULES_STATE = 7;

	// 
	loadModules();

	//
	printf("rpcUSBInit: %d\n", rpcUSBInit());

	//
	installState = 2;

    return 0;
}

//------------------------------------------------------------------------------
void loadModules(void)
{
	if (LOAD_MODULES_STATE < 7)
		return;

	if (LOAD_MODULES_STATE != 100)
	{
		//
		SifInitRpc(0);

		//if (USB_FS_ID > 0)
		//	SifUnloadModule(USB_FS_ID);
		//if (USB_SRV_ID > 0)
		//	SifUnloadModule(USB_SRV_ID);

		// Load modules
		printf("Loading MASS: %d\n", USB_FS_ID = SifExecModuleBuffer(USB_FS_BUFFER, USB_FS_SIZE, 0, NULL, NULL));
		printf("Loading USBSERV: %d\n", USB_SRV_ID = SifExecModuleBuffer(USB_SRV_BUFFER, USB_SRV_SIZE, 0, NULL, NULL));
	}

	// 
	//initialized = 0;

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
int readLevelVersion(char * name, int * version)
{
	int r, fd, fSize;

	// Generate version filename
	sprintf(membuffer, fVersion, name);

	// Open
	rpcUSBopen(membuffer, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &fd);

	// Ensure the file was opened successfully
	if (fd < 0)
	{
		printf("error opening file (%s): %d\n", membuffer, fd);
		return 0;	
	}
	
	// Get length of file
	rpcUSBseek(fd, 0, SEEK_END);
	rpcUSBSync(0, NULL, &fSize);

	// Check the file has a valid size
	if (fSize != 4)
	{
		printf("error seeking file (%s): %d\n", membuffer, fSize);
		rpcUSBclose(fd);
		rpcUSBSync(0, NULL, NULL);
		return 0;
	}

	// Go back to start of file
	rpcUSBseek(fd, 0, SEEK_SET);
	rpcUSBSync(0, NULL, NULL);

	// Read map
	if (rpcUSBread(fd, (void*)version, 4) != 0)
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
void onOnlineMenu(void)
{
	// call normal draw routine
	((void (*)(void))0x00707F28)();

	RECT boxRectEnable = {
		{ 0.1, 0.75 },
		{ 0.5, 0.75 },
		{ 0.1, 0.8 },
		{ 0.5, 0.8 }
	};
	RECT boxRectDownload = {
		{ 0.2, 0.35 },
		{ 0.8, 0.35 },
		{ 0.2, 0.65 },
		{ 0.8, 0.65 }
	};
	u32 bgColorEnable = 0x20000000;
	u32 bgColorDownload = 0x70000000;
	int activeUi = uiGetActive();
	if (activeUi != UI_ID_ONLINE_MAIN_MENU)
		return;

	if (installState == 0)
	{
		// if already tried to install then don't show
		if (LOAD_MODULES_STATE != 0)
			return;

		// render message
		gfxScreenSpaceBox(&boxRectEnable, bgColorEnable, bgColorEnable, bgColorEnable, bgColorEnable);
		gfxScreenSpaceText(SCREEN_WIDTH * 0.3, SCREEN_HEIGHT * 0.7, 1, 1, 0x80FFFFFF, "\x16+\x15 Enable Custom Maps", -1);

		// check for pad input
		if (padGetButtonDown(0, PAD_L2 | PAD_R1))
		{
			if (uiShowYesNoDialog("Enable Custom Maps", "Are you sure?") == 1)
			{
				// request irx modules from server
				SendCustomAppMessage(netGetLobbyServerConnection(), CUSTOM_MSG_ID_CLIENT_REQUEST_MAP_IRX_MODULES, 0, 0);
				installState = 1;
			}
		}
	}
	else if (installState == 1)
	{
		// render message
		gfxScreenSpaceBox(&boxRectDownload, bgColorDownload, bgColorDownload, bgColorDownload, bgColorDownload);
		gfxScreenSpaceText(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.45, 1, 1, 0x80FFFFFF, "Downloading modules, please wait...", -1);
	}
	else if (installState == 2)
	{
		uiShowOkDialog("Custom Maps", "Custom maps have been enabled.");

		installState = 3;
	}

	return;
}

//------------------------------------------------------------------------------
void hook(void)
{
	// 
	u32 * hookLoadAddr = (u32*)0x005CFB48;
	u32 * hookCheckAddr = (u32*)0x005CF9B0;
	u32 * hookLoadingScreenAddr = (u32*)0x00705554;
	u32 * hookTableAddr = (u32*)0x00159B20;
	u32 * hookMapAddr = (u32*)0x00557580;
	u32 * hookAudioAddr = (u32*)0x0053F970;
	u32 * hookLoadCdvdAddr = (u32*)0x00163814;

	// menu
	u32 * menuAddr = (u32*)0x00594CB8;

	// Load modules
	u32 * hookLoadModulesAddr = (u32*)0x00161364;

	// For some reason we can't load the IRX modules whenever we want
	// So here we hook into when the game uses rpc calls
	// This triggers when entering the online profile select, leaving profile select, and logging out.
	//if (!initialized || *hookLoadModulesAddr == 0x0C054632)
	//	*hookLoadModulesAddr = 0x0C000000 | ((u32)(&loadHookFunc) / 4);

	// Install hooks
	if (!initialized || *hookLoadAddr == 0x0C058E10)
	{
		*hookTableAddr = 0x0C000000 | ((u32)(&hookedGetTable) / 4);
		*hookLoadingScreenAddr = 0x0C000000 | ((u32)&hookedLoadingScreen / 4);
		*hookLoadCdvdAddr = 0x0C000000 | ((u32)&hookedLoadCdvd / 4);
		*hookCheckAddr = 0x0C000000 | ((u32)(&hookedCheck) / 4);
		*hookLoadAddr = 0x0C000000 | ((u32)(&hookedLoad) / 4);
	}

	// These get hooked after the map loads but before the game starts
	if (!initialized || *hookMapAddr == 0x0C058E02)
		*hookMapAddr = 0x0C000000 | ((u32)(&hookedGetMap) / 4);

	// Hook menu loop
	if (!initialized || *menuAddr == 0x0C1C1FCA)
		*menuAddr = 0x0C000000 | ((u32)(&onOnlineMenu) / 4);
}

//------------------------------------------------------------------------------
void runMapLoader(void)
{
	// 
    InstallCustomMsgHandler(CUSTOM_MSG_ID_SET_MAP_OVERRIDE, &onSetMapOverride);
    InstallCustomMsgHandler(CUSTOM_MSG_ID_SERVER_SENT_MAP_IRX_MODULES, &onServerSentMapIrxModules);

    // hook irx module loading 
    hook();

	// 
	if (!initialized)
	{
		// set map loader defaults
		State.Enabled = 0;
		State.CheckState = 0;

		initialized = 1;
	}

	// force map id to current map override if in staging
	if (State.Enabled == 1 && !isInGame())
	{
		GameSettings * settings = getGameSettings();
		if (settings && settings->GameLoadStartTime > 0)
		{
			settings->GameLevel = State.MapId;
		}
	}
}
