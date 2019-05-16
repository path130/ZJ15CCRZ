#pragma once

CAppBase* CreateMainApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateBlackScreenApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateUpgradeApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateStatusApp(DWORD pHwnd, DWORD lparam, DWORD zParam);
CAppBase* CreateTalkingApp(DWORD wParam, DWORD lParam, DWORD zParam);

// 主界面
CAppBase* CreateRecordApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateCallMainApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateMonitorMainApp(DWORD pHwnd, DWORD lparam, DWORD zParam);
CAppBase* CreateSmartHomeApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSafeMainApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSystemSetApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateUserSetApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreatePwdInputApp(DWORD pHwnd, DWORD lparam, DWORD zParam);

// 监视
CAppBase* CreateMonitorApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateIPCMonitorApp(DWORD wParam, DWORD lParam, DWORD zParam);

// 安防
CAppBase* CreateSafeSetApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSafeDisalarmApp(DWORD wParam, DWORD lParam, DWORD zParam);

// 用户设置
CAppBase* CreateSetBrightApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetDelayApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetInfoApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetLanguageApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetPwdApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetRingApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetScreenSaverApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetTimeDateApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetVolumeApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetWallPaperApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetCalibrateApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateSetCleanScreenApp(DWORD wParam, DWORD lParam, DWORD zParam);

// 系统设置
CAppBase* CreatePrjColorApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreatePrjCodeApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreatePrjDoorApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreatePrjIPCameraApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreatePrjPwdApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreatePrjResetApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreatePrjSafeApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreatePrjUpgradeApp(DWORD wParam, DWORD lParam, DWORD zParam);

// 记录中心
CAppBase* CreateRecAlarmApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateRecCallApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateRecLiuyanApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateRecLiuyingApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateRecMessageApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateRecPhotoApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateRecSafeApp(DWORD wParam, DWORD lParam, DWORD zParam);

// 可视对讲
CAppBase* CreateCallLineApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateCallRoomApp(DWORD wParam, DWORD lParam, DWORD zParam);
CAppBase* CreateTelBookApp(DWORD wParam, DWORD lParam, DWORD zParam);

