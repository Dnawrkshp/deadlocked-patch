#include <libdl/stdio.h>
#include <libdl/net.h>
#include "rpc.h"
#include "messageid.h"
#include <libdl/mc.h>
#include <libdl/string.h>
#include <libdl/ui.h>
#include <libdl/graphics.h>
#include <libdl/pad.h>
#include <libdl/gamesettings.h>
#include <libdl/game.h>

#include <sifcmd.h>
#include <iopheap.h>
#include <iopcontrol.h>
#include <sifrpc.h>
#include <loadfile.h>

#define NEWLIB_PORT_AWARE
#include <io_common.h>

#define MAP_FRAG_PAYLOAD_MAX_SIZE               (1024)
#define LOAD_MODULES_STATE                      (*(u8*)0x000EFF00)
#define HAS_LOADED_MODULES                      (LOAD_MODULES_STATE == 100)

#define USB_FS_ID                               (*(u8*)0x000EFF04)
#define USB_SRV_ID                              (*(u8*)0x000EFF08)


void hook(void);
void loadModules(void);

int readLevelVersion(char * name, int * version);
int readGlobalVersion(int * version);

void * usbFsModuleStart = (void*)0x000E0000;
int usbFsModuleSize = 0;
void * usbSrvModuleStart = (void*)0x000ED000;
int usbSrvModuleSize = 0;

enum MenuActionId
{
	ACTION_ERROR_LOADING_MODULES = -1,

	ACTION_MODULES_NOT_INSTALLED = 0,
	ACTION_DOWNLOADING_MODULES = 1,
	ACTION_MODULES_INSTALLED = 2,
	ACTION_NEW_MAPS_UPDATE = 3,

	ACTION_NONE = 100
};

// memory card fd
int fd = 0;
int initialized = 0;
int actionState = ACTION_MODULES_NOT_INSTALLED;
int rpcInit = 0;

// 
char membuffer[256];

// paths for level specific files
char * fWad = "dl/%s.wad";
char * fBg = "dl/%s.bg";
char * fMap = "dl/%s.map";
char * fVersion = "dl/%s.version";
char * fGlobalVersion = "dl/version";

typedef struct MapOverrideMessage
{
    u8 MapId;
    char MapName[32];
    char MapFileName[128];
} MapOverrideMessage;

typedef struct MapOverrideResponseMessage
{
	int Version;
} MapOverrideResponseMessage;

typedef struct MapClientRequestModulesMessage
{
	u32 Module1Start;
	u32 Module2Start;
} MapClientRequestModulesMessage;

typedef struct MapServerSentModulesMessage
{
	int Version;
	int Module1Size;
	int Module2Size;
} MapServerSentModulesMessage;

struct MapLoaderState
{
    u8 Enabled;
    u8 MapId;
		u8 CheckState;
    char MapName[32];
    char MapFileName[128];
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
		else if (!readLevelVersion(payload->MapFileName, &version))
			version = -2;

		// print
		DPRINTF("MapId:%d MapName:%s MapFileName:%s Version:%d\n", payload->MapId, payload->MapName, payload->MapFileName, version);

		// send response
		netSendCustomAppMessage(connection, CUSTOM_MSG_ID_SET_MAP_OVERRIDE_RESPONSE, 4, &version);

		// enable
		if (version >= 0)
		{
			State.Enabled = 1;
			State.CheckState = 0;
			State.MapId = payload->MapId;
			State.LoadingFd = -1;
			State.LoadingFileSize = -1;
			strncpy(State.MapName, payload->MapName, 32);
			strncpy(State.MapFileName, payload->MapFileName, 128);
		}
	}

	return sizeof(MapOverrideMessage);
}

//------------------------------------------------------------------------------
int onServerSentMapIrxModules(void * connection, void * data)
{
	DPRINTF("server sent map irx modules\n");

	MapServerSentModulesMessage * msg = (MapServerSentModulesMessage*)data;

	// initiate loading
	if (LOAD_MODULES_STATE == 0)
		LOAD_MODULES_STATE = 7;

	// 
	usbFsModuleSize = msg->Module1Size;
	usbSrvModuleSize = msg->Module2Size;

	// 
	loadModules();

	//
	int init = rpcInit = rpcUSBInit();

	DPRINTF("rpcUSBInit: %d\n", init);
	
	//
	if (init < 0)
	{
		actionState = ACTION_ERROR_LOADING_MODULES;
	}
	else
	{
		int remoteVersion = msg->Version;
		int localVersion = 0;
		if (!readGlobalVersion(&localVersion) || localVersion != remoteVersion)
		{
			// Indicate new version
			actionState = ACTION_NEW_MAPS_UPDATE;
		}
		else
		{
			// Indicate maps installed
			actionState = ACTION_MODULES_INSTALLED;
		}
	}

	return sizeof(MapServerSentModulesMessage);
}

//------------------------------------------------------------------------------
void loadModules(void)
{
	int mod_res = 0;
	if (LOAD_MODULES_STATE < 7)
		return;

	if (LOAD_MODULES_STATE != 100)
	{
		//
		SifInitRpc(0);

		// Load modules
		USB_FS_ID = SifExecModuleBuffer(usbFsModuleStart, usbFsModuleSize, 0, NULL, &mod_res);
		USB_SRV_ID = SifExecModuleBuffer(usbSrvModuleStart, usbSrvModuleSize, 0, NULL, &mod_res);

		DPRINTF("Loading MASS: %d\n", USB_FS_ID);
		DPRINTF("Loading USBSERV: %d\n", USB_SRV_ID);
	}

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

//------------------------------------------------------------------------------
int readFileLength(char * path)
{
	int fd, fSize;

	// Open
	rpcUSBopen(path, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &fd);

	// Ensure the file was opened successfully
	if (fd < 0)
	{
		DPRINTF("error opening file (%s): %d\n", path, fd);
		return -1;
	}
	
	// Get length of file
	rpcUSBseek(fd, 0, SEEK_END);
	rpcUSBSync(0, NULL, &fSize);

	// Close file
	rpcUSBclose(fd);
	rpcUSBSync(0, NULL, NULL);

	return fSize;
}

//------------------------------------------------------------------------------
int readFile(char * path, void * buffer, int length)
{
	int r, fd, fSize;

	// Open
	rpcUSBopen(path, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &fd);

	// Ensure the file was opened successfully
	if (fd < 0)
	{
		DPRINTF("error opening file (%s): %d\n", path, fd);
		return -1;
	}
	
	// Get length of file
	rpcUSBseek(fd, 0, SEEK_END);
	rpcUSBSync(0, NULL, &fSize);

	// limit read length to size of file
	if (fSize < length)
		length = fSize;

	// Go back to start of file
	rpcUSBseek(fd, 0, SEEK_SET);
	rpcUSBSync(0, NULL, NULL);

	// Read map
	if (rpcUSBread(fd, (void*)buffer, length) != 0)
	{
		DPRINTF("error reading from file.\n");
		rpcUSBclose(fd);
		rpcUSBSync(0, NULL, NULL);
		return -1;
	}
	rpcUSBSync(0, NULL, &r);

	// Close file
	rpcUSBclose(fd);
	rpcUSBSync(0, NULL, NULL);

	return r;
}

//------------------------------------------------------------------------------
int readGlobalVersion(int * version)
{
	int r;

	r = readFile(fGlobalVersion, (void*)version, 4);
	if (r != 4)
	{
		DPRINTF("error reading file (%s): %d\n", fGlobalVersion, fd);
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------
int readLevelVersion(char * name, int * version)
{
	int r;

	// Generate version filename
	sprintf(membuffer, fVersion, name);

	// read
	r = readFile(membuffer, (void*)version, 4);
	if (r != 4)
	{
		DPRINTF("error reading file (%s): %d\n", membuffer, fd);
		return 0;
	}

	return 1;
}

//--------------------------------------------------------------
int getLevelSizeUsb()
{
	// Generate wad filename
	sprintf(membuffer, fWad, State.MapFileName);

	// get file length
	State.LoadingFileSize = readFileLength(membuffer);

	// Check the file has a valid size
	if (State.LoadingFileSize <= 0)
	{
		DPRINTF("error seeking file (%s): %d\n", membuffer, State.LoadingFileSize);
		return 0;
	}

	return State.LoadingFileSize;
}

//--------------------------------------------------------------
int readLevelMapUsb(u8 * buf, int size)
{
	// Generate toc filename
	sprintf(membuffer, fMap, State.MapFileName);

	// read
	return readFile(membuffer, (void*)buf, size) > 0;
}

//--------------------------------------------------------------
int readLevelBgUsb(u8 * buf, int size)
{
	// Ensure a wad isn't already open
	if (State.LoadingFd >= 0)
	{
		DPRINTF("readLevelBgUsb() called but a file is already open (%d)!", State.LoadingFd);
		return 0;
	}

	// Generate toc filename
	sprintf(membuffer, fBg, State.MapFileName);

	// read
	return readFile(membuffer, (void*)buf, size) > 0;
}

//--------------------------------------------------------------
int openLevelUsb()
{
	// Ensure a wad isn't already open
	if (State.LoadingFd >= 0)
	{
		DPRINTF("openLevelUsb() called but a file is already open (%d)!", State.LoadingFd);
		return 0;
	}

	// Generate wad filename
	sprintf(membuffer, fWad, State.MapFileName);

	// open wad file
	rpcUSBopen(membuffer, FIO_O_RDONLY);
	rpcUSBSync(0, NULL, &State.LoadingFd);
	
	// Ensure wad successfully opened
	if (State.LoadingFd < 0)
	{
		DPRINTF("error opening file (%s): %d\n", membuffer, State.LoadingFd);
		return 0;									
	}

	// Get length of file
	rpcUSBseek(State.LoadingFd, 0, SEEK_END);
	rpcUSBSync(0, NULL, &State.LoadingFileSize);

	// Check the file has a valid size
	if (State.LoadingFileSize <= 0)
	{
		DPRINTF("error seeking file (%s): %d\n", membuffer, State.LoadingFileSize);
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

	DPRINTF("%s is %d byte long.\n", membuffer, State.LoadingFileSize);
	return State.LoadingFileSize;
}

//--------------------------------------------------------------
int readLevelUsb(u8 * buf)
{
	// Ensure the wad is open
	if (State.LoadingFd < 0 || State.LoadingFileSize <= 0)
	{
		DPRINTF("error opening file: %d\n", State.LoadingFd);
		return 0;									
	}

	// Try to read from usb
	if (rpcUSBread(State.LoadingFd, buf, State.LoadingFileSize) != 0)
	{
		DPRINTF("error reading from file.\n");
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
			DPRINTF("finished reading %d bytes from USB\n", r);
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
            DPRINTF("Error reading level wad from usb\n");
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
char* hookedLoadScreenMapNameString(char * dest, char * src)
{
	if (State.Enabled)
		strncpy(dest, State.MapName, 32);
	else
		strncpy(dest, src, 32);
	return dest;
}

//------------------------------------------------------------------------------
int mapsHasTriedLoading(void)
{
	return LOAD_MODULES_STATE != 0;
}

//------------------------------------------------------------------------------
int mapsDownloadingModules(void)
{
	return actionState == ACTION_DOWNLOADING_MODULES;
}

//------------------------------------------------------------------------------
int mapsPromptEnableCustomMaps(void)
{
	MapClientRequestModulesMessage request = { 0, 0 };
	if (uiShowYesNoDialog("Enable Custom Maps", "Are you sure?") == 1)
	{
		// request irx modules from server
		request.Module1Start = (u32)usbFsModuleStart;
		request.Module2Start = (u32)usbSrvModuleStart;
		netSendCustomAppMessage(netGetLobbyServerConnection(), CUSTOM_MSG_ID_CLIENT_REQUEST_MAP_IRX_MODULES, sizeof(MapClientRequestModulesMessage), &request);
		actionState = ACTION_DOWNLOADING_MODULES;
		return 1;
	}

	return 0;
}

//------------------------------------------------------------------------------
void onMapLoaderOnlineMenu(void)
{
	u32 bgColorDownload = 0x70000000;

	if (actionState == ACTION_DOWNLOADING_MODULES)
	{
		// disable input
		padDisableInput();

		// render background
		gfxScreenSpaceBox(0.2, 0.35, 0.6, 0.3, bgColorDownload);

		// flash color
		u32 downloadColor = 0x80808080;
		int gameTime = ((gameGetTime()/100) % 15);
		if (gameTime > 7)
			gameTime = 15 - gameTime;
		downloadColor += 0x101010 * gameTime;

		// render text
		gfxScreenSpaceText(SCREEN_WIDTH * 0.5, SCREEN_HEIGHT * 0.5, 1, 1, downloadColor, "Downloading modules, please wait...", -1, 4);
	}
	else if (actionState == ACTION_MODULES_INSTALLED)
	{
		// enable input
		padEnableInput();

		uiShowOkDialog("Custom Maps", "Custom maps have been enabled.");

		actionState = ACTION_NONE;
	}
	else if (actionState == ACTION_NEW_MAPS_UPDATE)
	{
		// enable input
		padEnableInput();
		
		uiShowOkDialog("Custom Maps", "New updates are available. Please download them at https://dl.uyaonline.com/maps");
		actionState = ACTION_MODULES_INSTALLED;
	}
	else if (actionState == ACTION_ERROR_LOADING_MODULES)
	{
		// enable input
		padEnableInput();
		
		uiShowOkDialog("Custom Maps", "There was an error loading the custom map modules.");
		actionState = ACTION_NONE;
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
	u32 * hookLoadScreenMapNameStringAddr = (u32*)0x007055B4;

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
	{
		*hookMapAddr = 0x0C000000 | ((u32)(&hookedGetMap) / 4);
	}

	if (!initialized || *hookLoadScreenMapNameStringAddr == 0x0C046A7B)
	{
		*hookLoadScreenMapNameStringAddr = 0x0C000000 | ((u32)(&hookedLoadScreenMapNameString) / 4);
	}
}

//------------------------------------------------------------------------------
void runMapLoader(void)
{
	// 
    netInstallCustomMsgHandler(CUSTOM_MSG_ID_SET_MAP_OVERRIDE, &onSetMapOverride);
    netInstallCustomMsgHandler(CUSTOM_MSG_ID_SERVER_SENT_MAP_IRX_MODULES, &onServerSentMapIrxModules);

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
	if (State.Enabled == 1 && !gameIsIn())
	{
		GameSettings * settings = gameGetSettings();
		if (settings && settings->GameLoadStartTime > 0)
		{
			settings->GameLevel = State.MapId;
		}
	}
}
