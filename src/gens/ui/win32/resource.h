// Resources for Gens/GS Win32

// Icons
#define IDI_GENS_APP		0x0001
#define IDI_GENS_APP_OLD	0x0002
#define IDI_GENS_MD		0x0003
#define IDI_SONIC		0x0004
#define IDI_SONIC_WAITING	0x0005

#define IDI_ARROW_UP		0x0010
#define IDI_ARROW_DOWN		0x0011

// Bitmaps
#define IDB_GENS_LOGO_SMALL	0x0501
#define IDB_GENS_LOGO_BIG	0x0502

// Accelerator Table for non-menu commands
#define IDR_GENS_WINDOW_ACCEL_NONMENU	0x0201

// Buttons
#define IDAPPLY			0x000F
#define IDC_BTN_ADD		0x1011
#define IDC_BTN_DELETE		0x1012
#define IDC_BTN_DEACTIVATEALL	0x1013

// Change Button. (LOWORD(wParam) & 0xFF) == file ID.
#define IDC_BTN_CHANGE		0x1100

// Trackbars
#define IDC_TRK_CA_CONTRAST	0x2001
#define IDC_TRK_CA_BRIGHTNESS	0x2002

// Zip Select Dialog
#define IDD_ZIPSELECT		0x3000
#define IDC_ZIPSELECT_LSTFILES	0x3001

// Controller Configuration Dialog
#define IDD_CONTROLLER_CONFIG	0x4000

// Controller Configuration Dialog - Reconfigure. (LOWORD(wParam) & 0xFF) == controller ID.
#define IDD_CONTROLLER_CONFIG_RECONFIGURE	0x4100

// Country Code Dialog
#define IDD_COUNTRY_CODE	0x5000
#define IDC_COUNTRY_CODE_UP	0x5001
#define IDC_COUNTRY_CODE_DOWN	0x5002

// Plugin Manager Window
#define	IDD_PMGR_WINDOW			0x6000
#define	IDC_PMGR_WINDOW_LSTPLUGINLIST	0x6100
