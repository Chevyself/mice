#include <stdio.h>
#include <windows.h>

BOOL keyStates[256] = {0};
BOOL isProfileActive = FALSE;
float ySpeed = 0;
float xSpeed = 0;
float speedFactor = 0.5;
float decelerationFactor = 1.3;
float maxSpeed = 10;
HHOOK hook;

INPUT createMouseInputType() {
  INPUT input;
  ZeroMemory(&input, sizeof(input));
  input.type = INPUT_MOUSE;
  return input;
}

void moveMouse(int dx, int dy) {
  INPUT input = createMouseInputType();
  input.mi.dx = dx;
  input.mi.dy = dy;
  input.mi.dwFlags = MOUSEEVENTF_MOVE;
  SendInput(1, &input, sizeof(INPUT));
}

void leftClick() {
    INPUT input = createMouseInputType();
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &input, sizeof(INPUT));
}

void releaseLeftClick() {
    INPUT input = createMouseInputType();
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &input, sizeof(INPUT));
}

void rightClick() {
    INPUT input = createMouseInputType();
    input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
    SendInput(1, &input, sizeof(INPUT));
}

void releaseRightClick() {
    INPUT input = createMouseInputType();
    input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
    SendInput(1, &input, sizeof(INPUT));
}

DWORD WINAPI CustomLoop(LPVOID lpParam) {
  while (1) {
    // H
    if (keyStates[0x48]) {
      xSpeed -= speedFactor;
    } else if (xSpeed < 0) {
      xSpeed += decelerationFactor;
    } 
    // J
    if (keyStates[0x4A]) {
      ySpeed += speedFactor;
    } else if (ySpeed > 0) {
      ySpeed -= decelerationFactor;
    }
    // K
    if (keyStates[0x4B]) {
      ySpeed -= speedFactor;
    } else if (ySpeed < 0) {
      ySpeed += decelerationFactor;
    }
    // L
    if (keyStates[0x4C]) {
      xSpeed += speedFactor;
    } else if (xSpeed > 0) {
      xSpeed -= decelerationFactor;
    }
    printf("xSpeed: %d, ySpeed: %d\n", xSpeed, ySpeed);
    if (xSpeed > 0 && xSpeed > maxSpeed) {
      xSpeed = maxSpeed;
    } else if (xSpeed < 0 && xSpeed < -maxSpeed) {
      xSpeed = -maxSpeed;
    }
    if (ySpeed > 0 && ySpeed > maxSpeed) {
      ySpeed = maxSpeed;
    } else if (ySpeed < 0 && ySpeed < -maxSpeed) {
      ySpeed = -maxSpeed;
    }
    moveMouse(xSpeed, ySpeed);
    // Simulate 60Hz by sleeping for 16 milliseconds
    Sleep(16);
  }

  return 0;
}


LRESULT CALLBACK KeyProc(int code, WPARAM wParam, LPARAM lParam) {
  if (code == HC_ACTION) {
    KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
    WORD keyCode = kbdStruct->vkCode;
    if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN){
      printf("Pressed %d\n", keyCode);
      if (keyCode == 0x5D) {
        printf("Profile toggled to %s/n", isProfileActive ? "OFF" : "ON");
        isProfileActive = !isProfileActive;
        Sleep(200);
        return 1;
      }
      else if (isProfileActive) {
        keyStates[keyCode] = TRUE;
        if (keyCode == 0xA0) {
          rightClick();
        }
        if (keyCode == 0x20) {
          leftClick();
        }
        return 1;
      }
    }
    else if (wParam == WM_KEYUP || wParam == WM_SYSKEYUP) {
      printf("Released %d\n", keyCode);
      keyStates[keyCode] = FALSE;
      if (isProfileActive) {
        if (keyCode == 0xA0) {
          releaseRightClick();
        }
        if (keyCode == 0x20) {
          releaseLeftClick();
        }
        return 1;
      }
    }
  }
  return CallNextHookEx(hook, code, wParam, lParam);
}

int main() {
  HANDLE threadHandle = CreateThread(NULL, 0, CustomLoop, NULL, 0, NULL);
  if (threadHandle == NULL) {
    printf("Error creating custom loop thread.\n");
    return 1;
  }

  hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyProc, GetModuleHandle(NULL), 0);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnhookWindowsHookEx(hook);
  WaitForSingleObject(threadHandle, INFINITE);

  CloseHandle(threadHandle); 
  return 0;
}
